# 外部依赖

本文记录外部依赖服务和编译宏影响。

## 外部依赖服务

| 依赖服务 | 用途 | 头文件 | 检查方法 |
|----------|------|--------|----------|
| HUKS | 密钥加密/解密 | `crypto/huks_master.h` | `HuksMaster::GetInstance().IsHuksAvailable()` |
| TEE | 安全执行环境（恢复密钥） | 编译宏 | 检查 `RECOVER_KEY_TEE_ENVIRONMENT` |
| OsAccount | 用户账号管理 | `os_account_manager.h` | `OsAccountManager::IsOsAccountServiceAvailable()` |
| AccessToken | 权限管理 | `accesstoken_kit.h` | `AccessTokenKit::GetTokenType()` |
| BundleFramework | 应用信息查询 | `bundle_mgr_interface.h` | 检查连接状态 |
| HiSysEvent | 事件上报 | `hisysevent.h` | 直接调用 |
| HiLog | 日志输出 | `hilog/log.h` | 直接调用 |

## 依赖不可用时的处理

### HUKS 不可用

```cpp
int32_t KeyManager::GenerateUserKeys(uint32_t userId, uint32_t flags) {
    if (!HuksMaster::GetInstance().IsHuksAvailable()) {
        LOGE("HUKS not available, cannot generate keys for user %{public}d", userId);
        return E_GLOBAL_KEY_INIT_ERROR;
    }
    // ...
}
```

### OsAccount 不可用

```cpp
int32_t UserManager::PrepareUserDirs(uint32_t userId) {
    bool isOsAccountExists = false;
    int32_t ret = OsAccountManager::IsOsAccountExists(userId, isOsAccountExists);
    if (ret != E_OK || !isOsAccountExists) {
        LOGE("OsAccount %{public}d not exists", userId);
        return E_SA_IS_NULLPTR;
    }
    // ...
}
```

### AccessToken 权限检查

```cpp
int32_t StorageManagerProvider::Mount(const std::string &volumeId) {
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (AccessTokenKit::GetTokenType(tokenId) == TOKEN_INVALID) {
        LOGE("Invalid token");
        return E_PERMISSION_DENIED;
    }
    // ...
}
```

## 编译宏影响

关键编译宏会影响代码行为，写代码时需注意：

| 编译宏 | 影响范围 | 行为差异 |
|--------|----------|----------|
| `USER_CRYPTO_MIGRATE_KEY` | KeyManager | `UpdateUserAuth` 增加 `needGenerateShield` 参数 |
| `RECOVER_KEY_TEE_ENVIRONMENT` | RecoverManager | 恢复密钥使用 TEE 环境，新增 `FileBasedEncryptfsMount` |
| `EL5_FILEKEY_MANAGER` | KeyManager | UECE 回调注册功能 `RegisterUeceActivationCallback` |
| `USER_AUTH_FRAMEWORK` | IAMClient | 用户认证框架支持，启用后依赖 `userauth_client` |
| `HUKS_IDL_ENVIRONMENT` | HuksMaster | HUKS HDI 接口版本，使用 `libhuks_proxy_1.1` |
| `SUPPORT_RECOVERY_KEY_SERVICE` | KeyManager | 恢复密钥服务支持 |

### USER_CRYPTO_MIGRATE_KEY

```cpp
#ifdef USER_CRYPTO_MIGRATE_KEY
int UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
                   bool needGenerateShield = true);  // 有额外参数
#else
int UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret);  // 无额外参数
#endif

// 写代码时应考虑宏是否定义
#ifdef USER_CRYPTO_MIGRATE_KEY
    ret = KeyManager::GetInstance().UpdateUserAuth(userId, userTokenSecret, true);
#else
    ret = KeyManager::GetInstance().UpdateUserAuth(userId, userTokenSecret);
#endif
```

### RECOVER_KEY_TEE_ENVIRONMENT

```cpp
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
// TEE 环境下的恢复密钥处理
int32_t FileBasedEncryptfsMount();
int32_t InstallEmptyUserKeyForRecovery(uint32_t userId);
#endif
```

### EL5_FILEKEY_MANAGER

```cpp
#ifdef EL5_FILEKEY_MANAGER
// UECE 回调注册
int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &callback);
int32_t UnregisterUeceActivationCallback();
#endif
```

### USER_AUTH_FRAMEWORK

```cpp
#ifdef USER_AUTH_FRAMEWORK
// 使用用户认证框架
#include "userauth_client.h"
// IAM 认证调用
#else
// 使用旧版 IAM 接口
#include "iam_client.h"
#endif
```

## 依赖初始化顺序

```
1. Daemon 启动
   └ InitGlobalDeviceKey() → 检查 HUKS
   └ InitGlobalUserKeys() → 加载所有用户 EL1 密钥
   └ NetlinkManager::Start() → 监听设备事件

2. Manager 启动
   └ StorageDaemonCommunication::Connect() → 连接 Daemon
   └ StorageMonitorService::Start() → 启动监控
   └ AccountSubscriber::Subscribe() → 监听用户事件
```