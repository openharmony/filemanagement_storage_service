# 存储空间管理服务

## 存储空间管理概述

存储空间管理服务（SA ID: 8650）是 OpenHarmony 系统的系统服务，提供全面的存储空间管理能力。该服务监控设备存储使用情况，基于智能排名管理应用缓存清理，并为不同存储档位提供灵活的存储空间配额计算。

## 系统架构

### 架构说明

```
┌─────────────────────────────────────────────────────────────┐
│                        应用层                                │
│              （设置、文件管理器、系统应用）                  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       接口层                                  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │   IDL    │  │Innerkits │  │   NAPI   │  │  C++ API │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                  存储空间管理 SA                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │             IPC Provider（SA 接口）                   │    │
│  │  - 服务生命周期管理                                  │    │
│  │  - RPC 调用处理                                      │    │
│  │  - 权限检查                                          │    │
│  └─────────────────────────────────────────────────────┘    │
│                              ↓                                │
│  ┌─────────────────────────────────────────────────────┐    │
│  │            缓存清理控制器                              │    │
│  │  - 基于活跃度的应用排名                               │    │
│  │  - 智能缓存清理                                      │    │
│  │  - 时间戳管理                                        │    │
│  └─────────────────────────────────────────────────────┘    │
│                              ↓                                │
│  ┌──────────┐  ┌──────────────────┐  ┌──────────────┐    │
│  │配额      │  │   存储状态        │  │   Bundle     │    │
│  │计算器    │  │    服务           │  │   管理器     │    │
│  │          │  │                  │  │   适配器     │    │
│  │-存储    │  │-总/空闲大小       │  │-代理到 BMS  │    │
│  │ 档位管理 │  │-Inode 查询        │  │-应用信息获取 │    │
│  │-配额    │  │-大小舍入计算       │  │-类型过滤     │    │
│  │-排名    │  └──────────────────┘  └──────────────┘    │
│  │基于配置 │                                              │
│  └──────────┘                                              │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                     依赖与适配器                               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │Bundle Mgr│  │UsageStats│  │Storage   │  │Filesystem│    │
│  │ 服务     │  │ 服务     │  │ 守护进程 │  │   API    │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## 目录结构

```text
services/storage_space_manager/
├── .clang-format                       # 代码风格配置
├── BUILD.gn                            # 构建配置
├── LICENSE                             # 许可证文件
├── OAT.xml                             # 开源合规文件
├── QUOTA_CALCULATOR_README.md          # 配额计算器文档
├── storage_space_manager.gni           # GN 导入文件
├── common/                             # 公共定义
│   ├── BUILD.gn
│   └── include/
│       └── storage_space_manager_errno.h         # 错误码定义
├── etc/                                # 配置文件
│   ├── BUILD.gn
│   └── storage_space_manager.cfg                 # 服务配置文件
├── interfaces/                         # 对外接口层
│   └── innerkits/                      # 原生 C++ 客户端 API
│       ├── BUILD.gn
│       ├── include/
│       │   ├── callback/
│       │   │   └── storage_space_manager_load_callback.h  # SA 加载回调
│       │   ├── i_quota_calculator.h              # 配额计算器接口
│       │   └── storage_space_manager_client.h    # 客户端接口
│       ├── src/
│       │   ├── callback/
│       │   │   └── storage_space_manager_load_callback.cpp
│       │   └── storage_space_manager_client.cpp
│       └── IStorageSpaceManager.idl              # IDL 接口定义
├── sa_profile/                         # SystemAbility 配置
│   ├── BUILD.gn
│   └── 8650.json                                   # SA 配置
├── services/storage_space_manager/     # SystemAbility 实现
│   ├── BUILD.gn
│   ├── include/                        # 公共头文件
│   │   ├── adapter/
│   │   │   ├── bundle_manager_adapter_proxy.h   # Bundle 管理器代理适配器
│   │   │   └── bundle_manager_connector.h       # Bundle 管理器连接器
│   │   ├── cache_clean_controller/
│   │   │   └── cache_clean_controller.h         # 缓存清理控制器
│   │   ├── common_event/
│   │   │   └── storage_common_event_subscriber.h # 事件订阅器
│   │   ├── ipc/
│   │   │   └── storage_space_manager_provider.h  # SA Provider 接口
│   │   └── storage/
│   │       └── storage_total_status_service.h    # 存储状态服务
│   └── src/                            # 实现文件
│       ├── adapter/
│       │   ├── bundle_manager_adapter_proxy.cpp
│       │   └── bundle_manager_connector.cpp
│       ├── cache_clean_controller/
│       │   └── cache_clean_controller.cpp
│       ├── common_event/
│       │   └── storage_common_event_subscriber.cpp
│       ├── ipc/
│       │   └── storage_space_manager_provider.cpp
│       └── storage/
│           └── storage_total_status_service.cpp
├── test/                               # 测试
│   ├── fuzztest/
│   │   └── storage_space_manager_fuzzer/
│   │       ├── storage_space_manager_fuzzer.cpp
│   │       ├── storage_space_manager_fuzzer.h
│   │       ├── project.xml
│   │       └── corpus/
│   ├── mock/
│   └── unittest/
│       ├── quota_calculator_test.cpp
│       ├── cache_clean_controller_test.cpp
│       ├── storage_space_manager_provider_test.cpp
│       ├── storage_total_status_service_test.cpp
│       └── BUILD.gn
└── utils/                              # 工具类
    ├── BUILD.gn
    ├── include/
    │   ├── ipc_caller_auth.h                   # IPC 调用者认证
    │   └── storage_space_manager_hilog.h        # 日志工具
    └── src/
        └── ipc_caller_auth.cpp

