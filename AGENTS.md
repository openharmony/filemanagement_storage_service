# 存储管理服务指引

## 项目定位

本仓库对应 OpenHarmony `foundation/filemanagement/storage_service`。优先按这些目录定位问题：

- `services/storage_daemon/`：底层守护进程，负责设备热插拔、分区/挂载/格式化、fscrypt 密钥管理、用户目录创建/销毁、配额管理。
- `services/storage_manager/`：系统 SA（ID 5003），负责卷/磁盘状态维护、用户生命周期、空间统计与监控、IPC 桥接到 Daemon（ID 5004）。
- `interfaces/innerkits/`、`interfaces/kits/`：Native 内部 API 和 JS/ETS 对外 API。
- `services/common/`：共享常量、日志宏、错误码定义。
- `test/`、`test/fuzztest/`：单元测试和 fuzz 目标。

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

```sh
./build.sh --product-name rk3568 --build-target storage_daemon --ccache
./build.sh --product-name rk3568 --build-target storage_manager --ccache
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 StorageDaemonProviderTest
```

涉及真实磁盘、加密、多用户或配额的行为，需要补充板侧证据。提交使用 `git commit -s`，并保留 `Co-Authored-By: Agent`。变更需通过单元测试和 fuzz 测试。

## 错误码与代码规范

### 命名规则

| 类型 | 规则 | 注意 |
|------|------|------|
| 类名 | 大驼峰 | `KeyManager`、`VolumeInfo` |
| 函数名 | 大驼峰 | `GenerateUserKeys`、`ActiveUserKey` |
| 变量名 | 小驼峰 | `userId`、`volumeId` |
| 成员变量 | 下划线结尾或小驼峰 | 源码中两种并存，修改时保持同文件一致 |
| 常量 | `extern const` | `MAX_USER_ID` 类型是 `int` 不是 `int32_t`；`EL1` 类型是 `std::string` 不是 `const char*` |
| 枚举 | 大驼峰类型 + 大写值 | `VolumeState` 是 `enum` 不是 `enum class`；`DiskInfo::Table` 是 `enum class` |

### 错误码选用

禁止使用 `E_ERR`（太笼统），必须使用对应类别的具体错误码：

| 场景 | 必须使用 | 禁止使用 |
|------|----------|----------|
| userId 范围无效 | `E_USERID_RANGE` | E_ERR |
| volumeId/diskId 不存在 | `E_NON_EXIST` | E_ERR |
| volume 状态不允许 | `E_VOL_STATE` | E_ERR |
| 目录创建失败 | `E_CREATE_DIR_RECURSIVE_FAILED` | E_ERR |
| 目录销毁失败 | `E_DESTROY_DIR` | E_ERR |
| 密钥存储失败 | `E_ELX_KEY_STORE_ERROR` | E_ERR |
| 密钥激活失败 | `E_ELX_KEY_ACTIVE_ERROR` | E_ERR |
| 权限不足 | `E_PERMISSION_DENIED` | E_ERR |
| 参数无效 | `E_PARAMS_INVALID` | E_ERR |

### 错误码范围

基础值 `STORAGE_SERVICE_SYS_CAP_TAG = 13600000`（定义于 `interfaces/innerkits/storage_manager/native/storage_service_errno.h`）：

| 范围 | 类别 | 示例 |
|------|------|------|
| 0 | 成功（`E_OK`） | — |
| 13600001-13600025 | 通用错误 | `E_PERMISSION_DENIED`、`E_NON_EXIST`、`E_PARAMS_INVALID` |
| 13600201-13600245 | 密钥管理 | `E_USERID_RANGE`、`E_ELX_KEY_STORE_ERROR`、`E_ELX_KEY_ACTIVE_ERROR` |
| 13600350-13600353 | 密钥完整性 | `E_DIR_INTEGRITY_ERR`、`E_KEY_SIZE_ERROR` |
| 13601201 | BundleManager | `E_BUNDLEMGR_ERROR` |
| 13601701-13601719 | 卷状态 | `E_VOL_STATE`、`E_TIMEOUT_MOUNT` |

### 日志必打节点

| 流程 | 必打节点 | 级别 |
|------|----------|------|
| 用户管理（Prepare/Start/Complete/Stop/Remove） | 开始、成功、失败 | LOGI/LOGE |
| 密钥管理（Generate/Active/Inactive/Lock/Unlock/Update） | 成功、失败 | LOGI/LOGE |
| 卷操作（Mount/Unmount/Format/TryToFix） | 开始、成功、失败 | LOGI/LOGE |
| 磁盘事件（添加/移除） | 设备信息、正常/异常移除 | LOGI/LOGW |
| IPC 连接 | 连接成功/失败、断开 | LOGI/LOGE/LOGW |

敏感信息禁止打印：不可在日志中出现密码、密钥数据、token 内容。只打印长度或状态。

### 编码约束

| 约束项 | 限制 |
|--------|------|
| 单个函数 | ≤ 50 行，超过则拆分 |
| 单个文件 | ≤ 2000 行，超过则按职责拆分 |

## 知识索引

稳定背景知识放在 `docs/knowledge/`。改动前按场景读取对应文件：

| 场景 | 先读 |
|------|------|
| 用户创建/删除、PrepareAddUser/StartUser/CompleteAddUser、EL 目录、多用户隔离、回滚 | `docs/knowledge/user-directory-lifecycle.md` |
| 卷 Mount/Unmount、EJECTING、磁盘热插拔、SD/USB/光驱/MTP 差异、挂载超时 | `docs/knowledge/disk-volume-lifecycle.md` |
| 密钥生成/激活、锁屏/解锁 EL2、V1/V2 兼容、App 密钥 | `docs/knowledge/crypto-key-management.md` |
| Manager-Daemon 职责、懒连接、崩溃恢复、IPC 新增流程 | `docs/knowledge/ipc-interface-guide.md` |
| 配额设置/查询、存储监控阈值、共享文件 ACL | `docs/knowledge/storage-quota-monitoring.md` |
| VolumeState 跳转、DiskType/KeyType 作用域、位掩码 | `docs/knowledge/enums-and-state-machine.md` |
| 代码放置位置、Instance/GetInstance 差异、头文件路径 | `docs/knowledge/architecture.md` |
| 实现必做步骤、各场景操作必做项 | `docs/knowledge/implementation-templates.md` |
| 测试覆盖要求、Mock 选择、Fuzz | `docs/knowledge/testing-guide.md` |
| 时序跳步、状态跳转、锁屏访问、跨用户、IPC 死锁 | `docs/knowledge/constraints-and-traps.md` |
| 日志查看、SA 状态检查、问题定位 | `docs/knowledge/debugging-guide.md` |
| 编译宏差异、HUKS/OsAccount 不可用 | `docs/knowledge/external-dependencies.md` |

## 项目约束

- 用户时序不可跳步：`PrepareAddUser → StartUser → CompleteAddUser`，缺步会导致 EL2-EL4 密钥未激活。
- 卷状态不可跳转：`MOUNTED → EJECTING → REMOVED`，必须经过 EJECTING，不可直接置 REMOVED。
- EL2 密钥锁屏移除、解锁安装：锁屏时不可访问 EL2 加密数据，必须在解锁后才操作。
- 用户隔离：不可用 userId=100 的密钥操作 userId=105 的目录，密钥和目录必须匹配。
- IPC 死锁预防：不要在持有 Manager 锁时调用 Daemon IPC，先释放锁再发起 IPC。
