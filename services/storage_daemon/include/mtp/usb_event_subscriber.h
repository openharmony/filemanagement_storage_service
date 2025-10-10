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
#ifndef OHOS_STORAGE_DAEMON_USB_EVENT_SUBSCRIBER_H
#define OHOS_STORAGE_DAEMON_USB_EVENT_SUBSCRIBER_H

#include <string>

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "cJSON.h"
#include "mtp_device_monitor.h"

namespace OHOS {
namespace StorageDaemon {
using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;
using CommonEventData = OHOS::EventFwk::CommonEventData;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;

class UsbEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    UsbEventSubscriber() = default;
    explicit UsbEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info);
    virtual ~UsbEventSubscriber() = default;

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
    static void SubscribeCommonEvent(void);
    static bool IsPtpMode();

private:
    void GetValueFromUsbDataInfo(const std::string &jsonStr, uint8_t &devNum, uint32_t &busLoc);
    bool IsMTPDevice(const std::string &usbInfo);
    std::string ToLowerString(const char* str);
    bool CheckMtpInterface(const cJSON* iface);
    bool CheckAllInterfaces(const cJSON* configs);

private:
    static std::atomic<bool> isPtp_;
};
} // namespace UsbEventSubscriber
} // namespace OHOS
#endif