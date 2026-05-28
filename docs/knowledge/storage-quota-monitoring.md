# 空间统计与监控知识

本文记录空间统计、配额管理和存储监控的完整实现细节。IPC 架构见 `ipc-interface-guide.md`。

## 空间统计层级

```
StorageTotalStatusService    ← 系统级总量/可用量
  └→ GetTotalSize() / GetFreeSize() / GetTotalInodes() / GetFreeInodes()

StorageStatusManager         ← 用户/应用级统计
  └→ GetBundleStats()        ← 单应用存储用量
  └→ GetUserStorageStats()   ← 用户存储分类（audio/video/image/file/app）
  └→ GetCurrentBundleStats() ← 当前调用方应用
  └→ GetExtBundleStats()     ← 扩展统计（含备份/缓存细分）

StorageMonitorService        ← 监控与清理
  └→ MonitorAndManageStorage() ← 阈值检查 + 自动清理
```

### 类定义位置

```
services/storage_manager/include/storage_status/storage_status_manager.h
services/storage_manager/include/storage_status/storage_total_status_service.h
services/storage_manager/include/storage_status/storage_monitor_service.h
```

## 统计来源与实现

### 系统级统计

```cpp
// StorageTotalStatusService 直接调用 statvfs
int64_t StorageTotalStatusService::GetTotalSize() {
    struct statvfs vfs;
    if (statvfs("/data", &vfs) != 0) {
        LOGE("statvfs failed: %{public}d", errno);
        return E_SYS_STATVFS;
    }
    return static_cast<int64_t>(vfs.f_blocks) * vfs.f_frsize;
}

int64_t StorageTotalStatusService::GetFreeSize() {
    struct statvfs vfs;
    statvfs("/data", &vfs);
    return static_cast<int64_t>(vfs.f_bfree) * vfs.f_frsize;
}

int64_t StorageTotalStatusService::GetTotalInodes() {
    struct statvfs vfs;
    statvfs("/data", &vfs);
    return static_cast<int64_t>(vfs.f_files);
}
```

### 应用级统计

```cpp
// StorageStatusManager::GetBundleStats
int32_t StorageStatusManager::GetBundleStats(uint32_t userId, 
    const std::string &bundleName, BundleStats &bundleStats) {
    
    // 1. 获取应用的 UID
    int uid = BundleMgrConnector::GetInstance().GetUidByBundleName(userId, bundleName);
    if (uid < 0) {
        return E_GET_UID_ERROR;
    }
    
    // 2. 通过 IPC 调用 Daemon 获取占用空间
    int64_t size = DelayedSingleton<StorageDaemonCommunication>::GetInstance()->GetOccupiedSpace(idType, uid, size);
    
    // 3. 填充统计结果
    bundleStats.appSize_ = size;
    
    // 4. 获取缓存大小（通过媒体库或扫描目录）
    int64_t cacheSize = GetBundleCacheSize(userId, bundleName);
    bundleStats.cacheSize_ = cacheSize;
    
    return E_OK;
}
```

### 用户分类统计

```cpp
// StorageStatusManager::GetUserStorageStats
int32_t StorageStatusManager::GetUserStorageStats(uint32_t userId, 
    UserStorageStats &stats) {
    
    // 1. 从媒体库获取媒体文件统计
    MediaLibraryConnector::GetInstance().GetUserMediaStats(userId, stats);
    
    // 2. 统计应用占用（遍历用户下所有应用）
    std::vector<std::string> bundles;
    BundleMgrConnector::GetInstance().GetBundlesByUserId(userId, bundles);
    
    int64_t totalAppSize = 0;
    for (const auto &bundle : bundles) {
        int uid = BundleMgrConnector::GetInstance().GetUidByBundleName(userId, bundle);
        int64_t size = GetOccupiedSpaceForUid(uid);
        totalAppSize += size;
    }
    stats.totalSize_ = totalAppSize;
    
    // 3. 通过 Daemon IPC 获取扩展统计（备份/缓存细分）
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->GetUserStorageStats(userId, stats);
    
    return E_OK;
}
```

### 扩展统计实现

