# 约束与陷阱

本文只记录项目硬性约束和最易犯的 6 个陷阱。各场景详细约束见对应知识文档：用户时序见 `user-directory-lifecycle.md`，卷状态见 `disk-volume-lifecycle.md`，密钥见 `crypto-key-management.md`，IPC 死锁见 `ipc-interface-guide.md`。

## 硬性约束

- 用户时序不可跳步：`PrepareAddUser → StartUser → CompleteAddUser`。
- 卷状态不可跳转：`MOUNTED → EJECTING → REMOVED`。
- EL2 密钥锁屏移除、解锁安装，锁屏时不可访问 EL2 数据。
- 用户隔离：密钥和目录的 userId 必须匹配。
- 错误码必须具体：禁止 E_ERR。

## 6 个常见陷阱

### 陷阱 1：用户目录创建跳步

跳过 StartUser 直接 CompleteAddUser → EL2-EL4 密钥未激活，加密数据不可访问。

### 陷阱 2：卷状态非法跳转

从 MOUNTED 直接置 REMOVED → 内核可能仍持有引用，必须经过 EJECTING。

### 陷阱 3：Daemon 崩溃后 IPC 状态假设

Daemon 重启后内存状态为空，Manager 的缓存可能过期。必须重新同步状态，不要假设之前的 IPC 结果仍有效。

### 陷阱 4：锁屏时访问 EL2 数据

`LockUserScreen` 后 EL2 密钥已从内核移除。此后访问 EL2 加密文件会失败，必须等 `UnlockUserScreen` 完成。

### 陷阱 5：跨用户密钥操作

用 userId=100 的密钥操作 userId=105 的目录 → 密钥不匹配，数据解密失败。密钥和目录的 userId 必须一致。

### 陷阱 6：持有锁时执行 IPC

在持有 Manager 锁时调用 Daemon IPC → 可能双进程互锁。先释放锁再发起 IPC。

## 修改前检查

对照上述 6 个陷阱，当前改动是否触犯了任何一个？
