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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "want.h"

using namespace OHOS::AAFwk;
namespace OHOS {
namespace StorageManager {
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
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        AccountSubscriber_ = std::make_shared<AccountSubscriber>(subscribeInfo);
        EventFwk::CommonEventManager::SubscribeCommonEvent(AccountSubscriber_);
    }
    return true;
}

void AccountSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    int32_t userId = eventData.GetCode();
    LOGI("StorageManager: OnReceiveEvent action:%{public}s.", action.c_str());

    std::unique_lock<std::mutex> lock(mutex_);
    /* get user status */
    uint32_t status = 0;
    auto entry = userRecord_.find(userId);
    if (entry != userRecord_.end()) {
        status = entry->second;
    }

    /* update status */
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        status |= 1 << USER_UNLOCK_BIT;
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        status |= 1 << USER_SWITCH_BIT;
        /* clear previous user status */
        auto oldEntry = userRecord_.find(userId_);
        if (oldEntry != userRecord_.end()) {
            userRecord_[userId_] = oldEntry->second & (~USER_SWITCH_BIT);
        }
    }
    userId_ = userId;
    userRecord_[userId] = status;

    LOGI("userId %{public}d, status %{public}d", userId, status);
    if (status != (1 << USER_UNLOCK_BIT | 1 << USER_SWITCH_BIT)) {
        return;
    }
    lock.unlock();

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

    LOGI("connect %{public}d media library", userId);
    mediaShare_ = DataShare::DataShareHelper::Creator(remoteObj, "datashare:///media");
}

void MediaShareDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    auto sptr = DataShare::DataShareHelper::Creator(object.promote(), "datashare:///media");
    AccountSubscriber::SetMediaShare(sptr);
}
}  // namespace StorageManager
}  // namespace OHOS
