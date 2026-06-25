# 代码放置与单例陷阱

本文只记录新增代码的放置位置规则和单例获取方式的易错点。IPC 新增流程见 [[ipc-interface-guide]]。

## 新增代码放置位置

| 新功能类型 | 放置位置 | 必须修改的文件 |
|------------|----------|----------------|
| 新增 IPC 接口 | Manager + Daemon 双侧 | Manager Stub + Provider + Daemon Stub + Provider + 核心类 |
| 修改挂载逻辑 | Daemon 侧 | `VolumeManager` 或 `VolumeInfo` |
| 新增密钥操作 | Daemon KeyManager | `key_manager.h/.cpp`，可选新增 BaseKey 子类 |
| 新增用户操作 | Daemon + Manager | Daemon `UserManager` + Manager Provider + Stub |
| 新增空间统计 | Manager 侧 | `StorageStatusManager` 或 `StorageTotalStatusService` |
| 新增配额功能 | Daemon 侧 | `QuotaManager` |
| 新增文件系统支持 | Daemon disk_manager/volume | 新增 Operator `.h/.cpp` + `volume_operator_factory.cpp` 注册 |
| 新增磁盘事件处理 | Daemon 侧 | `DiskManager` + `NetlinkHandler` |
| 新增监控阈值 | Manager 侧 | `StorageMonitorService` |
| 新增错误码 | 共享定义 | `storage_service_errno.h` |

不可只改一侧。Manager 和 Daemon 必须同时修改。

## 单例获取易错点

| 类名 | 获取方式 | 易错点 |
|------|----------|--------|
| KeyManager | `GetInstance()` | 不要用 `Instance()` |
| UserManager | `GetInstance()` | 不要用 `Instance()` |
| QuotaManager | `GetInstance()` | 不要用 `Instance()` |
| HuksMaster | `GetInstance()` | 不要用 `Instance()` |
| StorageRadar | `GetInstance()` | 不要用 `Instance()` |
| DiskManager | **`Instance()`** | **不是 GetInstance**，最容易出错 |
| VolumeManager | **`Instance()`** | **不是 GetInstance** |
| NetlinkManager | **`Instance()`** | **不是 GetInstance** |
| StorageDaemonCommunication | `DelayedSingleton<...>::GetInstance()` | 必须用 DelayedSingleton 包装 |
| BundleMgrConnector | `GetInstance()` | 类名是 **BundleMgrConnector** 不是 BundleManagerConnector |

使用单例前必须确认对应类的获取方式，不要默认全部用 `GetInstance()`。

## 头文件路径（新增代码时查找定义）

- Daemon IPC 入口：`services/storage_daemon/include/ipc/storage_daemon_provider.h`
- Manager IPC 入口：`services/storage_manager/include/ipc/storage_manager_provider.h`
- Manager Stub：`services/storage_manager/include/ipc/storage_manager_stub.h`
- Daemon 通信代理：`services/storage_manager/include/storage_daemon_communication/storage_daemon_communication.h`
- 共享常量：`services/common/include/storage_service_constant.h`
- 错误码：`interfaces/innerkits/storage_manager/native/storage_service_errno.h`

## 修改前检查

- 新代码放在了正确的侧（Manager/Daemon）？
- 对应双侧是否同时修改？
- 单例获取方式是否用了正确的方法名？
