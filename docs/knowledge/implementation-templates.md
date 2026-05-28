# 实现模板

本文记录各场景的标准实现模板，新增代码应遵循此模板以保证风格一致。

## Daemon 侧卷操作模板

```cpp
int32_t VolumeManager::Mount(const std::string &volumeId) {
    // 1. 参数校验（必须有）
    if (volumeId.empty()) {
        LOGE("VolumeId is empty");
        return E_PARAMS_INVALID;
    }
    
    // 2. 获取对象并校验状态（必须有）
    std::shared_ptr<VolumeInfo> volume = GetVolume(volumeId);
    if (volume == nullptr) {
        LOGE("Volume %{public}s not found", volumeId.c_str());
        return E_NON_EXIST;
    }
    
    VolumeState state = volume->GetState();
    if (state != UNMOUNTED) {
        LOGW("Volume %{public}s state %{public}d, cannot mount", volumeId.c_str(), state);
        return E_VOL_STATE;
    }
    
    // 3. 状态预更新（必须有）
    volume->SetState(CHECKING);
    
    // 4. 执行操作
    int32_t ret = volume->Check();
    if (ret != E_OK) {
        volume->SetState(DAMAGED);
        LOGE("Check volume %{public}s failed, ret %{public}d", volumeId.c_str(), ret);
        StorageRadar::ReportVolumeOperation("VolumeCheck", ret);
        return ret;
    }
    
    ret = volume->Mount(0);
    if (ret != E_OK) {
        volume->SetState(UNMOUNTED);
        LOGE("Mount volume %{public}s failed, ret %{public}d", volumeId.c_str(), ret);
        StorageRadar::ReportVolumeOperation("VolumeMount", ret);
        return ret;
    }
    
    // 5. 状态更新并返回（必须有）
    volume->SetState(MOUNTED);
    LOGI("Mount volume %{public}s successfully", volumeId.c_str());
    return E_OK;
}
```

## Daemon 侧密钥操作模板

```cpp
int32_t KeyManager::GenerateUserKeys(uint32_t userId, uint32_t flags) {
    // 1. 参数范围校验（必须有）
    if (userId < START_USER_ID || userId > MAX_USER_ID) {
        LOGE("Invalid userId %{public}d, range [100, 10738]", userId);
        return E_USERID_RANGE;
    }
    
    // 2. 外部依赖可用性检查（密钥操作必须有）
    if (!HuksMaster::GetInstance().IsHuksAvailable()) {
        LOGE("HUKS not available, cannot generate keys for user %{public}d", userId);
        return E_GLOBAL_KEY_INIT_ERROR;
    }
    
    // 3. 检查是否已存在（避免重复操作）
    if (HasElkey(userId, EL1_KEY)) {
        LOGW("User %{public}d already has EL1 key", userId);
        return E_OK;
    }
    
    // 4. 按顺序生成各等级密钥
    std::lock_guard<std::mutex> lock(keyMutex_);
    int32_t ret = E_OK;
    if (flags & CRYPTO_FLAG_EL1) {
        ret = GenerateAndInstallUserKey(userId, EL1_KEY);
        if (ret != E_OK) {
            LOGE("Generate EL1 key failed for user %{public}d", userId);
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    // EL2-EL5 类似...
    
    // 5. 成功日志（必须有）
    LOGI("Generate keys for user %{public}d success, flags %{public}u", userId, flags);
    return E_OK;
}
```

## Daemon 侧用户目录操作模板

```cpp
int32_t UserManager::PrepareUserDirs(uint32_t userId) {
    // 1. 参数校验
    if (userId < START_USER_ID || userId > MAX_USER_ID) {
        LOGE("Invalid userId %{public}d", userId);
        return E_USERID_RANGE;
    }
    
    // 2. 检查目录是否已存在
    std::string userDir = "/data/user/" + std::to_string(userId);
    if (access(userDir.c_str(), F_OK) == 0) {
        LOGW("User dir %{public}s already exists", userDir.c_str());
        return E_OK;
    }
    
    // 3. 按顺序创建目录，失败时需回滚
    int32_t ret = CreateEl1Dirs(userId);
    if (ret != E_OK) {
        LOGE("Create EL1 dirs failed for user %{public}d", userId);
        RollbackUserDirs(userId);
        return E_CREATE_DIR_RECURSIVE_FAILED;
    }
    
    ret = CreateEl2Dirs(userId);
    if (ret != E_OK) {
        LOGE("Create EL2 dirs failed for user %{public}d", userId);
        RollbackUserDirs(userId);
        return E_CREATE_DIR_RECURSIVE_FAILED;
    }
    // EL3-EL5 类似...
    
    // 4. 设置权限和 SELinux context
    ret = SetUserDirPermissions(userId);
    if (ret != E_OK) {
        LOGE("Set permissions failed for user %{public}d", userId);
        RollbackUserDirs(userId);
        return E_PREPARE_DIR;
    }
    
    LOGI("Prepare user dirs for %{public}d success", userId);
    return E_OK;
}
```

