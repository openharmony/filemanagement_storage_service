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

#include "mtp/usb_event_subscriber.h"
#include <libmtp.h>
#include <unistd.h>
#include "mtp/mtp_device_monitor.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *BUS_NUM_KEY = "busNum";
constexpr const char *DEV_ADDRESS_KEY = "devAddress";
constexpr const char *DEV_VENDOR_ID_KEY = "vendorId";
constexpr const char *DEV_PRODUCT_ID_KEY = "productId";
constexpr const char *DEV_CLASS_KEY = "clazz";
constexpr int USB_CLASS_IMAGE = 6;
constexpr int USB_CLASS_VENDOR_SPEC = 255;

UsbEventSubscriber::UsbEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info)
    : EventFwk::CommonEventSubscriber(info)
{}

std::shared_ptr<UsbEventSubscriber> usbEventSubscriber_ = nullptr;
void UsbEventSubscriber::SubscribeCommonEvent(void)
{
    if (usbEventSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_STATE);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_PORT_CHANGED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_ATTACHED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_DETACHED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        usbEventSubscriber_ = std::make_shared<UsbEventSubscriber>(subscribeInfo);
        if (!EventFwk::CommonEventManager::SubscribeCommonEvent(usbEventSubscriber_)) {
            usbEventSubscriber_ = nullptr;
            LOGE("UsbEventSubscriber subscribe common event failed.");
        }
    }
}

void UsbEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    LOGI("UsbEventSubscriber::OnReceiveEvent.");
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_STATE) {
        LOGI("OnReceiveEvent COMMON_EVENT_USB_STATE, data=%{public}s", data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_PORT_CHANGED) {
        LOGI("OnReceiveEvent COMMON_EVENT_USB_PORT_CHANGED, data=%{public}s", data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_ATTACHED) {
        LOGI("OnReceiveEvent COMMON_EVENT_USB_ACCESSORY_ATTACHED, data=%{public}s", data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_DETACHED) {
        LOGI("OnReceiveEvent COMMON_EVENT_USB_ACCESSORY_DETACHED, data=%{public}s", data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED) {
        std::string usbInfo = data.GetData();
        LOGI("OnReceiveEvent COMMON_EVENT_USB_DEVICE_ATTACHED, data=%{public}s", usbInfo.c_str());
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
        if (IsMTPDevice(usbInfo)) {
            DelayedSingleton<MtpDeviceMonitor>::GetInstance()->MountMtpDeviceByBroadcast();
        }
#endif
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED) {
        std::string usbInfo = data.GetData();
        LOGI("OnReceiveEvent COMMON_EVENT_USB_DEVICE_DETACHED, data=%{public}s", data.GetData().c_str());
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
        if (IsMTPDevice(usbInfo)) {
            uint8_t devNum = 0;
            uint32_t busLoc = 0;
            GetValueFromUsbDataInfo(data.GetData(), devNum, busLoc);
            DelayedSingleton<MtpDeviceMonitor>::GetInstance()->UmountDetachedMtpDevice(devNum, busLoc);
        }
#endif
    }
}

void UsbEventSubscriber::GetValueFromUsbDataInfo(const std::string &jsonStr, uint8_t &devNum, uint32_t &busLoc)
{
    cJSON *usbJson = cJSON_Parse(jsonStr.c_str());
    if (usbJson == nullptr) {
        LOGE("GetValueFromUsbDataInfo failed, parse json object is nullptr.");
        return;
    }
    cJSON *busLocObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_ADDRESS_KEY);
    if (busLocObj != nullptr && cJSON_IsNumber(busLocObj)) {
        devNum = static_cast<uint8_t>(busLocObj->valueint);
    }
    cJSON *devNumObj = cJSON_GetObjectItemCaseSensitive(usbJson, BUS_NUM_KEY);
    if (devNumObj != nullptr && cJSON_IsNumber(devNumObj)) {
        busLoc = static_cast<uint32_t>(devNumObj->valueint);
    }
    cJSON_Delete(usbJson);
}

std::string UsbEventSubscriber::toLowerString(const char* str)
{
    if (!str) return "";
    std::string lowerStr(str);
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

bool UsbEventSubscriber::CheckMtpInterface(const cJSON* iface)
{
    cJSON* clazz = cJSON_GetObjectItemCaseSensitive(iface, "clazz");
    cJSON* name = cJSON_GetObjectItemCaseSensitive(iface, "name");
    
    if (clazz && clazz->type == cJSON_Number) {
        int ifaceClazz = clazz->valueint;
        if (ifaceClazz == USB_CLASS_IMAGE || ifaceClazz == USB_CLASS_VENDOR_SPEC) {
            return true;
        }
    }
    if (name && name->type == cJSON_String && name->valuestring) {
        std::string ifaceName = toLowerString(name->valuestring);
        return (ifaceName.find("mtp") != std::string::npos ||
                ifaceName.find("ptp") != std::string::npos);
    }
    
    return false;
}

bool UsbEventSubscriber::CheckAllInterfaces(const cJSON* configs)
{
    if (!configs || configs->type != cJSON_Array) return false;

    cJSON* config = nullptr;
    cJSON_ArrayForEach(config, configs) {
        cJSON* interfaces = cJSON_GetObjectItemCaseSensitive(config, "interfaces");
        if (!interfaces || interfaces->type != cJSON_Array) continue;

        cJSON* iface = nullptr;
        cJSON_ArrayForEach(iface, interfaces) {
            if (CheckMtpInterface(iface)) {
                return true;
            }
        }
    }
    return false;
}

bool UsbEventSubscriber::IsMTPDevice(const std::string &usbInfo)
{
    cJSON *usbJson = cJSON_Parse(usbInfo.c_str());
    if (!usbJson) {
        return false;
    }

    cJSON* productName = cJSON_GetObjectItemCaseSensitive(usbJson, "productName");
    if (productName && (toLowerString(productName->valuestring).find("mtp") != std::string::npos ||
                        toLowerString(productName->valuestring).find("ptp") != std::string::npos)) {
        cJSON_Delete(usbJson);
        return true;
    }

    cJSON* configs = cJSON_GetObjectItemCaseSensitive(usbJson, "configs");
    if (CheckAllInterfaces(configs)) {
        cJSON_Delete(usbJson);
        return true;
    }

    cJSON *classObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_CLASS_KEY);
    cJSON *vendorObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_VENDOR_ID_KEY);
    cJSON *productObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_PRODUCT_ID_KEY);
    
    bool isMtp = classObj && vendorObj && productObj && cJSON_IsNumber(classObj) &&
                 cJSON_IsNumber(vendorObj) && cJSON_IsNumber(productObj) && LIBMTP_check_is_mtp_device(
                     static_cast<uint8_t>(classObj->valueint),
                     static_cast<uint16_t>(vendorObj->valueint),
                     static_cast<uint16_t>(productObj->valueint));
    
    cJSON_Delete(usbJson);
    LOGD("Final MTP check result: %{public}d", isMtp);
    return isMtp;
}
}  // namespace StorageDaemon
}  // namespace OHOS