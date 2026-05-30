---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# 核心概念

本文档介绍 OpenHarmony storage_service 空间统计子系统的核心概念，涵盖统计维度、数据结构、阈值体系及 DFX 上报等关键内容。

---

## 1. 存储空间统计维度

空间统计从多个维度对设备存储进行量化分析，每个维度关注不同的统计对象和字段。

### 应用维度（BundleStats）

针对单个应用的存储空间占用进行统计，包含以下字段：

- **appSize**：应用安装文件大小，即 HAP 包的大小。
- **cacheSize**：应用缓存文件大小，对应路径为 `/data/var/cache_overlay/bundleName/`。
- **dataSize**：应用数据文件大小，即除安装文件外的所有数据文件的总大小。

### 用户维度（StorageStats）

针对单个用户的存储空间占用进行统计，按文件类型分类汇总，包含以下字段：

- **total**：内置存储总空间。
- **audio**：音频文件总大小。
- **video**：视频文件总大小。
- **image**：图片文件总大小。
- **file**：文档文件总大小。
- **app**：应用数据总大小。

### 系统维度

针对设备整体存储空间的统计，包含以下字段：

- **systemSize**：系统分区数据大小。
- **freeSize**：可用空间。
- **totalSize**：总空间。

### inode维度

针对文件系统 inode 使用情况的统计，包含以下字段：

- **totalInodes**：文件系统总 inode 数。
- **freeInodes**：可用 inode 数。

---

## 2. 对比表

### BundleStats vs StorageStats

| 特征 | BundleStats | StorageStats |
|------|-------------|-------------|
| 统计对象 | 单个应用 | 单个用户 |
| 字段 | appSize, cacheSize, dataSize | audio, video, image, file, app, total |
| 数据来源 | BundleMgr IPC | 媒体库 + BundleMgr |
| API级别 | 公共 + 系统 | 仅系统 |
| 支持分身 | 系统API支持 index 参数 | 不支持 |

### 公共API vs 系统API

| 特征 | 公共API | 系统API |
|------|---------|---------|
| 权限 | 无 | ohos.permission.STORAGE_MANAGER |
| 调用方 | 第三方应用 | 仅系统应用 |
| 查询范围 | 当前应用自身、内置存储 | 任意应用、指定卷、系统空间、指定用户 |
| 同步接口 | getTotalSizeSync, getFreeSizeSync | 无 |
| 最小API版本 | 9+ / 15+ | 8+ |

---

## 3. Quota与目录扫描

### Quota统计

- 基于 Linux 内核提供的磁盘配额机制实现。
- 按UID统计空间占用，核心字段为 `dqbCurSpace`。
- 数据精确，由内核维护，不存在计算误差。
- 仅统计已设置配额的 UID，未设置配额的 UID 不在统计范围内。

### 目录扫描

- 通过递归遍历指定目录计算总大小。
- 不依赖 quota 设置，可独立运作。
- 主要用于补充 quota 无法覆盖的场景，例如白名单目录的空间统计。
- 扫描耗时较长，需要在后台线程中执行，避免阻塞主线程。

### 两者的配合

在实际扫描流程中，quota 统计与目录扫描结合使用，流程如下：

1. 先通过 quota 获取 memmgr（UID 1111）的空间占用。
2. 再通过目录扫描获取白名单目录的 root/system 空间占用。
3. 特殊路径（hyperhold、rgm_manager）从 root 中扣除，hyperhold 空间归入 memmgr。

---

## 4. 三级阈值体系

存储空间管理通过多级阈值实现精细化管控，分为空间阈值和 inode 阈值两部分。

### 空间阈值（默认值）

配置参数：`const.storage_service.storage_alert_policy`

| 参数 | 值 | 说明 |
|------|-----|------|
| notify_l | 500M | LOW 级通知阈值 |
| notify_m | 2G | MEDIUM 级通知阈值 |
| notify_h | 10% | HIGH 级通知阈值 |
| clean_l | 750M | LOW 级清理阈值 |
| clean_m | 5% | MEDIUM 级清理阈值 |
| clean_h | 12% | HIGH 级清理阈值 |

