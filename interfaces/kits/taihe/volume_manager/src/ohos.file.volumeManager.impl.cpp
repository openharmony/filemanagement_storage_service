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
} // namespace ANI::VolumeManager

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetVolumeByUuidSync(ANI::VolumeManager::GetVolumeByUuidSync);
TH_EXPORT_CPP_API_GetAllVolumesSync(ANI::VolumeManager::GetAllVolumesSync);
// NOLINTEND
