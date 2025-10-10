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
std::atomic<bool> UsbEventSubscriber::isPtp_ = true;

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
            MtpDeviceMonitor::GetInstance().MountMtpDeviceByBroadcast();
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
            MtpDeviceMonitor::GetInstance().UmountDetachedMtpDevice(devNum, busLoc);
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

std::string UsbEventSubscriber::ToLowerString(const char* str)
{
    if (str == nullptr) {
        return "";
    }
    std::string lowerStr(str);
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

bool UsbEventSubscriber::CheckMtpInterface(const cJSON* iface)
{
    cJSON* clazz = cJSON_GetObjectItemCaseSensitive(iface, "clazz");
    if (clazz != nullptr && cJSON_IsNumber(clazz) && clazz->valueint) {
        const int ifaceClazz = clazz->valueint;
        if (ifaceClazz == USB_CLASS_IMAGE || ifaceClazz == USB_CLASS_VENDOR_SPEC) {
            return true;
        }
    }

    cJSON* name = cJSON_GetObjectItemCaseSensitive(iface, "name");
    if (name != nullptr && cJSON_IsString(name) && name->valuestring) {
        const std::string ifaceName = ToLowerString(name->valuestring);
        if (ifaceName.find("mtp") != std::string::npos) {
            isPtp_ = false;
            return true;
        }
        if (ifaceName.find("ptp") != std::string::npos) {
            isPtp_ = true;
            return true;
        }
    }
    return false;
}

bool UsbEventSubscriber::CheckAllInterfaces(const cJSON* configs)
{
    if (configs == nullptr || !cJSON_IsArray(configs)) {
        return false;
    }

    cJSON* config = nullptr;
    cJSON_ArrayForEach(config, configs) {
        cJSON* name = cJSON_GetObjectItemCaseSensitive(config, "name");
        if (name != nullptr && cJSON_IsString(name) && name->valuestring) {
            const std::string configName = ToLowerString(name->valuestring);
            if (configName.find("mtp") != std::string::npos) {
                isPtp_ = false;
                return true;
            }
            if (configName.find("ptp") != std::string::npos) {
                isPtp_ = true;
                return true;
            }
        }
        cJSON* interfaces = cJSON_GetObjectItemCaseSensitive(config, "interfaces");
        if (interfaces == nullptr || !cJSON_IsArray(interfaces)) {
            continue;
        }
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
    if (usbJson == nullptr) {
        LOGE("IsMTPDevice failed, parse json object is nullptr.");
        return false;
    }

    cJSON* productName = cJSON_GetObjectItemCaseSensitive(usbJson, "productName");
    if (productName && cJSON_IsString(productName) && productName->valuestring) {
        std::string lowerName = ToLowerString(productName->valuestring);
        if ((lowerName.find("mtp") != std::string::npos || lowerName.find("ptp") != std::string::npos)) {
            cJSON_Delete(usbJson);
            return true;
        }
    }

    cJSON* configs = cJSON_GetObjectItemCaseSensitive(usbJson, "configs");
    if (CheckAllInterfaces(configs)) {
        cJSON_Delete(usbJson);
        return true;
    }

    cJSON *classObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_CLASS_KEY);
    if (classObj == nullptr || !cJSON_IsNumber(classObj)) {
        LOGE("parse deviceClass failed.");
        cJSON_Delete(usbJson);
        return false;
    }
    uint8_t deviceClass = static_cast<uint8_t>(classObj->valueint);
    cJSON *vendorObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_VENDOR_ID_KEY);
    if (vendorObj == nullptr || !cJSON_IsNumber(vendorObj)) {
        LOGE("parse vendorObj failed.");
        cJSON_Delete(usbJson);
        return false;
    }
    uint16_t idVendor = static_cast<uint16_t>(vendorObj->valueint);
    cJSON *productObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_PRODUCT_ID_KEY);
    if (productObj == nullptr || !cJSON_IsNumber(productObj)) {
        LOGE("parse productObj failed.");
        cJSON_Delete(usbJson);
        return false;
    }
    uint16_t idProduct = static_cast<uint16_t>(productObj->valueint);
    cJSON_Delete(usbJson);
    if (LIBMTP_check_is_mtp_device(deviceClass, idVendor, idProduct)) {
        LOGE("this is mtp device.");
        return true;
    }
    LOGE("this is not mtp device.");
    return false;
}

bool UsbEventSubscriber::IsPtpMode()
{
    LOGI("PTP mode status: %{public}d", isPtp_.load());
    return isPtp_;
}
}  // namespace StorageDaemon
}  // namespace OHOS