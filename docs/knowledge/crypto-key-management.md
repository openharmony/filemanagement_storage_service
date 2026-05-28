# 加密密钥管理

本文记录密钥类型、密钥生命周期、恢复密钥、App 密钥和典型场景完整流程。

## KeyType 和 EL 等级

```cpp
enum KeyType {
    EL0_KEY = 0,  // 设备级：设备启动时激活
    EL1_KEY = 1,  // 设备级：设备启动 + 用户 StartUser
    EL2_KEY = 2,  // 用户凭据：锁屏移除，解锁安装
    EL3_KEY = 3,  // 用户凭据：CompleteAddUser 激活
    EL4_KEY = 4,  // 用户凭据：CompleteAddUser 激活
    EL5_KEY = 5,  // 应用级：UECE 回调激活
};

// 加密标志位
enum IStorageDaemonEnum {
    CRYPTO_FLAG_EL1 = 1,
    CRYPTO_FLAG_EL2 = 2,
    CRYPTO_FLAG_EL3 = 4,
    CRYPTO_FLAG_EL4 = 8,
    CRYPTO_FLAG_EL5 = 16,
};
```

### 密钥存储路径

| 等级 | 密钥路径 |
|------|----------|
| 设备 EL0 | `/data/service/el0/storage_daemon/sd` |
| 设备 EL1 | `/data/service/el1/public/storage_daemon/sd/el1` |
| 用户 EL1 | `/data/service/el1/public/storage_daemon/sd/el1/{userId}` |
| 用户 EL2 | `/data/service/el1/public/storage_daemon/sd/el2/{userId}` |
| 用户 EL3 | `/data/service/el1/public/storage_daemon/sd/el3/{userId}` |
| 用户 EL4 | `/data/service/el1/public/storage_daemon/sd/el4/{userId}` |
| 用户 EL5 | `/data/service/el1/public/storage_daemon/sd/el5/{userId}` |

### 密钥文件结构

```
/data/service/el1/public/storage_daemon/sd/el2/{userId}/
  ├ version_2            # 密钥版本
  ├ encrypted_key        # 加密的密钥数据
  ├ master_key           # 主密钥（HUKS 加密）
  ├ need_update          # 更新标记
  └ need_restore         # 恢复标记
```

## 密钥生命周期完整流程

### 设备启动

```cpp
KeyManager::InitGlobalDeviceKey()
  ├ 检查是否存在设备密钥
  ├ 如果存在：RestoreDeviceKey()
  │   └ 加载密钥文件
  │   └ 使用 HUKS 解密
  │   └ 安装到内核 keyring
  └ 如果不存在：GenerateAndInstallDeviceKey()
      └ 生成随机密钥
      └ 使用 HUKS 加密存储
      └ 安装到内核

KeyManager::InitGlobalUserKeys()
  └ 遍历所有已存在用户
  └ LoadAllUsersEl1Key()
      └ 为每个用户加载 EL1 密钥
```

### 用户创建

```cpp
KeyManager::GenerateUserKeys(userId, flags)
  ├ 参数校验：userId ∈ [100, 10738]
  ├ HUKS 可用性检查
  ├ 检查密钥是否已存在
  ├ 按标志位生成对应等级密钥：
  │   if (flags & CRYPTO_FLAG_EL1) {
  │       GenerateAndInstallUserKey(userId, EL1_KEY);
  │   }
  │   if (flags & CRYPTO_FLAG_EL2) {
  │       GenerateAndInstallUserKey(userId, EL2_KEY);
  │   }
  │   // EL3-EL5 类似
  └ 设置目录加密策略 SetDirectoryElPolicy()

GenerateAndInstallUserKey(userId, type)
  ├ 创建密钥目录
  ├ BaseKey::InitKey(true) 生成密钥
  ├ BaseKey::StoreKey(auth) 存储密钥
  │   └ HuksMaster::GetInstance().EncryptKey() 加密密钥
  │   └ 保存加密后的密钥文件
  └ 注册密钥到内存 userElKeys_
```

### 用户解锁屏幕

```cpp
KeyManager::UnlockUserScreen(userId, token, secret)
  ├ 参数校验
  ├ 检查用户状态
  ├ ActiveCeSceSeceUserKey(userId, EL2_KEY, token, secret)
  │   ├ GetUserElKey(userId, EL2_KEY) 获取密钥对象
  │   ├ 解密密钥数据
  │   ├ BaseKey::ActiveKey(authToken)
  │   │   └ 安装密钥到内核 keyring
  │   └ 更新用户解锁状态
  └ 如果有 UECE：NotifyUeceActivation(userId)
```

### 用户锁屏

```cpp
KeyManager::LockUserScreen(userId)
  ├ InactiveUserElKey(userId, EL2_KEY)
  │   ├ BaseKey::InactiveKey()
  │   │   └ 从内核 keyring 移除密钥
  │   └ 清空内存中的密钥缓存
  └ 更新用户锁定状态
```

### 用户删除

```cpp
KeyManager::DeleteUserKeys(userId)
  ├ DoDeleteUserKeys(userId)
  │   ├ 遍历 EL1-EL5
  │   ├ 每个等级：
  │   │   ├ 从内核移除密钥
  │   │   ├ 删除密钥文件
  │   │   └ 清空内存缓存
  └ 清理密钥目录
```

## 密钥版本兼容

### V1 vs V2

