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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MANAGER_PROXY_H
#define OHOS_STORAGE_MANAGER_STORAGE_MANAGER_PROXY_H

#include "iremote_proxy.h"
#include "istorage_manager.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerProxy : public IRemoteProxy<IStorageManager> {
public:
    explicit StorageManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IStorageManager>(impl) {}
    ~StorageManagerProxy() override {}

    int32_t PrepareAddUser(int32_t userId, uint32_t flags) override;
    int32_t RemoveUser(int32_t userId, uint32_t flags) override;
    int32_t PrepareStartUser(int32_t userId) override;
    int32_t StopUser(int32_t userId) override;
    int64_t GetFreeSizeOfVolume(std::string volumeUuid) override;
    int64_t GetTotalSizeOfVolume(std::string volumeUuid) override;
    std::vector<int64_t> GetBundleStats(std::string pkgName) override;
    int64_t GetSystemSize() override;
    int64_t GetTotalSize() override;
    int64_t GetFreeSize() override;
    std::vector<int64_t> GetStorageTotalStats() override;
    std::vector<int64_t> GetUserStorageStats(int32_t userId) override;
    std::vector<int64_t> GetAppStorageStats() override;
    void NotifyVolumeCreated(VolumeCore vc) override;
    void NotifyVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
                             std::string path, std::string description) override;
    void NotifyVolumeDestroyed(std::string volumeId) override;
    int32_t Mount(std::string volumeId) override;
    int32_t Unmount(std::string volumeId) override;
    std::vector<VolumeExternal> GetAllVolumes() override;
    void NotifyDiskCreated(Disk disk) override;
    void NotifyDiskDestroyed(std::string diskId) override;
    int32_t Partition(std::string diskId, int32_t type) override;
    std::vector<Disk> GetAllDisks() override;

    // fscrypt api
    int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) override;
    int32_t DeleteUserKeys(uint32_t userId) override;
    int32_t UpdateUserAuth(uint32_t userId, std::string auth, std::string compSecret) override;
    int32_t ActiveUserKey(uint32_t userId, std::string auth, std::string compSecret) override;
    int32_t InactiveUserKey(uint32_t userId) override;
    int32_t UpdateKeyContext(uint32_t userId) override;

private:
    static inline BrokerDelegator<StorageManagerProxy> delegator_;
};
} // StorageManager   
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MANAGER_PROXY_H
