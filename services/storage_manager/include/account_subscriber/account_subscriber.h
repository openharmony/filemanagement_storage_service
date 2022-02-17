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
#ifndef STORAGE_MANAGER_ACCOUNT_SUBSCRIBER_H
#define STORAGE_MANAGER_ACCOUNT_SUBSCRIBER_H

#include "common_event_manager.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "matching_skills.h"

namespace OHOS {
namespace StorageManager {
class AccountSubscriber : public EventFwk::CommonEventSubscriber {
public:
    AccountSubscriber() = default;
    explicit AccountSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    static bool Subscriber(void);
    virtual ~AccountSubscriber() = default;
    
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
};
}  // namespace StorageManager
}  // namespace OHOS

#endif // STORAGE_MANAGER_ACCOUNT_SUBSCRIBER_H