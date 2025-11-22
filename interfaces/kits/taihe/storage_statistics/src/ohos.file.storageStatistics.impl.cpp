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

namespace ANI::StorageStatistics {
constexpr int64_t DEFAULTSIZE = -1;

int64_t GetFreeSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();

    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetFreeSize(*resultSize);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get free size.");
        return DEFAULTSIZE;
    }

    return *resultSize;
}

int64_t GetTotalSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();

    int32_t errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetTotalSize(*resultSize);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get total size.");
        return DEFAULTSIZE;
    }

    return *resultSize;
}

ohos::file::storageStatistics::BundleStats GetCurrentBundleStatsSync()
{
    uint32_t statFlag = 0;
    auto resultStats = std::make_shared<OHOS::StorageManager::BundleStats>();

    auto errNum = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->
        GetCurrentBundleStats(*resultStats, statFlag);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum),
            "Failed to get current bundle stats.");
        return { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };
    }
    return { (*resultStats).dataSize_, (*resultStats).cacheSize_, (*resultStats).appSize_ };
}

ohos::file::storageStatistics::StorageStats GetUserStorageStatsSync()
{
    auto resultStats = std::make_shared<OHOS::StorageManager::StorageStats>();

    auto errNum = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->
        GetUserStorageStats(*resultStats);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to get user storage stats");
        return { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };
    }

    return { (*resultStats).total_, (*resultStats).audio_, (*resultStats).video_, (*resultStats).image_,
        (*resultStats).file_, (*resultStats).app_ };
}

ohos::file::storageStatistics::StorageStats GetUserStorageStatsByidSync(int64_t userId)
{
    int32_t userId_i = static_cast<int32_t>(userId);
    auto resultStats = std::make_shared<OHOS::StorageManager::StorageStats>();

    auto errNum = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->
        GetUserStorageStats(userId_i, *resultStats);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum),
            "Failed to get user storage stats by ID");
        return { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };
    }
    return { (*resultStats).total_, (*resultStats).audio_, (*resultStats).video_, (*resultStats).image_,
        (*resultStats).file_, (*resultStats).app_ };
}
} // namespace ANI::storageStatistics

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetFreeSizeSync(ANI::StorageStatistics::GetFreeSizeSync);
TH_EXPORT_CPP_API_GetTotalSizeSync(ANI::StorageStatistics::GetTotalSizeSync);
TH_EXPORT_CPP_API_GetCurrentBundleStatsSync(ANI::StorageStatistics::GetCurrentBundleStatsSync);
TH_EXPORT_CPP_API_GetUserStorageStatsSync(ANI::StorageStatistics::GetUserStorageStatsSync);
TH_EXPORT_CPP_API_GetUserStorageStatsByidSync(ANI::StorageStatistics::GetUserStorageStatsByidSync);
// NOLINTEND
