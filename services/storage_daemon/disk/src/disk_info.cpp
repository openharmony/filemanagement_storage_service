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
constexpr int32_t DEFAULT_ALIGN_SIZE = 2048;
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

std::map<uint32_t, std::string> vendorMap_ = {
    {0x000003, "SanDisk"},
    {0x00001b, "SamSung"},
    {0x000028, "Lexar"},
    {0x000074, "Transcend"}
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
    SetTotalSector();
    SetSectorSize();
    SetAlignSector();
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

bool DiskInfo::SetTotalSector()
{
    std::string filePath = "/sys/block/" + diskName_ + "/size";
    std::string content;
    if (!ReadFile(filePath, &content)) {
        LOGE("[L3:DiskInfo] SetTotalSector: <<< EXIT FAILED <<< cannot read file=%{public}s", filePath.c_str());
        return false;
    }
    if (content.empty()) {
        LOGE("[L3:DiskInfo] SetTotalSector: <<< EXIT FAILED <<< file content is empty");
        return false;
    }
    int64_t sectorCount = 0;
    if (!ConvertStringToInt(content, sectorCount)) {
        LOGE("[L3:DiskInfo] SetTotalSector: <<< EXIT FAILED <<< convert failed, content=%{public}s", content.c_str());
        return false;
    }
    totalSector_ = static_cast<uint64_t>(sectorCount);
    LOGI("[L3:DiskInfo] SetTotalSector: <<< EXIT SUCCESS <<< totalSector=%{public}llu",
         static_cast<unsigned long long>(totalSector_));
    return true;
}

bool DiskInfo::SetSectorSize()
{
    std::string filePath = "/sys/block/" + diskName_ + "/queue/logical_block_size";
    std::string content;
    if (!ReadFile(filePath, &content)) {
        LOGE("[L3:DiskInfo] SetSectorSize: <<< EXIT FAILED <<< cannot read file=%{public}s", filePath.c_str());
        return false;
    }
    if (content.empty()) {
        LOGE("[L3:DiskInfo] SetSectorSize: <<< EXIT FAILED <<< file content is empty");
        return false;
    }
    int32_t sectorSize = 0;
    if (!ConvertStringToInt32(content, sectorSize)) {
        LOGE("[L3:DiskInfo] SetSectorSize: <<< EXIT FAILED <<< convert failed, content=%{public}s", content.c_str());
        return false;
    }
    sectorSize_ = static_cast<uint32_t>(sectorSize);
    LOGI("[L3:DiskInfo] SetSectorSize: <<< EXIT SUCCESS <<< sectorSize=%{public}d", sectorSize_);
    return true;
}

bool DiskInfo::SetAlignSector()
{
    std::string filePath = "/sys/block/" + diskName_ + "/queue/optimal_io_size";
    std::string content;
    if (!ReadFile(filePath, &content)) {
        LOGE("[L3:DiskInfo] SetAlignSector: <<< EXIT FAILED <<< cannot read file=%{public}s", filePath.c_str());
        return false;
    }
    if (content.empty()) {
        LOGE("[L3:DiskInfo] SetAlignSector: <<< EXIT FAILED <<< file content is empty");
        return false;
    }
    int32_t alignSize = 0;
    if (!ConvertStringToInt32(content, alignSize)) {
        LOGE("[L3:DiskInfo] SetAlignSector: <<< EXIT FAILED <<< convert failed, content=%{public}s", content.c_str());
        return false;
    }
    alignSector_ = alignSize == 0 ? DEFAULT_ALIGN_SIZE : alignSize;
    LOGI("[L3:DiskInfo] SetAlignSector: <<< EXIT SUCCESS <<< alignSize=%{public}d", sectorSize_);
    return true;
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
        auto res = CreateVolume(partitionDev);
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

bool DiskInfo::CreateMBRVolume(int32_t type, dev_t dev)
{
    LOGD("[L3:DiskInfo] CreateMBRVolume: >>> ENTER <<< type=0x%{public}x", type);
    // FAT16 || NTFS/EXFAT || W95 FAT32 || W95 FAT32 || W95 FAT16 || EFI FAT32 || EXT 2/3/4
    if (type == 0x06 || type == 0x07 || type == 0x0b || type == 0x0c || type == 0x0e || type == 0x1b || type == 0x83) {
        if (CreateVolume(dev) == E_OK) {
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
        CreateVolume(device_);
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
        if (CreateMBRVolume(type, partitionDev)) {
            foundPart = true;
            LOGD("[L3:DiskInfo] CreateTableVolume: <<< EXIT SUCCESS <<< MBR volume created");
        } else {
            LOGE("[L3:DiskInfo] CreateTableVolume: Create MBR Volume failed, type=0x%{public}x", type);
        }
    } else if (table == Table::GPT) {
        if (CreateVolume(partitionDev) == E_OK) {
            foundPart = true;
            LOGD("[L3:DiskInfo] CreateTableVolume: <<< EXIT SUCCESS <<< GPT volume created");
        } else {
            LOGW("[L3:DiskInfo] CreateTableVolume: Create GPT Volume failed");
        }
    }
}

int DiskInfo::CreateVolume(dev_t dev)
{
    LOGI("[L3:DiskInfo] CreateVolume: >>> ENTER <<< dev=%{public}u,%{public}u", major(dev), minor(dev));
    auto &volume = VolumeManager::Instance();

    std::string volumeId = volume.CreateVolume(GetDiskId(), dev, isUserdata);
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
    std::vector<std::string> cmd = {"sgdisk", "-p", devPath_};
    std::vector<std::string> output;
    int32_t res = ForkExec(cmd, &output);
    if (res != E_OK) {
        return res;
    }
    for (auto str : output) {
        LOGI("get partition output: %{public}s", str.c_str());
    }
    std::vector<std::string> tempInfo;
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        tempInfo.insert(tempInfo.end(), split.begin(), split.end());
    }
    auto count = static_cast<int32_t>(tempInfo.size());
    int32_t partitionIndex = -1;
    for (int32_t i = 0; i < count; i++) {
        std::string buf = tempInfo[i];
        if (buf.find("Number") == 0 && buf.find("Start") != std::string::npos) {
            partitionIndex = i + 1;
            break;
        }
    }
    std::vector<OHOS::StorageManager::PartitionInfo> partitions;
    if (partitionIndex > 0 && partitionIndex < count) {
        for (int32_t i = partitionIndex; i < count; i++) {
            std::string buf = tempInfo[i];
            OHOS::StorageManager::PartitionInfo info;
            if (ParsePartitionInfo(buf, info)) {
                partitions.push_back(info);
            }
        }
    }
    partitionTableInfo.SetDiskId(diskId_);
    partitionTableInfo.SetTableType("GPT");
    partitionTableInfo.SetPartitionCount(static_cast<uint32_t>(partitions.size()));
    partitionTableInfo.SetTotalSector(totalSector_);
    partitionTableInfo.SetSectorSize(sectorSize_);
    partitionTableInfo.SetAlignSector(alignSector_);
    partitionTableInfo.SetPartitions(partitions);
    LOGI("[L3:DiskInfo] GetPartitionTable: >>> EXIT SUCCESS <<<");
    return E_OK;
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
    std::string path = "/dev/block/" + diskName_ + std::to_string(partitionNum);
    std::string fsType = GetBlkidData(path, "TYPE");
    info.SetFsType(fsType);
    LOGI("partition info is partitionNum=%{public}d, startSector=%{public}lld, endSector=%{public}lld, "
         "sizeBytes=%{public}lld, fsType=%{public}s", partitionNum, static_cast<long long>(startSector),
         static_cast<long long>(endSector), static_cast<long long>(sizeBytes), fsType.c_str());
    return true;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS

