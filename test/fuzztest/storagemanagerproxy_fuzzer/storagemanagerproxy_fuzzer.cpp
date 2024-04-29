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
    if ((data == nullptr) || (size <= sizeof(uint64_t))) {
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

    std::string metaData(reinterpret_cast<const char *>(data), size);
    int32_t metaData2 = *(reinterpret_cast<const int32_t *>(data));
    uint32_t metaData3 = *(reinterpret_cast<const uint32_t *>(data));
    uint64_t metaData4 = *(reinterpret_cast<const uint64_t *>(data));
    token.push_back(*data);
    secret.push_back(*data);
    proxy->StopUser(metaData2);
    proxy->Mount(metaData);
    proxy->Unmount(metaData);
    proxy->DeleteUserKeys(metaData3);
    proxy->NotifyDiskCreated(disk);
    proxy->InactiveUserKey(metaData3);
    proxy->UpdateKeyContext(metaData3);
    proxy->NotifyVolumeCreated(vc);
    proxy->RemoveUser(metaData2, metaData3);
    proxy->PrepareStartUser(metaData2);
    proxy->Format(metaData, metaData);
    proxy->GetDiskById(metaData, disk);
    proxy->Partition(metaData, metaData2);
    proxy->PrepareAddUser(metaData2, metaData3);
    proxy->GenerateUserKeys(metaData2, metaData3);
    proxy->NotifyDiskDestroyed(metaData);
    proxy->ActiveUserKey(metaData2, token, secret);
    proxy->SetVolumeDescription(metaData, metaData);
    proxy->UpdateUserAuth(metaData2, metaData4, token, secret, secret);
    proxy->NotifyVolumeMounted(metaData, metaData2, metaData, metaData, metaData);
    return true;
}

bool StorageManagerProxyGetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(int64_t))) {
        return false;
    }
    auto impl = new StorageManagerProxyMock();
    auto proxy = std::make_shared<StorageManagerProxy>(impl);
    if (proxy == nullptr || impl == nullptr) {
        return 0;
    }
    VolumeExternal vc1;
    std::string metaData(reinterpret_cast<const char *>(data), size);
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    int64_t metaData4 = *(reinterpret_cast<const int64_t *>(data));

    BundleStats bundleStats;
    StorageStats storageStats;
    std::vector<VolumeExternal> vecOfVol;
    std::vector<Disk> vecOfDisk;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    std::vector<std::string> bundleName;
    incrementalBackTimes.push_back(metaData4);
    pkgFileSizes.push_back(metaData4);
    bundleName.push_back(metaData);
    proxy->GetAllVolumes(vecOfVol);
    proxy->GetAllDisks(vecOfDisk);
    proxy->GetSystemSize(metaData4);
    proxy->GetTotalSize(metaData4);
    proxy->GetFreeSize(metaData4);
    proxy->GetUserStorageStats(storageStats);
    proxy->GetBundleStats(metaData, bundleStats);
    proxy->GetCurrentBundleStats(bundleStats);
    proxy->GetUserStorageStats(userId, storageStats);
    proxy->GetUserStorageStatsByType(userId, storageStats, metaData);
    proxy->GetVolumeByUuid(metaData, vc1);
    proxy->GetVolumeById(metaData, vc1);
    proxy->GetFreeSizeOfVolume(metaData, metaData4);
    proxy->GetTotalSizeOfVolume(metaData, metaData4);
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
