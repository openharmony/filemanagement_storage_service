/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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
#ifdef SUPPORT_OPEN_SOURCE_GPHOTO2_DEVICE
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-log.h>
#endif
#include <iostream>
#include <libmtp.h>
#include <libusb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <thread>
#include <unistd.h>
#include "mtp/usb_event_subscriber.h"
#include "parameter.h"
#include "parameters.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_radar.h"
#include "usb_srv_client.h"
#include "utils/file_utils.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t SLEEP_TIME = 1;
constexpr int32_t MTP_VAL_LEN = 6;
constexpr int32_t MTP_TRUE_LEN = 5;
constexpr int32_t DETECT_CNT = 4;
constexpr int UPLOAD_RECORD_FALSE_LEN = 5;
constexpr int UPLOAD_RECORD_TRUE_LEN = 4;
constexpr int32_t BUF_SIZE_512 = 512;

constexpr const char *MTP_ROOT_PATH = "/mnt/data/external/";
constexpr const char *SYS_PARAM_SERVICE_PERSIST_ENABLE = "persist.edm.mtp_client_disable";
constexpr const char *SYS_PARAM_SERVICE_ENTERPRISE_ENABLE = "const.edm.is_enterprise_device";
constexpr const char *KEY_CUST = "const.cust.custPath";
constexpr const char *CUST_HWIT = "hwit";
constexpr const char *IS_PTP_MODE = "user.isptpmode";
constexpr const char *IS_OPEN_HARMONY = "user.isOpenHarmonyMtpDevice";
constexpr const char *GET_FRIENDLY_NAME = "user.getfriendlyname";
constexpr int32_t SYS_PARARMETER_SIZE = 256;
#ifdef SUPPORT_OPEN_SOURCE_GPHOTO2_DEVICE
constexpr int32_t SCANF_NUM = 2;
constexpr int32_t ERROR_CODE_PARSE_FAILED = -1;
constexpr int32_t ERROR_CODE_LIBUSB_INIT_FAILED = -2;
constexpr int32_t ERROR_CODE_GET_DEVICE_LIST_FAILED = -3;
#endif
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
        if (!uuids.empty()) {
            devInfo.uuid = uuids.front();
        }
        devInfo.devNum = rawDevice->devnum;
        devInfo.busLocation = rawDevice->bus_location;
        devInfo.vendor = rawDevice->device_entry.vendor;
        devInfo.product = rawDevice->device_entry.product;
        devInfo.vendorId = rawDevice->device_entry.vendor_id;
        devInfo.productId = rawDevice->device_entry.product_id;
        devInfo.id = "mtp-" + std::to_string(devInfo.vendorId) + "-" + std::to_string(devInfo.productId);
        devInfo.path = std::string(MTP_ROOT_PATH) + devInfo.id;
        devInfo.type = "mtpfs";
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
#ifdef SUPPORT_OPEN_SOURCE_GPHOTO2_DEVICE
        ret = GetGphotoDevices(devInfos);
        if (ret != E_OK) {
            LOGE("No GetGphotoDevices devices detected");
            return;
        }
#endif
        LOGE("No devices detected %{public}d", ret);
    }
    ret = MountMtpDevice(devInfos);
    bool isPtp = UsbEventSubscriber::IsPtpMode();
    std::string extraData = "{Protocol:" + std::string(isPtp ? "PTP" : "MTP") +
        "}{Result:" + std::to_string(ret) + "}";
    if (ret != E_OK) {
        LOGE("mount mtp device failed.");
        const auto& firstDev = devInfos[0];
        extraData += "{Vendor:" + firstDev.vendor + "}{Product:" + firstDev.product +
            "}{Name:unknown}{OpenHarmony:unknown}";
        StorageService::StorageRadar::ReportMtpResult("MountMtpDeviceInfo", E_MTP_MOUNT_FAILED, extraData);
        return;
    }
    for (const auto& devInfo : devInfos) {
        const char* value = isPtp ? "true" : "false";
        size_t ptpLen = isPtp ? UPLOAD_RECORD_TRUE_LEN : UPLOAD_RECORD_FALSE_LEN;
        if (setxattr(devInfo.path.c_str(), IS_PTP_MODE, value, ptpLen, 0) == -1) {
            LOGE("Failed to set PTP mode xattr");
        }

        std::string isOpenHarmony = "unknown";
        std::string devName = "unknown";
        char tempBuffer[BUF_SIZE_512] = {0};

        int32_t attrLen = getxattr(devInfo.path.c_str(), IS_OPEN_HARMONY, tempBuffer, BUF_SIZE_512);
        if (attrLen >= 0 && attrLen < BUF_SIZE_512) {
            tempBuffer[attrLen] = '\0';
            isOpenHarmony = tempBuffer;
        }

        memset_s(tempBuffer, sizeof(tempBuffer), 0, sizeof(tempBuffer));
        attrLen = getxattr(devInfo.path.c_str(), GET_FRIENDLY_NAME, tempBuffer, BUF_SIZE_512);
        if (attrLen >= 0 && attrLen < BUF_SIZE_512) {
            tempBuffer[attrLen] = '\0';
            devName = tempBuffer;
        }

        extraData += "{Vendor:" + devInfo.vendor + "}{Product:" + devInfo.product +
            "}{Name:" + devName + "}{OpenHarmony:" + isOpenHarmony + "}";
    }
    StorageService::StorageRadar::ReportMtpResult("MountMtpDeviceInfo", E_MTP_MOUNT_FAILED, extraData);
}

