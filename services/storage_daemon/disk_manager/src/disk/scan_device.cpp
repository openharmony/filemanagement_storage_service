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

#include "disk_manager/disk/scan_device.h"

#include "storage_service_log.h"
#include <charconv>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <linux/hdreg.h>
#include <regex>
#include <algorithm>
#include <cctype>
#include <sys/ioctl.h>
#include <unistd.h>

namespace OHOS {
namespace StorageDaemon {
constexpr const char *ZERO_STRING = "0";
constexpr const char *ONE_STRING = "1";
constexpr const char *SPLIT_STRING = "/";
constexpr const char *DEVICE_NUMBER_SPLIT = ":";
constexpr const char *REMOVABLE_NODE = "/removable";
constexpr const char *SD_STRING = "sd";
constexpr const char *UFS_STRING = "ufs";
constexpr const char *NVME_STRING = "nvme";
constexpr const char *NVME0_STRING = "nvme0";
constexpr const char *SIZE_NODE = "/size";
constexpr const char *DEV_PATH = "/dev/block";
constexpr const char *DEV_NODE = "/dev";
constexpr const char *MODEL_NODE = "/device/model";
constexpr const char *VENDOR_NODE = "/device/vendor";
constexpr const char *UEVENT_NODE = "/uevent";
constexpr const char *MAJOR_KEY = "MAJOR=";
constexpr const char *MINOR_KEY = "MINOR=";
constexpr const char *SERIAL_NODE = "/device/serial";
constexpr const char *ATA_PREFIX = "ata";
constexpr const char *NVME_PORT_PATTERN = "nvme([0-9]+)";
constexpr const char *NVME_PREFIX = "nvme";
constexpr const char *DISK_ID_PREFIX = "disk-";
constexpr const char *DISK_ID_CONTACT = "-";
constexpr const char *ATA_PORT_PATTERN = "ata([0-9]+)";

const uint64_t SECTOR_SIZE = 512;
const int NUMBER_BASE = 10;
const size_t NVME_BASE_LEN = 4;
const char NVME_LINK = 'n';
constexpr size_t HD_DRIVEID_SIZE = 512;
constexpr uint16_t RPM_ARRAY_INDEX = 11;
constexpr size_t NVME_MIN_NAME_LEN = 7;

ScanDevice::ScanDevice(const std::string &sysBlockPath, const std::string &devBlockPath)
    : sysBlockPath(sysBlockPath), devBlockPath(DEV_PATH)
{
}

using json = nlohmann::json;

json BlockInfo::ToJson() const
{
    return json{
        {"sizeBytes", sizeBytes},
        {"vendor", vendor},
        {"model", model},
        {"interfaceType", interfaceType},
        {"rpm", rpm},
        {"removable", removable},
        {"serialNumber", serialNumber},
        {"diskId", diskId},
        {"devicePath", devicePath},
        {"port", port}
    };
}

std::string BlockInfo::SerializeVector(const std::vector<BlockInfo>& infos)
{
    json j = json::array();
    for (const auto& info : infos) {
        j.push_back(info.ToJson());
    }
    return j.dump();
}


bool ScanDevice::ReadRemovableNode(const std::string &deviceName, bool &isRemovable)
{
    std::string removablePath = sysBlockPath + SPLIT_STRING + deviceName + REMOVABLE_NODE;
    std::string content;
    if (ReadSysfsNode(removablePath, content)) {
        LOGI("Read %{public}s success: %{public}s", removablePath.c_str(), content.c_str());
        if (content == ONE_STRING) {
            isRemovable = true;
            return true;
        }
        if (content == ZERO_STRING) {
            isRemovable = false;
            return true;
        }
    }
    isRemovable = false;
    return false;
}

bool ScanDevice::IsDataDisk(const std::string &deviceName, const bool isNeedCheckUfs, const bool isRemovable)
{
    if (!isNeedCheckUfs) {
        return !isRemovable && (deviceName.find(SD_STRING) == 0);
    }
    std::string devicePath = sysBlockPath + SPLIT_STRING + deviceName;
    char linkTarget[PATH_MAX];
    ssize_t len = readlink(devicePath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    if (len != -1) {
        linkTarget[len] = '\0';
        std::string targetPath(linkTarget);
        if (targetPath.find(UFS_STRING) == std::string::npos) {
            LOGI("%{public}s isn't ufs: %{public}s", devicePath.c_str(), targetPath.c_str());
            return true;
        } else {
            LOGE("%{public}s is ufs: %{public}s", devicePath.c_str(), targetPath.c_str());
            return false;
        }
    }
    LOGE("readlink of %{public}s failed", devicePath.c_str());
    return false;
}

bool ScanDevice::IsValidNvmeDevice(const std::string &deviceName)
{
    if (deviceName.find(NVME0_STRING) == 0) {
        LOGI("Ignore nvme0* node: %{public}s", deviceName.c_str());
        return false;
    }
    if (deviceName.size() < NVME_MIN_NAME_LEN) {
        LOGI("Ignore nvme name too short: %{public}s", deviceName.c_str());
        return false;
    }
    size_t pos = NVME_BASE_LEN;
    if (!std::isdigit(static_cast<unsigned char>(deviceName[pos]))) {
        LOGI("Ignore invalid nvme format: missing controller ID in %{public}s", deviceName.c_str());
        return false;
    }
    while (pos < deviceName.size() && std::isdigit(static_cast<unsigned char>(deviceName[pos]))) {
        ++pos;
    }
    if (pos >= deviceName.size() || deviceName[pos] != NVME_LINK) {
        LOGI("Ignore invalid nvme format: missing 'n' in %{public}s", deviceName.c_str());
        return false;
    }
    ++pos;
    if (pos >= deviceName.size() || !std::isdigit(static_cast<unsigned char>(deviceName[pos]))) {
        LOGI("Ignore invalid nvme format: missing partition ID in %{public}s", deviceName.c_str());
        return false;
    }
    return true;
}

std::vector<BlockInfo> ScanDevice::GetDataDisks()
{
    std::vector<BlockInfo> dataDisks;
    DIR *dir = opendir(sysBlockPath.c_str());
    if (!dir) {
        LOGE("%{public}s open failed", sysBlockPath.c_str());
        return dataDisks;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string deviceName = entry->d_name;
        if (deviceName == "." || deviceName == "..") {
            LOGI("Ignore %{public}s", deviceName.c_str());
            continue;
        }
        bool isSDevice = (deviceName.length() > 0 && deviceName[0] == 's');
        bool isNvmeDevice = (deviceName.find(NVME_STRING) == 0);
        if (!isSDevice && !isNvmeDevice) {
            LOGI("Ignore invalid node: %{public}s", deviceName.c_str());
            continue;
        }

        if (isNvmeDevice && !IsValidNvmeDevice(deviceName)) {
            continue;
        }
        bool isRemovable = false;
        bool isNeedCheckUfs = false;
        if (!ReadRemovableNode(deviceName, isRemovable)) {
            LOGE("Read removable node failed");
            isNeedCheckUfs = true;
        }
        if (isSDevice && !IsDataDisk(deviceName, isNeedCheckUfs, isRemovable)) {
            continue;
        }
        BlockInfo blockInfo;
        blockInfo.removable = isRemovable;
        if (GetBlockInfo(deviceName, isNvmeDevice, blockInfo) == 0) {
            dataDisks.push_back(blockInfo);
        }
    }
    closedir(dir);
    return dataDisks;
}

std::vector<BlockInfo> ScanDevice::GetExternalDisks()
{
    std::vector<BlockInfo> externalDisks;
    return externalDisks;
}

int ScanDevice::GetBlockInfo(const std::string &deviceName, const bool isNvmeDevice, BlockInfo &blockInfo)
{
    blockInfo.sizeBytes = GetDiskSize(deviceName);
    blockInfo.vendor = GetVendor(deviceName);
    blockInfo.model = GetModel(deviceName);
    blockInfo.interfaceType = GetInterfaceType(deviceName);
    blockInfo.rpm = GetDiskRpm(deviceName, isNvmeDevice);
    blockInfo.serialNumber = GetSerialNumber(deviceName, isNvmeDevice);
    blockInfo.diskId = GetDiskId(deviceName, isNvmeDevice);
    std::string pciePath = GetPciePath(deviceName);
    blockInfo.devicePath = GetDevicePath(deviceName);
    blockInfo.port = GetPort(pciePath, isNvmeDevice);
    return 0;
}

namespace {
std::string TrimSpaces(const std::string &str)
{
    auto first = std::find_if(str.begin(), str.end(), [](unsigned char c) {
        return !std::isspace(c);
    });
    auto last = std::find_if(str.rbegin(), str.rend(), [](unsigned char c) {
        return !std::isspace(c);
    });
    if (first == str.end()) {
        return "";
    }
    return std::string(first, last.base());
}
} // namespace

bool ScanDevice::ReadSysfsNode(const std::string &path, std::string &content)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        LOGE("Open %{public}s failed", path.c_str());
        return false;
    }
    std::string rawContent;
    if (std::getline(file, rawContent)) {
        content = TrimSpaces(rawContent);
        file.close();
        return true;
    } else {
        content = "";
        file.close();
        return false;
    }
}

bool ScanDevice::ParseStringToUlongLong(const std::string &str, unsigned long long &result)
{
    if (str.empty()) {
        return false;
    }
    result = 0;
    const char *begin = str.data();
    const char *end = str.data() + str.size();
    auto res = std::from_chars(begin, end, result, NUMBER_BASE);
    if (res.ec != std::errc{} || res.ptr == begin) {
        return false;
    }
    const char *ptr = res.ptr;
    while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) {
        ++ptr;
    }
    if (ptr != end) {
        LOGI("ParseStringToUlongLong: trailing invalid characters in %{public}s", str.c_str());
        return false;
    }
    return true;
}

