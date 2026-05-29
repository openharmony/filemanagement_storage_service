---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-29
---
# 存储管理服务知识库

本仓库对应 OpenHarmony `foundation/filemanagement/storage_service`，提供外部存储挂载、文件加密/解密、磁盘/卷管理、用户目录管理和空间统计。

## 空间统计子系统定位

storage_manager 中的空间统计子系统负责存储空间的查询、统计、监控和上报。它不是通信层也不是存储层，而是**存储空间信息的统计与监控编排层**。

核心能力包括：

- 应用维度空间查询（appSize/cacheSize/dataSize）
- 用户维度分类统计（audio/video/image/file/app）
- 系统级总量/可用量查询
- 外置卷设备空间查询
- 磁盘空间扫描（quota + 目录遍历）
- 存储空间监控与自动清理
- DFX 上报（雷达打点、HiSysEvent）

### 能力矩阵

| 能力域 | 具体功能 | 关键类 |
|--------|----------|--------|
| 应用空间查询 | 查询指定应用的 appSize/cacheSize/dataSize | StorageStatusManager |
| 用户分类统计 | 按 audio/video/image/file/app 维度统计用户存储 | StorageStatusManager |
| 总量/可用量 | 查询内置存储和外置卷的总量与可用量 | StorageTotalStatusService, VolumeStorageStatusService |
| 磁盘扫描 | 通过 quota + 目录遍历统计 root/system/memmgr 占用 | StorageManagerScan |
| 存储监控 | 60秒周期监控，三级阈值告警，自动清理缓存 | StorageMonitorService |
| DFX上报 | HAP/SA统计、目录统计、扫描结果上报到雷达平台 | StorageDfxReporter |

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
│
├─ docs/storage-statistics/       # 空间统计子系统知识库
│   ├─ 01-architecture.md         # 空间统计架构总览
│   ├─ 02-core-concepts.md        # 核心概念与术语
│   ├─ 03-data-structures.md      # 数据结构与字段语义
│   ├─ 04-workflows/              # 工作流文档
│   ├─ 05-interfaces/             # 接口参考
│   ├─ 06-debugging.md            # 调试与排障
│   └─ 08-faq.md                  # 常见问题
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

### 空间统计子系统

| 需要了解 | 知识文件 |
|----------|----------|
| 空间统计整体架构和模块分层 | `docs/storage-statistics/01-architecture.md` |
| BundleStats、StorageStats、quota、inode 等术语 | `docs/storage-statistics/02-core-concepts.md` |
| BundleStats、StorageStats、NextDqBlk 等结构体 | `docs/storage-statistics/03-data-structures.md` |
| 应用空间统计查询逻辑 | `docs/storage-statistics/04-workflows/01-app-storage-query.md` |
| 用户分类存储统计逻辑 | `docs/storage-statistics/04-workflows/02-user-category-stats.md` |
| 总量/可用量查询逻辑 | `docs/storage-statistics/04-workflows/03-total-free-query.md` |
| 磁盘扫描逻辑或扫描路径 | `docs/storage-statistics/04-workflows/04-disk-scan.md` |
| 存储监控阈值或缓存清理策略 | `docs/storage-statistics/04-workflows/05-storage-monitor-clean.md` |
| DFX/日志/雷达打点 | `docs/storage-statistics/04-workflows/06-dfx-report.md` |
| JS公共API接口 | `docs/storage-statistics/05-interfaces/01-js-public-api.md` |
| JS系统API接口 | `docs/storage-statistics/05-interfaces/02-js-system-api.md` |
| 模块内部及IPC接口 | `docs/storage-statistics/05-interfaces/03-inner-api.md` |
| 日志查看、状态检查、问题定位 | `docs/storage-statistics/06-debugging.md` |
| 空间统计常见问题 | `docs/storage-statistics/08-faq.md` |

### 空间统计术语表

