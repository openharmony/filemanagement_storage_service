/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ohos.file.volumeManager.impl.h"
#include "storageStatistics_taihe_error.h"
#include "storage_service_log.h"
#include "burn_params.h"

namespace ANI::VolumeManager {

ohos::file::volumeManager::Volume GetVolumeByUuidSync(taihe::string_view uuid)
{
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return { "", "", "", "", true, 0, "", "", "" };
    }
    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    int32_t errNum = instance->GetVolumeByUuid(uuid.c_str(), *volumeInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { "", "", "", "", true, 0, "", "", "" };
    }
    return { volumeInfo->GetId(), volumeInfo->GetUuid(), volumeInfo->GetDiskId(), volumeInfo->GetDescription(),
        true, volumeInfo->GetState(), volumeInfo->GetPath(), volumeInfo->GetFsTypeString(),
        volumeInfo->GetExtraInfo() };
}

taihe::array<ohos::file::volumeManager::Volume> GetAllVolumesSync()
{
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return taihe::array<ohos::file::volumeManager::Volume>::make(0, ohos::file::volumeManager::Volume{});
    }
    auto volumeInfo = std::make_shared<std::vector<OHOS::StorageManager::VolumeExternal>>();
    int32_t errNum = instance->GetAllVolumes(*volumeInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return taihe::array<ohos::file::volumeManager::Volume>::make(0, ohos::file::volumeManager::Volume{});
    }
    auto result = taihe::array<ohos::file::volumeManager::Volume>::
        make(volumeInfo->size(), ohos::file::volumeManager::Volume{});
    auto volumeTransformer = [](auto &vol) -> ohos::file::volumeManager::Volume {
        return {vol.GetId(), vol.GetUuid(), vol.GetDiskId(), vol.GetDescription(), true, vol.GetState(),
            vol.GetPath(), vol.GetFsTypeString(), vol.GetExtraInfo()};
    };
    std::transform(volumeInfo->begin(), volumeInfo->end(), result.begin(), volumeTransformer);
    return taihe::array<ohos::file::volumeManager::Volume>(taihe::copy_data_t{}, result.data(), result.size());
}