```cpp
// Daemon 侧：QuotaManager::GetDirListSpace
int64_t QuotaManager::GetDirListSpace(const std::string &path) {
    std::vector<std::string> dirs;
    std::vector<int64_t> sizes;
    
    // 遍历目录获取各子目录大小
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        return E_OPEN_DIR_FAILED;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 
            && strcmp(entry->d_name, "..") != 0) {
            std::string subPath = path + "/" + entry->d_name;
            int64_t size = CalculateDirSize(subPath);
            dirs.push_back(entry->d_name);
            sizes.push_back(size);
        }
    }
    closedir(dir);
    
    return sizes;  // 返回大小数组
}

int64_t QuotaManager::CalculateDirSize(const std::string &path) {
    int64_t totalSize = 0;
    std::function<void(const std::string&)> traverse = [&](const std::string &p) {
        DIR *dir = opendir(p.c_str());
        if (!dir) return;
        
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string fullPath = p + "/" + entry->d_name;
            if (entry->d_type == DT_REG) {
                struct stat st;
                if (stat(fullPath.c_str(), &st) == 0) {
                    totalSize += st.st_size;
                }
            } else if (entry->d_type == DT_DIR && 
                       strcmp(entry->d_name, ".") != 0 &&
                       strcmp(entry->d_name, "..") != 0) {
                traverse(fullPath);
            }
        }
        closedir(dir);
    };
    
    traverse(path);
    return totalSize;
}
```

## 配额管理实现

### QuotaManager 类结构

```cpp
class QuotaManager {
public:
    static QuotaManager &GetInstance();
    
    // 配额设置
    int32_t SetBundleQuota(const std::string &bundleName, int32_t userId, int64_t limit);
    
    // 空间查询
    int64_t GetOccupiedSpaceForUid(int uid);
    int32_t GetDqBlkSpacesByUids(const std::vector<int> &uids, 
                                  std::vector<DqBlkInfo> &infos);
    
    // 系统数据区统计
    int64_t GetSystemDataSize();
    
private:
    // 配额类型
    enum QuotaType {
        USRID = 0,  // 用户配额
        GRPID = 1,  // 组配额
        PRJID = 2,  // 项目配额
    };
    
    std::mutex quotaMutex_;
};
```

### 设置配额

```cpp
int32_t QuotaManager::SetBundleQuota(const std::string &bundleName, 
    int32_t userId, int64_t limit) {
    
    // 1. 参数校验
    if (bundleName.empty() || userId < START_USER_ID || limit <= 0) {
        return E_PARAMS_INVALID;
    }
    
    // 2. 获取应用 UID
    int uid = GetUidByBundleName(userId, bundleName);
    if (uid < 0) {
        return E_GET_UID_ERROR;
    }
    
    // 3. 检查文件系统是否支持配额
    if (!IsQuotaSupported("/data")) {
        LOGE("Quota not supported on /data");
        return E_QUOTA_NOT_SUPPORTED;
    }
    
    // 4. 设置配额
    std::string quotaCmd = "setquota -u " + std::to_string(uid) + 
                           " 0 " + std::to_string(limit / 1024) + " 0 0 /data";
    
    int ret = ExecuteCommand(quotaCmd);
    if (ret != 0) {
        LOGE("Set quota failed for %{public}s: %{public}d", 
             bundleName.c_str(), ret);
        return E_SET_QUOTA_FAILED;
    }
    
    LOGI("Set quota for %{public}s: limit=%{public}lld", 
         bundleName.c_str(), (long long)limit);
    return E_OK;
}
```

### 查询 UID 占用空间

```cpp
int64_t QuotaManager::GetOccupiedSpaceForUid(int uid) {
    // 使用 quotactl 获取占用空间
    struct dqblk dq;
    int ret = quotactl(QCMD(Q_GETQUOTA, USRQUOTA), "/data", uid, 
                       reinterpret_cast<char *>(&dq));
    
    if (ret != 0) {
        LOGE("quotactl failed for uid %{public}d: %{public}d", uid, errno);
        // 降级到目录遍历统计
        return GetOccupiedSpaceByTraversal(uid);
    }
    
    // dq.dqb_curspace 是当前占用空间（字节）
    return static_cast<int64_t>(dq.dqb_curspace);
}

int64_t QuotaManager::GetOccupiedSpaceByTraversal(int uid) {
    // 降级方案：遍历用户目录统计
    std::string userPath = "/data/app/el2/" + std::to_string(uid / USER_ID_BASE);
    return CalculateDirSize(userPath);
}
```

### 批量查询

```cpp
int32_t QuotaManager::GetDqBlkSpacesByUids(const std::vector<int> &uids, 
                                            std::vector<DqBlkInfo> &infos) {
    infos.clear();
    
    for (int uid : uids) {
        DqBlkInfo info;
        info.uid = uid;
        
        struct dqblk dq;
        int ret = quotactl(QCMD(Q_GETQUOTA, USRQUOTA), "/data", uid, 
                           reinterpret_cast<char *>(&dq));
        
        if (ret == 0) {
            info.curSpace = dq.dqb_curspace;
            info.curInodes = dq.dqb_curinodes;
            info.hardLimit = dq.dqb_bhardlimit;
            info.softLimit = dq.dqb_bsoftlimit;
        } else {
            info.curSpace = GetOccupiedSpaceByTraversal(uid);
            info.curInodes = 0;
        }
        
        infos.push_back(info);
    }
    
    return E_OK;
}
```

