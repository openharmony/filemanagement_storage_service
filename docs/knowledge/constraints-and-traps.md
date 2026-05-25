# 约束与陷阱

本文记录项目约束和常见陷阱。

## 时序约束

### 用户目录创建时序

必须按 `PrepareAddUser → StartUser → CompleteAddUser` 顺序执行，不可跳步或乱序：

```
PrepareAddUser(userId)   // 1. 创建目录，生成密钥（EL1）
StartUser(userId)        // 2. 挂载存储，激活 EL1 密钥
CompleteAddUser(userId)  // 3. 激活 EL2-EL4 密钥
```

### 卷状态转换时序

必须经过合法路径，不可在 `MOUNTED` 状态直接删除：

```
正确：MOUNTED → EJECTING → Unmount() → REMOVED
错误：MOUNTED → REMOVED（跳过 EJECTING）
```

### 锁屏/解锁时序

锁屏移除 EL2 内核密钥，解锁重新安装，不可在锁屏状态访问 EL2 加密文件：

```
LockUserScreen(userId)   // 移除 EL2 密钥
// 此时不可访问 EL2 加密文件
UnlockUserScreen(userId) // 重新安装 EL2 密钥
// 此时可以访问 EL2 加密文件
```

## 数据隔离约束

### EL 等级隔离

EL1-EL5 加密等级对应独立的密钥和目录层级，不可混用：

```cpp
// 错误：使用 EL1 密钥操作 EL2 目录
KeyManager::GetInstance().ActiveUserKey(userId, EL1_KEY);
AccessDirectory("/data/service/el2/" + std::to_string(userId));  // 密钥不匹配

// 正确：使用对应等级的密钥
KeyManager::GetInstance().ActiveUserKey(userId, EL2_KEY);
AccessDirectory("/data/service/el2/" + std::to_string(userId));
```

### 用户隔离

不可用一个用户的密钥或挂载点操作另一个用户的数据路径：

```cpp
// 错误：跨用户操作
int32_t userId = GetCurrentUserId();  // userId = 100
int32_t targetUserId = 105;
KeyManager::GetInstance().ActiveUserKey(userId);  // 激活 100 的密钥
AccessUserDirectory(targetUserId);  // 访问 105 的目录 —— 密钥不匹配

// 正确：使用目标用户的密钥
KeyManager::GetInstance().ActiveUserKey(targetUserId);
AccessUserDirectory(targetUserId);
```

### 路径校验

所有设备路径和挂载路径必须经过校验：

```cpp
if (!ValidateBlockDevicePath(devicePath)) {
    LOGE("Invalid device path");
    return E_PARAMS_INVALID;
}

if (!ValidateMountPath(mountPath)) {
    LOGE("Invalid mount path");
    return E_PARAMS_INVALID;
}
```

## 进程约束

### Daemon 崩溃恢复

Manager 会锁屏并重建连接，不可假设前一次 IPC 状态仍有效：

```cpp
// 错误：假设之前的调用仍然有效
storageDaemonProxy_->Mount(volumeId);  // Daemon 已崩溃重建
volumeMap_[volumeId]->SetState(MOUNTED);  // Daemon 内存状态已清空！

// 正确：重新同步状态
if (!storageDaemonProxy_) {
    Connect();
    SyncVolumeState();
}
```

### 懒连接

Manager 按需连接 Daemon，不可假设 Manager 启动时 Daemon 已经就绪：

```cpp
// 连接可能延迟到首次使用时才建立
if (!storageDaemonProxy_) {
    Connect();  // 按需连接
}
```

### 空间查询

可能涉及全盘扫描，不可在高频路径或持有锁时执行：

```cpp
// 错误：持有锁时执行空间查询
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    GetBundleStats(pkgName);  // 可能阻塞很久
}

// 正确：释放锁后再查询
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    pkgName = volumeMap_[id]->GetPkgName();
}
GetBundleStats(pkgName);  // 锁已释放
```

## 线程安全与性能

### 锁使用

