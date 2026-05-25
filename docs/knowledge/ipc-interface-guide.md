# IPC 接口指南

本文记录 IPC 接口概览、双进程架构、启动顺序、故障恢复和新增 IPC 接口流程。

## 双进程架构

```
┌─────────────────────────┐        IPC (SystemAbility)       ┌──────────────────────────┐
│    storage_manager       │◄──────────────────────────────►│    storage_daemon          │
│    (SA ID: 5003)         │                                 │    (SA ID: 5004)          │
│                           │                                 │                            │
│  StorageManagerProvider   │  StorageDaemonCommunication    │  StorageDaemonProvider     │
│  (Stub 实现)              │  (Manager 侧 Daemon 代理)       │  (Stub 实现)               │
│                           │                                 │                            │
│  DiskManagerService       │                                 │  DiskManager               │
│  VolumeManagerService     │                                 │  VolumeManager             │
│  StorageStatusManager     │                                 │  UserManager               │
│  StorageMonitorService    │                                 │  KeyManager                │
│  AccountSubscriber        │                                 │  QuotaManager              │
│  BundleMgrConnector        │                                 │  NetlinkManager              │
└─────────────────────────┘                                 └──────────────────────────┘
```

**职责划分**：
- Manager：面向应用和其他子系统的入口，处理权限校验、状态缓存、监控通知
- Daemon：面向内核和硬件的底层执行者，操作设备节点、密钥、文件系统
- Manager 不直接操作设备节点或密钥，Daemon 不直接响应应用请求

## 启动顺序

### Daemon 启动

```cpp
// services/storage_daemon/src/main.cpp（核心流程）
int main() {
    // 1. 注册 SA（注意：类名是 StorageDaemonProvider，不是 StorageDaemon）
    sptr<StorageDaemon::StorageDaemonProvider> sd(new StorageDaemon::StorageDaemonProvider());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int ret = samgr->AddSystemAbility(STORAGE_MANAGER_DAEMON_ID, sd);
    
    // 2. 设置优先级
    setpriority(PRIO_PROCESS, 0, -10);
    
    // 3. 初始化核心管理器
    DiskManager::Instance();
    NetlinkManager::Instance()->Start();
    
    // 4. 订阅依赖服务（使用 samgr->SubscribeSystemAbility）
    ret = samgr->SubscribeSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, listenter);
    ret = samgr->SubscribeSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, listenter);
    
    return 0;
}
```

### Manager 启动

```cpp
// services/storage_manager/src/storage_manager_provider.cpp（OnStart 核心流程）
void StorageManagerProvider::OnStart() {
    // 1. 加载扫描结果
    StorageManagerScan::GetInstance().LoadScanResultFromFile();
    
    // 2. 发布 SA
    SystemAbility::Publish(this);
    
    // 3. 启动监控服务
    StorageMonitorService::GetInstance().StartStorageMonitorTask();
    
    // 4. 懒连接 Daemon（首次调用时建立）
    // DelayedSingleton<StorageDaemonCommunication>::GetInstance()->Connect()
}
```

### 启动顺序约束

1. **Daemon 先启动**：内核 netlink 事件需要在 Manager 之前就绪
2. **Manager 后启动**：依赖 Daemon 的服务
3. **懒连接**：Manager 按需连接 Daemon，`StorageDaemonCommunication::Connect()` 在首次 IPC 调用时建立代理

**不要假设 Manager 启动时 Daemon 已经就绪**。连接可能延迟到首次使用时才建立。

## DeathRecipient 和崩溃恢复

### Daemon 崩溃时 Manager 的行为

`StorageDaemonCommunication` 注册了 `SdDeathRecipient`（注意：`StorageDaemonCommunication` 是 `DelayedSingleton`）：

```cpp
// SdDeathRecipient 定义在 storage_daemon_communication.h
class SdDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    SdDeathRecipient() = default;
    virtual ~SdDeathRecipient() = default;
    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
};

// OnRemoteDied 实际调用：
void SdDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object) {
    LOGI("StorageDaemon died");
    // 1. 清空 Daemon 代理指针（注意方法名是 ResetSdProxy，不是 ResetProxy）
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->ResetSdProxy();
    // 2. 对所有活跃用户执行锁屏降级（注意方法名是 ForceLockUserScreen，不是 LockAllActiveUsers）
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->ForceLockUserScreen();
    // 3. 下次 IPC 调用时触发 Connect() 重建连接
}
```

