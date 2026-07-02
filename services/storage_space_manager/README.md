# Storage Space Manager Service

## Storage Space Manager Overview

Storage Space Manager Service (SA ID: 8650) is a system service that provides comprehensive storage space management capabilities for OpenHarmony system. It monitors device storage usage, manages application cache cleaning based on intelligent ranking, and provides flexible storage quota calculation for different storage tiers.

## System Architecture

### Architecture Description

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                         │
│  (Settings, File Manager, System Applications)                │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      Interface Layer                           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │   IDL    │  │Innerkits │  │   NAPI   │  │  C++ API │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                  Storage Space Manager SA                    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              IPC Provider (SA Interface)              │    │
│  │  - Service Lifecycle Management                       │    │
│  │  - RPC Call Handling                                  │    │
│  │  - Permission Checking                                 │    │
│  └─────────────────────────────────────────────────────┘    │
│                              ↓                                │
│  ┌─────────────────────────────────────────────────────┐    │
│  │          Cache Clean Controller                        │    │
│  │  - Bundle Ranking by Activity Level                   │    │
│  │  - Intelligent Cache Cleaning                         │    │
│  │  - Timestamp Management                               │    │
│  └─────────────────────────────────────────────────────┘    │
│                              ↓                                │
│  ┌──────────┐  ┌──────────────────┐  ┌──────────────┐    │
│  │Quota     │  │  Storage Status  │  │   Bundle     │    │
│  │Calculator│  │    Service       │  │   Manager    │    │
│  │          │  │                  │  │   Adapter    │    │
│  │-Storage │  │-Total/Free Size  │  │-Proxy to BMS │    │
│  │ Tier Mgmt│  │-Inode Queries   │  │-App Info Get │    │
│  │-Quota    │  │-Rounded Size Calc│  │-Type Filter │    │
│  │-Rank     │  └──────────────────┘  └──────────────┘    │
│  │Based Config│                                              │
│  └──────────┘                                              │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    Dependencies & Adapters                   │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │Bundle Mgr│  │UsageStats│  │Storage   │  │Filesystem│    │
│  │ Service  │  │ Service  │  │ Daemon   │  │   APIs   │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Directory Structure

```text
services/storage_space_manager/
├── .clang-format                       # Code style configuration
├── BUILD.gn                            # Build configuration
├── LICENSE                             # License file
├── OAT.xml                             # Open-source compliance
├── QUOTA_CALCULATOR_README.md          # Quota calculator docs
├── storage_space_manager.gni           # GN import file
├── common/                             # Shared definitions
│   ├── BUILD.gn
│   └── include/
│       └── storage_space_manager_errno.h       # Error code definitions
├── etc/                                # Configuration files
│   ├── BUILD.gn
│   └── storage_space_manager.cfg               # Service configuration
├── interfaces/                         # Public API layer
│   └── innerkits/                      # Native C++ client API
│       ├── BUILD.gn
│       ├── include/
│       │   ├── callback/
│       │   │   └── storage_space_manager_load_callback.h  # SA load callback
│       │   ├── i_quota_calculator.h              # Quota calculator interface
│       │   └── storage_space_manager_client.h   # Client interface
│       ├── src/
│       │   ├── callback/
│       │   │   └── storage_space_manager_load_callback.cpp
│       │   └── storage_space_manager_client.cpp
│       └── IStorageSpaceManager.idl             # IDL interface definition
├── sa_profile/                         # SystemAbility profile
│   ├── BUILD.gn
│   └── 8650.json                                 # SA configuration
├── services/storage_space_manager/     # SystemAbility implementation
│   ├── BUILD.gn
│   ├── include/                        # Public headers
│   │   ├── adapter/
│   │   │   ├── bundle_manager_adapter_proxy.h   # Bundle manager proxy adapter
│   │   │   └── bundle_manager_connector.h       # Bundle manager connector
│   │   ├── cache_clean_controller/
│   │   │   └── cache_clean_controller.h         # Cache cleaning controller
│   │   ├── common_event/
│   │   │   └── storage_common_event_subscriber.h # Event subscriber
│   │   ├── ipc/
│   │   │   └── storage_space_manager_provider.h  # SA provider interface
│   │   └── storage/
│   │       └── storage_total_status_service.h    # Storage status service
│   └── src/                            # Implementation files
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
├── test/                               # Tests
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
└── utils/                              # Utilities
    ├── BUILD.gn
    ├── include/
    │   ├── ipc_caller_auth.h                   # IPC caller authentication
    │   └── storage_space_manager_hilog.h        # Logging utilities
    └── src/
        └── ipc_caller_auth.cpp

## Storage Space Manager Capabilities

### 1. Storage Space Query APIs

The service provides comprehensive storage space information:

- **GetTotalSize**: Retrieves total device storage capacity (rounded value)
- **GetSystemSize**: Calculates system partition size
- **GetFreeSize**: Queries available free storage space
- **GetTotalInodes**: Gets total inode count for the data partition
- **GetFreeInodes**: Queries available free inode count

**Features:**
- Automatic size rounding to appropriate units (KB/MB/GB/TB)
- Combines data and root partition sizes for total capacity
- Real-time storage status monitoring

### 2. Bundle Cache Management

Intelligent application cache cleaning with multi-dimensional analysis:

**Cache Cleaning Process:**
```
1. Get All Applications
   ↓
