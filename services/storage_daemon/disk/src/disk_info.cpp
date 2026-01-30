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
constexpr const char *SGDISK_PATH = "/system/bin/sgdisk";
constexpr const char *SGDISK_DUMP_CMD = "--ohos-dump";
constexpr const char *SGDISK_ZAP_CMD = "--zap-all";
constexpr const char *SGDISK_PART_CMD = "--new=0:0:-0 --typeconde=0:0c00 --gpttombr=1";
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

DiskInfo::DiskInfo(std::string &sysPath, std::string &devPath, dev_t device, int flag)
{
    id_ = StringPrintf("disk-%d-%d", major(device), minor(device));
    sysPath_ = sysPath;
    eventPath_ = devPath;
    devPath_ = StringPrintf("/dev/block/%s", id_.c_str());
    device_ = device;
    flags_ = static_cast<unsigned int>(flag);
    status = S_INITAL;
    isUserdata = false;
    sgdiskLines_ = std::vector<std::string>();
}

dev_t DiskInfo::GetDevice() const
{
    return device_;
}

std::string DiskInfo::GetId() const
{
    return id_;
}

std::string DiskInfo::GetDevPath() const
{
    return devPath_;
}

uint64_t DiskInfo::GetDevDSize() const
{
    return size_;
}

std::string DiskInfo::GetSysPath() const
{
    return sysPath_;
}

std::string DiskInfo::GetDevVendor() const
{
    return vendor_;
}

int DiskInfo::GetDevFlag() const
{
    return flags_;
}

DiskInfo::~DiskInfo()
{
    DestroyDiskNode(devPath_);
}

int DiskInfo::Create()
{
    int ret;

    CreateDiskNode(devPath_, device_);
    status = S_CREATE;
    ReadMetadata();

    StorageManagerClient client;
    ret = client.NotifyDiskCreated(*this);
    if (ret != E_OK) {
        LOGE("Notify Disk Created failed");
        return ret;
    }

    ret = ReadPartition();
    if (ret != E_OK) {
        LOGE("Create disk failed");
        return ret;
    }

    return E_OK;
}

int DiskInfo::Destroy()
{
    auto &volume = VolumeManager::Instance();

    for (auto volumeId : volumeId_) {
        auto ret = volume.DestroyVolume(volumeId);
        if (ret != E_OK) {
            LOGE("Destroy volume %{public}s failed", volumeId.c_str());
        }
    }
    status = S_DESTROY;
    volumeId_.clear();
    return E_OK;
}

void DiskInfo::ReadMetadata()
{
    size_ = -1;
    vendor_.clear();
    if (GetDevSize(devPath_, &size_) != E_OK) {
        size_ = -1;
    }

    unsigned int majorId = major(device_);
    if (majorId == DISK_MMC_MAJOR) {
        std::string path(sysPath_ + "/device/manfid");
        std::string str;
        if (!ReadFile(path, &str)) {
            LOGE("open file %{public}s failed", path.c_str());
            return;
        }
        LOGI("Raw manfid value from file: %{public}s", str.c_str());
        uint32_t manfid = 0;
        bool is_valid = ParseAndValidateManfid(str, manfid);
        if (is_valid) {
            auto it = vendorMap_.find(manfid);
            vendor_ = (it != vendorMap_.end()) ? it->second : "Unknown";
        } else {
            LOGI("Invalid manfid: %{public}s", str.c_str());
            vendor_ = "Invalid";
        }
    } else {
        std::string path(sysPath_ + "/device/vendor");
        std::string str;
        if (!ReadFile(path, &str)) {
            LOGE("open file %{public}s failed", path.c_str());
            return;
        }
        vendor_ = str;
        LOGI("Read metadata %{public}s", path.c_str());
    }
}

bool DiskInfo::ParseAndValidateManfid(const std::string& str, uint32_t& manfid)
{
    std::string trimmed = str;
    size_t start = trimmed.find_first_not_of(" \t\n\r");
    if (start != std::string::npos) {
        trimmed.erase(0, start);
    } else {
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
            return false;
        }
    }
    return true;
}

