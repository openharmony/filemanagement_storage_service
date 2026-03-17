/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
{
    LOGD("[L2:UsbEventSubscriber] UsbEventSubscriber: >>> ENTER <<<");
}

std::shared_ptr<UsbEventSubscriber> usbEventSubscriber_ = nullptr;
void UsbEventSubscriber::SubscribeCommonEvent(void)
{
    LOGI("[L2:UsbEventSubscriber] SubscribeCommonEvent: >>> ENTER <<<");
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
            LOGE("[L2:UsbEventSubscriber] SubscribeCommonEvent: <<< EXIT FAILED <<< subscribe failed");
        }
    }
    LOGI("[L2:UsbEventSubscriber] SubscribeCommonEvent: <<< EXIT SUCCESS <<<");
}

void UsbEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: >>> ENTER <<<");
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_STATE) {
        LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: COMMON_EVENT_USB_STATE, data=%{public}s",
            data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_PORT_CHANGED) {
        LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: COMMON_EVENT_USB_PORT_CHANGED, data=%{public}s",
            data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_ATTACHED) {
        LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: COMMON_EVENT_USB_ACCESSORY_ATTACHED, data=%{public}s",
            data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_ACCESSORY_DETACHED) {
        LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: COMMON_EVENT_USB_ACCESSORY_DETACHED, data=%{public}s",
            data.GetData().c_str());
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_ATTACHED) {
        std::string usbInfo = data.GetData();
        LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: COMMON_EVENT_USB_DEVICE_ATTACHED, data=%{public}s",
            usbInfo.c_str());
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
        if (IsMTPDevice(usbInfo)) {
            MtpDeviceMonitor::GetInstance().MountMtpDeviceByBroadcast();
        }
#endif
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USB_DEVICE_DETACHED) {
        std::string usbInfo = data.GetData();
        LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: COMMON_EVENT_USB_DEVICE_DETACHED, data=%{public}s",
            data.GetData().c_str());
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
        if (IsMTPDevice(usbInfo)) {
            uint8_t devNum = 0;
            uint32_t busLoc = 0;
            GetValueFromUsbDataInfo(data.GetData(), devNum, busLoc);
            MtpDeviceMonitor::GetInstance().UmountDetachedMtpDevice(devNum, busLoc);
        }
#endif
    }
    LOGI("[L2:UsbEventSubscriber] OnReceiveEvent: <<< EXIT SUCCESS <<<");
}

