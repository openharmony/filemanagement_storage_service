# 存储管理服务故障上报优化方案

## 背景

存储管理服务中存在大量 `ReportUserKeyResult`、`ReportUserManager`、`ReportFbexResult` 等故障打点。部分打点点位实际记录的是正常系统行为（如设备不支持、首次解锁前操作、查找进程等），而非真正的故障。这些误报会导致故障看板噪声，影响问题定位效率。

本次优化将行为类打点改为 `ReportFucBehavior`（行为打点），并对首次空 token/secret 导致的异常上报增加判断条件，避免误报。

## 打点点位分类

### 一、行为打点（由故障打点改为行为打点）

以下点位同时上报成功和失败结果，属于行为记录而非故障，由 `ReportUserManager` / `ReportUserKeyResult` 改为 `ReportFucBehavior`。

> **注意参数顺序差异**
> - `ReportUserManager(funcName, userId, ret, extraData)`
> - `ReportFucBehavior(funcName, userId, extraData, ret)`
> 转换时 `ret` 与 `extraData` 需交换位置。

#### 1. CreateUserDir（2 处）

| 文件 | 行号 | 修改前 | 修改后 |
|------|------|--------|--------|
| `services/storage_daemon/user/src/user_manager.cpp` | ~290 | `ReportUserManager("CreateUserDir", 0, ret, extraData)` | `ReportFucBehavior("CreateUserDir", 0, extraData, ret)` |
| `services/storage_manager/ipc/src/storage_manager_provider.cpp` | ~1634 | `ReportUserManager("CreateUserDir", 0, ret, extraData)` | `ReportFucBehavior("CreateUserDir", 0, extraData, ret)` |

#### 2. FindSaFd（1 处）

| 文件 | 行号 | 修改前 | 修改后 |
|------|------|--------|--------|
| `services/storage_daemon/user/src/mount_manager.cpp` | ~646 | `ReportUserManager("FindSaFd", userId, E_UMOUNT_FIND_FD, extraData)` | `ReportFucBehavior("FindSaFd", userId, extraData, E_OK)` |

#### 3. FindAndKillProcess（1 处）

| 文件 | 行号 | 修改前 | 修改后 |
|------|------|--------|--------|
| `services/storage_daemon/user/src/mount_manager.cpp` | ~765 | `ReportUserManager("FindAndKillProcess", userId, E_UMOUNT_FIND_PROCESS, extraData)` | `ReportFucBehavior("FindAndKillProcess", userId, extraData, E_UMOUNT_FIND_PROCESS)` |

#### 4. RestoreKey - candidate 为空（1 处）

| 文件 | 行号 | 修改前 | 修改后 |
|------|------|--------|--------|
| `services/storage_daemon/crypto/src/base_key.cpp` | ~596 | `ReportUserKeyResult("BaseKey::RestoreKey", 0, 0, "", "candidate is empty")` | `ReportFucBehavior("BaseKey::RestoreKey", 0, "candidate is empty", 0)` |

#### 5. GenerateAppkey / DeleteAppkey - UECE 不支持（2 处）

| 文件 | 行号 | 修改前 | 修改后 |
|------|------|--------|--------|
| `services/storage_daemon/crypto/src/key_manager.cpp` | ~2018 | 无打点，仅 LOGI | 新增 `ReportFucBehavior("GenerateAppkey", userId, "UECE not supported", -ENOTSUP)` |
| `services/storage_daemon/crypto/src/key_manager.cpp` | ~2101 | 无打点，仅 LOGI | 新增 `ReportFucBehavior("DeleteAppkey", user, "UECE not supported", -ENOTSUP)` |

### 二、条件抑制打点（空 token/secret 时不上报故障）

首次解锁前，用户 token 和 secret 均为空，此时执行的密钥操作产生的 ioctl 失败属于正常现象，不应作为故障上报。在这些点增加 `authToken.IsEmpty()` / `token.empty()` / `secret.empty()` 判断，空值时跳过故障上报。

#### 1. ActiveElXUserKey - 密钥恢复/激活失败（2 处）

| 文件 | 行号 | 抑制条件 | 说明 |
|------|------|----------|------|
| `services/storage_daemon/crypto/src/key_manager.cpp` | ~1859 | `!token.empty() \|\| !secret.empty()` | keyResult 失败时，空 token/secret 跳过 `ReportUserKeyResult("ActiveElxUserKey", ..., E_RESTORE_KEY_FAILED, ...)` |
| `services/storage_daemon/crypto/src/key_manager.cpp` | ~1892 | `!token.empty() \|\| !secret.empty()` | ActiveKey 失败时，空 token/secret 跳过 `ReportUserKeyResult("ActiveElxUserKey", ..., E_ELX_KEY_ACTIVE_ERROR, ...)` |

#### 2. RestoreKey - TryRestoreKey 失败（1 处）

| 文件 | 行号 | 抑制条件 | 说明 |
|------|------|----------|------|
| `services/storage_daemon/crypto/src/base_key.cpp` | ~602 | `!auth.token.IsEmpty() \|\| !auth.secret.IsEmpty()` | TryRestoreKey 失败时，空 token/secret 跳过 `ReportUserKeyResult("BaseKey::RestoreKey", ...)` |

