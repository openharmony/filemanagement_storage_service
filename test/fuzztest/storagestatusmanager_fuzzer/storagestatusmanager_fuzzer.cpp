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
#include "storagestatusmanager_fuzzer.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_status_manager.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageManager {
bool StorageStatusManagerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int64_t))) {
        return false;
    }
    StorageStatusManager &service = StorageStatusManager::GetInstance();
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    int64_t metaData2 = *(reinterpret_cast<const int64_t *>(data + sizeof(int32_t)));
    int32_t pos = sizeof(int32_t) + sizeof(int64_t);
    int32_t len = (size - pos) / 3;
    std::string pkgName(reinterpret_cast<const char *>(data + pos), len);
    std::string type(reinterpret_cast<const char *>(data + pos + len), len);
    std::string metaData(reinterpret_cast<const char *>(data + pos + len + len), len);
    bool isSchedule(static_cast<bool>(data + pos + len + len + sizeof(bool)));
    BundleStats bundleStats;
    StorageStats storageStats;
    std::vector<std::string> bundleName;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    std::vector<int64_t> incPkgFileSizes;
    bundleName.push_back(metaData);
    incrementalBackTimes.push_back(metaData2);
    pkgFileSizes.push_back(metaData2);
    incPkgFileSizes.push_back(metaData2);
    service.GetBundleStats(pkgName, bundleStats, 0, 0);
    service.GetUserStorageStats(storageStats);
    service.GetUserStorageStats(userId, storageStats, isSchedule);
    service.GetUserStorageStatsByType(userId, storageStats, type);
    service.GetCurrentBundleStats(bundleStats, 0);
    service.GetBundleStats(pkgName, userId, bundleStats, 0, 0);
    BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    return true;
}
} // namespace StorageManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StorageManager::StorageStatusManagerFuzzTest(data, size);
    return 0;
}
