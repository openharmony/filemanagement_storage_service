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
#include "common_event_support.h"
#include "datashare_helper.h"
#include "storage_service_constant.h"

namespace OHOS {
namespace StorageManager {
enum {
    USER_UNLOCK_BIT,
    USER_SWITCH_BIT
};

class AccountSubscriber : public EventFwk::CommonEventSubscriber {
public:
    AccountSubscriber() = default;
    explicit AccountSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    static void Subscriber(void);
    virtual ~AccountSubscriber() = default;
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
    static void ResetUserEventRecord(int32_t userId);
private:
    std::map<int32_t, std::shared_ptr<DataShare::DataShareHelper>> mediaShareMap_;

    int32_t userId_ = 0;
    std::unordered_map<int32_t, uint32_t> userRecord_;
    uint32_t GetUserStatus(int32_t userId);
    uint32_t HandleUserUnlockEvent(uint32_t userStatus);
    uint32_t HandleUserSwitchedEvent(uint32_t userStatus);
    void GetSystemAbility();
};
}  // namespace StorageManager
}  // namespace OHOS

#endif // STORAGE_MANAGER_ACCOUNT_SUBSCRIBER_H