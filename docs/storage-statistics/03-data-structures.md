---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# 数据结构

本文档描述 OpenHarmony storage_service 空间统计子系统涉及的核心数据结构，包括 IPC 传输对象、磁盘配额信息、DFX 上报结构等。

---

## 1. BundleStats（应用空间统计）

**定义位置：** `interfaces/innerkits/storage_manager/native/bundle_stats.h`

BundleStats 用于描述单个应用的空间占用情况，是 `GetBundleStats` 等接口的返回值类型。继承自 Parcelable，支持跨进程序列化传输。

```cpp
class BundleStats : public Parcelable {
    int64_t appSize_ = 0;    // 应用安装文件大小(字节)
    int64_t cacheSize_ = 0;  // 应用缓存文件大小(字节)
    int64_t dataSize_ = 0;   // 应用数据文件大小(字节)
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| appSize | int64_t | 应用安装文件大小(HAP包) | 字节 |
| cacheSize | int64_t | 应用缓存文件大小 | 字节 |
| dataSize | int64_t | 应用数据文件大小(除安装文件) | 字节 |

**生命周期：** 每次查询创建新实例，IPC 返回后销毁。支持 Parcelable 序列化。

---

## 2. StorageStats（用户存储分类统计）

**定义位置：** `interfaces/innerkits/storage_manager/native/storage_stats.h`

StorageStats 用于按媒体类型分类统计用户存储空间占用，是 `GetCurrentStorageStats` / `GetUserStorageStats` 等接口的返回值类型。继承自 Parcelable，支持跨进程序列化传输。

```cpp
class StorageStats : public Parcelable {
    int64_t total_ = 0;   // 内置存储总空间
    int64_t audio_ = 0;   // 音频数据大小
    int64_t video_ = 0;   // 视频数据大小
    int64_t image_ = 0;   // 图像数据大小
    int64_t file_ = 0;    // 文件数据大小
    int64_t app_ = 0;     // 应用数据大小
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| total | int64_t | 内置存储总空间 | 字节 |
| audio | int64_t | 音频文件总大小 | 字节 |
| video | int64_t | 视频文件总大小 | 字节 |
| image | int64_t | 图片文件总大小 | 字节 |
| file | int64_t | 文档文件总大小 | 字节 |
| app | int64_t | 应用数据总大小 | 字节 |

---

## 3. NextDqBlk（磁盘配额信息）

**定义位置：** `interfaces/innerkits/storage_manager/native/statistic_info.h`

NextDqBlk 对应 Linux 内核的 `if_dqblk` 结构，用于读取和设置磁盘配额信息。在空间统计场景中，该结构是获取各 UID 空间占用的底层数据来源。

```cpp
struct NextDqBlk {
    uint64_t dqbHardLimit;    // 磁盘块硬限制
    uint64_t dqbBSoftLimit;   // 磁盘块软限制
    uint64_t dqbCurSpace;     // 当前已用空间(字节) -- 核心字段
    uint64_t dqbIHardLimit;   // inode硬限制
    uint64_t dqbISoftLimit;   // inode软限制
    uint64_t dqbCurInodes;    // 当前已分配inode数
    uint64_t dqbBTime;        // 磁盘空间超限宽限期
    uint64_t dqbITime;        // inode超限宽限期
    uint32_t dqbValid;        // QIF_*有效位掩码
    uint32_t dqbId;           // UID
};
```

**核心字段说明：**

- `dqbCurSpace` 是扫描统计中获取 UID 空间占用的关键数据。通过 `StorageDaemonCommunication::GetDqBlkSpacesByUids` IPC 从 storage_daemon 获取。
- `dqbId` 标识配额所属的 UID。
- `dqbValid` 使用 `QIF_*` 位掩码标识哪些字段有效。

---

## 4. UidSaInfo（UID/SA 空间信息）

UidSaInfo 用于记录单个 UID 或 SA（System Ability）的空间占用信息，是 DFX 上报的基本单元。

```cpp
struct UidSaInfo {
    int32_t uid;          // 用户ID
    std::string saName;   // SA名称或应用包名
    int64_t size;         // 占用空间(字节)
    uint64_t iNodes;      // 占用inode数
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| uid | int32_t | 用户ID | - |
| saName | std::string | SA名称或应用包名 | - |
| size | int64_t | 占用空间 | 字节 |
| iNodes | uint64_t | 占用inode数 | 个 |

**用途：** 用于 DFX 上报，按 SYS_SA / SYS_APP / USER_APP / OTHER_APP 四类分组。

---

## 5. AllAppVec（全应用分类向量）

AllAppVec 将所有应用按类型分组存储，用于 DFX HAP/SA 统计上报。

```cpp
struct AllAppVec {
    std::vector<UidSaInfo> sysSaVec;     // 系统SA
    std::vector<UidSaInfo> sysAppVec;    // 系统应用
    std::vector<UidSaInfo> userAppVec;   // 用户应用
    std::vector<UidSaInfo> otherAppVec;  // 其余应用
};
```

| 字段 | 类型 | 含义 |
|------|------|------|
| sysSaVec | std::vector\<UidSaInfo\> | 系统SA列表 |
| sysAppVec | std::vector\<UidSaInfo\> | 系统应用列表 |
| userAppVec | std::vector\<UidSaInfo\> | 用户安装的应用列表 |
| otherAppVec | std::vector\<UidSaInfo\> | 其余应用列表 |

---

## 6. DirSpaceInfo（目录空间信息）

DirSpaceInfo 用于记录单个目录的空间占用信息，服务于目录扫描和 DFX 目录统计上报。

```cpp
struct DirSpaceInfo {
    std::string path;    // 目录路径
    uint32_t uid;        // 所属UID
    int64_t size;        // 占用大小(字节)
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| path | std::string | 目录路径 | - |
| uid | uint32_t | 所属UID | - |
| size | int64_t | 占用大小 | 字节 |

---

## 7. SizeInfo（空间信息 - 监控服务内部）

**定义位置：** StorageMonitorService 内部

SizeInfo 用于 StorageMonitorService 内部记录磁盘空间和 inode 的总量与可用量。

```cpp
struct SizeInfo {
    int64_t freeSize;     // 可用空间
    int64_t totalSize;    // 总空间
    int64_t freeInode;    // 可用inode
    int64_t totalInode;   // 总inode
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| freeSize | int64_t | 可用空间 | 字节 |
| totalSize | int64_t | 总空间 | 字节 |
| freeInode | int64_t | 可用inode数 | 个 |
| totalInode | int64_t | 总inode数 | 个 |

---

## 8. ScanResult（扫描结果）

ScanResult 是一次完整磁盘扫描的汇总结果，包含各系统用户的占用以及大文件/大目录列表。

注意：`fondationSize` 为源码中的原始拼写（非标准拼写 foundation），保持与源码一致，不可修改。

```cpp
struct ScanResult {
    int64_t rootSize;          // root用户占用(字节)
    int64_t systemSize;        // system用户占用(字节)
    int64_t fondationSize;     // foundation用户占用(字节)
    int64_t hyperholdRootSize; // hyperhold占用(字节)
    int64_t rgmManagerRootSize;// rgm_manager占用(字节)
    std::vector<LargeFileInfo> largeFiles;  // 大文件列表
    std::vector<LargeDirInfo> largeDirs;    // 大目录列表
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| rootSize | int64_t | root用户占用 | 字节 |
| systemSize | int64_t | system用户占用 | 字节 |
| fondationSize | int64_t | foundation用户占用 | 字节 |
| hyperholdRootSize | int64_t | hyperhold占用 | 字节 |
| rgmManagerRootSize | int64_t | rgm_manager占用 | 字节 |
| largeFiles | std::vector\<LargeFileInfo\> | 大文件列表 | - |
| largeDirs | std::vector\<LargeDirInfo\> | 大目录列表 | - |

---

## 9. LargeFileInfo / LargeDirInfo

大文件和大目录信息结构，用于 DFX 上报中的异常占用检测。

### LargeFileInfo（大文件信息）

```cpp
struct LargeFileInfo {
    std::string path;   // 文件路径
    int64_t size;       // 文件大小(字节)
};
```

### LargeDirInfo（大目录信息）

```cpp
struct LargeDirInfo {
    std::string path;       // 目录路径
    int64_t totalSize;      // 目录总大小(字节)
};
```

**阈值与上限：**

| 类别 | 阈值 | 最大上报数量 |
|------|------|-------------|
| 大文件 | > 1 MB | 15 个 |
| 大目录 | 累计 > 5 MB | 15 个 |

---

## 10. 关键枚举与辅助结构

### CleanType（清理类型 - StorageMonitorService 内部）

```cpp
enum class CleanType {
    CACHE_SPACE,    // 按空间维度清理
    CACHE_INODE     // 按inode维度清理
};
```

StorageMonitorService 根据当前压力类型选择清理维度：空间不足时执行 CACHE_SPACE 清理缓存，inode 不足时执行 CACHE_INODE 清理。

### BaselineCfg（配额基线配置）

```cpp
struct BaselineCfg {
    int32_t uid;        // 用户ID
    int64_t baseSpace;  // 基线空间配额(字节)
    int64_t curSpace;   // 当前已用空间(字节)
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| uid | int32_t | 用户ID | - |
| baseSpace | int64_t | 基线空间配额 | 字节 |
| curSpace | int64_t | 当前已用空间 | 字节 |

用于存储配额管理，比较 `curSpace` 与 `baseSpace` 判断是否超限。

### ExtBundleStats（扩展应用统计）

```cpp
class ExtBundleStats : public Parcelable {
    std::string businessName_;    // 业务名称
    int64_t businessSize_ = 0;   // 业务空间大小
    bool showFlag_ = false;       // 是否显示
};
```

| 字段 | 类型 | 含义 | 单位 |
|------|------|------|------|
| businessName | std::string | 业务名称 | - |
| businessSize | int64_t | 业务空间大小 | 字节 |
| showFlag | bool | 是否在界面上显示 | - |

ExtBundleStats 用于扩展应用空间统计维度，支持按业务名称细分空间占用，并通过 showFlag 控制是否对用户可见。

---

## 11. 跨进程字段精确限制

以下字段在 IPC 跨进程传输时有精确限制：

| 字段 | 所属结构 | 限制 | 说明 |
|------|---------|------|------|
| packageName | getBundleStats参数 | 最大256字节 | 包名长度限制 |
| volumeUuid | get*OfVolume参数 | 最大64字节 | UUID字符串 |
| saName | UidSaInfo | 最大256字节 | SA名称 |
| path | DirSpaceInfo | 最大4096字节 | 文件路径（PATH_MAX） |
| businessName | ExtBundleStats | 最大256字节 | 业务名称 |
| sysSaVec/sysAppVec/userAppVec/otherAppVec | AllAppVec | 各最大1024个元素 | 防止IPC数据过大 |
| largeFiles/largeDirs | ScanResult | 各最大15个元素 | 上报限制 |

---

## 12. 兼容性声明

- **Parcelable 序列化:** 所有继承 Parcelable 的结构体（BundleStats、StorageStats、VolumeCore 等），其字段序列化顺序为稳定契约。新增字段必须追加到 Marshalling/Unmarshalling 末尾，不得在已有字段间插入
- **JSON 持久化:** scan_result.json 的字段结构（rootSize/systemSize/memmgrSize）为稳定格式，不可删除或重命名已有字段