2. Filter by Bundle Type (exclude system apps, apps without bundle data)
   ↓
3. Rank by Activity Level (DeviceUsageStats integration)
   ↓
4. Calculate Storage Quota (based on total storage & app rank)
   ↓
5. Execute Cache Cleaning (respecting app-specific thresholds)
   ↓
6. Save Cleaning Timestamp
```

**Supported Scenarios:**
- Top-ranked apps cache cleaning (configurable ranking tiers)
- Full cache cleaning for unused apps
- System app whitelist with custom cache limits
- Time-based cleaning triggers

### 3. Storage Quota Calculator

Flexible quota calculation based on device storage capacity:

**Storage Tier Configuration:**
- Multiple storage ranges with different quota policies
- Example configurations:
  - 0-256GB: top1-3: 1500MB, top4-10: 600MB, top11-20: 300MB
  - 257GB-max: top1-5: 3000MB, top6-15: 1000MB, top16-30: 500MB

**Features:**
- JSON-based configuration file support
- Dynamic rank range parsing (e.g., "top1-3", "top4-10")
- Default quota fallback for unspecified ranks
- Batch quota query for multiple apps

**Configuration File Format:**
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

### 4. Cache Cleaning Timestamp Management

- Automatic timestamp saving after successful cache cleaning
- JSON format storage in `/data/service/el1/public/storage_space_manager/cache_clean_config.json`
- Tracks last cleaning time for scheduling purposes
- No try-catch usage (compliant with coding standards)

## Build and Integration

### Build Prerequisites

- OpenHarmony compilation environment
- Dependencies:
  - `bundle_manager` (Bundle Management Service)
  - `device_usage_statistics` (Device Usage Statistics Service, optional)
  - `hilog` (Logging framework)
  - `json` (nlohmann/json library)
  - `safwk` (SystemAbility framework)

### Build Commands

```bash
# Build Storage Space Manager service
./build.sh --product-name=<product> --build-target storage_space_manager --ccache

# Build unit tests
./build.sh --product-name=<product> --build-target storage_space_manager_ut --ccache

# Build fuzz tests
./build.sh --product-name=<product> --build-target storage_space_manager_fuzztest --ccache
```

### Integration Points

**System Ability Registration:**
- SA ID: 8650
- Process: `storage_space_manager`
- Library: `libstorage_space_manager.z.so`
- Configuration: `sa_profile/8650.json`

**Dependencies:**
- Storage Daemon (for storage status queries)
- Bundle Manager (for application information)
- Device Usage Statistics (for app activity ranking)

## Storage Space Manager Developer Guide

### Using the C++ Client API

```cpp
#include "storage_space_manager_client.h"

using namespace OHOS::StorageSpaceManager;

// Get client instance
auto& client = StorageSpaceManagerClient::GetInstance();

// Query storage information
int64_t totalSize = 0;
int32_t ret = client.GetTotalSize(totalSize);
if (ret == E_OK) {
    // Process totalSize
}

// Clean bundle cache for specific user
int32_t userId = 100;
ret = client.CleanBundleCache(userId);
if (ret == E_OK) {
    // Cache cleaning succeeded
}
```

### Using the N-API (JavaScript)

```javascript
import storageSpaceManager from '@ohos.filemanagement.storageSpaceManager';

// Query total storage size
let totalSize = storageSpaceManager.getTotalSize();