### 应用类型分类

```cpp
enum AppType {
    SYS_SA = 1,      // 系统 SA
    SYS_APP = 2,     // 系统应用
    USER_APP = 3,    // 用户应用
    OTHER_APP = 4,   // 其他
};

AppType ClassifyAppByUid(int uid) {
    if (uid < FIRST_APPLICATION_UID) {
        return SYS_SA;
    } else if (uid < FIRST_SYSTEM_APP_UID) {
        return SYS_APP;
    } else if (uid < FIRST_USER_APP_UID) {
        return USER_APP;
    }
    return OTHER_APP;
}
```

统计时不将 SYS_SA 空间归入用户应用统计。

## 存储监控实现

### StorageMonitorService 结构

```cpp
class StorageMonitorService {
public:
    static StorageMonitorService &GetInstance();
    
    void StartMonitoring();
    void StopMonitoring();
    void MonitorAndManageStorage();
    
private:
    void CheckAndCleanCache(CleanLevel level);
    void SendStorageNotification(int64_t freeSize, StorageLevel level);
    
    std::thread monitorThread_;
    std::atomic<bool> running_{false};
    std::mutex monitorMutex_;
};
```

### 阈值配置

```cpp
// 空间阈值（字节）
struct SpaceThreshold {
    int64_t notifyLow;      // 通知低阈值
    int64_t notifyMedium;   // 通知中阈值
    int64_t notifyHigh;     // 通知高阈值
    int64_t cleanLow;       // 清理低阈值
    int64_t cleanMedium;    // 清理中阈值
    int64_t cleanHigh;      // 清理高阈值
};

// Inode 阈值
struct InodeThreshold {
    int64_t notifyLow;
    int64_t notifyMedium;
    int64_t notifyHigh;
    int64_t cleanLow;
    int64_t cleanMedium;
    int64_t cleanHigh;
};

SpaceThreshold g_spaceThreshold = {
    .notifyLow = 500 * 1024 * 1024,      // 500MB
    .notifyMedium = 2 * 1024 * 1024 * 1024, // 2GB
    .notifyHigh = 0,  // 动态计算：总量的 10%
    .cleanLow = 750 * 1024 * 1024,       // 750MB
    .cleanMedium = 0,  // 动态计算：总量的 5%
    .cleanHigh = 0,    // 动态计算：总量的 12%
};

InodeThreshold g_inodeThreshold = {
    .notifyLow = 25000,
    .notifyMedium = 100000,
    .notifyHigh = 0,  // 动态计算：总量的 10%
    .cleanLow = 37500,
    .cleanMedium = 0,  // 动态计算：总量的 5%
    .cleanHigh = 0,    // 动态计算：总量的 12%
};
```

### 监控循环

```cpp
void StorageMonitorService::MonitorAndManageStorage() {
    // 1. 获取当前空间状态
    struct statvfs vfs;
    statvfs("/data", &vfs);
    
    int64_t totalSpace = static_cast<int64_t>(vfs.f_blocks) * vfs.f_frsize;
    int64_t freeSpace = static_cast<int64_t>(vfs.f_bfree) * vfs.f_frsize;
    int64_t totalInodes = static_cast<int64_t>(vfs.f_files);
    int64_t freeInodes = static_cast<int64_t>(vfs.f_ffree);
    
    // 2. 计算动态阈值
    int64_t notifyHigh = totalSpace * 10 / 100;  // 10%
    int64_t cleanMedium = totalSpace * 5 / 100;   // 5%
    int64_t cleanHigh = totalSpace * 12 / 100;    // 12%
    
    // 3. 检查空间阈值
    StorageLevel level = StorageLevel::NORMAL;
    if (freeSpace < g_spaceThreshold.notifyLow) {
        level = StorageLevel::CRITICAL;
    } else if (freeSpace < g_spaceThreshold.notifyMedium) {
        level = StorageLevel::LOW;
    }
    
    // 4. 发送通知
    if (level != StorageLevel::NORMAL) {
        SendStorageNotification(freeSpace, level);
    }
    
    // 5. 检查是否需要清理
    if (freeSpace < g_spaceThreshold.cleanLow) {
        CheckAndCleanCache(CleanLevel::RICH);
    } else if (freeSpace < cleanMedium) {
        CheckAndCleanCache(CleanLevel::HIGH);
    } else if (freeSpace < cleanHigh) {
        CheckAndCleanCache(CleanLevel::MEDIUM);
    }
    
    // 6. Inode 检查（类似逻辑）
    // ...
}

void StorageMonitorService::CheckAndCleanCache(CleanLevel level) {
    LOGI("Trigger cache clean, level=%{public}d", level);
    
    // 1. 获取应用缓存统计
    std::vector<BundleCacheInfo> cacheInfos;
    BundleMgrConnector::GetInstance().GetBundleCacheInfos(cacheInfos);
    
    // 2. 按缓存大小排序
    std::sort(cacheInfos.begin(), cacheInfos.end(), 
              [](const BundleCacheInfo &a, const BundleCacheInfo &b) {
                  return a.cacheSize > b.cacheSize;
              });
    
    // 3. 根据清理等级选择清理策略
    int64_t targetSize = CalculateCleanTarget(level);
    int64_t cleanedSize = 0;
    
    for (const auto &info : cacheInfos) {
        if (cleanedSize >= targetSize) {
            break;
        }
        
        // 发送清理通知给应用
        SendCleanNotification(info.bundleName, level);
        
        // 等待应用清理完成
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        cleanedSize += info.cacheSize;
    }
}
```

