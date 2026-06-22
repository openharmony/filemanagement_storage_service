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
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

#include "storage_service_log.h"
#include <charconv>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <cerrno>
#include <cinttypes>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <regex>
#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <cctype>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
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
constexpr const char *REVNUM_NODE = "/device/rev";
constexpr const char *MAJOR_KEY = "MAJOR=";
constexpr const char *MINOR_KEY = "MINOR=";
constexpr const char *SERIAL_NODE = "/device/serial";
constexpr const char *ATA_PREFIX = "ata";
constexpr const char *NVME_PORT_PATTERN = "nvme([0-9]+)";
constexpr const char *NVME_PREFIX = "nvme";
constexpr const char *DISK_ID_PREFIX = "disk-";
constexpr const char *DISK_ID_CONTACT = "-";
constexpr const char *ATA_PORT_PATTERN = "ata([0-9]+)";
constexpr const char *PARENT_DIR_PREFIX = "../";
constexpr const size_t PARENT_DIR_PREFIX_LEN = 3;
constexpr const size_t PARENT_DIR_SKIP_LEN = 2;

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

std::string ScanDevice::GetRealPath(const std::string &deviceName)
{
    std::string modelPath = sysBlockPath + SPLIT_STRING + deviceName;
    LOGI("GetRealPath modelPath: %{public}s", modelPath.c_str());
    char linkTarget[PATH_MAX] = {0};
    ssize_t len = readlink(modelPath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    std::string linkTargetStr;
    if (len > 0) {
        linkTargetStr.assign(linkTarget, len);
        if (linkTargetStr.size() >= PARENT_DIR_PREFIX_LEN &&
            linkTargetStr.substr(0, PARENT_DIR_PREFIX_LEN) == PARENT_DIR_PREFIX) {
            linkTargetStr = "/sys" + linkTargetStr.substr(PARENT_DIR_SKIP_LEN);
            LOGI("GetRealPath linkTargetStr: %{public}s", linkTargetStr.c_str());
            return linkTargetStr;
        }
    } else if (len == -1) {
        LOGE("readlink failed for: %s", modelPath.c_str());
        linkTargetStr.clear();
    }
    return "";
}

std::vector<BlockInfo> ScanDevice::GetExternalDisks(const std::string &devName, const std::string &diskId)
{
    LOGI("[L2:ScanDevice] GetExternalDisks ENTER");
    std::vector<BlockInfo> externalDisks;
    BlockInfo info;
    info.diskId = diskId;
    uint64_t size = -1;
    std::string devPath = std::string(DEV_PATH) + SPLIT_STRING + devName;
    std::string sizePath = devBlockPath + SPLIT_STRING + diskId;
    std::string realPath = GetRealPath(devName);
    GetExternalDiskSize(sizePath, &size);
    info.sizeBytes = size;
    info.vendor = GetVendor(devName);
    info.model = GetModel(devName);
    info.devnum = ReadFileInParentDirs(realPath, "devnum");
    info.busnum = ReadFileInParentDirs(realPath, "busnum");
    info.devNode = GetDevNode(devName);
    info.scsiBusNum = GetScsiBusNum(realPath);
    info.fwVersion = GetFwVersion(devName);
    struct stat st;
    if (stat(devPath.c_str(), &st) == 0 && S_ISBLK(st.st_mode)) {
        dev_t dev = st.st_rdev;
        if (major(dev) == DISK_CD_MAJOR) {
            nlohmann::json extraInfoJson;
            std::string driveType = GetOpticalDriveType(devPath);
            std::string discType = GetCDType(devPath);
            std::string oddDriveType = GetOddDriverType(realPath);
            extraInfoJson["DRIVE_TYPE"] = driveType;
            extraInfoJson["DISC_TYPE"] = discType;
            extraInfoJson["ODD_DRIVER_TYPE"] = oddDriveType;
            
            std::string maxWriteSpeed;
            nlohmann::json speedInfo = GetSpeedInfo(devPath, discType, maxWriteSpeed);
            
            extraInfoJson["SPEED_INFO"] = speedInfo;
            extraInfoJson["MAX_WRITE_SPEED"] = maxWriteSpeed + "X";
            info.ODD_INFO = extraInfoJson;
        }
    }
    LOGI("[L2:ScanDevice] GetExternalDisks: info.devnum=%{public}s, info.busnum=%{public}s,"
         "info.devNode=%{public}s, info.scsiBusNum=%{public}s, info.fwVersion=%{public}s, info.ODD_INFO=%{public}s",
         info.devnum.c_str(), info.busnum.c_str(), info.devNode.c_str(), info.scsiBusNum.c_str(),
         info.fwVersion.c_str(), info.ODD_INFO.dump().c_str());
    externalDisks.push_back(info);
    return externalDisks;
}

std::string ScanDevice::FormatSpeedValue(double speedVal)
{
    if (speedVal == static_cast<int64_t>(speedVal)) {
        return std::to_string(static_cast<int64_t>(speedVal));
    }
    std::string formatted = std::to_string(speedVal);
    while (formatted.back() == '0') {
        formatted.pop_back();
    }
    if (formatted.back() == '.') {
        formatted.pop_back();
    }
    return formatted;
}

std::string ScanDevice::ExtractSpeedFromLine(const std::string &line)
{
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return "";
    }
    std::string key = line.substr(0, colonPos);
    size_t keyStart = key.find_first_not_of(" \t");
    if (keyStart != std::string::npos) {
        key = key.substr(keyStart);
    }
    if (key.find("Write Speed") != 0) {
        return "";
    }
    std::string value = line.substr(colonPos + 1);
    size_t valStart = value.find_first_not_of(" \t");
    if (valStart != std::string::npos) {
        value = value.substr(valStart);
    }
    size_t xPos = value.find('x');
    if (xPos != std::string::npos) {
        value = value.substr(0, xPos);
    }
    errno = 0;
    char *endPtr = nullptr;
    double speedVal = std::strtod(value.c_str(), &endPtr);
    if (errno != 0 || endPtr == value.c_str() || speedVal < 0) {
        LOGE("[L2:ScanDevice] ExtractSpeedFromLine: invalid speed value=%{public}s", value.c_str());
        return "";
    }
    return FormatSpeedValue(speedVal);
}

