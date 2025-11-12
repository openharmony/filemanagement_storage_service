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

#include "mtp/mtp_device_monitor.h"

#include <cstdio>
#include <dirent.h>
#include <filesystem>
#include <iostream>
#include <libmtp.h>
#include "parameter.h"
#include "parameters.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <thread>
#include "mtp/usb_event_subscriber.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "usb_srv_client.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t SLEEP_TIME = 1;
constexpr int32_t MTP_VAL_LEN = 6;
constexpr int32_t MTP_TRUE_LEN = 5;
constexpr int32_t DETECT_CNT = 4;
constexpr int UPLOAD_RECORD_FALSE_LEN = 5;
constexpr int UPLOAD_RECORD_TRUE_LEN = 4;

constexpr const char *MTP_ROOT_PATH = "/mnt/data/external/";
constexpr const char *SYS_PARAM_SERVICE_PERSIST_ENABLE = "persist.edm.mtp_client_disable";
constexpr const char *SYS_PARAM_SERVICE_ENTERPRISE_ENABLE = "const.edm.is_enterprise_device";
constexpr const char *KEY_CUST = "const.cust.custPath";
constexpr const char *CUST_HWIT = "hwit";
constexpr const char *IS_PYP_MODE = "user.isptpmode";
constexpr int32_t SYS_PARARMETER_SIZE = 256;
bool g_keepMonitoring = true;

MtpDeviceMonitor::MtpDeviceMonitor() {}

MtpDeviceMonitor::~MtpDeviceMonitor()
{
    LOGI("MtpDeviceMonitor Destructor.");
    UmountAllMtpDevice();
}

void MtpDeviceMonitor::StartMonitor()
{
    LOGI("MtpDeviceMonitor, start mtp device monitor.");
    if (IsHwitDevice()) {
        LOGE("the vendor country of device is hwit/cn.");
        return;
    }
    std::thread([this]() { MonitorDevice(); }).detach();
}

bool MtpDeviceMonitor::IsNeedDisableMtp()
{
    char mtpEnterpriseEnable[MTP_VAL_LEN + 1] = {"false"};
    int retEn = GetParameter(SYS_PARAM_SERVICE_ENTERPRISE_ENABLE, "", mtpEnterpriseEnable, MTP_VAL_LEN);
    LOGI("GetParameter mtpEnterpriseEnable %{public}s, retEnterprise %{public}d", mtpEnterpriseEnable, retEn);
    char mtpEnable[MTP_VAL_LEN + 1] = {"false"};
    int ret = GetParameter(SYS_PARAM_SERVICE_PERSIST_ENABLE, "", mtpEnable, MTP_VAL_LEN);
    LOGI("GetParameter mtpEnable %{public}s, ret %{public}d", mtpEnable, ret);
    if (strncmp(mtpEnterpriseEnable, "true", MTP_TRUE_LEN) == 0 && strncmp(mtpEnable, "true", MTP_TRUE_LEN) == 0) {
        return true;
    }
    return false;
}

