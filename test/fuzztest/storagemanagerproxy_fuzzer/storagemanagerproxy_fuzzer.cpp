/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storagemanagerproxy_fuzzer.h"
#include "ipc/storage_manager_proxy.h"
#include <vector>

using namespace OHOS::StorageManager;

namespace OHOS {
namespace StorageManager {
bool StorageManagerProxyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    VolumeCore vc;
    Disk disk;
    MessageParcel reply;
    MessageOption option;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    const sptr<IRemoteObject> impl;
    StorageManagerProxy prePar(impl);
    std::string path((const char *)data, size);
    std::string fsUuid((const char *)data, size);
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    int32_t flags = *(reinterpret_cast<const int32_t *>(data));
    int32_t fsType = *(reinterpret_cast<const int32_t *>(data));
    std::string volumeUuid((const char *)data, size);
    std::string description((const char *)data, size);

    token.push_back(*data);
    secret.push_back(*data);
    prePar.StopUser(userId);
    prePar.Mount(volumeUuid);
    prePar.Unmount(volumeUuid);
    prePar.DeleteUserKeys(userId);
    prePar.NotifyDiskCreated(disk);
    prePar.InactiveUserKey(userId);
    prePar.UpdateKeyContext(userId);
    prePar.NotifyVolumeCreated(vc);
    prePar.RemoveUser(userId, flags);
    prePar.PrepareStartUser(userId);
    prePar.Format(volumeUuid, path);
    prePar.GetDiskById(volumeUuid, disk);
    prePar.Partition(volumeUuid, fsType);
    prePar.PrepareAddUser(userId, flags);
    prePar.GenerateUserKeys(userId, flags);
    prePar.NotifyDiskDestroyed(volumeUuid);
    prePar.NotifyVolumeDestroyed(volumeUuid);
    prePar.ActiveUserKey(userId, token, secret);
    prePar.SetVolumeDescription(fsUuid, description);
    prePar.UpdateUserAuth(userId, token, secret, secret);
    prePar.NotifyVolumeMounted(volumeUuid, fsType, fsUuid, path, description);
    return true;
}

bool StorageManagerProxyGetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }

    VolumeExternal vc1;
    const sptr<IRemoteObject> impl;
    StorageManagerProxy getStor(impl);
    std::string volumeUuid((const char *)data, size);
    std::string pkgName((const char *)data, size);
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    std::string fsUuid((const char *)data, size);
    int64_t systemSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t totalSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t freeSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t freeVolSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t totalVolSize = *(reinterpret_cast<const int64_t *>(data));
    BundleStats bundleStats;
    StorageStats storageStats;
    std::vector<VolumeExternal> vecOfVol;
    std::vector<Disk> vecOfDisk;
    getStor.GetAllVolumes(vecOfVol);
    getStor.GetAllDisks(vecOfDisk);
    getStor.GetSystemSize(systemSize);
    getStor.GetTotalSize(totalSize);
    getStor.GetFreeSize(freeSize);
    getStor.GetUserStorageStats(storageStats);
    getStor.GetBundleStats(pkgName, bundleStats);
    getStor.GetCurrentBundleStats(bundleStats);
    getStor.GetUserStorageStats(userId, storageStats);
    getStor.GetVolumeByUuid(fsUuid, vc1);
    getStor.GetVolumeById(volumeUuid, vc1);
    getStor.GetFreeSizeOfVolume(volumeUuid, freeVolSize);
    getStor.GetTotalSizeOfVolume(volumeUuid, totalVolSize);
    return true;
}

} // namespace StorageManager
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StorageManager::StorageManagerProxyFuzzTest(data, size);
    OHOS::StorageManager::StorageManagerProxyGetFuzzTest(data, size);
    return 0;
}
