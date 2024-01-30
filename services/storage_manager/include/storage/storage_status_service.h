/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_STATUS_SERVICE_H
#define OHOS_STORAGE_MANAGER_STORAGE_STATUS_SERVICE_H

#include <vector>
#include <nocopyable.h>
#include <singleton.h>
#include <iostream>
#include "bundle_stats.h"
#include "storage_stats.h"
#include "bundle_mgr_interface.h"
#include "iremote_object.h"

namespace OHOS {
namespace StorageManager {
class StorageStatusService : public NoCopyable  {
    DECLARE_DELAYED_SINGLETON(StorageStatusService);

public:
    int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats);
    int32_t GetUserStorageStats(StorageStats &storageStats);
    int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats);
    int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type);
    int32_t GetCurrentBundleStats(BundleStats &bundleStats);
    int32_t GetBundleStats(const std::string &pkgName, int32_t userId, BundleStats &bundleStats);
    int32_t GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
        const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes);
    int32_t ResetBundleMgrProxy();

private:
    int GetCurrentUserId();
    std::string GetCallingPkgName();
    int32_t ConnectBundleMgr();
    int32_t GetAppSize(int32_t userId, int64_t &size);
    const std::vector<std::string> dataDir = {"app", "local", "distributed", "database", "cache"};
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::mutex mutex_;
    const int DEFAULT_USER_ID = 100;
    enum BUNDLE_STATS {APP = 0, LOCAL, DISTRIBUTED, DATABASE, CACHE};
    enum BUNDLE_STATS_RESULT {APPSIZE = 0, CACHESIZE, DATASIZE};
};

class BundleMgrDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    BundleMgrDeathRecipient() = default;
    virtual ~BundleMgrDeathRecipient() = default;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_STATUS_SERVICE_H