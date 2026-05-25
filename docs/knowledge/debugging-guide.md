# 调试指引

本文记录日志查看、状态检查和问题定位方法。

## 日志查看

### 查看存储服务日志

```sh
# 查看 Daemon 日志
hilog -t StorageDaemon

# 查看 Manager 日志
hilog -t StorageManager

# 查看所有存储相关日志
hilog | grep -E "StorageDaemon|StorageManager"

# 实时跟踪
hilog -r && hilog -T StorageDaemon -T StorageManager

# 按进程过滤
hilog -P storage_daemon
hilog -P storage_manager
```

### 日志级别过滤

```sh
# 只看错误日志
hilog -t StorageDaemon -L E

# 只看信息和警告
hilog -t StorageDaemon -L I -L W
```

## 状态检查

### 查看挂载状态

```sh
# 查看所有挂载
mount

# 查看外部存储挂载
mount | grep -E "sdcard|usb|hmdfs"

# 查看 FUSE 挂载
mount | grep fuse
```

### 查看用户目录

```sh
# 查看用户目录结构
ls -la /data/user/
ls -la /data/service/el{1,2,3,4}/

# 查看特定用户目录
ls -la /data/service/el2/100/
```

### 查看密钥状态

```sh
# 查看内核密钥（需要 root）
cat /proc/keys | grep fscrypt

# 查看密钥文件
ls -la /data/service/el1/public/storage_daemon/sd/
```

### 查看配额

```sh
# 查看配额设置
repquota -a

# 查看特定用户配额
quota -u <uid>
```

### 查看 SELinux context

```sh
# 查看目录 SELinux context
ls -Z /data/service/el2/

# 查看文件 SELinux context
ls -Z /data/user/100/
```

## 服务状态检查

```sh
# 检查 Manager SA 注册
hidumper -s 5003 -a "-h"

# 检查 Daemon SA 注册
hidumper -s 5004 -a "-h"

# 检查服务进程
ps -ef | grep storage

# 检查服务内存
hidumper -s 5003 -a "-m"
```

## 问题定位流程

### 服务未启动

1. 检查 SA 注册：`hidumper -s 5003 -a "-h"` (Manager) / `hidumper -s 5004 -a "-h"` (Daemon)
2. 检查进程：`ps -ef | grep storage`
3. 检查启动日志：`hilog -t StorageDaemon | grep "OnStart"`

### IPC 调用失败

1. 检查 Daemon 连接日志：`hilog -t StorageManager | grep "Connect"`
2. 确认代理有效：检查 `StorageDaemonCommunication::Connect()` 返回值
3. 检查 Daemon 进程状态：`ps -ef | grep storage_daemon`

### 密钥操作失败

1. 检查 HUKS 服务状态：`hidumper -s HUKS`
2. 检查密钥文件是否存在：`ls /data/service/el1/public/storage_daemon/sd/el1/`
3. 检查 TEE 环境（如启用）：检查 `RECOVER_KEY_TEE_ENVIRONMENT` 编译宏
4. 检查密钥日志：`hilog -t StorageDaemon | grep -E "GenerateUserKeys|ActiveUserKey"`

### 挂载失败

1. 检查设备节点权限：`ls -la /dev/block/`
2. 检查文件系统类型：`blkid /dev/block/sdcard`
3. 检查挂载路径合法性：确认路径不存在且可写
4. 检查挂载日志：`hilog -t StorageDaemon | grep -E "Mount|Check"`

### 用户目录异常

1. 检查用户 ID 范围：确认 userId 在 [100, 10738]
2. 检查目录权限：`ls -la /data/user/<userId>`
3. 检查 SELinux context：`ls -Z /data/user/<userId>`
4. 检查目录创建日志：`hilog -t StorageDaemon | grep -E "PrepareUserDirs|CreateEl"`

### 空间统计异常

1. 检查统计日志：`hilog -t StorageManager | grep -E "GetBundleStats|GetUserStorageStats"`
2. 检查 quota 设置：`repquota -a`
3. 检查 BundleManager 连接：确认服务可用

### 卷状态异常

1. 检查卷状态日志：`hilog -t StorageDaemon | grep "SetState"`
2. 检查卷状态：通过 `GetAllVolumes` 查询
3. 检查状态转换是否合法

## 常用诊断命令

```sh
# 查看磁盘分区
fdisk -l /dev/block/sdcard

# 查看文件系统信息
df -h

# 查看块设备信息
lsblk

# 查看内核日志
dmesg | grep -E "sd|mmc|usb"

# 查看进程打开的文件
lsof -p <pid>
```