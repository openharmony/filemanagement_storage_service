# 空间统计与配额约束

本文只记录配额操作的前提条件、空间查询约束和监控阈值规则。IPC 调用约束见 [[ipc-interface-guide]]，线程安全见 [[constraints-and-traps]]。

## 配额前提条件

- 设置配额前必须检查文件系统是否支持 quota（`IsQuotaSupported("/data")`）。不支持时返回 `E_NOT_SUPPORT`，不要强制设置。
- 配额参数要求：uid 有效（>0）、目录路径非空、limit > 0。任一不满足返回 `E_PARAMS_INVALID`。
- 不要在共享目录上设置配额，因为多个应用共享访问。

## 空间查询约束

- 系统级和应用级统计来自不同来源（statvfs vs quotactl），可能有短暂不一致，不要假设两者实时同步。
- 空间查询可能涉及全盘扫描，不可在持有锁时执行，不可在高频路径中调用。先释放锁再查询。
- 查询失败时 quotactl 降级为目录遍历统计，不要直接返回失败而不降级。

## 监控阈值规则

默认值定义于 `services/storage_manager/storage/src/storage_monitor_service.cpp` 的 `DEFAULT_PARAMS`，可通过系统参数 `const.storage_service.storage_alert_policy` 覆盖：

| 阈值键 | 默认值 | 触发行为 | 源码符号 |
|--------|--------|----------|----------|
| `notify_l` | 500 MB | 发送低空间通知 | `BIZ_STAGE_THRESHOLD_NOTIFY_LOW` |
| `notify_m` | 2 GB | 发送中空间通知 | `BIZ_STAGE_THRESHOLD_NOTIFY_MEDIUM` |
| `notify_h` | 总量 10% | 发送高空间通知 | `BIZ_STAGE_THRESHOLD_NOTIFY_HIGH` |
| `clean_l` | 750 MB | 触发 LOW 级清理 | `BIZ_STAGE_THRESHOLD_CLEAN_LOW` |
| `clean_m` | 总量 5% | 触发 MEDIUM 级清理 | `BIZ_STAGE_THRESHOLD_CLEAN_MEDIUM` |
| `clean_h` | 总量 12% | 触发 HIGH 级清理 | `BIZ_STAGE_THRESHOLD_CLEAN_HIGH` |

清理按应用缓存大小排序，优先清理大缓存应用。清理通知发给应用，不主动删除应用数据。

## 统计分类约束

- SYS_SA（系统 SA）的空间不归入用户应用统计。
- 应用类型分类（SYS_SA / SYS_APP / USER_APP）必须基于 UID 范围判断，不要硬编码应用名。

## 共享文件约束

共享文件通过 ACL 控制访问权限，不通过配额限制。创建共享目录时必须设置 SELinux context 为 `u:object_r:share_file:s0`。

## 修改前检查

- 文件系统是否支持 quota？
- 空间查询是否在锁外执行？
- 统计来源不一致是否已处理？
- 共享目录是否绕过了配额？

## 测试指引

- 配额操作：`QuotaManagerTest`
- 监控阈值：`StorageMonitorServiceTest`
- 空间统计一致性：需要板侧验证
