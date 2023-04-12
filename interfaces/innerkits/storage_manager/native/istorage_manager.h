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
    virtual int32_t GetCurrentBundleStats(BundleStats &bundleStats) = 0;
    virtual int32_t NotifyVolumeCreated(VolumeCore vc) = 0;
    virtual int32_t NotifyVolumeMounted(std::string volumeId, int fsType, std::string fsUuid,
                                     std::string path, std::string description) = 0;
    virtual int32_t NotifyVolumeDestroyed(std::string volumeId) = 0;
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
    virtual int32_t UpdateUserAuth(uint32_t userId,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) = 0;
    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) = 0;
    virtual int32_t InactiveUserKey(uint32_t userId) = 0;
    virtual int32_t UpdateKeyContext(uint32_t userId) = 0;

    // app file share api
    virtual int32_t CreateShareFile(std::string uri, uint32_t tokenId, uint32_t flag) = 0;
    virtual int32_t DeleteShareFile(uint32_t tokenId, std::vector<std::string>sharePathList) = 0;

    enum {
        PREPARE_ADD_USER = 1,
        REMOVE_USER,
        PREPARE_START_USER,
        STOP_USER,
        GET_TOTAL,
        GET_FREE,
        GET_BUNDLE_STATUS,
        GET_SYSTEM_SIZE,
        GET_TOTAL_SIZE,
        GET_FREE_SIZE,
        GET_CURR_USER_STATS,
        GET_USER_STATS,
        GET_CURR_BUNDLE_STATS,
        NOTIFY_VOLUME_CREATED,
        NOTIFY_VOLUME_MOUNTED,
        NOTIFY_VOLUME_DESTROYED,
        MOUNT,
        UNMOUNT,
        GET_ALL_VOLUMES,
        NOTIFY_DISK_CREATED,
        NOTIFY_DISK_DESTROYED,
        PARTITION,
        GET_ALL_DISKS,
        CREATE_USER_KEYS,
        DELETE_USER_KEYS,
        UPDATE_USER_AUTH,
        ACTIVE_USER_KEY,
        INACTIVE_USER_KEY,
        UPDATE_KEY_CONTEXT,
        GET_VOL_BY_UUID,
        GET_VOL_BY_ID,
        SET_VOL_DESC,
        FORMAT,
        GET_DISK_BY_ID,
        CREATE_SHARE_FILE,
        DELETE_SHARE_FILE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.StorageManager.IStorageManager");
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_ISTORAGER_MANAGER_H