/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "key_crypto_utils.h"
#ifdef ENABLE_SCREENLOCK_MANAGER
#include "screenlock_manager.h"
#endif
#include "storage_service_log.h"
#include "utils/storage_radar.h"
#include "storage_manager_client.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageService {
void KeyCryptoUtils::ForceLockUserScreen()
{
    LOGI("[L8:KeyCryptoUtils] ForceLockUserScreen: >>> ENTER <<<");
#ifdef ENABLE_SCREENLOCK_MANAGER
    StorageDaemon::StorageManagerClient client;
    std::vector<int32_t> ids;
    auto ret = client.QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        LOGE("[L8:KeyCryptoUtils] ForceLockUserScreen: <<< EXIT FAILED <<< Query active userid failed, ret=%{public}u",
            ret);
        StorageRadar::ReportOsAccountResult("ForceLockUserScreen::QueryActiveOsAccountIds", ret, DEFAULT_USERID);
        return;
    }
    int reasonFlag = static_cast<int>(ScreenLock::StrongAuthReasonFlags::ACTIVE_REQUEST);
    ret = ScreenLock::ScreenLockManager::GetInstance()->RequestStrongAuth(reasonFlag, ids[0]);
    if (ret != ScreenLock::E_SCREENLOCK_OK) {
        LOGE("[L8:KeyCryptoUtils] ForceLockUserScreen: <<< EXIT FAILED <<< Request strong auth by screen lock manager"
            "failed, userId=%{public}d, ret=%{public}d", ids[0], ret);
        StorageRadar::ReportOsAccountResult("ForceLockUserScreen::RequestStrongAuth", ret, ids[0]);
        return;
    }
    ret = ScreenLock::ScreenLockManager::GetInstance()->Lock(ids[0]);
    if (ret != ScreenLock::E_SCREENLOCK_OK) {
        LOGE("[L8:KeyCryptoUtils] ForceLockUserScreen: <<< EXIT FAILED <<< Lock user screen by screen lock manager"
            "failed, userId=%{public}d, ret=%{public}d", ids[0], ret);
        StorageRadar::ReportOsAccountResult("ForceLockUserScreen::Lock", ret, ids[0]);
        return;
    }
    LOGI("[L8:KeyCryptoUtils] ForceLockUserScreen: <<< EXIT SUCCESS <<< userId=%{public}d", ids[0]);
#else
    LOGI("[L8:KeyCryptoUtils] ForceLockUserScreen: <<< EXIT SUCCESS <<< SCREENLOCK_MANAGER not enabled");
#endif
}

int32_t KeyCryptoUtils::CheckAccountExists(unsigned int userId, bool &isOsAccountExists)
{
    LOGW("[L8:KeyCryptoUtils] CheckAccountExists: >>> ENTER <<< userId=%{public}u", userId);
#ifdef ENABLE_SCREENLOCK_MANAGER
    StorageDaemon::StorageManagerClient client;
    int32_t ret = client.IsOsAccountExists(userId, isOsAccountExists);
    if (ret != ERR_OK) {
        LOGE("[L8:KeyCryptoUtils] CheckAccountExists: <<< EXIT FAILED <<< Check userId failed, userId=%{public}u,"
            "ret=%{public}u", userId, ret);
        StorageRadar::ReportOsAccountResult("CheckAccountExists::IsOsAccountExists", ret, userId);
        return ret;
    }
    LOGI("[L8:KeyCryptoUtils] CheckAccountExists: <<< EXIT SUCCESS <<< userId=%{public}u, isExists=%{public}d",
        userId, isOsAccountExists);
#else
    LOGI("[L8:KeyCryptoUtils] CheckAccountExists: <<< EXIT SUCCESS <<< SCREENLOCK_MANAGER not enabled");
#endif
    return 0;
}
}
}