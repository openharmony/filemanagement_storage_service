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
#include "mock/storage_status_service_mock.h"
namespace OHOS {
namespace StorageManager {

int32_t StorageStatusService::GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex,
    uint32_t statFlag)
{
    return IStorageStatusServiceMock::storageStatusService->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
}

int32_t StorageStatusService::GetUserStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule)
{
    return IStorageStatusServiceMock::storageStatusService->GetUserStorageStats(userId, storageStats, isSchedule);
}

int32_t StorageStatusService::GetUserStorageStats(StorageStats &storageStats)
{
    return IStorageStatusServiceMock::storageStatusService->GetUserStorageStats(storageStats);
}

int32_t StorageStatusService::GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type)
{
    return IStorageStatusServiceMock::storageStatusService->GetUserStorageStatsByType(userId, storageStats, type);
}

int32_t StorageStatusService::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
    return IStorageStatusServiceMock::storageStatusService->GetCurrentBundleStats(bundleStats, statFlag);
}

int32_t StorageStatusService::GetBundleStats(const std::string &pkgName, int32_t userId, BundleStats &bundleStats,
    int32_t appIndex, uint32_t statFlag)
{
    return IStorageStatusServiceMock::storageStatusService->GetBundleStats(pkgName, userId, bundleStats,
        appIndex, statFlag);
}

int32_t StorageStatusService::SetExtBundleStats(uint32_t userId, const std::string &businessName,
    uint64_t businessSize)
{
    return IStorageStatusServiceMock::storageStatusService->SetExtBundleStats(userId, businessName, businessSize);
}

int32_t StorageStatusService::GetExtBundleStats(uint32_t userId, const std::string &businessName,
    uint64_t &businessSize)
{
    return IStorageStatusServiceMock::storageStatusService->GetExtBundleStats(userId, businessName, businessSize);
}

int32_t StorageStatusService::GetBundleNameAndUid(int32_t userId, std::map<int32_t, std::string> &bundleNameAndUid)
{
    return IStorageStatusServiceMock::storageStatusService->GetBundleNameAndUid(userId, bundleNameAndUid);
}

int32_t StorageStatusService::DelBundleExtStats(uint32_t userId, const std::string &bundleName)
{
    return IStorageStatusServiceMock::storageStatusService->DelBundleExtStats(userId, bundleName);
}
} // StorageManager
} // OHOS