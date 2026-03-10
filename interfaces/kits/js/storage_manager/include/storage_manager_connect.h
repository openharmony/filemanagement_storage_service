/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ext_bundle_stats.h"
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
    int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex, uint32_t statFlag);
    int32_t GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize);
    int32_t GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize);
    int32_t Mount(const std::string &volumeId);
    int32_t Unmount(const std::string &volumeId);
    int32_t GetAllVolumes(std::vector<VolumeExternal> &vecOfVol);
    int32_t GetSystemSize(int64_t &systemSize);
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetFreeSize(int64_t &freeSize);
    int32_t GetTotalInodes(int64_t &totalInodes);
    int32_t GetFreeInodes(int64_t &freeInodes);
    int32_t GetCurrentBundleInodes(int64_t &curInodes);
    int32_t GetSystemDataSize(int64_t &systemDataSize);
    int32_t GetUserStorageStats(StorageStats &storageStats);
    int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats);
    int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, const std::string &type);
    int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag);
    int32_t GetVolumeByUuid(const std::string &uuid, VolumeExternal &vol);
    int32_t GetVolumeById(const std::string &volumeId, VolumeExternal &vol);
    int32_t SetVolumeDescription(const std::string &uuid, const std::string &description);
    int32_t Format(const std::string &volumeId, const std::string &fsType);
    int32_t Partition(const std::string &diskId, int32_t type);

    int32_t ResetProxy();
    int32_t DeactivateUserKey(uint32_t userId);
    int32_t SetExtBundleStats(uint32_t userId, ExtBundleStats &stats);
    int32_t GetExtBundleStats(uint32_t userId, ExtBundleStats &stats);
    int32_t GetAllExtBundleStats(uint32_t userId, std::vector<ExtBundleStats> &statsVec);
    int32_t ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs);

    //disk crypt api
    int32_t Encrypt(const std::string &volumeId, const std::string &pazzword);
    int32_t GetCryptProgressById(const std::string &volumeId, int32_t &progress);
    int32_t GetCryptUuidById(const std::string &volumeId, std::string &uuid);
    int32_t BindRecoverKeyToPasswd(const std::string &volumeId,
                                const std::string &pazzword,
                                const std::string &recoverKey);
    int32_t UpdateCryptPasswd(const std::string &volumeId,
                            const std::string &pazzword,
                            const std::string &newPazzword);
    int32_t ResetCryptPasswd(const std::string &volumeId,
                            const std::string &recoverKey,
                            const std::string &newPazzword);
    int32_t VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword);
    int32_t Unlock(const std::string &volumeId, const std::string &pazzword);
    int32_t Decrypt(const std::string &volumeId, const std::string &pazzword);
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

bool IsSystemApp();
int32_t Convert2JsErrNum(int32_t errNum);
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_CONNECT_H