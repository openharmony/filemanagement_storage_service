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

int32_t DiskManagerClient::GetPartitionTable(const std::string &diskId, PartitionTableInfo &out)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetPartitionTable(diskId, out);
}

int32_t DiskManagerClient::CreatePartition(const std::string &diskId, const PartitionParams &params)
{
    return IDiskManagerClientMock::diskManagerClientMock->CreatePartition(diskId, params);
}

int32_t DiskManagerClient::DeletePartition(const std::string &diskId, int32_t partitionNum)
{
    return IDiskManagerClientMock::diskManagerClientMock->DeletePartition(diskId, partitionNum);
}

int32_t DiskManagerClient::FormatPartition(const std::string &diskId, int32_t partitionNum,
    const FormatParams &params)
{
    return IDiskManagerClientMock::diskManagerClientMock->FormatPartition(diskId, partitionNum, params);
}

int32_t DiskManagerClient::EraseVolume(const std::string &volumeId)
{
    return IDiskManagerClientMock::diskManagerClientMock->EraseVolume(volumeId);
}

int32_t DiskManagerClient::EjectVolume(const std::string &volumeId)
{
    return IDiskManagerClientMock::diskManagerClientMock->EjectVolume(volumeId);
}

int32_t DiskManagerClient::CreateIsoImage(const std::string &volumeId, const std::string &filePath)
{
    return IDiskManagerClientMock::diskManagerClientMock->CreateIsoImage(volumeId, filePath);
}

int32_t DiskManagerClient::BurnVolume(const std::string &volumeId, const std::string &burnOptions)
{
    return IDiskManagerClientMock::diskManagerClientMock->BurnVolume(volumeId, burnOptions);
}

int32_t DiskManagerClient::GetVolumeOpProcess(const std::string &volumeId, int32_t &progressPct)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetVolumeOpProcess(volumeId, progressPct);
}

int32_t DiskManagerClient::VerifyBurnData(const std::string &volumeId, int32_t verifyType)
{
    return IDiskManagerClientMock::diskManagerClientMock->VerifyBurnData(volumeId, verifyType);
}

int32_t DiskManagerClient::ResetProxy()
{
    return IDiskManagerClientMock::diskManagerClientMock->ResetProxy();
}

int32_t DiskManagerClient::NotifyMtpMounted(const std::string &id, const std::string &path,
    const std::string &desc, const std::string &uuid, const std::string &fsType)
{
    return IDiskManagerClientMock::diskManagerClientMock->NotifyMtpMounted(id, path, desc, uuid, fsType);
}

int32_t DiskManagerClient::NotifyMtpUnmounted(const std::string &id, const bool isBadRemove)
{
    return IDiskManagerClientMock::diskManagerClientMock->NotifyMtpUnmounted(id, isBadRemove);
}

int32_t DiskManagerClient::OnBlockDiskUevent(const std::string &rawUeventMsg)
{
    return IDiskManagerClientMock::diskManagerClientMock->OnBlockDiskUevent(rawUeventMsg);
}

} // DiskManager
} // OHOS
