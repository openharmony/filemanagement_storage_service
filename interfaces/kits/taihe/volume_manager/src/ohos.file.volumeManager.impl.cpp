/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

namespace ANI::VolumeManager {

ohos::file::volumeManager::Volume GetVolumeByUuidSync(taihe::string_view uuid)
{
    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
        return { "", "", "", "", true, 0, "", "" };
    }

    int32_t errNum = instance->GetVolumeByUuid(uuid.c_str(), *volumeInfo);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "GetVolumeByUuid failed");
        return { "", "", "", "", true, 0, "", "" };
    }

    return { volumeInfo->GetId(), volumeInfo->GetUuid(), volumeInfo->GetDiskId(),
        volumeInfo->GetDescription(), true, volumeInfo->GetState(),
        volumeInfo->GetPath(), volumeInfo->GetFsTypeString() };
}

taihe::array<ohos::file::volumeManager::Volume> GetAllVolumesSync()
{
    auto volumeInfo = std::make_shared<std::vector<OHOS::StorageManager::VolumeExternal>>();

    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("Get StorageManagerConnect instacne failed");
        return taihe::array<ohos::file::volumeManager::Volume>::make(0, ohos::file::volumeManager::Volume{});
    }

    int32_t errNum = instance->GetAllVolumes(*volumeInfo);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "GetAllVolumes failed");
        return taihe::array<ohos::file::volumeManager::Volume>::make(0, ohos::file::volumeManager::Volume{});
    }

    auto result = taihe::array<ohos::file::volumeManager::Volume>::
        make(volumeInfo->size(), ohos::file::volumeManager::Volume{});
    auto volumeTransformer = [](auto &vol) -> ohos::file::volumeManager::Volume {
        return {vol.GetId(), vol.GetUuid(), vol.GetDiskId(), vol.GetDescription(), true, vol.GetState(),
            vol.GetPath(), vol.GetFsTypeString()};
    };
    std::transform(volumeInfo->begin(), volumeInfo->end(), result.begin(), volumeTransformer);

    return taihe::array<ohos::file::volumeManager::Volume>(taihe::copy_data_t{}, result.data(), result.size());
}

void FormatSync(::taihe::string_view volumeId, ::taihe::string_view fsType)
{
    std::string volumeIdString = std::string(volumeId);
    std::string fsTypeString = std::string(fsType);
    if (volumeIdString.empty() || fsTypeString.empty()) {
        LOGE("Invalid parameter, is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    if (!OHOS::StorageManager::IsSystemApp()) {
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
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
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }

    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
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
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    if (!OHOS::StorageManager::IsSystemApp()) {
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
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
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    if (!OHOS::StorageManager::IsSystemApp()) {
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
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
        LOGE("Invalid parameter, parameter is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    if (!OHOS::StorageManager::IsSystemApp()) {
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
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
        LOGE("Invalid parameter, parameter is empty");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }
    if (!OHOS::StorageManager::IsSystemApp()) {
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_PERMISSION_SYS);
        return;
    }

    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
        return;
    }
    int32_t errNum = instance->SetVolumeDescription(uuidString, descStr);
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
// NOLINTEND
