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
    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.GetVolumeByUuid(uuid.c_str(), *volumeInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { "", "", "", "", true, 0, "", "", "", 0 };
    }
    return { volumeInfo->GetId(), volumeInfo->GetUuid(), volumeInfo->GetDiskId(), volumeInfo->GetDescription(),
        true, volumeInfo->GetState(), volumeInfo->GetPath(), volumeInfo->GetFsTypeString(),
        volumeInfo->GetExtraInfo(), volumeInfo->GetPartitionNum() };
}

taihe::array<ohos::file::volumeManager::Volume> GetAllVolumesSync()
{
    auto volumeInfo = std::make_shared<std::vector<OHOS::StorageManager::VolumeExternal>>();
    int32_t errNum = OHOS::StorageManager::StorageManagerConnect::GetInstance().GetAllVolumes(*volumeInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return taihe::array<ohos::file::volumeManager::Volume>::make(0, ohos::file::volumeManager::Volume{});
    }
    auto result = taihe::array<ohos::file::volumeManager::Volume>::
        make(volumeInfo->size(), ohos::file::volumeManager::Volume{});
    auto volumeTransformer = [](auto &vol) -> ohos::file::volumeManager::Volume {
        return {vol.GetId(), vol.GetUuid(), vol.GetDiskId(), vol.GetDescription(), true, vol.GetState(),
            vol.GetPath(), vol.GetFsTypeString(), vol.GetExtraInfo(), vol.GetPartitionNum()};
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
    int32_t result = OHOS::StorageManager::StorageManagerConnect::GetInstance().Format(volumeIdString, fsTypeString);
    if (result != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(result);
        return;
    }
}

ohos::file::volumeManager::Volume GetVolumeByIdSync(::taihe::string_view volumeId)
{
    std::string volumeIdString = std::string(volumeId);
    if (volumeIdString.empty()) {
        LOGE("Invalid volumeId parameter, volumeId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return { "", "", "", "", true, 0, "", "", "", 0 };
    }
    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.GetVolumeById(volumeIdString, *volumeInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { "", "", "", "", true, 0, "", "", "", 0 };
    }
    return { volumeInfo->GetId(), volumeInfo->GetUuid(), volumeInfo->GetDiskId(), volumeInfo->GetDescription(),
        true, volumeInfo->GetState(), volumeInfo->GetPath(), volumeInfo->GetFsTypeString(),
        volumeInfo->GetExtraInfo(), volumeInfo->GetPartitionNum() };
}

void MountSync(::taihe::string_view volumeId)
{
    std::string volumeIdString = std::string(volumeId);
    if (volumeIdString.empty()) {
        LOGE("Invalid volumeId parameter, volumeId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    int32_t errNum = OHOS::StorageManager::StorageManagerConnect::GetInstance().Mount(volumeIdString);
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
    int32_t errNum = OHOS::StorageManager::StorageManagerConnect::GetInstance().Unmount(volumeIdString);
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
    int32_t errNum = OHOS::StorageManager::StorageManagerConnect::GetInstance().Partition(diskIdString, type);
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
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.SetVolumeDescription(uuidString, descStr);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

taihe::array<ohos::file::volumeManager::Disk> GetAllDisksSync()
{
    auto diskInfo = std::make_shared<std::vector<OHOS::StorageManager::Disk>>();
    int32_t errNum = OHOS::StorageManager::StorageManagerConnect::GetInstance().GetAllDisks(*diskInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return taihe::array<ohos::file::volumeManager::Disk>::make(0, ohos::file::volumeManager::Disk{});
    }
    auto result = taihe::array<ohos::file::volumeManager::Disk>::make(diskInfo->size(),
        ohos::file::volumeManager::Disk{});
    auto diskTransformer = [](auto &disk) -> ohos::file::volumeManager::Disk {
        auto volumeIds = disk.GetVolumeIds();
        auto volumeIdsArray = taihe::array<taihe::string>::make(volumeIds.size(), taihe::string{});
        size_t idx = 0;
        for (const auto &id : volumeIds) {
            volumeIdsArray[idx] = taihe::string(id);
            idx++;
        }
        return {disk.GetDiskId(), disk.GetSizeBytes(), disk.GetDiskType(), disk.GetRemovable(),
            taihe::array<taihe::string>(taihe::copy_data_t{}, volumeIdsArray.data(), volumeIdsArray.size()),
            disk.GetExtraInfo()};
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
        return { "", 0, 0, false, taihe::array<taihe::string>::make(0, taihe::string{}), "" };
    }
    auto diskInfo = std::make_shared<OHOS::StorageManager::Disk>();
    int32_t errNum = OHOS::StorageManager::StorageManagerConnect::GetInstance().GetDiskById(diskIdString, *diskInfo);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return { "", 0, 0, false, taihe::array<taihe::string>::make(0, taihe::string{}), "" };
    }
    auto volumeIds = diskInfo->GetVolumeIds();
    auto volumeIdsArray = taihe::array<taihe::string>::make(volumeIds.size(), taihe::string{});
    size_t idx = 0;
    for (const auto &id : volumeIds) {
        volumeIdsArray[idx] = taihe::string(id);
        idx++;
    }
    return { diskInfo->GetDiskId(), diskInfo->GetSizeBytes(), diskInfo->GetDiskType(), diskInfo->GetRemovable(),
        taihe::array<taihe::string>(taihe::copy_data_t{}, volumeIdsArray.data(), volumeIdsArray.size()),
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
    auto partitionTableInfo = std::make_shared<OHOS::StorageManager::PartitionTableInfo>();
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.GetPartitionTable(diskIdString, *partitionTableInfo);
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

void CreatePartitionSync(taihe::string_view diskId, ohos::file::volumeManager::PartitionParams partitionParams)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    OHOS::StorageManager::PartitionParams partitionParamsNative;
    partitionParamsNative.SetPartitionNum(partitionParams.partitionNum);
    partitionParamsNative.SetStartSector(partitionParams.startSector);
    partitionParamsNative.SetEndSector(partitionParams.endSector);
    std::string typeCodeStr(partitionParams.typeCode);
    partitionParamsNative.SetTypeCode(typeCodeStr);
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.CreatePartition(diskIdString, partitionParamsNative);
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
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.DeletePartition(diskIdString, partitionNum);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}

void FormatPartitionSync(taihe::string_view diskId, uint32_t partitionNum,
                         ohos::file::volumeManager::FormatParams formatParams)
{
    std::string diskIdString = std::string(diskId);
    if (diskIdString.empty()) {
        LOGE("Invalid parameter, diskId is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PARAMS);
        return;
    }
    OHOS::StorageManager::FormatParams formatParamsNative;
    std::string fsTypeStr(formatParams.fsType);
    formatParamsNative.SetFsType(fsTypeStr);
    formatParamsNative.SetQuickFormat(formatParams.quickFormat);
    std::string volumeNameStr(formatParams.volumeName);
    formatParamsNative.SetVolumeName(volumeNameStr);
    auto& instance = OHOS::StorageManager::StorageManagerConnect::GetInstance();
    int32_t errNum = instance.FormatPartition(diskIdString, partitionNum, formatParamsNative);
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
TH_EXPORT_CPP_API_GetAllDisksSync(ANI::VolumeManager::GetAllDisksSync);
TH_EXPORT_CPP_API_GetDiskByIdSync(ANI::VolumeManager::GetDiskByIdSync);
TH_EXPORT_CPP_API_GetPartitionTableSync(ANI::VolumeManager::GetPartitionTableSync);
TH_EXPORT_CPP_API_CreatePartitionSync(ANI::VolumeManager::CreatePartitionSync);
TH_EXPORT_CPP_API_DeletePartitionSync(ANI::VolumeManager::DeletePartitionSync);
TH_EXPORT_CPP_API_FormatPartitionSync(ANI::VolumeManager::FormatPartitionSync);
// NOLINTEND
