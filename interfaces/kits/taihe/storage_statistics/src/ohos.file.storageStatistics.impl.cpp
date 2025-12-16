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
#include "storageStatistics_taihe_error.h"
#include "storage_service_log.h"

namespace ANI::StorageStatistics {
constexpr int64_t DEFAULTSIZE = -1;

int64_t GetFreeSizeSync2()
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

int64_t GetTotalSizeSync2()
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

int64_t GetFreeSizeOfVolumeSync(::taihe::string_view volumeUuid)
{
    std::string uid = std::string(volumeUuid);
    auto resultSize = std::make_shared<int64_t>();
    int32_t errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetFreeSizeOfVolume(
            uid, *resultSize);
        if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return DEFAULTSIZE;
    }
    return *resultSize;
}

int64_t GetSystemSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();
    int32_t errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetSystemSize(*resultSize);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return DEFAULTSIZE;
    }
    return *resultSize;
}

int64_t GetTotalSizeOfVolumeSync(::taihe::string_view volumeUuid)
{
    std::string uid = std::string(volumeUuid);
    auto resultSize = std::make_shared<int64_t>();
    int32_t errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetTotalSizeOfVolume(
            uid, *resultSize);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return DEFAULTSIZE;
    }
    return *resultSize;
}

::ohos::file::storageStatistics::BundleStats GetBundleStatsSync(::taihe::string_view packageName,
    ::taihe::optional_view<int32_t> index)
{
    std::string nameString = std::string(packageName);
    if (nameString.empty()) {
        LOGE("packageName is empty!");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };
    }
    int32_t indexVelue = 0;
    if (index.has_value()) {
        indexVelue = static_cast<int32_t>(index.value());
    }
    uint32_t statFlag = 0;
    auto bundleStats = std::make_shared<OHOS::StorageManager::BundleStats>();
    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetBundleStats(nameString,
            *bundleStats, indexVelue, statFlag);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };
    }

    return { (*bundleStats).dataSize_, (*bundleStats).cacheSize_, (*bundleStats).appSize_} ;
}

int64_t GetFreeSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();
    int32_t errNum = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetFreeSize(
        *resultSize);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return DEFAULTSIZE;
    }
    return *resultSize;
}

int64_t GetTotalSizeSync()
{
    auto resultSize = std::make_shared<int64_t>();
    int32_t errNum = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetTotalSize(
        *resultSize);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return DEFAULTSIZE;
    }
    return *resultSize;
}

void SetExtBundleStatsSync(int32_t userId, ohos::file::storageStatistics::ExtBundleStats stats)
{
    if (userId < 0 || stats.size < 0) {
        taihe::set_error("SetExtBundleStatsSync::Invaild params.");
        return;
    }
    uint32_t userId_i = static_cast<uint32_t>(userId);
    OHOS::StorageManager::ExtBundleStats extBundleStats;
    extBundleStats.businessName_ = stats.businessName.c_str();
    extBundleStats.businessSize_ = static_cast<uint64_t>(stats.size);
    extBundleStats.showFlag_ = stats.flag;
    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->SetExtBundleStats(
            userId_i, extBundleStats);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum),
            "Failed to set ext bundle stats.");
        return;
    }
}

ohos::file::storageStatistics::ExtBundleStats GetExtBundleStatsSync(int32_t userId, taihe::string_view businessName)
{
    if (userId < 0) {
        taihe::set_error("GetExtBundleStatsSync::Invaild params.");
        return { "", DEFAULTSIZE, false };
    }
    uint32_t userId_i = static_cast<uint32_t>(userId);
    OHOS::StorageManager::ExtBundleStats extBundleStats;
    extBundleStats.businessName_ = businessName.c_str();
    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetExtBundleStats(
            userId_i, extBundleStats);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum),
            "Failed to get ext bundle stats");
        return { "", DEFAULTSIZE, false };
    }
    int64_t businessSize_o = static_cast<int64_t>(extBundleStats.businessSize_);
    return { businessName, businessSize_o, extBundleStats.showFlag_};
}

