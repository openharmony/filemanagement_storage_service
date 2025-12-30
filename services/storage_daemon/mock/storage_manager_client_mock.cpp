/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock/storage_manager_client_mock.h"

namespace OHOS {
namespace StorageDaemon {
int32_t StorageManagerClient::GetClient()
{
    return 0;
}

int32_t StorageManagerClient::NotifyDiskCreated(DiskInfo &diskInfo)
{
    return 0;
}

int32_t StorageManagerClient::NotifyDiskDestroyed(std::string id)
{
    return 0;
}

int32_t StorageManagerClient::NotifyVolumeCreated(std::shared_ptr<VolumeInfo> info)
{
    if (IStorageManagerClientMock::iStorageManagerClientMock_ == nullptr) {
        return -1;
    }
    return IStorageManagerClientMock::iStorageManagerClientMock_->NotifyVolumeCreated(info);
}

int32_t StorageManagerClient::NotifyVolumeMounted(std::shared_ptr<VolumeInfo> volumeInfo)
{
    return 0;
}

int32_t StorageManagerClient::NotifyVolumeStateChanged(std::string volId, StorageManager::VolumeState state)
{
    if (IStorageManagerClientMock::iStorageManagerClientMock_ == nullptr) {
        return -1;
    }
    return IStorageManagerClientMock::iStorageManagerClientMock_->NotifyVolumeStateChanged(volId, state);
}

int32_t StorageManagerClient::NotifyVolumeDamaged(std::shared_ptr<VolumeInfo> volumeInfo)
{
    return 0;
}

int32_t StorageManagerClient::NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                               const std::string &uuid)
{
    return 0;
}

int32_t StorageManagerClient::NotifyMtpUnmounted(const std::string &id, const std::string &path, const bool isBadRemove)
{
    return 0;
}

int32_t StorageManagerClient::IsUsbFuseByType(const std::string &fsType, bool &enabled)
{
    return 0;
}

int32_t StorageManagerClient::NotifyCreateBundleDataDirWithEl(uint32_t userId, uint8_t elx)
{
    return 0;
}
} // StorageDaemon
} // OHOS
