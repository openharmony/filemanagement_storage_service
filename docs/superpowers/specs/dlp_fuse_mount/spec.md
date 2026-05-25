



# Spec: DLP FUSE 文件系统挂载/卸载 Inner 接口

## 概述

storage_service 部件新增两个 inner 接口供 DLP 服务使用：
- `MountDlpFuse(dstPath1: string, dstPath2: string) → int32` — 打开一个 `/dev/fuse` 节点获取 fd，将同一 FUSE 文件系统分别挂载到 dstPath1（主挂载）和 dstPath2（bind mount），通过 out 参数返回一个 fuse fd
- `UMountDlpFuse(dstPath1: string, dstPath2: string) → int32` — 先卸载 dstPath2（bind mount），再卸载 dstPath1（FUSE 主挂载）

接口校验 `ohos.permission.STORAGE_MANAGER` 权限及调用方 UID（3553）白名单，
路径前缀限定为 `/data/service/el1/public/dlp_credential_service/mounts/`。

## 用户故事或场景

### US-01: DLP 服务开机挂载 FUSE 目录

作为 DLP 凭证服务开发者，我希望在开机阶段通过 inner 接口将同一个 FUSE 文件系统挂载到两个目标目录（dstPath1 和 dstPath2 共享同一 FUSE 源），获取一个 fuse fd 后启动 FUSE daemon 处理凭证读写。

- AC-001: 调用 `MountDlpFuse("/data/service/el1/public/dlp_credential_service/mounts/dir1", "/data/service/el1/public/dlp_credential_service/mounts/dir2")` 成功返回，一个 fuse fd 可用，两个目录均可见 FUSE 文件系统内容
- AC-002: 权限未授权或 UID≠3553 调用 `MountDlpFuse` 返回 `E_PERMISSION_DENIED`

### US-02: DLP 服务卸载 FUSE 目录

作为 DLP 凭证服务开发者，我希望在业务结束时卸载已挂载的 FUSE 目录。

- AC-004: 调用 `UMountDlpFuse` 成功卸载已挂载的 FUSE 文件系统
- 对未挂载目录调用 `UMountDlpFuse` 返回 `E_UMOUNT_DLP_FUSE` 或对应错误码

### US-03: 路径安全校验

作为系统安全负责人，我需要确保 DLP FUSE 挂载仅限定在安全目录下，防止路径逃逸。

- AC-003: 路径前缀不匹配、路径含 `..` 或符号链接逃逸、路径为空字符串时返回 `E_PARAMS_INVALID`

### US-04: 产品形态隔离

作为 PC 产品构建负责人，我需要确保 DLP FUSE 功能仅在 PC 产品上编译和可用。

- AC-005: PC 产品编译时包含 `DLP_FUSE_SERVICE` 定义和相关代码；非 PC 产品不编译

## 验收标准

### 一、功能正确

| AC | 验收标准 | 优先级 |
|----|---------|--------|
| AC-001 | 真机/集成: UID=3553 + 权限已授权 → MountDlpFuse 成功挂载同一 FUSE 到 dstPath1 和 dstPath2，返回 fd > 0 且可读写 | P0 |
| AC-002 | 真机/集成: 挂载后通过 dstPath1 和 dstPath2 分别读取同一文件，内容一致，证明共享同一 FUSE 源 | P0 |
| AC-003 | 真机/集成: UMountDlpFuse 成功卸载，卸载后 `mount | grep dstPath` 无残留，目录恢复为普通目录 | P0 |
| AC-004 | 真机/集成: 卸载顺序验证 — 先 umount dstPath2 再 umount dstPath1，过程无 EBUSY 错误 | P0 |

### 二、性能满足要求

| AC | 验收标准 | 优先级 |
|----|---------|--------|
| AC-005 | 真机: MountDlpFuse 端到端耗时 ≤ 500ms（IPC + open("/dev/fuse") + mount + bind mount），测量 10 次取平均值 | P1 |
| AC-006 | 真机: UMountDlpFuse 端到端耗时 ≤ 200ms（IPC + umount2 × 2），测量 10 次取平均值 | P1 |

