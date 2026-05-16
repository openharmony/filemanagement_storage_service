/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <dirent.h>
#include <future>
#include <nlohmann/json.hpp>
#include <regex>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "volume/volume_manager.h"

namespace OHOS {
namespace StorageDaemon {
constexpr int32_t MIN_LINES = 32;
constexpr int32_t VOL_LENGTH = 3;
constexpr int32_t MAJORID_BLKEXT = 259;
constexpr int32_t MAX_PARTITION = 16;
constexpr int32_t MAX_INTERVAL_PARTITION = 15;
constexpr int32_t PREFIX_LENGTH = 2;
constexpr int32_t HEX_SHIFT_BITS = 4;
constexpr int32_t HEX_LETTER_OFFSET = 10;
constexpr int32_t WAIT_THREAD_TIMEOUT_S = 60;
constexpr uint64_t VFAT_TYPECODE_MIN_SIZE = 16 * 1024 * 1024;
constexpr uint64_t EXFAT_TYPECODE_MIN_SIZE = 32 * 1024 * 1024;
constexpr uint64_t NTFS_TYPECODE_MIN_SIZE = 4 * 1024 * 1024;
constexpr uint64_t EXT4_TYPECODE_MIN_SIZE = 16 * 1024 * 1024;
constexpr uint64_t F2FS_TYPECODE_MIN_SIZE = 16 * 1024 * 1024;
constexpr const char *SGDISK_PATH = "/system/bin/sgdisk";
constexpr const char *SGDISK_DUMP_CMD = "--ohos-dump";
constexpr const char *SGDISK_ZAP_CMD = "--zap-all";
constexpr const char *SGDISK_PART_CMD = "--new=0:0:-0 --typecode=0:0c00 --gpttombr=1";
constexpr const char *BLOCK_PATH = "/dev/block";
constexpr const char *DISK_PREFIX = "DISK ";

enum DiskStatus:int {
    S_INITAL = 0,
    S_CREATE = 1,
    S_SCAN = 2,
    S_DESTROY = 4,
};

const std::map<uint32_t, std::string> vendorMap_ = {
    {0x000003, "SanDisk"},
    {0x00001b, "SamSung"},
    {0x000028, "Lexar"},
    {0x000074, "Transcend"}
};

const std::map<std::string, std::string> typeCodeMap_ = {
    {"vfat", "0x0700"},
    {"exfat", "0x0700"},
    {"ntfs", "0x0700"},
    {"ext4", "0x8300"},
    {"f2fs", "0x8300"},
    {"hmfs", "0x8300"},
};

const std::map<std::string, std::string> formatTypeMap_ = {
    {"exfat", "mkfs.exfat"},
    {"vfat", "newfs_msdos"},
    {"ext4", "mke2fs"},
};

DiskInfo::DiskInfo(std::string &diskName, std::string &sysPath, std::string &devPath, dev_t device, int diskType)
{
    diskId_ = StringPrintf("disk-%d-%d", major(device), minor(device));
    diskName_ = diskName;
    sysPath_ = sysPath;
    eventPath_ = devPath;
    devPath_ = StringPrintf("/dev/block/%s", diskId_.c_str());
    device_ = device;
    diskType_ = static_cast<DiskType>(diskType);
    status = S_INITAL;
    isUserdata = false;
    sgdiskLines_ = std::vector<std::string>();
}

dev_t DiskInfo::GetDevice() const
{
    return device_;
}

std::string DiskInfo::GetDiskId() const
{
    return diskId_;
}

std::string DiskInfo::GetDevPath() const
{
    return devPath_;
}

uint64_t DiskInfo::GetTotalSize() const
{
    return totalSize_;
}

std::string DiskInfo::GetSysPath() const
{
    return sysPath_;
}

std::string DiskInfo::GetDevVendor() const
{
    return vendor_;
}

int32_t DiskInfo::GetDiskType() const
{
    return diskType_;
}

int32_t DiskInfo::GetMediaType() const
{
    return mediaType_;
}

std::string DiskInfo::GetDiskName() const
{
    return diskName_;
}

bool DiskInfo::GetRemovable() const
{
    return removable_;
}

std::string DiskInfo::GetExtraInfo() const
{
    return extraInfo_;
}

DiskInfo::~DiskInfo()
{
    DestroyDiskNode(devPath_);
}

int DiskInfo::Create()
{
    LOGI("[L3:DiskInfo] Create: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    int ret;
    CreateDiskNode(devPath_, device_);
    status = S_CREATE;
    ReadMetadata();
    StorageManagerClient client;
    ret = client.NotifyDiskCreated(*this);
    if (ret != E_OK) {
        LOGE("[L3:DiskInfo] Create: <<< EXIT FAILED <<< Notify Disk Created failed, err=%{public}d", ret);
        return ret;
    }
    ret = ReadPartition();
    if (ret != E_OK) {
        LOGE("[L3:DiskInfo] Create: <<< EXIT FAILED <<< Create disk failed, err=%{public}d", ret);
        client.NotifyDiskDestroyed(diskId_);
        return ret;
    }
    LOGI("[L3:DiskInfo] Create: <<< EXIT SUCCESS <<< diskId=%{public}s", diskId_.c_str());
    return E_OK;
}

int DiskInfo::Destroy()
{
    LOGI("[L3:DiskInfo] Destroy: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    auto &volume = VolumeManager::Instance();

    for (const auto& volumeId : volumeId_) {
        auto ret = volume.DestroyVolume(volumeId);
        if (ret != E_OK) {
            LOGE("[L3:DiskInfo] Destroy: <<< EXIT FAILED <<< Destroy volume=%{public}s failed, err=%{public}d",
                 volumeId.c_str(), ret);
            return E_ERR;
        }
    }
    status = S_DESTROY;
    volumeId_.clear();
    LOGI("[L3:DiskInfo] Destroy: <<< EXIT SUCCESS <<< diskId=%{public}s", diskId_.c_str());
    return E_OK;
}

void DiskInfo::ReadMetadata()
{
    LOGI("[L3:DiskInfo] ReadMetadata: >>> ENTER <<< devPath=%{public}s", devPath_.c_str());
    totalSize_ = -1;
    vendor_.clear();
    if (GetDevSize(devPath_, &totalSize_) != E_OK) {
        totalSize_ = -1;
    }

    unsigned int majorId = major(device_);
    if (majorId == DISK_MMC_MAJOR) {
        std::string path(sysPath_ + "/device/manfid");
        std::string str;
        if (!ReadFile(path, &str)) {
            LOGE("[L3:DiskInfo] ReadMetadata: <<< EXIT FAILED <<< open file=%{public}s failed", path.c_str());
            return;
        }
        LOGI("[L3:DiskInfo] ReadMetadata: Raw manfid value from file=%{public}s", str.c_str());
        uint32_t manfid = 0;
        bool is_valid = ParseAndValidateManfid(str, manfid);
        if (is_valid) {
            auto it = vendorMap_.find(manfid);
            vendor_ = (it != vendorMap_.end()) ? it->second : "Unknown";
        } else {
            LOGI("[L3:DiskInfo] ReadMetadata: Invalid manfid=%{public}s", str.c_str());
            vendor_ = "Invalid";
        }
    } else {
        std::string path(sysPath_ + "/device/vendor");
        std::string str;
        if (!ReadFile(path, &str)) {
            LOGE("[L3:DiskInfo] ReadMetadata: <<< EXIT FAILED <<< open file=%{public}s failed", path.c_str());
            return;
        }
        vendor_ = str;
        LOGI("[L3:DiskInfo] ReadMetadata: <<< EXIT SUCCESS <<< path=%{public}s", path.c_str());
    }
}

bool DiskInfo::ParseAndValidateManfid(const std::string& str, uint32_t& manfid)
{
    LOGD("[L3:DiskInfo] ParseAndValidateManfid: >>> ENTER <<<");
    std::string trimmed = str;
    size_t start = trimmed.find_first_not_of(" \t\n\r");
    if (start != std::string::npos) {
        trimmed.erase(0, start);
    } else {
        LOGD("[L3:DiskInfo] ParseAndValidateManfid: <<< EXIT FAILED <<< empty string");
        return false;
    }
    size_t end = trimmed.find_last_not_of(" \t\n\r");
    trimmed.erase(std::min(end + 1, trimmed.size()));

    const char* p = trimmed.c_str();
    if (trimmed.size() > PREFIX_LENGTH && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += PREFIX_LENGTH;
    }
    manfid = 0;
    while (*p) {
        char c = *p++;
        if (c >= '0' && c <= '9') {
            manfid = (manfid << HEX_SHIFT_BITS) | (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            manfid = (manfid << HEX_SHIFT_BITS) | (c - 'A' + HEX_LETTER_OFFSET);
        } else if (c >= 'a' && c <= 'f') {
            manfid = (manfid << HEX_SHIFT_BITS) | (c - 'a' + HEX_LETTER_OFFSET);
        } else {
            LOGD("[L3:DiskInfo] ParseAndValidateManfid: <<< EXIT FAILED <<< invalid character");
            return false;
        }
    }
    LOGD("[L3:DiskInfo] ParseAndValidateManfid: <<< EXIT SUCCESS <<< manfid=0x%{public}x", manfid);
    return true;
}

int DiskInfo::ReadPartition(const std::string &ejectStatus)
{
    LOGI("[L3:DiskInfo] ReadPartition: >>> ENTER <<< diskId=%{public}s, ejectStatus=%{public}s",
         diskId_.c_str(), ejectStatus.c_str());
    int ret = 0;
    if (major(device_) == DISK_CD_MAJOR) {
        ret = ReadPartitionCD(ejectStatus);
    } else {
        ret = ReadPartitionUSB();
    }
    if (ret == E_OK) {
        LOGI("[L3:DiskInfo] ReadPartition: <<< EXIT SUCCESS <<< diskId=%{public}s", diskId_.c_str());
    } else {
        LOGE("[L3:DiskInfo] ReadPartition: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId_.c_str(), ret);
    }
    return ret;
}

int DiskInfo::ReadPartitionCD(const std::string &ejectStatus)
{
    LOGI("[L3:DiskInfo] ReadPartitionCD: >>> ENTER <<< diskId=%{public}s, ejectStatus=%{public}s",
         diskId_.c_str(), ejectStatus.c_str());
    if (ejectStatus == "1") {
        if (Destroy() != E_OK) {
            LOGE("[L3:DiskInfo] ReadPartitionCD: <<< EXIT FAILED <<< Destroy failed");
            return E_ERR;
        }
        auto res = Eject(devPath_);
        if (res != E_OK) {
            LOGE("[L3:DiskInfo] ReadPartitionCD: <<< EXIT FAILED <<< eject failed, err=%{public}d", res);
            return res;
        }
        LOGI("[L3:DiskInfo] ReadPartitionCD: <<< EXIT SUCCESS <<< ejected");
        return E_OK;
    }

    bool isExistCD = false;
    int existRet = IsExistCD(devPath_, isExistCD);
    if (existRet != E_OK || !isExistCD) {
        for (auto volumeId : volumeId_) {
            auto ret = VolumeManager::Instance().DestroyVolume(volumeId);
            if (ret != E_OK) {
                LOGE("[L3:DiskInfo] ReadPartitionCD: Destroy volume=%{public}s failed, err=%{public}d",
                     volumeId.c_str(), ret);
            }
        }
        volumeId_.clear();
        LOGI("[L3:DiskInfo] ReadPartitionCD: <<< EXIT SUCCESS <<< CD not exist, cleared");
        return E_OK;
    } else {
        LOGI("[L3:DiskInfo] ReadPartitionCD: ejectStatus=%{public}s, CD exists", ejectStatus.c_str());
        dev_t partitionDev = makedev(major(device_), minor(device_));
        auto res = CreateVolume(partitionDev, 1);
        if (res != E_OK) {
            LOGE("[L3:DiskInfo] ReadPartitionCD: <<< EXIT FAILED <<< CreateVolume failed, err=%{public}d", res);
            return res;
        }
    }
    LOGI("[L3:DiskInfo] ReadPartitionCD: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int DiskInfo::ReadPartitionUSB()
{
    LOGI("[L3:DiskInfo] ReadPartitionUSB: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    int maxVolumes = GetMaxVolume(device_);
    if (maxVolumes < 0) {
        LOGE("[L3:DiskInfo] ReadPartitionUSB: <<< EXIT FAILED <<< Invalid maxVolumes=%{public}d", maxVolumes);
        return E_ERR;
    }
    std::vector<std::string> output;
    std::vector<std::string> lines;
    std::vector<std::string> cmd = {SGDISK_PATH, SGDISK_DUMP_CMD, devPath_};
    int res = ForkExec(cmd, &output);
    for (auto str : output) {
        std::string maskedStr = MaskSensitiveInfo(str);
        LOGI("ReadPartitionUSB output: %{public}s", maskedStr.c_str());
    }
    FilterOutput(lines, output);
    if (res != E_OK || lines.empty()) {
        int destroyRes = Destroy();
        sgdiskLines_.clear();
        LOGE("[L3:DiskInfo] ReadPartitionUSB: <<< EXIT FAILED <<< get partition failed, destroy error=%{public}d",
             destroyRes);
        return res;
    }
    isUserdata = false;
    if (lines.size() > MIN_LINES) {
        auto userdataIt = std::find_if(lines.begin(), lines.end(), [](const std::string &str) {
            return str.find("userdata") != std::string::npos;
        });
        if (userdataIt != lines.end()) {
            isUserdata = true;
            std::vector<std::string> hmfsLines;
            hmfsLines.push_back(lines.front());
            hmfsLines.push_back(*userdataIt);
            status = S_SCAN;
            return ReadDiskLines(hmfsLines, maxVolumes, isUserdata);
        }
    }
    status = S_SCAN;
    std::sort(lines.begin() + 1, lines.end());
    if (sgdiskLines_.empty()) {
        sgdiskLines_ = lines;
    } else {
        ProcessPartitionChanges(lines, maxVolumes, isUserdata);
        LOGI("[L3:DiskInfo] ReadPartitionUSB: <<< EXIT SUCCESS <<< processed changes");
        return E_OK;
    }
    return ReadDiskLines(sgdiskLines_, maxVolumes, isUserdata);
}

void DiskInfo::FilterOutput(std::vector<std::string> &lines, std::vector<std::string> &output)
{
    LOGD("[L3:DiskInfo] FilterOutput: >>> ENTER <<<");
    std::vector<std::string> tempInfo;
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        tempInfo.insert(tempInfo.end(), split.begin(), split.end());
    }
    int32_t count = static_cast<int32_t>(tempInfo.size());
    int32_t index = -1;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = tempInfo[i];
        if (buf.find(DISK_PREFIX) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        LOGE("[L3:DiskInfo] FilterOutput: <<< EXIT FAILED <<< disk info not found");
        return;
    }
    for (int32_t i = index; i < count; i++) {
        std::string target = tempInfo[i];
        if (std::find(lines.begin(), lines.end(), target) == lines.end()) {
            lines.push_back(target);
        }
    }
    LOGE("[L3:DiskInfo] FilterOutput: <<< EXIT SUCCESS <<< lines.size=%{public}zu", lines.size());
}

void DiskInfo::ProcessPartitionChanges(const std::vector<std::string>& lines, int maxVolumes, bool isUserdata)
{
    LOGI("[L3:DiskInfo] ProcessPartitionChanges: >>> ENTER <<<");
    std::vector<std::string> addedLines;
    std::vector<std::string> removedLines;
    std::set_difference(
        lines.begin(), lines.end(),
        sgdiskLines_.begin(), sgdiskLines_.end(),
        std::back_inserter(addedLines)
    );
    std::set_difference(
        sgdiskLines_.begin(), sgdiskLines_.end(),
        lines.begin(), lines.end(),
        std::back_inserter(removedLines)
    );

    if (!addedLines.empty()) {
        std::vector<std::string> sgdiskAddedLines;
        sgdiskAddedLines.reserve(addedLines.size() + 1);
        sgdiskAddedLines.push_back(lines.front());
        sgdiskAddedLines.insert(sgdiskAddedLines.end(), addedLines.begin(), addedLines.end());
        sgdiskLines_ = lines;
        if (ReadDiskLines(sgdiskAddedLines, maxVolumes, isUserdata) != E_OK) {
            LOGI("[L3:DiskInfo] ProcessPartitionChanges: Failed to read added disk lines");
        }
    }
    if (!removedLines.empty()) {
        UmountLines(removedLines, maxVolumes, isUserdata);
        sgdiskLines_.clear();
        sgdiskLines_ = lines;
    }
    LOGI("[L3:DiskInfo] ProcessPartitionChanges: <<< EXIT SUCCESS <<<");
}

void DiskInfo::UmountLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata)
{
    LOGI("[L3:DiskInfo] UmountLines: >>> ENTER <<<");
    std::string lineToken = " ";
    for (auto &line : lines) {
        auto split = SplitLine(line, lineToken);
        auto it = split.begin();
        if (it == split.end()) {
            continue;
        }

        if (*it == "PART") {
            if (++it == split.end()) {
                continue;
            }
            dev_t partitionDev = ProcessPartition(it, maxVols, isUserdata);
            if (partitionDev == makedev(0, 0)) {
                continue;
            }
            std::string volumeId = StringPrintf("vol-%u-%u", major(partitionDev), minor(partitionDev));
            auto ret = VolumeManager::Instance().DestroyVolume(volumeId);
            if (ret != E_OK) {
                LOGE("[L3:DiskInfo] UmountLines: Destroy volume=%{public}s failed, err=%{public}d",
                     volumeId.c_str(), ret);
            }
        }
    }
    LOGI("[L3:DiskInfo] UmountLines: <<< EXIT SUCCESS <<<");
}

bool DiskInfo::CreateMBRVolume(int32_t type, dev_t dev, uint32_t partitionNum)
{
    LOGD("[L3:DiskInfo] CreateMBRVolume: >>> ENTER <<< type=0x%{public}x", type);
    // FAT16 || NTFS/EXFAT || W95 FAT32 || W95 FAT32 || W95 FAT16 || EFI FAT32 || EXT 2/3/4
    if (type == 0x06 || type == 0x07 || type == 0x0b || type == 0x0c || type == 0x0e || type == 0x1b || type == 0x83) {
        if (CreateVolume(dev, partitionNum) == E_OK) {
            LOGD("[L3:DiskInfo] CreateMBRVolume: <<< EXIT SUCCESS <<<");
            return true;
        }
    }
    LOGD("[L3:DiskInfo] CreateMBRVolume: <<< EXIT FAILED <<< unsupported type=0x%{public}x", type);
    return false;
}

int32_t DiskInfo::CreateUnknownTabVol()
{
    LOGI("[L3:DiskInfo] CreateUnknownTabVol: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    std::string fsType;
    std::string uuid;
    std::string label;
    auto ret = OHOS::StorageDaemon::ReadMetadata(devPath_, fsType, uuid, label);
    if (ret == E_OK) {
        CreateVolume(device_, 0);
    } else {
        StorageService::StorageRadar::ReportUserManager("DiskInfo::CreateUnknownTabVol::ReadMetadata", 0,
                                                        ret, "devPath_=" + devPath_);
        LOGE("[L3:DiskInfo] CreateUnknownTabVol: <<< EXIT FAILED <<< failed to identify the disk device");
        return E_NON_EXIST;
    }
    LOGI("[L3:DiskInfo] CreateUnknownTabVol: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskInfo::ReadDiskLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata)
{
    LOGI("[L3:DiskInfo] ReadDiskLines: >>> ENTER <<< lines.size=%{public}zu", lines.size());
    std::string lineToken = " ";
    bool foundPart = false;
    Table table = Table::UNKNOWN;
    for (auto &line : lines) {
        auto split = SplitLine(line, lineToken);
        auto it = split.begin();
        if (it == split.end()) {
            continue;
        }
        if (*it == "DISK") {
            if (++it == split.end()) {
                continue;
            }
            if (*it == "mbr") {
                table = Table::MBR;
            } else if (*it == "gpt") {
                table = Table::GPT;
            } else {
                LOGI("[L3:DiskInfo] ReadDiskLines: Unknown partition table=%{public}s", (*it).c_str());
                continue;
            }
        } else if (*it == "PART") {
            if (++it == split.end()) {
                continue;
            }
            dev_t partitionDev = ProcessPartition(it, maxVols, isUserdata);
            if (partitionDev == makedev(0, 0)) {
                continue;
            }
            CreateTableVolume(it, split.end(), table, foundPart, partitionDev);
        }
    }
    if (lines.size() == 1 && table != Table::UNKNOWN) {
        LOGI("[L3:DiskInfo] ReadDiskLines: <<< EXIT SUCCESS <<<");
        return E_OK;
    }
    if (table == Table::UNKNOWN || !foundPart) {
        LOGI("[L3:DiskInfo] ReadDiskLines: trying unknown table");
        return CreateUnknownTabVol();
    }
    LOGI("[L3:DiskInfo] ReadDiskLines: <<< EXIT SUCCESS <<<");
    return E_OK;
}

dev_t DiskInfo::ProcessPartition(std::vector<std::string>::iterator &it, int32_t maxVols, bool isUserdata)
{
    LOGD("[L3:DiskInfo] ProcessPartition: >>> ENTER <<<");
    int32_t index = std::atoi((*it).c_str());
    unsigned int majorId = major(device_);
    if ((index > maxVols && majorId == DISK_MMC_MAJOR) || index < 1) {
        LOGE("[L3:DiskInfo] ProcessPartition: <<< EXIT FAILED <<< Invalid partition=%{public}d", index);
        return makedev(0, 0);
    }
    dev_t partitionDev = makedev(0, 0);
    if (isUserdata) {
        int32_t maxMinor = GetMaxMinor(MAJORID_BLKEXT);
        if (maxMinor == -1) {
            partitionDev = makedev(MAJORID_BLKEXT, static_cast<uint32_t>(index) - MAX_PARTITION);
        } else {
            partitionDev = makedev(MAJORID_BLKEXT, static_cast<uint32_t>(maxMinor) +
                                   static_cast<uint32_t>(index) - MAX_INTERVAL_PARTITION);
        }
    } else {
        if (index > MAX_SCSI_VOLUMES) {
            partitionDev = makedev(MAJORID_BLKEXT, static_cast<uint32_t>(index) - MAX_PARTITION);
        } else {
            partitionDev = makedev(major(device_), minor(device_) + static_cast<uint32_t>(index));
        }
    }
    LOGD("[L3:DiskInfo] ProcessPartition: <<< EXIT SUCCESS <<< partition=%{public}u,%{public}u",
         major(partitionDev), minor(partitionDev));
    return partitionDev;
}

int32_t DiskInfo::GetMaxMinor(int32_t major)
{
    LOGD("[L3:DiskInfo] GetMaxMinor: >>> ENTER <<< major=%{public}d", major);
    DIR* dir;
    struct dirent* entry;
    int32_t maxMinor = -1;
    if ((dir = opendir(BLOCK_PATH)) == nullptr) {
        LOGE("[L3:DiskInfo] GetMaxMinor: <<< EXIT FAILED <<< open=%{public}s failed, errno=%{public}d",
             BLOCK_PATH, errno);
        return E_ERR;
    }
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.' || strncmp(entry->d_name, "vol", VOL_LENGTH) != 0) {
            continue;
        }
        std::string devicePath = std::string(BLOCK_PATH) + "/" + entry->d_name;
        struct stat statbuf;
        if (stat(devicePath.c_str(), &statbuf) == 0) {
            int32_t majorNum = static_cast<int32_t>major(statbuf.st_rdev);
            int32_t minorNum = static_cast<int32_t>minor(statbuf.st_rdev);

            if (majorNum == major) {
                maxMinor = minorNum > maxMinor ? minorNum : maxMinor;
            }
        }
    }
    closedir(dir);
    LOGD("[L3:DiskInfo] GetMaxMinor: <<< EXIT SUCCESS <<< maxMinor=%{public}d", maxMinor);
    return maxMinor;
}

void DiskInfo::CreateTableVolume(std::vector<std::string>::iterator &it, const std::vector<std::string>::iterator &end,
    Table table, bool &foundPart, dev_t partitionDev)
{
    LOGD("[L3:DiskInfo] CreateTableVolume: >>> ENTER <<<");
    int32_t partitionNum;
    if (!ConvertStringToInt32((*it), partitionNum)) {
        return;
    }
    if (table == Table::MBR) {
        if (++it == end) {
            return;
        }
        char *endptr;
        int64_t val = std::strtoll(("0x0" + *it).c_str(), &endptr, 16);
        if (val > INT32_MAX) {
            LOGE("Illegal type value , out of range");
            return;
        }
        int32_t type = static_cast<int32_t>(val);
        if (CreateMBRVolume(type, partitionDev, static_cast<uint32_t>(partitionNum))) {
            foundPart = true;
            LOGD("[L3:DiskInfo] CreateTableVolume: <<< EXIT SUCCESS <<< MBR volume created");
        } else {
            LOGE("[L3:DiskInfo] CreateTableVolume: Create MBR Volume failed, type=0x%{public}x", type);
        }
    } else if (table == Table::GPT) {
        if (CreateVolume(partitionDev, static_cast<uint32_t>(partitionNum)) == E_OK) {
            foundPart = true;
            LOGD("[L3:DiskInfo] CreateTableVolume: <<< EXIT SUCCESS <<< GPT volume created");
        } else {
            LOGW("[L3:DiskInfo] CreateTableVolume: Create GPT Volume failed");
        }
    }
}

int DiskInfo::CreateVolume(dev_t dev, uint32_t partitionNum)
{
    LOGI("[L3:DiskInfo] CreateVolume: >>> ENTER <<< dev=%{public}u,%{public}u", major(dev), minor(dev));
    auto &volume = VolumeManager::Instance();

    nlohmann::json extraInfoJson;

    if (major(device_) == DISK_CD_MAJOR) {
        std::string driveType = GetOpticalDriveType(devPath_);
        std::string discType = GetCDType(devPath_);
        int32_t maxWriteSpeed = 0;
        GetOpticalDriveMaxWriteSpeed(devPath_, maxWriteSpeed);
        extraInfoJson["ODD_INFO"]["DRIVE_TYPE"] = driveType;
        extraInfoJson["ODD_INFO"]["DISC_TYPE"] = discType;
        extraInfoJson["ODD_INFO"]["MAX_WRITE_SPEED"] = std::to_string(maxWriteSpeed) + "X";
    }

    std::string extraInfo = extraInfoJson.dump();

    std::string volumeId = volume.CreateVolume(GetDiskId(), dev, isUserdata, partitionNum, extraInfo);
    if (volumeId.empty()) {
        LOGE("[L3:DiskInfo] CreateVolume: <<< EXIT FAILED <<< Create volume failed");
        return E_ERR;
    }

    volumeId_.push_back(volumeId);
    LOGI("[L3:DiskInfo] CreateVolume: <<< EXIT SUCCESS <<< volumeId=%{public}s", volumeId.c_str());
    return E_OK;
}

int DiskInfo::Partition()
{
    LOGI("[L3:DiskInfo] Partition: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    if (major(device_) == DISK_CD_MAJOR) {
        LOGE("[L3:DiskInfo] Partition: <<< EXIT FAILED <<< CD/DVD not support partition");
        return E_NOT_SUPPORT;
    }
    std::vector<std::string> cmd;
    int res;

    res = Destroy();
    if (res != E_OK) {
        LOGE("[L3:DiskInfo] Partition: Destroy failed in Partition(), err=%{public}d", res);
    }

    cmd.push_back(SGDISK_PATH);
    cmd.push_back(SGDISK_ZAP_CMD);
    cmd.push_back(devPath_);
    LOGI("[L3:DiskInfo] Partition: executing sgdisk zap command");
    std::vector<std::string> output;
    res = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("Partition sgdisk zap cmd output: %{public}s", str.c_str());
    }
    if (res != E_OK) {
        LOGE("[L3:DiskInfo] Partition: <<< EXIT FAILED <<< sgdisk zap failed, err=%{public}d", res);
        return res;
    }

    cmd.clear();
    output.clear();
    cmd.push_back(SGDISK_PATH);
    cmd.push_back(SGDISK_PART_CMD);
    cmd.push_back(devPath_);
    res = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("Partition sgdisk part cmd output: %{public}s", str.c_str());
    }
    if (res != E_OK) {
        LOGE("[L3:DiskInfo] Partition: <<< EXIT FAILED <<< sgdisk partition failed, err=%{public}d", res);
        return res;
    }

    LOGI("[L3:DiskInfo] Partition: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskInfo::GetPartitionTable(OHOS::StorageManager::PartitionTableInfo &partitionTableInfo)
{
    LOGI("[L3:DiskInfo] GetPartitionTable: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    std::vector<std::string> output;
    int32_t ret = ExecAsyncGetPartitionTable(output);
    if (ret != E_OK) {
        return ret;
    }
    std::vector<std::string> tempInfo;
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        tempInfo.insert(tempInfo.end(), split.begin(), split.end());
    }
    if (!SetTotalSector(tempInfo)) {
        return E_SET_TOTAL_SECTOR_ERROR;
    }
    if (!SetSectorSize(tempInfo)) {
        return E_SET_SECTOR_SIZE_ERROR;
    }
    if (!SetAlignSector(tempInfo)) {
        return E_SET_ALIGN_SECTOR_ERROR;
    }
    if (!SetUsableSector(tempInfo)) {
        return E_SET_USABLE_SECTOR_ERROR;
    }
    SetPartitions(tempInfo, partitionTableInfo);
    SetTableType(tempInfo, partitionTableInfo);
    partitionTableInfo.SetDiskId(diskId_);
    partitionTableInfo.SetPartitionCount(static_cast<uint32_t>(partitionTableInfo.GetPartitions().size()));
    partitionTableInfo.SetTotalSector(totalSector_);
    partitionTableInfo.SetSectorSize(sectorSize_);
    partitionTableInfo.SetAlignSector(alignSector_);
    LOGI("[L3:DiskInfo] GetPartitionTable: >>> EXIT SUCCESS <<<");
    return E_OK;
}

bool DiskInfo::SetUsableSector(std::vector<std::string> &content)
{
    auto count = static_cast<int32_t>(content.size());
    std::string prefix = "First usable sector is";
    std::string target;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = content[i];
        if (buf.find(prefix) == 0) {
            target = buf;
            break;
        }
    }
    if (target.empty()) {
        LOGE("[L3:DiskInfo] SetUsableSector: <<< EXIT FAILED <<< not found usable sector");
        return false;
    }
    std::regex pattern(R"(last usable sector is (\d+))");
    std::smatch match;
    if (!std::regex_search(target, match, pattern)) {
        LOGE("[L3:DiskInfo] SetUsableSector: <<< EXIT FAILED <<< usable sector not match, target=%{public}s",
             target.c_str());
        return false;
    }
    std::string result = match[1].str();
    int64_t lastUsableSector = 0;
    if (!ConvertStringToInt(result, lastUsableSector)) {
        LOGE("[L3:DiskInfo] SetUsableSector: <<< EXIT FAILED <<< convert last usable sector failed, result=%{public}s",
             result.c_str());
        return false;
    }
    lastUsableSector_ = static_cast<uint64_t>(lastUsableSector);
    LOGI("[L3:DiskInfo] SetUsableSector: <<< EXIT SUCCESS <<< lastUsableSector=%{public}lld",
         static_cast<long long>(lastUsableSector));
    return true;
}

int32_t DiskInfo::ExecAsyncGetPartitionTable(std::vector<std::string> &output)
{
    std::promise<std::pair<int32_t, std::vector<std::string>>> promise;
    std::future<std::pair<int32_t, std::vector<std::string>>> future = promise.get_future();
    std::thread partitionThread([this, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskInfo] exec get partition");
        std::vector<std::string> temp;
        std::vector<std::string> cmd = {"sgdisk", "-p", devPath_};
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
        LOGE("[L3:DiskInfo] GetPartitionTable: <<< EXIT FAILED <<< exec get partition failed, err=%{public}d", ret);
        return E_GET_PARTITION_ERROR;
    }
    output = std::move(result.second);
    return E_OK;
}

int32_t DiskInfo::ExecAsyncCreatePartition(const OHOS::StorageManager::PartitionParams &partitionParams)
{
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread partitionThread([this, partitionParams, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskInfo] exec create partition");
        std::string sector = "0:" + std::to_string(partitionParams.GetStartSector()) + ":" +
            std::to_string(partitionParams.GetEndSector());
        std::string code = "0:" + typeCodeMap_.find(partitionParams.GetTypeCode())->second;
        std::vector<std::string> cmd = {SGDISK_PATH, "-n", sector, "-t", code, devPath_};
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("create partition output: %{public}s", str.c_str());
        }
        p.set_value(ret);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskInfo] exec create partition: <<< EXIT FAILED <<< timed out");
        partitionThread.detach();
        return E_CREATE_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    partitionThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskInfo] ExecAsyncCreatePartition: <<< EXIT FAILED <<< create partition failed,err=%{public}d", ret);
        return E_CREATE_PARTITION_ERROR;
    }
    LOGI("[L3:DiskInfo] ExecAsyncCreatePartition: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskInfo::CreatePartition(const OHOS::StorageManager::PartitionParams &partitionParams)
{
    LOGI("[L3:DiskInfo] CreatePartition: >>> ENTER <<< diskId=%{public}s", diskId_.c_str());
    if (diskType_ == CD_DVD_BD || diskType_ == MTP_PTP || diskType_ == UNKNOWN_DISK_TYPE) {
        LOGE("this disk not support create partition");
        return E_CREATE_PARTITION_NOT_SUPPORT;
    }
    std::string typeCode = partitionParams.GetTypeCode();
    auto type = typeCodeMap_.find(typeCode);
    if (type == typeCodeMap_.end()) {
        LOGE("type code not support create partition");
        return E_CREATE_PARTITION_NOT_SUPPORT;
    }
    if (!IsOptionsValid(partitionParams)) {
        return E_PARAMS_INVALID;
    }
    if (!volumeId_.empty() && VolumeManager::Instance().IsDiskHasMountedVolume(diskId_)) {
        LOGE("[L3:DiskInfo] CreatePartition: <<< EXIT FAILED <<< volume status is mounted");
        return E_VOL_STATE;
    }
    if (Destroy() != E_OK) {
        LOGE("[L3:DiskInfo] CreatePartition: <<< EXIT FAILED <<< destroy volume failed");
        return E_CREATE_PARTITION_ERROR;
    }
    sgdiskLines_.clear();
    int32_t ret = ExecAsyncCreatePartition(partitionParams);
    if (ret != E_OK) {
        std::thread thread([this]() { ReadPartitionUSB(); });
        thread.detach();
        LOGI("[L3:DiskInfo] CreatePartition: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L3:DiskInfo] CreatePartition: <<< EXIT SUCCESS <<<");
    }
    return ret;
}

bool DiskInfo::IsOptionsValid(const OHOS::StorageManager::PartitionParams &partitionParams)
{
    uint64_t startSector = partitionParams.GetStartSector();
    if (startSector > lastUsableSector_ || startSector < alignSector_) {
        LOGE("start sector out range");
        return false;
    }
    if (((startSector * sectorSize_) % alignSector_) != 0) {
        LOGE("start sector not align");
        return false;
    }
    uint64_t endSector = partitionParams.GetEndSector();
    if (endSector > lastUsableSector_ || endSector < alignSector_) {
        LOGE("end sector out range");
        return false;
    }
    std::string typeCode = partitionParams.GetTypeCode();
    uint64_t sectorInterval = (endSector - startSector + 1) * sectorSize_;
    if (typeCode == "vfat" && sectorInterval < VFAT_TYPECODE_MIN_SIZE) {
        LOGE("vfat sector interval invalid");
        return false;
    }
    if (typeCode == "exfat" && sectorInterval < EXFAT_TYPECODE_MIN_SIZE) {
        LOGE("exfat sector interval invalid");
        return false;
    }
    if (typeCode == "ntfs" && sectorInterval < NTFS_TYPECODE_MIN_SIZE) {
        LOGE("ntfs sector interval invalid");
        return false;
    }
    if (typeCode == "ext4" && sectorInterval < EXT4_TYPECODE_MIN_SIZE) {
        LOGE("ext4 sector interval invalid");
        return false;
    }
    if ((typeCode == "f2fs" || typeCode == "hmfs") && sectorInterval < F2FS_TYPECODE_MIN_SIZE) {
        LOGE("f2fs or hmfs sector interval invalid");
        return false;
    }
    return true;
}

int32_t DiskInfo::ExecAsyncDeletePartition(uint32_t partitionNum)
{
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread partitionThread([this, partitionNum, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskInfo] exec delete partition");
        std::vector<std::string> cmd = {SGDISK_PATH, "-d", std::to_string(partitionNum), devPath_};
        std::vector<std::string> output;
        int32_t ret = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("delete partition output: %{public}s", str.c_str());
        }
        p.set_value(ret);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskInfo] exec delete partition: <<< EXIT FAILED <<< timed out");
        partitionThread.detach();
        return E_DELETE_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    partitionThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskInfo] ExecAsyncDeletePartition: <<< EXIT FAILED <<< delete partition failed,err=%{public}d", ret);
        return E_DELETE_PARTITION_ERROR;
    }
    LOGI("[L3:DiskInfo] ExecAsyncDeletePartition: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t DiskInfo::DeletePartition(uint32_t partitionNum)
{
    LOGI("[L3:DiskInfo] DeletePartition: >>> ENTER <<< diskId=%{public}s, partitionNum=%{public}u",
         diskId_.c_str(), partitionNum);
    if (diskType_ == CD_DVD_BD || diskType_ == MTP_PTP || diskType_ == UNKNOWN_DISK_TYPE) {
        LOGE("[L3:DiskInfo] DeletePartition: <<< EXIT FAILED <<< this disk not support delete partition");
        return E_DELETE_PARTITION_NOT_SUPPORT;
    }
    if (!IsPartitionNumExists(partitionNum)) {
        LOGE("[L3:DiskInfo] DeletePartition: <<< EXIT FAILED <<< partition %{public}u not exists", partitionNum);
        return E_NON_EXIST;
    }
    if (!volumeId_.empty() && VolumeManager::Instance().IsDiskHasMountedVolume(diskId_)) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< volume status is mounted");
        return E_VOL_STATE;
    }
    if (Destroy() != E_OK) {
        LOGE("[L3:DiskInfo] DeletePartition: <<< EXIT FAILED <<< destroy volume failed");
        return E_DELETE_PARTITION_ERROR;
    }
    sgdiskLines_.clear();
    int32_t ret = ExecAsyncDeletePartition(partitionNum);
    if (ret != E_OK) {
        std::thread thread([this]() { ReadPartitionUSB(); });
        thread.detach();
        LOGI("[L3:DiskInfo] DeletePartition: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L3:DiskInfo] DeletePartition: <<< EXIT SUCCESS <<<");
    }
    return ret;
}

int32_t DiskInfo::FormatPartition(uint32_t partitionNum, const OHOS::StorageManager::FormatParams &formatParams)
{
    LOGI("[L3:DiskInfo] FormatPartition: >>> ENTER <<< diskId=%{public}s, partitionNum=%{public}u",
         diskId_.c_str(), partitionNum);
    if (diskType_ == CD_DVD_BD || diskType_ == MTP_PTP || diskType_ == UNKNOWN_DISK_TYPE) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< this disk not support format partition");
        return E_FORMAT_PARTITION_NOT_SUPPORT;
    }
    std::string fsType = options.GetFsType();
    auto iter = formatTypeMap_.find(fsType);
    if (iter == formatTypeMap_.end()) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< fsType %{public}s not supported", fsType.c_str());
        return E_FORMAT_PARTITION_NOT_SUPPORT;
    }
    if (!IsPartitionNumExists(partitionNum)) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< partition %{public}u not exists", partitionNum);
        return E_NON_EXIST;
    }
    if (!volumeId_.empty() && VolumeManager::Instance().IsDiskHasMountedVolume(diskId_)) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< volume status is mounted");
        return E_VOL_STATE;
    }
    if (Destroy() != E_OK) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< destroy volume failed");
        return E_FORMAT_PARTITION_ERROR;
    }
    sgdiskLines_.clear();
    int32_t ret = ExecAsyncFormatPartition(partitionNum, formatParams);
    std::thread thread([this]() { ReadPartitionUSB(); });
    thread.detach();
    if (ret != E_OK) {
        LOGI("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L3:DiskInfo] FormatPartition: <<< EXIT SUCCESS <<<");
    }
    return E_OK;
}

