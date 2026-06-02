# 存储管理服务知识库

本仓库对应 OpenHarmony `foundation/filemanagement/storage_service`，提供外部存储挂载、文件加密/解密、磁盘/卷管理、用户目录管理和空间统计。

## 知识库结构

```
AGENTS.md                        # 入口索引（本文档）
│
├─ docs/knowledge/               # 知识库目录
│   ├─ architecture.md           # 架构概览
│   ├─ code-conventions.md       # 代码规范
│   ├─ implementation-templates.md # 实现模板
│   ├─ enums-and-state-machine.md # 枚举与状态机
│   ├─ ipc-interface-guide.md    # IPC 接口指南
│   ├─ testing-guide.md          # 测试指南
│   ├─ constraints-and-traps.md  # 约束与陷阱
│   ├─ debugging-guide.md        # 调试指引
│   ├─ external-dependencies.md  # 外部依赖
│   ├─ user-directory-lifecycle.md   # 用户目录生命周期（场景知识）
│   ├─ disk-volume-lifecycle.md      # 磁盘卷生命周期（场景知识）
│   ├─ crypto-key-management.md      # 密钥管理（场景知识）
│   └─ storage-quota-monitoring.md   # 空间统计（场景知识）
```

## 快速索引

### 架构与规范

| 需要了解 | 知识文件 |
|----------|----------|
| 目录职责、头文件路径、继承关系 | `docs/knowledge/architecture.md` |
| 命名规范、错误码、日志、编码约束 | `docs/knowledge/code-conventions.md` |
| VolumeState、DiskType、KeyType 枚举 | `docs/knowledge/enums-and-state-machine.md` |
| IPC 接口、代码放置位置 | `docs/knowledge/ipc-interface-guide.md` |

### 开发实操

| 需要编写 | 知识文件 |
|----------|----------|
| Daemon 侧卷/密钥/用户操作 | `docs/knowledge/implementation-templates.md` |
| Manager 侧 IPC 入口/Stub | `docs/knowledge/implementation-templates.md` |
| 新增文件系统 Operator | `docs/knowledge/implementation-templates.md` |
| 单元测试、Mock | `docs/knowledge/testing-guide.md` |

### 约束与陷阱

| 需要遵守 | 知识文件 |
|----------|----------|
| 时序约束、数据隔离、线程安全 | `docs/knowledge/constraints-and-traps.md` |
| 6 个常见陷阱 | `docs/knowledge/constraints-and-traps.md` |
| 安全规范 | `docs/knowledge/constraints-and-traps.md` |

### 场景知识

| 开发场景 | 知识文件 |
|----------|----------|
| 用户添加/删除、目录创建/销毁 | `docs/knowledge/user-directory-lifecycle.md` |
| 磁盘热插拔、分区、卷状态机 | `docs/knowledge/disk-volume-lifecycle.md` |
| EL1-EL5 加密、锁屏/解锁 | `docs/knowledge/crypto-key-management.md` |
| Manager-Daemon IPC、崩溃恢复 | `docs/knowledge/ipc-interface-guide.md` |
| 空间统计、配额管理 | `docs/knowledge/storage-quota-monitoring.md` |

### 调试与依赖

| 需要排查 | 知识文件 |
|----------|----------|
| 日志查看、状态检查、问题定位 | `docs/knowledge/debugging-guide.md` |
| HUKS、TEE、编译宏影响 | `docs/knowledge/external-dependencies.md` |

## 快速参考

### 错误码范围

| 范围 | 类别 |
|------|------|
| E_OK = 0 | 成功 |
| E_ERR = -1 | 通用失败（尽量不用） |
| 13600001-13600200 | 通用错误 |
| 13600201-13600700 | 密钥管理 |
| 13600701-13601200 | 用户管理 |
| 13601201-13601700 | 空间统计 |
| 13601701-13602200 | 外卡管理 |

**规则：必须使用具体错误码，禁止使用 E_ERR**

### 核心枚举

```cpp
// VolumeState（13 个状态）
UNMOUNTED, CHECKING, MOUNTED, EJECTING, REMOVED, BADREMOVABLE,
DAMAGED, FUSE_REMOVED, DAMAGED_MOUNTED, ENCRYPTING,
ENCRYPTED_AND_LOCKED, ENCRYPTED_AND_UNLOCKED, DECRYPTING

// KeyType（6 个等级）
EL0_KEY(设备级), EL1_KEY(设备级), EL2_KEY(锁屏移除),
EL3_KEY, EL4_KEY, EL5_KEY(UECE)

// 加密标志位
CRYPTO_FLAG_EL1=1, EL2=2, EL3=4, EL4=8, EL5=16
```

### 核心约束

1. **用户时序**：PrepareAddUser → StartUser → CompleteAddUser（不可跳步）
2. **卷状态**：MOUNTED → EJECTING → REMOVED（必须经过 EJECTING）
3. **EL2 密钥**：锁屏移除，解锁安装（锁屏时不可访问 EL2 数据）
4. **用户隔离**：不可用 userId=100 的密钥操作 userId=105 的目录
5. **IPC 死锁**：不要在持有锁时调用 Daemon IPC

### 单例获取

```cpp
KeyManager::GetInstance()          // GetInstance
UserManager::GetInstance()         // GetInstance
QuotaManager::GetInstance()        // GetInstance
HuksMaster::GetInstance()          // GetInstance
StorageRadar::GetInstance()        // GetInstance
DiskManager::Instance()            // Instance（不是 GetInstance）
VolumeManager::Instance()          // Instance（不是 GetInstance）
DelayedSingleton<StorageDaemonCommunication>::GetInstance() // DelayedSingleton
```

## 构建命令

```sh
# 构建 Daemon
./build.sh --product-name rk3568 --build-target storage_daemon --ccache

# 构建 Manager
./build.sh --product-name rk3568 --build-target storage_manager --ccache

# 构建测试
./build.sh --product-name rk3568 --build-target storage_daemon_test --ccache
```

## 验证约束

- 涉及真实磁盘、加密、多用户或配额的行为，**需要板侧验证**
- 提交使用 `git commit -s`（签署 DCO）
- 变更需通过单元测试和 fuzz 测试