### 三、异常场景覆盖全面

| AC | 验收标准 | 优先级 |
|----|---------|--------|
| AC-007 | 单元/集成: bind mount 失败 → 自动回滚 FUSE 主挂载 + close(fd)，`mount | grep dstPath` 无残留，返回 E_MOUNT_DLP_FUSE | P0 |
| AC-008 | 单元: /dev/fuse 打开失败（mock open 返回 -1）→ 返回 E_OPEN_FUSE，无 fd 泄漏 | P0 |
| AC-009 | 集成: 对已挂载目录重复调用 MountDlpFuse → 返回 E_MOUNT_DLP_FUSE | P0 |
| AC-010 | 集成: 对未挂载目录调用 UMountDlpFuse → 返回 E_UMOUNT_DLP_FUSE | P0 |
| AC-011 | 单元: dstPath1 == dstPath2（相同路径）→ 返回 E_PARAMS_INVALID | P0 |
| AC-012a | 单元: 路径前缀不匹配 → 返回 E_PARAMS_INVALID | P0 |
| AC-012b | 单元: 路径含 `..` → 返回 E_PARAMS_INVALID | P0 |
| AC-012c | 单元: 路径含符号链接逃逸（symlink 指向 /etc/passwd）→ 返回 E_PARAMS_INVALID | P0 |
| AC-012d | 单元: dstPath1 或 dstPath2 为空字符串 → 返回 E_PARAMS_INVALID | P0 |
| AC-012e | 单元: 超长路径（> PATH_MAX=4096）→ 返回 E_PARAMS_INVALID | P1 |
| AC-012f | 单元: dstPath1 合法但 dstPath2 不合法 → 返回 E_PARAMS_INVALID（整体拒绝） | P0 |

### 四、权限校验通过

| AC | 验收标准 | 优先级 |
|----|---------|--------|
| AC-013 | 集成: 权限未授权 + UID=3553 → 返回 E_PERMISSION_DENIED | P0 |
| AC-014 | 集成: 权限已授权 + UID≠3553（如 UID=1000）→ 返回 E_PERMISSION_DENIED | P0 |
| AC-015 | 集成: 权限未授权 + UID≠3553 → 返回 E_PERMISSION_DENIED | P0 |
| AC-016 | 集成: 权限已授权 + UID=3553 → MountDlpFuse/UMountDlpFuse 成功（与 AC-001 合并验证） | P0 |

### 五、真机测试通过

| AC | 验收标准 | 优先级 |
|----|---------|--------|
| AC-017 | 真机(PC): 开机阶段完整流程 — 挂载→获取fd→FUSE daemon读写→卸载，无崩溃/卡死 | P0 |
| AC-018 | 真机(PC): SELinux `dlp_fuse:s0` 上下文生效，`dmesg | grep avc` 无 dlp_fuse 相关拒绝 | P0 |
| AC-019 | 构建: 非 PC 产品（如 rk3568）编译产物不含 DLP_FUSE_SERVICE 定义和相关代码 | P1 |
| AC-020 | 真机(PC): Demo 工具端到端验证 — 挂载两个目录→读写验证一致性→卸载→验证恢复 | P0 |

## 业务规则

- BR-001: 调用方必须同时满足权限校验（`ohos.permission.STORAGE_MANAGER`）和 UID 白名单校验（UID=3553），两者缺一不可
- BR-002: 目标路径前缀必须为 `/data/service/el1/public/dlp_credential_service/mounts/`，且需经过 symlink/路径逃逸校验
- BR-003: dstPath1 为 FUSE 主挂载点，dstPath2 通过 bind mount 指向 dstPath1，共享同一 FUSE 文件系统；同一目录不可重复挂载
- BR-004: 卸载时使用 `MNT_DETACH` 标志，确保即使有引用也能安全卸载
- BR-005: 仅 PC 产品形态 (`target_platform` 为 `pc`/`2in1`/`tablet`) 编译包含该功能

