/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef STORAGE_STATUS_MANAGER_MOCK_H
#define STORAGE_STATUS_MANAGER_MOCK_H

#include <gmock/gmock.h>

#include "ext_bundle_stats.h"
#include "storage/storage_status_manager.h"

namespace OHOS {
namespace StorageManager {

class IStorageStatusManagerMock {
public:
    virtual ~IStorageStatusManagerMock() = default;
    virtual int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex,
        uint32_t statFlag);
    virtual int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule = false);
    virtual int32_t GetUserStorageStats(StorageStats &storageStats);
    virtual int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type);
    virtual int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag);
    virtual int32_t GetBundleStats(const std::string &pkgName, int32_t userId, BundleStats &bundleStats,
        int32_t appIndex, uint32_t statFlag);
    virtual int32_t GetBundleNameAndUid(int32_t userId, std::map<int32_t, std::string> &bundleNameAndUid);
    virtual int32_t DelBundleExtStats(uint32_t userId, const std::string &bundleName);
    virtual int32_t SetExtBundleStats(uint32_t userId, const ExtBundleStats &stats);
    virtual int32_t GetExtBundleStats(uint32_t userId, ExtBundleStats &stats);
    virtual int32_t GetAllExtBundleStats(uint32_t userId, std::vector<ExtBundleStats> &statsVec);
    static inline std::shared_ptr<IStorageStatusManagerMock> storageStatusManager = nullptr;
};

class StorageStatusManagerMock : public IStorageStatusManagerMock {
public:
    MOCK_METHOD(int32_t, GetBundleStats, (const std::string &, BundleStats &, int32_t, uint32_t));
    MOCK_METHOD(int32_t, GetUserStorageStats, (int32_t, StorageStats &, bool));
    MOCK_METHOD(int32_t, GetUserStorageStats, (StorageStats &));
    MOCK_METHOD(int32_t, GetUserStorageStatsByType, (int32_t, StorageStats &, std::string));
    MOCK_METHOD(int32_t, GetCurrentBundleStats, (BundleStats &, uint32_t));
    MOCK_METHOD(int32_t, GetBundleStats, (const std::string &, int32_t, BundleStats &, int32_t, uint32_t));
    MOCK_METHOD(int32_t, SetExtBundleStats, (uint32_t, const ExtBundleStats &));
    MOCK_METHOD(int32_t, GetExtBundleStats, (uint32_t, ExtBundleStats &));
    MOCK_METHOD(int32_t, GetAllExtBundleStats, (uint32_t, std::vector<ExtBundleStats> &));
    MOCK_METHOD(int32_t, GetBundleNameAndUid, (int32_t, (std::map<int32_t, std::string> &)));
    MOCK_METHOD(int32_t, DelBundleExtStats, (uint32_t, const std::string &));
};
} // StorageManager
} // OHOS

#endif // STORAGE_STATUS_MANAGER_MOCK_H