**约束**：不要在 Daemon 崩溃后假设之前的 IPC 状态有效。Daemon 重启后内存状态（卷映射、密钥缓存）为空，Manager 的缓存可能过期。

### Manager 崩溃时 Daemon 的行为

Daemon 侧同样注册 DeathRecipient 监听 Manager。Manager 重启后会重新加载磁盘/卷状态。

## Manager 侧关键接口

定义在 `services/storage_manager/include/ipc/storage_manager_provider.h`：

| 接口分类 | 关键接口 | 说明 |
|----------|----------|------|
| 用户管理 | `PrepareAddUser`, `RemoveUser`, `PrepareStartUser`, `StopUser`, `CompleteAddUser` | 用户生命周期 |
| 密钥管理 | `UpdateUserAuth`, `ActiveUserKey`, `InactiveUserKey`, `LockUserScreen`, `UnlockUserScreen` | 加密密钥操作 |
| 卷管理 | `Mount`, `Unmount`, `Format`, `TryToFix`, `GetAllVolumes` | 外部存储卷 |
| 磁盘管理 | `Partition`, `GetAllDisks`, `GetDiskById` | 磁盘分区 |
| 空间统计 | `GetBundleStats`, `GetUserStorageStats`, `GetTotalSize`, `GetFreeSize` | 存储空间查询 |
| 配额管理 | `SetBundleQuota` | 应用配额设置 |
| 文件分享 | `CreateShareFile`, `DeleteShareFile` | 跨应用文件分享 |
| FUSE 挂载 | `MountMediaFuse`, `MountFileMgrFuse` | 媒体/文件管理 FUSE |

## Daemon 侧关键接口

定义在 `services/storage_daemon/include/ipc/storage_daemon_provider.h`：

| 接口分类 | 关键接口 | 说明 |
|----------|----------|------|
| 用户目录 | `PrepareUserDirs`, `DestroyUserDirs`, `StartUser`, `StopUser` | 用户目录创建/销毁 |
| 密钥操作 | `GenerateUserKeys`, `DeleteUserKeys`, `UpdateUserAuth`, `ActiveUserKey` | 密钥生成/安装 |
| 卷操作 | `Mount`, `Unmount`, `Check`, `Format` | 卷底层操作 |
| 磁盘操作 | `Partition`, `GetPartitionTable`, `CreatePartition` | 分区操作 |
| 配额操作 | `SetBundleQuota(uid, bundleDataDirPath, limitSizeMb)` | 配额底层实现（注意3个参数） |

## IPC 调用链

### 用户创建流程

```
应用/系统 → StorageManagerProvider::PrepareAddUser()
  → DelayedSingleton<StorageDaemonCommunication>::GetInstance()->PrepareAddUser()
    → StorageDaemonProvider::PrepareUserDirs()
      → UserManager::GetInstance().PrepareUserDirs()
      → KeyManager::GetInstance().GenerateUserKeys()
```

### 卷挂载流程

```
应用 → StorageManagerProvider::Mount()
  → DelayedSingleton<StorageDaemonCommunication>::GetInstance()->Mount()
    → StorageDaemonProvider::Mount()
      → VolumeManager::Instance().Mount()
        → VolumeInfo::Mount()
```

### 磁盘事件通知（反向）

```
内核 netlink → NetlinkHandler::OnEvent()
  → DiskManager::Instance().HandleDiskEvent()
    → DiskInfo::Create()
      → 通过 IPC 通知 Manager
        → DiskManagerService::OnDiskCreated()
```

## 权限校验

### Manager 侧权限校验

```cpp
// StorageManagerProvider 对公开 API 做权限检查（注意权限名和方法名）
const std::string PERMISSION_MOUNT_MANAGER = "ohos.permission.MOUNT_UNMOUNT_MANAGER";
int32_t StorageManagerProvider::Mount(const std::string &volumeId) {
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        LOGE("Permission denied for Mount");
        return E_PERMISSION_DENIED;
    }
    
    return DelayedSingleton<StorageDaemonCommunication>::GetInstance()->Mount(volumeId, flag);
}
```

### Daemon 侧路径校验