## Manager 侧 IPC 入口模板

```cpp
int32_t StorageManagerProvider::Mount(const std::string &volumeId) {
    // 1. 权限校验（Manager 入口必须有）
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        LOGE("Permission denied for uid %{public}d", IPCSkeleton::GetCallingUid());
        return E_PERMISSION_DENIED;
    }
    
    // 2. 参数校验
    if (volumeId.empty()) {
        LOGE("VolumeId is empty");
        return E_PARAMS_INVALID;
    }
    
    // 3. 检查 Manager 侧状态
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    auto volume = volumeMap_.find(volumeId);
    if (volume == volumeMap_.end()) {
        LOGE("Volume %{public}s not found in Manager", volumeId.c_str());
        return E_NON_EXIST;
    }
    
    // 4. 调用 Daemon
    int32_t ret = storageDaemonCommunication_->Mount(volumeId);
    if (ret != E_OK) {
        LOGE("Daemon mount failed for %{public}s, ret %{public}d", volumeId.c_str(), ret);
        return ret;
    }
    
    // 5. 更新 Manager 侧状态
    volume->second->SetState(MOUNTED);
    LOGI("Mount %{public}s success", volumeId.c_str());
    return E_OK;
}
```

## Manager 侧 Stub 解析模板

```cpp
int32_t StorageManagerStub::OnMount(MessageParcel &data, MessageParcel &reply) {
    // 1. 读取参数
    std::string volumeId = data.ReadString();
    if (volumeId.empty()) {
        LOGE("Read volumeId failed from parcel");
        reply.WriteInt32(E_PARAMS_INVALID);
        return E_PARAMS_INVALID;
    }
    
    // 2. 调用实际实现
    int32_t ret = Mount(volumeId);
    
    // 3. 写入返回值
    reply.WriteInt32(ret);
    return ret;
}
```

## 新增文件系统 Operator 模板

```cpp
// 1. 头文件：services/storage_daemon/include/disk_manager/volume/xfs_operator.h
class XfsOperator : public IVolumeOperator {
public:
    XfsOperator() = default;
    ~XfsOperator() = default;
    
    int32_t Mount(const std::string &devPath, const std::string &mountPath,
                  const MountArgument &arg) override;
    int32_t UMount(const std::string &mountPath) override;
    int32_t Format(const std::string &devPath, const std::string &fsType) override;
    int32_t Check(const std::string &devPath) override;
    std::string GetFsType() override { return "xfs"; }
};

// 2. 实现：services/storage_daemon/disk_manager/volume/src/xfs_operator.cpp
int32_t XfsOperator::Mount(const std::string &devPath, const std::string &mountPath,
                           const MountArgument &arg) {
    std::vector<std::string> cmd = { "mount", "-t", "xfs" };
    if (arg.readOnly) {
        cmd.push_back("-o");
        cmd.push_back("ro");
    }
    cmd.push_back(devPath);
    cmd.push_back(mountPath);
    
    int32_t ret = ExecuteCommand(cmd, MOUNT_TIMEOUT_MS);
    if (ret != E_OK) {
        LOGE("XFS mount failed: %{public}s -> %{public}s, ret %{public}d",
             devPath.c_str(), mountPath.c_str(), ret);
        return E_OTHER_MOUNT;
    }
    return E_OK;
}

// 3. 注册到工厂：disk_manager/volume/volume_operator_factory.cpp
// 在 CreateOperator() 中添加：
if (fsType == "xfs") {
    return std::make_shared<XfsOperator>();
}
```

## Daemon 侧配额操作模板

