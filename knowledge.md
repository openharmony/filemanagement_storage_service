---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-29
---
# 空间统计（Storage Statistics）知识库

## 模块定位

storage_manager 中的空间统计子系统负责存储空间的查询、统计、监控和上报。它不是通信层也不是存储层，而是**存储空间信息的统计与监控编排层**。

核心能力包括：

- 应用维度空间查询（appSize/cacheSize/dataSize）
- 用户维度分类统计（audio/video/image/file/app）
- 系统级总量/可用量查询
- 外置卷设备空间查询
- 磁盘空间扫描（quota + 目录遍历）
- 存储空间监控与自动清理
- DFX 上报（雷达打点、HiSysEvent）

## 能力矩阵

| 能力域 | 具体功能 | 关键类 |
|--------|----------|--------|
| 应用空间查询 | 查询指定应用的 appSize/cacheSize/dataSize | StorageStatusManager |
| 用户分类统计 | 按 audio/video/image/file/app 维度统计用户存储 | StorageStatusManager |
| 总量/可用量 | 查询内置存储和外置卷的总量与可用量 | StorageTotalStatusService, VolumeStorageStatusService |
| 磁盘扫描 | 通过 quota + 目录遍历统计 root/system/memmgr 占用 | StorageManagerScan |
| 存储监控 | 60秒周期监控，三级阈值告警，自动清理缓存 | StorageMonitorService |
| DFX上报 | HAP/SA统计、目录统计、扫描结果上报到雷达平台 | StorageDfxReporter |

## 场景速查表

| 我要做的改动 | 先读 |
|-------------|------|
| 理解空间统计整体架构和模块分层 | [架构总览](docs/storage-statistics/01-architecture.md) |
| 搞清 BundleStats、StorageStats、quota、inode 等术语 | [核心概念](docs/storage-statistics/02-core-concepts.md) |
| 查看 BundleStats、StorageStats、NextDqBlk 等结构体定义 | [数据结构](docs/storage-statistics/03-data-structures.md) |
| 改应用空间统计查询逻辑 | [应用空间统计查询](docs/storage-statistics/04-workflows/01-app-storage-query.md) |
| 改用户分类存储统计逻辑 | [用户分类存储统计](docs/storage-statistics/04-workflows/02-user-category-stats.md) |
| 改总量/可用量查询逻辑 | [总量与可用量查询](docs/storage-statistics/04-workflows/03-total-free-query.md) |
| 改磁盘扫描逻辑或扫描路径 | [磁盘扫描统计](docs/storage-statistics/04-workflows/04-disk-scan.md) |
| 改存储监控阈值或缓存清理策略 | [存储监控与自动清理](docs/storage-statistics/04-workflows/05-storage-monitor-clean.md) |
| 改 DFX/日志/雷达打点 | [DFX统计上报](docs/storage-statistics/04-workflows/06-dfx-report.md) |
| 开发上层应用调用空间查询接口 | [JS公共API](docs/storage-statistics/05-interfaces/01-js-public-api.md) |
| 开发系统应用调用空间统计接口 | [JS系统API](docs/storage-statistics/05-interfaces/02-js-system-api.md) |
| 理解模块内部及与 storage_daemon 的 IPC 接口 | [内部接口](docs/storage-statistics/05-interfaces/03-inner-api.md) |
| 排查线上问题 | [调试与排障](docs/storage-statistics/06-debugging.md) |
| 查找常见问题解答 | [常见问题](docs/storage-statistics/08-faq.md) |

## 全局术语表

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

## 文档索引

### 基础文档

- [架构总览](docs/storage-statistics/01-architecture.md) — 模块在系统中的定位、内部分层、外部依赖关系
- [核心概念](docs/storage-statistics/02-core-concepts.md) — 空间统计领域术语、概念对比、业务含义
- [数据结构](docs/storage-statistics/03-data-structures.md) — 关键结构体、枚举、字段语义、生命周期

### 工作流文档

- [应用空间统计查询](docs/storage-statistics/04-workflows/01-app-storage-query.md) — getCurrentBundleStats/getBundleStats 查询应用占用的完整流程
- [用户分类存储统计](docs/storage-statistics/04-workflows/02-user-category-stats.md) — getUserStorageStats 按媒体类型分类统计的流程
- [总量与可用量查询](docs/storage-statistics/04-workflows/03-total-free-query.md) — getTotalSize/getFreeSize 查询系统空间总量的流程
- [磁盘扫描统计](docs/storage-statistics/04-workflows/04-disk-scan.md) — StorageManagerScan 扫描磁盘空间占用的完整流程
- [存储监控与自动清理](docs/storage-statistics/04-workflows/05-storage-monitor-clean.md) — StorageMonitorService 周期监控、阈值告警、自动清理流程
- [DFX统计上报](docs/storage-statistics/04-workflows/06-dfx-report.md) — HAP/SA/目录统计上报到雷达平台的流程

### 接口文档

- [JS公共API](docs/storage-statistics/05-interfaces/01-js-public-api.md) — 第三方应用可用的空间查询接口
- [JS系统API](docs/storage-statistics/05-interfaces/02-js-system-api.md) — 系统应用专用的空间统计接口
- [内部接口](docs/storage-statistics/05-interfaces/03-inner-api.md) — storage_manager内部及与storage_daemon的IPC接口

### 运维文档

- [调试与排障](docs/storage-statistics/06-debugging.md) — 日志标签、雷达打点、故障排查流程
- [常见问题](docs/storage-statistics/08-faq.md) — 按场景分类的常见问题和解决方案

## 贡献规则

| 代码变更类型 | 需要更新的文档 |
|-------------|--------------|
| 新增/修改JS API | 对应接口文档 + knowledge.md 术语表 |
| 新增/修改内部接口 | docs/storage-statistics/05-interfaces/03-inner-api.md |
| 修改统计逻辑 | 对应工作流文档 + 数据结构文档 |
| 新增扫描路径 | docs/storage-statistics/04-workflows/04-disk-scan.md |
| 修改阈值参数 | docs/storage-statistics/04-workflows/05-storage-monitor-clean.md |
| 新增DFX上报字段 | docs/storage-statistics/04-workflows/06-dfx-report.md + docs/storage-statistics/06-debugging.md |
