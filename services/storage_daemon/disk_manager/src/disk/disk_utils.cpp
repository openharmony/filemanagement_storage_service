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
#include <libgen.h>

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
    if (ret != E_OK) {
        LOGE("DiskUtils::Partition sgdisk partition failed, ret=%{public}d", ret);
        return ret;
    }

    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
