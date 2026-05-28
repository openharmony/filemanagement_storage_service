# 磁盘与卷生命周期

本文记录磁盘热插拔、分区、卷状态机、文件系统差异和典型场景完整流程。

## 事件来源与处理

磁盘事件通过内核 netlink `block` 子系统到达：

```
内核 netlink 事件
  └ NetlinkManager::Start() 启动监听
    └ NetlinkHandler::OnEvent(data)
      ├ 解码事件类型：ADD / CHANGE / REMOVE
      └ 投递到工作线程（不阻塞）
        └ DiskManager::HandleDiskEvent(data)
```

### 事件处理约束

```cpp
// 错误：在 netlink 回调中执行阻塞操作
void NetlinkHandler::OnEvent(const NetlinkData &data) {
    if (data.action == "add") {
        DiskManager::HandleDiskEvent(data);  // 可能执行 mount、fsck
    }
}

// 正确：投递到工作线程
void NetlinkHandler::OnEvent(const NetlinkData &data) {
    auto dataCopy = data;
    workerThread_.PostTask([dataCopy]() {
        DiskManager::HandleDiskEvent(dataCopy);
    });
}
```

## 磁盘生命周期

```
ADD 事件：
  DiskManager::CreateDisk(sysPath, devPath, device, diskType)
    └ DiskInfo::Create()
      ├ ReadMetadata()         // 读取磁盘元数据
      ├ NotifyDiskCreated()    // 通过 IPC 通知 Manager
      └ ReadPartition()       // 读取分区表
        └ 为每个分区创建 VolumeInfo

CHANGE 事件：
  DiskManager::ChangeDisk()
    └ 更新磁盘元数据
    └ 重新扫描分区

REMOVE 事件：
  DiskManager::DestroyDisk(diskId)
    └ DiskInfo::Destroy()
      ├ 销毁所有关联 Volume
      └ 卸载所有挂载点
      └ 清理资源
      └ NotifyDiskDestroyed() // 通过 IPC 通知 Manager
```

## 磁盘类型处理差异

| 类型 | 枚举值 | 分区支持 | 格式化支持 | 挂载方式 | 特殊处理 |
|------|--------|----------|------------|----------|----------|
| SD_CARD | 1 | 支持（最多 8 分区） | 支持 | 内核 mount | 可分区、可格式化 |
| USB_FLASH | 2 | 支持（最多 16 分区） | 支持 | 内核 mount | 分区数限制、超时 60s |
| CD_DVD_BD | 3 | 不支持 | 不支持 | 内核 mount | 只读、支持弹出 Eject |
| MTP_PTP | 4 | 不支持 | 不支持 | FUSE (gphotofs) | 特殊挂载流程 |

### 类型判断

```cpp
DiskType DiskInfo::GetDiskType(const std::string &sysPath) {
    if (sysPath.find("mmcblk") != std::string::npos) {
        return SD_CARD;
    } else if (sysPath.find("sd") != std::string::npos) {
        return USB_FLASH;
    } else if (sysPath.find("sr") != std::string::npos) {
        return CD_DVD_BD;
    }
    return UNKNOWN_DISK_TYPE;
}
```

## 卷状态机完整定义

### 状态枚举

```cpp
enum VolumeState {
    UNMOUNTED = 0,           // 未挂载（初始状态）
    CHECKING,                // 正在检查文件系统
    MOUNTED,                 // 已挂载
    EJECTING,                // 正在弹出
    REMOVED,                 // 已移除（正常移除）
    BADREMOVABLE,            // 异常移除（热拔出）
    DAMAGED,                 // 文件系统损坏
    FUSE_REMOVED,            // FUSE 设备移除
    DAMAGED_MOUNTED,         // 损坏但已挂载（TryToFix 成功）
    ENCRYPTING,              // 正在加密
    ENCRYPTED_AND_LOCKED,    // 已加密且锁定
    ENCRYPTED_AND_UNLOCKED,  // 已加密且解锁
    DECRYPTING,              // 正在解密
};
```

### 状态转换图

```
                     ┌──────────────────────────────┐
                     │         UNMOUNTED             │◄──── 创建、卸载完成
                     └──────┬───────────────────────┘
                            │ Mount() 触发 Check()
                            ▼
                     ┌──────────────┐
                     │   CHECKING   │
                     └──────┬───────┘
                       Check() │    ┌─────────────────┐
                     ┌─────────▼────▼─┐               │
                     │    MOUNTED     │──── TryToFix()─┤
                     └───────┬────────┘               │
                             │                         ▼
                      Unmount() │               ┌──────────────────┐
                             ▼                 │ DAMAGED_MOUNTED  │
                     ┌──────────────┐          └──────────────────┘
                     │   EJECTING   │
                     └──────┬───────┘
                            │
               ┌────────────▼─────────────┐
               │ REMOVED / BADREMOVABLE   │◄─── 正常移除 / 异常拔出
               └──────────────────────────┘

                     ┌──────────────┐
                     │   DAMAGED    │◄──── Check() 失败
                     └──────┬───────┘
                            │ TryToFix()
                            ▼
                     ┌──────────────────┐
                     │ DAMAGED_MOUNTED  │
                     └──────────────────┘
```

### 状态转换代码示例