int32_t DiskInfo::ExecAsyncFormatPartition(uint32_t partitionNum,
    const OHOS::StorageManager::FormatParams &formatParams)
{
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
std::thread formatThread([this, partitionNum, formatParams, p = std::move(promise)]() mutable {
        LOGI("[L3:DiskInfo] exec format partition");
        std::string devPath = std::string(BLOCK_PATH) + "/" + diskName_ + std::to_string(partitionNum);
        std::string fsType = formatParams.GetFsType();
        std::string volName = formatParams.GetVolumeName();
        std::vector<std::string> cmd = GetFormatCMD(fsType, devPath, volName);
        if (!cmd.empty()) {
            std::vector<std::string> output;
            int32_t ret = ForkExec(cmd, &output);
            for (auto str : output) {
                LOGI("format partition output: %{public}s", str.c_str());
            }
            p.set_value(ret);
        }
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:DiskInfo] exec format partition: <<< EXIT FAILED <<< timed out");
        formatThread.detach();
        return E_FORMAT_PARTITION_TIMEOUT;
    }
    int32_t ret = future.get();
    formatThread.join();
    if (ret != E_OK) {
        LOGE("[L3:DiskInfo] FormatPartition: <<< EXIT FAILED <<< format partition failed, err=%{public}d", ret);
        return E_FORMAT_PARTITION_ERROR;
    }
    LOGI("[L3:DiskInfo] FormatPartition: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::vector<std::string> DiskInfo::GetFormatCMD(const std::string &fsType, const std::string &devPath,
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

bool DiskInfo::IsPartitionNumExists(uint32_t partitionNum)
{
    std::string partitionPath = std::string(BLOCK_PATH) + "/" + diskName_ + std::to_string(partitionNum);
    struct stat statBuf{};
    if (lstat(partitionPath.c_str(), &statBuf) == 0) {
        return true;
    }
    LOGI("[L3:DiskInfo] IsPartitionNumExists: partition %{public}u not exists", partitionNum);
    return false;
}

void DiskInfo::SetPartitions(std::vector<std::string> &content,
    OHOS::StorageManager::PartitionTableInfo &partitionTableInfo)
{
    auto count = static_cast<int32_t>(content.size());
    int32_t partitionIndex = -1;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = content[i];
        if (buf.find("Number") == 0 && buf.find("Start") != std::string::npos) {
            partitionIndex = i + 1;
            break;
        }
    }
    if (partitionIndex < 0 || partitionIndex >= count) {
        return;
    }
    std::vector<OHOS::StorageManager::PartitionInfo> partitions;
    for (int32_t i = partitionIndex; i < count; i++) {
        std::string buf = content[i];
        OHOS::StorageManager::PartitionInfo info;
        if (ParsePartitionInfo(buf, info)) {
            partitions.push_back(info);
        }
    }
    partitionTableInfo.SetPartitions(partitions);
}

bool DiskInfo::ParsePartitionInfo(const std::string &context, OHOS::StorageManager::PartitionInfo &info)
{
    if (context.empty()) {
        return false;
    }
    std::stringstream ss(context);
    std::string item;
    ss >> item;
    if (item.empty()) {
        return false;
    }
    int32_t partitionNum;
    if (!ConvertStringToInt32(item, partitionNum)) {
        LOGE("convert partition num failed, %{public}s", item.c_str());
        return false;
    }
    info.SetPartitionNum(static_cast<uint32_t>(partitionNum));
    ss >> item;
    if (item.empty()) {
        return false;
    }
    int64_t startSector;
    if (!ConvertStringToInt(item, startSector)) {
        LOGE("convert start sector failed, %{public}s", item.c_str());
        return false;
    }
    info.SetStartSector(static_cast<uint64_t>(startSector));
    ss >> item;
    if (item.empty()) {
        return false;
    }
    int64_t endSector;
    if (!ConvertStringToInt(item, endSector)) {
        LOGE("convert end sector failed, %{public}s", item.c_str());
        return false;
    }
    info.SetEndSector(static_cast<uint64_t>(endSector));
    uint64_t sizeBytes = (endSector - startSector + 1) * static_cast<uint64_t>(sectorSize_);
    info.SetSizeBytes(sizeBytes);
    info.SetFsType(VolumeManager::Instance().GetFsTypeByDiskIdAndPartNum(diskId_, partitionNum));
    LOGI("partition info is partitionNum=%{public}d, startSector=%{public}lld, endSector=%{public}lld, "
         "sizeBytes=%{public}lld, fsType=%{public}s", partitionNum, static_cast<long long>(startSector),
         static_cast<long long>(endSector), static_cast<long long>(sizeBytes), info.GetFsType().c_str());
    return true;
}

bool DiskInfo::SetTotalSector(std::vector<std::string> &content)
{
    auto count = static_cast<int32_t>(content.size());
    std::string prefix = "Disk " + devPath_;
    std::string target;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = content[i];
        if (buf.find(prefix) == 0) {
            target = buf;
            break;
        }
    }
    if (target.empty()) {
        LOGE("[L3:DiskInfo] SetTotalSector: <<< EXIT FAILED <<< not found total sector");
        return false;
    }
    std::regex pattern(R"((\d+)\s+sectors)");
    std::smatch match;
    if (!std::regex_search(target, match, pattern)) {
        LOGE("[L3:DiskInfo] SetTotalSector: <<< EXIT FAILED <<< total sector not match, target=%{public}s",
             target.c_str());
        return false;
    }
    std::string result = match[1].str();
    int64_t totalSector = 0;
    if (!ConvertStringToInt(result, totalSector)) {
        LOGE("[L3:DiskInfo] SetTotalSector: <<< EXIT FAILED <<< convert failed, result=%{public}s", result.c_str());
        return false;
    }
    totalSector_ = static_cast<uint64_t>(totalSector);
    LOGI("[L3:DiskInfo] SetTotalSector: <<< EXIT SUCCESS <<< totalSector=%{public}lld",
         static_cast<long long>(totalSector));
    return true;
}