```cpp
int32_t QuotaManager::SetBundleQuota(int32_t uid, const std::string &bundleDataDirPath,
                                     int32_t limitSizeMb) {
    // 1. 参数校验
    if (uid == 0 || bundleDataDirPath.empty() || limitSizeMb <= 0) {
        LOGE("Invalid uid %{public}d or limit %{public}d", uid, limitSizeMb);
        return E_PARAMS_INVALID;
    }
    
    // 2. 检查 quota 支持
    if (!IsQuotaSupported()) {
        LOGE("Quota not supported on this filesystem");
        return E_NOT_SUPPORT;
    }
    
    // 3. 执行 quota 设置
    int32_t ret = SetQuotaLimit(uid, bundleDataDirPath, limitSizeMb);
    if (ret != E_OK) {
        LOGE("Set quota failed for uid %{public}d, ret %{public}d", uid, ret);
        StorageRadar::ReportCommonResult("SetBundleQuota", ret, uid, "");
        return ret;
    }
    
    LOGI("Set quota for uid %{public}d limit %{public}d success", uid, limitSizeMb);
    return E_OK;
}

## 端到端业务示例

### 示例 1：用户创建完整流程

从系统事件到密钥激活的完整链路：

```cpp
// 步骤 1：AccountSubscriber 监听用户事件
void AccountSubscriber::OnAccountStateChanged(int userId, OsAccountState state) {
    LOGI("User %{public}d state changed to %{public}d", userId, state);
    
    if (state == OsAccountState::READY) {
        // 步骤 2：调用 Manager 入口
        StorageManagerProvider::GetInstance().PrepareAddUser(userId, CRYPTO_FLAG_EL1 | CRYPTO_FLAG_EL2);
    } else if (state == OsAccountState::STARTING) {
        StorageManagerProvider::GetInstance().PrepareStartUser(userId);
    } else if (state == OsAccountState::UNLOCKED) {
        StorageManagerProvider::GetInstance().CompleteAddUser(userId);
    }
}

// 步骤 3：Manager 侧 PrepareAddUser
int32_t StorageManagerProvider::PrepareAddUser(uint32_t userId, uint32_t flags) {
    // 权限校验
    if (!CheckClientPermission("ohos.permission.MANAGE_USER")) {
        return E_PERMISSION_DENIED;
    }
    
    // 参数校验
    if (userId < START_USER_ID || userId > MAX_USER_ID) {
        LOGE("Invalid userId %{public}d", userId);
        return E_USERID_RANGE;
    }
    
    // 调用 Daemon
    int32_t ret = DelayedSingleton<StorageDaemonCommunication>::GetInstance()->PrepareAddUser(userId, flags);
    if (ret != E_OK) {
        LOGE("PrepareAddUser failed for %{public}d, ret %{public}d", userId, ret);
        StorageRadar::ReportUserManager("PrepareAddUser", userId, ret, "");
        return ret;
    }
    
    LOGI("PrepareAddUser success for %{public}d", userId);
    return E_OK;
}

// 步骤 4：Daemon 侧 PrepareUserDirs
int32_t StorageDaemonProvider::PrepareAddUser(uint32_t userId, uint32_t flags) {
    LOGI("PrepareAddUser for %{public}d, flags %{public}u", userId, flags);
    
    // 创建目录
    int32_t ret = UserManager::GetInstance().PrepareUserDirs(userId);
    if (ret != E_OK) {
        LOGE("PrepareUserDirs failed: %{public}d", ret);
        return ret;
    }
    
    // 生成密钥
    ret = KeyManager::GetInstance().GenerateUserKeys(userId, flags);
    if (ret != E_OK) {
        LOGE("GenerateUserKeys failed: %{public}d", ret);
        UserManager::GetInstance().DestroyUserDirs(userId);
        return ret;
    }
    
    LOGI("PrepareAddUser complete for %{public}d", userId);
    return E_OK;
}

// 步骤 5：UserManager::PrepareUserDirs（见前面模板）
// 步骤 6：KeyManager::GenerateUserKeys（见前面模板）
// 步骤 7：CompleteAddUser 激活 EL2-EL4 密钥（见 crypto-key-management.md）
```

### 示例 2：SD 卡挂载完整流程

从内核事件到应用可见的完整链路：

```cpp
// 步骤 1：NetlinkHandler 监听内核事件
void NetlinkHandler::OnEvent(const NetlinkData &data) {
    LOGI("Netlink event: %{public}s for %{public}s", 
         data.action.c_str(), data.sysPath.c_str());
    
    if (data.action == "add") {
        // 投递到工作线程（避免阻塞 netlink）
        workerThread_.PostTask([data]() {
            DiskManager::Instance().HandleDiskEvent(data);
        });
    }
}

