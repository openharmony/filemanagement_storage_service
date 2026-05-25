# Design: DLP FUSE 文件系统挂载/卸载 Inner 接口

## 需求基线摘要

见 proposal.md。20 个 AC 覆盖功能正确（4）、性能（2）、异常场景（8+）、权限校验（4）、真机测试（4）。

## 方案概述

复用现有 storage_manager → storage_daemon IPC 分层架构：

- **IStorageManager.idl** 新增 `MountDlpFuse` / `UMountDlpFuse` 两个 IPC 方法
- **StorageManagerProvider** 实现权限+UID校验，委托 `StorageDaemonClient` 调用 daemon 端
- **StorageDaemonProvider / MountManager** 实现底层 `/dev/fuse` 打开、mount 系统调用、umount 操作
- **storage_manager_sa_proxy** 自动生成代理类，DLP 服务通过代理调用
- **GN feature flag** `storage_service_dlp_fuse` 控制 PC 产品编译隔离

## 模块影响

| 子系统 | 仓库 | 模块 | 影响类型 |
|--------|------|------|---------|
| filemanagement | storage_service | IStorageManager.idl | 新增 2 个 IPC 方法 |
| filemanagement | storage_service | StorageManagerProvider | 新增 2 个方法实现 + UID校验方法 |
| filemanagement | storage_service | MountManager | 新增 2 个 FUSE 挂载/卸载方法 |
| filemanagement | storage_service | storage_manager_sa_proxy | IDL 自动生成代理更新 |
| filemanagement | storage_service | storage_service_aafwk.gni | 新增 feature flag |
| filemanagement | storage_service | storage_service_errno.h | 新增 2 个错误码 (E_MOUNT_DLP_FUSE=759, E_UMOUNT_DLP_FUSE=760) |
| filemanagement | storage_service | BUILD.gn (多个) | 新增源文件 + feature 条件编译 |

## 数据结构

### IDL 方法签名

```idl
// IStorageManager.idl 新增
// MountDlpFuse: 打开一个 /dev/fuse 节点，将同一 FUSE 文件系统挂载到 dstPath1（主挂载）和 dstPath2（bind mount），返回一个 fuse fd
int MountDlpFuse([in] String dstPath1, [in] String dstPath2, [out] FileDescriptor fd);
// UMountDlpFuse: 先卸载 dstPath2（bind mount），再卸载 dstPath1（FUSE 主挂载）
int UMountDlpFuse([in] String dstPath1, [in] String dstPath2);
```

> 语义说明：dstPath1 和 dstPath2 是两个目标目录（挂载点），共享同一个 FUSE 源（同一个 /dev/fuse fd）。
> 实现方式：先 open("/dev/fuse") 获取一个 fd，将 FUSE 挂载到 dstPath1，再通过 bind mount 将 dstPath1 绑定到 dstPath2。
> DLP 服务只需在一个 fd 上运行一个 FUSE daemon，两个目录均可见相同文件系统内容。

### 错误码新增

```c
// storage_service_errno.h 新增（紧接 E_MOUNT_AGAIN_FAILED = SYS_CAP_TAG + 758）
constexpr int32_t E_MOUNT_DLP_FUSE = STORAGE_SERVICE_SYS_CAP_TAG + 759;   // DLP FUSE 挂载失败
constexpr int32_t E_UMOUNT_DLP_FUSE = STORAGE_SERVICE_SYS_CAP_TAG + 760;  // DLP FUSE 卸载失败
```

> `E_PERMISSION_DENIED`（SYS_CAP_TAG+201）、`E_PARAMS_INVALID`（SYS_CAP_TAG+2）、`E_OPEN_FUSE`（SYS_CAP_TAG+745）
> 已有定义，本次不新增。权限未授权和 UID 白名单校验失败均统一返回 `E_PERMISSION_DENIED`，
> 路径相关校验失败统一返回 `E_PARAMS_INVALID`。

### 常量定义

```c
// storage_manager_provider.cpp 或 storage_service_constants.h
static constexpr int32_t DLP_SERVICE_UID = 3553;
static const std::string DLP_FUSE_MOUNTS_PREFIX = "/data/service/el1/public/dlp_credential_service/mounts/";
```

## 交互流程

### MountDlpFuse 调用流程

