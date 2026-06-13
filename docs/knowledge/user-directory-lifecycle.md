# 用户目录生命周期约束

本文只记录用户存储操作的时序约束、隔离规则和回滚要求。密钥激活时序见 `crypto-key-management.md`，卷状态约束见 `disk-volume-lifecycle.md`，线程死锁见 `constraints-and-traps.md`。

## 时序约束

用户存储操作必须严格按以下顺序执行，不可跳步或乱序：

| 步骤 | 触发 | 不可跳过的原因 |
|------|------|----------------|
| PrepareAddUser | OsAccount READY | 创建目录和生成 EL1 密钥，后续步骤依赖目录存在 |
| StartUser | OsAccount STARTING | 挂载存储、激活 EL1 密钥，EL2-EL4 封装在此之后才可激活 |
| CompleteAddUser | OsAccount UNLOCKED | 激活 EL2-EL4 密钥，跳过则这些等级的加密数据不可访问 |

`AccountSubscriber` 监听 `OsAccountState` 事件自动触发对应步骤，不要绕过它直接调用 Daemon。

## 失败回滚

目录创建是链式操作（EL1→EL2→EL3→EL4→EL5→权限设置），任何一步失败必须回滚全部已创建目录。密钥生成失败也必须销毁已创建的目录，不要保留半成品状态。

## EL 等级隔离

| 等级 | 路径 | 密钥激活时机 | 不可访问条件 |
|------|------|--------------|--------------|
| EL1 | `/data/service/el1/public/{userId}` | StartUser | 密钥未激活时 |
| EL2 | `/data/service/el2/{userId}` | CompleteAddUser / UnlockUserScreen | **锁屏时不可访问** |
| EL3 | `/data/service/el3/{userId}` | CompleteAddUser | 密钥未激活时 |
| EL4 | `/data/service/el4/{userId}` | CompleteAddUser | 密钥未激活时 |
| EL5 | `/data/app/el5/{userId}` | UECE 回调 | 回调未触发时 |

不可用 EL1 密钥操作 EL2 目录，密钥和目录等级必须匹配。

## 用户隔离

不可用一个用户的密钥或挂载点操作另一个用户的数据路径。userId=100 的密钥只操作 userId=100 的目录，userId=105 的密钥只操作 userId=105 的目录。

## NATO 和 AppClone

- NATO 用户有独立的恢复密钥目录（`el2_NATO/{userId}`），不要与普通用户密钥路径混用。
- AppClone 用户 ID 范围独立，密钥管理使用 `AppCloneKeyManager` 而非 `KeyManager`，不要用 `KeyManager::GetInstance()` 操作 AppClone 用户。

## 修改前检查

- 当前步骤处于时序哪个位置？前置步骤是否已完成？
- 失败时是否回滚了全部已创建状态？
- 密钥等级和目录等级是否匹配？
- 用户 ID 是否与操作目标匹配？

## 测试指引

- 时序覆盖：`UserManagerTest`、`StorageManagerProviderTest`
- 多用户隔离：至少覆盖跨用户密钥/目录操作拒绝场景
- 涉及真实挂载/加密：需要板侧验证
