/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "common_event/common_event_service.h"

#include "storage_service_log.h"
#include "storage/storage_monitor_service.h"

namespace OHOS {
namespace StorageManager {

std::shared_ptr<CommonEventService> commonEventSubscriber_ = nullptr;
constexpr int32_t BATTERY_LEVEL_TEN = 10;

CommonEventService::CommonEventService(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    LOGI("CommonEventService constructor");
}

void CommonEventService::SubscribeScreenAndPowerEvent()
{
    if (commonEventSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);

        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        commonEventSubscriber_ = std::make_shared<CommonEventService>(subscribeInfo);

        if (!EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_)) {
            commonEventSubscriber_ = nullptr;
            LOGE("CommonEventService subscribe common event failed.");
        }
    }
}

void CommonEventService::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    LOGI("CommonEventService receive event: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        HandleScreenOffEvent();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        HandleScreenOnEvent();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        HandlePowerConnectedEvent();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        HandlePowerDisconnectedEvent();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED) {
        int batteryLevel = data.GetCode();
        HandleBatteryChangedEvent(batteryLevel);
    }
}

void CommonEventService::HandleScreenOffEvent()
{
    LOGI("Handle screen off event");
    isScreenOff_ = true;
    CheckAndTriggerStatistic();
}

void CommonEventService::HandleScreenOnEvent()
{
    LOGI("Handle screen on event");
    isScreenOff_ = false;
    CheckAndTriggerStatistic();
}

void CommonEventService::HandlePowerConnectedEvent()
{
    LOGI("Handle power connected event");
    isCharging_ = true;
    CheckAndTriggerStatistic();
}

void CommonEventService::HandlePowerDisconnectedEvent()
{
    LOGI("Handle power disconnected event");
    isCharging_ = false;
    CheckAndTriggerStatistic();
}

void CommonEventService::HandleBatteryChangedEvent(int batteryLevel)
{
    LOGI("Handle battery changed event, level=%{public}d", batteryLevel);
    batteryLevel_ = batteryLevel;
    CheckAndTriggerStatistic();
}

void CommonEventService::CheckAndTriggerStatistic()
{
    LOGI("CheckAndTriggerStatistic: isScreenOff=%{public}d, isCharging=%{public}d, battery=%{public}d%%",
         isScreenOff_, isCharging_, batteryLevel_);
    if (isScreenOff_ && isCharging_ && batteryLevel_ > BATTERY_LEVEL_TEN) {
        LOGI("Trigger storage scan - device is charging, screen off, and battery > 10%%");
        StorageMonitorService::GetInstance().StartScan();
    } else {
        LOGI("Stop storage scan - device exits charging or screen-off state");
        StorageMonitorService::GetInstance().StopScan();
    }
}
} // namespace StorageManager
} // namespace OHOS