| 术语 | 全称 | 含义 |
|------|------|------|
| storage_manager | Storage Manager | 存储管理SA服务，负责卷/磁盘/空间统计/加密等 |
| storage_daemon | Storage Daemon | 存储守护进程，执行底层操作（挂载/quota查询/目录扫描） |
| StorageStatusManager | Storage Status Manager | 空间统计管理器，提供应用和用户维度的空间查询 |
| StorageTotalStatusService | Storage Total Status Service | 总量统计服务，查询系统级总空间/可用空间/inode |
| StorageMonitorService | Storage Monitor Service | 存储监控服务，60秒周期监控，三级阈值告警与自动清理 |
| StorageManagerScan | Storage Manager Scan | 磁盘扫描服务，通过quota+目录遍历统计root/system/memmgr占用 |
| StorageDfxReporter | Storage DFX Reporter | DFX上报器，统计HAP/SA/目录空间并上报到雷达平台 |
| BundleStats | Bundle Statistics | 应用空间统计结果（appSize/cacheSize/dataSize） |
| StorageStats | Storage Statistics | 用户存储分类统计结果（total/audio/video/image/file/app） |
| quota | Disk Quota | 磁盘配额，Linux内核提供的按UID限制磁盘空间的机制 |
| inode | Index Node | 文件系统索引节点，每个文件/目录占用一个inode |
| HMFS | Harmony Mobile File System | OpenHarmony文件系统类型 |
| DFX | Design for eXcellence | 面向质量的设计，包括日志/打点/故障检测等 |
| HiSysEvent | HiSysEvent | OpenHarmony系统事件打点框架 |
| SA | System Ability | OpenHarmony系统服务能力 |
| IPC | Inter-Process Communication | 进程间通信 |
| FUSE | Filesystem in Userspace | 用户态文件系统 |

### 空间统计文档索引

**基础文档：**
- [架构总览](docs/storage-statistics/01-architecture.md) — 模块在系统中的定位、内部分层、外部依赖关系
- [核心概念](docs/storage-statistics/02-core-concepts.md) — 空间统计领域术语、概念对比、业务含义
- [数据结构](docs/storage-statistics/03-data-structures.md) — 关键结构体、枚举、字段语义、生命周期

**工作流文档：**
- [应用空间统计查询](docs/storage-statistics/04-workflows/01-app-storage-query.md) — getCurrentBundleStats/getBundleStats 查询应用占用的完整流程
- [用户分类存储统计](docs/storage-statistics/04-workflows/02-user-category-stats.md) — getUserStorageStats 按媒体类型分类统计的流程
- [总量与可用量查询](docs/storage-statistics/04-workflows/03-total-free-query.md) — getTotalSize/getFreeSize 查询系统空间总量的流程
- [磁盘扫描统计](docs/storage-statistics/04-workflows/04-disk-scan.md) — StorageManagerScan 扫描磁盘空间占用的完整流程
- [存储监控与自动清理](docs/storage-statistics/04-workflows/05-storage-monitor-clean.md) — StorageMonitorService 周期监控、阈值告警、自动清理流程
- [DFX统计上报](docs/storage-statistics/04-workflows/06-dfx-report.md) — HAP/SA/目录统计上报到雷达平台的流程

**接口文档：**
- [JS公共API](docs/storage-statistics/05-interfaces/01-js-public-api.md) — 第三方应用可用的空间查询接口
- [JS系统API](docs/storage-statistics/05-interfaces/02-js-system-api.md) — 系统应用专用的空间统计接口
- [内部接口](docs/storage-statistics/05-interfaces/03-inner-api.md) — storage_manager内部及与storage_daemon的IPC接口

**运维文档：**
- [调试与排障](docs/storage-statistics/06-debugging.md) — 日志标签、雷达打点、故障排查流程
- [常见问题](docs/storage-statistics/08-faq.md) — 按场景分类的常见问题和解决方案

### 空间统计贡献规则

| 代码变更类型 | 需要更新的文档 |
|-------------|--------------|
| 新增/修改JS API | 对应接口文档 + AGENTS.md 术语表 |
| 新增/修改内部接口 | docs/storage-statistics/05-interfaces/03-inner-api.md |
| 修改统计逻辑 | 对应工作流文档 + 数据结构文档 |
| 新增扫描路径 | docs/storage-statistics/04-workflows/04-disk-scan.md |
| 修改阈值参数 | docs/storage-statistics/04-workflows/05-storage-monitor-clean.md |
| 新增DFX上报字段 | docs/storage-statistics/04-workflows/06-dfx-report.md + docs/storage-statistics/06-debugging.md |

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