int32_t MtpDeviceMonitor::GetMtpDevices(std::vector<MtpDeviceInfo> &devInfos)
{
    int rawDevSize = 0;
    LIBMTP_raw_device_t *rawDevices = nullptr;
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawDevices, &rawDevSize);
    if ((err == LIBMTP_ERROR_NO_DEVICE_ATTACHED) || (rawDevices == nullptr) || (rawDevSize <= 0)) {
        if (rawDevices != nullptr) {
            free(static_cast<void *>(rawDevices));
        }
        return E_MTP_MOUNT_FAILED;
    }

    for (int index = 0; index < rawDevSize; ++index) {
        LIBMTP_raw_device_t *rawDevice = &rawDevices[index];
        if (rawDevice == nullptr) {
            LOGE("MonitorDevice: rawDevice is nullptr.");
            continue;
        }
        std::vector<std::string> cmd = {
            "cat",
            "/proc/sys/kernel/random/uuid",
        };
        std::vector<std::string> uuids;
        ForkExec(cmd, &uuids);

        MtpDeviceInfo devInfo;
        devInfo.uuid = uuids.front();
        devInfo.devNum = rawDevice->devnum;
        devInfo.busLocation = rawDevice->bus_location;
        devInfo.vendor = rawDevice->device_entry.vendor;
        devInfo.product = rawDevice->device_entry.product;
        devInfo.vendorId = rawDevice->device_entry.vendor_id;
        devInfo.productId = rawDevice->device_entry.product_id;
        devInfo.id = "mtp-" + std::to_string(devInfo.vendorId) + "-" + std::to_string(devInfo.productId);
        devInfo.path = std::string(MTP_ROOT_PATH) + devInfo.id;
        devInfos.push_back(devInfo);
        LOGI("Detect new mtp device: id=%{public}s, vendor=%{public}s, product=%{public}s, devNum=%{public}d",
            (devInfo.id).c_str(), (devInfo.vendor).c_str(), (devInfo.product).c_str(), devInfo.devNum);
    }
    free(static_cast<void *>(rawDevices));
    return E_OK;
}

void MtpDeviceMonitor::MountMtpDeviceByBroadcast()
{
    std::vector<MtpDeviceInfo> devInfos;
    int32_t ret = GetMtpDevices(devInfos);
    if (ret != E_OK) {
        LOGE("No MTP devices detected");
        return;
    }
    ret = MountMtpDevice(devInfos);
    if (ret != E_OK) {
        LOGE("mount mtp device failed");
        return;
    }
    bool isPtp = UsbEventSubscriber::IsPtpMode();
 
    for (const auto& devInfo : devInfos) {
        const char* value = isPtp ? "true" : "false";
        size_t len = isPtp ? UPLOAD_RECORD_TRUE_LEN : UPLOAD_RECORD_FALSE_LEN;
        if (setxattr(devInfo.path.c_str(), IS_PYP_MODE, value, len, 0) == -1) {
            LOGE("Failed to set xattr");
        }
    }
}

void MtpDeviceMonitor::MonitorDevice()
{
    LOGI("MonitorDevice: mtp device monitor thread begin.");
    int32_t cnt = DETECT_CNT;
    while (cnt > 0) {
        bool hasMtp = false;
        if (HasMTPDevice(hasMtp) != E_OK) {
            cnt--;
            sleep(SLEEP_TIME);
            continue;
        }
        if (hasMtp) {
            MountMtpDeviceByBroadcast();
            break;
        }
        cnt--;
        sleep(SLEEP_TIME);
    }
    RegisterMTPParamListener();
    while (g_keepMonitoring) {
        sleep(SLEEP_TIME);
        UsbEventSubscriber::SubscribeCommonEvent();
    }
    RemoveMTPParamListener();
    LOGI("MonitorDevice: mtp device monitor thread end.");
}

int32_t MtpDeviceMonitor::MountMtpDevice(const std::vector<MtpDeviceInfo> &monitorDevices)
{
    if (IsNeedDisableMtp()) {
        LOGE("This device cannot support MTP.");
        return E_MTP_MOUNT_FAILED;
    }
    LOGI("MountMtpDevice: start mount mtp device.");
    for (auto device : monitorDevices) {
        if (HasMounted(device)) {
            LOGI("MountMtpDevice: mtp device has mounted.");
            continue;
        }

        if (lastestMtpDevList_.size() > 0) {
            LOGW("Multiple devices detected. Only one device is supported. Ignoring additional devices.");
            return E_MTP_MOUNT_FAILED;
        }
        int32_t ret = MtpDeviceManager::GetInstance().MountDevice(device);
        if (ret == E_OK) {
            lastestMtpDevList_.push_back(device);
        } else if (ret == E_MTP_IS_MOUNTING) {
            return E_OK;
        } else {
            return E_MTP_MOUNT_FAILED;
        }
    }
    return E_OK;
}

