/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "storagestatusservice_fuzzer.h"
#include "storage/storage_status_service.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageManager {
bool StorageStatusServiceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(int64_t))) {
        return false;
    }
    std::shared_ptr<StorageStatusService> service = DelayedSingleton<StorageStatusService>::GetInstance();
    if (service == nullptr) {
        return 0;
    }
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    std::string pkgName(reinterpret_cast<const char *>(data), size);
    std::string type(reinterpret_cast<const char *>(data), size);
    BundleStats bundleStats;
    StorageStats storageStats;
    std::vector<std::string> bundleName;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    std::string metaData(reinterpret_cast<const char *>(data), size);
    int64_t metaData2 = *(reinterpret_cast<const int64_t *>(data));
    bundleName.push_back(metaData);
    incrementalBackTimes.push_back(metaData2);
    pkgFileSizes.push_back(metaData2);
    service->GetBundleStats(pkgName, bundleStats);
    service->GetUserStorageStats(storageStats);
    service->GetUserStorageStats(userId, storageStats);
    service->GetUserStorageStatsByType(userId, storageStats, type);
    service->GetCurrentBundleStats(bundleStats);
    service->GetBundleStats(pkgName, userId, bundleStats);
    service->GetBundleStatsForIncrease(userId, bundleName, incrementalBackTimes, pkgFileSizes);
    service->ResetBundleMgrProxy();
    return true;
}
} // namespace StorageManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StorageManager::StorageStatusServiceFuzzTest(data, size);
    return 0;
}
