# 代码规范约束

本文只记录命名规则、错误码选用和日志必打节点。枚举引用陷阱见 `enums-and-state-machine.md`，编码约束（50行限制等）见本文件末尾。

## 命名规则

| 类型 | 规则 | 注意 |
|------|------|------|
| 类名 | 大驼峰 | `KeyManager`、`VolumeInfo` |
| 函数名 | 大驼峰 | `GenerateUserKeys`、`ActiveUserKey` |
| 变量名 | 小驼峰 | `userId`、`volumeId` |
| 成员变量 | 下划线结尾或小驼峰 | 源码中两种并存，修改时保持同文件一致 |
| 常量 | `extern const` | `MAX_USER_ID` 类型是 `int` 不是 `int32_t`；`EL1` 类型是 `std::string` 不是 `const char*` |
| 枚举 | 大驼峰类型 + 大写值 | `VolumeState` 是 `enum` 不是 `enum class`；`DiskInfo::Table` 是 `enum class` |

## 错误码选用

禁止使用 `E_ERR`（太笼统），必须使用对应类别的具体错误码：

| 场景 | 必须使用 | 禁止使用 |
|------|----------|----------|
| userId 范围无效 | `E_USERID_RANGE` | E_ERR |
| volumeId/diskId 不存在 | `E_NON_EXIST` | E_ERR |
| volume 状态不允许 | `E_VOL_STATE` | E_ERR |
| 目录创建失败 | `E_CREATE_DIR_RECURSIVE_FAILED` | E_ERR |
| 目录销毁失败 | `E_DESTROY_DIR` | E_ERR |
| 密钥存储失败 | `E_ELX_KEY_STORE_ERROR` | E_ERR |
| 密钥激活失败 | `E_ELX_KEY_ACTIVE_ERROR` | E_ERR |
| 权限不足 | `E_PERMISSION_DENIED` | E_ERR |
| 参数无效 | `E_PARAMS_INVALID` | E_ERR |

## 日志必打节点

以下节点遗漏将导致问题无法定位，必须打印：

| 流程 | 必打节点 | 级别 |
|------|----------|------|
| 用户管理（Prepare/Start/Complete/Stop/Remove） | 开始、成功、失败 | LOGI/LOGE |
| 密钥管理（Generate/Active/Inactive/Lock/Unlock/Update） | 成功、失败 | LOGI/LOGE |
| 卷操作（Mount/Unmount/Format/TryToFix） | 开始、成功、失败 | LOGI/LOGE |
| 磁盘事件（添加/移除） | 设备信息、正常/异常移除 | LOGI/LOGW |
| IPC 连接 | 连接成功/失败、断开 | LOGI/LOGE/LOGW |

敏感信息禁止打印：不可在日志中出现密码、密钥数据、token 内容。只打印长度或状态。

## 编码约束

| 约束项 | 限制 |
|--------|------|
| 单个函数 | ≤ 50 行，超过则拆分 |
| 单个文件 | ≤ 2000 行，超过则按职责拆分 |

## 修改前检查

- 错误码是否用了具体码而非 E_ERR？
- 必打日志节点是否遗漏？
- 函数是否超过 50 行？