bool MtpDeviceMonitor::HasMounted(const MtpDeviceInfo &device)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    for (auto iter = lastestMtpDevList_.begin(); iter != lastestMtpDevList_.end(); iter++) {
        if (iter != lastestMtpDevList_.end() && (iter->id == device.id)) {
            return true;
        }
    }
    return false;
}

void MtpDeviceMonitor::UmountAllMtpDevice()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    for (auto iter = lastestMtpDevList_.begin(); iter != lastestMtpDevList_.end(); iter++) {
        int32_t ret = MtpDeviceManager::GetInstance().UmountDevice(*iter, true, false);
        if (ret != E_OK) {
            LOGE("UmountAllMtpDevice: umount mtp device failed, path=%{public}s", (iter->path).c_str());
        }
    }
    lastestMtpDevList_.clear();
}

void MtpDeviceMonitor::UmountDetachedMtpDevice(uint8_t devNum, uint32_t busLoc)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    LOGI("MtpDeviceMonitor::Umount detached mtp device, devNum=%{public}d, busLoc=%{public}d.", devNum, busLoc);

    for (auto iter = lastestMtpDevList_.begin(); iter != lastestMtpDevList_.end();) {
        LOGI("Mtp device mount path=%{public}s is not exist or removed, umount it.", (iter->path).c_str());
        if (iter->devNum != devNum || iter->busLocation != busLoc) {
            LOGI("not this mtp device");
            return;
        }
        int32_t ret = MtpDeviceManager::GetInstance().UmountDevice(*iter, true, true);
        if (ret == E_OK) {
            iter = lastestMtpDevList_.erase(iter);
        } else {
            LOGE("Umount mtp device failed, path=%{public}s", (iter->path).c_str());
            iter++;
        }
    }
}

int32_t MtpDeviceMonitor::Mount(const std::string &id)
{
    LOGI("MtpDeviceMonitor: start mount mtp device by id=%{public}s", id.c_str());
    std::lock_guard<std::mutex> lock(listMutex_);
    for (auto iter = lastestMtpDevList_.begin(); iter != lastestMtpDevList_.end(); iter++) {
        if (iter->id != id) {
            continue;
        }
        int32_t ret = MtpDeviceManager::GetInstance().MountDevice(*iter);
        if (ret != E_OK) {
            LOGE("MountMtpDevice: mtp device mount failed.");
        }
        return ret;
    }
    LOGE("the volume id %{public}s does not exist.", id.c_str());
    return E_NON_EXIST;
}

int32_t MtpDeviceMonitor::Umount(const std::string &id)
{
    LOGI("MtpDeviceMonitor: start umount mtp device by id=%{public}s", id.c_str());
    std::lock_guard<std::mutex> lock(listMutex_);
    for (auto iter = lastestMtpDevList_.begin(); iter != lastestMtpDevList_.end(); iter++) {
        if (iter->id != id) {
            continue;
        }
        int32_t ret = MtpDeviceManager::GetInstance().UmountDevice(*iter, true, false);
        if (ret == E_OK) {
            lastestMtpDevList_.erase(iter);
        } else {
            LOGE("Umount mtp device failed, path=%{public}s", (iter->path).c_str());
        }
        return ret;
    }
    LOGE("the volume id %{public}s does not exist.", id.c_str());
    return E_NON_EXIST;
}

bool MtpDeviceMonitor::IsHwitDevice()
{
    char param[SYS_PARARMETER_SIZE] = {0};
    int errorCode = GetParameter(KEY_CUST, "", param, SYS_PARARMETER_SIZE);
    if (errorCode <= 0) {
        LOGE("get vendor country fail, errorCode:%{public}d", errorCode);
        return false;
    }
    LOGI("vendor counry: %{public}s, errorCode: %{public}d", param, errorCode);
    if (std::string(param).find(CUST_HWIT) != std::string::npos) {
        return true;
    }
    return false;
}

