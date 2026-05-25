# 架构概览

本文记录目录结构、核心类、头文件路径和继承关系。

## 目录职责

| 目录 | 职责 | 核心类 |
|------|------|--------|
| `services/storage_daemon/` | 底层守护进程，负责设备热插拔、分区/挂载/格式化、fscrypt 密钥管理、用户目录创建/销毁、配额管理 | `DiskManager`, `VolumeManager`, `KeyManager`, `UserManager`, `QuotaManager` |
| `services/storage_manager/` | 系统 SA，负责卷/磁盘状态维护、用户生命周期、空间统计与监控、IPC 桥接 | `StorageManagerProvider`, `DiskManagerService`, `VolumeManagerService`, `StorageStatusManager` |
| `interfaces/innerkits/` | Native 内部 API | `Disk`, `VolumeCore`, `StorageStats` |
| `interfaces/kits/` | JS/ETS 和 CJ 对外 API | 存储管理 Kit |
| `services/common/` | 共享常量和日志 | `StorageServiceConstant`, `storage_service_log.h` |
| `test/` | 单元测试和 fuzz 目标 | `*Test`, `*Fuzzer` |

## 关键头文件路径

```
# Daemon 核心类
services/storage_daemon/include/crypto/key_manager.h          # 密钥管理核心
services/storage_daemon/include/crypto/base_key.h             # 密钥基类
services/storage_daemon/include/crypto/huks_master.h          # HUKS 封装
services/storage_daemon/include/crypto/fscrypt_key_v1.h       # fscrypt V1
services/storage_daemon/include/crypto/fscrypt_key_v2.h       # fscrypt V2
services/storage_daemon/include/user/user_manager.h           # 用户目录管理
services/storage_daemon/include/user/mount_manager.h          # 挂载管理
services/storage_daemon/include/volume/volume_manager.h       # 卷管理
services/storage_daemon/include/volume/volume_info.h          # 卷状态机
services/storage_daemon/include/volume/external_volume_info.h # 外部存储卷
services/storage_daemon/include/disk/disk_manager.h           # 磁盘管理
services/storage_daemon/include/disk/disk_info.h              # 磁盘信息
services/storage_daemon/include/netlink/netlink_manager.h     # Netlink 事件
services/storage_daemon/include/quota/quota_manager.h         # 配额管理
services/storage_daemon/include/ipc/storage_daemon_provider.h # Daemon IPC 入口

# Manager 核心类
services/storage_manager/include/ipc/storage_manager_provider.h        # SA 入口
services/storage_manager/include/volume/volume_manager_service.h       # 卷服务
services/storage_manager/include/disk/disk_manager_service.h           # 磁盘服务
services/storage_manager/include/storage/storage_status_manager.h      # 空间统计
services/storage_manager/include/storage/storage_monitor_service.h     # 存储监控
services/storage_manager/include/account_subscriber/account_subscriber.h # 用户事件订阅
services/storage_manager/include/storage_daemon_communication/storage_daemon_communication.h # Daemon 通信

# 共享定义
services/common/include/storage_service_constant.h            # KeyType、EL 常量
services/common/include/storage_service_log.h                 # 日志宏
interfaces/innerkits/storage_manager/native/storage_service_errno.h # 错误码

# 文件系统操作
services/storage_daemon/include/disk_manager/volume/ivolume_operator.h    # 卷操作接口
services/storage_daemon/include/disk_manager/volume/ext4_operator.h       # ext4
services/storage_daemon/include/disk_manager/volume/exfat_operator.h      # exFAT
services/storage_daemon/include/disk_manager/volume/ntfs_operator.h       # NTFS
services/storage_daemon/include/disk_manager/volume/vfat_operator.h       # VFAT
services/storage_daemon/include/disk_manager/volume/hmfs_operator.h       # HMFS
```

## 关键类继承关系

### 密钥类继承