uint64_t ScanDevice::GetDiskSize(const std::string &deviceName)
{
    std::string sizePath = sysBlockPath + SPLIT_STRING + deviceName + SIZE_NODE;
    std::string content;
    if (!ReadSysfsNode(sizePath, content)) {
        LOGE("GetDiskSize failed: read node of disk size failed");
        return 0;
    }
    if (content.empty()) {
        LOGE("GetDiskSize: empty content");
        return 0;
    }
    unsigned long long sectors = 0;
    if (!ParseStringToUlongLong(content, sectors)) {
        LOGE("GetDiskSize: failed to parse size value from %{public}s", content.c_str());
        return 0;
    }
    if (sectors > UINT64_MAX / SECTOR_SIZE) {
        LOGE("GetDiskSize: used bytes overflow detected for %{public}s", deviceName.c_str());
        return 0;
    }
    uint64_t sectorSize = static_cast<uint64_t>(sectors) * SECTOR_SIZE;
    LOGI("GetDiskSize success: sectors is %{public}s, size is %{public}s", std::to_string(sectors).c_str(),
         std::to_string(sectorSize).c_str());
    return sectorSize;
}

std::string ScanDevice::GetVendor(const std::string &deviceName)
{
    std::string vendorPath = sysBlockPath + SPLIT_STRING + deviceName + VENDOR_NODE;
    std::string content;
    if (ReadSysfsNode(vendorPath, content)) {
        LOGI("GetVendor success: %{public}s", content.c_str());
        return content;
    }
    LOGE("GetVendor failed");
    return "Unknown";
}

