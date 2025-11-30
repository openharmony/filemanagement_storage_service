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
#ifndef STORAGE_COMMON_EVENT_SUBSCRIBER_H
#define STORAGE_COMMON_EVENT_SUBSCRIBER_H

#include <string>
#include <atomic>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace StorageManager {

enum DeviceState : uint8_t {
    STATE_SCREEN_OFF = 0x01,      // bit 0: 息屏标志
    STATE_POWER_CONNECTED = 0x02, // bit 1: 充电标志
    STATE_CHARGING_SCREEN_OFF = 0x03, // 0x01 | 0x02: 充电灭屏状态
};

class StorageCommonEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    StorageCommonEventSubscriber() = default;
    explicit StorageCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info);
    virtual ~StorageCommonEventSubscriber() = default;
    static void SubscribeCommonEvent(void);
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    static inline std::shared_ptr<StorageCommonEventSubscriber> subscriber_ = nullptr;

private:
    void UpdateDeviceState(DeviceState state, bool set);
    void HandleBatteryChangedEvent(int batteryCapacity);
    void CheckAndTriggerStatistic();
    std::atomic<uint8_t> deviceState_{0x00}; // 默认: 0x00 = 亮屏+断电
    std::atomic<int> batteryCapacity_{0};
};
}  // namespace StorageManager
}  // namespace OHOS
#endif // STORAGE_COMMON_EVENT_SUBSCRIBER_H