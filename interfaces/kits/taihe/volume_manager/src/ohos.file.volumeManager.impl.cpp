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

using namespace ANI::volumeManager;

namespace ANI::volumeManager {

Volume MakeVolume(string_view a, string_view b) {
    return {a, b};
}

Volume GetVolumeByUuidSync(string_view uuid) {
    if (!OHOS::StorageManager::IsSystemApp()) {
        set_business_error(OHOS::E_PERMISSION_SYS, "Not a system app");
        return MakeVolume("", "");
    }

    auto volumeInfo = std::make_shared<OHOS::StorageManager::VolumeExternal>();
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        set_error("StorageManagerConnect instance failed");
        return MakeVolume("", "");
    }

    int32_t errNum = instance->GetVolumeByUuid(uuid.c_str(), *volumeInfo);;
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "GetVolumeByUuid failed");
        return MakeVolume("", "");
    }

    return MakeVolume(volumeInfo->GetDescription(), volumeInfo->GetUuid());
}

array_view<Volume> GetAllVolumesSync() {
    auto volumeInfo = std::make_shared<std::vector<OHOS::StorageManager::VolumeExternal>>();

    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        set_error("Get StorageManagerConnect instacne failed");
        return array<Volume>::make(0, Volume{});
    }

    int32_t errNum = instance->GetAllVolumes(*volumeInfo);
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "GetAllVolumes failed");
        return array<Volume>::make(0, Volume{});
    }
    
    auto result = array<Volume>::make(volumeInfo->size(), Volume{});
    std::transform(volumeInfo->begin(), volumeInfo->end(), result.begin(), [](auto& vol) {
        return MakeVolume(vol.GetDescription(), vol.GetUuid());
    });

    return result;
}
}  // namespace ANI::volumeManager

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_MakeVolume(MakeVolume);
TH_EXPORT_CPP_API_GetVolumeByUuidSync(GetVolumeByUuidSync);
TH_EXPORT_CPP_API_GetAllVolumesSync(GetAllVolumesSync);
// NOLINTEND