std::string ScanDevice::GetModel(const std::string &deviceName)
{
    std::string modelPath = sysBlockPath + SPLIT_STRING + deviceName + MODEL_NODE;
    std::string content;
    if (ReadSysfsNode(modelPath, content)) {
        LOGI("GetModel success: %{public}s", content.c_str());
        return content;
    }
    LOGE("GetModel: read node of model failed");
    return "Unknown";
}

std::string ScanDevice::GetInterfaceType(const std::string &deviceName)
{
    if (deviceName.find(NVME_STRING) == 0) {
        return "NVMe";
    } else if (deviceName[0] == 's') {
        return "SATA";
    }
    LOGE("Get interface type failed");
    return "Unknown";
}

uint32_t ScanDevice::GetDiskRpm(const std::string &deviceName, const bool isNvmeDevice)
{
    if (isNvmeDevice) {
        return 0;
    }
    std::string devicePath = devBlockPath + SPLIT_STRING + deviceName;
    int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        LOGE("GetDiskRpm failed: open %{public}s failed", devicePath.c_str());
        return 0;
    }
    struct hd_driveid hdId;
    if (ioctl(fd, HDIO_GET_IDENTITY, &hdId) < 0) {
        LOGE("GetDiskRpm failed: execute ioctl failed");
        close(fd);
        return 0;
    }
    close(fd);
    if (sizeof(hdId) < HD_DRIVEID_SIZE) {
        LOGE("GetDiskRpm: hd_driveid structure size is smaller");
        return 0;
    }
    return static_cast<uint32_t>(hdId.words206_254[RPM_ARRAY_INDEX]);
}

std::string ScanDevice::GetSataSerialNumber(int fd)
{
    struct hd_driveid hdId;
    if (ioctl(fd, HDIO_GET_IDENTITY, &hdId) < 0) {
        LOGE("GetSataSsdSerialNumber: execute ioctl failed");
        return "Unknown";
    }
    size_t len = sizeof(hdId.serial_no);
    std::string serial(reinterpret_cast<const char *>(hdId.serial_no), len);
    return TrimSpaces(serial);
}

std::string ScanDevice::GetNvmeSerialNumber(const std::string &deviceName)
{
    std::string serialPath = sysBlockPath + SPLIT_STRING + deviceName + SERIAL_NODE;
    std::string content;
    if (ReadSysfsNode(serialPath, content)) {
        LOGI("GetNvmeSerialNumber success: %{public}s", content.c_str());
        return content;
    }
    LOGE("GetNvmeSerialNumber: read serial node failed for %{public}s", deviceName.c_str());
    return "Unknown";
}