```
DLP Service (UID=3553)
    │
    ├─ StorageManagerSaProxy::MountDlpFuse(dstPath1, dstPath2)
    │   │ [IPC 客户端] 序列化参数，发送请求 (ipccode=N)
    │
    ▼
StorageManagerProvider::MountDlpFuse(dstPath1, dstPath2, fd)
    │
    ├─ 1. CheckClientPermission(PERMISSION_STORAGE_MANAGER)
    │      → 失败则返回 E_PERMISSION_DENIED
    │
    ├─ 2. CheckCallerIsDlpService()
    │      → IPCSkeleton::GetCallingUid() == DLP_SERVICE_UID (3553)
    │      → 失败则返回 E_PERMISSION_DENIED
    │
    ├─ 3. ValidateDlpFusePath(dstPath1) && ValidateDlpFusePath(dstPath2)
    │      → 路径前缀校验 + realpath 逃逸校验 + 空路径校验
    │      → 失败则返回 E_PARAMS_INVALID
    │
    ├─ 4. StorageDaemonClient::MountDlpFuse(dstPath1, dstPath2, fd)
    │      [IPC 跨进程调用到 storage_daemon]
    │
    ▼
StorageDaemonProvider::MountDlpFuse(dstPath1, dstPath2, fd)
    │
    ├─ 5. MountManager::MountDlpFuseDevice(dstPath1, dstPath2, devFd)
    │      │
    │      ├─ 5a. open("/dev/fuse", O_RDWR) → devFd
    │      │        失败 → 返回 E_OPEN_FUSE
    │      │
    │      ├─ 5b. 构造 mount options: "fd=devFd,rootmode=40000,default_permissions,allow_other,..."
    │      │        Mount("/dev/fuse", dstPath1, "fuse", MS_NOSUID|MS_NODEV|MS_NOEXEC|MS_NOATIME, opts)
    │      │        失败 → close(devFd)，返回 E_MOUNT_DLP_FUSE
    │      │
    │      ├─ 5c. bind mount: Mount(dstPath1, dstPath2, "", MS_BIND, "")
    │      │        将 dstPath1 绑定挂载到 dstPath2，共享同一 FUSE 文件系统
    │      │        失败 → UMount2(dstPath1, MNT_DETACH) 回滚主挂载，close(devFd)，返回 E_MOUNT_DLP_FUSE
    │      │
    │      └─ 5d. fd = devFd (返回给 DLP 服务)
    │
    └─ 6. 返回 E_OK + fd
    │
    ▼
DLP Service 拿到一个 fd，启动 FUSE daemon
  → dstPath1 和 dstPath2 均可见 FUSE 文件系统内容
```

### UMountDlpFuse 调用流程

```
DLP Service (UID=3553)
    │
    ├─ StorageManagerSaProxy::UMountDlpFuse(dstPath1, dstPath2)
    │
    ▼
StorageManagerProvider::UMountDlpFuse(dstPath1, dstPath2)
    │
    ├─ 1. CheckClientPermission(PERMISSION_STORAGE_MANAGER) → 失败返回 E_PERMISSION_DENIED
    ├─ 2. CheckCallerIsDlpService() → 失败返回 E_PERMISSION_DENIED
    ├─ 3. ValidateDlpFusePath(dstPath1) && ValidateDlpFusePath(dstPath2) → 失败返回 E_PARAMS_INVALID
    │
    ├─ 4. StorageDaemonClient::UMountDlpFuse(dstPath1, dstPath2)
    │
    ▼
StorageDaemonProvider::UMountDlpFuse(dstPath1, dstPath2)
    │
    ├─ 5. 先卸载 bind mount: UMount2(dstPath2, MNT_DETACH)
    │      失败 → 返回 E_UMOUNT_DLP_FUSE
    │
    ├─ 6. 再卸载 FUSE 主挂载: UMount2(dstPath1, MNT_DETACH)
    │      失败 → 返回 E_UMOUNT_DLP_FUSE
    │      （此时 FUSE daemon 的 fd 已无内核端消费者，DLP 服务应自行停止 daemon）
    │
    └─ 7. 返回 E_OK
```

> 卸载顺序：必须先卸载 dstPath2（bind mount），再卸载 dstPath1（FUSE 主挂载）。
> 若顺序相反，dstPath2 的 bind mount 仍引用 dstPath1，可能导致卸载不干净或 EBUSY。

## 关键设计决策

