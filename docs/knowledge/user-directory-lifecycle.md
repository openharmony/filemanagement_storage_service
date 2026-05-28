# 用户目录生命周期

本文记录用户目录创建/销毁时序、目录结构、多用户隔离规则和典型场景完整流程。

## 用户生命周期时序

用户存储操作必须严格按以下顺序执行，不能跳步或乱序：

```
1. PrepareAddUser(userId, flags)
   ├ UserManager::PrepareUserDirs(userId)    // 创建目录
   ├ KeyManager::GenerateUserKeys(userId, flags) // 生成密钥
   └ 设置目录加密策略

2. StartUser(userId)
   ├ MountManager::MountEl1Dirs(userId)     // 挂载 EL1 目录
   ├ KeyManager::ActiveUserKey(userId, EL1_KEY) // 激活 EL1 密钥
   └ 挂载 HMDFS、沙箱等

3. CompleteAddUser(userId)
   ├ KeyManager::ActiveUserKey(userId, EL2_KEY) // 激活 EL2-EL4 密钥
   ├ 完成用户注册

4. StopUser(userId)
   ├ KeyManager::InactiveUserKey(userId)    // 停用密钥
   └ 卸载挂载点

5. RemoveUser(userId, flags)
   ├ UserManager::DestroyUserDirs(userId)   // 销毁目录
   ├ KeyManager::DeleteUserKeys(userId)     // 删除密钥
```

`AccountSubscriber` 监听 `OsAccountState` 事件自动触发对应步骤，不要绕过它直接调用 Daemon。

## EL 等级目录结构

每个用户拥有独立的加密等级目录：

| 等级 | 路径 | 密钥激活时机 | 典型内容 |
|------|------|--------------|----------|
| EL1 | `/data/service/el1/public/{userId}` | StartUser | 设备级全局数据 |
| EL2 | `/data/service/el2/{userId}` | CompleteAddUser / UnlockUserScreen | 用户凭据保护数据 |
| EL3 | `/data/service/el3/{userId}` | CompleteAddUser | 高安全等级数据 |
| EL4 | `/data/service/el4/{userId}` | CompleteAddUser | 最高安全等级数据 |
| EL5 | `/data/app/el5/{userId}` | UECE 激活 | 应用级加密 |

### 目录创建顺序

```cpp
// UserManager::PrepareUserDirs() 实现顺序
int32_t UserManager::PrepareUserDirs(uint32_t userId) {
    // 1. 基础用户目录
    CreateDir("/data/user/" + userId);           // 主目录
    CreateSymlink("/data/user/" + userId, "/data/media/" + userId); // 媒体链接
    
    // 2. 服务目录（各 EL 等级）
    CreateDir("/data/service/el1/public/" + userId);
    CreateDir("/data/service/el2/" + userId);
    CreateDir("/data/service/el3/" + userId);
    CreateDir("/data/service/el4/" + userId);
    
    // 3. 应用目录
    CreateDir("/data/app/el1/" + userId);
    CreateDir("/data/app/el2/" + userId);
    CreateDir("/data/app/el5/" + userId);
    
    // 4. HMDFS 挂载点
    CreateDir("/mnt/hmdfs/" + userId + "/account");
    CreateDir("/mnt/hmdfs/" + userId + "/non_account");
    CreateDir("/mnt/hmdfs/" + userId + "/cloud");
    
    // 5. 沙箱目录
    CreateDir("/mnt/sandbox/" + userId);
    
    // 6. 设置权限和 SELinux
    SetPermissions(userId);
    return E_OK;
}
```

## 多用户隔离规则

### 用户 ID 范围

```cpp
START_USER_ID = 100           // 最小用户 ID
MAX_USER_ID = 10738           // 最大用户 ID
START_APP_CLONE_USER_ID = ... // AppClone 起始 ID
MAX_APP_CLONE_USER_ID = ...   // AppClone 最大 ID
USER_ID_BASE = 200000         // UID 计算基数
// uid = USER_ID_BASE * userId + appId
```

