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
#include <scsi/sg.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include "storage_service_errno.h"
#include "securec.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"

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
constexpr int32_t DEF_TIMEOUT = 120000;
constexpr int32_t SENSE_BUFF_LEN = 64;
constexpr int32_t READ_DISC_INFO_CDB_LEN = 10;
constexpr int32_t CDB_ALLOCATION_LENGTH_HIGH = 7;
constexpr int32_t CDB_ALLOCATION_LENGTH_LOW = 8;
constexpr int32_t MAX_ALLOC_LEN = 0xFFFF;
constexpr int32_t READ_DISC_INFO_OPCODE = 0x51;

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
    std::vector<std::string> lines = std::move(result.second);
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
    if (ReadCDDiscInfo(diskPath, READ_DISC_INFO_OPCODE, buf, sizeof(buf)) == E_OK) {
        uint8_t discStatus = buf[DISC_STATUS_BYTE_INDEX] & DISC_STATUS_MASK;
        isCDBlank = (discStatus == 0);
        LOGI("IsCDBlank: <<< EXIT SUCCESS <<< isBlank=%{public}d", isCDBlank);
        return E_OK;
    }
    LOGE("IsCDBlank:Unable to read disc information.");
    return E_ERR;
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

std::string DiskUtils::DiskPathToVolPath(const std::string& diskPath)
{
    auto pos = diskPath.find("/disk-");
    if (pos == std::string::npos) {
        return diskPath;
    }
    std::string volPath = diskPath.substr(0, pos + 1) + "vol" + diskPath.substr(pos + 5);
    auto lastDash = volPath.rfind('-');
    if (lastDash != std::string::npos && lastDash > pos + 1) {
        volPath = volPath.substr(0, lastDash + 1) +
                  std::to_string(std::stoi(volPath.substr(lastDash + 1)) + 1);
    }
    return volPath;
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

    output.clear();
    std::string volPath = DiskPathToVolPath(diskPath);
    std::vector<std::string> mkfsCmd = {
        "mkfs.f2fs",
        "-d1",
        "-O", "encrypt",
        "-O", "verity",
        "-O", "sb_checksum",
        volPath
    };
    ret = ForkExec(mkfsCmd, &output);
    for (auto &str : output) {
        LOGI("DiskUtils::PartitionHmfs mkfs: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("DiskUtils::PartitionHmfs mkfs.f2fs failed, ret=%{public}d", ret);
        return ret;
    }

    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
