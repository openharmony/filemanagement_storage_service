---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# 常见问题

## 应用空间查询相关

### Q1: getCurrentBundleStats返回的appSize为什么比实际HAP包大？
A: appSize是应用安装文件的总大小，可能包含多个HAP模块（Entry + Feature）、资源文件和库文件。如果应用有多个模块或适配了多种设备形态，安装时会解压所有匹配的HAP，总大小可能大于单个HAP包。

详见 docs/storage-statistics/04-workflows/01-app-storage-query.md
A: 13600008表示"无此对象"。可能原因：
1. 传入的packageName拼写错误
2. 该应用未安装在当前用户下
3. 该应用是系统预装但未初始化的存根应用

详见 docs/storage-statistics/04-workflows/01-app-storage-query.md
A: 使用系统API getBundleStats(packageName, index)，index参数从API 12开始支持：
- index=0或不传: 查询主应用（默认）
- index>=1: 查询分身应用，索引从1开始
分身索引可通过BundleResourceInfo.appIndex获取。

详见 docs/storage-statistics/04-workflows/01-app-storage-query.md

---

## 总量查询相关

### Q4: getTotalSize和getFreeSize从哪个API版本开始可用？
A: 公共API版本从API 15开始可用（包含Promise/Callback/Sync三种方式）。系统API getTotalSizeOfVolume/getFreeSizeOfVolume从API 8开始可用。

详见 docs/storage-statistics/04-workflows/03-total-free-query.md
A: 传入的volumeUuid对应的卷设备不存在或未挂载。可通过getAllVolumes()先获取有效的卷UUID列表。

详见 docs/storage-statistics/04-workflows/03-total-free-query.md
A: 检查storage_manager服务是否正常运行（通过 `sa_checker storage_manager` 确认）。IPC错误通常表示对端服务未启动或通信断开。

详见 docs/storage-statistics/04-workflows/03-total-free-query.md

---

## 用户分类统计相关

### Q7: getUserStorageStats中audio+video+image+file+app的总和不等于total？
A: 这是正常的。total是/data分区的总空间大小（statfs获取），而audio/video/image/file/app是各类别的数据占用，它们之间可能存在以下差异：
1. 系统数据（systemSize）未计入分类
2. 部分临时文件和缓存未归入任何分类
3. 文件系统元数据占用

详见 docs/storage-statistics/04-workflows/02-user-category-stats.md
A: 传入的userId不合法。仅在使用带userId参数的重载时可能出现。确保userId是有效的系统用户ID。

详见 docs/storage-statistics/04-workflows/02-user-category-stats.md

---

## 扫描相关

### Q9: 为什么扫描不执行？
A: StorageManagerScan有以下前置条件：
1. 设备必须正在充电
2. 屏幕必须关闭（息屏状态）
3. 电量必须大于10%
4. 距上次扫描必须超过24小时（首次扫描除外）
检查所有条件是否同时满足。

详见 docs/storage-statistics/04-workflows/04-disk-scan.md

### Q10: 扫描结果为什么和quota查询不一致？
A: 扫描流程中会对结果进行修正（CalculateFinalSizes）：
1. hyperhold空间从root中扣除
2. hyperhold空间加到memmgr上
3. rgm_manager空间从root中扣除
这种修正确保"谁使用谁负责"的空间归属原则。

详见 docs/storage-statistics/04-workflows/04-disk-scan.md

### Q11: 扫描超时会怎样？
A: 有30秒超时保护机制。超时监控线程（scan_timeout_mon）会检测到扫描仍在运行，强制调用StopScan()停止，并通过SetStopScanFlag通知storage_daemon也停止当前操作。

详见 docs/storage-statistics/04-workflows/04-disk-scan.md

---

## 监控与清理相关

### Q12: 空间不足时为什么没有收到通知？
A: 检查以下几点：
1. StorageMonitorService是否正常启动（通过日志确认）
2. 可用空间是否低于notify_l阈值（默认500M）
3. 通知频率是否在间隔期内（LOW级5分钟，MEDIUM/HIGH级24小时）
4. 空间是否曾经恢复过（恢复后时间戳会重置）

详见 docs/storage-statistics/04-workflows/05-storage-monitor-clean.md

### Q13: 清理缓存后空间没有显著增加？
A: 可能原因：
1. LOW级清理目标是清理阈值的2倍，默认配置下为 750M * 2 = 1500M
2. 应用缓存可能本身就很少
3. 清理频率限制：LOW级5分钟，清理效果不达标时升为1小时
4. 仅清理应用缓存，不清理应用数据

详见 docs/storage-statistics/04-workflows/05-storage-monitor-clean.md

### Q14: 如何自定义阈值参数？
A: 通过系统参数配置：
```bash
# 空间阈值（默认: notify_l:500M/notify_m:2G/notify_h:10%/clean_l:750M/clean_m:5%/clean_h:12%）
param set const.storage_service.storage_alert_policy "notify_l:500M/notify_m:2G/..."

# Inode阈值（默认: notify_l:25000/notify_m:100000/notify_h:10%/clean_l:37500/clean_m:5%/clean_h:12%）
param set const.storage_service.inode_alert_policy "notify_l:25000/notify_m:100000/..."
```
支持单位: M(MB), G(GB), %(百分比)。

详见 docs/storage-statistics/04-workflows/05-storage-monitor-clean.md

---

## DFX相关

### Q15: DFX上报数据在哪里查看？
A: 通过StorageRadar::ReportSpaceRadar上报到HiSysEvent平台。可在DevEco Studio的HiSysEvent查看器或命令行工具中查看。

详见 docs/storage-statistics/04-workflows/06-dfx-report.md

### Q16: HAP/SA统计上报的触发时间为什么不是精确的0:00/8:00/16:00？
A: 触发条件是分钟为0或1时，而Execute()每60秒执行一次。因此实际触发时间可能在整点前后1分钟内偏差，属于正常行为。

详见 docs/storage-statistics/04-workflows/06-dfx-report.md
