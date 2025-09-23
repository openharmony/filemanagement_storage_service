/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
    size_t dataMinSize = sizeof(int32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(bool);
    if ((data == nullptr) || (size <= dataMinSize)) {
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

    int32_t metaData2 = *(reinterpret_cast<const int32_t *>(data));
    uint32_t metaData3 = *(reinterpret_cast<const uint32_t *>(data + sizeof(uint32_t)));
    uint64_t metaData4 = *(reinterpret_cast<const uint64_t *>(data + sizeof(uint32_t) + sizeof(uint64_t)));
    bool metadata6 = *(reinterpret_cast<const bool *>(data + sizeof(uint32_t) + sizeof(uint64_t) +
        sizeof(bool)));
    std::string metaData(reinterpret_cast<const char *>(data + dataMinSize), size - dataMinSize);
    std::map<std::string, std::string> metaData5 = {{metaData, metaData}};
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
    proxy->NotifyDiskDestroyed(metaData);
    proxy->ActiveUserKey(metaData2, token, secret);
    proxy->SetVolumeDescription(metaData, metaData);
    proxy->UpdateUserAuth(metaData2, metaData4, token, secret, secret);
    proxy->NotifyVolumeMounted(metaData, metaData, metaData, metaData, metaData, metadata6);
    proxy->MountDisShareFile(metaData2, metaData5);
    proxy->UMountDisShareFile(metaData2, metaData);
    return true;
}

bool StorageManagerProxyGetFuzzTest(const uint8_t *data, size_t size)
{
    size_t dataMinSize = sizeof(int32_t) + sizeof(uint32_t) + sizeof(uint64_t);
    if ((data == nullptr) || (size <= dataMinSize)) {
        return false;
    }
    auto impl = new StorageManagerProxyMock();
    auto proxy = std::make_shared<StorageManagerProxy>(impl);
    if (proxy == nullptr || impl == nullptr) {
        return 0;
    }
    VolumeExternal vc1;
    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    int64_t metaData4 = *(reinterpret_cast<const int64_t *>(data + sizeof(int32_t)));
    std::string metaData(reinterpret_cast<const char *>(data + dataMinSize), size - dataMinSize);

    BundleStats bundleStats;
    StorageStats storageStats;
    std::vector<VolumeExternal> vecOfVol;
    std::vector<Disk> vecOfDisk;
    std::vector<int64_t> incrementalBackTimes;
    std::vector<int64_t> pkgFileSizes;
    std::vector<int64_t> incPkgFileSizes;
    std::vector<std::string> bundleName;
    incrementalBackTimes.push_back(metaData4);
    pkgFileSizes.push_back(metaData4);
    incPkgFileSizes.push_back(metaData4);
    bundleName.push_back(metaData);
    proxy->GetAllVolumes(vecOfVol);
    proxy->GetAllDisks(vecOfDisk);
    proxy->GetSystemSize(metaData4);
    proxy->GetTotalSize(metaData4);
    proxy->GetFreeSize(metaData4);
    proxy->GetUserStorageStats(storageStats);
    proxy->GetBundleStats(metaData, bundleStats, 0, 0);
    proxy->GetCurrentBundleStats(bundleStats, 0);
    proxy->GetUserStorageStats(userId, storageStats);
    proxy->GetUserStorageStatsByType(userId, storageStats, metaData);
    proxy->GetVolumeByUuid(metaData, vc1);
    proxy->GetVolumeById(metaData, vc1);
    proxy->GetFreeSizeOfVolume(metaData, metaData4);
    proxy->GetTotalSizeOfVolume(metaData, metaData4);
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
