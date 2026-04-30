/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "scan_device.h"

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/nvme_ioctl.h>
#include <cstring>
#include <regex>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <charconv>
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char* ZERO_STRING = "0";
constexpr const char* ONE_STRING = "1";
constexpr const char* SPLIT_STRING = "/";
constexpr const char* DEVICE_NUMBER_SPLIT = ":";
constexpr const char* REMOVABLE_NODE = "/removable";
constexpr const char* SD_STRING = "sd";
constexpr const char* UFS_STRING = "ufs";
constexpr const char* NVME_STRING = "nvme";
constexpr const char* NVME0_STRING = "nvme0";
constexpr const char* SIZE_NODE = "/size";
constexpr const char* DEV_NODE = "/dev";
constexpr const char* ROTATIONAL_NODE = "/queue/rotational";
constexpr const char* STATE_NODE = "/device/state";
constexpr const char* MODEL_NODE = "/device/model";
constexpr const char* VENDOR_NODE = "/device/vendor";
// 磁盘 ID 格式前缀
constexpr const char* DISK_ID_PREFIX = "disk-";
constexpr const char* DISK_ID_CONTACT = "-";
// 用于匹配 SATA 端口标识符的正则表达式，例如 "ata1", "ata2"
constexpr const char* ATA_PORT_PATTERN = "ata([0-9]+)";

constexpr int NVME_IDENTIFY_DATA_SIZE = 4096;
constexpr uint8_t NVME_ADMIN_IDENTIFY = 0x06;
const uint64_t SECTOR_SIZE = 512;
const int NUMBER_BASE = 10;
const size_t NVME_BASE_LEN = 4;
const char NVME_LINK = 'n';
constexpr uint32_t NVME_SERIAL_NUMBER_OFFSET = 0x40;
constexpr uint32_t NVME_SERIAL_NUMBER_LENGTH = 20;
// 单个扇区上限
constexpr uint64_t MAX_SAFE_SECTORS_PER_PARTITION = 100000000000000;