### 清理策略

| 清理等级 | 时间范围 | 说明 |
|----------|----------|------|
| LOW | 最近分钟级 | 清理临时缓存 |
| MEDIUM | 最近天级 | 清理过期缓存 |
| HIGH | 最近周级 | 大范围清理 |
| RICH | 周级以上 | 紧急清理 |

### 统计任务

```cpp
void StorageMonitorService::HapAndSaStatisticsThd() {
    while (running_) {
        // 1. 收集所有应用空间占用
        std::vector<int> uids = GetAllActiveUids();
        std::vector<DqBlkInfo> infos;
        QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, infos);
        
        // 2. 上报统计结果
        for (const auto &info : infos) {
            StorageRadar::ReportStorageStats(info.uid, info.curSpace);
        }
        
        // 3. 定期执行（如每小时）
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
}
```

## 文件共享空间

### 共享目录路径

```
/data/service/el1/public/storage_daemon/share/public
```

### 创建共享文件

```cpp
int32_t QuotaManager::CreateShareFile(const std::string &sharePath, 
                                       const std::vector<std::string> &authorizedApps) {
    // 1. 创建共享目录
    if (mkdir(sharePath.c_str(), 0777) != 0 && errno != EEXIST) {
        LOGE("Create share dir failed: %{public}s", strerror(errno));
        return E_CREATE_DIR_RECURSIVE_FAILED;
    }
    
    // 2. 设置 ACL（访问控制列表）
    for (const auto &app : authorizedApps) {
        int uid = GetUidByBundleName(app);
        SetupDirAcl(sharePath, uid);
    }
    
    // 3. 设置 SELinux 上下文
    SetSelinuxContext(sharePath, "u:object_r:share_file:s0");
    
    return E_OK;
}

int32_t QuotaManager::SetupDirAcl(const std::string &path, int uid) {
    // 设置 ACL：允许特定 UID 访问
    std::string cmd = "setfacl -m u:" + std::to_string(uid) + ":rwx " + path;
    return ExecuteCommand(cmd);
}
```

**不要在共享目录上设置配额**，因为多个应用共享访问。

## 实现约束

1. **统计不一致**：系统级和应用级统计来自不同来源，可能有短暂不一致，需要处理这种差异
2. **阻塞 I/O**：不要在监控回调中执行阻塞 I/O，统计和清理操作应在独立线程执行
3. **配额支持**：使用前检查文件系统是否支持配额（`IsQuotaSupported`）
4. **缓存大小**：清理时按大小排序，优先清理大缓存应用

## 头文件路径

```
services/storage_manager/include/storage_status/storage_status_manager.h
services/storage_manager/include/storage_status/storage_total_status_service.h
services/storage_manager/include/storage_status/storage_monitor_service.h
services/storage_daemon/include/quota/quota_manager.h
services/storage_daemon/include/quota/quota_manager.h
```

## 测试指引

- 空间统计：`StorageStatusManagerTest`、`StorageTotalStatusServiceTest`
- 配额管理：`QuotaManagerTest`
- 存储监控：`StorageMonitorServiceTest`
- 应用统计：`BundleStatsFuzzer`、`ExtBundleStatsFuzzer`
- 端到端验证：需要板侧运行，观察实际空间回收和通知行为