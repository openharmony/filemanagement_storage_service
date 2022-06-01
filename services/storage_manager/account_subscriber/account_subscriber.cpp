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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "want.h"

#include "storage_service_log.h"

#include "media_library_manager.h"

using namespace OHOS::AAFwk;
namespace OHOS {
namespace StorageManager {
AccountSubscriber::AccountSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : EventFwk::CommonEventSubscriber(subscriberInfo)
{}

std::shared_ptr<AccountSubscriber> AccountSubscriber_ = nullptr;
bool AccountSubscriber::Subscriber(void)
{
    if (AccountSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        AccountSubscriber_ = std::make_shared<AccountSubscriber>(subscribeInfo);
        EventFwk::CommonEventManager::SubscribeCommonEvent(AccountSubscriber_);
    }
    return true;
}

Media::MediaLibraryManager *mgr = nullptr;

void AccountSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    LOGI("StorageManager: OnReceiveEvent action:%{public}s.", action.c_str());

    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("GetSystemAbilityManager sam == nullptr");
        return;
    }

    if (mgr == nullptr) {
        mgr = Media::MediaLibraryManager::GetMediaLibraryManager();
        auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (remoteObj == nullptr) {
            LOGE("GetSystemAbility remoteObj == nullptr");
            return;
        }
        mgr->InitMediaLibraryManager(remoteObj);
    }
}
}  // namespace StorageManager
}  // namespace OHOS