ScanDevice::ScanDevice(const std::string &sysBlockPath, const std::string &devBlockPath)
    : sysBlockPath(sysBlockPath), devBlockPath(DEV_NODE)
{
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
    // removable标签被设置, 值为0，且是sd*，则是数据盘
    if (!isNeedCheckUfs) {
        return !isRemovable && (deviceName.find(SD_STRING) == 0);
    }
    // 如果无removable标签，使用readlink函数读取实际device节点
    std::string devicePath = sysBlockPath + SPLIT_STRING + deviceName;
    char linkTarget[PATH_MAX];
    ssize_t len = readlink(devicePath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    // LCOV_EXCL_START
    if (len != -1) {
        linkTarget[len] = '\0';
        std::string targetPath(linkTarget);
        // 检查是否为ufs设备，非ufs设备则为数据盘
        if (targetPath.find(UFS_STRING) == std::string::npos) {
            LOGI("%{public}s isn't ufs: %{public}s", devicePath.c_str(), targetPath.c_str());
            return true;
        } else {
            LOGE("%{public}s is ufs: %{public}s", devicePath.c_str(), targetPath.c_str());
            return false;
        }
    }
    // LCOV_EXCL_STOP
    LOGE("readlink of %{public}s failed", devicePath.c_str());
    return false;
}

bool ScanDevice::IsValidNvmeDevice(const std::string &deviceName)
{
    // 排除 nvme0* 开头的设备（通常视为系统盘）
    if (deviceName.find(NVME0_STRING) == 0) {
        LOGI("Ignore nvme0* node: %{public}s", deviceName.c_str());
        return false;
    }

    // 基础长度检查：nvmeXnY 至少需要 7 个字符 (nvme0n1)
    if (deviceName.size() < 7) {
        LOGI("Ignore nvme name too short: %{public}s", deviceName.c_str());
        return false;
    }

    // 验证格式为 nvme[数字]n[分区]
    size_t pos = NVME_BASE_LEN;

    // 验证并跳过控制器编号 (至少1位数字)
    if (!std::isdigit(static_cast<unsigned char>(deviceName[pos]))) {
        LOGI("Ignore invalid nvme format: missing controller ID in %{public}s", deviceName.c_str());
        return false;
    }
    while (pos < deviceName.size() && std::isdigit(static_cast<unsigned char>(deviceName[pos]))) {
        ++pos;
    }

    // 验证 'n' 字符存在
    if (pos >= deviceName.size() || deviceName[pos] != NVME_LINK) {
        LOGI("Ignore invalid nvme format: missing 'n' in %{public}s", deviceName.c_str());
        return false;
    }
    ++pos; // 跳过 'n'

    // 验证分区编号 (至少1位数字)
    if (pos >= deviceName.size() || !std::isdigit(static_cast<unsigned char>(deviceName[pos]))) {
        LOGI("Ignore invalid nvme format: missing partition ID in %{public}s", deviceName.c_str());
        return false;
    }
    return true;
}

std::vector<BlockInfo> ScanDevice::GetDataDisks()
{
    std::vector<BlockInfo> dataDisks;
    DIR* dir = opendir(sysBlockPath.c_str());
    if (!dir) {
        LOGE("%{public}s open failed", sysBlockPath.c_str());
        return dataDisks;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string deviceName = entry->d_name;
        if (deviceName == "." || deviceName == "..") {
            LOGI("Ignore %{public}s", deviceName.c_str());
            continue;
        }

        // 判断是否为 s* 节点或 nvme* 节点
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
        // 获取设备信息
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
    DIR *dir = opendir(sysBlockPath.c_str());
    if (!dir) {
        LOGE("%{public}s open failed", sysBlockPath.c_str());
        return externalDisks;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string deviceName = entry->d_name;
        if (deviceName == "." || deviceName == "..") {
            LOGI("Ignore %{public}s", deviceName.c_str());
            continue;
        }

        // 判断是否为 s* 节点
        bool isSDevice = (deviceName.length() > 0 && deviceName[0] == 's');
        if (!isSDevice) {
            LOGI("Ignore non-s* node: %{public}s", deviceName.c_str());
            continue;
        }
        bool isRemovable = false;
        // 如果removable标签被设置，且值为1，则是外盘
        if (!ReadRemovableNode(deviceName, isRemovable) || !isRemovable) {
            continue;
        }
        // 获取设备信息
        BlockInfo blockInfo;
        blockInfo.removable = isRemovable;
        if (GetBlockInfo(deviceName, false, blockInfo) == 0) {
            externalDisks.push_back(blockInfo);
        }
    }

    closedir(dir);
    return externalDisks;
}

int ScanDevice::GetBlockInfo(const std::string& deviceName, const bool isNvmeDevice, BlockInfo& blockInfo)
{
    // 获取磁盘容量
    blockInfo.sizeBytes = GetDiskSize(deviceName);

    // 获取厂家信息
    blockInfo.vendor = GetVendor(deviceName);

    // 获取设备型号
    blockInfo.model = GetModel(deviceName);

    // 获取接口类型
    blockInfo.interfaceType = GetInterfaceType(deviceName);

    // 获取机械硬盘转速
    blockInfo.rpm = GetDiskRpm(deviceName, isNvmeDevice);

    // 获取磁盘状态
    blockInfo.state = GetDiskState(deviceName);

    // 获取介质类型
    blockInfo.mediaType = GetMediaType(deviceName, isNvmeDevice);

    // 获取序列号
    blockInfo.serialNumber = GetSerialNumber(deviceName, isNvmeDevice);

    // 获取PCIe路径
    blockInfo.pciePath = GetPciePath(deviceName);

    // 获取磁盘ID
    blockInfo.diskId = GetDiskId(deviceName);

    // 获取已使用容量
    blockInfo.usedBytes = GetUsedBytes(deviceName);

    // 获取可用容量
    blockInfo.availableBytes = GetAvailableBytes(deviceName);

    // 获取设备路径
    blockInfo.devicePath = GetDevicePath(deviceName);

    // 获取物理接口
    blockInfo.port = GetPort(blockInfo.pciePath, isNvmeDevice);

    return 0;
}

bool ScanDevice::ReadSysfsNode(const std::string &path, std::string &content)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        LOGE("Open %{public}s failed", path.c_str());
        return false;
    }
    std::getline(file, content);
    file.close();
    return true;
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
    // 先跳过尾部空白字符
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
    if (content.back() == '\n') {
        content.pop_back();
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
    return static_cast<uint64_t>(sectors) * SECTOR_SIZE;
}

std::string ScanDevice::GetVendor(const std::string& deviceName)
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

std::string ScanDevice::GetModel(const std::string& deviceName)
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

std::string ScanDevice::GetInterfaceType(const std::string& deviceName)
{
    if (deviceName.find(NVME_STRING) == 0) {
        return "NVMe";
    } else if (deviceName[0] == 's') {
        // 可能是SATA或SAS
        return "SATA";
    }
    LOGE("Get interface type failed");
    return "Unknown";
}

uint32_t ScanDevice::GetDiskRpm(const std::string& deviceName, const bool isNvmeDevice)
{
    // NVME设备不用获取转速
    if (isNvmeDevice) {
        return 0;
    }
    // 机械硬盘转速根据ioctl获取IDENTIFY DEVICE信息获取
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
    return static_cast<uint64_t>(hdId.cur_cyls);
}

std::string ScanDevice::GetDiskState(const std::string& deviceName)
{
    std::string statePath = sysBlockPath + SPLIT_STRING + deviceName + STATE_NODE;
    std::string content;

    if (ReadSysfsNode(statePath, content)) {
        LOGI("GetDiskState success: %{public}s", content.c_str());
        return content;
    }
    LOGE("GetDiskState failed");
    return "Unknown";
}

MediaType ScanDevice::GetMediaType(const std::string& deviceName, const bool isNvmeDevice)
{
    // Nvme结点暂定是SSD
    if (isNvmeDevice) {
        return MediaType::SSD;
    }
    std::string rotationalPath = sysBlockPath + SPLIT_STRING + deviceName + ROTATIONAL_NODE;
    std::string content;

    if (ReadSysfsNode(rotationalPath, content)) {
        LOGI("GetMediaType success: %{public}s", content.c_str());
        if (content == "0") {
            return MediaType::SSD;
        } else if (content == "1") {
            return MediaType::HDD;
        }
    }
    LOGE("GetMediaType failed");
    return MediaType::UNKNOWN;
}

// 去除字符串尾部空格
std::string TrimTrailingSpaces(const std::string& str)
{
    size_t len = str.length();
    while (len > 0 && str[len - 1] == ' ') {
        len--;
    }
    return str.substr(0, len);
}

// 获取 Sd* 设备序列号
std::string ScanDevice::GetSataSerialNumber(int fd)
{
    // LCOV_EXCL_START
    struct hd_driveid hdId;
    if (ioctl(fd, HDIO_GET_IDENTITY, &hdId) < 0) {
        LOGE("GetSataSsdSerialNumber: execute ioctl failed");
        return "Unknown";
    }

    size_t len = sizeof(hdId.serial_no);
    std::string serial(reinterpret_cast<const char *>(hdId.serial_no), len);
    return TrimTrailingSpaces(serial);
    // LCOV_EXCL_STOP
}

std::string ScanDevice::GetNvmeSerialNumber(int fd)
{
    // LCOV_EXCL_START
    uint8_t identifyData[NVME_IDENTIFY_DATA_SIZE] = {0};
    struct nvme_admin_cmd cmd = {0};
    cmd.opcode = NVME_ADMIN_IDENTIFY;
    cmd.nsid = 0;
    cmd.addr = reinterpret_cast<uint64_t>(reinterpret_cast<uintptr_t>(identifyData));
    cmd.data_len = NVME_IDENTIFY_DATA_SIZE;
    cmd.cdw10 = 1;
    cmd.cdw11 = 0;
    // cdw10 = 1 表示获取 Page 0 (Namespace Identifier)，序列号在偏移 0x40 处
    int ret = ioctl(fd, NVME_IOCTL_ADMIN_CMD, cmd);
    if (ret < 0) {
        LOGE("GetNvmeSerialNumber: NvmeIdentifyCmd failed, ret=%d", ret);
        return "Unknown";
    }
    // 序列号位于偏移 0x40 (64) 处，长度为 20 字节
    const char *serial_ptr = reinterpret_cast<const char *>(identifyData) + NVME_SERIAL_NUMBER_OFFSET;
    std::string serial(serial_ptr, NVME_SERIAL_NUMBER_LENGTH);
    return TrimTrailingSpaces(serial);
    // LCOV_EXCL_STOP
}

std::string ScanDevice::GetSerialNumber(const std::string &deviceName, const bool isNvmeDevice)
{
    // 序列号根据用户态程序获取（发ioctl）获取
    // LCOV_EXCL_START
    std::string devicePath = devBlockPath + SPLIT_STRING + deviceName;
    int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        LOGE("GetSerialNumber: open %{public}s failed", devicePath.c_str());
        return "Unknown";
    }
    std::string serial;
    if (isNvmeDevice) {
        serial = GetNvmeSerialNumber(fd);
    } else {
        serial = GetSataSerialNumber(fd);
    }
    close(fd);
    if (serial.empty() || serial == "Unknown") {
        LOGE("GetSerialNumber: failed to get serial number for %{public}s", deviceName.c_str());
        return "Unknown";
    }
    LOGI("GetSerialNumber success: %{public}s", serial.c_str());
    // LCOV_EXCL_STOP
    return serial;
}

