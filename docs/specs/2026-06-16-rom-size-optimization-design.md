# storage_service ROM体积优化设计方案

## 背景

storage_service 当前 bundle.json 声明 ROM 为 4096KB。分析发现，JS/NAPI kits 层库已使用 `-Oz`、`-flto`、`-ffunction-sections` 等体积优化编译选项，而 storage_manager 和 storage_daemon 主产物仅使用 `-O2`（性能优化），缺少体积优化选项。本方案通过统一编译优化级别来减小二进制体积。

## 优化目标

为 storage_manager、storage_daemon 及相关内部库添加与 kits 层一致的体积优化编译和链接选项，预期减少 15-30% 二进制体积。

## 变更范围

需要修改的 BUILD.gn 文件（共 6 个）：

| 文件 | 产物类型 | 产物名 |
|------|---------|--------|
| `services/storage_manager/BUILD.gn` | shared lib | storage_manager |
| `services/storage_daemon/BUILD.gn` (storage_daemon target) | executable | storage_daemon |
| `services/storage_daemon/BUILD.gn` (storage_common_utils target) | shared lib | storage_common_utils |
| `services/storage_daemon/crypto/BUILD.gn` | static lib | libsdcrypto |
| `interfaces/innerkits/storage_manager/native/BUILD.gn` (sa_proxy target) | shared lib | storage_manager_sa_proxy |
| `interfaces/innerkits/acl/native/BUILD.gn` | shared lib | storage_manager_acl |

不需要修改的（已使用优化选项）：NAPI kits（storagestatistics/volumemanager/keymanager/encryptedvolumemanager）、Taihe kits（storage_statistics/key_manager/volume_manager）、CJ FFI（cj_storage_manager_ffi）

## 具体变更

### 1. cflags 变更

所有 6 个产物的 cflags 统一从 `-O2` 替换为体积优化选项集：

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

说明：
- `-Oz` 替换 `-O2`：最大化体积优化，对存储管理非实时路径性能影响极小
- `-ffunction-sections` + `-fdata-sections`：配合 `--gc-sections` 消除未引用的函数和数据
- `-fvisibility=hidden`：隐藏非导出符号，减少符号表体积
- `-flto`：链接时优化，跨模块消除死代码
- `-fno-unwind-tables`：去除展开表，减少 5-10% 体积（影响栈回溯可读性，DFX需评估）

### 2. ldflags 变增

**storage_manager**（当前无任何链接优化选项）：

```gn
ldflags = [
  "-flto",
  "-Wl,--gc-sections",
  "-Wl,-O1",
  "-Wl,-z,max-page-size=4096",
  "-Wl,-z,separate-code",
]
```

**storage_daemon**（已有部分链接选项，补充 `-flto`）：

```gn
ldflags = [
  "-flto",
  "-Wl,--gc-sections",
  "-Wl,-O1",
  "-Wl,-z,max-page-size=4096",
  "-Wl,-z,separate-code",
]
```

**storage_common_utils / storage_manager_sa_proxy / storage_manager_acl**（共享库）：

```gn
ldflags = [
  "-flto",
  "-Wl,--gc-sections",
  "-Wl,-O1",
]
```

**libsdcrypto**（静态库）：仅修改 cflags，无需 ldflags（最终链接选项由消费方决定）。

### 3. sdc 工具

sdc 可执行文件也需同步添加相同的 cflags 和 ldflags 变更。

### 4. 不变更项

- `-D_FORTIFY_SOURCE=2` 和 `-fstack-protector-strong`：安全选项保持不变
- sanitize 配置：CFI/UBSAN/integer_overflow 保持不变
- branch_protector_ret = "pac_ret"：保持不变
- feature flag 条件编译逻辑：不变

## 风险评估

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| `-Oz` 性能差异 | 非实时路径影响极小（<5%） | 存储管理无实时性能要求 |
| `-fno-unwind-tables` 影响崩溃分析 | 栈回溯信息缺失 | 如DFX团队有顾虑可先不加此选项 |
| `-flto` 编译时间增加 | 约增加 20-30% 编译时间 | 已有 ccache 缓存缓解 |
| `-fvisibility=hidden` 导出符号缺失 | 需确认 SA 注册和 IPC 代理导出 | SA 使用 IDL 自动生成，导出符号由 IDL 框架处理 |

## 验证方法

### 编译验证

```sh
./build.sh --product-name rk3568 --build-target storage_daemon --ccache
./build.sh --product-name rk3568 --build-target storage_manager --ccache
```

### 体积对比

```sh
size out/rk3568/storage_daemon/storage_daemon
size out/rk3568/storage_manager/libstorage_manager.z.so
size out/rk3568/libstorage_common_utils.z.so
```

对比优化前后 text/data/bss/total 大小。

### 功能验证

```sh
./build.sh --product-name rk3568 --build-target storage_daemon_unit_test --ccache
./build.sh --product-name rk3568 --build-target storage_manager_unit_test --ccache
```

运行单元测试确保功能不回归。需板侧验证 SA 注册和 IPC 正常工作。

## 预期收益

| 产物 | 预期体积减少 |
|------|-------------|
| storage_daemon (executable) | 15-25% |
| storage_manager (shared lib) | 20-30% |
| storage_common_utils (shared lib) | 15-25% |
| libsdcrypto (→ 体现在 daemon) | 15-20% |
| storage_manager_sa_proxy (shared lib) | 15-25% |
| storage_manager_acl (shared lib) | 15-20% |

综合预期：storage_service ROM 占用从 4096KB 减少至约 2800-3400KB。
