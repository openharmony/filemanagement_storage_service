---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# 存储监控与自动清理工作流

## 概述

StorageMonitorService 是单例服务，每60秒执行一次磁盘空间监控，根据三级阈值体系判定是否需要发送通知和清理缓存。

## 服务启动流程

```mermaid
sequenceDiagram
    participant SA as StorageManagerProvider.OnStart()
    participant SMS as StorageMonitorService
    participant EH as EventHandler
    participant ER as EventRunner

    SA->>SMS: StartStorageMonitorTask()
    SMS->>SMS: 创建eventThread_线程
    SMS->>ER: CreateEventRunner(非自动运行)
    SMS->>EH: CreateEventHandler(ER)
    Note over SMS: 等待EH创建完成(5s超时)
    SMS->>EH: PostTask(Execute, 延迟60s)
    SMS->>EH: PostTask(UpdateBaseLineByUid, 延迟10min)
```

## 周期监控流程

```mermaid
graph TD
    A[Execute: 每60s执行] --> B[MonitorAndManageStorage]
    B --> C[获取磁盘空间和inode]
    C --> D[ParseStorageParameters: 解析阈值]
    D --> E[CheckAndEventNotify: 通知判定]
    D --> F[CheckAndCleanCache: 清理判定]
    F --> G{freeSize < clean_l 或 freeInode < inode_clean_l}
    G -->|是| H[LOW级清理: 5min间隔]
    G -->|否| I{freeSize < clean_m 或 freeInode < inode_clean_m}
    I -->|是| J[MEDIUM级清理: 1天间隔]
    I -->|否| K{freeSize < clean_h 或 freeInode < inode_clean_h}
    K -->|是| L[HIGH级清理: 1周间隔]
    K -->|否| M[RICH: 空间充足]
    A --> N[HapAndSaStatisticsThd: 检查是否触发DFX统计]
```

## 清理执行流程

```mermaid
sequenceDiagram
    participant SMS as StorageMonitorService
    participant BMC as BundleManagerConnector
    participant BM as BundleMgr
    participant CEE as CommonEventManager
    participant FCA as FileCacheAdapter

    SMS->>SMS: CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode)

    SMS->>FCA: GetLastNotifyTime(cleanLevel)
    FCA-->>SMS: lastNotifyTime

    alt 距上次通知 >= 清理间隔
        SMS->>CEE: PublishCleanCacheEvent(cleanLevel, type, free, total)
        Note over CEE: 发布 usual.event.DEVICE_STORAGE_LOW

        alt LOW/MEDIUM级需要清理
            SMS->>BMC: CleanBundleCacheFilesAutomatic(lowThreshold*2, cleanType)
            Note over SMS: 清理目标 = lowThreshold * 2, 即清理出比阈值多一倍的空间, 确保清理后空间回到安全水位以上
            BMC->>BM: IPC: 清理应用缓存
            BM-->>BMC: cleanedSize
            BMC-->>SMS: 清理结果
        end

        SMS->>FCA: SetLastNotifyTime(cleanLevel, curTime)
    end
```

## 通知流程

```mermaid
sequenceDiagram
    participant SMS as StorageMonitorService
    participant CEE as CommonEventManager
    participant HiCare as HiCare框架

    SMS->>SMS: CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode)

    alt LOW级(freeSize < notify_l)
        SMS->>CEE: SendSmartNotificationEvent(faultId=845010021, suggestId=545010023, isHighFreq=true)
        Note over CEE: 发布 hicare.event.SMART_NOTIFICATION
        Note over CEE: 845010021=空间不足LOW级故障ID, 545010023=建议清理缓存ID
    else MEDIUM级(freeSize < notify_m)
        SMS->>CEE: SendSmartNotificationEvent(faultId=845010008, suggestId=545010008, isHighFreq=false)
        Note over CEE: 845010008=空间不足MEDIUM级故障ID, 545010008=建议清理缓存ID
    else HIGH级(freeSize < notify_h)
        SMS->>CEE: SendSmartNotificationEvent(faultId=845010008, suggestId=845010008, isHighFreq=false)
        Note over CEE: 845010008=空间不足HIGH级故障ID, 545010008=建议清理缓存ID
    end

    alt 空间恢复正常
        SMS->>SMS: RefreshAllNotificationTimeStamp()
        Note over SMS: 重置所有通知时间戳
    end
```

## 阈值参数配置

空间阈值参数: const.storage_service.storage_alert_policy
默认值: notify_l:500M/notify_m:2G/notify_h:10%/clean_l:750M/clean_m:5%/clean_h:12%
支持单位: M(MB), G(GB), %(百分比)

Inode阈值参数: const.storage_service.inode_alert_policy
默认值: notify_l:25000/notify_m:100000/notify_h:10%/clean_l:37500/clean_m:5%/clean_h:12%
支持具体数值和百分比

### 时间阈值常量

| 常量 | 值 | 说明 | 是否可配置 |
|------|-----|------|-----------|
| DEFAULT_CHECK_INTERVAL | 60s | 周期监控间隔 | 否（代码常量） |
| LOW级清理间隔 | 300s/3600s | 清理效果不达标时升级 | 否（代码常量） |
| MEDIUM级清理间隔 | 86400s | 1天 | 否（代码常量） |
| HIGH级清理间隔 | 604800s | 1周 | 否（代码常量） |
| LOW级通知间隔 | 300s | 5分钟高频 | 否（代码常量） |
| MEDIUM/HIGH级通知间隔 | 86400s | 24小时 | 否（代码常量） |

## 关键代码路径

| 流程 | 源码文件 |
|------|---------|
| 监控主逻辑 | services/storage_manager/storage/src/storage_monitor_service.cpp |
| 启动入口 | services/storage_manager/ipc/src/storage_manager_provider.cpp |
| 包管理清理 | services/storage_manager/storage/src/bundle_manager_connector.cpp |
| 配额基线 | services/storage_manager/storage/src/storage_quota_controller.cpp |
| 文件缓存 | services/storage_manager/utils/file_cache_adapter.cpp |
| 常量定义 | services/common/include/storage_service_constant.h |
