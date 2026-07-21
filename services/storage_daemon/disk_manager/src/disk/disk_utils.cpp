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

#include <charconv>
#include <climits>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <future>
#include <libgen.h>
#include <regex>
#include <thread>
#include <sstream>

#include "disk_manager/disk/disk_utils.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/xattr.h>
#include <scsi/sg.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include "storage_service_errno.h"
#include "securec.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {

constexpr int32_t MTP_QUERY_RESULT_LEN = 10;
constexpr const char *MTP_PATH_PREFIX = "/mnt/data/external/mtp";
#define STORAGE_MANAGER_IOC_CHK_BUSY _IOR(0xAC, 77, int)

constexpr const char *BLOCK_DEVICE_PREFIX = "/dev/block/";
constexpr const char *SGDISK_PATH = "/system/bin/sgdisk";
constexpr const char *SGDISK_DUMP_CMD = "--ohos-dump";
constexpr int32_t PATH_MAX_LEN = 4096;
constexpr int32_t DEVICE_MAJOR_MIN = 0;
constexpr int32_t DEVICE_MAJOR_MAX = 4095;
constexpr int32_t DEVICE_MINOR_MIN = 0;
constexpr int32_t DEVICE_MINOR_MAX = 1048575;
constexpr int32_t WAIT_THREAD_TIMEOUT_S = 60;
constexpr int32_t DEF_TIMEOUT = 120000;
constexpr int32_t SENSE_BUFF_LEN = 64;
constexpr int32_t READ_DISC_INFO_CDB_LEN = 10;
constexpr int32_t CDB_ALLOCATION_LENGTH_HIGH = 7;
constexpr int32_t CDB_ALLOCATION_LENGTH_LOW = 8;
constexpr int32_t MAX_ALLOC_LEN = 0xFFFF;
constexpr int32_t READ_DISC_INFO_OPCODE = 0x51;
constexpr uint8_t GET_CAPACITY_CMD_BUF_LEN = 16;
constexpr uint8_t GET_CAPACITY_DATA_BUF_LEN = 48;
constexpr uint8_t GET_DVD_USED_CAPACITY_CMD_LEN = 10;
constexpr uint8_t GET_DVD_USED_CAPACITY_DATA_LEN = 8;
constexpr uint8_t BYTE_SHIFT_24 = 24;
constexpr uint8_t BYTE_SHIFT_16 = 16;
constexpr uint8_t BYTE_SHIFT_8 = 8;
constexpr uint8_t BYTE_MASK = 0xff;
constexpr uint8_t LAST_LBA_BYTE_0 = 0;
constexpr uint8_t LAST_LBA_BYTE_1 = 1;
constexpr uint8_t LAST_LBA_BYTE_2 = 2;
constexpr uint8_t LAST_LBA_BYTE_3 = 3;
constexpr uint8_t BLOCK_SIZE_BYTE_0 = 4;
constexpr uint8_t BLOCK_SIZE_BYTE_1 = 5;
constexpr uint8_t BLOCK_SIZE_BYTE_2 = 6;
constexpr uint8_t BLOCK_SIZE_BYTE_3 = 7;
constexpr int32_t FORMAT_PARTITION_TIMEOUT_S = 5 * 60;

const std::map<std::string, std::string> formatTypeMap_ = {
    {"exfat", "mkfs.exfat"},
    {"vfat", "newfs_msdos"},
    {"ext4", "mke2fs"},
};

static std::string TrimString(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

static bool ParseKeyValuePair(const std::string &key, const std::string &value, BurnOptions &options)
{
    if (value.empty()) {
        LOGE("[L3:DiskUtils] ParseBurnOptions: %{public}s is empty.", key.c_str());
        return false;
    }
    if (key == "diskName") {
        options.diskName = value;
        return true;
    }
    if (key == "burnPath") {
        options.burnPath = value;
        return true;
    }
    if (key == "fsType") {
        options.fsType = value;
        return true;
    }
    if (key == "isIsoImage") {
        options.isIsoImage = (value == "true" || value == "1");
        return true;
    }
    if (key == "isIncBurnSupport") {
        options.isIncBurnSupport = (value == "true" || value == "1");
        return true;
    }
    if (key == "isVerifyBurn") {
        options.isVerifyBurn = (value == "true" || value == "1");
        return true;
    }
    if (key == "burnSpeed") {
        char *endPtr = nullptr;
        errno = 0;
        double speed = strtod(value.c_str(), &endPtr);
        if (errno != 0 || endPtr == nullptr || endPtr == value.c_str() || *endPtr != '\0' || speed < 0) {
            LOGE("[L3:DiskUtils] ParseBurnOptions: invalid burnSpeed: %{public}s", value.c_str());
            return false;
        }
        options.burnSpeed = value;
        return true;
    }
    LOGW("[L3:DiskUtils] ParseBurnOptions: unknown key ignored: %{public}s", key.c_str());
    return true;
}

int32_t ParseBurnOptions(const std::string &burnOptions, BurnOptions &parsedOptions)
{
    LOGI("[L3:DiskUtils] ParseBurnOptions: >>> ENTER <<<");
    
    if (burnOptions.empty()) {
        LOGE("[L3:DiskUtils] ParseBurnOptions: burnOptions is empty");
        return E_PARAMS_INVALID;
    }
    parsedOptions = BurnOptions();
    std::istringstream stream(burnOptions);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) {
            LOGW("[L3:DiskUtils] ParseBurnOptions: invalid line format: %{public}s", line.c_str());
            continue;
        }
        std::string key = TrimString(line.substr(0, equalPos));
        std::string value = TrimString(line.substr(equalPos + 1));
        if (!ParseKeyValuePair(key, value, parsedOptions)) {
            return E_PARAMS_INVALID;
        }
    }
    if (parsedOptions.diskName.empty() || parsedOptions.burnPath.empty() ||
        parsedOptions.fsType.empty()) {
        LOGE("[L3:DiskUtils] ParseBurnOptions: missing required fields - "
             "diskName=%{public}s, burnPath=%{public}s, fsType=%{public}s",
             parsedOptions.diskName.empty() ? "<missing>" : "<present>",
             parsedOptions.burnPath.empty() ? "<missing>" : "<present>",
             parsedOptions.fsType.empty() ? "<missing>" : "<present>");
        return E_PARAMS_INVALID;
    }
    LOGI("[L3:DiskUtils] ParseBurnOptions: <<< EXIT SUCCESS <<< diskName=%{public}s, burnPath=%{public}s, "
         "fsType=%{public}s, burnSpeed=%{public}s, isIsoImage=%{public}d, isIncBurnSupport=%{public}d, "
         "isVerifyBurn=%{public}d",
         parsedOptions.diskName.c_str(), parsedOptions.burnPath.c_str(), parsedOptions.fsType.c_str(),
         parsedOptions.burnSpeed.c_str(), parsedOptions.isIsoImage, parsedOptions.isIncBurnSupport,
         parsedOptions.isVerifyBurn);
    return E_OK;
}