// Clean bundle cache
storageSpaceManager.cleanBundleCache(userId);
```

### Application-Side Development Flow

**For Storage Query:**
1. Initialize StorageSpaceManagerClient
2. Call query APIs (GetTotalSize, GetFreeSize, etc.)
3. Handle return codes and process results

**For Cache Cleaning:**
1. Ensure proper permissions (system app or signed with proper signature)
2. Call CleanBundleCache with target user ID
3. Monitor cleaning results through callbacks if needed

### Configuration Customization

**Deploy Custom Quota Configuration:**
```bash
# Place configuration file to system directory
cp quota_config.json /system/etc/storage_space_manager/

# Or use data directory
cp quota_config.json /data/service/el1/public/storage_space_manager/

# Restart service
kill -9 $(pidof storage_space_manager)
```

### Error Handling

Common error codes:
- `E_OK`: Success
- `E_SERVICE_NOT_READY`: Service not initialized
- `E_INVALID_ARGUMENT`: Invalid input parameters
- `E_STATVFS_FAILED`: Filesystem query failed
- `E_IO_ERROR`: I/O operation failed

## API Reference

### IPC Interface (IDL)

| IPC Code | API | Parameters | Return | Description |
|----------|-----|------------|--------|-------------|
| 1 | GetTotalSize | output: long totalSize | void | Get total device storage |
| 2 | GetSystemSize | output: long systemSize | void | Get system partition size |
| 3 | GetFreeSize | output: long freeSize | void | Get available free space |
| 4 | GetTotalInodes | output: long totalInodes | void | Get total inode count |
| 5 | GetFreeInodes | output: long freeInodes | void | Get free inode count |
| 6 | CleanBundleCache | input: int userId | void | Clean bundle cache |

### C++ Client APIs

See `interfaces/innerkits/include/storage_space_manager_client.h` for complete API definitions.

## Testing

### Unit Tests

Located in `test/unittest/`:
- `quota_calculator_test.cpp` - 30+ test cases covering quota calculation
- `cache_clean_controller_test.cpp` - 20+ test cases for cache cleaning
- `storage_space_manager_provider_test.cpp` - 25+ test cases for provider
- `storage_total_status_service_test.cpp` - 25+ test cases for storage status

**Run Unit Tests:**
```bash
# Build and run
./build.sh --product-name=<product> --build-target storage_space_manager_ut --ccache

# Or use installed test binary
./StorageSpaceManagerUt -f
```

### Fuzz Tests

Located in `test/fuzztest/`:
- Comprehensive fuzzing of all 6 public APIs
- 12 different fuzzing scenarios
- Boundary value testing
- Concurrent access simulation

**Run Fuzz Tests:**
```bash
# Build fuzz tests
./build.sh --product-name=<product> --build-target storage_space_manager_fuzztest --ccache

# Run fuzz tests
./StorageSpaceManagerFuzzTest corpus/
```

### Code Coverage

Target branch coverage: 80%+

## Related Repositories

- [filemanagement_storage_service](https://gitcode.com/openharmony/filemanagement_storage_service) - Storage service framework
- [applications_settings_data](https://gitcode.com/openharmony/applications_settings_data) - Settings application
- [third_party_gptfdisk](https://gitcode.com/openharmony/third_party_gptfdisk) - GPT partition table support
- [third_party_e2fsprogs](https://gitcode.com/openharmony/third_party_e2fsprogs) - Ext2/3/4 filesystem utilities
- [third_party_f2fs-tools](https://gitcode.com/openharmony/third_party_f2fs-tools) - F2FS filesystem tools
- [third_party_ntfs-3g](https://gitcode.com/openharmony/third_party_ntfs-3g) - NTFS filesystem support
- [third_party_exfatprogs](https://gitcode.com/openharmony/third_party_exfatprogs) - exFAT filesystem utilities

## Changelog

### Version Updates

**v1.0 (2026-06-18)**
- Initial release with core storage space management features
- Storage query APIs (total/free/system size, inodes)
- Bundle cache cleaning with activity-based ranking
- Configurable storage quota calculator
- Timestamp management for cleaning operations

**v1.1 (Current)**
- Added comprehensive unit tests (100+ test cases)
- Added fuzz tests for all public APIs
- Enhanced error handling without try-catch
- Improved quota calculation with tier-based configuration
- Added storage rounding algorithm

## License

Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0