### 隔离规则

```cpp
// 错误：跨用户操作
KeyManager::GetInstance().ActiveUserKey(100);  // 激活用户 100 的密钥
AccessDirectory("/data/service/el2/105");     // 访问用户 105 的目录

// 正确：使用目标用户的密钥
KeyManager::GetInstance().ActiveUserKey(105);
AccessDirectory("/data/service/el2/105");
```

## NATO 和 AppClone 用户

### NATO 用户

NATO（Non-Authenticated Trusted OS）用户有独立的恢复目录：

```
/data/service/el1/public/storage_daemon/sd/el2_NATO/{userId}
/data/service/el1/public/storage_daemon/sd/el3_NATO/{userId}
/data/service/el1/public/storage_daemon/sd/el4_NATO/{userId}
```

### AppClone 用户

AppClone 用户 ID 范围独立，密钥管理使用 `AppCloneKeyManager`：

```cpp
if (IsAppCloneUser(userId)) {
    AppCloneKeyManager::GetInstance().GenerateUserKeys(userId, flags);
} else {
    KeyManager::GetInstance().GenerateUserKeys(userId, flags);
}
```

## 失败回滚机制

```cpp
int32_t UserManager::PrepareUserDirs(uint32_t userId) {
    int32_t ret = CreateEl1Dirs(userId);
    if (ret != E_OK) {
        RollbackUserDirs(userId);  // 回滚已创建的目录
        return E_CREATE_DIR_RECURSIVE_FAILED;
    }
    
    ret = CreateEl2Dirs(userId);
    if (ret != E_OK) {
        RollbackUserDirs(userId);
        return E_CREATE_DIR_RECURSIVE_FAILED;
    }
    // ... EL3-EL5 类似
    
    return E_OK;
}

void UserManager::RollbackUserDirs(uint32_t userId) {
    RemoveDir("/data/user/" + userId);
    RemoveDir("/data/service/el1/public/" + userId);
    RemoveDir("/data/service/el2/" + userId);
    RemoveDir("/data/service/el3/" + userId);
    RemoveDir("/data/service/el4/" + userId);
    RemoveDir("/data/app/el1/" + userId);
    RemoveDir("/data/app/el2/" + userId);
    RemoveDir("/data/app/el5/" + userId);
}
```

## 典型场景：用户创建完整流程

```
系统触发 OsAccountManager::CreateOsAccount()
  └ AccountSubscriber::OnAccountStateChanged(userId, READY)
    └ StorageManagerProvider::PrepareAddUser(userId, flags)
       DelayedSingleton<StorageDaemonCommunication>::GetInstance()->PrepareAddUser()
         StorageDaemonProvider::PrepareUserDirs()
           UserManager::PrepareUserDirs()
           KeyManager::GenerateUserKeys(userId, flags)
             HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce)  // 加密密钥
             BaseKey::StoreKey()           // 存储密钥文件
             SetDirectoryElPolicy()        // 设置加密策略

用户解锁后：
   AccountSubscriber::OnAccountStateChanged(userId, UNLOCKED)
     StorageManagerProvider::CompleteAddUser(userId)
       KeyManager::ActiveUserKey(userId, EL2_KEY)
         解密并安装 EL2 密钥到内核
```

## 头文件路径

```
services/storage_daemon/include/user/user_manager.h        # UserManager 定义
services/storage_daemon/include/user/mount_manager.h       # 挂载管理
services/storage_daemon/include/crypto/key_manager.h       # 密钥管理
services/storage_manager/include/account_subscriber/account_subscriber.h # 用户事件订阅
```

## 测试指引

- 目录创建/销毁：`UserManagerTest`
- 多用户隔离：`StorageManagerProviderTest`
- 用户生命周期：`AccountSubscriberTest`
- 密钥操作：`KeyManagerTest`
- 涉及真实挂载/加密：需要板侧验证