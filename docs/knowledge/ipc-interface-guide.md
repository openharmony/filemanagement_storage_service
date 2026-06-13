# IPC 接口约束

本文只记录 Manager-Daemon 双进程架构的交互约束和新增接口规则。密钥时序见 `crypto-key-management.md`，卷约束见 `disk-volume-lifecycle.md`，线程死锁见 `constraints-and-traps.md`。

## 双进程职责边界

| 进程 | 职责 | 不可越界 |
|------|------|----------|
| Manager (SA 5003) | 权限校验、状态缓存、监控通知、IPC 桥接 | 不可直接操作设备节点或密钥 |
| Daemon (SA 5004) | 设备操作、密钥管理、文件系统操作、用户目录 | 不可直接响应应用请求 |

Manager 不操作设备，Daemon 不响应应用，职责不可交叉。

## 启动顺序约束

Daemon 必须先启动（内核 netlink 事件需先就绪），Manager 后启动。Manager 按需连接 Daemon（懒连接），不要假设 Manager 启动时 Daemon 已就绪。

## 死锁预防

不要在持有 Manager 锁时调用 Daemon IPC。IPC 调用是同步的，持有锁发起 IPC 可能导致双进程互锁。先释放锁再发起 IPC。

## 崩溃恢复约束

Daemon 崩溃后 Manager 会锁屏降级并重建连接。不要假设崩溃前的 IPC 状态仍有效——Daemon 重启后内存状态（卷映射、密钥缓存）为空，Manager 的缓存可能过期，必须重新同步。

## 权限校验

Manager 入口必须校验权限（`CheckClientPermission`），Daemon 入口必须校验路径合法性（`ValidateBlockDevicePath`、`ValidateMountPath`）。两层校验不可绕过。

## 新增 IPC 接口规则

| 步骤 | 修改位置 | 必须包含 |
|------|----------|----------|
| 1. 定义接口 | `istorage_manager.h` | 虚函数声明 |
| 2. Stub 解析 | `storage_manager_stub.cpp` | MessageParcel 读取 + 参数校验 |
| 3. Manager 实现 | `StorageManagerProvider` | 权限校验 + 调用 Daemon |
| 4. Daemon Stub | `storage_daemon_stub.cpp` | MessageParcel 读取 + 参数校验 |
| 5. Daemon 实现 | `StorageDaemonProvider` → 核心类 | 参数校验 + 状态检查 + 操作 |
| 6. 测试 | 对应模块 test + fuzztest | Stub/Provider/Fuzz 全覆盖 |

不可只改一侧。Manager 和 Daemon 必须同时修改，Stub 必须做参数校验。

## 修改前检查

- 当前操作是否在持有锁时发起 IPC？
- 崩溃恢复后是否重新同步了状态？
- 两层权限/路径校验是否都保留？
- 新增接口是否覆盖了双侧 Stub + 测试？

## 测试指引

- Stub 解析：`StorageManagerStubTest`、`StorageDaemonStubTest`
- Provider：`StorageManagerProviderTest`、`StorageDaemonProviderTest`
- 崩溃恢复：需要板侧验证（kill storage_daemon 进程）
