# 代码规范

本文记录命名、错误码、日志、注释和编码约束。

## 命名规范

```cpp
// 类名：大驼峰
class KeyManager { };
class VolumeInfo { };

// 函数名：大驼峰
int32_t GenerateUserKeys(uint32_t userId, uint32_t flags);
void ActiveUserKey(uint32_t userId);

// 变量名：小驼峰（与源码一致）
int32_t userId = 0;
std::string volumeId = "";

// 成员变量：下划线结尾或小驼峰（源码中两种风格并存）
std::mutex keyMutex_;
std::map<std::string, std::shared_ptr<DiskInfo>> diskMap_;
int32_t userId_ = 0;           // 部分成员变量也用下划线结尾

// 常量：extern const（定义在 storage_service_constant.h）
extern const int MAX_USER_ID;       // 注意：类型是 int，不是 int32_t
extern const std::string EL1;       // 注意：类型是 std::string，不是 const char*
extern const int START_USER_ID;

// 枚举：大驼峰类型 + 大写值（注意：VolumeState 是 enum，不是 enum class）
enum KeyType {
    EL0_KEY = 0,
    EL1_KEY = 1,
    EL2_KEY = 2,
};

// 枚举类：DiskInfo 内嵌枚举使用 enum class
enum class Table {
    UNKNOWN,
    MBR,
    GPT,
};
```

## 错误码体系

错误码定义在 `interfaces/innerkits/storage_manager/native/storage_service_errno.h`：

| 错误码范围 | 类别 | 典型错误码 |
|------------|------|------------|
| `E_OK = 0` | 成功 | 操作成功返回 |
| `E_ERR = -1` | 通用失败 | 未分类错误（尽量不用） |
| 13600001-13600200 | 通用错误 | `E_PERMISSION_DENIED`, `E_PARAMS_INVALID`, `E_NON_EXIST`, `E_SA_IS_NULLPTR` |
| 13600201-13600700 | 密钥管理 | `E_USERID_RANGE`, `E_ELX_KEY_STORE_ERROR`, `E_ELX_KEY_ACTIVE_ERROR`, `E_UNLOCK_SCREEN_FAILED` |
| 13600701-13601200 | 用户管理 | `E_USER_MOUNT_ERR`, `E_USER_UMOUNT_ERR`, `E_CREATE_DIR_RECURSIVE_FAILED` |
| 13601201-13601700 | 空间统计 | `E_BUNDLEMGR_ERROR`, `E_SET_QUOTA_UID_FAILED` |
| 13601701-13602200 | 外卡管理 | `E_VOL_MOUNT_ERR`, `E_VOL_UMOUNT_ERR`, `E_VOL_STATE`, `E_MTP_MOUNT_FAILED`, `E_TIMEOUT_MOUNT` |

### 错误码选用规则

**必须使用对应类别的具体错误码，禁止使用 E_ERR（太笼统）**：

| 场景 | 推荐错误码 | 禁止使用 |
|------|------------|----------|
| userId 无效（范围错误） | `E_USERID_RANGE` | `E_ERR` |
| volumeId/diskId 不存在 | `E_NON_EXIST` | `E_ERR` |
| volume 状态不允许操作 | `E_VOL_STATE` | `E_ERR` |
| 目录创建失败 | `E_CREATE_DIR_RECURSIVE_FAILED` | `E_ERR` |
| 目录销毁失败 | `E_DESTROY_DIR` | `E_ERR` |
| 密钥存储失败 | `E_ELX_KEY_STORE_ERROR` | `E_ERR` |
| 密钥激活失败 | `E_ELX_KEY_ACTIVE_ERROR` | `E_ERR` |
| 密钥更新失败 | `E_ELX_KEY_UPDATE_ERROR` | `E_ERR` |
| 密钥停用失败 | `E_ELX_KEY_INACTIVE_ERROR` | `E_ERR` |
| 解锁屏幕失败 | `E_UNLOCK_SCREEN_FAILED` | `E_ERR` |
| 用户挂载失败 | `E_USER_MOUNT_ERR` | `E_ERR` |
| 用户卸载失败 | `E_USER_UMOUNT_ERR` | `E_ERR` |
| 卷挂载失败 | `E_VOL_MOUNT_ERR` | `E_ERR` |
| 卷卸载失败 | `E_VOL_UMOUNT_ERR` | `E_ERR` |
| 卷检查失败 | `E_CHECK` | `E_ERR` |
| 挂载超时 | `E_TIMEOUT_MOUNT` | `E_ERR` |
| 权限不足 | `E_PERMISSION_DENIED` | `E_ERR` |
| 参数无效 | `E_PARAMS_INVALID` | `E_ERR` |
| 服务指针为空 | `E_SA_IS_NULLPTR` | `E_ERR` |
| 不支持的文件系统 | `E_NOT_SUPPORT` (13600007) | `E_UNSUPPORTED` (13601721, 外卡管理类别)、`E_ERR` |
| 分区操作失败 | `E_GET_PARTITION_ERROR`, `E_CREATE_PARTITION_ERROR`, `E_DELETE_PARTITION_ERROR`, `E_FORMAT_PARTITION_ERROR` | `E_ERR` |

```cpp
// 错误码使用示例
int32_t KeyManager::GenerateUserKeys(uint32_t userId, uint32_t flags) {
    if (userId < START_USER_ID || userId > MAX_USER_ID) {
        LOGE("Invalid userId: %{public}d", userId);
        return E_USERID_RANGE;  // 使用密钥管理类别的错误码
    }
    // ...
    return E_OK;
}

// 错误必须上报 StorageRadar
StorageRadar::ReportUserKeyResult("GenerateUserKeys", userId, ret, keyElxLevel, "");
```

