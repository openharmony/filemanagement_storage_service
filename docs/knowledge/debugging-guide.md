# 调试指引

本文只记录存储服务的日志查看命令和问题定位流程。密钥异常见 `crypto-key-management.md`，IPC 故障见 `ipc-interface-guide.md`。

## 日志查看

```sh
hilog -t StorageDaemon              # Daemon 日志
hilog -t StorageManager             # Manager 日志
hilog -r && hilog -T StorageDaemon -T StorageManager  # 清空后实时跟踪
hilog -t StorageDaemon -L E         # 只看错误
hilog -P storage_daemon             # 按进程过滤
```

## 服务状态检查

```sh
hidumper -s 5003 -a "-h"            # Manager SA 注册
hidumper -s 5004 -a "-h"            # Daemon SA 注册
ps -ef | grep storage               # 进程状态
```

## 问题定位流程

### 服务未启动

1. `hidumper -s 5003 -a "-h"` / `hidumper -s 5004 -a "-h"` 检查 SA 注册
2. `ps -ef | grep storage` 检查进程
3. `hilog -t StorageDaemon | grep "OnStart"` 检查启动日志

### IPC 调用失败

1. `hilog -t StorageManager | grep "Connect"` 检查连接
2. 确认 `StorageDaemonCommunication::Connect()` 返回值
3. `ps -ef | grep storage_daemon` 确认进程存活

### 密钥操作失败

1. `hidumper -s HUKS` 检查 HUKS 服务
2. `ls /data/service/el1/public/storage_daemon/sd/el1/` 检查密钥文件
3. 检查 `RECOVER_KEY_TEE_ENVIRONMENT` 编译宏

### 挂载失败

1. `ls -la /dev/block/` 检查设备节点权限
2. `blkid /dev/block/sdcard` 检查文件系统类型
3. 确认挂载路径不存在且可写

### 用户目录异常

1. 确认 userId 在 [100, 10738]
2. `ls -la /data/user/<userId>` 检查目录权限
3. `ls -Z /data/user/<userId>` 检查 SELinux context

### 空间统计异常

1. `hilog -t StorageManager | grep "GetBundleStats"` 检查统计日志
2. `repquota -a` 检查配额设置
3. 确认 BundleManager 连接可用

## 常用诊断命令

```sh
mount | grep -E "sdcard|usb|hmdfs"   # 外部存储挂载
cat /proc/keys | grep fscrypt        # 内核密钥（需 root）
df -h                               # 文件系统空间
dmesg | grep -E "sd|mmc|usb"        # 内核设备日志
```