std::string ScanDevice::GetPciePath(const std::string& deviceName)
{
    // PCIe路径根据/sys/block/sda软链接指向的路径
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

std::string ScanDevice::GetDiskId(const std::string& deviceName)
{
    std::string devPath = sysBlockPath + SPLIT_STRING + deviceName + DEV_NODE;
    std::string content;

    if (!ReadSysfsNode(devPath, content)) {
        LOGE("GetDiskId failed: read dev node failed");
        return "";
    }

    // 内容格式为 "主设备号:次设备号"
    size_t colonPos = content.find(DEVICE_NUMBER_SPLIT);
    if (colonPos == std::string::npos) {
        LOGE("GetDiskId failed: invalid dev node format");
        return "";
    }

    std::string major = content.substr(0, colonPos);
    std::string minor = content.substr(colonPos + 1);
    // 去除可能的换行符
    if (!minor.empty() && minor.back() == '\n') {
        minor.pop_back();
    }

    std::string diskId = DISK_ID_PREFIX + major + DISK_ID_CONTACT + minor;
    LOGI("GetDiskId success: %{public}s", diskId.c_str());
    return diskId;
}

uint64_t ScanDevice::GetUsedBytes(const std::string &deviceName)
{
    std::string blockPath = sysBlockPath + SPLIT_STRING + deviceName;
    DIR *dir = opendir(blockPath.c_str());
    if (!dir) {
        LOGE("GetUsedBytes failed: open %{public}s failed", blockPath.c_str());
        return 0;
    }

    uint64_t totalSectors = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        // 查找 deviceName* 开头的目录
        if (name.find(deviceName) != 0 || name.length() <= deviceName.length() || !std::isdigit(name.back())) {
            continue;
        }
        std::string sizePath = blockPath + SPLIT_STRING + name + SIZE_NODE;
        std::string content;
        if (!ReadSysfsNode(sizePath, content) || content.empty()) {
            continue;
        }
        if (content.back() == '\n') {
            content.pop_back();
        }
        if (content.empty()) {
            continue;
        }
        unsigned long long sectors = 0;
        if (!ParseStringToUlongLong(content, sectors)) {
            continue;
        }
        // LCOV_EXCL_START
        if (sectors > MAX_SAFE_SECTORS_PER_PARTITION) {
            LOGW("GetUsedBytes: partition %{public}s has suspiciously large size: %{public}llu sectors, skipping.", 
                 name.c_str(), sectors);
            continue;
        }
        if (totalSectors > UINT64_MAX - sectors) {
            LOGE("GetUsedBytes: total sectors overflow detected for %{public}s", deviceName.c_str());
            closedir(dir);
            return 0;
        }
        // LCOV_EXCL_STOP
        totalSectors += sectors;
    }
    closedir(dir);
    uint64_t usedBytes = totalSectors * SECTOR_SIZE;
    LOGI("GetUsedBytes success: %{public}ld", usedBytes);
    return usedBytes;
}

