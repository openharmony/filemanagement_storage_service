# 测试指南

本文记录单元测试、Fuzz 测试、Mock 使用和测试场景覆盖要求。

## 单元测试

| 模块 | 测试类 | 路径 |
|------|--------|------|
| 密钥管理 | `KeyManagerTest` | `services/storage_daemon/crypto/test/key_manager_test/` |
| 用户管理 | `UserManagerTest` | `services/storage_daemon/user/test/` |
| 卷管理 | `VolumeManagerTest` | `services/storage_daemon/volume/test/` |
| 磁盘管理 | `DiskManagerTest` | `services/storage_daemon/disk/test/` |
| 配额管理 | `QuotaManagerTest` | `services/storage_daemon/quota/test/` |
| 空间统计 | `StorageStatusManagerTest` | `services/storage_manager/storage/test/` |
| IPC 层 | `StorageManagerProxyTest` | `services/storage_manager/ipc/test/` |

## Fuzz 测试

位于 `test/fuzztest/`，覆盖：
- `StorageManagerProxyFuzzer`：Manager 代理层
- `StorageManagerStubFuzzer`：Manager Stub 层
- `StorageManagerProviderFuzzer`：Manager Provider 层
- `KeyControlFuzzer`：密钥控制
- `VolumeCoreFuzzer`：卷核心操作
- `BundleStatsFuzzer`：空间统计

## 测试覆盖要求

- 新增功能必须有对应单元测试
- 公开 API 必须有 fuzz 测试
- 加密/用户相关操作需板侧验证
- 状态机转换需覆盖所有合法路径

## 测试场景覆盖清单

每个模块必须覆盖以下测试场景：

### KeyManager

| 函数 | 必测场景 | 覆盖要求 |
|------|----------|----------|
| GenerateUserKeys | userId 正常(100)、边界(100/10738)、超范围(99/10739)、flags 组合、HUKS 不可用、密钥已存在 | 全覆盖 |
| ActiveUserKey | userId 有效、userId 无效、密钥类型(EL1-EL5)、密钥不存在 | 全覆盖 |
| InactiveUserKey | userId 有效、userId 无效、密钥已停用 | 全覆盖 |
| LockUserScreen | userId 有效、userId 无效、密钥状态 | 全覆盖 |
| UnlockUserScreen | userId 有效、token/secret 无效、密钥状态 | 全覆盖 |
| UpdateUserAuth | userId 有效、token/secret 无效、宏差异(USER_CRYPTO_MIGRATE_KEY) | 全覆盖 |

### VolumeManager

| 函数 | 必测场景 | 覆盖要求 |
|------|----------|----------|
| Mount | volumeId 正常、volumeId 空、volume 不存在、状态不对(UNMOUNTED以外)、挂载超时、Check 失败 | 全覆盖 |
| Unmount | volumeId 正常、volumeId 空、volume 不存在、状态不对(MOUNTED以外)、强制卸载 | 全覆盖 |
| Format | volumeId 正常、fsType 有效(ext4/exfat/vfat/ntfs)、fsType 无效、状态不对 | 全覆盖 |
| TryToFix | volumeId 正常、volume 不损坏、volume 已损坏、修复成功、修复失败 | 全覆盖 |

### UserManager

| 函数 | 必测场景 | 覆盖要求 |
|------|----------|----------|
| PrepareUserDirs | userId 正常、userId 边界、userId 无效、目录已存在、目录创建失败、挂载失败回滚 | 全覆盖 |
| DestroyUserDirs | userId 正常、userId 无效、目录不存在、卸载失败 | 全覆盖 |
| StartUser | userId 正常、userId 无效、目录不存在、EL1 密钥激活失败 | 全覆盖 |
| StopUser | userId 正常、userId 无效、密钥停用失败 | 全覆盖 |

### QuotaManager

| 函数 | 必测场景 | 覆盖要求 |
|------|----------|----------|
| SetBundleQuota | uid 正常、uid 无效、limit 为 0、limit 超上限、quota 不支持 | 全覆盖 |
| GetOccupiedSpaceForUid | uid 正常、uid 无效、空间查询失败 | 全覆盖 |

### 状态机

| 场景 | 必测转换 | 覆盖要求 |
|------|----------|----------|
| VolumeState | UNMOUNTED→CHECKING、CHECKING→MOUNTED、MOUNTED→EJECTING、EJECTING→REMOVED、UNMOUNTED→BADREMOVABLE、MOUNTED→DAMAGED、DAMAGED→DAMAGED_MOUNTED、非法转换拒绝 | 全覆盖 |
| DiskState | MOUNTED→REMOVED、非法转换拒绝 | 全覆盖 |