nlohmann::json ScanDevice::ParseMediaInfoLines(const std::vector<std::string> &output)
{
    LOGI("[L2:ScanDevice] ParseMediaInfoLines: output size=%{public}d", static_cast<int>(output.size()));
    std::unordered_set<std::string> seen;
    nlohmann::json speedArr = nlohmann::json::array();
    for (const auto &chunk : output) {
        std::stringstream ss(chunk);
        std::string line;
        while (std::getline(ss, line)) {
            std::string formatted = ExtractSpeedFromLine(line);
            if (formatted.empty()) {
                continue;
            }
            if (seen.insert(formatted).second) {
                speedArr.push_back(formatted);
            }
        }
    }
    LOGI("[L2:ScanDevice] ParseMediaInfoLines: speedArr=%{public}s", speedArr.dump().c_str());
    return speedArr;
}

bool ScanDevice::IsValidSpeedString(const std::string &speedStr)
{
    if (speedStr.empty()) {
        return false;
    }
    
    bool hasDot = false;
    bool hasDigit = false;
    
    for (char c : speedStr) {
        if (c == '.') {
            if (hasDot) {
                return false;
            }
            hasDot = true;
        } else if (c >= '0' && c <= '9') {
            hasDigit = true;
        } else {
            return false;
        }
    }
    
    return hasDigit;
}

