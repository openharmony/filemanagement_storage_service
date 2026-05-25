---
target_release: OpenHarmony-7.0-Release
---

# Proposal: DLP FUSE 文件系统挂载/卸载 Inner 接口

## 背景与问题

DLP（数据防泄漏）服务需要在开机阶段挂载 FUSE 文件系统，用于沙箱化 DLP 凭证数据的读写访问。
当前 storage_service 部件已提供 MountMediaFuse、MountFileMgrFuse 等 FUSE 挂载接口，但这些接口面向
媒体库和文件管理器场景，路径前缀、SELinux 上下文、权限管控策略均与 DLP 需求不匹配。

DLP 服务的特殊性在于：
- 调用方 UID 固定（3553），需要白名单级管控而非通用权限授权
- 目标目录前缀必须限定在 `/data/service/el1/public/dlp_credential_service/mounts/`，防止路径逃逸
- 仅 PC 产品形态需要该功能，需通过 feature 编译隔离

## 目标

为 DLP 服务提供专用的 FUSE 文件系统挂载/卸载 inner 接口：
- `MountDlpFuse(dstPath1, dstPath2)` — 打开一个 FUSE 节点（/dev/fuse），将同一 FUSE 文件系统挂载到两个目标目录，返回一个 fuse fd
- `UMountDlpFuse(dstPath1, dstPath2)` — 卸载两个 FUSE 挂载点（先卸载 bind mount，再卸载 FUSE 主挂载）
- 权限校验：`ohos.permission.STORAGE_MANAGER` + 调用方 UID（3553）白名单
- 路径校验：目标目录前缀必须为 `/data/service/el1/public/dlp_credential_service/mounts/`
- 仅 PC 产品形态编译生效，通过 GN feature flag 隔离

## 非目标

- 不修改现有 MountFileMgrFuse / MountMediaFuse 接口行为
- 不实现 FUSE daemon 运行逻辑（由 DLP 服务自身在拿到 fd 后运行）
- 不扩展为通用 FUSE 挂载框架（本接口专为 DLP 场景设计）

## 范围

| 层 | 变更 |
|----|------|
| IDL | `IStorageManager.idl` 新增 `MountDlpFuse` / `UMountDlpFuse` |
| SA层 | `StorageManagerProvider` 实现权限+UID校验，委托 `StorageDaemonClient` |
| Daemon层 | `MountManager` 新增 `MountDlpFuseDevice` / `UMountDlpFuseDevice` 实现 |
| Innerkits | `storage_manager_sa_proxy` 新增代理方法 |
| GN/Feature | `storage_service_aafwk.gni` 新增 `storage_service_dlp_fuse` flag |
| 影响子系统 | filemanagement |

## 验收基线

### 功能正确

- AC-001: DLP 服务（UID=3553 + `ohos.permission.STORAGE_MANAGER`）调用 `MountDlpFuse` 成功将同一个 FUSE 文件系统挂载到两个目标目录，返回一个有效 fuse fd（fd > 0，可读写）
- AC-002: 挂载后 dstPath1 和 dstPath2 均可见 FUSE 文件系统内容，且内容一致（同一 FUSE 源）
- AC-003: `UMountDlpFuse` 成功卸载已挂载的 FUSE 文件系统，卸载后 dstPath1 和 dstPath2 恢复为普通目录
- AC-004: 卸载顺序正确：先 dstPath2（bind mount）后 dstPath1（FUSE 主挂载），无 EBUSY 残留

### 性能满足要求

- AC-005: 单次 `MountDlpFuse` 调用端到端耗时 ≤ 500ms（含 IPC 跨进程 + open + mount + bind mount）
- AC-006: 单次 `UMountDlpFuse` 调用端到端耗时 ≤ 200ms

### 异常场景覆盖全面

- AC-007: bind mount 失败时自动回滚 FUSE 主挂载 + 关闭 fd，不留部分挂载残留
- AC-008: `/dev/fuse` 打开失败返回 `E_OPEN_FUSE`
- AC-009: 对已挂载目录重复调用 `MountDlpFuse` 返回 `E_MOUNT_DLP_FUSE`
- AC-010: 对未挂载目录调用 `UMountDlpFuse` 返回 `E_UMOUNT_DLP_FUSE`
- AC-011: dstPath1 == dstPath2（相同路径）返回 `E_PARAMS_INVALID`
- AC-012: 路径含 `..`、符号链接逃逸、空字符串、超长路径均返回 `E_PARAMS_INVALID`

### 权限校验通过

- AC-013: 权限未授权 + UID=3553 → 返回 `E_PERMISSION_DENIED`
- AC-014: 权限已授权 + UID≠3553 → 返回 `E_PERMISSION_DENIED`
- AC-015: 权限未授权 + UID≠3553 → 返回 `E_PERMISSION_DENIED`
- AC-016: 权限已授权 + UID=3553 → 挂载成功（正常路径）

### 真机测试通过

- AC-017: PC 真机上开机阶段完整走通挂载→获取fd→读写→卸载流程
- AC-018: PC 真机 SELinux 上下文 `dlp_fuse:s0` 正常生效，无 AVC 拒绝
- AC-019: 非 PC 产品编译不包含 DLP FUSE 相关代码和 feature flag
- AC-020: Demo 工具在 PC 真机上端到端验证通过

## 不涉及项确认

| 维度 | 是否涉及 | 说明 |
|------|---------|------|
| 性能 | 低风险 | FUSE 挂载为低频操作（开机阶段一次），不涉及性能热路径 |
| 安全/权限 | 是 | 新增 UID 白名单校验 + 路径前缀校验，防止未授权挂载和路径逃逸 |
| 兼容性 | 否 | 纯增量 inner 接口，不影响现有接口行为 |
| API/SDK | 否 | 仅新增 inner 接口，不涉及公开 JS/NAPI API |
| IPC/跨进程 | 是 | storage_manager → storage_daemon IPC 需新增方法 |
| 构建/组件 | 是 | 新增 GN feature flag `storage_service_dlp_fuse` |
| 国际化/无障碍 | 否 | 挂载逻辑与语言无关 |
| 数据迁移 | 否 | 无持久化数据 |