int DiskInfo::ReadPartition(const std::string &ejectStatus)
{
    int ret = 0;
    if (major(device_) == DISK_CD_MAJOR) {
        ret = ReadPartitionCD(ejectStatus);
    } else {
        ret = ReadPartitionUSB();
    }
    return ret;
}

int DiskInfo::ReadPartitionCD(const std::string &ejectStatus)
{
    std::string volumeId = StringPrintf("vol-%u-%u", major(device_), minor(device_));
    if (ejectStatus == "1") {
        for (auto volumeId : volumeId_) {
            auto ret = VolumeManager::Instance().DestroyVolume(volumeId);
            if (ret != E_OK) {
                LOGE("Destroy volume %{public}s failed", volumeId.c_str());
            }
        }
        volumeId_.clear();
        auto res = Eject(devPath_);
        if (res != E_OK) {
            LOGE("eject failed, %{public}d", res);
            return res;
        }
        return E_OK;
    }
 
    bool isExistCD = false;
    IsExistCD(devPath_, isExistCD);
    if (!isExistCD) {
        for (auto volumeId : volumeId_) {
            auto ret = VolumeManager::Instance().DestroyVolume(volumeId);
            if (ret != E_OK) {
                LOGE("Destroy volume %{public}s failed", volumeId.c_str());
            }
        }
        volumeId_.clear();
        return E_OK;
    } else {
        LOGI("ejectStatus is %{public}s", ejectStatus.c_str());
        dev_t partitionDev = makedev(major(device_), minor(device_));
        auto res = CreateVolume(partitionDev);
        if (res != E_OK) {
            LOGE("CreateVolume failed");
            return res;
        }
    }
    return E_OK;
}

int DiskInfo::ReadPartitionUSB()
{
    int maxVolumes = GetMaxVolume(device_);
    if (maxVolumes < 0) {
        LOGE("Invaild maxVolumes: %{public}d", maxVolumes);
        return E_ERR;
    }

    std::vector<std::string> output;
    std::vector<std::string> lines;
    std::vector<std::string> cmd = {SGDISK_PATH, SGDISK_DUMP_CMD, devPath_};
    int res = ForkExec(cmd, &output);
    FilterOutput(lines, output);
    if (res != E_OK || lines.empty()) {
        int destroyRes = Destroy();
        sgdiskLines_.clear();
        LOGE("get partition failed, destroy error is %{public}d", destroyRes);
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
        return E_OK;
    }
    return ReadDiskLines(sgdiskLines_, maxVolumes, isUserdata);
}

void DiskInfo::FilterOutput(std::vector<std::string> &lines, std::vector<std::string> &output)
{
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
        LOGE("disk info not found");
        return;
    }
    for (int32_t i = index; i < count; i++) {
        std::string target = tempInfo[i];
        if (std::find(lines.begin(), lines.end(), target) == lines.end()) {
            lines.push_back(target);
        }
    }
    LOGE("lines size is %{public}zu.", lines.size());
}

void DiskInfo::ProcessPartitionChanges(const std::vector<std::string>& lines, int maxVolumes, bool isUserdata)
{
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
        std::vector<std::string> SDLines;
        SDLines.reserve(addedLines.size() + 1);
        SDLines.push_back(lines.front());
        SDLines.insert(SDLines.end(), addedLines.begin(), addedLines.end());
        sgdiskLines_ = lines;
        if (ReadDiskLines(SDLines, maxVolumes, isUserdata) != E_OK) {
            LOGI("Failed to read disk lines ");
        }
    }
    if (!removedLines.empty()) {
        UmountLines(removedLines, maxVolumes, isUserdata);
        sgdiskLines_.clear();
        sgdiskLines_ = lines;
    }
}

void DiskInfo::UmountLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata)
{
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
                LOGE("Destroy volume %{public}s failed", volumeId.c_str());
            }
        }
    }
}

