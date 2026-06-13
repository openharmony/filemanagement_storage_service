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
| 错误码选用、日志节点、命名规则、50 行限制 | `docs/knowledge/code-conventions.md` |
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
- 错误码必须具体：禁止使用 E_ERR，必须使用对应类别的具体错误码（如 E_USERID_RANGE、E_VOL_STATE）。
- C++ 改动优先复用项目宏和返回约定：LOGI/LOGE/LOGW/LOGD、E_OK/E_PARAMS_INVALID、StorageRadar::Report*。
