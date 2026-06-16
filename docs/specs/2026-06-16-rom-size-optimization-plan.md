# storage_service ROM体积优化 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 storage_manager 和 storage_daemon 及相关内部库添加体积优化编译/链接选项，统一与 kits 层优化级别。

**Architecture:** 修改 6 个 BUILD.gn 文件的 cflags 和 ldflags，将 `-O2` 替换为 `-Oz` 并补充 `-flto`、`-ffunction-sections`、`-fdata-sections`、`-fvisibility=hidden`、`-fno-unwind-tables` 等体积优化选项；补充 ldflags 中的 `-flto`、`-Wl,--gc-sections`、`-Wl,-O1`。

**Tech Stack:** OpenHarmony GN build system, C/C++ compiler flags optimization

---

### Task 1: 修改 storage_manager BUILD.gn

**Files:**
- Modify: `services/storage_manager/BUILD.gn:30-34` (cflags)
- Modify: `services/storage_manager/BUILD.gn` (add ldflags block)

- [ ] **Step 1: 修改 cflags**

将 `storage_manager_config` 中的 cflags 从 `-O2` 替换为体积优化选项集：

```gn
  cflags = [
    "-D_FORTIFY_SOURCE=2",
    "-fstack-protector-strong",
    "-Oz",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

- [ ] **Step 2: 添加 ldflags**

在 `ohos_shared_library("storage_manager")` 的 `configs` 之前添加 ldflags：

```gn
  ldflags = [
    "-flto",
    "-Wl,--gc-sections",
    "-Wl,-O1",
    "-Wl,-z,max-page-size=4096",
    "-Wl,-z,separate-code",
  ]
```

- [ ] **Step 3: 验证语法**

确认 GN 文件语法无误，`cflags` 和 `ldflags` 位置正确（cflags 在 config block 内，ldflags 在 ohos_shared_library block 内）。

---

### Task 2: 修改 storage_daemon BUILD.gn (storage_daemon executable)

**Files:**
- Modify: `services/storage_daemon/BUILD.gn:154-157` (cflags)
- Modify: `services/storage_daemon/BUILD.gn:322-327` (ldflags)

- [ ] **Step 1: 修改 cflags**

将 `ohos_executable("storage_daemon")` 的 cflags 从 `-O2` 替换为体积优化选项集：

```gn
  cflags = [
    "-fstack-protector-strong",
    "-Oz",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

注意：原 cflags 没有 `-D_FORTIFY_SOURCE=2`，保持与原文件一致不额外添加。

- [ ] **Step 2: 补充 ldflags**

在现有 ldflags 中添加 `-flto`：

```gn
  ldflags = [
    "-flto",
    "-Wl,--gc-sections",
    "-Wl,-O1",
    "-Wl,-z,max-page-size=4096",
    "-Wl,-z,separate-code",
  ]
```

---

### Task 3: 修改 storage_daemon BUILD.gn (storage_common_utils shared lib)

**Files:**
- Modify: `services/storage_daemon/BUILD.gn:430-434` (cflags in storage_common_utils)
- Add ldflags to storage_common_utils

- [ ] **Step 1: 修改 cflags**

将 `ohos_shared_library("storage_common_utils")` 的 cflags 替换为体积优化选项集：

```gn
  cflags = [
    "-D_FORTIFY_SOURCE=2",
    "-fstack-protector-strong",
    "-Oz",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

- [ ] **Step 2: 添加 ldflags**

在 `ohos_shared_library("storage_common_utils")` 中添加 ldflags（放在 cflags 之后）：

```gn
  ldflags = [
    "-flto",
    "-Wl,--gc-sections",
    "-Wl,-O1",
  ]
```

---

### Task 4: 修改 storage_daemon BUILD.gn (sdc executable)

**Files:**
- Modify: `services/storage_daemon/BUILD.gn:367-369` (cflags in sdc)
- Add ldflags to sdc

- [ ] **Step 1: 修改 cflags**

将 `ohos_executable("sdc")` 的 cflags 替换为体积优化选项集：

```gn
  cflags = [
    "-fstack-protector-strong",
    "-Oz",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

- [ ] **Step 2: 添加 ldflags**

在 `ohos_executable("sdc")` 中添加 ldflags：

```gn
  ldflags = [
    "-flto",
    "-Wl,--gc-sections",
    "-Wl,-O1",
  ]
```

---

### Task 5: 修改 crypto BUILD.gn (libsdcrypto static lib)

**Files:**
- Modify: `services/storage_daemon/crypto/BUILD.gn:27-31` (cflags)

- [ ] **Step 1: 修改 cflags**

将 `config("storage_daemon_crypto_config")` 的 cflags 替换为体积优化选项集：

```gn
  cflags = [
    "-Oz",
    "-Wall",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

注意：原 cflags 有 `-g3`（调试符号），体积优化时应去掉。保留 `-Wall`。原无 `-D_FORTIFY_SOURCE=2`，保持不额外添加。

---

### Task 6: 修改 storage_manager_sa_proxy BUILD.gn

**Files:**
- Modify: `interfaces/innerkits/storage_manager/native/BUILD.gn:119-121` (cflags in sa_proxy)
- Add ldflags to sa_proxy

- [ ] **Step 1: 修改 cflags**

将 `ohos_shared_library("storage_manager_sa_proxy")` 的 cflags 替换为体积优化选项集：

```gn
  cflags = [
    "-D_FORTIFY_SOURCE=2",
    "-fstack-protector-strong",
    "-Oz",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

- [ ] **Step 2: 添加 ldflags**

在 `ohos_shared_library("storage_manager_sa_proxy")` 中添加 ldflags：

```gn
  ldflags = [
    "-flto",
    "-Wl,--gc-sections",
    "-Wl,-O1",
  ]
```

---

### Task 7: 修改 storage_manager_acl BUILD.gn

**Files:**
- Modify: `interfaces/innerkits/acl/native/BUILD.gn:54-56` (cflags in acl)
- Add ldflags to acl

- [ ] **Step 1: 修改 cflags**

将 `ohos_shared_library("storage_manager_acl")` 的 cflags 替换为体积优化选项集：

```gn
  cflags = [
    "-D_FORTIFY_SOURCE=2",
    "-fstack-protector-strong",
    "-Oz",
    "-ffunction-sections",
    "-fdata-sections",
    "-fvisibility=hidden",
    "-flto",
    "-fno-unwind-tables",
  ]
```

- [ ] **Step 2: 添加 ldflags**

在 `ohos_shared_library("storage_manager_acl")` 中添加 ldflags：

```gn
  ldflags = [
    "-flto",
    "-Wl,--gc-sections",
    "-Wl,-O1",
  ]
```

---

### Task 8: 提交并创建PR

- [ ] **Step 1: 检查变更**

```bash
git diff --stat
```

确认仅修改了 6 个 BUILD.gn 文件。

- [ ] **Step 2: 提交变更**

```bash
git add -A
git commit -s -m "feat: add size optimization flags for storage_manager and storage_daemon

Add -Oz, -flto, -ffunction-sections, -fdata-sections, -fvisibility=hidden,
-fno-unwind-tables cflags and corresponding ldflags to align with
existing kits layer optimization level. Expected 15-30% binary size reduction.
"
```

- [ ] **Step 3: 创建PR**

使用 quick-pr skill 创建 PR。
