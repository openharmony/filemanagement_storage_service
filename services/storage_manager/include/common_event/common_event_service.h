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

#ifndef OHOS_STORAGE_MANAGER_COMMON_EVENT_SERVICE_H
#define OHOS_STORAGE_MANAGER_COMMON_EVENT_SERVICE_H

#include <memory>
#include "common_event_manager.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

namespace OHOS {
namespace StorageManager {
class CommonEventService : public EventFwk::CommonEventSubscriber {
public:
    explicit CommonEventService(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    virtual ~CommonEventService() = default;

    static void SubscribeScreenAndPowerEvent();
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

private:
    void HandleScreenOffEvent();
    void HandleScreenOnEvent();
    void HandlePowerConnectedEvent();
    void HandlePowerDisconnectedEvent();
    void HandleBatteryChangedEvent(int batteryLevel);
    void CheckAndTriggerStatistic();

    bool isScreenOff_ = false;
    bool isCharging_ = false;
    int batteryLevel_ = 0;
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_COMMON_EVENT_SERVICE_H