std::string ScanDevice::GetMaxSpeedFromSpeedInfo(const nlohmann::json &speedInfo)
{
    LOGI("[L2:ScanDevice] GetMaxSpeedFromSpeedInfo: >>> ENTER <<< speedInfo=%{public}s", speedInfo.dump().c_str());
    if (speedInfo.empty()) {
        LOGE("[L2:ScanDevice] GetMaxSpeedFromSpeedInfo: <<< EXIT FAILED <<< speedInfo is empty");
        return "";
    }
    double maxSpeed = 0.0;
    std::string maxSpeedStr;
    for (const auto &speed : speedInfo) {
        std::string speedStr = speed.get<std::string>();
        if (!IsValidSpeedString(speedStr)) {
            LOGE("[L2:ScanDevice] GetMaxSpeedFromSpeedInfo: invalid speed format, speed=%{public}s",
                 speedStr.c_str());
            continue;
        }
        double speedValue = std::stod(speedStr);
        if (speedValue > maxSpeed) {
            maxSpeed = speedValue;
            maxSpeedStr = speedStr;
        }
    }
    LOGI("[L2:ScanDevice] GetMaxSpeedFromSpeedInfo: <<< EXIT SUCCESS <<< maxSpeedStr=%{public}s",
         maxSpeedStr.c_str());
    return maxSpeedStr;
}

nlohmann::json ScanDevice::ParseWodimPrcapOutput(const std::vector<std::string> &output)
{
    LOGI("[L2:ScanDevice] ParseWodimPrcapOutput: >>> ENTER <<<");
    
    std::unordered_set<std::string> seen;
    nlohmann::json speedArr = nlohmann::json::array();
    
    for (const auto &line : output) {
        if (line.find("Write speed #") == std::string::npos) {
            continue;
        }
        
        size_t cdPos = line.find("(CD");
        if (cdPos == std::string::npos) {
            continue;
        }
        
        size_t xPos = line.find('x', cdPos);
        if (xPos == std::string::npos) {
            continue;
        }
        
        std::string speedStr = line.substr(cdPos + 3, xPos - (cdPos + 3));
        speedStr.erase(std::remove_if(speedStr.begin(), speedStr.end(), ::isspace), speedStr.end());
        
        if (!speedStr.empty() && seen.insert(speedStr).second) {
            speedArr.push_back(speedStr);
            LOGI("[L2:ScanDevice] ParseWodimPrcapOutput: found CD speed=%{public}s", speedStr.c_str());
        }
    }
    
    LOGI("[L2:ScanDevice] ParseWodimPrcapOutput: <<< EXIT SUCCESS <<< speedArr=%{public}s", speedArr.dump().c_str());
    return speedArr;
}

nlohmann::json ScanDevice::GetSpeedInfo(const std::string &devPath,
                                        const std::string &discType,
                                        std::string &maxWriteSpeed)
{
    LOGI("[L2:ScanDevice] GetSpeedInfo: >>> ENTER <<< devPath=%{public}s, discType=%{public}s",
         devPath.c_str(), discType.c_str());
    nlohmann::json speedInfo;
    bool isCD = (discType.find("CD") != std::string::npos || discType.find("cd") != std::string::npos);
    if (isCD) {
        LOGI("GetSpeedInfo: discType contains CD, using wodim -prcap");
        std::vector<std::string> cmd = { "wodim", "dev=" + devPath, "-prcap" };
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        LOGI("GetSpeedInfo wodim -prcap ret=%{public}d", ret);
        for (const auto &str : output) {
            LOGI("GetSpeedInfo wodim output: %{public}s", str.c_str());
        }
        speedInfo = ParseWodimPrcapOutput(output);
    } else {
        LOGI("GetSpeedInfo: discType does not contain CD, using dvd+rw-mediainfo");
        std::vector<std::string> cmd = { "dvd+rw-mediainfo", devPath };
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        LOGI("GetSpeedInfo dvd+rw-mediainfo ret=%{public}d", ret);
        for (const auto &str : output) {
            LOGI("GetSpeedInfo output: %{public}s", str.c_str());
        }
        speedInfo = ParseMediaInfoLines(output);
    }
    maxWriteSpeed = GetMaxSpeedFromSpeedInfo(speedInfo);
    LOGI("[L2:ScanDevice] GetSpeedInfo: <<< EXIT SUCCESS <<< maxWriteSpeed=%{public}s", maxWriteSpeed.c_str());
    return speedInfo;
}

