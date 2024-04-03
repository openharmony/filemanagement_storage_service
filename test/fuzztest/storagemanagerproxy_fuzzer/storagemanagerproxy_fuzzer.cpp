/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "storage_manager_proxy.h"
#include "storagemanagerproxymock.h"
#include <vector>
#include <memory>

using namespace OHOS::StorageManager;

namespace OHOS {
namespace StorageManager {
bool StorageManagerProxyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    auto impl = new StorageManagerProxyMock();
    auto proxy = std::make_shared<StorageManagerProxy>(impl);
    if (proxy == nullptr || impl == nullptr) {
        return 0;
    }
    
    VolumeCore vc;
    Disk disk;
    MessageParcel reply;
    MessageOption option;
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;

    std::string path(reinterpret_cast<const char *>(data), size);
    std::string fsUuid(reinterpret_cast<const char *>(data), size);
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    int32_t flags = *(reinterpret_cast<const int32_t *>(data));
    int32_t fsType = *(reinterpret_cast<const int32_t *>(data));
    uint64_t secureUid = *(reinterpret_cast<const uint64_t *>(data));
    std::string volumeUuid(reinterpret_cast<const char *>(data), size);
    std::string description(reinterpret_cast<const char *>(data), size);
    token.push_back(*data);
    secret.push_back(*data);
    proxy->StopUser(userId);
    proxy->Mount(volumeUuid);
    proxy->Unmount(volumeUuid);
    proxy->DeleteUserKeys(userId);
    proxy->NotifyDiskCreated(disk);
    proxy->InactiveUserKey(userId);
    proxy->UpdateKeyContext(userId);
    proxy->NotifyVolumeCreated(vc);
    proxy->RemoveUser(userId, flags);
    proxy->PrepareStartUser(userId);
    proxy->Format(volumeUuid, path);
    proxy->GetDiskById(volumeUuid, disk);
    proxy->Partition(volumeUuid, fsType);
    proxy->PrepareAddUser(userId, flags);
    proxy->GenerateUserKeys(userId, flags);
    proxy->NotifyDiskDestroyed(volumeUuid);
    proxy->ActiveUserKey(userId, token, secret);
    proxy->SetVolumeDescription(fsUuid, description);
    proxy->UpdateUserAuth(userId, secureUid, token, secret, secret);
    proxy->NotifyVolumeMounted(volumeUuid, fsType, fsUuid, path, description);
    return true;
}

bool StorageManagerProxyGetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    auto impl = new StorageManagerProxyMock();
    auto proxy = std::make_shared<StorageManagerProxy>(impl);
    if (proxy == nullptr || impl == nullptr) {
        return 0;
    }
    VolumeExternal vc1;
    std::string volumeUuid(reinterpret_cast<const char *>(data), size);
    std::string pkgName(reinterpret_cast<const char *>(data), size);
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    std::string fsUuid(reinterpret_cast<const char *>(data), size);
    int64_t systemSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t totalSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t freeSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t freeVolSize = *(reinterpret_cast<const int64_t *>(data));
    int64_t totalVolSize = *(reinterpret_cast<const int64_t *>(data));
    std::string type(reinterpret_cast<const char *>(data), size);
    BundleStats bundleStats;
    StorageStats storageStats;
    std::vector<VolumeExternal> vecOfVol;
    std::vector<Disk> vecOfDisk;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    std::vector<std::string> bundleName;
    incrementalBackTimes.push_back(*data);
    pkgFileSizes.push_back(*data);
    bundleName.push_back(reinterpret_cast<const char *>(data));
    proxy->GetAllVolumes(vecOfVol);
    proxy->GetAllDisks(vecOfDisk);
    proxy->GetSystemSize(systemSize);
    proxy->GetTotalSize(totalSize);
    proxy->GetFreeSize(freeSize);
    proxy->GetUserStorageStats(storageStats);
    proxy->GetBundleStats(pkgName, bundleStats);
    proxy->GetCurrentBundleStats(bundleStats);
    proxy->GetUserStorageStats(userId, storageStats);
    proxy->GetUserStorageStatsByType(userId, storageStats, type);
    proxy->GetVolumeByUuid(fsUuid, vc1);
    proxy->GetVolumeById(volumeUuid, vc1);
    proxy->GetFreeSizeOfVolume(volumeUuid, freeVolSize);
    proxy->GetTotalSizeOfVolume(volumeUuid, totalVolSize);
    proxy->GetBundleStatsForIncrease(userId, bundleName, incrementalBackTimes, pkgFileSizes);
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
