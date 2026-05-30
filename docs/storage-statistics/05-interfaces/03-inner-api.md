---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# 内部接口

## 1. StorageDaemonCommunication（与storage_daemon的IPC接口）

StorageDaemonCommunication 是 storage_manager 与 storage_daemon 之间的IPC代理层，通过 sptr<IStorageDaemon> 代理进行跨进程调用。

### 空间统计相关接口

| 方法签名 | 功能 | 参数说明 |
|----------|------|---------|
| `int32_t GetTotalSize(int64_t &totalSize)` | 获取/data分区总空间 | totalSize: 输出，字节 |
| `int32_t GetFreeSize(int64_t &freeSize)` | 获取/data分区可用空间 | freeSize: 输出，字节 |
| `int32_t GetTotalInodes(int64_t &totalInodes)` | 获取总inode数 | totalInodes: 输出 |
| `int32_t GetFreeInodes(int64_t &freeInodes)` | 获取可用inode数 | freeInodes: 输出 |
| `int32_t GetBundleStats(string pkgName, BundleStats &bundleStats)` | 获取应用空间统计 | pkgName: 包名, bundleStats: 输出 |
| `int32_t GetSystemSize(int64_t &systemSize)` | 获取系统分区大小 | systemSize: 输出，字节 |
| `int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats)` | 获取用户分类统计 | userId: 用户ID, storageStats: 输出 |
| `int32_t GetDqBlkSpacesByUids(vector<int32_t> uids, map<int32_t,int64_t> &sizeMap)` | 按UID查询quota空间 | uids: UID列表, sizeMap: 输出UID到空间的映射 |
| `int32_t GetDirListSpaceByPaths(vector<string> paths, vector<int32_t> uids, int64_t &rootSize, int64_t &systemSize)` | 按目录路径和UID扫描空间 | paths: 目录列表, uids: UID列表, 输出按UID归类的大小 |
| `int32_t GetDirListSpace(vector<DirSpaceInfo> inDirs)` | 批量查询目录空间 | inDirs: 输入目录列表(含path和uid) |
| `int32_t SetStopScanFlag(bool flag)` | 设置停止扫描标志 | flag: true停止 |
| `int32_t QueryOccupiedSpaceForSa(int32_t userId, int32_t saType, vector<UidSaInfo> &saInfos)` | 查询SA空间占用 | saType: SYS_SA/SYS_APP/USER_APP/OTHER_APP |

---

## 2. BundleManagerConnector（与BundleMgr的IPC接口）

通过 IBundleMgr 代理查询应用空间信息和清理缓存。

| 方法签名 | 功能 | 参数说明 |
|----------|------|---------|
| `int32_t GetBundleStats(string pkgName, int32_t userId, BundleStats &bundleStats)` | 获取应用空间统计 | pkgName: 包名, userId: 用户ID |
| `int32_t CleanBundleCacheFilesAutomatic(int64_t cacheSize, CleanType cleanType, int64_t &cleanedSize)` | 自动清理应用缓存 | cacheSize: 目标清理大小(阈值的2倍), cleanType: CACHE_SPACE或CACHE_INODE |

---

## 3. StorageStatusManager（内部统计管理器）

StorageStatusManager 是空间统计的核心管理器，被 StorageManagerProvider IPC层调用。

| 方法签名 | 功能 | 调用方 |
|----------|------|--------|
| `int32_t GetBundleStats(string pkgName, int32_t userId, BundleStats &bundleStats)` | 获取指定应用空间统计 | StorageManagerProvider |
| `int32_t GetCurrentBundleStats(BundleStats &bundleStats)` | 获取当前调用方应用统计 | StorageManagerProvider |
| `int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats)` | 获取用户分类存储统计 | StorageManagerProvider, StorageDfxReporter |
| `int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, string type)` | 按类型获取用户统计 | StorageManagerProvider |
| `int32_t GetSystemSize(int64_t &systemSize)` | 获取系统空间大小 | StorageManagerProvider |
| `int32_t GetSystemDataSize(int64_t &systemDataSize)` | 获取系统数据大小 | StorageDfxReporter |
| `int32_t GetExtBundleStats(string pkgName, vector<ExtBundleStats> &stats)` | 获取扩展应用统计 | StorageManagerProvider |
| `int32_t GetBundleNameAndUid(int32_t userId, vector<pair<string,int32_t>> &bundleNameUid)` | 获取包名-UID映射 | StorageDfxReporter |

---

## 4. StorageTotalStatusService（总量统计服务）

| 方法签名 | 功能 |
|----------|------|
| `int32_t GetTotalSize(int64_t &totalSize)` | 获取/data分区总空间(字节) |
| `int32_t GetFreeSize(int64_t &freeSize)` | 获取/data分区可用空间(字节) |
| `int32_t GetSystemSize(int64_t &systemSize)` | 获取系统分区大小(字节) |
| `int32_t GetTotalInodes(int64_t &totalInodes)` | 获取总inode数 |
| `int32_t GetFreeInodes(int64_t &freeInodes)` | 获取可用inode数 |
| `int32_t GetUsedInodes(int64_t &usedInodes)` | 获取已用inode数 |

---

## 5. VolumeStorageStatusService（卷级统计服务）

| 方法签名 | 功能 |
|----------|------|
| `int32_t GetTotalSizeOfVolume(string volumeUuid, int64_t &totalSize)` | 获取指定卷总空间 |
| `int32_t GetFreeSizeOfVolume(string volumeUuid, int64_t &freeSize)` | 获取指定卷可用空间 |

---

## 6. StorageManagerScan（扫描服务接口）

| 方法签名 | 功能 |
|----------|------|
| `int32_t Init()` | 初始化(加载缓存或quota查询) |
| `void StartScan()` | 启动异步扫描 |
| `void StopScan()` | 停止扫描 |
| `int64_t GetRootSize()` | 获取root用户占用(字节) |
| `int64_t GetSystemSize()` | 获取system用户占用(字节) |
| `int64_t GetMemmgrSize()` | 获取memmgr用户占用(字节) |

---

## 7. StorageDfxReporter（DFX上报接口）

| 方法签名 | 功能 |
|----------|------|
| `void StartReportHapAndSaStorageStatus()` | 启动HAP/SA统计上报 |
| `void CheckAndTriggerHapAndSaStatistics()` | 检查并触发统计(防重入) |
| `int32_t StartReportDirStatus()` | 启动目录状态上报 |
| `void StartScan()` | 启动扫描 |
| `void StopScan()` | 停止扫描 |

---

## 8. StorageMonitorService（监控服务接口）

| 方法签名 | 功能 |
|----------|------|
| `static StorageMonitorService& GetInstance()` | 获取单例 |
| `void StartStorageMonitorTask()` | 启动监控任务(在OnStart中调用) |

---

## 9. 兼容性声明

- **IPC接口变更:** StorageDaemonCommunication 的所有 IPC 方法签名和参数结构为 storage_manager 与 storage_daemon 之间的稳定契约。新增参数必须通过新的重载方法实现，不可修改已有方法的参数列表
- **Parcelable兼容:** Inner API 数据结构（BundleStats、StorageStats、NextDqBlk 等）的序列化字段顺序不可变更
- **新增IPC方法:** 必须在 IStorageDaemon 定义文件中追加，不得在已有方法间插入
