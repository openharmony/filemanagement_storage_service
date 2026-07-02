/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "storage_space_manager_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <securec.h>
#include <string>
#include <algorithm>
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>
#include "storage_space_manager_client.h"
#include "storage_space_manager_errno.h"

using namespace OHOS;
using namespace OHOS::StorageSpaceManager;

namespace OHOS {
namespace StorageSpaceManager {

static StorageSpaceManagerClient* g_client = nullptr;

// Initialize StorageSpaceManager client
bool InitializeClient()
{
    if (g_client == nullptr) {
        g_client = DelayedSingleton<StorageSpaceManagerClient>::GetInstance().get();
    }
    return g_client != nullptr;
}

// Fuzz GetTotalSize API
void FuzzGetTotalSize(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    int64_t totalSize = 0;
    g_client->GetTotalSize(totalSize);
}

// Fuzz GetSystemSize API
void FuzzGetSystemSize(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    int64_t systemSize = 0;
    g_client->GetSystemSize(systemSize);
}

// Fuzz GetFreeSize API
void FuzzGetFreeSize(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    int64_t freeSize = 0;
    g_client->GetFreeSize(freeSize);
}

// Fuzz GetTotalInodes API
void FuzzGetTotalInodes(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    int64_t totalInodes = 0;
    g_client->GetTotalInodes(totalInodes);
}

// Fuzz GetFreeInodes API
void FuzzGetFreeInodes(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    int64_t freeInodes = 0;
    g_client->GetFreeInodes(freeInodes);
}

// Fuzz CleanBundleCache API with various userId values
void FuzzCleanBundleCache(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    // Generate userId with various edge cases
    int32_t userId = provider.ConsumeIntegral<int32_t>();
    g_client->CleanBundleCache(userId);
}

// Fuzz with invalid/null parameters
void FuzzInvalidParameters(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    // Test with extreme values
    int64_t size = 0;

    // Test with int64 boundaries
    g_client->GetTotalSize(size);

    // Test negative userId
    int32_t negativeUserId = provider.ConsumeIntegral<int32_t>();
    if (negativeUserId > 0) {
        negativeUserId = -negativeUserId;
    }
    g_client->CleanBundleCache(negativeUserId);

    // Test large userId
    int32_t largeUserId = provider.ConsumeIntegral<int32_t>();
    g_client->CleanBundleCache(largeUserId);
}

// Fuzz with rapid sequential calls
void FuzzRapidSequentialCalls(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    uint32_t callCount = provider.ConsumeIntegralInRange<uint32_t>(1, 100);

    for (uint32_t i = 0; i < callCount; i++) {
        int64_t size = 0;
        int32_t apiChoice = provider.ConsumeIntegralInRange<int32_t>(0, 4);

        switch (apiChoice) {
            case 0:
                g_client->GetTotalSize(size);
                break;
            case 1:
                g_client->GetSystemSize(size);
                break;
            case 2:
                g_client->GetFreeSize(size);
                break;
            case 3:
                g_client->GetTotalInodes(size);
                break;
            case 4:
                g_client->GetFreeInodes(size);
                break;
            default:
                break;
        }
    }
}

// Fuzz with alternating API calls
void FuzzAlternatingCalls(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    uint32_t iterations = provider.ConsumeIntegralInRange<uint32_t>(1, 50);

    for (uint32_t i = 0; i < iterations; i++) {
        int64_t size = 0;

        // Alternate between size and inode APIs
        if (i % 2 == 0) {
            g_client->GetTotalSize(size);
        } else {
            g_client->GetTotalInodes(size);
        }

        if (i % 3 == 0) {
            int32_t userId = provider.ConsumeIntegral<int32_t>();
            g_client->CleanBundleCache(userId);
        }
    }
}

// Fuzz concurrent access simulation
void FuzzConcurrentAccess(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    // Simulate concurrent access by calling multiple APIs
    int64_t totalSize = 0;
    int64_t systemSize = 0;
    int64_t freeSize = 0;
    int64_t totalInodes = 0;
    int64_t freeInodes = 0;

    // Call all APIs
    g_client->GetTotalSize(totalSize);
    g_client->GetSystemSize(systemSize);
    g_client->GetFreeSize(freeSize);
    g_client->GetTotalInodes(totalInodes);
    g_client->GetFreeInodes(freeInodes);
}

// Fuzz with boundary values
void FuzzBoundaryValues(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    // Test boundary userId values
    std::vector<int32_t> boundaryUserIds = {
        0, 100, 101, 102, 999, INT32_MAX, INT32_MIN, -1, -100
    };

    for (int32_t testUserId : boundaryUserIds) {
        g_client->CleanBundleCache(testUserId);
    }
}

// Fuzz with random data patterns
void FuzzRandomDataPatterns(FuzzedDataProvider& provider)
{
    if (!InitializeClient()) {
        return;
    }

    uint32_t pattern = provider.ConsumeIntegral<uint32_t>();

    switch (pattern % 5) {
        case 0:
            // Pattern: All size APIs
            {
                int64_t size = 0;
                g_client->GetTotalSize(size);
                g_client->GetSystemSize(size);
                g_client->GetFreeSize(size);
            }
            break;
        case 1:
            // Pattern: All inode APIs
            {
                int64_t inodes = 0;
                g_client->GetTotalInodes(inodes);
                g_client->GetFreeInodes(inodes);
            }
            break;
        case 2:
            // Pattern: Mix of APIs
            {
                int64_t size = 0;
                g_client->GetTotalSize(size);
                int32_t userId = provider.ConsumeIntegral<int32_t>();
                g_client->CleanBundleCache(userId);
                g_client->GetFreeSize(size);
            }
            break;
        case 3:
            // Pattern: Repeat same API
            {
                int64_t size = 0;
                for (int i = 0; i < 10; i++) {
                    g_client->GetTotalSize(size);
                }
            }
            break;
        case 4:
            // Pattern: All CleanBundleCache with different user IDs
            {
                for (int i = 0; i < 5; i++) {
                    int32_t userId = provider.ConsumeIntegral<int32_t>();
                    g_client->CleanBundleCache(userId);
                }
            }
            break;
        default:
            break;
    }
}

} // namespace StorageSpaceManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }

