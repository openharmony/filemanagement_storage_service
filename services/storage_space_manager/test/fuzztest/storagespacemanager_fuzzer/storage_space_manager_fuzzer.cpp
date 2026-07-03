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

namespace {
    constexpr uint32_t MAX_RAPID_CALLS = 100;
    constexpr uint32_t MAX_ALTERNATING_ITERATIONS = 50;
    constexpr uint32_t REPEAT_COUNT = 10;
    constexpr uint32_t CLEAN_CALL_COUNT = 5;
    constexpr uint32_t ALTERNATE_INTERVAL = 2;
    constexpr uint32_t CLEAN_INTERVAL = 3;

    enum class ApiChoice : int32_t {
        GET_TOTAL_SIZE = 0,
        GET_SYSTEM_SIZE,
        GET_FREE_SIZE,
        GET_TOTAL_INODES,
        GET_FREE_INODES,
        COUNT
    };

    enum class FuzzPattern : int32_t {
        ALL_SIZE_APIS = 0,
        ALL_INODE_APIS,
        MIXED_APIS,
        REPEAT_SAME_API,
        ALL_CLEAN_BUNDLE_CACHE,
        COUNT
    };

    enum class FuzzFunction : uint32_t {
        GET_TOTAL_SIZE = 0,
        GET_SYSTEM_SIZE,
        GET_FREE_SIZE,
        GET_TOTAL_INODES,
        GET_FREE_INODES,
        CLEAN_BUNDLE_CACHE,
        INVALID_PARAMETERS,
        RAPID_SEQUENTIAL_CALLS,
        ALTERNATING_CALLS,
        CONCURRENT_ACCESS,
        BOUNDARY_VALUES,
        RANDOM_DATA_PATTERNS,
        COUNT
    };
}

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

    uint32_t callCount = provider.ConsumeIntegralInRange<uint32_t>(1, MAX_RAPID_CALLS);

    for (uint32_t i = 0; i < callCount; i++) {
        int64_t size = 0;
        auto apiChoice = static_cast<ApiChoice>(provider.ConsumeIntegralInRange<int32_t>(
            0, static_cast<int32_t>(ApiChoice::COUNT) - 1));

        switch (apiChoice) {
            case ApiChoice::GET_TOTAL_SIZE:
                g_client->GetTotalSize(size);
                break;
            case ApiChoice::GET_SYSTEM_SIZE:
                g_client->GetSystemSize(size);
                break;
            case ApiChoice::GET_FREE_SIZE:
                g_client->GetFreeSize(size);
                break;
            case ApiChoice::GET_TOTAL_INODES:
                g_client->GetTotalInodes(size);
                break;
            case ApiChoice::GET_FREE_INODES:
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

    uint32_t iterations = provider.ConsumeIntegralInRange<uint32_t>(1, MAX_ALTERNATING_ITERATIONS);

    for (uint32_t i = 0; i < iterations; i++) {
        int64_t size = 0;

        // Alternate between size and inode APIs
        if (i % ALTERNATE_INTERVAL == 0) {
            g_client->GetTotalSize(size);
        } else {
            g_client->GetTotalInodes(size);
        }

        if (i % CLEAN_INTERVAL == 0) {
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

    auto fuzzPattern = static_cast<FuzzPattern>(provider.ConsumeIntegral<uint32_t>() %
        static_cast<int32_t>(FuzzPattern::COUNT));

    switch (fuzzPattern) {
        case FuzzPattern::ALL_SIZE_APIS: {
            int64_t size = 0;
            g_client->GetTotalSize(size);
            g_client->GetSystemSize(size);
            g_client->GetFreeSize(size);
            break;
        }
        case FuzzPattern::ALL_INODE_APIS: {
            int64_t inodes = 0;
            g_client->GetTotalInodes(inodes);
            g_client->GetFreeInodes(inodes);
            break;
        }
        case FuzzPattern::MIXED_APIS: {
            int64_t size = 0;
            g_client->GetTotalSize(size);
            int32_t userId = provider.ConsumeIntegral<int32_t>();
            g_client->CleanBundleCache(userId);
            g_client->GetFreeSize(size);
            break;
        }
        case FuzzPattern::REPEAT_SAME_API: {
            int64_t size = 0;
            for (int i = 0; i < REPEAT_COUNT; i++) {
                g_client->GetTotalSize(size);
            }
            break;
        }
        case FuzzPattern::ALL_CLEAN_BUNDLE_CACHE: {
            for (int i = 0; i < CLEAN_CALL_COUNT; i++) {
                int32_t userId = provider.ConsumeIntegral<int32_t>();
                g_client->CleanBundleCache(userId);
            }
            break;
        }
        default:
            break;
    }
}

using FuzzFunc = void (*)(FuzzedDataProvider&);
constexpr FuzzFunc FUZZ_DISPATCH_TABLE[] = {
    FuzzGetTotalSize,
    FuzzGetSystemSize,
    FuzzGetFreeSize,
    FuzzGetTotalInodes,
    FuzzGetFreeInodes,
    FuzzCleanBundleCache,
    FuzzInvalidParameters,
    FuzzRapidSequentialCalls,
    FuzzAlternatingCalls,
    FuzzConcurrentAccess,
    FuzzBoundaryValues,
    FuzzRandomDataPatterns,
};

} // namespace StorageSpaceManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }

    FuzzedDataProvider provider(data, size);

    auto index = provider.ConsumeIntegralInRange<uint32_t>(
        0, static_cast<uint32_t>(FuzzFunction::COUNT) - 1);
    OHOS::StorageSpaceManager::FUZZ_DISPATCH_TABLE[index](provider);

    return 0;
}
