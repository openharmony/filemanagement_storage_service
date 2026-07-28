/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "mock/disk_manager_client_mock.h"

namespace OHOS {
namespace DiskManager {

int32_t DiskManagerClient::Mount(const std::string &volumeId)
{
    return IDiskManagerClientMock::diskManagerClientMock->Mount(volumeId);
}

int32_t DiskManagerClient::Unmount(const std::string &volumeId)
{
    return IDiskManagerClientMock::diskManagerClientMock->Unmount(volumeId);
}

int32_t DiskManagerClient::Format(const std::string &volumeId, const std::string &fsType)
{
    return IDiskManagerClientMock::diskManagerClientMock->Format(volumeId, fsType);
}

int32_t DiskManagerClient::SetVolumeDescription(const std::string &fsUuid, const std::string &description)
{
    return IDiskManagerClientMock::diskManagerClientMock->SetVolumeDescription(fsUuid, description);
}

int32_t DiskManagerClient::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetAllVolumes(vecOfVol);
}

int32_t DiskManagerClient::GetVolumeByUuid(const std::string &uuid, VolumeExternal &vc)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetVolumeByUuid(uuid, vc);
}

int32_t DiskManagerClient::GetVolumeById(const std::string &volumeId, VolumeExternal &vc)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetVolumeById(volumeId, vc);
}

int32_t DiskManagerClient::GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetFreeSizeOfVolume(volumeUuid, freeSize);
}

int32_t DiskManagerClient::GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetTotalSizeOfVolume(volumeUuid, totalSize);
}

int32_t DiskManagerClient::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetAllDisks(vecOfDisk);
}

int32_t DiskManagerClient::GetDiskById(const std::string &diskId, Disk &disk)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetDiskById(diskId, disk);
}

int32_t DiskManagerClient::Partition(const std::string &diskId, int32_t type)
{
    return IDiskManagerClientMock::diskManagerClientMock->Partition(diskId, type);
}

} // DiskManager
} // OHOS
