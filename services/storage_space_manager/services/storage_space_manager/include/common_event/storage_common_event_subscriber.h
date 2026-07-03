/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef STORAGE_SPACE_MANAGER_STORAGE_COMMON_EVENT_SUBSCRIBER_H
#define STORAGE_SPACE_MANAGER_STORAGE_COMMON_EVENT_SUBSCRIBER_H

#include <string>
#include <atomic>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace StorageSpaceManager {

class StorageCommonEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    StorageCommonEventSubscriber() = default;
    explicit StorageCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info);
    virtual ~StorageCommonEventSubscriber() = default;
    static void SubscribeCommonEvent(void);
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    static inline std::shared_ptr<StorageCommonEventSubscriber> subscriber_ = nullptr;

private:
    std::atomic<uint8_t> deviceState_{0x00}; // default: 0x00 = screen on + power connected
    std::atomic<int> batteryCapacity_{0};
};

}  // namespace StorageSpaceManager
}  // namespace OHOS
#endif // STORAGE_SPACE_MANAGER_STORAGE_COMMON_EVENT_SUBSCRIBER_H
