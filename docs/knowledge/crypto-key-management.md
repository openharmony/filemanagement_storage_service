# 加密密钥管理约束

本文只记录密钥操作的时序约束、激活规则和版本兼容边界。用户目录时序见 `user-directory-lifecycle.md`，编译宏行为差异见 `external-dependencies.md`。

## 密钥激活时序约束

| KeyType | 激活时机 | 不可访问条件 | 不可跳过的依赖 |
|---------|----------|--------------|----------------|
| EL0 | 设备启动 | 无 | 无 |
| EL1 | 设备启动 + StartUser | StartUser 未完成时 | 依赖目录已创建 |
| EL2 | CompleteAddUser / UnlockUserScreen | **锁屏时密钥已从内核移除** | 依赖 EL1 已激活 |
| EL3 | CompleteAddUser | CompleteAddUser 未完成时 | 依赖 EL2 已激活 |
| EL4 | CompleteAddUser | CompleteAddUser 未完成时 | 依赖 EL2 已激活 |
| EL5 | UECE 回调 | 回调未触发时 | 依赖用户密钥已激活 |

密钥激活是链式依赖：EL1 未激活则 EL2 不可激活，EL2 未激活则 EL3/EL4 不可激活。不要跳步激活。

## 锁屏/解锁约束

- 锁屏：EL2 密钥从内核 keyring 移除，此后不可访问 `/data/service/el2/{userId}` 下任何文件。
- 解锁：EL2 密钥重新安装到内核，恢复对 EL2 目录的访问。
- 不要在 `LockUserScreen` 后尝试读取 EL2 加密数据，必须等 `UnlockUserScreen` 完成。

## 版本兼容边界

| 特性 | V1 | V2 |
|------|----|----|
| 密钥格式 | 传统格式 | 新格式 |
| HUKS 调用方式 | 直接调用 | 通过 HDI 调用（`HUKS_IDL_ENVIRONMENT`） |
| 密钥恢复 | 简单恢复 | TEE 环境恢复（`RECOVER_KEY_TEE_ENVIRONMENT`） |

新增密钥操作时必须同时考虑 V1 和 V2 的行为差异，不要只测试当前版本。

## App 密钥约束

App 密钥（`GenerateAppkey`）依赖对应用户的 EL2 密钥已激活。用户密钥未激活时生成 App 密钥会失败，不要在 StartUser 之前调用。

## 密钥操作失败上报

所有密钥操作失败必须通过 `StorageRadar` 上报，不要吞掉错误。上报 API 使用 `ReportUserKeyResult` 和 `ReportActiveUserKey`，不是 `ErrorReport`。

## 修改前检查

- 当前密钥等级是什么？前置等级是否已激活？
- 锁屏状态？EL2 密钥是否在内核？
- V1/V2 版本是否都考虑了？
- 失败是否上报了 StorageRadar？

## 测试指引

- 密钥激活/停用：`KeyManagerTest`，必须覆盖锁屏/解锁场景
- 版本兼容：`FscryptKeyV1Test`、`FscryptKeyV2Test`
- 涉及真实 HUKS/TEE：需要板侧验证
