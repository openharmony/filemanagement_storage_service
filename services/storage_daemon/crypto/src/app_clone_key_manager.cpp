/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <string>
#include <filesystem>

#include "app_clone_key_manager.h"
#include "crypto/key_manager.h"
#include "utils/string_utils.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
static const std::string NEED_RESTORE_PATH = "/data/service/el1/public/storage_daemon/sd/el1/%d/latest/need_restore";

int AppCloneKeyManager::ActiveAppCloneUserKey()
{
    for (int userId = StorageService::START_APP_CLONE_USER_ID;
         userId < StorageService::MAX_APP_CLONE_USER_ID; userId++) {
        std::string keyPath = StringPrintf(NEED_RESTORE_PATH.c_str(), userId);
        std::error_code errCode;
        if (!std::filesystem::exists(keyPath, errCode)) {
            LOGE("restore path do not exist, errCode %{public}d", errCode.value());
            continue;
        }
        if (KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, EL2_KEY, {}, {}) != 0) {
            LOGE("Active app clone user %{public}u el2 failed", userId);
            return -EFAULT;
        }
        if (KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, EL3_KEY, {}, {}) != 0) {
            LOGE("Active app clone user %{public}u el3 failed", userId);
            return -EFAULT;
        }
        if (KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, EL4_KEY, {}, {}) != 0) {
            LOGE("Active app clone user %{public}u el4 failed", userId);
            return -EFAULT;
        }
        LOGI("ActiveAppCloneUserKey::userkey %{public}u activated", userId);
        return 0;
    }
    LOGE("ActiveAppCloneUserKey::Did not find app clone user in valid range");
    return -EFAULT;
}
} // namespace StorageDaemon
} // namespace OHOS