bool ScanDevice::GetExternalDiskSize(const std::string &path, uint64_t *size)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f == nullptr) {
        LOGE("[L2:ScanDevice] GetExternalDiskSize: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        return false;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("[L2:ScanDevice] GetExternalDiskSize: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        (void)fclose(f);
        return false;
    }
    if (ioctl(fd, BLKGETSIZE64, size)) {
        LOGE("[L2:ScanDevice] GetExternalDiskSize: <<< EXIT FAILED <<< get size failed, errno=%{public}d", errno);
        (void)fclose(f);
        return false;
    }
    (void)fclose(f);
    LOGI("[L2:ScanDevice] GetExternalDiskSize: <<< EXIT SUCCESS <<< size=%{public}" PRIu64, *size);
    return true;
}

int ScanDevice::GetBlockInfo(const std::string &deviceName, const bool isNvmeDevice, BlockInfo &blockInfo)
{
    std::string devPath = std::string(DEV_PATH) + SPLIT_STRING + deviceName;
    std::string realPath = GetRealPath(deviceName);
    blockInfo.sizeBytes = GetDiskSize(deviceName);
    blockInfo.vendor = GetVendor(deviceName);
    blockInfo.model = GetModel(deviceName);
    blockInfo.devnum = ReadFileInParentDirs(realPath, "devnum");
    blockInfo.busnum = ReadFileInParentDirs(realPath, "busnum");
    blockInfo.devNode = GetDevNode(deviceName);
    blockInfo.scsiBusNum = GetScsiBusNum(realPath);
    blockInfo.fwVersion = GetFwVersion(deviceName);
    struct stat st;
    if (stat(devPath.c_str(), &st) == 0 && S_ISBLK(st.st_mode)) {
        dev_t dev = st.st_rdev;
        if (major(dev) == DISK_CD_MAJOR) {
            nlohmann::json extraInfoJson;
            std::string driveType = GetOpticalDriveType(devPath);
            std::string discType = GetCDType(devPath);
            std::string oddDriveType = GetOddDriverType(realPath);
            extraInfoJson["DRIVE_TYPE"] = driveType;
            extraInfoJson["DISC_TYPE"] = discType;
            extraInfoJson["ODD_DRIVER_TYPE"] = oddDriveType;
            
            std::string maxWriteSpeed;
            nlohmann::json speedInfo = GetSpeedInfo(devPath, discType, maxWriteSpeed);
            
            extraInfoJson["SPEED_INFO"] = speedInfo;
            extraInfoJson["MAX_WRITE_SPEED"] = maxWriteSpeed + "X";
            blockInfo.ODD_INFO = extraInfoJson;
        }
    }
    blockInfo.interfaceType = GetInterfaceType(deviceName);
    blockInfo.rpm = GetDiskRpm(deviceName, isNvmeDevice);
    blockInfo.serialNumber = GetSerialNumber(deviceName, isNvmeDevice);
    blockInfo.diskId = GetDiskId(deviceName, isNvmeDevice);
    std::string pciePath = GetPciePath(deviceName);
    blockInfo.devicePath = GetDevicePath(deviceName);
    blockInfo.port = GetPort(pciePath, isNvmeDevice);
    LOGE("[L2:ScanDevice] GetBlockInfo: info.devnum=%{public}s, info.busnum=%{public}s,"
        "info.devNode=%{public}s, info.scsiBusNum=%{public}s, info.fwVersion=%{public}s, info.ODD_INFO=%{public}s",
        blockInfo.devnum.c_str(), blockInfo.busnum.c_str(), blockInfo.devNode.c_str(), blockInfo.scsiBusNum.c_str(),
        blockInfo.fwVersion.c_str(), blockInfo.ODD_INFO.dump().c_str());
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

std::string ScanDevice::ReadFileContent(const std::string &path)
{
    LOGD("[L8:ScanDevice] ReadFileContent: >>> ENTER <<< path=%{public}s", path.c_str());
    std::ifstream infile;
    std::string result;
    char realPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), realPath) == nullptr) {
        LOGE("[L8:ScanDevice] ReadFileContent: <<< EXIT FAILED <<< realpath failed, errno: %{public}d.", errno);
        return "";
    }
    infile.open(realPath);
    if (!infile) {
        LOGE("[L8:ScanDevice] ReadFileContent: <<< EXIT FAILED <<< Cannot open file, errno: %{public}d.", errno);
        return "";
    }
    std::string line;
    bool firstLine = true;
    while (std::getline(infile, line)) {
        if (!firstLine) {
            result += "\n";
        }
        result += line;
        firstLine = false;
    }
    infile.close();
    if (result.empty()) {
        LOGE("[L8:ScanDevice] ReadFileContent: <<< EXIT FAILED <<< file is empty");
    } else {
        LOGD("[L8:ScanDevice] ReadFileContent: <<< EXIT SUCCESS <<<");
    }
    return result;
}

