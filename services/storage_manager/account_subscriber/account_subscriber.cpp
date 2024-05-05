/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "account_subscriber/account_subscriber.h"

#include <cinttypes>
#include <memory>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#ifdef USER_CRYPTO_MANAGER
#include "crypto/filesystem_crypto.h"
#endif
#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AccountSA;
namespace OHOS {
namespace StorageManager {
static std::mutex userRecordLock;
std::shared_ptr<DataShare::DataShareHelper> AccountSubscriber::mediaShare_ = nullptr;

AccountSubscriber::AccountSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : EventFwk::CommonEventSubscriber(subscriberInfo)
{}

std::shared_ptr<AccountSubscriber> AccountSubscriber_ = nullptr;
bool AccountSubscriber::Subscriber(void)
{
    if (AccountSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        AccountSubscriber_ = std::make_shared<AccountSubscriber>(subscribeInfo);
        EventFwk::CommonEventManager::SubscribeCommonEvent(AccountSubscriber_);
    }
    return true;
}

static void MountCryptoPathAgain(int32_t userId)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->MountCryptoPathAgain(userId);
    if (err != 0) {
        LOGI("mount crypto path failed err is %{public}d", err);
        return;
    }
    LOGI("MountCryptoPathAgain success");
}

void AccountSubscriber::ResetUserEventRecord(int32_t userId)
{
    if (userId < StorageService::START_USER_ID || userId > StorageService::MAX_USER_ID) {
        return;
    }
    LOGI("ResetUserEventRecord start, userId is %{public}d", userId);
    if (AccountSubscriber_->userRecord_.find(userId) != AccountSubscriber_->userRecord_.end()) {
        std::lock_guard<std::mutex> lock(userRecordLock);
        AccountSubscriber_->userRecord_.erase(userId);
    }
}

void AccountSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    int32_t userId = eventData.GetCode();
    std::unique_lock<std::mutex> lock(mutex_);
    LOGI("OnReceiveEvent action:%{public}s, userId is %{public}d", action.c_str(), userId);
    /* get user status */
    uint32_t status = 0;
    auto entry = userRecord_.find(userId);
    if (entry != userRecord_.end()) {
        status = entry->second;
    }
    /* update status */
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        if (status == (1 << USER_UNLOCK_BIT | 1 << USER_SWITCH_BIT)) {
            status = 0;
        }
        status |= 1 << USER_UNLOCK_BIT;
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        status |= 1 << USER_SWITCH_BIT;
        /* clear previous user status */
        auto oldEntry = userRecord_.find(userId_);
        if (oldEntry != userRecord_.end()) {
            userRecord_[userId_] = oldEntry->second & (~USER_SWITCH_BIT);
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED) {
        std::vector<int32_t> ids;
        int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
        if (ret != 0 || ids.empty()) {
            LOGE("Query active userid failed, ret = %{public}u", ret);
            return;
        }
        userId = ids[0];
        if (!OnReceiveEventLockUserScreen(userId)) {
            LOGE("user %{public}u LockUserScreen fail", userId);
        }
        LOGI("Handle EventFwk::CommonEventSupport::Common_EVENT_SCREEN_LOCKED finished, userId is %{public}u",
            userId);
        return;
    }
    userId_ = userId;
    userRecord_[userId] = status;
    LOGI("userId %{public}d, status %{public}d", userId, status);
    if (status != (1 << USER_UNLOCK_BIT | 1 << USER_SWITCH_BIT)) {
        return;
    }

    if ((status & USER_UNLOCK_BIT) == USER_UNLOCK_BIT) {
        MountCryptoPathAgain(userId);
    }
    lock.unlock();

    LOGI("connect %{public}d media library", userId);
    GetSystemAbility();
}

void AccountSubscriber::GetSystemAbility()
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("GetSystemAbilityManager sam == nullptr");
        return;
    }
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOGE("GetSystemAbility remoteObj == nullptr");
        return;
    }
    mediaShare_ = DataShare::DataShareHelper::Creator(remoteObj, "datashare:///media");
}

bool AccountSubscriber::OnReceiveEventLockUserScreen(int32_t userId)
{
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    if (fsCrypto != nullptr) {
        int ret = fsCrypto->LockUserScreen(userId);
        if (ret != 0) {
            LOGE("user %{public}u LockUserScreen fail", userId);
            return false;
        }
    } else {
        LOGE("fsCrypto is nullptr");
        return false;
    }
    return true;
}
}  // namespace StorageManager
}  // namespace OHOS
