# 实现必做清单

本文只记录实现新功能时的必做步骤，不包含代码模板。IPC 新增流程见 [[ipc-interface-guide]]，代码放置见 [[architecture]]。

## 所有公开接口的必做步骤

| 步骤 | 要求 | 不做的后果 |
|------|------|------------|
| 参数校验 | 所有公开接口必须有 | 调用方传空或越界导致内核 crash |
| 权限校验 | Manager 入口必须有 | 未授权调用操作密钥或挂载 |
| 状态检查 | 操作前检查当前状态 | 非法状态操作导致数据损坏 |
| 状态预更新 | 操作前先置中间状态 | 操作中途查询到不一致状态 |
| 失败回退 | 操作失败回退到之前状态 | 停留在中间状态导致死锁 |
| 成功日志 | 成功必须 LOGI | 无法确认操作是否完成 |
| 失败日志 | 失败必须 LOGE | 无法定位问题 |
| 失败上报 | 关键操作失败必须 StorageRadar 上报 | 故障统计缺失 |

## 各场景必做项

### 卷操作

- Mount 前：置 CHECKING → 执行 Check → 成功置 MOUNTED，失败置 DAMAGED 或 UNMOUNTED
- Unmount 前：置 EJECTING → 执行 UMount → 成功置 REMOVED，失败置 MOUNTED
- 失败必须 `StorageRadar::ReportVolumeOperation`

### 密钥操作

- 操作前：HUKS 可用性检查（`IsHuksAvailable()`）
- userId 校验：范围 [100, 10738]，超出返回 `E_USERID_RANGE`
- 失败必须 `StorageRadar::ReportUserKeyResult` 或 `ReportActiveUserKey`

### 用户目录操作

- 创建失败必须回滚全部已创建目录（`RollbackUserDirs`）
- 密钥生成失败也必须销毁已创建目录
- 权限设置：SELinux context 必须设置

### 配额操作

- 设置前：检查文件系统是否支持 quota
- 查询失败降级：quotactl 失败降级为目录遍历，不要直接报错

### IPC 层

- Stub 解析：MessageParcel 读取后必须校验参数非空
- Manager Provider：必须权限校验 + 状态检查 + 调用 Daemon
- Daemon Provider：必须路径校验 + 调用核心类

## 新增文件系统 Operator

1. 新增 `xxx_operator.h/.cpp` 继承 `IVolumeOperator`
2. 实现 `Mount/UMount/Format/Check/GetFsType`
3. 在 `volume_operator_factory.cpp` 的 `CreateOperator()` 注册

## 修改前检查

- 必做8步骤是否全部覆盖？
- 失败时是否上报了 StorageRadar？
- 回滚逻辑是否覆盖了半成品状态？
