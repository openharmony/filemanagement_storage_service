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

using namespace taihe;
using namespace ANI::keyManager;
namespace ANI::keyManager {
void DeactivateUserKey(int64_t userId)
{
    uint32_t userId_i = static_cast<uint32_t>(userId);

    if (!OHOS::StorageManager::IsSystemApp()) {
        set_business_error(OHOS::E_PERMISSION_SYS, "DeactivateUserKey is not allowed for non-system apps.");
        return;
    }

    auto errNum =
        OHOS::DelayedSingleton<OHOS::StorageManager::StorageManagerConnect>::GetInstance()->DeactivateUserKey(userId_i);
    std::cout << "DeactivateUserKey: userId = " << userId_i << ", errNum = " << errNum << std::endl;
    if (errNum != OHOS::E_OK) {
        set_business_error(OHOS::StorageManager::Convert2JsErrNum(errNum), "Failed to deactivate user key.");
        return;
    }
}
} // namespace ANI::keyManager
TH_EXPORT_CPP_API_DeactivateUserKey(DeactivateUserKey);