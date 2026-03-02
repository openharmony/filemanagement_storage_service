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

#include "ohos.file.keyManager.impl.h"
#include "storageStatistics_taihe_error.h"
#include "storage_service_log.h"

namespace ANI::KeyManager {
void DeactivateUserKey(int64_t userId)
{
    auto instance = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance();
    if (instance == nullptr) {
        LOGE("Get StorageManagerConnect instance failed");
        OHOS::StorageTaiheError::SetStorageTaiheError(OHOS::E_IPCSS);
        return;
    }
    uint32_t userId_i = static_cast<uint32_t>(userId);
    int32_t errNum = instance->DeactivateUserKey(userId_i);
    if (errNum != OHOS::E_OK) {
        OHOS::StorageTaiheError::SetStorageTaiheError(errNum);
        return;
    }
}
} // namespace ANI::keyManager
TH_EXPORT_CPP_API_DeactivateUserKey(ANI::KeyManager::DeactivateUserKey);
