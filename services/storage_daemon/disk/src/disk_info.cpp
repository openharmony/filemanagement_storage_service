/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
constexpr const char *SGDISK_PATH = "/system/bin/sgdisk";
constexpr const char *SGDISK_DUMP_CMD = "--ohos-dump";
constexpr const char *SGDISK_ZAP_CMD = "--zap-all";
constexpr const char *SGDISK_PART_CMD = "--new=0:0:-0 --typeconde=0:0c00 --gpttombr=1";
constexpr const char *BLOCK_PATH = "/dev/block";

enum DiskStatus:int {
    S_INITAL = 0,
    S_CREATE = 1,
    S_SCAN = 2,
    S_DESTROY = 4,
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
    auto volume = VolumeManager::Instance();

    for (auto volumeId : volumeId_) {
        auto ret = volume->DestroyVolume(volumeId);
        if (ret != E_OK) {
            LOGE("Destroy volume %{public}s failed", volumeId.c_str());
            return E_ERR;
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
        int manfid = std::stoi(str);
        switch (manfid) {
            case 0x000003: {
                vendor_ = "SanDisk";
                break;
            }
            case 0x00001b: {
                vendor_ = "SamSung";
                break;
            }
            case 0x000028: {
                vendor_ = "Lexar";
                break;
            }
            case 0x000074: {
                vendor_ = "Transcend";
                break;
            }
            default : {
                vendor_ = "Unknown";
                LOGI("Unknown vendor information: %{public}d", manfid);
                break;
            }
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

int DiskInfo::ReadPartition()
{
    int maxVolumes = GetMaxVolume(device_);
    if (maxVolumes < 0) {
        LOGE("Invaild maxVolumes: %{public}d", maxVolumes);
        return E_ERR;
    }
    int res = Destroy();
    if (res != E_OK) {
        LOGE("Destroy failed in ReadPartition");
    }

    std::vector<std::string> cmd;
    std::vector<std::string> output;
    std::vector<std::string> lines;

    cmd.push_back(SGDISK_PATH);
    cmd.push_back(SGDISK_DUMP_CMD);
    cmd.push_back(devPath_);
    res = ForkExec(cmd, &output);
    if (res != E_OK) {
        LOGE("get %{private}s partition failed", devPath_.c_str());
        return res;
    }
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        for (auto &tmp : split)
            lines.push_back(tmp);
    }
    isUserdata = false;
    if (lines.size() > MIN_LINES) {
        auto userdataIt = std::find_if(lines.begin(), lines.end(), [](const std::string &str) {
            return str.find("userdata") != std::string::npos;
        });
        if (userdataIt != lines.end()) {
            std::vector<std::string> hmfsLines;
            hmfsLines.push_back(lines.front());
            hmfsLines.push_back(*userdataIt);
            status = S_SCAN;
            return ReadDiskLines(hmfsLines, maxVolumes);
        }
    }
    status = S_SCAN;
    return ReadDiskLines(lines, maxVolumes);
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

int32_t DiskInfo::ReadDiskLines(std::vector<std::string> lines, int32_t maxVols)
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
            ProcessPartition(it, split.end(), table, maxVols, foundPart);
        }
    }

    if (table == Table::UNKNOWN || !foundPart) {
        return CreateUnknownTabVol();
    }

    return E_OK;
}

void DiskInfo::ProcessPartition(std::vector<std::string>::iterator &it, const std::vector<std::string>::iterator &end,
                                Table table, int32_t maxVols, bool &foundPart)
{
    if (++it == end) {
        return;
    }
    int32_t index = std::stoi(*it);
    unsigned int majorId = major(device_);
    if ((index > maxVols && majorId == DISK_MMC_MAJOR) || index < 1) {
        LOGE("Invalid partition %{public}d", index);
        return;
    }
    dev_t partitionDev;
    if (index > MAX_SCSI_VOLUMES) {
        int32_t maxMinor = GetMaxMinor(MAJORID_BLKEXT);
        if (maxMinor == -1 && minor(device_) == 0) {
            partitionDev = makedev(MAJORID_BLKEXT, minor(device_) + static_cast<uint32_t>(index) - MAX_PARTITION);
        } else if (maxMinor == -1 && minor(device_) == MAX_PARTITION) {
            partitionDev = makedev(MAJORID_BLKEXT, static_cast<uint32_t>(index) - MAX_PARTITION);
        } else {
            partitionDev = makedev(MAJORID_BLKEXT, maxMinor + static_cast<uint32_t>(index) - MAX_INTERVAL_PARTITION);
        }
    } else {
        partitionDev = makedev(major(device_), minor(device_) + static_cast<uint32_t>(index));
    }

    if (table == Table::MBR) {
        if (++it == end) {
            return;
        }
        int32_t type = std::stoi("0x0" + *it, 0, 16);
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

int32_t DiskInfo::GetMaxMinor(int32_t major)
{
    DIR* dir;
    struct dirent* entry;
    int maxMinor = -1;
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
            int majorNum = major(statbuf.st_rdev);
            int minorNum = minor(statbuf.st_rdev);

            if (majorNum == major) {
                maxMinor = minorNum > maxMinor ? minorNum : maxMinor;
            }
        }
    }
    closedir(dir);
    return maxMinor;
}

int DiskInfo::CreateVolume(dev_t dev)
{
    auto volume = VolumeManager::Instance();

    LOGI("disk read volume metadata");
    std::string volumeId = volume->CreateVolume(GetId(), dev, isUserdata);
    if (volumeId == "") {
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
    res = ForkExec(cmd);
    if (res != E_OK) {
        LOGE("sgdisk: zap fail");
        return res;
    }

    cmd.clear();
    cmd.push_back(SGDISK_PATH);
    cmd.push_back(SGDISK_PART_CMD);
    cmd.push_back(devPath_);
    res = ForkExec(cmd);
    if (res != E_OK) {
        LOGE("sgdisk: partition fail");
        return res;
    }

    return E_OK;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