```cpp
// StorageDaemonProvider 做设备路径和挂载路径的合法性校验
int32_t StorageDaemonProvider::Mount(const std::string &volumeId) {
    auto volume = VolumeManager::Instance().GetVolume(volumeId);
    
    // 路径合法性校验（ValidateBlockDevicePath 要求路径以 /dev/block/ 开头）
    if (!ValidateBlockDevicePath(volume->GetDevicePath())) {
        LOGE("Invalid device path: %{public}s", volume->GetDevicePath().c_str());
        return E_PARAMS_INVALID;  // 注意：返回 E_PARAMS_INVALID，不是 E_INVALID_PATH
    }
    
    // ValidateMountPath 要求路径以 /mnt/data/ 开头
    if (!ValidateMountPath(volume->GetMountPath())) {
        LOGE("Invalid mount path: %{public}s", volume->GetMountPath().c_str());
        return E_PARAMS_INVALID;  // 注意：E_INVALID_PATH 不存在
    }
    
    return VolumeManager::Instance().Mount(volumeId, flags);
}
```

**不要绕过这些校验**。

## 线程安全

### Manager 侧锁和数据结构

```cpp
// DiskManagerService（注意：map 不是 unordered_map，shared_ptr 不是 sptr，Disk 不是 DiskInfo）
std::mutex diskMapMutex_;
std::map<std::string, std::shared_ptr<Disk>> diskMap_;

// VolumeManagerService（注意：map 不是 unordered_map，shared_ptr 不是 sptr，VolumeExternal 不是 VolumeInfo）
std::mutex volumeMapMutex_;
std::map<std::string, std::shared_ptr<VolumeExternal>> volumeMap_;
```

### Daemon 侧锁和数据结构

```cpp
// KeyManager（注意：KeyMap 是 std::map<KeyType, std::shared_ptr<BaseKey>>）
std::mutex keyMutex_;
using KeyMap = std::map<KeyType, std::shared_ptr<BaseKey>>;
std::map<unsigned int, KeyMap> userElKeys_;

// UserManager
std::mutex userManagerMutex_;
```

### Manager 侧其他单例

```cpp
// BundleMgrConnector（注意类名是 BundleMgrConnector，不是 BundleManagerConnector）
BundleMgrConnector::GetInstance()
```

### 死锁约束

```cpp
// 错误：在持有 Manager 锁时调用 Daemon IPC（可能死锁）
void DiskManagerService::SomeMethod() {
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->SomeOperation();  // 可能死锁
}

// 正确：先释放锁再调用 IPC
void DiskManagerService::SomeMethod() {
    sptr<DiskInfo> disk;
    {
        std::lock_guard<std::mutex> lock(diskMapMutex_);
        disk = diskMap_[diskId];
    }  // 锁已释放
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->SomeOperation();
}
```

**IPC 调用本身是同步的，不要在持有 Manager 锁时调用 Daemon IPC**。

## StorageRadar 上报

所有关键操作（用户管理、加密、挂载）失败时通过 `StorageRadar` 上报（注意：API 是 `RecordFunctionResult` 和静态 `Report*` 方法，不是 `ErrorReport`）：

```cpp
// 实例方法：RecordFunctionResult（用于通用故障上报）
StorageRadar::GetInstance().RecordFunctionResult(parameterRes, eventName);

// 静态方法：各类专用上报
StorageRadar::ReportVolumeOperation("Mount", ret);
StorageRadar::ReportActiveUserKey("ActiveUserKey", userId, ret, keyElxLevel);
StorageRadar::ReportUserManager("PrepareAddUser", userId, ret, extraData);
StorageRadar::ReportUpdateUserAuth("UpdateUserAuth", userId, ret, keyLevel, extraData);
StorageRadar::ReportCommonResult("SomeOperation", ret, userId, extraData);
```
```

**不要吞掉错误而不上报**。

## 代码放置指引

新增功能时，必须按照以下规则放置代码：

| 新功能类型 | 放置位置 | 修改文件 | 示例 |
|------------|----------|----------|------|
| **新增 IPC 接口** | Manager + Daemon 双侧 | 1. `interfaces/innerkits/.../istorage_manager.h` 定义接口<br>2. `storage_manager_stub.cpp` 实现 Stub<br>3. `StorageManagerProvider` 实现业务<br>4. `StorageDaemonProvider` 实现底层<br>5. 对应核心类 | `GetVolumeXxx` |
| **修改挂载逻辑** | Daemon 侧 | `VolumeManager` 或 `VolumeInfo` | Mount 参数调整 |
| **新增密钥操作** | Daemon 侧 KeyManager | `key_manager.h` 声明<br>`key_manager.cpp` 实现<br>可选：新增 BaseKey 子类 | `GenerateAppkey` |
| **新增用户操作** | Daemon + Manager | Daemon: `UserManager`<br>Manager: `StorageManagerProvider` + Stub | `PrepareXxx` |
| **新增空间统计** | Manager 侧 | `StorageStatusManager` 或 `StorageTotalStatusService` | `GetXxxStats` |
| **新增配额功能** | Daemon 侧 | `QuotaManager` | `GetXxxQuota` |
| **新增文件系统支持** | Daemon disk_manager/volume | 1. 新增 `xfs_operator.h`<br>2. 新增 `xfs_operator.cpp`<br>3. `volume_operator_factory.cpp` 注册 | `XfsOperator` |
| **新增磁盘事件处理** | Daemon 侧 | `DiskManager` + `NetlinkHandler` | 特殊磁盘类型 |
| **新增监控阈值** | Manager 侧 | `StorageMonitorService` | 新增清理策略 |
| **新增错误码** | 共享定义 | `storage_service_errno.h` | 新增错误场景 |

## IPC 接口新增完整流程

```
步骤 1：定义接口（interfaces/innerkits）
  └ istorage_manager.h        - 声明虚函数
  └ storage_manager_stub.h    - 声明 OnXxx 处理函数