std::string ScanDevice::GetDevnum(const std::string &deviceName)
{
    std::string modelPath = sysBlockPath + SPLIT_STRING + deviceName;
    LOGI("GetDevnum modelPath: %{public}s", modelPath.c_str());
    char linkTarget[PATH_MAX] = {0};
    ssize_t len = readlink(modelPath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    std::string linkTargetStr;
    if (len > 0) {
        linkTargetStr.assign(linkTarget, len);
        if (linkTargetStr.size() >= PARENT_DIR_PREFIX_LEN &&
            linkTargetStr.substr(0, PARENT_DIR_PREFIX_LEN) == PARENT_DIR_PREFIX) {
            linkTargetStr = "/sys" + linkTargetStr.substr(PARENT_DIR_SKIP_LEN);
            LOGI("GetDevnum linkTargetStr: %{public}s", linkTargetStr.c_str());
            std::string content = ReadFileInParentDirs(linkTargetStr, "devnum");
            LOGI("GetDevnum success: %{public}s", content.c_str());
            return content;
        }
    } else if (len == -1) {
        LOGE("readlink failed for: %s", modelPath.c_str());
        linkTargetStr.clear();
    }
    LOGE("GetDevnum: read node of model failed");
    return "Unknown";
}
 
std::string ScanDevice::GetBusnum(const std::string &deviceName)
{
    std::string modelPath = sysBlockPath + SPLIT_STRING + deviceName;
    LOGI("GetBusnum modelPath: %{public}s", modelPath.c_str());
    char linkTarget[PATH_MAX] = {0};
    ssize_t len = readlink(modelPath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    std::string linkTargetStr;
    if (len > 0) {
        linkTargetStr.assign(linkTarget, len);
        if (linkTargetStr.size() >= PARENT_DIR_PREFIX_LEN &&
            linkTargetStr.substr(0, PARENT_DIR_PREFIX_LEN) == PARENT_DIR_PREFIX) {
            linkTargetStr = "/sys" + linkTargetStr.substr(PARENT_DIR_SKIP_LEN);
            LOGI("GetDevnum linkTargetStr: %{public}s", linkTargetStr.c_str());
            std::string content = ReadFileInParentDirs(linkTargetStr, "busnum");
            LOGI("GetDevnum success: %{public}s", content.c_str());
            return content;
        }
    } else if (len == -1) {
        LOGE("readlink failed for: %s", modelPath.c_str());
        linkTargetStr.clear();
    }
    LOGE("GetBusnum: read node of model failed");
    return "Unknown";
}
 
std::string ScanDevice::GetDevNode(const std::string &deviceName)
{
    struct stat st;
    std::string devNode;
    std::string devPath = std::string(DEV_PATH) + SPLIT_STRING + deviceName;
    LOGI("GetDevNode devPath: %{public}s", devPath.c_str());
    if (stat(devPath.c_str(), &st) == 0 && S_ISBLK(st.st_mode)) {
        dev_t dev = st.st_rdev;
        devNode = "block " + std::to_string(major(dev)) + ":" + std::to_string(minor(dev));
    }
    LOGI("GetDevNode devNode: %{public}s", devNode.c_str());
    return devNode;
}

std::string ScanDevice::GetFwVersion(const std::string &deviceName)
{
    std::string modelPath = sysBlockPath + SPLIT_STRING + deviceName + REVNUM_NODE;
    std::string content = ReadFileContent(modelPath);
    LOGI("GetFwVersion content: %{public}s", content.c_str());
    return content;
}
} // namespace StorageDaemon
} // namespace OHOS