## 日志规范

### 日志级别使用场景

```cpp
LOGI("User %{public}d started successfully", userId);      // 正常流程关键节点
LOGW("Retry mount for volume %{public}s", volumeId.c_str()); // 异常但可恢复
LOGE("Failed to generate key for user %{public}d: %{public}d", userId, ret); // 错误
LOGD("Volume state: %{public}d", state);                    // 仅调试时
```

### 敏感信息禁止打印

```cpp
// 错误：LOGI("User password: %{public}s", password);  // 禁止！
// 正确：LOGI("User auth token received, length: %{public}zu", token.size());
```

### 关键日志节点清单

以下节点**必须打印日志**，遗漏将导致问题无法定位：

| 流程 | 必打节点 | 级别 | 格式模板 |
|------|----------|------|----------|
| **用户管理** | | | |
| PrepareAddUser | 开始、成功、失败 | LOGI/LOGE | `"Prepare user %{public}d, flags %{public}u"` |
| StartUser | 开始、成功、失败 | LOGI/LOGE | `"Start user %{public}d"` |
| CompleteAddUser | 成功、失败 | LOGI/LOGE | `"Complete user %{public}d success"` |
| StopUser | 成功、失败 | LOGI/LOGE | `"Stop user %{public}d success"` |
| RemoveUser | 成功、失败 | LOGI/LOGE | `"Remove user %{public}d success"` |
| **密钥管理** | | | |
| GenerateUserKeys | 开始、成功、失败 | LOGI/LOGE | `"Generate keys for user %{public}d"` |
| ActiveUserKey | 成功、失败 | LOGI/LOGE | `"Active %{public}s key for user %{public}d success"` |
| InactiveUserKey | 成功、失败 | LOGI/LOGE | `"Inactive user %{public}d key success"` |
| LockUserScreen | 成功、失败 | LOGI/LOGE | `"Lock screen for user %{public}d success"` |
| UnlockUserScreen | 成功、失败 | LOGI/LOGE | `"Unlock screen for user %{public}d success"` |
| UpdateUserAuth | 成功、失败 | LOGI/LOGE | `"Update auth for user %{public}d success"` |
| **卷管理** | | | |
| Mount | 开始、成功、失败 | LOGI/LOGE | `"Mount volume %{public}s"` |
| Unmount | 成功、失败 | LOGI/LOGE | `"Unmount volume %{public}s success"` |
| Format | 开始、成功、失败 | LOGI/LOGE | `"Format volume %{public}s to %{public}s"` |
| TryToFix | 开始、成功、失败 | LOGI/LOGE | `"TryToFix volume %{public}s"` |
| **磁盘管理** | | | |
| 设备添加 | 设备信息 | LOGI | `"Disk %{public}s added, type %{public}d"` |
| 设备移除 | 正常/异常移除 | LOGI/LOGW | `"Disk %{public}s removed"` / `"Disk %{public}s bad removal"` |
| **IPC 层** | | | |
| 连接建立 | 成功、失败 | LOGI/LOGE | `"Connect to Daemon success"` |
| 连接断开 | 断开原因 | LOGW | `"Daemon disconnected"` |

```cpp
// 正确的日志打印示例
int32_t UserManager::PrepareUserDirs(uint32_t userId) {
    LOGI("Prepare user dirs for %{public}d", userId);  // 必打：开始
    
    if (userId < START_USER_ID || userId > MAX_USER_ID) {
        LOGE("Invalid userId %{public}d", userId);  // 必打：参数错误
        return E_USERID_RANGE;
    }
    
    int32_t ret = CreateEl1Dirs(userId);
    if (ret != E_OK) {
        LOGE("Create EL1 dirs failed for user %{public}d, ret %{public}d", userId, ret);  // 必打：失败
        return E_CREATE_DIR_RECURSIVE_FAILED;
    }
    
    LOGI("Prepare user dirs for %{public}d success", userId);  // 必打：成功
    return E_OK;
}
```

## 注释规范

```cpp
// 函数注释：描述、参数、返回值
/**
 * @brief Generate and install encryption keys for a user
 * @param userId User identifier (100-10738)
 * @param flags Bitmask of CRYPTO_FLAG_EL* indicating which EL keys to generate
 * @return E_OK on success, error code on failure
 */
int32_t GenerateUserKeys(uint32_t userId, uint32_t flags);

// 行内注释：解释"为什么"而非"是什么"
// Delay key removal to allow ongoing operations to complete
std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
```

## 编码约束

| 约束项 | 限制 | 说明 |
|--------|------|------|
| 单个函数 | ≤ 50 行 | 超过则拆分为多个子函数，每个子函数职责单一 |
| 单个文件 | ≤ 2000 行 | 超过则按职责拆分为多个文件，头文件声明与实现分离 |

```cpp
// 错误：单个函数超过 50 行
int32_t ProcessUserOperation(uint32_t userId) {
    // 80 行代码...
}

// 正确：拆分为多个子函数
int32_t ProcessUserOperation(uint32_t userId) {
    int32_t ret = PrepareUser(userId);
    if (ret != E_OK) {
        return ret;
    }
    ret = ExecuteUserAction(userId);
    if (ret != E_OK) {
        CleanupUser(userId);
        return ret;
    }
    return CompleteUser(userId);
}
```