// 步骤 2：DiskManager 处理磁盘事件
int32_t DiskManager::HandleDiskEvent(const NetlinkData &data) {
    std::string sysPath = data.sysPath;
    
    if (data.action == "add") {
        // 创建磁盘对象
        auto disk = CreateDisk(sysPath, data.devPath, data.device, data.diskType);
        if (disk == nullptr) {
            LOGE("Create disk failed for %{public}s", sysPath.c_str());
            return E_DISK_CREATE_ERROR;
        }
        
        // 读取分区，为每个分区创建卷
        disk->ReadPartition();
        
        // IPC 通知 Manager
        StorageManagerClient::GetInstance().NotifyDiskCreated(disk->GetDiskInfo());
    }
    return E_OK;
}

// 步骤 3：DiskInfo::ReadPartition 创建卷
void DiskInfo::ReadPartition() {
    // 解析分区表
    std::vector<PartitionInfo> partitions;
    GetPartitionTable(partitions);
    
    for (const auto &part : partitions) {
        // 创建 VolumeInfo
        auto volume = VolumeManager::Instance().CreateVolume(
            GetDiskId(), part.devPath, part.volumeId, part.fsType);
        
        volume->SetState(UNMOUNTED);
        
        // IPC 通知 Manager
        StorageManagerClient::GetInstance().NotifyVolumeCreated(volume->GetVolumeInfo());
    }
}

// 步骤 4：Manager 侧接收通知
void DiskManagerService::OnDiskCreated(const DiskInfo &info) {
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    
    sptr<DiskInfo> disk = new DiskInfo(info);
    diskMap_[disk->GetDiskId()] = disk;
    
    LOGI("Disk %{public}s added to Manager", disk->GetDiskId().c_str());
    
    // 发布事件给上层应用
    PublishDiskEvent(disk->GetDiskId(), "add");
}

// 步骤 5：应用调用 Mount
int32_t StorageManagerProvider::Mount(const std::string &volumeId) {
    // 权限校验
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    
    // 状态检查
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    auto it = volumeMap_.find(volumeId);
    if (it == volumeMap_.end()) {
        return E_NON_EXIST;
    }
    
    if (it->second->GetState() != UNMOUNTED) {
        return E_VOL_STATE;
    }
    
    // 调用 Daemon
    int32_t ret = DelayedSingleton<StorageDaemonCommunication>::GetInstance()->Mount(volumeId, flag);
    if (ret != E_OK) {
        LOGE("Mount failed for %{public}s: %{public}d", volumeId.c_str(), ret);
        return ret;
    }
    
    it->second->SetState(MOUNTED);
    PublishVolumeEvent(volumeId, "mounted");
    return E_OK;
}

// 步骤 6：Daemon 侧 Mount（见前面模板）
// 步骤 7：VolumeInfo::Mount 执行实际挂载
int32_t VolumeInfo::Mount(uint32_t flags) {
    auto op = VolumeOperatorFactory::CreateOperator(fsType_);
    
    MountArgument arg;
    arg.readOnly = (flags & MOUNT_READONLY);
    arg.fsType = fsType_;
    
    int32_t ret = op->Mount(devicePath_, mountPath_, arg);
    if (ret != E_OK) {
        LOGE("Mount %{public}s failed: %{public}d", devicePath_.c_str(), ret);
        return ret;
    }
    
    LOGI("Volume %{public}s mounted at %{public}s", id_.c_str(), mountPath_.c_str());
    return E_OK;
}
```

### 示例 3：应用配额设置完整流程

从应用请求到 quota 落地的完整链路：

```cpp
// 步骤 1：应用调用 SetBundleQuota
int32_t StorageManagerProvider::SetBundleQuota(const std::string &bundleName, 
    int32_t userId, int64_t limit) {
    // 权限校验
    if (!CheckClientPermission("ohos.permission.SET_QUOTA")) {
        LOGE("Permission denied for SetBundleQuota");
        return E_PERMISSION_DENIED;
    }
    
    // 参数校验
    if (bundleName.empty() || userId < START_USER_ID || limit <= 0) {
        LOGE("Invalid params: bundle=%{public}s, userId=%{public}d, limit=%{public}lld",
             bundleName.c_str(), userId, (long long)limit);
        return E_PARAMS_INVALID;
    }
    
    // 获取应用 UID
    int uid = BundleMgrConnector::GetInstance().GetUidByBundleName(userId, bundleName);
    if (uid < 0) {
        LOGE("Get UID failed for %{public}s", bundleName.c_str());
        return E_GET_UID_ERROR;
    }
    
    // 调用 Daemon
    int32_t ret = DelayedSingleton<StorageDaemonCommunication>::GetInstance()->SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    if (ret != E_OK) {
        LOGE("SetBundleQuota failed: %{public}d", ret);
        StorageRadar::ReportCommonResult("SetBundleQuota", ret, uid, "");
        return ret;
    }
    
    LOGI("SetBundleQuota success: %{public}s -> %{public}lld bytes",
         bundleName.c_str(), (long long)limit);
    return E_OK;
}