| ID | 问题 | 选择 | 备选 | 理由 |
|----|------|------|------|------|
| D-001 | 两个目录如何共享同一 FUSE | dstPath1 做 FUSE 主挂载，dstPath2 做 bind mount | 分别 open 两次 /dev/fuse 各自挂载 | 需求明确要求一个 fuse fd，DLP 服务只需运行一个 FUSE daemon；bind mount 是 Linux 标准机制，让两个目录共享同一文件系统内容 |
| D-002 | 权限/UID 校验错误码 | 权限未授权和 UID≠3553 统一返回 E_PERMISSION_DENIED | 分别返回不同错误码 | 对调用方而言，两者均属"无权访问"，无需区分具体原因；统一错误码简化调用方处理逻辑 |
| D-003 | SELinux 上下文 | 新增 DLP 专用 SELinux label `u:object_r:dlp_fuse:s0` | 复用 hmdfs:s0 | DLP 凭证数据与媒体库/文件管理器安全域不同，需独立 SELinux 上下文 |
| D-004 | FUSE mount options | `default_permissions,allow_other` + DLP SELinux context | `allow_other` only | 复用现有 FileMgrFuse 的 mount options 模式，仅替换 SELinux 上下文 |
| D-005 | Feature 编译隔离 | 新增 `storage_service_dlp_fuse` flag，默认跟随 `pc_device_enable` | 直接用 `PC_USER_MANAGER` | DLP FUSE 是独立功能，应有独立 flag 便于单独开关；但默认值跟随 PC 设备 |
| D-006 | 路径校验实现 | 复用 `IsValidPath()` (realpath) + 新增 `IsDlpFuseMountsPath()` 前缀校验 | 自行实现全套路径校验 | `IsValidPath` 已经过充分测试验证，复用降低风险；前缀校验逻辑简单独立新增 |
| D-007 | IPC code 分配 | 延续现有编号序列（当前最大 ~113） | 使用新编号段 | 遵循现有编号规则，由 IDL 工具自动分配或手动指定延续序列 |
| D-008 | bind mount 失败回滚策略 | 先回滚 FUSE 主挂载 + 关闭 fd | 仅返回错误不回滚 | 必须保持一致性：bind mount 失败时不应留下部分挂载状态，回滚确保系统干净 |

## 文件变更清单

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `services/storage_manager/IStorageManager.idl` | 修改 | 新增 MountDlpFuse / UMountDlpFuse 方法 |
| `services/storage_manager/ipc/src/storage_manager_provider.cpp` | 修改 | 新增 MountDlpFuse / UMountDlpFuse 实现、CheckCallerIsDlpService、ValidateDlpFusePath |
| `services/storage_manager/ipc/include/storage_manager_provider.h` | 修改 | 新增方法声明、常量定义 |
| `services/storage_daemon/IStorageDaemon.idl` | 修改 | 新增 MountDlpFuse / UMountDlpFuse daemon 端方法 |
| `services/storage_daemon/user/src/mount_manager.cpp` | 修改 | 新增 MountDlpFuseDevice / UMountDlpFuseDevice 实现 |
| `services/storage_daemon/user/include/mount_manager.h` | 修改 | 新增方法声明 |
| `services/storage_daemon/ipc/src/storage_daemon_provider.cpp` | 修改 | 新增 MountDlpFuse / UMountDlpFuse IPC 处理 |
| `services/storage_daemon/ipc/include/storage_daemon_provider.h` | 修改 | 新增方法声明 |
| `interfaces/kits/native/storage_service_errno.h` | 修改 | 新增错误码 |
| `storage_service_aafwk.gni` | 修改 | 新增 `storage_service_dlp_fuse` feature flag |
| `services/storage_manager/BUILD.gn` | 修改 | `#ifdef DLP_FUSE_SERVICE` 条件编译 |
| `services/storage_daemon/BUILD.gn` | 修改 | `#ifdef DLP_FUSE_SERVICE` 条件编译 |
| `interfaces/innerkits/storage_manager/native/BUILD.gn` | 修改 | IDL 生成包含新方法 |

## 详细实现设计

### CheckCallerIsDlpService

```cpp
bool StorageManagerProvider::CheckCallerIsDlpService()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != DLP_SERVICE_UID) {
        LOGE("Caller UID %{public}d is not DLP service UID %{public}d", callingUid, DLP_SERVICE_UID);
        return false;
    }
    return true;
}
```

> 参考现有 `CheckClientPermissionForShareFile` 的进程名+UID双重校验模式（storage_manager_provider.cpp:129-147），
> 但 DLP 场景仅需 UID 校验，因 DLP 服务进程名可能变化，UID 是更稳定的身份标识。

### ValidateDlpFusePath