void UsbEventSubscriber::GetValueFromUsbDataInfo(const std::string &jsonStr, uint8_t &devNum, uint32_t &busLoc)
{
    LOGD("[L2:UsbEventSubscriber] GetValueFromUsbDataInfo: >>> ENTER <<<");
    cJSON *usbJson = cJSON_Parse(jsonStr.c_str());
    if (usbJson == nullptr) {
        LOGE("[L2:UsbEventSubscriber] GetValueFromUsbDataInfo: <<< EXIT FAILED <<< parse json failed");
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
    LOGD("[L2:UsbEventSubscriber] GetValueFromUsbDataInfo: <<< EXIT SUCCESS <<< devNum=%{public}u, busLoc=%{public}u",
        devNum, busLoc);
}

std::string UsbEventSubscriber::ToLowerString(const char* str)
{
    LOGD("[L2:UsbEventSubscriber] ToLowerString: >>> ENTER <<<");
    if (str == nullptr) {
        LOGD("[L2:UsbEventSubscriber] ToLowerString: <<< EXIT FAILED <<< str is nullptr");
        return "";
    }
    std::string lowerStr(str);
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    LOGD("[L2:UsbEventSubscriber] ToLowerString: <<< EXIT SUCCESS <<<");
    return lowerStr;
}

bool UsbEventSubscriber::CheckMtpInterface(const cJSON* iface)
{
    LOGD("[L2:UsbEventSubscriber] CheckMtpInterface: >>> ENTER <<<");
    cJSON* clazz = cJSON_GetObjectItemCaseSensitive(iface, "clazz");
    if (clazz != nullptr && cJSON_IsNumber(clazz) && clazz->valueint) {
        const int ifaceClazz = clazz->valueint;
        if (ifaceClazz == USB_CLASS_IMAGE || ifaceClazz == USB_CLASS_VENDOR_SPEC) {
            LOGD("[L2:UsbEventSubscriber] CheckMtpInterface: <<< EXIT SUCCESS <<< MTP interface found by class");
            return true;
        }
    }

    cJSON* name = cJSON_GetObjectItemCaseSensitive(iface, "name");
    if (name != nullptr && cJSON_IsString(name) && name->valuestring) {
        const std::string ifaceName = ToLowerString(name->valuestring);
        if (ifaceName.find("mtp") != std::string::npos) {
            isPtp_ = false;
            LOGD("[L2:UsbEventSubscriber] CheckMtpInterface: <<< EXIT SUCCESS <<< MTP interface found by name");
            return true;
        }
        if (ifaceName.find("ptp") != std::string::npos) {
            isPtp_ = true;
            LOGD("[L2:UsbEventSubscriber] CheckMtpInterface: <<< EXIT SUCCESS <<< PTP interface found by name");
            return true;
        }
    }
    LOGD("[L2:UsbEventSubscriber] CheckMtpInterface: <<< EXIT SUCCESS <<< not MTP interface");
    return false;
}

bool UsbEventSubscriber::CheckAllInterfaces(const cJSON* configs)
{
    LOGD("[L2:UsbEventSubscriber] CheckAllInterfaces: >>> ENTER <<<");
    if (configs == nullptr || !cJSON_IsArray(configs)) {
        LOGD("[L2:UsbEventSubscriber] CheckAllInterfaces: <<< EXIT SUCCESS <<< configs is null or not array");
        return false;
    }

    cJSON* config = nullptr;
    cJSON_ArrayForEach(config, configs) {
        cJSON* name = cJSON_GetObjectItemCaseSensitive(config, "name");
        if (name != nullptr && cJSON_IsString(name) && name->valuestring) {
            const std::string configName = ToLowerString(name->valuestring);
            if (configName.find("mtp") != std::string::npos) {
                isPtp_ = false;
                LOGD("[L2:UsbEventSubscriber] CheckAllInterfaces: <<< EXIT SUCCESS <<< MTP config found");
                return true;
            }
            if (configName.find("ptp") != std::string::npos) {
                isPtp_ = true;
                LOGD("[L2:UsbEventSubscriber] CheckAllInterfaces: <<< EXIT SUCCESS <<< PTP config found");
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
                LOGD("[L2:UsbEventSubscriber] CheckAllInterfaces: <<< EXIT SUCCESS <<< MTP interface found");
                return true;
            }
        }
    }
    LOGD("[L2:UsbEventSubscriber] CheckAllInterfaces: <<< EXIT SUCCESS <<< no MTP interface found");
    return false;
}

bool UsbEventSubscriber::IsMTPDevice(const std::string &usbInfo)
{
    LOGI("[L2:UsbEventSubscriber] IsMTPDevice: >>> ENTER <<<");
    cJSON *usbJson = cJSON_Parse(usbInfo.c_str());
    if (usbJson == nullptr) {
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT FAILED <<< parse json failed");
        return false;
    }

    cJSON* productName = cJSON_GetObjectItemCaseSensitive(usbJson, "productName");
    if (productName && cJSON_IsString(productName) && productName->valuestring) {
        std::string lowerName = ToLowerString(productName->valuestring);
        if ((lowerName.find("mtp") != std::string::npos || lowerName.find("ptp") != std::string::npos)) {
            cJSON_Delete(usbJson);
            LOGI("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT SUCCESS <<< MTP/PTP device found by product name");
            return true;
        }
    }

    cJSON* configs = cJSON_GetObjectItemCaseSensitive(usbJson, "configs");
    if (CheckAllInterfaces(configs)) {
        cJSON_Delete(usbJson);
        LOGI("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT SUCCESS <<< MTP device found by configs");
        return true;
    }

    cJSON *classObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_CLASS_KEY);
    if (classObj == nullptr || !cJSON_IsNumber(classObj)) {
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: parse deviceClass failed");
        cJSON_Delete(usbJson);
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT FAILED <<< deviceClass not found");
        return false;
    }
    uint8_t deviceClass = static_cast<uint8_t>(classObj->valueint);
    cJSON *vendorObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_VENDOR_ID_KEY);
    if (vendorObj == nullptr || !cJSON_IsNumber(vendorObj)) {
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: parse vendorObj failed");
        cJSON_Delete(usbJson);
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT FAILED <<< vendorId not found");
        return false;
    }
    uint16_t idVendor = static_cast<uint16_t>(vendorObj->valueint);
    cJSON *productObj = cJSON_GetObjectItemCaseSensitive(usbJson, DEV_PRODUCT_ID_KEY);
    if (productObj == nullptr || !cJSON_IsNumber(productObj)) {
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: parse productObj failed");
        cJSON_Delete(usbJson);
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT FAILED <<< productId not found");
        return false;
    }
    uint16_t idProduct = static_cast<uint16_t>(productObj->valueint);
    cJSON_Delete(usbJson);
    if (LIBMTP_check_is_mtp_device(deviceClass, idVendor, idProduct)) {
        LOGE("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT SUCCESS <<< MTP device found by LIBMTP check");
        return true;
    }
    LOGE("[L2:UsbEventSubscriber] IsMTPDevice: <<< EXIT SUCCESS <<< not MTP device");
    return false;
}

bool UsbEventSubscriber::IsPtpMode()
{
    LOGI("[L2:UsbEventSubscriber] IsPtpMode: >>> ENTER <<<");
    bool isPtp = isPtp_.load();
    LOGD("[L2:UsbEventSubscriber] IsPtpMode: <<< EXIT SUCCESS <<< isPtp=%{public}d", isPtp);
    return isPtp;
}
}  // namespace StorageDaemon
}  // namespace OHOS