# 枚举与状态机

本文记录核心枚举定义和状态转换规则。

## VolumeState（卷状态）

定义在 `services/storage_daemon/include/volume/volume_info.h:28`（注意：是 `enum`，不是 `enum class`，定义在命名空间作用域）：

```cpp
enum VolumeState {
    UNMOUNTED,           // 未挂载（初始状态）
    CHECKING,            // 正在检查文件系统
    MOUNTED,             // 已挂载
    EJECTING,            // 正在弹出
    REMOVED,             // 已移除（正常移除）
    BADREMOVABLE,        // 异常移除
    DAMAGED,             // 文件系统损坏
    FUSE_REMOVED,        // FUSE 设备移除
    DAMAGED_MOUNTED,     // 损坏但已挂载（TryToFix 后）
    ENCRYPTING,          // 正在加密
    ENCRYPTED_AND_LOCKED, // 已加密且锁定
    ENCRYPTED_AND_UNLOCKED, // 已加密且解锁
    DECRYPTING,          // 正在解密
};
```

### 合法状态转换路径

```
UNMOUNTED → CHECKING → MOUNTED → EJECTING → REMOVED
                   ↘ DAMAGED → DAMAGED_MOUNTED（TryToFix）
         ↘ BADREMOVABLE（异常拔出）

加密卷额外状态：
ENCRYPTING → ENCRYPTED_AND_LOCKED → ENCRYPTED_AND_UNLOCKED → DECRYPTING
```

### 状态转换规则

```cpp
// 合法转换
UNMOUNTED → CHECKING      // Mount 开始
CHECKING → MOUNTED        // Mount 成功
CHECKING → DAMAGED        // Check 失败
MOUNTED → EJECTING        // Unmount 开始
EJECTING → REMOVED        // Unmount 完成
MOUNTED → DAMAGED         // 使用中发现损坏
DAMAGED → DAMAGED_MOUNTED // TryToFix 成功

// 非法转换（禁止）
MOUNTED → REMOVED         // 必须经过 EJECTING
UNMOUNTED → MOUNTED       // 必须经过 CHECKING
```

## DiskType（磁盘类型）

定义在 `services/storage_daemon/include/disk/disk_info.h:32-38`（注意：是 `DiskInfo` 类的**内嵌枚举**，不是独立枚举，引用时需用 `DiskInfo::DiskType`）：

```cpp
class DiskInfo {
    enum DiskType {
        SD_CARD = 1,         // SD 卡：可分区、可格式化
        USB_FLASH = 2,       // USB 闪存：最多 16 分区
        CD_DVD_BD = 3,       // 光驱：只读挂载，支持弹出
        MTP_PTP = 4,         // MTP/PTP 设备：FUSE 挂载
        UNKNOWN_DISK_TYPE = 255,
    };
    // ...
};
```

### 各类型特殊处理

| 类型 | 分区支持 | 格式化支持 | 挂载方式 | 特殊处理 |
|------|----------|------------|----------|----------|
| SD_CARD | 支持 | 支持 | 内核 mount | 可分区、可格式化 |
| USB_FLASH | 支持（最多 16 分区） | 支持 | 内核 mount | 分区数限制 |
| CD_DVD_BD | 不支持 | 不支持 | 内核 mount | 只读、支持弹出 |
| MTP_PTP | 不支持 | 不支持 | FUSE (gphotofs) | 特殊挂载流程 |

## KeyType（密钥类型）

定义在 `services/common/include/storage_service_constant.h:71-78`：

```cpp
enum KeyType {
    EL0_KEY = 0,  // 设备级：/data/service/el0/，设备启动时激活
    EL1_KEY = 1,  // 设备级：/data/service/el1/，设备启动 + 用户 StartUser 时激活
    EL2_KEY = 2,  // 用户凭据保护：锁屏移除密钥，解锁重新安装
    EL3_KEY = 3,  // 用户凭据保护：CompleteAddUser 激活
    EL4_KEY = 4,  // 用户凭据保护：CompleteAddUser 激活
    EL5_KEY = 5,  // 应用级文件加密（UECE）：需要单独激活
};
```

### 加密标志位（位掩码）

```cpp
enum IStorageDaemonEnum {
    CRYPTO_FLAG_EL1 = 1,   // EL1 密钥
    CRYPTO_FLAG_EL2 = 2,   // EL2 密钥
    CRYPTO_FLAG_EL3 = 4,   // EL3 密钥
    CRYPTO_FLAG_EL4 = 8,   // EL4 密钥
    CRYPTO_FLAG_EL5 = 16,  // EL5 密钥
};

// 组合使用
uint32_t flags = CRYPTO_FLAG_EL1 | CRYPTO_FLAG_EL2 | CRYPTO_FLAG_EL3;
```

### 密钥激活时序

| KeyType | 激活时机 | 密钥生命周期 |
|---------|----------|--------------|
| EL0_KEY | 设备启动 | 常驻 |
| EL1_KEY | 设备启动 + StartUser | 用户运行期间常驻 |
| EL2_KEY | CompleteAddUser / UnlockUserScreen | 锁屏时移除，解锁时重新安装 |
| EL3_KEY | CompleteAddUser | 用户运行期间常驻 |
| EL4_KEY | CompleteAddUser | 用户运行期间常驻 |
| EL5_KEY | UECE 回调激活 | 应用控制 |

## DiskState（磁盘状态）

定义在 `services/storage_daemon/include/disk/disk_info.h:45-48`（注意：是 `DiskInfo` 类的**内嵌枚举**，引用时需用 `DiskInfo::DiskState`）：

```cpp
class DiskInfo {
    enum DiskState {
        MOUNTED,    // 磁盘已挂载
        REMOVED,    // 磁盘已移除
    };
    // ...
};
```

### 合法转换

```
MOUNTED → REMOVED  // 设备移除
```

## QuotaIdType（配额类型）

```cpp
enum QuotaIdType {
    USRID,   // 用户配额（按 UID）
    GRPID,   // 组配额（按 GID）
    PRJID,   // 项目配额（按项目 ID）
};
```