### IPC 层

| 场景 | 必测场景 | 覆盖要求 |
|------|----------|----------|
| Stub 解析 | 参数正常、参数空、参数超长、参数类型错误 | 全覆盖 |
| Manager Provider | 权限正常、权限不足、Daemon 连接正常、Daemon 连接断开 | 全覆盖 |
| Daemon Provider | 参数正常、参数空、底层调用成功、底层调用失败 | 全覆盖 |

## 测试用例示例

```cpp
// 覆盖所有边界条件
TEST_F(KeyManagerTest, GenerateUserKeys_ValidUserId) {
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(100, CRYPTO_FLAG_EL1), E_OK);
}

TEST_F(KeyManagerTest, GenerateUserKeys_BoundaryUserId) {
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(START_USER_ID, CRYPTO_FLAG_EL1), E_OK);
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(MAX_USER_ID, CRYPTO_FLAG_EL1), E_OK);
}

TEST_F(KeyManagerTest, GenerateUserKeys_InvalidUserId) {
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(99, CRYPTO_FLAG_EL1), E_USERID_RANGE);
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(10739, CRYPTO_FLAG_EL1), E_USERID_RANGE);
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(0, CRYPTO_FLAG_EL1), E_USERID_RANGE);
}

TEST_F(KeyManagerTest, GenerateUserKeys_HuksUnavailable) {
    EXPECT_CALL(*mockHuks, IsHuksAvailable()).WillOnce(Return(false));
    EXPECT_EQ(KeyManager::GetInstance().GenerateUserKeys(100, CRYPTO_FLAG_EL1), E_GLOBAL_KEY_INIT_ERROR);
}

TEST_F(VolumeManagerTest, Mount_StateTransition_Valid) {
    auto volume = CreateTestVolume("vol-1");
    volume->SetState(UNMOUNTED);
    EXPECT_EQ(VolumeManager::Instance().Mount("vol-1", 0), E_OK);
    EXPECT_EQ(volume->GetState(), MOUNTED);
}

TEST_F(VolumeManagerTest, Mount_StateTransition_Invalid) {
    auto volume = CreateTestVolume("vol-2");
    volume->SetState(MOUNTED);
    EXPECT_EQ(VolumeManager::Instance().Mount("vol-2", 0), E_VOL_STATE);
    EXPECT_EQ(volume->GetState(), MOUNTED);
}
```

## Mock 指南

Mock 文件位于 `services/storage_daemon/include/mock/`：

| Mock 文件 | 用途 | Mock 方法 |
|-----------|------|----------|
| `base_key_mock.h` | Mock BaseKey | `MOCK_METHOD(ActiveKey)`, `MOCK_METHOD(InactiveKey)` |
| `huks_master_mock.h` | Mock HUKS | `MOCK_METHOD(EncryptKey)`, `MOCK_METHOD(DecryptKey)` |
| `iam_client_mock.h` | Mock IAM | `MOCK_METHOD(AuthUser)` |
| `key_manager_mock.h` | Mock KeyManager | `MOCK_METHOD(GenerateUserKeys)` |
| `mount_manager_mock.h` | Mock 挂载 | `MOCK_METHOD(Mount)`, `MOCK_METHOD(Umount)` |
| `fscrypt_control_mock.h` | Mock fscrypt | `MOCK_METHOD(InstallKey)`, `MOCK_METHOD(RemoveKey)` |
| `file_utils_mock.h` | Mock 文件操作 | `MOCK_METHOD(Mkdir)`, `MOCK_METHOD(Chmod)` |
| `disk_utils_mock.h` | Mock 磁盘 | `MOCK_METHOD(GetDiskInfo)` |

### Mock 使用示例

```cpp
class KeyManagerTest : public testing::Test {
public:
    void SetUp() override {
        mockHuks = std::make_shared<MockHuksMaster>();
        mockBaseKey = std::make_shared<MockBaseKey>();
        
        EXPECT_CALL(*mockHuks, EncryptKey(_, _, _, _))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(*mockBaseKey, ActiveKey(_, _, _))
            .WillOnce(Return(E_OK));
    }
    
    std::shared_ptr<MockHuksMaster> mockHuks;
    std::shared_ptr<MockBaseKey> mockBaseKey;
};

TEST_F(KeyManagerTest, GenerateUserKeys_Success) {
    int32_t ret = KeyManager::GetInstance().GenerateUserKeys(100, CRYPTO_FLAG_EL1);
    EXPECT_EQ(ret, E_OK);
}
```