### Inode阈值（默认值）

配置参数：`const.storage_service.inode_alert_policy`

| 参数 | 值 | 说明 |
|------|-----|------|
| notify_l | 25000 | LOW 级通知阈值 |
| notify_m | 100000 | MEDIUM 级通知阈值 |
| notify_h | 10% | HIGH 级通知阈值 |
| clean_l | 37500 | LOW 级清理阈值 |
| clean_m | 5% | MEDIUM 级清理阈值 |
| clean_h | 12% | HIGH 级清理阈值 |

### 清理触发规则（WHEN/THEN格式）

- WHEN freeSize < clean_l 或 freeInode < inode clean_l THEN 触发 LOW 级清理，间隔 300s，清理效果不达标时升为 3600s
- WHEN freeSize < clean_m 或 freeInode < inode clean_m THEN 触发 MEDIUM 级清理，间隔 86400s（1天）
- WHEN freeSize < clean_h 或 freeInode < inode clean_h THEN 触发 HIGH 级清理，间隔 604800s（1周）
- WHEN freeSize >= clean_h 且 freeInode >= inode clean_h THEN 空间充足（RICH），不触发清理

- WHEN freeSize < notify_l 或 freeInode < inode notify_l THEN 发送 LOW 级通知，间隔 5分钟（高频）
- WHEN freeSize < notify_m 或 freeInode < inode notify_m THEN 发送 MEDIUM 级通知，间隔 24小时
- WHEN freeSize < notify_h 或 freeInode < inode notify_h THEN 发送 HIGH 级通知，间隔 24小时
- WHEN 空间从不足恢复到 RICH THEN 重置所有通知时间戳

### 清理级别对比

| 级别 | 触发条件 | 清理事件间隔 | 通知间隔 |
|------|---------|-------------|---------|
| LOW | 低于 clean_l | 300s / 3600s | 5分钟（高频） |
| MEDIUM | 低于 clean_m | 86400s（1天） | 24小时 |
| HIGH | 低于 clean_h | 604800s（1周） | 24小时 |
| RICH | 空间充足 | 604800s（1周） | 不通知 |

---

## 5. 特殊UID空间归属

系统通过特殊 UID 的空间归属机制，确保存储空间的责任清晰可追溯。

| UID | 用户 | 说明 |
|-----|------|------|
| 0（root） | root | 扣除 hyperhold 和 rgm_manager 后的空间 |
| 1000（system） | system | system 用户的白名单目录空间 |
| 1111（memmgr） | 内存管理服务 | memmgr quota 空间 + hyperhold 空间 |

其中，hyperhold（内存扩展/swap 压缩存储）的空间从 root 中扣除并归入 memmgr，体现了"谁使用谁负责"的空间归属原则。

---

## 6. DFX上报类型

系统通过多种 DFX 上报类型，为存储空间分析和问题定位提供数据支撑。

| 类型 | 上报内容 | 触发时机 |
|------|---------|---------|
| HAP/SA统计 | 按应用/SA分类的空间占用 | 每天 0:00 / 8:00 / 16:00 |
| 目录统计 | root/system/foundation 的 TOP20 大目录 | 目录空间增量 >= 1GB |
| 扫描结果 | root/system/memmgr 占用 + 大文件/大目录 | 扫描完成后 |
| 清理雷达 | 清理级别/清理效果/通知情况 | 清理和通知时 |

## 7. 兼容性声明

- **阈值参数格式:** 参数字符串格式为 `key:value/key:value/...`，新增键不破坏已有键的解析。值的单位支持 M/G/%，新增单位需确保旧版本忽略不崩溃
- **清理级别语义:** LOW/MEDIUM/HIGH/RICH 四级语义不可变更，新增级别需追加