std::string ScanDevice::GetSerialNumber(const std::string &deviceName, const bool isNvmeDevice)
{
    std::string serial;
    if (isNvmeDevice) {
        serial = GetNvmeSerialNumber(deviceName);
    } else {
        std::string devicePath = devBlockPath + SPLIT_STRING + deviceName;
        int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            LOGE("GetSerialNumber: open %{public}s failed", devicePath.c_str());
            return "Unknown";
        }
        serial = GetSataSerialNumber(fd);
        close(fd);
    }
    if (serial.empty() || serial == "Unknown") {
        LOGE("GetSerialNumber: failed to get serial number for %{public}s", deviceName.c_str());
        return "Unknown";
    }
    LOGI("GetSerialNumber success: %{public}s", serial.c_str());
    return serial;
}

std::string ScanDevice::GetPciePath(const std::string &deviceName)
{
    std::string devicePath = sysBlockPath + SPLIT_STRING + deviceName;
    char linkTarget[PATH_MAX];
    ssize_t len = readlink(devicePath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    if (len > 0) {
        linkTarget[len] = '\0';
        LOGI("GetPciePath success: %{public}s", linkTarget);
        return std::string(linkTarget);
    }
    LOGE("GetPciePath failed");
    return "";
}

bool ScanDevice::ReadSataDeviceNumber(const std::string &deviceName, std::string &major, std::string &minor)
{
    std::string devPath = sysBlockPath + SPLIT_STRING + deviceName + DEV_NODE;
    std::string content;
    if (!ReadSysfsNode(devPath, content)) {
        LOGE("ReadSataDeviceNumber failed: read dev node failed");
        return false;
    }
    size_t colonPos = content.find(DEVICE_NUMBER_SPLIT);
    if (colonPos == std::string::npos) {
        LOGE("ReadSataDeviceNumber failed: invalid dev node format");
        return false;
    }
    major = content.substr(0, colonPos);
    minor = content.substr(colonPos + 1);
    return true;
}

bool ScanDevice::ReadNvmeDeviceNumber(const std::string &deviceName, std::string &major, std::string &minor)
{
    std::string ueventPath = sysBlockPath + SPLIT_STRING + deviceName + UEVENT_NODE;
    std::ifstream file(ueventPath);
    if (!file.is_open()) {
        LOGE("ReadNvmeDeviceNumber failed: open %{public}s failed", ueventPath.c_str());
        return false;
    }
    std::string line;
    major = "";
    minor = "";
    while (std::getline(file, line)) {
        if (line.find(MAJOR_KEY) == 0) {
            major = line.substr(strlen(MAJOR_KEY));
        } else if (line.find(MINOR_KEY) == 0) {
            minor = line.substr(strlen(MINOR_KEY));
        }
    }
    file.close();
    if (major.empty() || minor.empty()) {
        LOGE("ReadNvmeDeviceNumber failed: MAJOR or MINOR not found in %{public}s", ueventPath.c_str());
        return false;
    }
    return true;
}

std::string ScanDevice::GetDiskId(const std::string &deviceName, const bool isNvmeDevice)
{
    std::string major;
    std::string minor;
    bool success = false;
    if (isNvmeDevice) {
        success = ReadNvmeDeviceNumber(deviceName, major, minor);
    } else {
        success = ReadSataDeviceNumber(deviceName, major, minor);
    }
    if (!success) {
        LOGE("GetDiskId failed: read device number failed for %{public}s", deviceName.c_str());
        return "";
    }
    std::string diskId = DISK_ID_PREFIX + major + DISK_ID_CONTACT + minor;
    LOGI("GetDiskId success: %{public}s", diskId.c_str());
    return diskId;
}

std::string ScanDevice::GetDevicePath(const std::string &deviceName)
{
    std::string devicePath = devBlockPath + SPLIT_STRING + deviceName;
    LOGI("GetDevicePath success: %{public}s", devicePath.c_str());
    return devicePath;
}

std::string ScanDevice::GetPort(const std::string &pciePath, const bool isNvmeDevice)
{
    if (pciePath.empty()) {
        LOGE("GetPort failed: pciePath is empty");
        return "";
    }
    std::string pattern = isNvmeDevice ? NVME_PORT_PATTERN : ATA_PORT_PATTERN;
    std::string prefix = isNvmeDevice ? NVME_PREFIX : ATA_PREFIX;
    std::regex portPattern(pattern);
    std::smatch match;
    if (std::regex_search(pciePath, match, portPattern)) {
        std::string port = prefix + std::string(match[1]);
        LOGI("GetPort success: %{public}s", port.c_str());
        return port;
    }
    LOGE("GetPort failed: no match found in pciePath");
    return "";
}
} // namespace StorageDaemon
} // namespace OHOS