## 存储空间管理能力

### 1. 存储空间查询 API

服务提供全面的存储空间信息：

- **GetTotalSize**：获取设备存储总容量（已舍入的值）
- **GetSystemSize**：计算系统分区大小
- **GetFreeSize**：查询可用空闲存储空间
- **GetTotalInodes**：获取数据分区的总 inode 数量
- **GetFreeInodes**：查询可用空闲 inode 数量

**功能特性：**
- 自动将大小舍入到合适的单位（KB/MB/GB/TB）
- 合并数据和根分区大小计算总容量
- 实时存储状态监控

### 2. Bundle 缓存管理

多维度分析的智能应用缓存清理：

**缓存清理流程：**
```
1. 获取所有应用
   ↓
2. 按应用类型过滤（排除系统应用、无 Bundle 数据的应用）
   ↓
3. 按活跃度排名（DeviceUsageStats 集成）
   ↓
4. 计算存储配额（基于总存储空间和应用排名）
   ↓
5. 执行缓存清理（尊重应用特定阈值）
   ↓
6. 保存清理时间戳
```

**支持场景：**
- Top 排名应用缓存清理（可配置的排名档位）
- 未使用应用的全部缓存清理
- 系统应用白名单及自定义缓存限制
- 基于时间的清理触发

### 3. 存储配额计算器

基于设备存储容量的灵活配额计算：

**存储档位配置：**
- 多个存储范围及不同的配额策略
- 配置示例：
  - 0-256GB：top1-3: 1500MB，top4-10: 600MB，top11-20: 300MB
  - 257GB-最大：top1-5: 3000MB，top6-15: 1000MB，top16-30: 500MB

**功能特性：**
- 支持 JSON 格式的配置文件
- 动态排名范围解析（如 "top1-3"、"top4-10"）
- 未指定排名的默认配额回退
- 批量查询多个应用的配额

**配置文件格式：**
```json
{
  "cache_auto_clean_switch": 1,
  "auto_cache_clean_span": 168,
  "system_app_cache_config": {
    "com.example.system": 100
  },
  "top_app_cache_config": {
    "top_ranking_hours_span": 336,
    "no_use_hours_for_clean_all": 2160,
    "size_lowerlimit_0_upperlimit_256": {
      "top1-3": 1500,
      "top4-10": 600,
      "default": 100
    }
  }
}
```

### 4. 缓存清理时间戳管理

- 缓存清理成功后自动保存时间戳
- JSON 格式存储在 `/data/service/el1/public/storage_space_manager/cache_clean_config.json`
- 跟踪上次清理时间用于调度目的
- 不使用 try-catch（符合编码标准）

## 构建与集成

### 构建前提条件

- OpenHarmony 编译环境
- 依赖项：
  - `bundle_manager`（Bundle 管理服务）
  - `device_usage_statistics`（设备使用统计服务，可选）
  - `hilog`（日志框架）
  - `json`（nlohmann/json 库）
  - `safwk`（SystemAbility 框架）

### 构建命令

```bash
# 构建存储空间管理服务
./build.sh --product-name=<产品名> --build-target storage_space_manager --ccache

# 构建单元测试
./build.sh --product-name=<产品名> --build-target storage_space_manager_ut --ccache

# 构建模糊测试
./build.sh --product-name=<产品名> --build-target storage_space_manager_fuzztest --ccache
```

### 集成点

**System Ability 注册：**
- SA ID：8650
- 进程：`storage_space_manager`
- 库：`libstorage_space_manager.z.so`
- 配置：`sa_profile/8650.json`

**依赖服务：**
- Storage Daemon（用于存储状态查询）
- Bundle Manager（用于应用信息）
- Device Usage Statistics（用于应用活跃度排名）

## 存储空间管理开发指南

### 使用 C++ 客户端 API

```cpp
#include "storage_space_manager_client.h"

using namespace OHOS::StorageSpaceManager;

// 获取客户端实例
auto& client = StorageSpaceManagerClient::GetInstance();

// 查询存储信息
int64_t totalSize = 0;
int32_t ret = client.GetTotalSize(totalSize);
if (ret == E_OK) {
    // 处理 totalSize
}

// 清理特定用户的 Bundle 缓存
int32_t userId = 100;
ret = client.CleanBundleCache(userId);
if (ret == E_OK) {
    // 缓存清理成功
}
```

