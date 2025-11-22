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

#include "ohos.file.keyManager.impl.h"

namespace ANI::KeyManager {
void DeactivateUserKey(int64_t userId)
{
    uint32_t userId_i = static_cast<uint32_t>(userId);

    auto errNum = OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>
        ::GetInstance()->DeactivateUserKey(userId_i);
    if (errNum != OHOS::E_OK) {
        taihe::set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to deactivate user key.");
        return;
    }
}
} // namespace ANI::keyManager
TH_EXPORT_CPP_API_DeactivateUserKey(ANI::KeyManager::DeactivateUserKey);