    FuzzedDataProvider provider(data, size);

    // Choose a fuzz function based on the data
    uint32_t fuzzFunction = provider.ConsumeIntegralInRange<uint32_t>(0, 11);

    switch (fuzzFunction) {
        case 0:
            OHOS::StorageSpaceManager::FuzzGetTotalSize(provider);
            break;
        case 1:
            OHOS::StorageSpaceManager::FuzzGetSystemSize(provider);
            break;
        case 2:
            OHOS::StorageSpaceManager::FuzzGetFreeSize(provider);
            break;
        case 3:
            OHOS::StorageSpaceManager::FuzzGetTotalInodes(provider);
            break;
        case 4:
            OHOS::StorageSpaceManager::FuzzGetFreeInodes(provider);
            break;
        case 5:
            OHOS::StorageSpaceManager::FuzzCleanBundleCache(provider);
            break;
        case 6:
            OHOS::StorageSpaceManager::FuzzInvalidParameters(provider);
            break;
        case 7:
            OHOS::StorageSpaceManager::FuzzRapidSequentialCalls(provider);
            break;
        case 8:
            OHOS::StorageSpaceManager::FuzzAlternatingCalls(provider);
            break;
        case 9:
            OHOS::StorageSpaceManager::FuzzConcurrentAccess(provider);
            break;
        case 10:
            OHOS::StorageSpaceManager::FuzzBoundaryValues(provider);
            break;
        case 11:
            OHOS::StorageSpaceManager::FuzzRandomDataPatterns(provider);
            break;
        default:
            break;
    }

    return 0;
}
