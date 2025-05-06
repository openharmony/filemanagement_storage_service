/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ohos.file.storageStatistics.impl.h"

using namespace taihe;
using namespace ANI::storageStatistics;
namespace ANI::storageStatistics {

int64_t GetFreeSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();

    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetFreeSize(*resultSize);
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get free size.");
        return -1;
    }

    return *resultSize;
}

int64_t GetTotalSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();

    int32_t errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetTotalSize(*resultSize);
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get total size.");
        return -1;
    }

    return *resultSize;
}

BundleStats MakeBundleStats(int64_t a, int64_t b, int64_t c)
{
    return {a, b, c};
}

StorageStats MakeStorageStats(int64_t a)
{
    return {a};
}

BundleStats GetCurrentBundleStatsSync()
{
    uint32_t statFlag = 0;
    auto resultStats = std::make_shared<OHOS::StorageManager::BundleStats>();

    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetCurrentBundleStats(
            *resultStats, statFlag);
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get current bundle stats.");
        return MakeBundleStats(-1, -1, -1);
    }

    return MakeBundleStats((*resultStats).dataSize_, (*resultStats).cacheSize_, (*resultStats).appSize_);
}

StorageStats GetUserStorageStatsSync()
{
    if (!OHOS::StorageManager::IsSystemApp()) {
        set_business_error(OHOS::E_PERMISSION_SYS, "GetUserStorageStatsSync is not allowed for non-system apps");
        return MakeBundleStats(-1);
    }

    auto resultStats = std::make_shared<OHOS::StorageManager::StorageStats>();

    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetUserStorageStats(
            *resultStats);
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get user storage stats");
        return MakeStorageStats(-1);
    }

    return MakeStorageStats((*resultStats).total_);
}

StorageStats GetUserStorageStatsByidSync(int64_t userId)
{
    int32_t userId_i = static_cast<int32_t>(userId);

    if (!OHOS::StorageManager::IsSystemApp()) {
        set_business_error(OHOS::E_PERMISSION_SYS, "GetUserStorageStatsSync is not allowed for non-system apps");
        return MakeStorageStats(-1);
    }

    auto resultStats = std::make_shared<OHOS::StorageManager::StorageStats>();

    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetUserStorageStats(
            userId_i, *resultStats);
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get user storage stats by ID");
        return MakeStorageStats(-1);
    }
    return MakeStorageStats((*resultStats).total_);
}
} // namespace ANI::storageStatistics

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetFreeSizeSync(GetFreeSizeSync);
TH_EXPORT_CPP_API_GetTotalSizeSync(GetTotalSizeSync);
TH_EXPORT_CPP_API_MakeBundleStats(MakeBundleStats);
TH_EXPORT_CPP_API_MakeStorageStats(MakeStorageStats);
TH_EXPORT_CPP_API_GetCurrentBundleStatsSync(GetCurrentBundleStatsSync);
TH_EXPORT_CPP_API_GetUserStorageStatsSync(GetUserStorageStatsSync);
TH_EXPORT_CPP_API_GetUserStorageStatsByidSync(GetUserStorageStatsByidSync);
// NOLINTEND
