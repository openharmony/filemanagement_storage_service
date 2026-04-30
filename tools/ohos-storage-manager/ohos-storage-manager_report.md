# ohos-storageManager 接口封装报告

## 概述

`ohos-storageManager` 是一个为 OpenHarmony storage_service 组件封装的命令行工具，用于提供存储管理能力的 shell 可调用接口。

**目标用户**：
- 系统运维人员
- 自动化测试脚本
- AI Agent（如 Claw）
- 开发调试人员

**使用场景**：
- 存储空间查询与监控
- 应用存储统计

## 接口来源

### 来源头文件路径

| 类型 | 路径 |
|------|------|
| IDL 定义 | `services/storage_manager/IStorageManager.idl` |
| 数据类型头文件 | `interfaces/innerkits/storage_manager/native/` |
| Proxy 实现 | `interfaces/innerkits/storage_manager/native:storage_manager_sa_proxy` |

### 选择的接口层级及原因

选择 **IStorageManager SA Proxy** 层级进行封装，原因如下：

1. **同步调用支持**：该层提供的接口可以通过 SA Proxy 同步调用，适合一次性 CLI 执行
2. **完整的业务能力**：覆盖存储统计、卷管理、磁盘管理等核心运维能力
3. **标准化的错误处理**：返回标准错误码，便于 CLI 输出统一的 JSON 响应
4. **已导出的 inner_kits**：`storage_manager_sa_proxy` 是已导出的平台 SDK 接口，稳定性有保障

## CLI 子命令映射表

| CLI 子命令 | 对应接口 | 参数说明 | 返回值 |
|-----------|---------|---------|--------|
| `get-total-size` | `IStorageManager::GetTotalSize()` | 无 | total_bytes, total_readable |
| `get-free-size` | `IStorageManager::GetFreeSize()` | 无 | free_bytes, free_readable |
| `get-system-size` | `IStorageManager::GetSystemSize()` | 无 | system_bytes, system_readable |
| `get-user-storage-stats` | `IStorageManager::GetUserStorageStats()` | [userId] (可选) | total, audio, video, image, file, app |
| `get-bundle-stats` | `StorageManagerConnect::GetBundleStats()` | packageName, [appIndex] | appSize, cacheSize, dataSize |
| `get-current-bundle-stats` | `StorageManagerConnect::GetCurrentBundleStats()` | 无 | appSize, cacheSize, dataSize |

## 未封装接口

| 接口名称 | 未封装原因 |
|---------|-----------|
| `GetAllVolumes` | 需要卷管理权限，场景特定 |
| `GetVolumeById` | 需要卷管理权限，场景特定 |
| `Mount` | 涉及系统挂载操作，需特殊权限 |
| `Unmount` | 涉及系统卸载操作，需特殊权限 |
| `Format` | 磁盘格式化操作，具有破坏性 |
| `GetBundleStats` | **(已封装)** 参数较复杂，场景特定，现已支持 |
| `GetCurrentBundleStats` | **(已封装)** 现已支持 |
| `PrepareAddUser` | 系统内部接口，需要特殊权限 |
| `RemoveUser` | 系统内部接口，需要特殊权限 |

## 异步接口分析

### 上层接口

IStorageManager IDL 接口定义中，大部分接口为同步接口。以下接口标记为 oneway：
- `LockUserScreen` - oneway 异步
- `DeleteShareFile` - oneway 异步

### 调用链关键节点

```
CLI Handler -> GetStorageManagerProxy() -> iface_cast<IStorageManager>() -> proxy->Method()
```

所有封装的接口都是同步调用 SA Proxy，通过 IPC 与 StorageManager 服务通信。

## Claw 规范落实情况

### 命名与调用风格

- [x] 使用统一前缀 `ohos-storage-manager`
- [x] 子命令使用动词-名词结构
- [x] 参数只提供长选项（如 `--volume-id`、`--user-id`）
- [x] 禁止短选项

### 输入管道规范

- [x] 参数保持标量化
- [x] 不要求嵌套 JSON 字符串参数
- [x] 长文本通过常规参数传递

### 输出管道规范

