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
#ifndef MOCK_STORAGE_MANAGER_SERVICE_H
#define MOCK_STORAGE_MANAGER_SERVICE_H

#include "gmock/gmock.h"
#include "iremote_stub.h"
#include "istorage_manager.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerServiceMock :  public IRemoteStub<IStorageManager> {
public:
    int code_ = 0;
    StorageManagerServiceMock() : code_(0) {}
    virtual ~StorageManagerServiceMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    int32_t InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return E_OK;
    }

    virtual int32_t PrepareAddUser(int32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t RemoveUser(int32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t PrepareStartUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t StopUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int64_t GetFreeSizeOfVolume(std::string volumeUuid) override
    {
        return E_OK;
    }

    virtual int64_t GetTotalSizeOfVolume(std::string volumeUuid) override
    {
        return E_OK;
    }

    virtual BundleStats GetBundleStats(std::string pkgName) override
    {
        BundleStats result;
        return result;
    }

    virtual int64_t GetSystemSize() override
    {
        return E_OK;
    }

    virtual int64_t GetTotalSize() override
    {
        return E_OK;
    }

    virtual int64_t GetFreeSize() override
    {
        return E_OK;
    }

    virtual StorageStats GetUserStorageStats() override
    {
        StorageStats result;
        return result;
    }

    virtual StorageStats GetUserStorageStats(int32_t userId) override
    {
        StorageStats result;
        return result;
    }

    virtual BundleStats GetCurrentBundleStats() override
    {
        BundleStats result;
        return result;
    }

    virtual void NotifyVolumeCreated(VolumeCore vc) override {}

    virtual void NotifyVolumeMounted(std::string volumeId, int fsType, std::string fsUuid,
                                     std::string path, std::string description) override {}

    virtual void NotifyVolumeDestroyed(std::string volumeId) override {}

    virtual int32_t Mount(std::string volumeId) override
    {
        return E_OK;
    }

    virtual int32_t Unmount(std::string volumeId) override
    {
        return E_OK;
    }

    virtual std::vector<VolumeExternal> GetAllVolumes() override
    {
        std::vector<VolumeExternal> result;
        return result;
    }

    virtual void NotifyDiskCreated(Disk disk) override {}

    virtual void NotifyDiskDestroyed(std::string diskId) override {}

    virtual int32_t Partition(std::string diskId, int32_t type) override
    {
        return E_OK;
    }

    virtual std::vector<Disk> GetAllDisks() override
    {
        std::vector<Disk> result;
        return result;
    }

    virtual int32_t GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc) override
    {
        return E_OK;
    }

    virtual int32_t GetVolumeById(std::string volumeId, VolumeExternal &vc) override
    {
        return E_OK;
    }

    virtual int32_t SetVolumeDescription(std::string fsUuid, std::string description) override
    {
        return E_OK;
    }

    virtual int32_t Format(std::string volumeId, std::string fsType) override
    {
        return E_OK;
    }

    virtual int32_t GetDiskById(std::string diskId, Disk &disk) override
    {
        return E_OK;
    }

    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t DeleteUserKeys(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t UpdateUserAuth(uint32_t userId,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) override
    {
        return E_OK;
    }

    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) override
    {
        return E_OK;
    }

    virtual int32_t InactiveUserKey(uint32_t userId) override
    {
        return E_OK;
    }
    
    virtual int32_t UpdateKeyContext(uint32_t userId) override
    {
        return E_OK;
    }
};
} // namespace StorageManager
} // namespace OHOS
#endif // MOCK_STORAGE_MANAGER_SERVICE_H