static bool IsValidBlockDevicePath(const std::string& devPath)
{
    if (devPath.empty() || devPath.length() >= PATH_MAX_LEN) {
        LOGE("Invalid device path: empty or too long");
        return false;
    }
    if (devPath.find(BLOCK_DEVICE_PREFIX) != 0) {
        LOGE("Invalid device path: must start with /dev/block/");
        return false;
    }

    char realPath[PATH_MAX] = {0};
    if (realpath(devPath.c_str(), realPath) == nullptr) {
        char pathBuf[PATH_MAX] = {0};
        if (strncpy_s(pathBuf, PATH_MAX, devPath.c_str(), PATH_MAX - 1) != EOK) {
            LOGE("Invalid device path: strncpy_s failed");
            return false;
        }
        std::string dirPath = dirname(pathBuf);
        if (realpath(dirPath.c_str(), realPath) == nullptr) {
            LOGE("Invalid device path: parent directory doesn't exist");
            return false;
        }
        std::string resolvedParent(realPath);
        if (resolvedParent != "/dev/block" && resolvedParent.find(BLOCK_DEVICE_PREFIX) != 0) {
            LOGE("Invalid device path: resolved parent escapes /dev/block/");
            return false;
        }
    } else {
        struct stat st;
        if (stat(devPath.c_str(), &st) == 0 && !S_ISBLK(st.st_mode)) {
            LOGE("Invalid device path: not a block device");
            return false;
        }
    }
    return true;
}

int32_t DiskUtils::CreateBlockDeviceNode(const std::string& devPath,
                                         uint32_t mode,
                                         int32_t major,
                                         int32_t minor)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::CreateBlockDeviceNode invalid devPath");
        return E_PARAMS_INVALID;
    }
    if (major < DEVICE_MAJOR_MIN || major > DEVICE_MAJOR_MAX) {
        LOGE("DiskUtils::CreateBlockDeviceNode invalid major=%{public}d, valid range [%{public}d, %{public}d]",
             major, DEVICE_MAJOR_MIN, DEVICE_MAJOR_MAX);
        return E_PARAMS_INVALID;
    }
    if (minor < DEVICE_MINOR_MIN || minor > DEVICE_MINOR_MAX) {
        LOGE("DiskUtils::CreateBlockDeviceNode invalid minor=%{public}d, valid range [%{public}d, %{public}d]",
             minor, DEVICE_MINOR_MIN, DEVICE_MINOR_MAX);
        return E_PARAMS_INVALID;
    }

    dev_t dev = makedev(static_cast<unsigned int>(major), static_cast<unsigned int>(minor));
    if (mknod(devPath.c_str(), mode | S_IFBLK, dev) < 0) {
        if (errno == EEXIST) {
            LOGW("DiskUtils::CreateBlockDeviceNode node already exists, path=%{public}s", devPath.c_str());
            return E_OK;
        }
        LOGE("DiskUtils::CreateBlockDeviceNode mknod failed, path=%{public}s, errno=%{public}d",
             devPath.c_str(), errno);
        return E_ERR;
    }
    return E_OK;
}

int32_t DiskUtils::DestroyBlockDeviceNode(const std::string& devPath)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::DestroyBlockDeviceNode invalid devPath");
        return E_PARAMS_INVALID;
    }

    if (TEMP_FAILURE_RETRY(unlink(devPath.c_str())) < 0) {
        if (errno == ENOENT) {
            LOGI("DiskUtils::DestroyBlockDeviceNode node does not exist, path=%{public}s", devPath.c_str());
            return E_OK;
        }
        LOGE("DiskUtils::DestroyBlockDeviceNode unlink failed, errno=%{public}d", errno);
        return E_ERR;
    }
    return E_OK;
}

int32_t DiskUtils::ReadPartitionTable(const std::string& devPath,
                                      std::string& output,
                                      int32_t& maxVolume)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::ReadPartitionTable invalid devPath");
        return E_PARAMS_INVALID;
    }
    struct stat st;
    bool statOk = (stat(devPath.c_str(), &st) == 0);
    if (statOk && major(st.st_rdev) == DISK_CD_MAJOR) {
        LOGW("DiskUtils::ReadPartitionTable skip CD/DVD/BD device, devPath=%{public}s", devPath.c_str());
        return E_NOT_SUPPORT;
    }
    output.clear();
    maxVolume = 0;
    std::vector<std::string> cmd = {
        SGDISK_PATH,
        SGDISK_DUMP_CMD,
        devPath
    };
    std::vector<std::string> lines;
    int32_t res = ForkExec(cmd, &lines);
    if (res != E_OK) {
        LOGE("DiskUtils::ReadPartitionTable ForkExec failed, err=%{public}d", res);
        return res;
    }

    std::string fullContent;
    for (const auto &line : lines) {
        fullContent += line;
    }
    std::istringstream iss(fullContent);
    std::string oneLine;
    while (std::getline(iss, oneLine)) {
        output += oneLine;
        output += "\n";
    }
    if (statOk) {
        maxVolume = GetMaxVolume(st.st_rdev);
    } else {
        maxVolume = MAX_SCSI_VOLUMES;
    }

    LOGI("DiskUtils::ReadPartitionTable success, outputLen=%{public}zu, maxVolume=%{public}d",
         output.size(), maxVolume);
    return E_OK;
}

