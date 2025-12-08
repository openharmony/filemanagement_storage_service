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
#include "storage/storage_status_manager.h"
#include "dfx_report/storage_dfx_reporter.h"

namespace OHOS {
namespace StorageManager {
using namespace OHOS::StorageService;
static constexpr int32_t WANT_DEFAULT_VALUE = -1;
constexpr int32_t BATTERY_LEVEL_TEN = 10;
constexpr const char* BATTERY_SOC_KEY = "soc";
StorageCommonEventSubscriber::StorageCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info)
    : EventFwk::CommonEventSubscriber(info) {}

void StorageCommonEventSubscriber::SubscribeCommonEvent(void)
{
    LOGI("subscribe common event start");
    if (subscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
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
        StorageStatusManager::GetInstance().DelBundleExtStats(static_cast<uint32_t>(userId), bundleName);
#endif
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        UpdateDeviceState(STATE_SCREEN_OFF, true);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        UpdateDeviceState(STATE_SCREEN_OFF, false);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        UpdateDeviceState(STATE_POWER_CONNECTED, true);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        UpdateDeviceState(STATE_POWER_CONNECTED, false);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED) {
        int batteryCapacity = eventData.GetWant().GetIntParam(BATTERY_SOC_KEY, 0);
        HandleBatteryChangedEvent(batteryCapacity);
    }
}

void StorageCommonEventSubscriber::UpdateDeviceState(DeviceState state, bool set)
{
    uint8_t oldState = deviceState_.load();
    uint8_t newState;
    if (set) {
        newState = oldState | state;
    } else {
        newState = oldState & ~state;
    }
    deviceState_.store(newState);
    LOGI("UpdateDeviceState: state=0x%{public}02x, set=%{public}d, deviceState=0x%{public}02x",
         state, set, newState);
    CheckAndTriggerStatistic();
}

void StorageCommonEventSubscriber::HandleBatteryChangedEvent(int batteryCapacity)
{
    LOGI("Handle battery changed event, level=%{public}d", batteryCapacity);
    batteryCapacity_.store(batteryCapacity);
    CheckAndTriggerStatistic();
}

void StorageCommonEventSubscriber::CheckAndTriggerStatistic()
{
    uint8_t currentDeviceState = deviceState_.load();
    int currentBatteryCapacity = batteryCapacity_.load();
    bool isChargingScreenOff = (currentDeviceState == STATE_CHARGING_SCREEN_OFF);
    LOGI("CheckAndTriggerStatistic: deviceState=0x%{public}02x, battery=%{public}d%%, isChargingScreenOff=%{public}d",
         currentDeviceState, currentBatteryCapacity, isChargingScreenOff);

    if (isChargingScreenOff && currentBatteryCapacity > BATTERY_LEVEL_TEN) {
        LOGI("Trigger storage scan - device is charging, screen off, and battery > 10%%");
        StorageDfxReporter::GetInstance().StartScan();
    } else {
        StorageDfxReporter::GetInstance().StopScan();
    }
}
}  // namespace StorageManager
}  // namespace OHOS