uint64_t ScanDevice::GetAvailableBytes(const std::string& deviceName)
{
    uint64_t sizeBytes = GetDiskSize(deviceName);
    uint64_t usedBytes = GetUsedBytes(deviceName);
    if (sizeBytes < usedBytes) {
        LOGE("GetAvailableBytes warning: sizeBytes < usedBytes");
        return 0;
    }

    uint64_t availableBytes = sizeBytes - usedBytes;
    LOGI("GetAvailableBytes success: %{public}ld", availableBytes);
    return availableBytes;
}

std::string ScanDevice::GetDevicePath(const std::string& deviceName)
{
    std::string devicePath = devBlockPath + SPLIT_STRING + deviceName;
    LOGI("GetDevicePath success: %{public}s", devicePath.c_str());
    return devicePath;
}

std::string ScanDevice::GetPort(const std::string &pciePath, const bool isNvmeDevice)
{
    // 如果是 nvme 设备，暂时返回空字符串
    if (isNvmeDevice) {
        LOGI("GetPort: nvme device, return empty");
        return "";
    }
    if (pciePath.empty()) {
        LOGE("GetPort failed: pciePath is empty");
        return "";
    }

    // 查找 ata[数字] 模式
    std::regex ataPattern(ATA_PORT_PATTERN);
    std::smatch match;
    if (std::regex_search(pciePath, match, ataPattern)) {
        std::string port = "ata" + std::string(match[1]);
        LOGI("GetPort success: %{public}s", port.c_str());
        return port;
    }

    LOGE("GetPort failed: no ata* found in pciePath");
    return "";
}
} // namespace StorageDaemon
} // namespace OHOS