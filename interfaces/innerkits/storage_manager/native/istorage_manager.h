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

#ifndef OHOS_STORAGE_MANAGER_ISTORAGE_MANAGER_H
#define OHOS_STORAGE_MANAGER_ISTORAGE_MANAGER_H

#include "iremote_broker.h"
#include "volume_core.h"
#include "volume_external.h"
#include "disk.h"
#include "bundle_stats.h"
#include "storage_stats.h"

namespace OHOS {
namespace StorageManager {
// PrepareAddUser flags
enum {
    CRYPTO_FLAG_EL1 = 1,
    CRYPTO_FLAG_EL2 = 2,
    CRYPTO_FLAG_EL3 = 4,
    CRYPTO_FLAG_EL4 = 8,
    CRYPTO_FLAG_EL5 = 16,
};
class IStorageManager : public IRemoteBroker {
public:
    virtual int32_t PrepareAddUser(int32_t userId, uint32_t flags) = 0;
    virtual int32_t RemoveUser(int32_t userId, uint32_t flags) = 0;
    virtual int32_t PrepareStartUser(int32_t userId) = 0;
    virtual int32_t StopUser(int32_t userId) = 0;
    virtual int32_t GetFreeSizeOfVolume(std::string volumeUuid, int64_t &freeSize) = 0;
    virtual int32_t GetTotalSizeOfVolume(std::string volumeUuid, int64_t &totalSize) = 0;
    virtual int32_t GetBundleStats(std::string pkgName, BundleStats &bundleStats) = 0;
    virtual int32_t GetSystemSize(int64_t &systemSize) = 0;
    virtual int32_t GetTotalSize(int64_t &totalSize) = 0;
    virtual int32_t GetFreeSize(int64_t &freeSize) = 0;
    virtual int32_t GetUserStorageStats(StorageStats &storageStats) = 0;
    virtual int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats) = 0;
    virtual int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type) = 0;
    virtual int32_t GetCurrentBundleStats(BundleStats &bundleStats) = 0;
    virtual int32_t NotifyVolumeCreated(VolumeCore vc) = 0;
    virtual int32_t NotifyVolumeMounted(std::string volumeId, int fsType, std::string fsUuid,
                                     std::string path, std::string description) = 0;
    virtual int32_t NotifyVolumeStateChanged(std::string volumeId, VolumeState state) = 0;
    virtual int32_t Mount(std::string volumeId) = 0;
    virtual int32_t Unmount(std::string volumeId) = 0;
    virtual int32_t GetAllVolumes(std::vector<VolumeExternal> &vecOfVol) = 0;
    virtual int32_t NotifyDiskCreated(Disk disk) = 0;
    virtual int32_t NotifyDiskDestroyed(std::string diskId) = 0;
    virtual int32_t Partition(std::string diskId, int32_t type) = 0;
    virtual int32_t GetAllDisks(std::vector<Disk> &vecOfDisk) = 0;
    virtual int32_t GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc) = 0;
    virtual int32_t GetVolumeById(std::string volumeId, VolumeExternal &vc) = 0;
    virtual int32_t SetVolumeDescription(std::string fsUuid, std::string description) = 0;
    virtual int32_t Format(std::string volumeId, std::string fsType) = 0;
    virtual int32_t GetDiskById(std::string diskId, Disk &disk) = 0;

    // fscrypt api
    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) = 0;
    virtual int32_t DeleteUserKeys(uint32_t userId) = 0;
    virtual int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) = 0;
    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) = 0;
    virtual int32_t InactiveUserKey(uint32_t userId) = 0;
    virtual int32_t UpdateKeyContext(uint32_t userId) = 0;
    virtual int32_t LockUserScreen(uint32_t userId) = 0;
    virtual int32_t UnlockUserScreen(uint32_t userId) = 0;
    virtual int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus) = 0;
    virtual int32_t GenerateAppkey(uint32_t appUid, std::string &keyId) = 0;
    virtual int32_t DeleteAppkey(const std::string keyId) = 0;

    // app file share api
    virtual std::vector<int32_t> CreateShareFile(const std::vector<std::string> &uriList,
                                                uint32_t tokenId, uint32_t flag) = 0;
    virtual int32_t DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &uriList) = 0;

    virtual int32_t SetBundleQuota(const std::string &bundleName, int32_t uid, const std::string &bundleDataDirPath,
        int32_t limitSizeMb)
    {
        return 0;
    }

    virtual int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize) = 0;
    virtual int32_t GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
        const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes) = 0;

    // dfs service
    virtual int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.StorageManager.IStorageManager");
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_ISTORAGER_MANAGER_H