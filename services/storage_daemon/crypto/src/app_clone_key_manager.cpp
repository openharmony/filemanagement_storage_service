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

#include <filesystem>

#include "app_clone_key_manager.h"
#include "crypto/key_manager.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr const char *NEED_RESTORE_PATH = "/data/service/el1/public/storage_daemon/sd/el1/%d/latest/need_restore";

int AppCloneKeyManager::ActiveAppCloneUserKey(unsigned int &failedUserId)
{
    for (int userId = StorageService::START_APP_CLONE_USER_ID;
         userId < StorageService::MAX_APP_CLONE_USER_ID; userId++) {
        std::string keyPath = StringPrintf(NEED_RESTORE_PATH, userId);
        std::error_code errCode;
        if (!std::filesystem::exists(keyPath, errCode)) {
            LOGD("restore path do not exist, errCode %{public}d", errCode.value());
            continue;
        }
        int ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, EL2_KEY, {}, {});
        if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
            failedUserId = static_cast<unsigned int>(userId);
            LOGE("Active app clone user %{public}u el2 failed, ret=%{public}d.", userId, ret);
            StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveAppCloneUserKey", userId, ret, "EL2");
            return ret;
        }
        ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, EL3_KEY, {}, {});
        if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
            failedUserId = static_cast<unsigned int>(userId);
            LOGE("Active app clone user %{public}u el3 failed, ret=%{public}d.", userId, ret);
            StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveAppCloneUserKey", userId, ret, "EL3");
            return ret;
        }
        ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, EL4_KEY, {}, {});
        if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
            failedUserId = static_cast<unsigned int>(userId);
            LOGE("Active app clone user %{public}u el4 failed, ret=%{public}d.", userId, ret);
            StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveAppCloneUserKey", userId, ret, "EL4");
            return ret;
        }
        LOGI("ActiveAppCloneUserKey::userkey %{public}u activated.", userId);
        return E_OK;
    }
    LOGE("ActiveAppCloneUserKey::Did not find app clone userId in valid range = {219 ~ 239}.");
    return E_NOT_SUPPORT;
}
} // namespace StorageDaemon
} // namespace OHOS