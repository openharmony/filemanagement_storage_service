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

namespace OHOS {
namespace StorageManager {
class IStorageManager : public IRemoteBroker {
public:
    virtual int32_t PrepareAddUser(int32_t userId) = 0;
    virtual int32_t RemoveUser(int32_t userId) = 0;
    virtual int32_t PrepareStartUser(int32_t userId) = 0;
    virtual int32_t StopUser(int32_t userId) = 0;
    virtual int64_t GetFreeSizeOfVolume(std::string volumeUuid) = 0;
    virtual int64_t GetTotalSizeOfVolume(std::string volumeUuid) = 0;
    virtual std::vector<int64_t> GetBundleStats(std::string uuid, std::string pkgName) = 0;
    virtual void NotifyVolumeCreated(VolumeCore vc) = 0;
    virtual void NotifyVolumeMounted(std::string volumeId, int fsType, std::string fsUuid,
        std::string path, std::string description) = 0;
    virtual void NotifyVolumeDestoryed(std::string volumeId) = 0;
    virtual int32_t Mount(std::string volumeId) = 0;
    virtual int32_t Unmount(std::string volumeId) = 0;
    virtual std::vector<VolumeExternal> GetAllVolumes() = 0;
    virtual void NotifyDiskCreated(Disk disk) = 0;
    virtual void NotifyDiskDestroyed(std::string diskId) = 0;
    virtual int32_t Partition(std::string diskId, int32_t type) = 0;
    virtual std::vector<Disk> GetAllDisks() = 0;

    // fscrypt api
    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) = 0;
    virtual int32_t DeleteUserKeys(uint32_t userId) = 0;
    virtual int32_t UpdateUserAuth(uint32_t userId, std::string auth, std::string compSecret) = 0;
    virtual int32_t ActiveUserKey(uint32_t userId, std::string auth, std::string compSecret) = 0;
    virtual int32_t InactiveUserKey(uint32_t userId) = 0;

    enum {
        PREPARE_ADD_USER = 1,
        REMOVE_USER,
        PREPARE_START_USER,
        STOP_USER,
        GET_TOTAL,
        GET_FREE,
        GET_BUNDLE_STATUS,
        NOTIFY_VOLUME_CREATED,
        NOTIFY_VOLUME_MOUNTED,
        NOTIFY_VOLUME_DESTORYED,
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
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.StorageManager.IStorageManager");
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_ISTORAGER_MANAGER_H