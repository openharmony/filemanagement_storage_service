# 磁盘与卷生命周期约束

本文只记录卷状态转换规则、文件系统差异边界和挂载操作约束。枚举定义和非法转换清单见 `enums-and-state-machine.md`，IPC 调用约束见 `ipc-interface-guide.md`。

## 状态转换约束

卷状态转换必须经过合法路径，不可跳步：

| 操作 | 必经路径 | 禁止跳转 |
|------|----------|----------|
| 挂载 | UNMOUNTED → CHECKING → MOUNTED | 禁止 UNMOUNTED → MOUNTED |
| 卸载 | MOUNTED → EJECTING → REMOVED | 禁止 MOUNTED → REMOVED |
| 修复 | DAMAGED → DAMAGED_MOUNTED（TryToFix） | 禁止 DAMAGED → MOUNTED |
| 异常拔出 | MOUNTED → BADREMOVABLE | 无需经过 EJECTING |

状态预更新原则：Mount 前先置 CHECKING，Unmount 前先置 EJECTING。操作失败时回退到操作前状态，不要停留在中间状态。

## Netlink 事件约束

Netlink 回调中不可执行阻塞操作（mount、fsck、格式化），必须投递到工作线程后再处理。

## 磁盘类型处理边界

| 类型 | 分区限制 | 格式化限制 | 挂载方式 | 不可折叠进通用路径的规则 |
|------|----------|------------|----------|--------------------------|
| SD_CARD | ≤8 分区 | 支持 | 内核 mount | 分区/格式化逻辑 |
| USB_FLASH | ≤16 分区 | 支持 | 内核 mount | 分区数限制、60s 超时 |
| CD_DVD_BD | 不支持 | 不支持 | 内核 mount | 只读、Eject 弹出 |
| MTP_PTP | 不支持 | 不支持 | FUSE (gphotofs) | 特殊挂载流程，不走内核 mount |

不要把不同磁盘类型的处理规则折叠进通用路径，每种类型的限制必须保持显式。

## 文件系统差异边界

| 文件系统 | 挂载超时 | fsck 要求 | 特殊约束 |
|------|:------:|------|------|
| ext4 | 60s | `e2fsck -y` 挂载前检查 | SELinux context |
| exFAT | 60s | `fsck.exfat` | `uid/gid` 参数必须设置 |
| NTFS | 60s | `ntfsfix`（非真正 fsck） | 不支持格式化 |
| VFAT/FAT32 | 60s | `fsck.vfat -y` | `utf8,shortname=mixed` |
| HMFS | 无 | 无 | 内部分区专用 |

挂载超时 60s 后必须回退状态并返回 `E_TIMEOUT_MOUNT`，不要停留在 CHECKING。

## 修改前检查

- 卷当前处于什么状态？转换路径是否合法？
- 磁盘类型是什么？是否应用了类型特定限制？
- Netlink 处理是否在工作线程？
- 挂载失败是否回退了状态？

## 测试指引

- 状态转换：`VolumeManagerTest`，必须覆盖非法跳转拒绝
- 磁盘热插拔：需要板侧验证（真实 SD 卡/USB）