步骤 2：实现 Stub（services/storage_manager）
  └ storage_manager_stub.cpp  - 实现 OnXxx，解析 MessageParcel

步骤 3：实现 Manager 业务（services/storage_manager）
  └ StorageManagerProvider    - 实现接口，调用 Daemon

步骤 4：定义 Daemon 接口（services/storage_daemon）
  └ storage_daemon.h          - 声明 Daemon 接口
  └ storage_daemon_stub.cpp   - 实现 Stub 解析

步骤 5：实现 Daemon 业务（services/storage_daemon）
  └ StorageDaemonProvider     - 调用对应核心类
  └ 对应核心类（如 VolumeManager）

步骤 6：添加测试
  └ 单元测试：对应模块的 test 目录
  └ Fuzz 测试：test/fuzztest/
```

### 示例：新增 GetVolumeStats 接口

```cpp
// 步骤 1：interfaces/innerkits/.../istorage_manager.h
virtual int32_t GetVolumeStats(const std::string &volumeId, VolumeStats &stats) = 0;

// 步骤 2：services/storage_manager/src/storage_manager_stub.cpp
int32_t StorageManagerStub::OnGetVolumeStats(MessageParcel &data, MessageParcel &reply) {
    std::string volumeId = data.ReadString();
    VolumeStats stats;
    int32_t ret = GetVolumeStats(volumeId, stats);
    reply.WriteInt32(ret);
    if (ret == E_OK) {
        stats.Marshalling(reply);
    }
    return ret;
}

// 步骤 3：services/storage_manager/src/storage_manager_provider.cpp
int32_t StorageManagerProvider::GetVolumeStats(const std::string &volumeId, VolumeStats &stats) {
    return DelayedSingleton<StorageDaemonCommunication>::GetInstance()->GetVolumeStats(volumeId, stats);
}

// 步骤 4：services/storage_daemon/src/storage_daemon_stub.cpp
int32_t StorageDaemonStub::OnGetVolumeStats(MessageParcel &data, MessageParcel &reply) {
    std::string volumeId = data.ReadString();
    VolumeStats stats;
    int32_t ret = GetVolumeStats(volumeId, stats);
    // ... 序列化回复
    return ret;
}

// 步骤 5：services/storage_daemon/src/storage_daemon_provider.cpp
int32_t StorageDaemonProvider::GetVolumeStats(const std::string &volumeId, VolumeStats &stats) {
    return VolumeManager::Instance().GetVolumeStats(volumeId, stats);
}
```

## 头文件路径

```
interfaces/innerkits/storage_service/istorage_manager.h
services/storage_manager/include/ipc/storage_manager_provider.h
services/storage_manager/include/ipc/storage_manager_stub.h
services/storage_daemon/include/ipc/storage_daemon_provider.h
services/storage_daemon/include/ipc/storage_daemon_stub.h
services/storage_manager/include/storage_daemon_communication/storage_daemon_communication.h
services/storage_manager/include/storage/bundle_manager_connector.h  # 类名是 BundleMgrConnector
```

## 测试指引

- IPC 层：`StorageManagerProxyTest`、`StorageDaemonProxyTest`、`StorageManagerStubTest`、`StorageDaemonStubTest`
- Daemon Provider：`StorageDaemonProvider*` 系列测试
- Manager Provider：`StorageManagerProvider*` 系列测试
- 通信层：`StorageDaemonCommunicationTest`
- 崩溃恢复：需要板侧验证（kill storage_daemon 进程）