```cpp
// Manager 侧锁
std::mutex diskMapMutex_;
std::mutex volumeMapMutex_;

// Daemon 侧锁
std::mutex keyMutex_;

// 死锁预防：不要在持有锁时调用 IPC
// 错误：
std::lock_guard<std::mutex> lock(volumeMapMutex_);
storageDaemonCommunication_->Mount(volumeId);  // 可能死锁

// 正确：
VolumeInfo volume;
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    volume = volumeMap_[volumeId];
}
storageDaemonCommunication_->Mount(volume);
```

### 阻塞操作

不要在以下场景执行阻塞操作：
1. Netlink 回调中（mount、fsck、格式化）
2. 监控回调中（统计、清理）
3. 持有锁时（IPC 调用、文件 I/O）

```cpp
// 正确做法：投递到工作线程
void NetlinkHandler::OnEvent(const NetlinkData &data) {
    workerThread_.PostTask([data]() {
        DiskManager::HandleDiskEvent(data);
    });
}
```

### 内存管理

```cpp
// 使用智能指针
std::shared_ptr<VolumeInfo> volume = std::make_shared<VolumeInfo>();

// 长期持有的映射表
std::map<std::string, std::shared_ptr<DiskInfo>> diskMap_;
```

## 安全规范

### 敏感数据处理

```cpp
// 密钥数据：使用后立即清零
std::vector<uint8_t> key = GenerateRandomKey();
OPENSSL_cleanse(key.data(), key.size());

// 禁止打印敏感信息
// 错误：LOGI("User password: %{public}s", password);
// 正确：LOGI("Token received, length: %{public}zu", token.size());
```

### 权限校验

```cpp
// Manager 入口必须校验权限
int32_t StorageManagerProvider::Mount(const std::string &volumeId) {
    if (!CheckPermission(callerUid, "ohos.permission.MOUNT")) {
        LOGE("Permission denied for uid %{public}d", callerUid);
        return E_PERMISSION_DENIED;
    }
}
```

## 常见陷阱

### 陷阱 1：用户目录创建跳步

```cpp
// 错误：跳过 StartUser
PrepareAddUser(userId);
CompleteAddUser(userId);  // EL2-EL4 密钥未激活

// 正确：完整时序
PrepareAddUser(userId);
StartUser(userId);
CompleteAddUser(userId);
```

### 陷阱 2：卷状态非法跳转

```cpp
// 错误：从 MOUNTED 直接删除
volume->SetState(VolumeState::REMOVED);

// 正确：经过 EJECTING
volume->SetState(VolumeState::EJECTING);
Unmount(volume);
volume->SetState(VolumeState::REMOVED);
```

### 陷阱 3：Daemon 崩溃后 IPC 状态假设

```cpp
// 错误
storageDaemonProxy_->Mount(volumeId);
volumeMap_[volumeId]->SetState(MOUNTED);

// 正确
if (!storageDaemonProxy_) {
    Connect();
    SyncVolumeState();
}
```

### 陷阱 4：锁屏时访问 EL2 数据

```cpp
// 错误
void OnLockScreen(uint32_t userId) {
    LockUserScreen(userId);
    ReadFile("/data/service/el2/" + std::to_string(userId) + "/config");
}

// 正确
void OnUnlockScreen(uint32_t userId) {
    UnlockUserScreen(userId);
    ReadFile("/data/service/el2/" + std::to_string(userId) + "/config");
}
```

### 陷阱 5：跨用户密钥操作

```cpp
// 错误
int32_t userId = GetCurrentUserId();
int32_t targetUserId = 105;
KeyManager::GetInstance().ActiveUserKey(userId);
AccessUserDirectory(targetUserId);

// 正确
KeyManager::GetInstance().ActiveUserKey(targetUserId);
AccessUserDirectory(targetUserId);
```

### 陷阱 6：持有锁时执行 IPC

```cpp
// 错误
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    storageDaemonProxy_->Mount(volumeId);
}

// 正确
std::string volumeId;
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    volumeId = volumeMap_[id]->GetId();
}
storageDaemonProxy_->Mount(volumeId);
```