```cpp
std::shared_ptr<BaseKey> KeyManager::GetBaseKey(const std::string &dir) {
    int version = GetFscryptVersion();
    if (version == 1) {
        return std::make_shared<FscryptKeyV1>(dir);
    } else {
        return std::make_shared<FscryptKeyV2>(dir);
    }
}

// 版本判断依据
int GetFscryptVersion() {
    // 1. 检查 /fscrypt_version 文件
    // 2. 检查编译宏
    // 3. 默认返回 V2
}
```

### 版本差异

| 特性 | V1 | V2 |
|------|----|----|
| 密钥格式 | 传统格式 | 新格式（更强的加密） |
| HUKS 调用 | 直接调用 | 通过 HDI 调用 |
| 密钥恢复 | 简单恢复 | TEE 环境恢复 |
| 编译宏 | 无 | `HUKS_IDL_ENVIRONMENT` |

## 恢复密钥

### 创建恢复密钥

```cpp
KeyManager::CreateRecoverKey(userId, userType, token, secret)
  ├ 创建恢复密钥
  ├ 使用 TEE 加密存储（如果启用）
  ├ 保存到 RecoverManager
  └ 返回恢复密钥 ID
```

### 使用恢复密钥重置密码

```cpp
KeyManager::ResetSecretWithRecoveryKey(userId, rkType, key)
  ├ 从 RecoverManager 获取恢复密钥
  ├ 使用恢复密钥解密用户密钥
  ├ 重新生成用户认证密钥
  ├ 更新密钥存储
  └ 返回结果
```

### 恢复密钥索引

```cpp
enum RecoverKeyIndex {
    DEVICE_EL1 = 0,       // 设备 EL1 恢复密钥
    GLOBAL_USER_EL1 = 1,  // 全局用户 EL1 恢复密钥
    USER_EL1 = 2,         // 用户 EL1 恢复密钥
    USER_EL2 = 3,         // 用户 EL2 恢复密钥
    USER_EL3 = 4,         // 用户 EL3 恢复密钥
    USER_EL4 = 5,         // 用户 EL4 恢复密钥
};
```

## App 密钥

### 生成 App 密钥

```cpp
KeyManager::GenerateAppkey(userId, hashId, keyId, needReSet)
  ├ 检查用户密钥是否激活
  ├ 生成应用专用密钥
  ├ 使用用户密钥加密
  ├ 安装到内核
  └ 返回 keyId

// keyId 格式："{userId}_{hashId}"
```

### 删除 App 密钥

```cpp
KeyManager::DeleteAppkey(userId, keyId)
  ├ 从内核移除密钥
  ├ 删除密钥文件
  └ 清理内存缓存
```

## 编译宏影响

| 编译宏 | 影响 |
|--------|------|
| `USER_CRYPTO_MIGRATE_KEY` | `UpdateUserAuth` 增加 `needGenerateShield` 参数 |
| `RECOVER_KEY_TEE_ENVIRONMENT` | 恢复密钥使用 TEE 环境 |
| `EL5_FILEKEY_MANAGER` | UECE 回调注册功能 |
| `USER_AUTH_FRAMEWORK` | 使用用户认证框架 |
| `HUKS_IDL_ENVIRONMENT` | HUKS 使用 HDI 接口 |

```cpp
#ifdef USER_CRYPTO_MIGRATE_KEY
int UpdateUserAuth(userId, userTokenSecret, needGenerateShield = true);
#else
int UpdateUserAuth(userId, userTokenSecret);
#endif
```

## 典型场景：用户解锁完整流程

```
用户输入密码解锁
  └ OsAccountManager 解锁验证
    └ AccountSubscriber::OnAccountStateChanged(userId, UNLOCKED)
      └ StorageManagerProvider::CompleteAddUser(userId)
        StorageDaemonCommunication::CompleteAddUser(userId)
          StorageDaemonProvider::CompleteAddUser(userId)
            KeyManager::CompleteAddUser(userId)
              ├ ActiveUserKey(userId, EL2_KEY)
              │   ├ GetUserElKey(userId, EL2_KEY)
              │   ├ 解密密钥数据（使用 token/secret）
              │   ├ BaseKey::ActiveKey()
              │   │   └ KeyControl::AddKey() 安装到内核
              │   └ 更新状态
              ├ ActiveUserKey(userId, EL3_KEY)
              ├ ActiveUserKey(userId, EL4_KEY)
              └ 如果有 UECE：NotifyUeceActivation(userId)
```

## 头文件路径

```
services/storage_daemon/include/crypto/key_manager.h      # KeyManager
services/storage_daemon/include/crypto/base_key.h         # BaseKey 基类
services/storage_daemon/include/crypto/fscrypt_key_v1.h   # V1 实现
services/storage_daemon/include/crypto/fscrypt_key_v2.h   # V2 实现
services/storage_daemon/include/crypto/huks_master.h      # HUKS 封装
services/storage_daemon/include/crypto/recover_manager.h  # 恢复密钥
services/storage_daemon/include/crypto/crypto_delay_handler.h # 延迟处理
```

## 测试指引

- 密钥生成/激活：`KeyManagerTest`
- 密钥版本兼容：`FscryptKeyV1Test`, `FscryptKeyV2Test`
- 锁屏/解锁：`KeyControlFuzzer`
- 恢复密钥：`RecoverManagerTest`
- HUKS 调用：`HuksMasterTest`
- 涉及真实 HUKS/TEE：需要板侧验证