void FormatSync(::taihe::string_view volumeId, ::taihe::string_view fsType)
{
    std::string volumeIdString = std::string(volumeId);
    std::string fsTypeString = std::string(fsType);
    if (volumeIdString.empty() || fsTypeString.empty()) {
        LOGE("Invalid parameter, volumeId or fsType is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t result = instance->Format(volumeIdString, fsTypeString);
    if (result != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(result);
        return;
    }
}

void GetVolumeByIdSync(::taihe::string_view volumeId)
{
    std::string volumeIdString = std::string(volumeId);
    if (volumeIdString.empty()) {
        LOGE("Invalid volumeId parameter, volumeId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->GetVolumeById(volumeIdString, *volumeInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void MountSync(::taihe::string_view volumeId)
{
    std::string volumeIdString = std::string(volumeId);
    if (volumeIdString.empty()) {
        LOGE("Invalid volumeId parameter, volumeId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }

    int32_t errNum = instance->Mount(volumeIdString);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void UnmountSync(::taihe::string_view volumeId)
{
    std::string volumeIdString = std::string(volumeId);
    if (volumeIdString.empty()) {
        LOGE("Invalid volumeId parameter, volumeId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->Unmount(volumeIdString);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void PartitionSync(::taihe::string_view diskId, int32_t type)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->Partition(diskIdString, type);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void SetVolumeDescriptionSync(::taihe::string_view uuid, ::taihe::string_view description)
{
    std::string uuidString = std::string(uuid);
    std::string descStr = std::string(description);
    if (uuidString.empty() || descStr.empty()) {
        LOGE("Invalid parameter, uuid or description is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->SetVolumeDescription(uuidString, descStr);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void EjectSync(::taihe::string_view volumeId)
{
    std::string volumeIdStr = std::string(volumeId);
    if (volumeIdStr.empty()) {
        LOGE("Invalid parameter, volumeIdStr is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }

    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->Eject(volumeIdStr);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void GetOpticalDriveOpsProgressSync(::taihe::string_view volumeId)
{
    std::string volumeIdStr = std::string(volumeId);
    if (volumeIdStr.empty()) {
        LOGE("Invalid parameter, volumeIdStr is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }

    auto progress = std::make_shared<uint32_t>(0);
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->GetOpticalDriveOpsProgress(volumeIdStr, *progress);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void EraseSync(::taihe::string_view volumeId)
{
    std::string volumeIdStr = std::string(volumeId);
    if (volumeIdStr.empty()) {
        LOGE("Invalid parameter, volumeIdStr is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->Erase(volumeIdStr);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void CreateIsoImageSync(::taihe::string_view volumeId, ::taihe::string_view filePath)
{
    std::string volumeIdStr = std::string(volumeId);
    if (volumeIdStr.empty()) {
        LOGE("Invalid parameter, volumeIdStr is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    std::string filePathStr = std::string(filePath);
    if (filePathStr.empty()) {
        LOGE("Invalid parameter, filePathStr is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->CreateIsoImage(volumeIdStr, filePathStr);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void BurnSync(::taihe::string_view volumeId, const ohos::file::volumeManager::BurnParams &burnParams)
{
    std::string volumeIdStr(volumeId);
    std::string diskNameStr(burnParams.diskName.data(), burnParams.diskName.size());
    std::string burnPathStr(burnParams.burnPath.data(), burnParams.burnPath.size());
    std::string fsTypeStr(burnParams.fsType.data(), burnParams.fsType.size());

    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    OHOS::StorageManager::BurnParams params;
    params.diskName = diskNameStr;
    params.burnPath = burnPathStr;
    params.fsType = fsTypeStr;
    params.burnSpeed = burnParams.burnSpeed;
    params.isIsoImage = burnParams.isIsoImage;
    params.isIncBurnSupport = burnParams.isIncBurnSupport;
    int32_t errNum = instance->Burn(volumeIdStr, params);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void VerifyBurnDataSync(::taihe::string_view volumeId, uint32_t verType)
{
    std::string volumeIdStr = std::string(volumeId);
    if (volumeIdStr.empty()) {
        LOGE("Invalid parameter, volumeIdStr is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->VerifyBurnData(volumeIdStr, verType);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

taihe::array<ohos::file::volumeManager::Disk> GetAllDisksSync()
{
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return taihe::array<ohos::file::volumeManager::Disk>::make(0, ohos::file::volumeManager::Disk{});
    }
    auto diskInfo = std::make_shared<std::vector<OHOS::StorageManager::Disk>>();
    int32_t errNum = instance->GetAllDisks(*diskInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return taihe::array<ohos::file::volumeManager::Disk>::make(0, ohos::file::volumeManager::Disk{});
    }
    auto result = taihe::array<ohos::file::volumeManager::Disk>::make(diskInfo->size(),
        ohos::file::volumeManager::Disk{});
    auto diskTransformer = [](auto &disk) -> ohos::file::volumeManager::Disk {
        return {disk.GetDiskId(), disk.GetDiskName(), disk.GetSizeBytes(), disk.GetSysPath(), disk.GetVendor(),
            disk.GetDiskType(), disk.GetMediaType(), disk.GetRemovable(), disk.GetExtraInfo()};
    };
    std::transform(diskInfo->begin(), diskInfo->end(), result.begin(), diskTransformer);
    return taihe::array<ohos::file::volumeManager::Disk>(taihe::copy_data_t{}, result.data(), result.size());
}

ohos::file::volumeManager::Disk GetDiskByIdSync(taihe::string_view diskId)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return { "", "", 0, "", "", 0, 0, 0, "" };
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return { "", "", 0, "", "", 0, 0, 0, "" };
    }
    auto diskInfo = std::make_shared<OHOS::StorageManager::Disk>();
    int32_t errNum = instance->GetDiskById(diskIdString, *diskInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { "", "", 0, "", "", 0, 0, 0, "" };
    }
    return { diskInfo->GetDiskId(), diskInfo->GetDiskName(), diskInfo->GetSizeBytes(), diskInfo->GetSysPath(),
        diskInfo->GetVendor(), diskInfo->GetDiskType(), diskInfo->GetMediaType(), diskInfo->GetRemovable(),
        diskInfo->GetExtraInfo() };
}

ohos::file::volumeManager::PartitionTableInfo GetPartitionTableSync(taihe::string_view diskId)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return { "", "", 0, 0, 0, 0, taihe::array<ohos::file::volumeManager::PartitionInfo>::make(0,
            ohos::file::volumeManager::PartitionInfo{}) };
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return { "", "", 0, 0, 0, 0, taihe::array<ohos::file::volumeManager::PartitionInfo>::make(0,
            ohos::file::volumeManager::PartitionInfo{}) };
    }
    auto partitionTableInfo = std::make_shared<OHOS::StorageManager::PartitionTableInfo>();
    int32_t errNum = instance->GetPartitionTable(diskIdString, *partitionTableInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { "", "", 0, 0, 0, 0, taihe::array<ohos::file::volumeManager::PartitionInfo>::make(0,
            ohos::file::volumeManager::PartitionInfo{}) };
    }
    auto partitions = partitionTableInfo->GetPartitions();
    auto partitionsResult = taihe::array<ohos::file::volumeManager::PartitionInfo>::make(partitions.size(),
        ohos::file::volumeManager::PartitionInfo{});
    auto partitionTransformer = [](auto &part) -> ohos::file::volumeManager::PartitionInfo {
        return {part.GetPartitionNum(), part.GetDiskId(), part.GetStartSector(), part.GetEndSector(),
            part.GetSizeBytes(), part.GetFsType()};
    };
    std::transform(partitions.begin(), partitions.end(), partitionsResult.begin(), partitionTransformer);
    auto partitionsArray = taihe::array<ohos::file::volumeManager::PartitionInfo>(taihe::copy_data_t{},
        partitionsResult.data(), partitionsResult.size());
    return { partitionTableInfo->GetDiskId(), partitionTableInfo->GetTableType(),
             partitionTableInfo->GetPartitionCount(), partitionTableInfo->GetTotalSector(),
             partitionTableInfo->GetSectorSize(), partitionTableInfo->GetAlignSector(),
        partitionsArray };
}

void CreatePartitionSync(taihe::string_view diskId, ohos::file::volumeManager::PartitionOptions options)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    OHOS::StorageManager::PartitionOptions partitionOptions;
    partitionOptions.SetPartitionNum(options.partitionNum);
    partitionOptions.SetStartSector(options.startSector);
    partitionOptions.SetEndSector(options.endSector);
    std::string typeCodeStr(options.typeCode);
    partitionOptions.SetTypeCode(typeCodeStr);
    int32_t errNum = instance->CreatePartition(diskIdString, partitionOptions);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void DeletePartitionSync(taihe::string_view diskId, uint32_t partitionNum)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    int32_t errNum = instance->DeletePartition(diskIdString, partitionNum);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void FormatPartitionSync(taihe::string_view diskId, uint32_t partitionNum,
                         ohos::file::volumeManager::FormatOptions options)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    OHOS::StorageManager::FormatOptions formatOptions;
    std::string fsTypeStr(options.fsType);
    formatOptions.SetFsType(fsTypeStr);
    formatOptions.SetQuickFormat(options.quickFormat);
    std::string volumeNameStr(options.volumeName);
    formatOptions.SetVolumeName(volumeNameStr);
    int32_t errNum = instance->FormatPartition(diskIdString, partitionNum, formatOptions);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}
} // namespace ANI::VolumeManager

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetVolumeByUuidSync(ANI::VolumeManager::GetVolumeByUuidSync);
TH_EXPORT_CPP_API_GetAllVolumesSync(ANI::VolumeManager::GetAllVolumesSync);
TH_EXPORT_CPP_API_FormatSync(ANI::VolumeManager::FormatSync);
TH_EXPORT_CPP_API_GetVolumeByIdSync(ANI::VolumeManager::GetVolumeByIdSync);
TH_EXPORT_CPP_API_MountSync(ANI::VolumeManager::MountSync);
TH_EXPORT_CPP_API_UnmountSync(ANI::VolumeManager::UnmountSync);
TH_EXPORT_CPP_API_PartitionSync(ANI::VolumeManager::PartitionSync);
TH_EXPORT_CPP_API_SetVolumeDescriptionSync(ANI::VolumeManager::SetVolumeDescriptionSync);
TH_EXPORT_CPP_API_EjectSync(ANI::VolumeManager::EjectSync);
TH_EXPORT_CPP_API_GetOpticalDriveOpsProgressSync(ANI::VolumeManager::GetOpticalDriveOpsProgressSync);
TH_EXPORT_CPP_API_EraseSync(ANI::VolumeManager::EraseSync);
TH_EXPORT_CPP_API_CreateIsoImageSync(ANI::VolumeManager::CreateIsoImageSync);
TH_EXPORT_CPP_API_BurnSync(ANI::VolumeManager::BurnSync);
TH_EXPORT_CPP_API_VerifyBurnDataSync(ANI::VolumeManager::VerifyBurnDataSync);
TH_EXPORT_CPP_API_GetAllDisksSync(ANI::VolumeManager::GetAllDisksSync);
TH_EXPORT_CPP_API_GetDiskByIdSync(ANI::VolumeManager::GetDiskByIdSync);
TH_EXPORT_CPP_API_GetPartitionTableSync(ANI::VolumeManager::GetPartitionTableSync);
TH_EXPORT_CPP_API_CreatePartitionSync(ANI::VolumeManager::CreatePartitionSync);
TH_EXPORT_CPP_API_DeletePartitionSync(ANI::VolumeManager::DeletePartitionSync);
TH_EXPORT_CPP_API_FormatPartitionSync(ANI::VolumeManager::FormatPartitionSync);
// NOLINTEND