- [x] stdout 只输出一行合法 JSON
- [x] 成功返回 0，失败返回非 0
- [x] 日志和调试信息输出到 stderr

### 统一 JSON 响应契约

- [x] 成功响应：`{"success": true, "data": {...}}`
- [x] 失败响应：`{"success": false, "error": {"code": ..., "message": ...}}`
- [x] 失败时提供 suggestion 字段引导用户排查

### 行为与状态约束

- [x] 命令设计为幂等
- [x] 不进行交互式确认

### 长耗时任务范式

当前封装的接口都是快速响应的同步操作，预计执行时间不超过 5 秒，无需实现 LRO 模式。

## 使用示例

### 查询存储总空间

```bash
ohos-storage-manager get-total-size
```

输出：
```json
{"success":true,"data":{"total_bytes":128000000000,"total_readable":"119.21GB"}}
```

### 查询剩余空间

```bash
ohos-storage-manager get-free-size
```

输出：
```json
{"success":true,"data":{"free_bytes":50000000000,"free_readable":"46.57GB"}}
```

### 查询系统分区大小

```bash
ohos-storage-manager get-system-size
```

输出：
```json
{"success":true,"data":{"system_bytes":8000000000,"system_readable":"7.45GB"}}
```

### 查询用户存储统计

```bash
ohos-storage-manager get-user-storage-stats 100
```

输出：
```json
{"success":true,"data":{"total":30000000000,"audio":5000000000,"video":10000000000,"image":8000000000,"file":2000000000,"app":5000000000}}
```

### 查询指定应用存储统计

```bash
ohos-storage-manager get-bundle-stats com.example.myapp 0
```

输出：
```json
{"success":true,"data":{"appSize":50000000,"cacheSize":10000000,"dataSize":30000000}}
```

### 查询当前应用存储统计

```bash
ohos-storage-manager get-current-bundle-stats
```

输出：
```json
{"success":true,"data":{"appSize":50000000,"cacheSize":10000000,"dataSize":30000000}}
```

### 错误响应示例

```bash
ohos-storage-manager unknown-cmd
```

输出：
```json
{"success":false,"error":{"code":12900010,"message":"unknown: unknown-cmd"}}
```

## 文件清单

```
tools/
  ohos-storage-manager/
    BUILD.gn                    # 构建配置文件
    config.json                 # 工具描述文件（含权限配置）
    src/
      └── main.cpp             # CLI 主程序源码
    docs/
      ├── README.md            # 工具说明
      └── USAGE.md             # 使用指南
    tests/
      └── test_main.cpp        # 单元测试
    SKILL.md                    # OH-CLI Creator 技能文档
    ohos-storage-manager_report.md  # 本报告文件
```

## 修改清单

| 文件 | 修改内容 |
|------|---------|
| `bundle.json` | 在 `fwk_group` 中添加 CLI 目标 |
| `tools/ohos-storage-manager/BUILD.gn` | 新建构建配置 |
| `tools/ohos-storage-manager/src/main.cpp` | 新建 CLI 主程序 |
| `tools/ohos-storage-manager/config.json` | 新建工具描述文件 |
| `tools/ohos-storage-manager/docs/README.md` | 新建说明文档 |
| `tools/ohos-storage-manager/docs/USAGE.md` | 新建使用指南 |
| `tools/ohos-storage-manager/tests/test_main.cpp` | 新建单元测试 |
| `tools/ohos-storage-manager/SKILL.md` | 新建技能文档 |
| `tools/ohos-storage-manager/ohos-storage-manager_report.md` | 新建本报告 |

## 技术特点

1. **静态命令表**: 使用 std::unordered_map 实现高效的命令分发
2. **Proxy 模式**: 通过 IStorageManager Proxy 访问系统服务
3. **无异常设计**: 不使用 try-catch，符合编译环境限制
4. **模块化设计**: 命令实现、输出格式化、主入口分离
5. **完整文档**: 包含 README、USAGE、SKILL、测试和报告

## 结论

本 CLI 工具已实现 6 个存储管理相关命令，覆盖了日常运维中最常用的存储查询功能。代码结构清晰，文档完整，符合 OpenHarmony CLI 规范。