```
BaseKey (crypto/base_key.h) - 密钥基类，提供 StoreKey/RestoreKey/ActiveKey/InactiveKey
  └→ FscryptKeyV1 (crypto/fscrypt_key_v1.h) - fscrypt V1 密钥实现
  └→ FscryptKeyV2 (crypto/fscrypt_key_v2.h) - fscrypt V2 密钥实现
  └→ FscryptKeyV1Ext (crypto/fscrypt_key_v1_ext.h) - fscrypt V1 扩展版本
```

### 卷类继承

```
VolumeInfo (volume/volume_info.h) - 卷信息基类，定义状态机和基本操作
  └→ ExternalVolumeInfo (volume/external_volume_info.h) - 外部存储卷实现
```

### 文件系统操作类继承

```
IVolumeOperator (disk_manager/volume/ivolume_operator.h) - 卷操作接口
  └→ Ext4Operator - ext4 文件系统操作
  └→ ExfatOperator - exFAT 文件系统操作
  └→ NtfsOperator - NTFS 文件系统操作
  └→ VfatOperator - VFAT/FAT32 文件系统操作
  └→ HmfsOperator - HMFS 文件系统操作
```

## 单例模式使用

以下核心类使用单例模式：

| 类名 | 获取方式 | 说明 |
|------|----------|------|
| `KeyManager` | `GetInstance()` | 密钥管理 |
| `UserManager` | `GetInstance()` | 用户目录管理 |
| `QuotaManager` | `GetInstance()` | 配额管理 |
| `HuksMaster` | `GetInstance()` | HUKS 封装 |
| `StorageRadar` | `GetInstance()` | 雷达上报 |
| `DiskManager` | `Instance()` | 磁盘管理（注意不是 GetInstance） |
| `VolumeManager` | `Instance()` | 卷管理（注意不是 GetInstance） |
| `NetlinkManager` | `Instance()` | netlink 管理（注意不是 GetInstance） |
| `StorageDaemonCommunication` | `DelayedSingleton<StorageDaemonCommunication>::GetInstance()` | Daemon 通信（DelayedSingleton） |
| `DiskManagerService` | `GetInstance()` | Manager 侧磁盘服务 |
| `VolumeManagerService` | `GetInstance()` | Manager 侧卷服务 |
| `BundleMgrConnector` | `GetInstance()` | 应用信息查询（注意类名是 BundleMgrConnector） |

```cpp
KeyManager::GetInstance().GenerateUserKeys(user, flags);
DiskManager::Instance().HandleDiskEvent(data);
VolumeManager::Instance().Mount(volId, flags);
DelayedSingleton<StorageDaemonCommunication>::GetInstance()->Mount(volumeId, flag);
StorageRadar::GetInstance().RecordFunctionResult(parameterRes);
StorageRadar::ReportVolumeOperation("Mount", ret);
```

## IPC 架构概览

```
┌─────────────────────────┐        IPC (SystemAbility)       ┌──────────────────────────┐
│    storage_manager       │◄──────────────────────────────►│    storage_daemon          │
│    (SA ID: 5003)         │                                 │    (SA ID: 5004)           │
│                           │                                 │                            │
│  StorageManagerProvider   │  StorageDaemonCommunication    │  StorageDaemonProvider     │
│  (Stub 实现)              │  (Manager 侧 Daemon 代理)       │  (Stub 实现)               │
│                           │                                 │                            │
│  DiskManagerService       │                                 │  DiskManager               │
│  VolumeManagerService     │                                 │  VolumeManager             │
│  StorageStatusManager     │                                 │  UserManager               │
│  StorageMonitorService    │                                 │  KeyManager                │
│  AccountSubscriber        │                                 │  QuotaManager              │
└─────────────────────────┘                                 └──────────────────────────┘
```

Manager 是面向应用和其他子系统的入口，Daemon 是面向内核和硬件的底层执行者。

详细 IPC 流程见 `ipc-interface-guide.md`。