```cpp
bool StorageManagerProvider::ValidateDlpFusePath(const std::string &path)
{
    if (path.empty()) {
        return false;
    }
    // 前缀校验
    if (path.find(DLP_FUSE_MOUNTS_PREFIX) != 0) {
        LOGE("Path %{public}s prefix not match DLP mounts prefix", path.c_str());
        return false;
    }
    // 路径逃逸校验 - 复用 storage_daemon 的 IsValidPath
    // 通过 IPC 在 daemon 端执行 realpath 校验，或在 manager 端做基础校验后
    // daemon 端再次校验（双端校验确保 IPC 边界安全）
    if (path.find("..") != std::string::npos) {
        return false;
    }
    return true;
}
```

> daemon 端的 MountDlpFuseDevice 内部也执行 `IsValidPath()` realpath 校验作为二次防线，
> 防止 manager→daemon IPC 边界的参数篡改。

### MountDlpFuseDevice (daemon 端)

```cpp
int32_t MountManager::MountDlpFuseDevice(const std::string &dstPath1,
                                           const std::string &dstPath2,
                                           int32_t &devFd)
{
    // 1. 路径二次校验（IPC 边界安全）
    if (!IsValidPath(dstPath1) || !IsValidPath(dstPath2)) {
        LOGE("DLP FUSE path realpath check failed");
        return E_PARAMS_INVALID;
    }
    if (dstPath1.find(DLP_FUSE_MOUNTS_PREFIX) != 0 ||
        dstPath2.find(DLP_FUSE_MOUNTS_PREFIX) != 0) {
        return E_PARAMS_INVALID;
    }

    // 2. 打开 /dev/fuse（只打开一次，两个目录共享同一 FUSE 源）
    devFd = open("/dev/fuse", O_RDWR);
    if (devFd < 0) {
        LOGE("Open /dev/fuse failed, errno %{public}d", errno);
        return E_OPEN_FUSE;
    }

    // 3. 构造 mount options 并挂载到 dstPath1（FUSE 主挂载）
    std::string opts = StringFormat(
        "fd=%i,rootmode=40000,default_permissions,allow_other,user_id=0,group_id=0,"
        "context=\"u:object_r:dlp_fuse:s0\",fscontext=u:object_r:dlp_fuse:s0",
        devFd);

    int32_t ret = Mount("/dev/fuse", dstPath1, "fuse",
                        MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opts);
    if (ret != 0) {
        LOGE("Mount DLP FUSE to dstPath1 failed, errno %{public}d", errno);
        close(devFd);
        devFd = -1;
        return E_MOUNT_DLP_FUSE;
    }

    // 4. bind mount: 将 dstPath1 绑定到 dstPath2（共享同一 FUSE 文件系统）
    ret = Mount(dstPath1, dstPath2, "", MS_BIND, "");
    if (ret != 0) {
        LOGE("Bind mount dstPath1 to dstPath2 failed, errno %{public}d", errno);
        // 回滚：卸载 FUSE 主挂载 + 关闭 fd
        UMount2(dstPath1, MNT_DETACH);
        close(devFd);
        devFd = -1;
        return E_MOUNT_DLP_FUSE;
    }

    LOGI("Mount DLP FUSE success: dstPath1=%{public}s, dstPath2=%{public}s (bind), fd=%{public}d",
         dstPath1.c_str(), dstPath2.c_str(), devFd);
    return E_OK;
}
```

> 复用 `MountFileMgrFuse` 的实现模式（mount_manager.cpp:1139-1188），
> 仅替换 SELinux 上下文为 `dlp_fuse:s0`。
> bind mount 使用 `MS_BIND` 标志，无需额外的 mount options，这是 Linux 标准机制。

### UMountDlpFuseDevice (daemon 端)

```cpp
int32_t MountManager::UMountDlpFuseDevice(const std::string &dstPath1,
                                            const std::string &dstPath2)
{
    // 1. 先卸载 bind mount (dstPath2)
    int32_t ret = UMount2(dstPath2, MNT_DETACH);
    if (ret != 0) {
        LOGE("UMount DLP FUSE bind mount at %{public}s failed, errno %{public}d",
             dstPath2.c_str(), errno);
        return E_UMOUNT_DLP_FUSE;
    }

    // 2. 再卸载 FUSE 主挂载 (dstPath1)
    ret = UMount2(dstPath1, MNT_DETACH);
    if (ret != 0) {
        LOGE("UMount DLP FUSE main mount at %{public}s failed, errno %{public}d",
             dstPath1.c_str(), errno);
        return E_UMOUNT_DLP_FUSE;
    }

    LOGI("UMount DLP FUSE success: dstPath2=%{public}s (bind), dstPath1=%{public}s (main)");
    return E_OK;
}
```

