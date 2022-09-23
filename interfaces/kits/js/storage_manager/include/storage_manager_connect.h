/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_CONNECT_H
#define OHOS_STORAGE_MANAGER_CONNECT_H

#include <nocopyable.h>
#include <singleton.h>

#include "system_ability.h"
#include "istorage_manager.h"
#include "bundle_stats.h"
#include "storage_stats.h"
#include "volume_external.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerConnect : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageManagerConnect);
public:
    int32_t Connect();
    BundleStats GetBundleStats(std::string pkgName);
    int64_t GetFreeSizeOfVolume(std::string volumeUuid);
    int64_t GetTotalSizeOfVolume(std::string volumeUuid);
    bool Mount(std::string volumeId);
    bool Unmount(std::string volumeId);
    std::vector<VolumeExternal> GetAllVolumes();
    int64_t GetSystemSize();
    int64_t GetTotalSize();
    int64_t GetFreeSize();
    StorageStats GetUserStorageStats();
    StorageStats GetUserStorageStats(int32_t userId);
    BundleStats GetCurrentBundleStats();
    VolumeExternal GetVolumeByUuid(std::string uuid);
    VolumeExternal GetVolumeById(std::string volumeId);
    bool SetVolumeDescription(std::string uuid, std::string description);
    bool Format(std::string volumeId, std::string fsType);
    bool Partition(std::string diskId, int32_t type);

    int32_t ResetProxy();
private:
    sptr<StorageManager::IStorageManager> storageManager_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::mutex mutex_;
};

class SmDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    SmDeathRecipient() = default;
    virtual ~SmDeathRecipient() = default;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
};
} // StorageManager
} // OHOS

#endif