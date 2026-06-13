# 测试覆盖要求

本文只记录测试场景覆盖要求和 Mock 选择规则，不包含测试代码示例。状态机转换测试见 `enums-and-state-machine.md`，IPC 测试见 `ipc-interface-guide.md`。

## 覆盖要求

所有新增功能必须有对应单元测试，公开 API 必须有 fuzz 测试。

## 必测场景

### KeyManager

| 函数 | 必测场景 |
|------|----------|
| GenerateUserKeys | userId 正常(100)、边界(100/10738)、超范围(99/10739)、flags 组合、HUKS 不可用、密钥已存在 |
| ActiveUserKey | userId 有效/无效、密钥类型 EL1-EL5、密钥不存在 |
| LockUserScreen | userId 有效/无效、密钥状态 |
| UnlockUserScreen | userId 有效、token/secret 无效 |

### VolumeManager

| 函数 | 必测场景 |
|------|----------|
| Mount | volumeId 正常/空/不存在、状态不对(非 UNMOUNTED)、Check 失败 |
| Unmount | volumeId 正常/空/不存在、状态不对(非 MOUNTED/DAMAGED_MOUNTED) |
| Format | volumeId 正常、fsType 有效/无效、状态不对 |

### UserManager

| 函数 | 必测场景 |
|------|----------|
| PrepareUserDirs | userId 正常/边界/无效、目录已存在、创建失败回滚 |
| DestroyUserDirs | userId 正常/无效、目录不存在 |

### QuotaManager

| 函数 | 必测场景 |
|------|----------|
| SetBundleQuota | uid 正常/无效、limit 为 0/超上限、quota 不支持 |

### 状态机

| 必测 | 合法转换 + 所有非法跳转拒绝 |
|------|------|

### IPC Stub

| 必测 | 参数正常/空/超长/类型错误 |

## Mock 选择

| Mock 文件 | 用途 |
|-----------|------|
| `huks_master_mock.h` | Mock HUKS 加密/解密 |
| `base_key_mock.h` | Mock 密钥激活/停用 |
| `iam_client_mock.h` | Mock 用户认证 |
| `key_manager_mock.h` | Mock 密钥生成 |
| `mount_manager_mock.h` | Mock 挂载/卸载 |
| `fscrypt_control_mock.h` | Mock fscrypt 安装/移除 |
| `file_utils_mock.h` | Mock 文件操作 |
| `disk_utils_mock.h` | Mock 磁盘信息 |

## 测试位置

| 模块 | 路径 |
|------|------|
| 密钥管理 | `services/storage_daemon/crypto/test/key_manager_test/` |
| 用户管理 | `services/storage_daemon/user/test/` |
| 卷管理 | `services/storage_daemon/volume/test/` |
| 磁盘管理 | `services/storage_daemon/disk/test/` |
| IPC 层 | `services/storage_manager/ipc/test/` |
| Fuzz | `test/fuzztest/` |

## 修改前检查

- 新增功能是否有对应单元测试？
- 非法状态转换是否被覆盖？
- 加密/多用户相关操作是否有板侧验证计划？
