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

ohos::file::volumeManager::Volume MakeVolume(taihe::string_view description, taihe::string_view uuid)
{
    return {description, uuid};
}

ohos::file::volumeManager::Volume GetVolumeByUuidSync(taihe::string_view uuid)
{
    if (!OHOS::StorageManager::IsSystemApp()) {
        taihe::set_business_error(OHOS::E_PERMISSION_SYS, "Not a system app");
        return MakeVolume("", "");
    }

    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        taihe::set_error("StorageManagerConnect instance failed");
        return MakeVolume("", "");
    }

    int32_t errNum = instance->GetVolumeByUuid(uuid.c_str(), *volumeInfo);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "GetVolumeByUuid failed");
        return MakeVolume("", "");
    }

    return MakeVolume(volumeInfo->GetDescription(), volumeInfo->GetUuid());
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
    std::transform(volumeInfo->begin(), volumeInfo->end(), result.begin(),
                   [](auto &vol) { return MakeVolume(vol.GetDescription(), vol.GetUuid()); });

    return result;
}
} // namespace ANI::VolumeManager

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_MakeVolume(ANI::VolumeManager::MakeVolume);
TH_EXPORT_CPP_API_GetVolumeByUuidSync(ANI::VolumeManager::GetVolumeByUuidSync);
TH_EXPORT_CPP_API_GetAllVolumesSync(ANI::VolumeManager::GetAllVolumesSync);
// NOLINTEND