#### 3. InstallDoubleDeKeyToKernel - ioctl 失败（2 处）

| 文件 | 行号 | 抑制条件 | 说明 |
|------|------|----------|------|
| `services/storage_daemon/crypto/src/fbex.cpp` | ~354 | `!authToken.IsEmpty()` | `FBEX_IOC_ADD_DOUBLE_DE_IV` ioctl 失败(EL1)时，空 authToken 跳过 `ReportFbexResult("InstallDoubleDeKeyToKernel", ...)` |
| `services/storage_daemon/crypto/src/fbex.cpp` | ~679 | `hasAuth`（由 `!authToken.IsEmpty()` 传入） | `HandleIoctlError` 中 `FBEX_READ_CLASS_E` ioctl 失败(EL5)时，空 authToken 跳过 `ReportFbexResult`，保留 LOGE |

**HandleIoctlError 签名变更：**

```
// 修改前
static void HandleIoctlError(int ret, int errnoVal, const std::string &cmd,
                             uint32_t userIdSingle, uint32_t userIdDouble);

// 修改后（新增 hasAuth 参数，默认 true 保持兼容）
static void HandleIoctlError(int ret, int errnoVal, const std::string &cmd,
                             uint32_t userIdSingle, uint32_t userIdDouble, bool hasAuth = true);
```

调用方 `ReadESecretToKernel` 传入 `!authToken.IsEmpty()`。

### 三、条件抑制打点（设备不支持时不上报故障）

#### 1. CreateVirtualDirs - 内核不支持（1 处）

| 文件 | 行号 | 抑制条件 | 说明 |
|------|------|----------|------|
| `services/storage_daemon/user/src/mount_manager.cpp` | ~792 | `savedErrno != ENOSYS` | errno 为 ENOSYS（功能未实现）时跳过 `ReportUserManager("CreateVirtualDirs", ...)` |

### 四、不修改的点位

以下 `InstallDoubleDeKeyToKernel` 相关打点点位不属于空 token/secret 场景，不修改：

| 文件 | 行号 | 点位 | 不修改原因 |
|------|------|------|------------|
| `services/storage_daemon/crypto/src/fbex.cpp` | ~320 | iv 为空/无效 | 参数校验失败，与 token/secret 无关 |
| `services/storage_daemon/crypto/src/fbex.cpp` | ~328 | open 设备失败 | 设备节点问题，与 token/secret 无关 |
| `services/storage_daemon/crypto/src/fbex.cpp` | ~801 | WriteESecretToKernel ioctl 失败(EL5) | 仅在 auth 有效时才被调用（`DecryptClassE` 在空 token/secret 时提前返回），无需抑制 |

### 五、其他修改

#### DeleteAppkey - el5Key 为 null 时的判断条件优化

| 文件 | 行号 | 修改前 | 修改后 | 说明 |
|------|------|--------|--------|------|
| `services/storage_daemon/crypto/src/key_manager.cpp` | ~2106 | `if (HashElxActived(user, EL5_KEY))` | `std::string keyDir = GetKeyDirByUserAndType(user, EL5_KEY); if (!IsDir(keyDir))` | 用目录是否存在替代密钥是否激活，更准确判断是否需要上报 |

## 修改文件清单

| 文件 | 修改类型 |
|------|----------|
| `services/storage_daemon/crypto/src/base_key.cpp` | 行为打点 + 条件抑制 |
| `services/storage_daemon/crypto/src/key_manager.cpp` | 行为打点 + 条件抑制 + 判断条件优化 |
| `services/storage_daemon/crypto/src/fbex.cpp` | 条件抑制 + HandleIoctlError 签名变更 |
| `services/storage_daemon/include/crypto/fbex.h` | HandleIoctlError 声明变更 |
| `services/storage_daemon/user/src/mount_manager.cpp` | 行为打点 + 条件抑制 |
| `services/storage_daemon/user/src/user_manager.cpp` | 行为打点 |
| `services/storage_manager/ipc/src/storage_manager_provider.cpp` | 行为打点 |

## 提交记录

| Commit | 说明 |
|--------|------|
| `82b2401af` | merge: resolve conflicts with master |
| `0803b4311` | fix: optimize fault reporting to reduce false reports |
| `69ac0ba75` | fix: change CreateUserDir report from fault to behavior |
| `9e4a49384` | fix: change FindAndKillProcess report from fault to behavior |
| `91df666a8` | fix: skip fault report when authToken is empty in InstallDoubleDeKeyToKernel |

## 验证项

- [ ] 首次解锁前的密钥操作（GenerateAppkey/DeleteAppkey/ActiveElXUserKey/RestoreKey/InstallDoubleDeKeyToKernel）不上报故障
- [ ] 设备不支持场景（UECE 不支持、ENOSYS）不上报故障
- [ ] 行为打点（CreateUserDir/FindSaFd/FindAndKillProcess/RestoreKey candidate 为空）正常上报行为
- [ ] 真正的故障仍然正常上报
- [ ] 不影响现有功能逻辑
