/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "common_event/storage_common_event_subscriber.h"

#include "iservice_registry.h"
#include "int_wrapper.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "storage_service_constant.h"
#include "storage/storage_status_service.h"

namespace OHOS {
namespace StorageManager {
using namespace OHOS::StorageService;
static constexpr int32_t WANT_DEFAULT_VALUE = -1;
StorageCommonEventSubscriber::StorageCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info)
    : EventFwk::CommonEventSubscriber(info) {}

void StorageCommonEventSubscriber::SubscribeCommonEvent(void)
{
    LOGI("subscribe common event start");
    if (subscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscriber_ = std::make_shared<StorageCommonEventSubscriber>(subscribeInfo);
        if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
            subscriber_ = nullptr;
            LOGE("subscribe common event failed.");
            return;
        }
    }
    LOGI("subscribe common event success");
}

void StorageCommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        int32_t userId = want.GetIntParam(USER_ID, WANT_DEFAULT_VALUE);
        if (userId <= 0) {
            return;
        }
        std::string bundleName = want.GetStringParam(BUNDLE_NAME);
        LOGI("receive app remove action, userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
#ifdef STORAGE_STATISTICS_MANAGER
        StorageStatusService::GetInstance().DelBundleExtStats(static_cast<uint32_t>(userId), bundleName);
#endif
    }
}
}  // namespace StorageManager
}  // namespace OHOS