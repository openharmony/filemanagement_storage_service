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

#include <climits>
#include <future>
#include <libgen.h>
#include <thread>

#include "disk_manager/disk/disk_utils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include "storage_service_errno.h"
#include "securec.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

constexpr const char *BLOCK_DEVICE_PREFIX = "/dev/block/";
constexpr const char *SGDISK_PATH = "/system/bin/sgdisk";
constexpr const char *SGDISK_DUMP_CMD = "--ohos-dump";
constexpr int32_t PATH_MAX_LEN = 4096;
constexpr int32_t DEVICE_MAJOR_MIN = 0;
constexpr int32_t DEVICE_MAJOR_MAX = 4095;
constexpr int32_t DEVICE_MINOR_MIN = 0;
constexpr int32_t DEVICE_MINOR_MAX = 1048575;
constexpr int32_t WAIT_THREAD_TIMEOUT_S = 60;
const std::map<std::string, std::string> formatTypeMap_ = {
    {"exfat", "mkfs.exfat"},
    {"vfat", "newfs_msdos"},
    {"ext4", "mke2fs"},
};

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

    for (const auto &line : lines) {
        output += line;
        output += "\n";
    }
    struct stat st;
    if (stat(devPath.c_str(), &st) == 0) {
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
    std::promise<std::pair<int32_t, std::vector<std::string>>> promise;
    std::future<std::pair<int32_t, std::vector<std::string>>> future = promise.get_future();
    std::thread partitionThread([devPath, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec get partition");
        std::vector<std::string> temp;
        std::vector<std::string> cmd = {"sgdisk", "-p", devPath};
        int32_t res = ForkExec(cmd, &temp);
        for (auto str : temp) {
            LOGI("get partition output: %{public}s", str.c_str());
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
    int32_t ret = result.first;
    if (ret != E_OK) {
        LOGE("[L3:DiskInfo] GetPartitionTableInfo: <<< EXIT FAILED <<< exec get partition failed, err=%{public}d", ret);
        return E_GET_PARTITION_ERROR;
    }
    std::vector<std::string> lines = result.second;
    for (const auto &line : lines) {
        execRet += line;
        execRet += "\n";
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
        std::vector<std::string> cmd = {SGDISK_PATH, "-n", sector, "-t", code, devPath};
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

int32_t DiskUtils::DeletePartitionInfo(const std::string &devPath, int32_t partitionNum)
{
    if (!IsValidBlockDevicePath(devPath)) {
        LOGE("DiskUtils::DeletePartitionInfo invalid devPath");
        return E_PARAMS_INVALID;
    }
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread partitionThread([devPath, partitionNum, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskUtils] exec delete partition");
        std::vector<std::string> cmd = {SGDISK_PATH, "-d", std::to_string(partitionNum), devPath};
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
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
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
} // namespace StorageDaemon
} // namespace OHOS