bool DiskInfo::SetSectorSize(std::vector<std::string> &content)
{
    auto count = static_cast<int32_t>(content.size());
    std::string prefix = "Sector size (logical/physical)";
    std::string target;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = content[i];
        if (buf.find(prefix) == 0) {
            target = buf;
            break;
        }
    }
    if (target.empty()) {
        LOGE("[L3:DiskInfo] SetSectorSize: <<< EXIT FAILED <<< not found sector size");
        return false;
    }
    std::regex pattern(R"(Sector size \(logical/physical\):\s*(\d+)/\d+)");
    std::smatch match;
    if (!std::regex_search(target, match, pattern)) {
        LOGE("[L3:DiskInfo] SetSectorSize: <<< EXIT FAILED <<< sector size not match, target=%{public}s",
             target.c_str());
        return false;
    }
    std::string result = match[1].str();
    int32_t sectorSize = 0;
    if (!ConvertStringToInt32(result, sectorSize)) {
        LOGE("[L3:DiskInfo] SetSectorSize: <<< EXIT FAILED <<< convert failed, result=%{public}s", result.c_str());
        return false;
    }
    sectorSize_ = static_cast<uint32_t>(sectorSize);
    LOGI("[L3:DiskInfo] SetSectorSize: <<< EXIT SUCCESS <<< sectorSize=%{public}d", sectorSize);
    return true;
}