### 使用 N-API（JavaScript）

```javascript
import storageSpaceManager from '@ohos.filemanagement.storageSpaceManager';

// 查询总存储大小
let totalSize = storageSpaceManager.getTotalSize();

// 清理 Bundle 缓存
storageSpaceManager.cleanBundleCache(userId);
```

### 应用侧开发流程

**存储查询：**
1. 初始化 StorageSpaceManagerClient
2. 调用查询 API（GetTotalSize、GetFreeSize 等）
3. 处理返回码并处理结果

**缓存清理：**
1. 确保拥有正确权限（系统应用或使用正确签名）
2. 使用目标用户 ID 调用 CleanBundleCache
3. 如需要，通过回调监控清理结果

### 配置自定义

**部署自定义配额配置：**
```bash
# 将配置文件放置到系统目录
cp quota_config.json /system/etc/storage_space_manager/

# 或使用数据目录
cp quota_config.json /data/service/el1/public/storage_space_manager/

# 重启服务
kill -9 $(pidof storage_space_manager)
```

### 错误处理

常用错误码：
- `E_OK`：成功
- `E_SERVICE_NOT_READY`：服务未初始化
- `E_INVALID_ARGUMENT`：无效的输入参数
- `E_STATVFS_FAILED`：文件系统查询失败
- `E_IO_ERROR`：I/O 操作失败

## API 参考

### IPC 接口（IDL）

| IPC Code | API | 参数 | 返回值 | 说明 |
|----------|-----|------|--------|------|
| 1 | GetTotalSize | 输出：long totalSize | void | 获取设备总存储 |
| 2 | GetSystemSize | 输出：long systemSize | void | 获取系统分区大小 |
| 3 | GetFreeSize | 输出：long freeSize | void | 获取可用空闲空间 |
| 4 | GetTotalInodes | 输出：long totalInodes | void | 获取总 inode 数 |
| 5 | GetFreeInodes | 输出：long freeInodes | void | 获取空闲 inode 数 |
| 6 | CleanBundleCache | 输入：int userId | void | 清理 Bundle 缓存 |

### C++ 客户端 API

完整的 API 定义参见 `interfaces/innerkits/include/storage_space_manager_client.h`

## 测试

### 单元测试

位于 `test/unittest/`：
- `quota_calculator_test.cpp` - 30+ 个配额计算测试用例
- `cache_clean_controller_test.cpp` - 20+ 个缓存清理测试用例
- `storage_space_manager_provider_test.cpp` - 25+ 个 Provider 测试用例
- `storage_total_status_service_test.cpp` - 25+ 个存储状态测试用例

**运行单元测试：**
```bash
# 构建并运行
./build.sh --product-name=<产品名> --build-target storage_space_manager_ut --ccache

# 或使用已安装的测试二进制文件
./StorageSpaceManagerUt -f
```

### 模糊测试

位于 `test/fuzztest/`：
- 覆盖全部 6 个公共 API 的综合模糊测试
- 12 种不同的模糊测试场景
- 边界值测试
- 并发访问模拟

**运行模糊测试：**
```bash
# 构建模糊测试
./build.sh --product-name=<产品名> --build-target storage_space_manager_fuzztest --ccache

# 运行模糊测试
./StorageSpaceManagerFuzzTest corpus/
```

### 代码覆盖率

目标分支覆盖率：80%+

## 相关仓库

- [filemanagement_storage_service](https://gitcode.com/openharmony/filemanagement_storage_service) - 存储服务框架
- [applications_settings_data](https://gitcode.com/openharmony/applications_settings_data) - 设置应用
- [third_party_gptfdisk](https://gitcode.com/openharmony/third_party_gptfdisk) - GPT 分区表支持
- [third_party_e2fsprogs](https://gitcode.com/openharmony/third_party_e2fsprogs) - Ext2/3/4 文件系统工具
- [third_party_f2fs-tools](https://gitcode.com/openharmony/third_party_f2fs-tools) - F2FS 文件系统工具
- [third_party_ntfs-3g](https://gitcode.com/openharmony/third_party_ntfs-3g) - NTFS 文件系统支持
- [third_party_exfatprogs](https://gitcode.com/openharmony/third_party_exfatprogs) - exFAT 文件系统工具

## 更新日志

### 版本更新

**v1.0 (2026-06-18)**
- 初始版本，包含核心存储空间管理功能
- 存储查询 API（总/空闲/系统大小，inode）
- 基于活跃度排名的 Bundle 缓存清理
- 可配置的存储配额计算器
- 清理操作的时间戳管理

**v1.1 (当前版本)**
- 添加全面的单元测试（100+ 测试用例）
- 为所有公共 API 添加模糊测试
- 增强错误处理，不使用 try-catch
- 改进基于档位配置的配额计算
- 添加存储舍入算法

## 许可证

版权所有 (c) 2026 华为技术有限公司
根据 Apache License 2.0 许可