int32_t DiskUtils::Partition(const std::string& diskPath,
                             const std::string& partitionType)
{
    if (!IsValidBlockDevicePath(diskPath)) {
        LOGE("DiskUtils::Partition invalid diskPath");
        return E_PARAMS_INVALID;
    }
    if (partitionType == "hmfs") {
        return PartitionHmfs(diskPath);
    }

    constexpr const char *SGDISK_ZAP_CMD = "--zap-all";
    constexpr const char *SGDISK_PART_CMD = "--new=0:0:-0 --typecode=0:0c00 --gpttombr=1";
    std::vector<std::string> zapCmd = {
        SGDISK_PATH,
        SGDISK_ZAP_CMD,
        diskPath
    };
    std::vector<std::string> output;
    int32_t ret = ForkExec(zapCmd, &output);
    for (auto &str : output) {
        LOGI("DiskUtils::Partition zap: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("DiskUtils::Partition sgdisk zap failed, ret=%{public}d", ret);
        return ret;
    }

    output.clear();
    std::vector<std::string> partCmd = {
        SGDISK_PATH,
        SGDISK_PART_CMD,
        diskPath
    };
    ret = ForkExec(partCmd, &output);
    for (auto &str : output) {
        LOGI("DiskUtils::Partition new: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("DiskUtils::Partition sgdisk partition failed, ret=%{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t DiskUtils::GetPartitionTableInfo(const std::string &devPath, std::string &execRet)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::GetPartitionTableInfo invalid devPath");
        return E_PARAMS_INVALID;
    }
    std::vector<std::string> lines;
    int32_t ret = ExecAsyncGetPartitionTableInfo(devPath, lines);
    if (ret != E_OK) {
        return ret;
    }
    std::string fullContent;
    for (const auto &line : lines) {
        fullContent += line;
    }
    std::istringstream iss(fullContent);
    std::string oneLine;
    while (std::getline(iss, oneLine)) {
        execRet += oneLine;
        execRet += "\n";
        LOGI("partition line: %{public}s", oneLine.c_str());
    }
    return E_OK;
}

int32_t DiskUtils::CreatePartition(const std::string &devPath, int32_t partitionNum, int64_t startSector,
                                   int64_t endSector, const std::string &typeCode)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::CreatePartition invalid devPath");
        return E_PARAMS_INVALID;
    }
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread partitionThread([devPath, partitionNum, startSector, endSector, typeCode,
                                 p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec create partition");
        std::string partNum = std::to_string(partitionNum);
        std::string sector = partNum + ":" + std::to_string(startSector) + ":" + std::to_string(endSector);
        std::string code = partNum + ":" + typeCode;
        std::vector<std::string> cmd = {SGDISK_PATH, "-n", sector, "-t", code, "-W", "always", devPath};
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("create partition output: %{public}s", str.c_str());
        }
        p.set_value(ret);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskUtils] exec create partition: <<< EXIT FAILED <<< timed out");
        partitionThread.detach();
        return E_CREATE_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    partitionThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskUtils] CreatePartitionInfo: <<< EXIT FAILED <<< create partition failed, err=%{public}d", ret);
        return E_CREATE_PARTITION_ERROR;
    }
    LOGI("[L3:DiskUtils] CreatePartitionInfo: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskUtils::DeletePartitionInfo(const std::string &devPath, const std::string &diskId, int32_t partitionNum)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::DeletePartitionInfo invalid devPath");
        return E_PARAMS_INVALID;
    }
    LOGI("damage partition start");
    ExecAsyncDamagePartition(devPath, partitionNum);
    LOGI("damage partition end, delete partition start");
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread partitionThread([diskId, partitionNum, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec delete partition");
        std::vector<std::string> cmd = {SGDISK_PATH, "-d", std::to_string(partitionNum), "/dev/block/" + diskId};
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("delete partition output: %{public}s", str.c_str());
        }
        p.set_value(ret);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskUtils] exec delete partition: <<< EXIT FAILED <<< timed out");
        partitionThread.detach();
        return E_DELETE_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    partitionThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskUtils] DeletePartitionInfo: <<< EXIT FAILED <<< delete partition failed, err=%{public}d", ret);
        return E_DELETE_PARTITION_ERROR;
    }
    LOGI("[L3:DiskUtils] DeletePartitionInfo: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskUtils::ExecAsyncDamagePartition(const std::string &devPath, int32_t partitionNum)
{
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread partitionThread([devPath, partitionNum, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec damage partition");
        std::string of = "of=" + devPath + std::to_string(partitionNum);
        std::vector<std::string> cmd = {"dd", "if=/dev/zero", of, "bs=1M", "count=1"};
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("damage partition output: %{public}s", str.c_str());
        }
        p.set_value(ret);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskInfo] exec damage partition: <<< EXIT FAILED <<< timed out");
        partitionThread.detach();
        return E_DELETE_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    partitionThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskUtils] ExecAsyncDamagePartition: <<< EXIT FAILED <<< damage failed,err=%{public}d", ret);
        return E_DELETE_PARTITION_ERROR;
    }
    LOGI("[L3:DiskUtils] ExecAsyncDamagePartition: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskUtils::ExecAsyncGetPartitionTableInfo(const std::string &devPath, std::vector<std::string> &lines)
{
    std::promise<std::pair<int32_t, std::vector<std::string>>> promise;
    std::future<std::pair<int32_t, std::vector<std::string>>> future = promise.get_future();
    std::thread partitionThread([devPath, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec get partition");
        std::vector<std::string> temp;
        std::vector<std::string> cmd = {"sgdisk", "-p", devPath};
        int32_t res = ForkExec(cmd, &temp);
        std::string errorMsg;
        for (const auto& str : temp) {
            if (str.find("ERROR") != std::string::npos) {
                errorMsg += str;
            }
            LOGI("get partition output: %{public}s", str.c_str());
        }
        if (!errorMsg.empty()) {
            StorageService::StorageRadar::ReportUserManager("GetPartition", 0, E_GET_PARTITION_ERROR, errorMsg);
        }
        p.set_value({res, std::move(temp)});
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskInfo] exec get partition: <<< EXIT FAILED <<< time out");
        partitionThread.detach();
        return E_GET_PARTITION_TIMEOUT;
    }
    auto result = future.get();
    partitionThread.join();
    if (result.first != E_OK) {
        LOGE("[L3:DiskUtils] ExecAsyncGetPartitionTableInfo: <<< EXIT FAILED <<< exec failed, err=%{public}d",
             result.first);
        return E_GET_PARTITION_ERROR;
    }
    lines = std::move(result.second);
    LOGI("[L3:DiskUtils] ExecAsyncGetPartitionTableInfo: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskUtils::FormatPartition(const std::string &devPath, const std::string &fsType,
                                   const std::string &volumeName, bool quickFormat)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::FormatPartitionInfo invalid devPath");
        return E_PARAMS_INVALID;
    }
    if (formatTypeMap_.find(fsType) == formatTypeMap_.end()) {
        LOGE("DiskUtils::FormatPartitionInfo unsupported fsType=%{public}s", fsType.c_str());
        return E_FORMAT_PARTITION_NOT_SUPPORT;
    }
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread formatThread([devPath, fsType, volumeName, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec format partition");
        std::vector<std::string> cmd = GetFormatCMD(fsType, devPath, volumeName);
        if (!cmd.empty()) {
            std::vector<std::string> output;
            int32_t ret = ForkExec(cmd, &output);
            for (auto str : output) {
                LOGI("format partition output: %{public}s", str.c_str());
            }
            p.set_value(ret);
        } else {
            p.set_value(E_FORMAT_PARTITION_ERROR);
        }
    });
    if (future.wait_for(std::chrono::seconds(FORMAT_PARTITION_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskUtils] exec format partition: <<< EXIT FAILED <<< timed out");
        formatThread.detach();
        return E_FORMAT_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    formatThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskUtils] FormatPartitionInfo: <<< EXIT FAILED <<< format partition failed, err=%{public}d", ret);
        return E_FORMAT_PARTITION_ERROR;
    }
    LOGI("[L3:DiskUtils] FormatPartitionInfo: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::vector<std::string> DiskUtils::GetFormatCMD(const std::string &fsType, const std::string &devPath,
                                                 const std::string &volName)
{
    std::vector<std::string> cmd;
    if (fsType == "vfat") {
        if (volName.empty()) {
            cmd = {formatTypeMap_.find(fsType)->second, "-A", devPath};
        } else {
            cmd = {formatTypeMap_.find(fsType)->second, "-L", volName, "-A", devPath};
        }
    } else if (fsType == "ext4") {
        if (volName.empty()) {
            cmd = {formatTypeMap_.find(fsType)->second, "-t", "ext4", devPath};
        } else {
            cmd = {formatTypeMap_.find(fsType)->second, "-L", volName, "-t", "ext4", devPath};
        }
    } else if (fsType == "exfat") {
        if (volName.empty()) {
            cmd = {formatTypeMap_.find(fsType)->second, devPath};
        } else {
            cmd = {formatTypeMap_.find(fsType)->second, "-L", volName, devPath};
        }
    }
    return cmd;
}

int ExecuteScsiCmd(int fd, uint8_t *cdb, int cdbLen, uint8_t *dxferp, int dxferLen)
{
    LOGD("ExecuteScsiCmd: >>> ENTER <<< fd=%{public}d", fd);
    sg_io_hdr_t ioHdr;
    uint8_t sense[SENSE_BUFF_LEN];

    if (memset_s(&ioHdr, sizeof(ioHdr), 0, sizeof(ioHdr)) != 0) {
        LOGE("ExecuteScsiCmd: <<< EXIT FAILED <<< ioHdr memset_s failed");
        return E_ERR;
    }
    if (memset_s(sense, sizeof(sense), 0, sizeof(sense)) != 0) {
        LOGE("ExecuteScsiCmd: <<< EXIT FAILED <<< sense memset_s failed");
        return E_ERR;
    }
    ioHdr.interface_id = 'S';
    ioHdr.dxfer_direction = SG_DXFER_FROM_DEV;
    ioHdr.cmdp = cdb;
    ioHdr.cmd_len = cdbLen;
    ioHdr.dxferp = dxferp;
    ioHdr.dxfer_len = static_cast<unsigned int>(dxferLen);
    ioHdr.mx_sb_len = sizeof(sense);
    ioHdr.sbp = sense;
    ioHdr.timeout = DEF_TIMEOUT;
    if (ioctl(fd, SG_IO, &ioHdr) < 0) {
        LOGE("ExecuteScsiCmd: <<< EXIT FAILED <<< ioctl SG_IO failed");
        return E_ERR;
    }
    if ((ioHdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
        LOGE("ExecuteScsiCmd: <<< EXIT FAILED <<< SG_INFO not OK");
        return E_ERR;
    }
    LOGD("ExecuteScsiCmd: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int ReadCDDiscInfo(const std::string &diskPath, int32_t cmdIndex, uint8_t *buf, int len)
{
    LOGI("ReadCDDiscInfo: >>> ENTER <<< diskPath=%{public}s, len=%{public}d", diskPath.c_str(), len);
    char realPath[PATH_MAX] = { 0 };
    if (realpath(diskPath.c_str(), realPath) == nullptr) {
        LOGE("ReadCDDiscInfo: <<< EXIT FAILED <<< realpath failed");
        return E_ERR;
    }
    FILE* file = fopen(realPath, "rb");
    if (file == nullptr) {
        LOGE("ReadCDDiscInfo: <<< EXIT FAILED <<< fopen failed, errno=%{public}d", errno);
        return E_ERR;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("ReadCDDiscInfo: <<< EXIT FAILED <<< fileno error=%{public}d", errno);
        (void)fclose(file);
        return E_ERR;
    }

    uint8_t cdb[READ_DISC_INFO_CDB_LEN] = { cmdIndex };
    cdb[CDB_ALLOCATION_LENGTH_HIGH] = static_cast<uint8_t>(static_cast<uint32_t>(len) >> CDB_ALLOCATION_LENGTH_LOW);
    cdb[CDB_ALLOCATION_LENGTH_LOW] = static_cast<uint8_t>(static_cast<uint32_t>(len) & MAX_ALLOC_LEN);

    int ret = ExecuteScsiCmd(fd, cdb, sizeof(cdb), buf, len);
    if (ret != 0) {
        LOGE("ReadCDDiscInfo: <<< EXIT FAILED <<< ExecuteScsiCmd failed, err=%{public}d", ret);
        (void)fclose(file);
        return ret;
    }
    (void)fclose(file);
    LOGI("ReadCDDiscInfo: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int GetCDDiskStatus(const char *device, int &status)
{
    char realPath[PATH_MAX] = { 0 };
    if (realpath(device, realPath) == nullptr) {
        LOGE("realpath failed.");
        return E_FILE_PATH_INVALID;
    }
    FILE* file = fopen(realPath, "rb");
    if (file == nullptr) {
        LOGE("fopen failed errStr:%{public}s errno:%{public}d", strerror(errno), errno);
        return E_SYS_KERNEL_ERR;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("fileno failed, errStr:%{public}s errno:%{public}d", strerror(errno), errno);
        (void)fclose(file);
        return E_SYS_KERNEL_ERR;
    }

    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int slot = INT_MAX;
    status = ioctl(fd, CDROM_DRIVE_STATUS, &slot);
    if (status < 0) {
        LOGE("CD status:%{public}d errStr:%{public}s errno:%{public}d", status, strerror(errno), errno);
        (void)fclose(file);
        return E_ERR;
    }
    (void)fclose(file);
    auto delay = StorageService::StorageRadar::ReportDuration("GET CD STATUS: CD ROM",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, 0);
    LOGW("SD_DURATION: GET CD STATUS: device=%{public}s, delay = %{public}s", device, delay.c_str());
    return E_OK;
}

int IsCDExist(const std::string &diskPath, bool &isCDExist)
{
    LOGI("IsCDExist: >>> ENTER <<< diskPath=%{public}s", diskPath.c_str());
    int status = 0;
    isCDExist = false;
    int ret = GetCDDiskStatus(diskPath.c_str(), status);
    if (ret != E_OK) {
        LOGE("Unknown cd status !");
        return E_ERR;
    }
    isCDExist = (status == CDS_DRIVE_NOT_READY || status == CDS_DISC_OK);
    LOGI("IsCDExist:Current CD statue, IsCDExist: %{public}d", isCDExist);
    return E_OK;
}

int IsCDBlank(const std::string &diskPath, bool &isCDBlank)
{
    isCDBlank = false;
    uint8_t buf[MAX_BUF];
    
    if (ReadCDDiscInfo(diskPath, READ_DISC_INFO_OPCODE, buf, sizeof(buf)) != E_OK) {
        LOGE("IsCDBlank: Unable to read disc information.");
        return E_ERR;
    }

    uint8_t discStatus = buf[DISC_STATUS_BYTE_INDEX] & DISC_STATUS_MASK;
    std::string diskType = GetCDType(diskPath);
    if (discStatus == 0) {
        isCDBlank = true;
    } else if (diskType == "DVD+RW" || diskType == "DVD-RW" || diskType == "BD-RE") {
        std::string fsType = GetBlkidData(diskPath, "TYPE");
        isCDBlank = fsType.empty();
        LOGI("IsCDBlank: %{public}s has filesystem=%{public}s", diskType.c_str(), fsType.c_str());
    }
    
    LOGI("IsCDBlank: <<< EXIT SUCCESS <<< isBlank=%{public}d", isCDBlank);
    return E_OK;
}

int32_t DiskUtils::QueryCDStatus(const std::string &devPath, int32_t &status)
{
    LOGI("QueryCDStatus: >>> ENTER <<< devPath=%{public}s", devPath.c_str());

    bool isCDExist = false;

    int existRet = IsCDExist(devPath, isCDExist);
    if (existRet != E_OK) {
        LOGE("QueryCDStatus: IsCDExist failed, ret=%{public}d", existRet);
        return E_ERR;
    }

    if (!isCDExist) {
        status = 0;
        LOGI("QueryCDStatus: No CD found, status = %{public}d", status);
        return E_OK;
    }

    bool isCDBlank = false;
    int blankRet = IsCDBlank(devPath, isCDBlank);
    if (blankRet != E_OK) {
        LOGE("QueryCDStatus: IsCDBlank failed, ret=%{public}d", blankRet);
        return E_ERR;
    }

    status = (isCDExist ? 1 : 0) | ((isCDBlank ? 1 : 0) << 1);
    LOGI("QueryCDStatus: <<< EXIT SUCCESS <<< status = %{public}d", status);
    return E_OK;
}

int32_t DiskUtils::EjectCD(const std::string &devPath)
{
    LOGI("EjectCD: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    std::vector<std::string> output;
    std::vector<std::string> cmd = {
        "eject",
        "-s",
        devPath
    };
    int res = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("EjectCD output: %{public}s", str.c_str());
    }
    if (res != E_OK) {
        LOGE("EjectCD: <<< EXIT FAILED <<< ForkExec eject failed, err=%{public}d", res);
        return res;
    }
    LOGI("EjectCD: <<< EXIT SUCCESS <<<");
    return E_OK;
}
int32_t DiskUtils::PartitionHmfs(const std::string& diskPath)
{
    std::vector<std::string> clearCmd = {
        SGDISK_PATH,
        "-zog",
        diskPath
    };
    std::vector<std::string> output;
    int32_t ret = ForkExec(clearCmd, &output);
    for (auto &str : output) {
        LOGI("DiskUtils::PartitionHmfs clear: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("DiskUtils::PartitionHmfs sgdisk clear failed, ret=%{public}d", ret);
        return ret;
    }

    output.clear();
    std::vector<std::string> partCmd = {
        SGDISK_PATH,
        "--new=0:0:-0",
        "--typecode=0:8300",
        diskPath
    };
    ret = ForkExec(partCmd, &output);
    for (auto &str : output) {
        LOGI("DiskUtils::PartitionHmfs new: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("DiskUtils::PartitionHmfs sgdisk partition failed, ret=%{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t DiskUtils::Erase(const std::string &devPath)
{
    LOGI("Erase: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    int32_t res = CleanTempDirectory();
    if (res != E_OK) {
        LOGE("Erase: CleanTempDirectory entry failed, non-critical, res=%{public}d", res);
    }
    std::vector<std::string> output;
    std::vector<std::string> cmd = {};
    std::string oddLabel = GetCDType(devPath);
    if (oddLabel.find("CD") != std::string::npos) {
        std::string wodimPath = "dev=" + devPath;
        cmd = {"wodim", wodimPath, "blank=fast"};
    } else if (oddLabel == "DVD+RW") {
        cmd = {"dvd+rw-format", "-force", devPath};
    } else if (oddLabel == "DVD-RW") {
        cmd = {"dvd+rw-format", "-force=full", devPath};
    } else {
        cmd = {"dvd+rw-format", "-blank", devPath};
    }
    int err = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("Erase output: %{public}s", str.c_str());
    }
    if (err != E_OK) {
        LOGE("Erase: <<< EXIT FAILED <<< ForkExec erase failed, err=%{public}d", err);
        return err;
    }
    res = CleanTempDirectory();
    if (res != E_OK) {
        LOGE("Erase: CleanTempDirectory exit failed, non-critical, res=%{public}d", res);
    }
    LOGI("Erase: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskUtils::Eject(const std::string &devName)
{
    LOGI("Eject: >>> ENTER <<< devName=%{public}s", devName.c_str());
    std::string deviceLinkPath = "/sys/block/" + devName;

    char linkTarget[PATH_MAX] = {0};
    ssize_t len = readlink(deviceLinkPath.c_str(), linkTarget, sizeof(linkTarget) - 1);
    if (len <= 0) {
        LOGE("Eject: Failed to read symlink %{public}s, errno=%{public}d", deviceLinkPath.c_str(), errno);
        return E_ERR;
    }
    linkTarget[len] = '\0';
    std::string linkStr(linkTarget);
    std::regex targetRegex(R"(target(\d+:\d+:\d+))");
    std::smatch match;
    if (!std::regex_search(linkStr, match, targetRegex) || match.size() <= 1) {
        LOGE("Eject: Failed to extract SCSI device from path: %{public}s", linkStr.c_str());
        return E_ERR;
    }
    std::string scsiDev = match[1].str();
    std::replace(scsiDev.begin(), scsiDev.end(), ':', ',');
    LOGI("Eject: Extracted SCSI device: %{public}s", scsiDev.c_str());
    std::vector<std::string> output;
    std::vector<std::string> cmd = {
        "wodim",
        "-d",
        "-v",
        "-eject",
        "dev=" + scsiDev
    };
    int res = ForkExec(cmd, &output);
    for (const auto &str : output) {
        LOGI("Eject output: %{public}s", str.c_str());
    }
    if (res != E_OK) {
        LOGE("Eject: <<< EXIT FAILED <<< ForkExec failed, err=%{public}d", res);
        return res;
    }
    LOGI("Eject: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ValidateBurnOptions(const BurnOptions &options)
{
    LOGI("[L3:DiskUtils] ValidateBurnOptions: >>> ENTER <<< "
         "diskName=%{public}s, burnPath=%{public}s, fsType=%{public}s",
         options.diskName.c_str(), options.burnPath.c_str(), options.fsType.c_str());
    if (options.diskName.empty()) {
        LOGE("[L3:DiskUtils] ValidateBurnOptions: diskName is empty");
        return E_PARAMS_INVALID;
    }
    struct stat st;
    if (stat(options.burnPath.c_str(), &st) != 0) {
        LOGE("[L3:DiskUtils] ValidateBurnOptions: burnPath does not exist: %{public}s, errno=%{public}d",
             options.burnPath.c_str(), errno);
        return E_PARAMS_INVALID;
    }
    if (options.isIsoImage) {
        if (!S_ISREG(st.st_mode)) {
            LOGE("[L3:DiskUtils] ValidateBurnOptions: isIsoImage=true but burnPath is not a regular file: %{public}s",
                 options.burnPath.c_str());
            return E_PARAMS_INVALID;
        }
    } else {
        if (!S_ISDIR(st.st_mode)) {
            LOGE("[L3:DiskUtils] ValidateBurnOptions: isIsoImage=false but burnPath is not a directory: %{public}s",
                 options.burnPath.c_str());
            return E_PARAMS_INVALID;
        }
    }
    LOGI("[L3:DiskUtils] ValidateBurnOptions: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::string GetLastNumberSimple(const std::vector<std::string>& lines)
{
    LOGI("GetLastNumberSimple: >>> ENTER <<<");
    std::vector<std::string> allLines;
    for (const auto& item : lines) {
        if (item.find('\n') != std::string::npos) {
            std::istringstream iss(item);
            std::string line;
            while (std::getline(iss, line)) {
                allLines.push_back(line);
            }
        } else {
            allLines.push_back(item);
        }
    }

    std::regex pattern(R"(^\s*\d+,\d+\s*$)");
    for (const auto& line : allLines) {
        if (std::regex_match(line, pattern)) {
            auto start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return "";
            auto end = line.find_last_not_of(" \t\r\n");
            return line.substr(start, end - start + 1);
        }
    }

    return "";
}
int32_t GetIncBurnAddr(const std::string &devPath, std::string &incBurnAddr)
{
    LOGI("BurnGetIncBurnAddr: >>> ENTER <<< devPath=%{public}s", devPath.c_str());

    int32_t err = 0;
    std::vector<std::string> cmd;
    std::vector<std::string> output;

    cmd = {"wodim", devPath, "-msinfo"};
    err = ForkExec(cmd, &output);
    if (err != E_OK) {
        for (const auto& s : output) {
            LOGI("BurnGetIncBurnAddr:s=%{public}s", s.c_str());
        }
        LOGE("BurnGetIncBurnAddr:<<< EXIT FAILED <<< failed for devPath: %{public}s",
            devPath.c_str());
        return err;
    }
    incBurnAddr = GetLastNumberSimple(output);
    LOGI("BurnGetIncBurnAddr:<<< EXIT SUCCESS <<<"
         "devPath=%{public}s, incBurnAddr=%{public}s",
         devPath.c_str(), incBurnAddr.c_str());
    return err;
}

static int32_t GetLatestProgressFromFile(const char* filePath, int32_t &progress)
{
    progress = 0;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOGE("GetLatestProgressFromFile:<<< EXIT FAILED <<< open failed");
        return E_NOT_SUPPORT;
    }
    std::string content;
    std::getline(file, content);
    if (file.fail() && !file.eof()) {
        LOGE("GetLatestProgressFromFile:<<< EXIT FAILED <<< getline failed");
        return E_NOT_SUPPORT;
    }
    if (content.empty()) {
        LOGE("GetLatestProgressFromFile:<<< EXIT FAILED <<< empty content");
        return E_NOT_SUPPORT;
    }
    LOGI("GetLatestProgressFromFile content = %{public}s", content.c_str());
    std::istringstream iss(content);
    int32_t tempProgress = 0;
    iss >> tempProgress;
    if (iss.fail()) {
        LOGE("GetLatestProgressFromFile:<<< EXIT FAILED <<< iss failed");
        return E_NOT_SUPPORT;
    }
    progress = tempProgress;
    return E_OK;
}

int32_t DiskUtils::GetVolumeOpProcess(const std::string &volId, int32_t &progressPct)
{
    LOGI("GetVolumeOpProcess: >>> ENTER <<< volId=%{public}s", volId.c_str());
    int32_t err = 0;

    std::string filePath;
    if (!GetRealPath("/data/local/vol_tmp/percent", filePath)) {
        LOGE("GetVolumeOpProcess:<<< EXIT FAILED <<< volId: %{public}s",
            volId.c_str());
        return E_PARAMS_INVALID;
    }
    if (IsFilePathInvalid(filePath)) {
        LOGE("GetVolumeOpProcess:<<< EXIT FAILED <<<filePath: %{public}s",
            filePath.c_str());
        return E_PARAMS_INVALID;
    }
    LOGI("GetVolumeOpProcess filePath = %{public}s", filePath.c_str());

    err = GetLatestProgressFromFile(filePath.c_str(), progressPct);
    if (err != E_OK) {
        LOGE("GetVolumeOpProcess:<<< EXIT FAILED <<< volId: %{public}s",
            volId.c_str());
        return err;
    }
    LOGI("GetVolumeOpProcess:<<< EXIT SUCCESS <<< volId=%{public}s, progressPct=%{public}d",
        volId.c_str(), progressPct);
    return err;
}

int32_t DiskUtils::VerifyBurnData(const std::string &devPath, int32_t verifyType)
{
    LOGI("VerifyBurnData:<<< ENTER <<< devPath=%{public}s, verifyType=%{public}d", devPath.c_str(), verifyType);
    return E_OK;
}

static int32_t GetDvdPlusRwTotalCapacity(int fd, int64_t &dvdTotalCapacity)
{
    unsigned char cmdBuf[GET_CAPACITY_CMD_BUF_LEN] = {0};
    int cmdLen = GET_DVD_USED_CAPACITY_CMD_LEN;
    unsigned char dataBuf[GET_CAPACITY_DATA_BUF_LEN] = {0};
    unsigned int dataLen = GET_DVD_USED_CAPACITY_DATA_LEN;
    int ret = 0;
    unsigned int blkCnt = 0;
    unsigned int blkSize = 0;
    /*
     * 使用 SCSI READ CAPACITY 指令 (0x25) 获取DVD+RW光盘容量信息。
     * 字段详解:
     * cmdBuf[0]: 操作码 0x25 (GPCMD_READ_CDVD_CAPACITY)。
     *             用于请求光盘的最后逻辑块地址 (Last LBA) 和块大小 (Block Length)。
     * cmdBuf[7-8]: 分配长度 (Allocation Length)。
     *               告知驱动器返回数据的最大长度（通常为 8 字节）。
     *               高 8 位填入 cmdBuf[7]，低 8 位填入 cmdBuf[8]。
     * 返回数据:
     * dataBuf[0-3]: 最后逻辑块地址 (Last LBA)。
     * dataBuf[4-7]: 块大小 (Block Length)。
     * 注意：此指令返回的 LBA 需加 1 才是总扇区数。
     */
    cmdBuf[0] = GPCMD_READ_CDVD_CAPACITY;
    cmdBuf[CDB_ALLOCATION_LENGTH_HIGH] = (dataLen >> BYTE_SHIFT_8) & BYTE_MASK;
    cmdBuf[CDB_ALLOCATION_LENGTH_LOW] = dataLen & BYTE_MASK;
    ret = SendScsiCmd(fd, cmdBuf, cmdLen, dataBuf, dataLen);
    if (ret != 0) {
        LOGE("GetDvdPlusRwTotalCapacity SendScsiCmd failed, ret val is %{public}d", ret);
        return E_ERR;
    }
 
    blkCnt = (static_cast<unsigned int>(dataBuf[LAST_LBA_BYTE_0]) << BYTE_SHIFT_24) |
             (static_cast<unsigned int>(dataBuf[LAST_LBA_BYTE_1]) << BYTE_SHIFT_16) |
             (static_cast<unsigned int>(dataBuf[LAST_LBA_BYTE_2]) << BYTE_SHIFT_8) |
             dataBuf[LAST_LBA_BYTE_3];
    blkSize = (static_cast<unsigned int>(dataBuf[BLOCK_SIZE_BYTE_0]) << BYTE_SHIFT_24) |
              (static_cast<unsigned int>(dataBuf[BLOCK_SIZE_BYTE_1]) << BYTE_SHIFT_16) |
              (static_cast<unsigned int>(dataBuf[BLOCK_SIZE_BYTE_2]) << BYTE_SHIFT_8) |
              dataBuf[BLOCK_SIZE_BYTE_3];
    dvdTotalCapacity = (static_cast<int64_t>(blkCnt) + 1) * static_cast<int64_t>(blkSize);
    LOGI("GetDvdPlusRwTotalCapacity used_capacity: %{public}u * %{public}u = %{public}" PRIu64,
        blkCnt + 1, blkSize, dvdTotalCapacity);
    return E_OK;
}

int32_t DiskUtils::GetDiscCapacity(int cmdFd, const std::string& discType,
                                   int64_t &totalSize, int64_t &usedSize)
{
    int err1 = E_OK;
    if (discType == "DVD-R"  || discType == "DVD+R") {
        err1 = GetDvdTotalCapacity(cmdFd, totalSize);
    } else if (discType == "DVD+RW" || discType == "DVD-RW") {
        err1 = GetDvdPlusRwTotalCapacity(cmdFd, totalSize);
    } else if (discType == "CD-ROM" || discType == "CD-RW" || discType == "CD-R") {
        err1 = GetCdTotalCapacity(cmdFd, totalSize);
    } else if (discType == "BD-R" || discType == "BD-RE") {
        err1 = GetBdTotalCapacity(cmdFd, totalSize);
    } else if (discType == "BD-ROM") {
        err1 = GetBdTotalCapacity(cmdFd, totalSize);
    } else {
        totalSize = 0;
        usedSize = 0;
    }
    usedSize = totalSize;
    if (err1 != E_OK) {
        LOGI("GetDiscCapacity failed, err1=%{public}d", err1);
        return E_ERR;
    }
    return E_OK;
}

void DiskUtils::AdjustBlankDiscCapacity(const std::string& devPath, const std::string& discType,
                                        int64_t &totalSize, int64_t &usedSize)
{
    bool isBlankDisc = false;
    int blankRet = IsCDBlank(devPath, isBlankDisc);
    if (blankRet != E_OK || !isBlankDisc) {
        return;
    }
    
    usedSize = 0;
    if (discType != "DVD+RW" && discType != "DVD-RW" && discType != "BD-R" && discType != "BD-RE") {
        return;
    }
    
    std::vector<std::string> cmd = {"dvd+rw-mediainfo", devPath};
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);
    if (ret != E_OK) {
        LOGE("AdjustBlankDiscCapacity: ForkExec failed, ret=%{public}d", ret);
        return;
    }
    
    std::regex pattern(R"(\bunformatted:\s+\d+\*2048=(\d+))");
    for (const auto& line : output) {
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            std::string subStr = match[1].str();
            const char *ptr = subStr.c_str();
            const char *end = ptr + subStr.size();
            int64_t mediaInfoCapacity = 0;
            auto result = std::from_chars(ptr, end, mediaInfoCapacity);
            if (result.ec == std::errc() && result.ptr == end && mediaInfoCapacity > 0) {
                totalSize = mediaInfoCapacity;
                LOGI("AdjustBlankDiscCapacity: DVD+RW capacity from mediainfo=%{public}" PRId64, totalSize);
            }
            return;
        }
    }
}

int32_t DiskUtils::GetCapacity(const std::string& devPath, int64_t &totalSize, int64_t &freeSize)
{
    LOGI("GetCapacity: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    int cmdFd = open(devPath.c_str(), O_RDONLY | O_NONBLOCK);
    if (cmdFd < 0) {
        LOGE("GetCapacity:<<< EXIT FAILED <<< open failed");
        return E_ERR;
    }
    std::string discType = GetCDType(devPath);
    LOGI("label is %{public}s", discType.c_str());
    int64_t usedSize = 0;
    int32_t ret = GetDiscCapacity(cmdFd, discType, totalSize, usedSize);
    close(cmdFd);

    AdjustBlankDiscCapacity(devPath, discType, totalSize, usedSize);
    LOGI("GetCapacity totalsize is %{public}" PRId64 ", usedsize is %{public}" PRId64, totalSize, usedSize);
    freeSize = (usedSize > totalSize) ? 0 : totalSize - usedSize;

    if (ret != E_OK) {
        LOGI("GetCapacity: <<< EXIT FAILED <<<");
    }
    return E_OK;
}

bool DiskUtils::IsMtpDeviceInUse(const std::string &diskPath)
{
    if (diskPath.rfind(MTP_PATH_PREFIX, 0) != 0) {
        return false;
    }

    std::string key = "user.queryMtpIsInUse";
    char value[MTP_QUERY_RESULT_LEN] = { 0 };
    int32_t len = getxattr(diskPath.c_str(), key.c_str(), value, MTP_QUERY_RESULT_LEN);
    if (len < 0) {
        LOGE("[L3:DiskUtils] IsMtpDeviceInUse: Failed to getxattr for diskPath = %{public}s", diskPath.c_str());
        return false;
    }

    if ("true" == std::string(value)) {
        LOGI("[L3:DiskUtils] IsMtpDeviceInUse: MTP device in use for diskPath=%{public}s", diskPath.c_str());
        return true;
    }
    return false;
}

int32_t DiskUtils::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
    LOGI("[L3:DiskUtils] QueryUsbIsInUse: >>> ENTER <<< diskPath=%{public}s", diskPath.c_str());

    isInUse = true;
    if (diskPath.empty()) {
        LOGE("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT FAILED <<< diskPath is empty");
        return E_PARAMS_NULLPTR_ERR;
    }
    char realPath[PATH_MAX] = { 0 };
    if (realpath(diskPath.c_str(), realPath) == nullptr) {
        LOGE("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT FAILED <<< realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    if (strncmp(realPath, diskPath.c_str(), diskPath.size()) != 0) {
        LOGE("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT FAILED <<< path mismatch");
        return E_PARAMS_INVALID;
    }
    if (IsMtpDeviceInUse(diskPath)) {
        isInUse = true;
        LOGI("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT SUCCESS <<< MTP device in use");
        return E_OK;
    }
    int fd = open(realPath, O_RDONLY);
    if (fd < 0) {
        LOGE("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        return E_OPEN_FAILED;
    }
    int inUse = -1;
    if (ioctl(fd, STORAGE_MANAGER_IOC_CHK_BUSY, &inUse) < 0) {
        LOGE("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT FAILED <<< ioctl failed, errno=%{public}d", errno);
        close(fd);
        return E_IOCTL_FAILED;
    }

    if (inUse) {
        LOGI("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT SUCCESS <<< inUse=%{public}d", inUse);
        close(fd);
        isInUse = true;
        return E_OK;
    }
    LOGI("[L3:DiskUtils] QueryUsbIsInUse: usb not inUse");
    isInUse = false;
    close(fd);
    LOGI("[L3:DiskUtils] QueryUsbIsInUse: <<< EXIT SUCCESS <<< not in use");
    return E_OK;
}

int32_t DiskUtils::CleanTempDirectory()
{
    LOGI("CleanTempDirectory: >>> ENTER <<<");
    std::vector<std::string> cmd = {"rm", "-rf", "/data/local/vol_tmp/percent"};
    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);
    if (err != E_OK) {
        for (const auto& s : output) {
            LOGI("CleanTempDirectory: output=%{public}s", s.c_str());
        }
        LOGE("CleanTempDirectory: <<< EXIT FAILED <<< rm -rf /data/local/vol_tmp/percent failed, err=%{public}d", err);
        return err;
    }
    LOGI("CleanTempDirectory: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::vector<std::string> DiskUtils::SplitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str);
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string DiskUtils::GetRelativePath(const std::string& fullPath, const std::string& baseDir)
{
    if (baseDir.empty()) {
        return fullPath;
    }
    size_t pos = fullPath.find(baseDir);
    if (pos == 0) {
        std::string relative = fullPath.substr(baseDir.size());
        if (!relative.empty() && relative[0] == '/') {
            return relative.substr(1);
        }
        return relative;
    }
    return fullPath;
}

std::vector<std::string> DiskUtils::MergeOutputLines(const std::vector<std::string>& output)
{
    std::vector<std::string> mergedLines;
    std::string accumulatedLine;
    for (const auto& line : output) {
        if (line.empty()) {
            if (!accumulatedLine.empty()) {
                mergedLines.push_back(accumulatedLine);
                accumulatedLine.clear();
            }
            continue;
        }
        size_t firstNonSpace = line.find_first_not_of(' ');
        char firstChar = (firstNonSpace != std::string::npos) ? line[firstNonSpace] : ' ';
        bool isContinuation = (firstChar != '-' && firstChar != 'd' &&
            line.find("Directory listing of") == std::string::npos &&
            line.find('[') == std::string::npos);
        if (isContinuation) {
            accumulatedLine = accumulatedLine.empty() ? line : accumulatedLine + " " + line;
        } else {
            if (!accumulatedLine.empty()) {
                mergedLines.push_back(accumulatedLine);
            }
            accumulatedLine = line;
        }
    }
    if (!accumulatedLine.empty()) {
        mergedLines.push_back(accumulatedLine);
    }
    return mergedLines;
}

std::string DiskUtils::ParseDirectoryPath(const std::string& line)
{
    size_t start = line.find("Directory listing of");
    if (start == std::string::npos) {
        return "";
    }
    size_t pathStart = line.find('/', start);
    if (pathStart == std::string::npos) {
        return "";
    }
    size_t pathEnd = line.find_last_of('/');
    if (pathEnd <= pathStart) {
        return "";
    }
    return line.substr(pathStart, pathEnd - pathStart);
}

bool DiskUtils::IsFileEntry(const std::string& line, char& entryType)
{
    size_t firstNonSpace = line.find_first_not_of(' ');
    if (firstNonSpace == std::string::npos) {
        return false;
    }
    std::string trimmedLine = line.substr(firstNonSpace);
    entryType = trimmedLine[0];
    return (entryType == '-' || entryType == 'd');
}

std::string DiskUtils::ParseFileName(const std::string& trimmedLine)
{
    size_t bracketEnd = trimmedLine.rfind(']');
    if (bracketEnd == std::string::npos) {
        return "";
    }
    std::string name = trimmedLine.substr(bracketEnd + 1);
    size_t nameStart = name.find_first_not_of(' ');
    if (nameStart != std::string::npos) {
        name = name.substr(nameStart);
    }
    size_t nameEnd = name.find_last_not_of(' ');
    if (nameEnd != std::string::npos) {
        name = name.substr(0, nameEnd + 1);
    }
    if (name.empty() || name == "." || name == "..") {
        return "";
    }
    size_t semicolonPos = name.find(';');
    if (semicolonPos != std::string::npos) {
        name = name.substr(0, semicolonPos);
    }
    return name;
}

int32_t DiskUtils::GenerateChecksums(const std::string& dirPath, const std::string& checksumFilePath)
{
    LOGI("GenerateChecksums: generating MD5 for %{public}s", dirPath.c_str());
    std::vector<std::string> cmd = {"find", dirPath, "-type", "f", "-exec", "md5sum", "{}", ";"};
    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);
    if (err != E_OK) {
        LOGE("GenerateChecksums: find/md5sum failed for %{public}s", dirPath.c_str());
        return E_ERR;
    }
    std::string checksumContent;
    for (const auto& s : output) {
        checksumContent += s + "\n";
    }
    std::string errMsg;
    if (!WriteFileSync(checksumFilePath.c_str(),
                       reinterpret_cast<const uint8_t*>(checksumContent.c_str()),
                       checksumContent.size(), errMsg)) {
        LOGE("GenerateChecksums: write checksum failed: %{public}s", errMsg.c_str());
        return E_ERR;
    }
    return E_OK;
}

std::map<std::string, std::string> DiskUtils::ParseChecksumFile(
    const std::string& checksumContent, const std::string& basePath)
{
    std::map<std::string, std::string> checksumMap;
    std::vector<std::string> lines = SplitString(checksumContent, '\n');
    for (const auto& line : lines) {
        if (line.empty()) {
            continue;
        }
        size_t pos = line.find("  ");
        if (pos != std::string::npos) {
            std::string md5 = line.substr(0, pos);
            std::string filePath = line.substr(pos + 2);
            std::string relativePath = GetRelativePath(filePath, basePath);
            checksumMap[relativePath] = md5;
        }
    }
    return checksumMap;
}

int32_t DiskUtils::CompareChecksums(
    const std::map<std::string, std::string>& sourceMap,
    const std::map<std::string, std::string>& discMap)
{
    constexpr int32_t E_VERIFY_BURN_DATA_FAILED = 13600030;
    LOGI("CompareChecksums: comparing, sourceFiles=%{public}zu, discFiles=%{public}zu",
         sourceMap.size(), discMap.size());
    for (const auto& pair : sourceMap) {
        const std::string& filename = pair.first;
        const std::string& sourceMd5 = pair.second;
        auto it = discMap.find(filename);
        if (it == discMap.end()) {
            LOGE("CompareChecksums: file not found on disc: %{public}s", filename.c_str());
            return E_VERIFY_BURN_DATA_FAILED;
        }
        if (sourceMd5 != it->second) {
            LOGE("CompareChecksums: MD5 mismatch for file: %{public}s", filename.c_str());
            return E_VERIFY_BURN_DATA_FAILED;
        }
    }
    LOGI("CompareChecksums: checksum comparison passed");
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