taihe::array<ohos::file::storageStatistics::ExtBundleStats> GetAllExtBundleStatsSync(int32_t userId)
{
    if (userId < 0) {
        taihe::set_error("GetAllExtBundleStatsSync::Invaild params.");
        return taihe::array<ohos::file::storageStatistics::ExtBundleStats>::make(0,
            ohos::file::storageStatistics::ExtBundleStats{});
    }
    uint32_t userId_i = static_cast<uint32_t>(userId);
    std::vector<OHOS::StorageManager::ExtBundleStats> statsVec;
    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->GetAllExtBundleStats(
            userId_i, statsVec);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum),
            "Failed to get all ext bundle stats");
        return taihe::array<ohos::file::storageStatistics::ExtBundleStats>::make(0,
            ohos::file::storageStatistics::ExtBundleStats{});
    }
    auto result = taihe::array<ohos::file::storageStatistics::ExtBundleStats>::
        make(statsVec.size(), ohos::file::storageStatistics::ExtBundleStats{});
    auto extBundleStatsTransformer = [](auto &stats) -> ohos::file::storageStatistics::ExtBundleStats {
        return { stats.businessName_, stats.businessSize_, stats.showFlag_ };
    };
    std::transform(statsVec.begin(), statsVec.end(), result.begin(), extBundleStatsTransformer);
    return taihe::array<ohos::file::storageStatistics::ExtBundleStats>(taihe::copy_data_t{},
        result.data(), result.size());
}

taihe::array<ohos::file::storageStatistics::UserdataDirInfo> ListUserdataDirInfoSync()
{
    auto scanDirs = std::make_shared<std::vector<OHOS::StorageManager::UserdataDirInfo>>();

    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("Get StorageManagerConnect instacne failed");
        return taihe::array<ohos::file::storageStatistics::UserdataDirInfo>::make(0,
            ohos::file::storageStatistics::UserdataDirInfo{});
    }

    int32_t errNum = instance->ListUserdataDirInfo(*scanDirs);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "ListUserdataDirInfoSync failed");
        return taihe::array<ohos::file::storageStatistics::UserdataDirInfo>::make(0,
            ohos::file::storageStatistics::UserdataDirInfo{});
    }

    auto result = taihe::array<ohos::file::storageStatistics::UserdataDirInfo>::
        make(scanDirs->size(), ohos::file::storageStatistics::UserdataDirInfo{});
    auto volumeTransformer = [](auto &dir) -> ohos::file::storageStatistics::UserdataDirInfo {
        return {dir.path_, dir.totalSize_, dir.totalCnt_};
    };
    std::transform(scanDirs->begin(), scanDirs->end(), result.begin(), volumeTransformer);

    return taihe::array<ohos::file::storageStatistics::UserdataDirInfo>(taihe::copy_data_t{},
        result.data(), result.size());
}
} // namespace ANI::storageStatistics

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetFreeSizeSync2(ANI::StorageStatistics::GetFreeSizeSync2);
TH_EXPORT_CPP_API_GetTotalSizeSync2(ANI::StorageStatistics::GetTotalSizeSync2);
TH_EXPORT_CPP_API_GetCurrentBundleStatsSync(ANI::StorageStatistics::GetCurrentBundleStatsSync);
TH_EXPORT_CPP_API_GetUserStorageStatsSync(ANI::StorageStatistics::GetUserStorageStatsSync);
TH_EXPORT_CPP_API_GetUserStorageStatsByidSync(ANI::StorageStatistics::GetUserStorageStatsByidSync);
TH_EXPORT_CPP_API_SetExtBundleStatsSync(ANI::StorageStatistics::SetExtBundleStatsSync);
TH_EXPORT_CPP_API_GetExtBundleStatsSync(ANI::StorageStatistics::GetExtBundleStatsSync);
TH_EXPORT_CPP_API_GetAllExtBundleStatsSync(ANI::StorageStatistics::GetAllExtBundleStatsSync);
TH_EXPORT_CPP_API_ListUserdataDirInfoSync(ANI::StorageStatistics::ListUserdataDirInfoSync);
TH_EXPORT_CPP_API_GetFreeSizeOfVolumeSync(ANI::StorageStatistics::GetFreeSizeOfVolumeSync);
TH_EXPORT_CPP_API_GetSystemSizeSync(ANI::StorageStatistics::GetSystemSizeSync);
TH_EXPORT_CPP_API_GetTotalSizeOfVolumeSync(ANI::StorageStatistics::GetTotalSizeOfVolumeSync);
TH_EXPORT_CPP_API_GetBundleStatsSync(ANI::StorageStatistics::GetBundleStatsSync);
TH_EXPORT_CPP_API_GetFreeSizeSync(ANI::StorageStatistics::GetFreeSizeSync);
TH_EXPORT_CPP_API_GetTotalSizeSync(ANI::StorageStatistics::GetTotalSizeSync);
// NOLINTEND