// 步骤 2：Daemon 侧 SetBundleQuota
int32_t StorageDaemonProvider::SetBundleQuota(int32_t uid, int64_t limit) {
    // 路径校验
    if (!ValidateUid(uid)) {
        LOGE("Invalid uid %{public}d", uid);
        return E_PARAMS_INVALID;
    }
    
    return QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
}

// 步骤 3：QuotaManager::SetBundleQuota（见前面模板）
// 步骤 4：验证 quota 生效
int32_t VerifyQuota(int32_t uid) {
    struct dqblk dq;
    int ret = quotactl(QCMD(Q_GETQUOTA, USRQUOTA), "/data", uid, 
                       reinterpret_cast<char *>(&dq));
    
    if (ret == 0) {
        LOGI("Quota for uid %{public}d: cur=%{public}llu, limit=%{public}llu",
             uid, (unsigned long long)dq.dqb_curspace, 
             (unsigned long long)dq.dqb_bhardlimit * 1024);
        return E_OK;
    }
    return E_QUOTA_QUERY_FAILED;
}
```

### 示例 4：用户锁屏 EL2 密钥移除流程

```cpp
// 步骤 1：系统触发锁屏事件
void AccountSubscriber::OnScreenLocked(int userId) {
    LOGI("Screen locked for user %{public}d", userId);
    StorageManagerProvider::GetInstance().LockUserScreen(userId);
}

// 步骤 2：Manager 侧 LockUserScreen
int32_t StorageManagerProvider::LockUserScreen(uint32_t userId) {
    // 权限校验
    if (!CheckClientPermission("ohos.permission.MANAGE_USER")) {
        return E_PERMISSION_DENIED;
    }
    
    // 调用 Daemon
    int32_t ret = DelayedSingleton<StorageDaemonCommunication>::GetInstance()->LockUserScreen(userId);
    if (ret != E_OK) {
        LOGE("LockUserScreen failed for %{public}d: %{public}d", userId, ret);
        return ret;
    }
    
    LOGI("LockUserScreen success for %{public}d", userId);
    return E_OK;
}

// 步骤 3：Daemon 侧 LockUserScreen
int32_t StorageDaemonProvider::LockUserScreen(uint32_t userId) {
    return KeyManager::GetInstance().LockUserScreen(userId);
}

// 步骤 4：KeyManager::LockUserScreen
int32_t KeyManager::LockUserScreen(uint32_t userId) {
    std::lock_guard<std::mutex> lock(keyMutex_);
    
    // 移除 EL2 密钥（核心安全操作）
    int32_t ret = InActiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("Inactive EL2 key failed for %{public}d: %{public}d", userId, ret);
        StorageRadar::ReportUserKeyResult("LockScreen", userId, ret, "EL2", "");
        return ret;
    }
    
    // 更新用户锁定状态
    userLockState_[userId] = true;
    
    LOGI("EL2 key removed for user %{public}d (screen locked)", userId);
    return E_OK;
}

// 步骤 5：BaseKey::InactiveKey
int32_t BaseKey::InactiveKey() {
    // 从内核 keyring 移除密钥
    key_serial_t keyId = GetKeyId();
    if (keyId <= 0) {
        LOGW("Key not installed, skip inactive");
        return E_OK;
    }
    
    long ret = keyctl(KEYCTL_REVOKE, keyId);
    if (ret != 0) {
        LOGE("Keyctl revoke failed: %{public}d", errno);
        return E_KEY_REVOKE_ERROR;
    }
    
    LOGI("Key %{public}d revoked from kernel", keyId);
    return E_OK;
}
```

## 模板使用规则

1. **参数校验必须有**：所有公开接口必须有参数校验
2. **状态预更新**：卷/磁盘操作必须先更新状态再执行
3. **失败回滚**：用户目录创建失败必须回滚已创建的目录
4. **权限校验**：Manager 侧入口必须有权限校验
5. **日志必须有**：成功和失败都必须有日志
6. **上报必须有**：关键操作失败必须通过 StorageRadar 上报
7. **错误码必须具体**：禁止使用 E_ERR，使用具体错误码如 E_VOL_STATE、E_USERID_RANGE
```