> 卸载顺序：先 bind mount (dstPath2) → 后 FUSE 主挂载 (dstPath1)，
> 防止 bind mount 引用已卸载的源导致 EBUSY。

### StorageManagerProvider::MountDlpFuse 组合校验

```cpp
int32_t StorageManagerProvider::MountDlpFuse(const std::string &dstPath1,
                                               const std::string &dstPath2,
                                               int32_t &fd)
{
    // 权限校验
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    // UID 白名单校验
    if (!CheckCallerIsDlpService()) {
        return E_PERMISSION_DENIED;
    }
    // 路径校验
    if (!ValidateDlpFusePath(dstPath1) || !ValidateDlpFusePath(dstPath2)) {
        return E_PARAMS_INVALID;
    }
    // 委托 daemon（一个 fd，两个目录共享同一 FUSE 源）
    auto daemonClient = StorageDaemonClient::GetInstance();
    return daemonClient->MountDlpFuse(dstPath1, dstPath2, fd);
}
```

> 参考现有 `MountFileMgrFuse` 的组合校验模式（storage_manager_provider.cpp:1666）：
> `CheckClientPermission(PERMISSION_STORAGE_MANAGER) || !IsCalledByFileMgr()`，
> DLP 场景改为权限+UID的双重 AND 校验。

### Feature Flag 配置

```gni
# storage_service_aafwk.gni 新增
storage_service_dlp_fuse = pc_device_enable

# 当 pc_device_enable 为 true 时自动启用 DLP FUSE 功能
# 也可通过产品配置单独覆盖为 true/false
```

```gni
# BUILD.gn 中条件编译
if (storage_service_dlp_fuse) {
  defines += [ "DLP_FUSE_SERVICE" ]
}
```

```cpp
// storage_manager_provider.cpp / mount_manager.cpp 中
#ifdef DLP_FUSE_SERVICE
int32_t StorageManagerProvider::MountDlpFuse(...) { ... }
int32_t StorageManagerProvider::UMountDlpFuse(...) { ... }
#endif
```

## 风险与兼容性

### 风险

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| SELinux `dlp_fuse:s0` 上下文未预配置 | 中 | 挂载失败 | 需与 SELinux 策略团队协调，确保 PC 产品 SELinux policy 包含 dlp_fuse 类型 |
| bind mount 回滚时 umount 失败 | 低 | 系统残留部分挂载 | 使用 `MNT_DETACH` 标志确保安全卸载；日志记录残留状态便于人工排查 |
| `/dev/fuse` 节点权限不足 | 低 | open 失败 | storage_daemon 以 root 权限运行，现有 FUSE 挂载已验证此路径可行 |
| UID 3553 硬编码可能变化 | 低 | 白名单失效 | 将 DLP_SERVICE_UID 提取为常量，便于后续更新；同时记录在文档中供配置管理 |

### 兼容性

- 纯增量 inner 接口，不影响现有 MountFileMgrFuse / MountMediaFuse / MountCloudFuse 行为
- IPC code 编号新增，不占用已有编号
- 非 PC 产品编译时全部新增代码被 `#ifdef DLP_FUSE_SERVICE` 排除，零影响
- IDL 生成代码变化仅影响 storage_manager_sa_proxy 的方法集，向下兼容

## 开发者测试

### 单元测试

| 测试项 | 测试内容 | 覆盖 AC |
|--------|---------|---------|
| CheckCallerIsDlpService_test | UID=3553 返回 true，UID≠3553 返回 false（映射为 E_PERMISSION_DENIED） | AC-013~AC-016 |
| ValidateDlpFusePath_test | 合法前缀通过；非法前缀/空路径/含../symlink逃逸/超长路径/dstPath1==dstPath2 均拒绝（映射为 E_PARAMS_INVALID） | AC-012a~f, AC-011 |
| MountDlpFuseDevice_test | mock open/mount/bind mount 成功返回 fd；bind mount 失败时回滚验证（umount dstPath1 + close fd） | AC-001, AC-007 |
| MountDlpFuseDevice_open_fail | mock open("/dev/fuse") 返回 -1 → 返回 E_OPEN_FUSE | AC-008 |
| MountDlpFuseDevice_repeat_mount | mock mount 重复挂载 → 返回 E_MOUNT_DLP_FUSE | AC-009 |
| UMountDlpFuseDevice_test | mock umount2 成功/失败；验证卸载顺序（先 dstPath2 后 dstPath1） | AC-003, AC-004 |
| UMountDlpFuseDevice_not_mounted | mock umount2 返回 EBUSY/ENOENT → 返回 E_UMOUNT_DLP_FUSE | AC-010 |