bool DiskInfo::CreateMBRVolume(int32_t type, dev_t dev)
{
    // FAT16 || NTFS/EXFAT || W95 FAT32 || W95 FAT32 || W95 FAT16 || EFI FAT32 || EXT 2/3/4
    if (type == 0x06 || type == 0x07 || type == 0x0b || type == 0x0c || type == 0x0e || type == 0x1b || type == 0x83) {
        if (CreateVolume(dev) == E_OK) {
            return true;
        }
    }
    return false;
}

int32_t DiskInfo::CreateUnknownTabVol()
{
    LOGI("%{public}s has unknown table", id_.c_str());
    std::string fsType;
    std::string uuid;
    std::string label;
    auto ret = OHOS::StorageDaemon::ReadMetadata(devPath_, fsType, uuid, label);
    if (ret == E_OK) {
        CreateVolume(device_);
    } else {
        StorageService::StorageRadar::ReportUserManager("DiskInfo::CreateUnknownTabVol::ReadMetadata", 0,
                                                        ret, "devPath_=" + devPath_);
        LOGE("failed to identify the disk device");
        return E_NON_EXIST;
    }
    return E_OK;
}

int32_t DiskInfo::ReadDiskLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata)
{
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
                LOGI("Unknown partition table %{public}s", (*it).c_str());
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
        return CreateUnknownTabVol();
    }

    return E_OK;
}

dev_t DiskInfo::ProcessPartition(std::vector<std::string>::iterator &it, int32_t maxVols, bool isUserdata)
{
    int32_t index = std::atoi((*it).c_str());
    unsigned int majorId = major(device_);
    if ((index > maxVols && majorId == DISK_MMC_MAJOR) || index < 1) {
        LOGE("Invalid partition %{public}d", index);
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
    return partitionDev;
}

int32_t DiskInfo::GetMaxMinor(int32_t major)
{
    DIR* dir;
    struct dirent* entry;
    int32_t maxMinor = -1;
    if ((dir = opendir(BLOCK_PATH)) == nullptr) {
        LOGE("fail to open %{public}s", BLOCK_PATH);
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
    return maxMinor;
}

void DiskInfo::CreateTableVolume(std::vector<std::string>::iterator &it, const std::vector<std::string>::iterator &end,
    Table table, bool &foundPart, dev_t partitionDev)
{
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
        } else {
            LOGE("Create MBR Volume failed");
        }
    } else if (table == Table::GPT) {
        if (CreateVolume(partitionDev) == E_OK) {
            foundPart = true;
        }
    }
}

int DiskInfo::CreateVolume(dev_t dev)
{
    auto &volume = VolumeManager::Instance();

    LOGI("disk read volume metadata");
    std::string volumeId = volume.CreateVolume(GetId(), dev, isUserdata);
    if (volumeId.empty()) {
        LOGE("Create volume failed");
        return E_ERR;
    }

    volumeId_.push_back(volumeId);
    return E_OK;
}

int DiskInfo::Partition()
{
    LOGI("Partitioning the disk.");
    std::vector<std::string> cmd;
    int res;

    res = Destroy();
    if (res != E_OK) {
        LOGE("Destroy failed in Partition()");
    }

    cmd.push_back(SGDISK_PATH);
    cmd.push_back(SGDISK_ZAP_CMD);
    cmd.push_back(devPath_);
    LOGI("Partition executing command.");
    std::vector<std::string> output;
    res = ForkExec(cmd, &output);
    if (res != E_OK) {
        LOGE("sgdisk: zap fail");
        return res;
    }

    cmd.clear();
    output.clear();
    cmd.push_back(SGDISK_PATH);
    cmd.push_back(SGDISK_PART_CMD);
    cmd.push_back(devPath_);
    res = ForkExec(cmd, &output);
    if (res != E_OK) {
        LOGE("sgdisk: partition fail");
        return res;
    }

    return E_OK;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