void MtpDeviceMonitor::MonitorDevice()
{
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
        return E_NOT_SUPPORT;
    }
    for (auto device : monitorDevices) {
        if (HasMounted(device)) {
            LOGE("MountMtpDevice: mtp device has mounted.");
            continue;
        }

        if (lastestMtpDevList_.size() > 0) {
            LOGW("Multiple devices detected. Only one device is supported. Ignoring additional devices.");
            return E_MTP_MOUNT_FAILED;
        }
        int32_t ret = MtpDeviceManager::GetInstance().MountDevice(device);
        if (ret == E_OK) {
            lastestMtpDevList_.push_back(device);
            LOGI("lastestMtpDevList_.push_back.");
        } else if (ret == E_MTP_IS_MOUNTING) {
            return E_OK;
        } else {
            return ret;
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
    
    auto newEnd = std::remove_if(lastestMtpDevList_.begin(), lastestMtpDevList_.end(),
        [devNum, busLoc, this](const MtpDeviceInfo& device) {
            if (device.devNum == devNum && device.busLocation == busLoc) {
                int32_t ret = MtpDeviceManager::GetInstance().UmountDevice(device, true, true);
                if (ret == E_OK) {
                    LOGI("Successfully unmounted device, path=%{public}s", device.path.c_str());
                    return true;
                } else {
                    LOGE("Umount mtp device failed, path=%{public}s, ret=%{public}d", device.path.c_str(), ret);
                    StorageService::StorageRadar::ReportMtpResult("UmountDetachedMtpDevice::UmountDevice", ret, "NA");
                    return false;
                }
            }
            return false;
        });
    lastestMtpDevList_.erase(newEnd, lastestMtpDevList_.end());
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

    for (auto iter = lastestMtpDevList_.begin(); iter != lastestMtpDevList_.end();) {
        if (iter->id != id) {
            ++iter;
            continue;
        }

        int32_t ret = MtpDeviceManager::GetInstance().UmountDevice(*iter, true, false);
        if (ret == E_OK) {
            iter = lastestMtpDevList_.erase(iter);
        } else {
            LOGE("Umount mtp device failed, path=%{public}s", iter->path.c_str());
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

#ifdef SUPPORT_OPEN_SOURCE_GPHOTO2_DEVICE
static bool FindLineValue(const char* sourceText, const char* targetKey, char* outputBuffer, size_t bufferSize)
{
    if (!sourceText || !targetKey || !outputBuffer || bufferSize == 0) {
        return false;
    }

    const char* keyPosition = strstr(sourceText, targetKey);
    if (!keyPosition) {
        return false;
    }

    keyPosition += strlen(targetKey);
    while (*keyPosition == ' ' || *keyPosition == '\t') {
        keyPosition++;
    }
    const char* lineEnd = strpbrk(keyPosition, "\r\n");
    if (lineEnd == nullptr) {
        return false;
    }
    size_t valueLength = static_cast<size_t>(lineEnd - keyPosition);
    if (valueLength >= bufferSize) {
        valueLength = bufferSize - 1;
    }

    int copyResult = memcpy_s(outputBuffer, bufferSize, keyPosition, valueLength);
    if (copyResult != EOK) {
        LOGE("memcpy failed %{public}d", copyResult);
    }
    outputBuffer[valueLength] = '\0';
    return true;
}

static bool ParsePortPath(const char* portPath, int &bus, int &dev)
{
    if (!portPath) {
        return false;
    }
    if (sscanf_s(portPath, "usb:%d,%d", &bus, &dev) == SCANF_NUM) {
        return true;
    }
    return false;
}

static int GetVidPidFromPortPath(const char* portPath, uint16_t &vid, uint16_t &pid)
{
    int bus = 0;
    int dev = 0;
    if (!ParsePortPath(portPath, bus, dev)) {
        return ERROR_CODE_PARSE_FAILED;
    }
    libusb_context *ctx = nullptr;
    if (libusb_init(&ctx) != 0) {
        return ERROR_CODE_LIBUSB_INIT_FAILED;
    }
    libusb_device **list = nullptr;
    ssize_t n = libusb_get_device_list(ctx, &list);
    if (n < 0) {
        libusb_exit(ctx);
        return ERROR_CODE_GET_DEVICE_LIST_FAILED;
    }
    int ret = -1;
    for (ssize_t i = 0; i < n; i++) {
        libusb_device *d = list[i];
        if (libusb_get_bus_number(d) == bus &&
            libusb_get_device_address(d) == dev) {
            libusb_device_descriptor desc;
            if (libusb_get_device_descriptor(d, &desc) == 0) {
                vid = desc.idVendor;
                pid = desc.idProduct;
                ret = 0;
            }
            break;
        }
    }
    libusb_free_device_list(list, 1);
    libusb_exit(ctx);
    return ret;
}

static void CleanupGphotoResources(GPContext *context, CameraList* list, Camera *camera)
{
    if (camera) {
        gp_camera_exit(camera, context);
        gp_camera_free(camera);
        camera = NULL;
    }
    if (list) {
        gp_list_free(list);
    }
    if (context) {
        gp_context_unref(context);
    }
}

static void GenerateGphotoDeviceInfo(MtpDeviceInfo& devInfo, const char* portPath,
                                     Camera* camera, GPContext* context)
{
    std::vector<std::string> cmd = {
        "cat",
        "/proc/sys/kernel/random/uuid",
    };
    std::vector<std::string> uuids;
    ForkExec(cmd, &uuids);

    int bus = 0;
    int dev = 0;
    if (!ParsePortPath(portPath, bus, dev)) {
        LOGE("ParsePortPath failed.");
        return;
    }
    devInfo.busLocation = bus;
    devInfo.devNum = dev;

    CameraText summary;
    int ret = gp_camera_get_summary(camera, &summary, context);
    if (ret >= GP_OK) {
        char model[256];
        if (FindLineValue(summary.text, "Model:", model, sizeof(model))) {
            devInfo.vendor = model;
            devInfo.product = model;
        }
    }

    uint16_t vid = 0;
    uint16_t pid = 0;
    if (GetVidPidFromPortPath(portPath, vid, pid) != E_OK) {
        return;
    }
    devInfo.vendorId = vid;
    devInfo.productId = pid;
    devInfo.id = "mtp-" + std::to_string(devInfo.vendorId) + "-" + std::to_string(devInfo.productId);
    devInfo.uuid = devInfo.id;
    devInfo.path = std::string(MTP_ROOT_PATH) + devInfo.id;
    devInfo.type = "gphotofs";

    LOGI("Detect new mtp device: vendorId=%{public}u, productId=%{public}u, id=%{public}s, path=%{public}s",
        devInfo.vendorId, devInfo.productId, (devInfo.id).c_str(), (devInfo.path).c_str());
}

static int32_t InitGphoto(GPContext* &context, CameraList* &list, Camera* &camera)
{
    context = gp_context_new();
    if (!context) {
        LOGI("context nullptr.");
        CleanupGphotoResources(context, list, camera);
        return E_MTP_MOUNT_FAILED;
    }

    int ret = gp_list_new(&list);
    if (ret < GP_OK || !list) {
        LOGE("gp_list_new failed, ret:%{public}d.", ret);
        CleanupGphotoResources(context, list, camera);
        return E_MTP_MOUNT_FAILED;
    }

    ret = gp_camera_autodetect(list, context);
    if (ret < GP_OK) {
        LOGE("gp_camera_autodetect failed, ret:%{public}d.", ret);
        CleanupGphotoResources(context, list, camera);
        return E_MTP_MOUNT_FAILED;
    }

    ret = gp_camera_new(&camera);
    if (ret < GP_OK || !camera) {
        LOGE("gp_camera_new failed ret:%{public}d.", ret);
        CleanupGphotoResources(context, list, camera);
        return E_MTP_MOUNT_FAILED;
    }

    ret = gp_camera_init(camera, context);
    if (ret < GP_OK) {
        LOGE("gp_camera_init failed ret:%{public}d.", ret);
        CleanupGphotoResources(context, list, camera);
        return E_MTP_MOUNT_FAILED;
    }
    return E_OK;
}

int32_t MtpDeviceMonitor::GetGphotoDevices(std::vector<MtpDeviceInfo> &devInfos)
{
    GPContext *context = nullptr;
    CameraList *list = nullptr;
    Camera *camera = nullptr;
    if (InitGphoto(context, list, camera) != E_OK) {
        LOGE("InitGphoto failed.");
        return E_MTP_MOUNT_FAILED;
    }

    const char *model = nullptr;
    const char *portPath = nullptr;

    int ret = gp_list_get_name(list, 0, &model);
    if (ret < GP_OK || !model) {
        LOGE("gp_list_get_name failed ret:%{public}d.", ret);
        CleanupGphotoResources(context, list, nullptr);
        return E_MTP_MOUNT_FAILED;
    }

    ret = gp_list_get_value(list, 0, &portPath);
    if (ret < GP_OK || !portPath) {
        LOGE("gp_list_get_value failed ret:%{public}d.", ret);
        CleanupGphotoResources(context, list, nullptr);
        return E_MTP_MOUNT_FAILED;
    }

    LOGI("model=%{public}s port=%{public}s", model, portPath);

    MtpDeviceInfo devInfo;
    GenerateGphotoDeviceInfo(devInfo, portPath, camera, context);
    devInfos.push_back(devInfo);

    LOGI("free camera.");
    CleanupGphotoResources(context, list, camera);
    return E_OK;
}
#endif
}  // namespace StorageDaemon
}  // namespace OHOS