```cpp
int32_t VolumeManager::Instance().Mount(const std::string &volumeId, uint32_t flags) {
    auto volume = GetVolume(volumeId);
    if (volume->GetState() != UNMOUNTED) {
        LOGW("Volume %{public}s state %{public}d, cannot mount", 
             volumeId.c_str(), volume->GetState());
        return E_VOL_STATE;
    }
    
    volume->SetState(CHECKING);  // 状态预更新
    int32_t ret = volume->Check();
    if (ret != E_OK) {
        volume->SetState(DAMAGED);
        return E_CHECK;
    }
    
    ret = volume->Mount(0);
    if (ret != E_OK) {
        volume->SetState(UNMOUNTED);  // 回退状态
        return E_VOL_MOUNT_ERR;
    }
    
    volume->SetState(MOUNTED);
    NotifyVolumeMounted(volumeId);  // IPC 通知 Manager
    return E_OK;
}

int32_t VolumeManager::Instance().Unmount(const std::string &volumeId) {
    auto volume = GetVolume(volumeId);
    if (volume->GetState() != MOUNTED && 
        volume->GetState() != DAMAGED_MOUNTED) {
        return E_VOL_STATE;
    }
    
    volume->SetState(EJECTING);  // 必须经过 EJECTING
    int32_t ret = volume->UMount();
    volume->SetState(ret == E_OK ? REMOVED : MOUNTED);
    return ret;
}
```

## 文件系统类型差异

| 文件系统 | 挂载工具 | 格式化工具 | fsck 工具 | 超时 | 特殊参数 |
|----------|----------|------------|-----------|------|----------|
| ext4 | 内核 mount | `mke2fs -t ext4` | `e2fsck -y` | 60s | SELinux context |
| exFAT | `mount.exfat` | `mkexfatfs` | `fsck.exfat` | 60s | `uid=1000,gid=1000` |
| NTFS | `mount.ntfs` | 不支持 | `ntfsfix` | 60s | `rw,big_writes` |
| VFAT/FAT32 | 内核 mount | `mkfs.vfat` | `fsck.vfat -y` | 60s | `utf8,shortname=mixed` |
| HMFS | 内核 mount | - | - | - | 内部分区专用 |
| F2FS | 内核 mount | `mkfs.f2fs` | - | - | 内部 userdata |
| UDF | 内核 mount | - | - | - | 只读，光驱 |

### 挂载超时处理

```cpp
int32_t VolumeInfo::Mount(uint32_t flags) {
    std::vector<std::string> cmd = BuildMountCommand();
    
    // 60秒超时
    int32_t ret = ExecuteCommandWithTimeout(cmd, 60000);
    if (ret == E_TIMEOUT) {
        LOGE("Mount timeout for %{public}s", id_.c_str());
        SetState(UNMOUNTED);  // 回退状态
        return E_TIMEOUT_MOUNT;
    }
    return ret;
}
```

## 分区操作

### 获取分区表

```cpp
int32_t DiskInfo::GetPartitionTable(PartitionTableInfo &info) {
    // 使用 sgdisk 获取分区信息
    std::vector<std::string> output;
    ExecuteCommand({"sgdisk", "-p", devPath_}, output);
    
    // 解析输出
    for (const auto &line : output) {
        ParsePartitionLine(line, info);
    }
    return E_OK;
}
```

### 创建分区

```cpp
int32_t DiskInfo::CreatePartition(const PartitionParams &params) {
    // 验证参数
    if (!ValidatePartitionParams(params)) {
        return E_PARAMS_INVALID;
    }
    
    // 使用 sgdisk 创建分区
    std::vector<std::string> cmd = {
        "sgdisk", "-n", std::to_string(params.partitionNum),
        "-s", std::to_string(params.startSector),
        "-e", std::to_string(params.endSector),
        "-t", std::to_string(params.type),
        devPath_
    };
    
    int32_t ret = ExecuteCommand(cmd, 60000);  // 60秒超时
    if (ret != E_OK) {
        return E_CREATE_PARTITION_ERROR;
    }
    
    // 触发内核重读分区表
    RefreshPartitionTable();
    return E_OK;
}
```

## 典型场景：磁盘热插拔完整流程

```
用户插入 SD 卡
  └ 内核发送 netlink ADD 事件
    └ NetlinkHandler::OnEvent()
      └ 投递到工作线程
        DiskManager::HandleDiskEvent()
          DiskManager::CreateDisk()
            DiskInfo::Create()
              ├ 读取磁盘元数据（大小、类型、厂商）
              ├ NotifyDiskCreated() → IPC 通知 Manager
              │   Manager: DiskManagerService::OnDiskCreated()
              │     更新 diskMap_，通知上层应用
              └ ReadPartition()
                ├ 读取分区表（MBR/GPT）
                ├ 为每个分区创建 VolumeInfo
                └ 设置卷状态为 UNMOUNTED

用户拔出 SD 卡
  └ 内核发送 netlink REMOVE 事件
    DiskManager::DestroyDisk()
      ├ 卸载所有卷（如果已挂载）
      ├ 销毁所有 VolumeInfo
      ├ NotifyDiskDestroyed() → IPC 通知 Manager
      └ 清理资源
```

## 头文件路径

```
services/storage_daemon/include/disk/disk_manager.h     # DiskManager
services/storage_daemon/include/disk/disk_info.h        # DiskInfo
services/storage_daemon/include/volume/volume_manager.h # VolumeManager
services/storage_daemon/include/volume/volume_info.h    # VolumeInfo
services/storage_daemon/include/netlink/netlink_manager.h # NetlinkManager
services/storage_daemon/include/netlink/netlink_handler.h # NetlinkHandler
```

## 测试指引

- 磁盘生命周期：`DiskManagerTest`
- 卷生命周期：`VolumeManagerTest`
- 分区操作：`DiskInfoPartitionTest`
- 文件系统挂载：`ExternalVolumeInfoTest`
- 状态转换：`VolumeStateTest`
- 热插拔场景：需要板侧验证（真实 SD 卡/USB）