### 集成测试

| 测试项 | 测试内容 | 覆盖 AC |
|--------|---------|---------|
| MountDlpFuse_integrate_test | UID=3553 + 权限已授权 + 合法路径 → 成功挂载，返回一个 fd，dstPath1/dstPath2 内容一致 | AC-001, AC-002 |
| MountDlpFuse_permission_combo | 四种权限+UID组合的正负向验证 | AC-013~AC-016 |
| UMountDlpFuse_integrate_test | 挂载后卸载成功，`mount | grep dstPath` 无残留 | AC-003 |
| MountDlpFuse_repeat_test | 挂载后再次挂载同一目录 → 返回 E_MOUNT_DLP_FUSE | AC-009 |
| UMountDlpFuse_not_mounted_test | 对未挂载目录调用 → 返回 E_UMOUNT_DLP_FUSE | AC-010 |
| MountDlpFuse_bind_fail_rollback | bind mount mock 失败 → 验证 dstPath1 无 FUSE 挂载残留 + fd 已关闭 | AC-007 |

### 性能测试

| 测试项 | 测试内容 | 覆盖 AC |
|--------|---------|---------|
| MountDlpFuse_perf | 真机测量 MountDlpFuse 端到端耗时（10次取均值）≤ 500ms | AC-005 |
| UMountDlpFuse_perf | 真机测量 UMountDlpFuse 端到端耗时（10次取均值）≤ 200ms | AC-006 |

### 真机测试

| 测试项 | 测试内容 | 覆盖 AC |
|--------|---------|---------|
| DlpFuse_boot_test | PC 真机开机阶段完整流程：挂载→获取fd→读写→卸载 | AC-017 |
| DlpFuse_selinux_test | PC 真机验证 `dlp_fuse:s0` SELinux 上下文，`dmesg | grep avc` 无拒绝 | AC-018 |
| DlpFuse_build_isolation | 非 PC 产品编译产物不含 DLP_FUSE_SERVICE | AC-019 |
| DlpFuse_demo_test | Demo 工具 PC 真机端到端：挂载两个目录→读写一致性→卸载→恢复验证 | AC-020 |

## 代码映射

| AC | 预期实现模块 | 实际实现文件 | 关键行 |
|----|-------------|-------------|--------|
| AC-001 | MountManager::MountDlpFuseDevice (FUSE 主挂载 + bind mount) | mount_manager.cpp | 新增方法 |
| AC-002 | FUSE 双目录内容一致性 | 验证层（测试代码） | 测试断言 |
| AC-003 | MountManager::UMountDlpFuseDevice | mount_manager.cpp | 新增方法 |
| AC-004 | UMount 卸载顺序（先 bind 后主） | mount_manager.cpp | UMount2 调用顺序 |
| AC-005~AC-006 | 性能测量 | 测试代码 | 计时统计 |
| AC-007 | bind mount 回滚逻辑 | mount_manager.cpp | Mount 失败分支 |
| AC-008 | open("/dev/fuse") 错误处理 | mount_manager.cpp | open 失败分支 |
| AC-009 | 重复挂载检测 | mount_manager.cpp / storage_manager_provider.cpp | 挂载前检查 |
| AC-010 | 未挂载目录卸载 | mount_manager.cpp | umount2 失败处理 |
| AC-011 | ValidateDlpFusePath (dstPath1==dstPath2) | storage_manager_provider.cpp | 新增校验 |
| AC-012 | ValidateDlpFusePath 全场景 | storage_manager_provider.cpp | 新增校验 |
| AC-013~AC-016 | CheckClientPermission + CheckCallerIsDlpService | storage_manager_provider.cpp | 组合校验 |
| AC-017 | 开机阶段真机流程 | 真机测试 | 端到端 |
| AC-018 | SELinux dlp_fuse:s0 | SELinux policy + 真机验证 | dmesg 检查 |
| AC-019 | storage_service_aafwk.gni + BUILD.gn feature flag | 多个 | 新增 flag + ifdef |
| AC-020 | Demo 工具 | test/demo/ | 新增目录 |