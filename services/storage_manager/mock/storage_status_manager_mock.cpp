/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "ext_bundle_stats.h"
#include "mock/storage_status_manager_mock.h"
namespace OHOS {
namespace StorageManager {

int32_t StorageStatusManager::GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex,
    uint32_t statFlag)
{
    return IStorageStatusManagerMock::storageStatusManager->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
}

int32_t StorageStatusManager::GetUserStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule)
{
    return IStorageStatusManagerMock::storageStatusManager->GetUserStorageStats(userId, storageStats, isSchedule);
}

int32_t StorageStatusManager::GetUserStorageStats(StorageStats &storageStats)
{
    return IStorageStatusManagerMock::storageStatusManager->GetUserStorageStats(storageStats);
}

int32_t StorageStatusManager::GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type)
{
    return IStorageStatusManagerMock::storageStatusManager->GetUserStorageStatsByType(userId, storageStats, type);
}

int32_t StorageStatusManager::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
    return IStorageStatusManagerMock::storageStatusManager->GetCurrentBundleStats(bundleStats, statFlag);
}

int32_t StorageStatusManager::GetBundleStats(const std::string &pkgName, int32_t userId, BundleStats &bundleStats,
    int32_t appIndex, uint32_t statFlag)
{
    return IStorageStatusManagerMock::storageStatusManager->GetBundleStats(pkgName, userId, bundleStats,
        appIndex, statFlag);
}

int32_t StorageStatusManager::SetExtBundleStats(uint32_t userId, const ExtBundleStats &stats)
{
    return IStorageStatusManagerMock::storageStatusManager->SetExtBundleStats(userId, stats);
}

int32_t StorageStatusManager::GetExtBundleStats(uint32_t userId, ExtBundleStats &stats)
{
    return IStorageStatusManagerMock::storageStatusManager->GetExtBundleStats(userId, stats);
}

int32_t GetAllExtBundleStats(uint32_t userId, std::vector<ExtBundleStats> &statsVec)
{
    return IStorageStatusManagerMock::storageStatusManager->GetAllExtBundleStats(userId, statsVec);
}

int32_t StorageStatusManager::GetBundleNameAndUid(int32_t userId, std::map<int32_t, std::string> &bundleNameAndUid)
{
    return IStorageStatusManagerMock::storageStatusManager->GetBundleNameAndUid(userId, bundleNameAndUid);
}

int32_t StorageStatusManager::DelBundleExtStats(uint32_t userId, const std::string &bundleName)
{
    return IStorageStatusManagerMock::storageStatusManager->DelBundleExtStats(userId, bundleName);
}
} // StorageManager
} // OHOS