void MtpDeviceMonitor::RegisterMTPParamListener()
{
    LOGI("RegisterMTPParamListener");
    WatchParameter(SYS_PARAM_SERVICE_PERSIST_ENABLE, OnMtpDisableParamChange, this);
    WatchParameter(SYS_PARAM_SERVICE_ENTERPRISE_ENABLE, OnEnterpriseParamChange, this);
}

void MtpDeviceMonitor::RemoveMTPParamListener()
{
    LOGI("RemoveMTPParamListener");
    RemoveParameterWatcher(SYS_PARAM_SERVICE_PERSIST_ENABLE, OnMtpDisableParamChange, this);
    RemoveParameterWatcher(SYS_PARAM_SERVICE_ENTERPRISE_ENABLE, OnEnterpriseParamChange, this);
}

void MtpDeviceMonitor::OnMtpDisableParamChange(const char *key, const  char *value, void *context)
{
    if (key == nullptr || value == nullptr || context == nullptr) {
        LOGE("OnMtpDisableParamChange return invaild value");
        return;
    }
    LOGI("OnMtpDisableParamChange key = %{public}s, value = %{public}s,",  key, value);
    if (strcmp(key, SYS_PARAM_SERVICE_PERSIST_ENABLE) != 0) {
        LOGE("event key mismatch");
        return;
    }
    MtpDeviceMonitor* instance = reinterpret_cast<MtpDeviceMonitor*>(context);
    if (instance == nullptr) {
        LOGE("Converted context is null");
        return;
    }
    if (instance->IsNeedDisableMtp()) {
        LOGI("MTP disable parameter changed, unmount all mtp devices.");
        instance->UmountAllMtpDevice();
    }
}

void MtpDeviceMonitor::OnEnterpriseParamChange(const char *key, const  char *value, void *context)
{
    if (key == nullptr || value == nullptr || context == nullptr) {
        LOGE("OnEnterpriseParamChange return invaild value");
        return;
    }
    LOGI("OnEnterpriseParamChange key = %{public}s, value = %{public}s,",  key, value);
    if (strcmp(key, SYS_PARAM_SERVICE_ENTERPRISE_ENABLE) != 0) {
        LOGE("event key mismatch");
        return;
    }
    MtpDeviceMonitor* instance = reinterpret_cast<MtpDeviceMonitor*>(context);
    if (instance == nullptr) {
        LOGE("Converted context is null");
        return;
    }
    if (instance->IsNeedDisableMtp()) {
        LOGI("Enterprise device parameter changed, unmount all mtp devices.");
        instance->UmountAllMtpDevice();
    }
}

int32_t MtpDeviceMonitor::HasMTPDevice(bool &hasMtp)
{
    auto &usbSrvClient = OHOS::USB::UsbSrvClient::GetInstance();
    std::vector<UsbDevice> deviceList;
    int32_t ret = usbSrvClient.GetDevices(deviceList);
    if (ret != E_OK) {
        LOGE("USB GetDevices failed, ret is %{public}d.", ret);
        return ret;
    }
    for (UsbDevice &dev : deviceList) {
        uint8_t deviceClass = static_cast<uint8_t>(dev.GetClass());
        uint16_t idVendor = static_cast<uint16_t>(dev.GetVendorId());
        uint16_t idProduct = static_cast<uint16_t>(dev.GetProductId());
        LOGI("device class is %{public}d, vendor id is %{public}d, product id is %{public}d.",
             deviceClass, idVendor, idProduct);
        if (LIBMTP_check_is_mtp_device(deviceClass, idVendor, idProduct)) {
            LOGE("this is mtp device.");
            hasMtp = true;
            return E_OK;
        }
    }
    LOGI("there has no mtp device.");
    hasMtp = false;
    return E_OK;
}
}  // namespace StorageDaemon
}  // namespace OHOS