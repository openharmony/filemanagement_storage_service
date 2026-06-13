# 外部依赖约束

本文只记录编译宏对代码行为的影响和依赖不可用时的处理规则。密钥操作依赖见 `crypto-key-management.md`，HUKS 封装见源码 `crypto/huks_master.h`。

## 编译宏行为差异

| 编译宏 | 影响范围 | 行为差异 | 不考虑的后果 |
|--------|----------|----------|--------------|
| `USER_CRYPTO_MIGRATE_KEY` | UpdateUserAuth | 增加 `needGenerateShield` 参数 | 只写2参数版本，宏定义时编译失败 |
| `RECOVER_KEY_TEE_ENVIRONMENT` | RecoverManager | 恢复密钥使用 TEE 环境 | 非 TEE 环境下调用了 TEE 专属接口 |
| `EL5_FILEKEY_MANAGER` | KeyManager | UECE 回调注册功能 | 不处理 UECE 导致 EL5 密钥未激活 |
| `USER_AUTH_FRAMEWORK` | IAMClient | 使用 `userauth_client.h` 代替 `iam_client.h` | include 了不存在的头文件 |
| `HUKS_IDL_ENVIRONMENT` | HuksMaster | HUKS 使用 HDI 1.1 接口 | 链接了错误的 HUKS 库 |
| `SUPPORT_RECOVERY_KEY_SERVICE` | KeyManager | 恢复密钥服务支持 | 未实现恢复密钥流程 |

新增代码涉及上述宏影响的功能时，必须同时考虑宏定义和未定义两种行为，不要只写一种。

## 依赖不可用时的处理规则

| 依赖 | 检查方法 | 不可用时返回 | 不可用时不可做 |
|------|----------|--------------|----------------|
| HUKS | `HuksMaster::GetInstance().IsHuksAvailable()` | `E_GLOBAL_KEY_INIT_ERROR` | 不要继续生成/激活密钥 |
| OsAccount | `OsAccountManager::IsOsAccountExists()` | `E_SA_IS_NULLPTR` | 不要操作不存在用户的目录 |
| AccessToken | `AccessTokenKit::GetTokenType()` | `E_PERMISSION_DENIED` | 不要跳过权限校验 |
| BundleFramework | 检查连接状态 | `E_GET_UID_ERROR` 或 `E_BUNDLEMGR_ERROR` | 不要假设应用 UID 可查 |

依赖不可用时必须返回具体错误码并上报 StorageRadar，不要静默忽略或返回 E_ERR。

## 依赖初始化顺序

1. Daemon 启动：`InitGlobalDeviceKey()` → 检查 HUKS → 加载 EL1 密钥 → 启动 Netlink
2. Manager 启动：懒连接 Daemon → 启动监控 → 订阅用户事件

不要假设 Manager 启动时所有依赖已就绪。

## 修改前检查

- 涉及的编译宏是否考虑了定义/未定义两种行为？
- 依赖不可用时是否返回了具体错误码？
- 新增依赖是否添加了可用性检查？