bool DiskInfo::SetAlignSector(std::vector<std::string> &content)
{
    auto count = static_cast<int32_t>(content.size());
    std::string prefix = "Partitions will be aligned on";
    std::string target;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = content[i];
        if (buf.find(prefix) == 0) {
            target = buf;
            break;
        }
    }
    if (target.empty()) {
        LOGE("[L3:DiskInfo] SetAlignSector: <<< EXIT FAILED <<< not found align sector");
        return false;
    }
    std::regex pattern(R"(Partitions will be aligned on (\d+)-sector boundaries)");
    std::smatch match;
    if (!std::regex_search(target, match, pattern)) {
        LOGE("[L3:DiskInfo] SetAlignSector: <<< EXIT FAILED <<< align sector not match, target=%{public}s",
             target.c_str());
        return false;
    }
    std::string result = match[1].str();
    int32_t alignSector = 0;
    if (!ConvertStringToInt32(result, alignSector)) {
        LOGE("[L3:DiskInfo] SetAlignSector: <<< EXIT FAILED <<< convert failed, result=%{public}s", result.c_str());
        return false;
    }
    alignSector_ = static_cast<uint32_t>(alignSector);
    LOGI("[L3:DiskInfo] SetAlignSector: <<< EXIT SUCCESS <<< alignSector=%{public}d", alignSector);
    return true;
}

void DiskInfo::SetTableType(std::vector<std::string> &content,
    OHOS::StorageManager::PartitionTableInfo &partitionTableInfo)
{
    auto count = static_cast<int32_t>(content.size());
    std::string prefix = "Found invalid GPT and valid MBR";
    bool isMBR = false;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = content[i];
        if (buf.find(prefix) == 0) {
            LOGI("this disk table type is mbr");
            isMBR = true;
            break;
        }
    }
    partitionTableInfo.SetTableType(isMBR ? "MBR" : "GPT");
}

int DiskInfo::EjectDisk()
{
    LOGI("[L3:DiskInfo] EjectDisk: >>> ENTER <<< devPath_=%{public}s", devPath_.c_str());
    if (Destroy() != E_OK) {
        LOGE("[L3:DiskInfo] EjectDisk: <<< EXIT FAILED <<< Destroy failed");
        return E_ERR;
    }
    auto res = Eject(devPath_);
    if (res != E_OK) {
        LOGE("[L3:DiskInfo] EjectDisk: <<< EXIT FAILED <<< Eject failed, err=%{public}d", res);
        return res;
    }
    LOGI("[L3:DiskInfo] EjectDisk: <<< EXIT SUCCESS <<<");
    return E_OK;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS

