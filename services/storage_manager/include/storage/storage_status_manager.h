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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_STATUS_MANAGER_H
#define OHOS_STORAGE_MANAGER_STORAGE_STATUS_MANAGER_H

#include <iostream>
#include <map>
#include <sstream>
#include <singleton.h>
#include <thread>

#include "bundle_stats.h"
#include "ext_bundle_stats.h"
#include "storage_stats.h"

namespace OHOS {
namespace StorageManager {
class StorageStatusManager : public NoCopyable  {
public:
    static StorageStatusManager &GetInstance()
    {
        static StorageStatusManager instance;
        return instance;
    }
    int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex, uint32_t statFlag);
    int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule = false);
    int32_t GetUserStorageStats(StorageStats &storageStats);
    int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type);
    int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag);
    int32_t GetBundleStats(const std::string &pkgName, int32_t userId, BundleStats &bundleStats,
        int32_t appIndex, uint32_t statFlag);
    int32_t GetBundleNameAndUid(int32_t userId, std::map<int32_t, std::string> &bundleNameAndUid);
    int32_t DelBundleExtStats(uint32_t userId, const std::string &bundleName);
    int32_t SetExtBundleStats(uint32_t userId, const ExtBundleStats &stats);
    int32_t GetExtBundleStats(uint32_t userId, ExtBundleStats &stats);
    int32_t GetAllExtBundleStats(uint32_t userId, std::vector<ExtBundleStats> &statsVec);

private:
    StorageStatusManager();
    ~StorageStatusManager();
    std::string ConvertBytesToMB(int64_t bytes);
    int GetCurrentUserId();
    std::string GetCallingPkgName();
    int32_t GetAppSize(int32_t userId, int64_t &size);
    const std::vector<std::string> dataDir = {"app", "local", "distributed", "database", "cache"};
    const int DEFAULT_APP_INDEX = 0;
    enum BUNDLE_STATS {APP = 0, LOCAL, DISTRIBUTED, DATABASE, CACHE};
    enum BUNDLE_STATS_RESULT {APPSIZE = 0, CACHESIZE, DATASIZE};
    int32_t GetMediaAndFileStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule = false);
    int32_t GetBundleName(uint32_t userId, const std::string &businessName, std::string &dbBundleName);
    int32_t InsertOrUpdateExtBundleStats(uint32_t userId, const ExtBundleStats &stats, std::string callingBundleName);
    std::string GetCallingBundleName();
    std::mutex extBundleMtx_;
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_STATUS_MANAGER_H