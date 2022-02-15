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
#include "storage_service_log.h"
#include "want.h"

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
    if (AccountSubscriber_ == nullptr) {
    }
    return true;
}

void AccountSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    LOGI("StorageManager: OnReceiveEvent action:%{public}s.", action.c_str());
    int pid;
    if ((pid = fork()) == 0) {
        std::vector<std::string> args = {"/system/bin/aa", "start", "-b",
            "com.ohos.medialibrary.MediaScannerAbilityA", "-a", "MediaScannerAbility"};
        std::vector<char*> argv;
        for (size_t i = 0; i < args.size(); i++) {
            argv.push_back(const_cast<char*>(args[i].c_str()));
        }
        argv.push_back(nullptr);
        execvp("/system/bin/aa", argv.data());
        exit(0);
    }
    waitpid(pid, nullptr, 0);
}
}  // namespace StorageManager
}  // namespace OHOS