## 异常或边界规则

| 场景 | 行为 |
|------|------|
| 权限未授权 | 返回 `E_PERMISSION_DENIED` |
| UID 非 3553 | 返回 `E_PERMISSION_DENIED` |
| 路径前缀不匹配 | 返回 `E_PARAMS_INVALID` |
| 路径含 `..` 或符号链接逃逸 | 返回 `E_PARAMS_INVALID` |
| `/dev/fuse` 打开失败 | 返回 `E_OPEN_FUSE` |
| mount 系统调用失败 | 返回 `E_MOUNT_DLP_FUSE` |
| umount 失败 | 返回 `E_UMOUNT_DLP_FUSE` |
| 目标目录已有 FUSE 挂载 | 返回 `E_MOUNT_DLP_FUSE`（重复挂载拒绝） |
| dstPath1 或 dstPath2 为空字符串 | 返回 `E_PARAMS_INVALID` |
| dstPath1 == dstPath2（相同路径） | 返回 `E_PARAMS_INVALID` |
| dstPath1 合法但 dstPath2 不合法 | 返回 `E_PARAMS_INVALID`（整体拒绝，不执行挂载） |
| 超长路径（> PATH_MAX=4096） | 返回 `E_PARAMS_INVALID` |
| dstPath1 主挂载失败但 dstPath2 bind mount 尚未执行 | 返回 `E_MOUNT_DLP_FUSE`，不执行 bind mount，关闭已打开的 fd |
| bind mount 失败 | 回滚 FUSE 主挂载 + close(fd)，返回 `E_MOUNT_DLP_FUSE` |
| 对未挂载目录调用 UMountDlpFuse | 返回 `E_UMOUNT_DLP_FUSE` |
| 非 PC 产品形态调用（编译时排除） | 接口不存在，链接失败 |

## 错误码定义

| 错误码常量 | 数值 | 含义 |
|-----------|------|------|
| E_OK | 0 | 操作成功 |
| E_PERMISSION_DENIED | 已有 | 权限未授权或调用方 UID 不在白名单（3553）中 |
| E_PARAMS_INVALID | 已有 (SYS_CAP_TAG+2) | 路径前缀不匹配、路径含逃逸字符、路径为空字符串 |
| E_OPEN_FUSE | 已有 (SYS_CAP_TAG+745) | 打开 /dev/fuse 失败 |
| E_MOUNT_DLP_FUSE | 新增 (SYS_CAP_TAG+759) | DLP FUSE 挂载失败 |
| E_UMOUNT_DLP_FUSE | 新增 (SYS_CAP_TAG+760) | DLP FUSE 卸载失败 |

## 兼容性声明

- API 行为: 纯增量 inner 接口，不影响现有 MountFileMgrFuse / MountMediaFuse 行为
- 配置格式: 新增 GN feature flag `storage_service_dlp_fuse`，默认值跟随 `pc_device_enable`
- 数据存储格式: 无变化
- 最低支持版本: OpenHarmony-7.0-Release
- 编译隔离: `#ifdef DLP_FUSE_SERVICE` 守卫全部新增代码

## 验证映射

| ID | 规格项 | 验证方式 | 验证重点 |
|----|--------|---------|---------|
| V-001 | AC-001~AC-004 (功能正确) | 真机 + 集成测试 | 挂载成功/双目录一致性/卸载干净/卸载顺序正确 |
| V-002 | AC-005~AC-006 (性能) | 真机耗时测量 | Mount ≤500ms, UMount ≤200ms |
| V-003 | AC-007~AC-012 (异常场景) | 单元 + 集成测试 | 回滚干净/重复挂载/未挂载卸载/路径校验全场景 |
| V-004 | AC-013~AC-016 (权限校验) | 集成测试 | 四种权限+UID组合的正负向验证 |
| V-005 | AC-017~AC-020 (真机测试) | PC 真机端到端 | 开机流程/SELinux/构建隔离/Demo工具 |