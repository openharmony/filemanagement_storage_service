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

#include "disk/disk_info.h"

#include <sys/sysmacros.h>

#include "disk/disk_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "volume/volume_manager.h"

namespace OHOS {
namespace StorageDaemon {
const std::string sgdiskPath = "/system/bin/sgdisk";
const std::string sgdiskDumpCmd = "--ohos-dump";
const std::string sgdiskZapCmd = "--zap-all";
const std::string sgdiskPartCmd = "--new=0:0:-0 --typeconde=0:0c00 --gpttombr=1";

DiskInfo::DiskInfo(std::string sysPath, std::string devPath, dev_t device, int flag)
{
    id_ = StringPrintf("disk-%d-%d", major(device), minor(device));
    sysPath_ = sysPath;
    eventPath_ = devPath;
    devPath_ = StringPrintf("/dev/block/%s", id_.c_str());
    device_ = device;
    flags_ = flag;
    status = sInital;
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
    CreateDiskNode(devPath_, device_);
    status = sCreate;
    ReadMetadata();
    ReadPartition();
    return E_OK;
}

int DiskInfo::Destroy()
{
    auto volume = VolumeManager::Instance();
    for (auto volumeId : volumeId_) {
        volume->DestroyVolume(volumeId);
    }
    status = sDestroy;
    volumeId_.clear();
    return E_OK;
}

int DiskInfo::ReadMetadata()
{
    size_ = -1;
    vendor_.clear();
    if (GetDevSize(devPath_, &size_) != E_OK) {
        size_ = -1;
    }

    unsigned int majorId = major(device_);
    if (majorId == diskMmc) {
        std::string path(sysPath_ + "/device/manfid");
        std::string str;
        if (!ReadFile(path, &str)) {
            LOGE("open file %{public}s failed", path.c_str());
            return E_ERR;
        }
        unsigned int manfid = std::stoi(str);
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
                LOGE("Unknown vendor information: %{public}d", manfid);
                break;
            }
        }
    } else {
        std::string path(sysPath_ + "/device/vendor");
        std::string str;
        if (!ReadFile(path, &str)) {
            LOGE("open file %{public}s failed", path.c_str());
            return E_ERR;
        }
        vendor_ = str;
    LOGI("Read metadata %{public}s", path.c_str());
    }
    return E_OK;
}

int DiskInfo::ReadPartition()
{
    int maxVolumes = GetMaxVolume(device_);
    if (maxVolumes < 0) {
        LOGE("Invaild maxVolumes");
        return E_ERR;
    }

    std::vector<std::string> cmd;
    std::vector<std::string> output;
    std::vector<std::string> lines;
    int res;

    cmd.push_back(sgdiskPath);
    cmd.push_back(sgdiskDumpCmd);
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

    std::string lineToken = " \t";
    status = sScan;
    for (auto &line : lines) {
        auto split = SplitLine(line, lineToken);
        auto it = split.begin();
        if (it == split.end()) {
            continue;
        }
        if (*it == "DISK") {
            continue;
        } else if (*it == "PART") {
            if (++it == split.end()) {
                continue;
            }
            int index = std::stoi(*it);
            if (index > maxVolumes || index < 1) {
                LOGE("Invalid partition %{public}d", index);
                continue;
            }
            dev_t partitionDev = makedev(major(device_), minor(device_) + index);
            CreateVolume(partitionDev);
        }
    }
    return E_OK;
}

int DiskInfo::CreateVolume(dev_t dev)
{
    auto volume = VolumeManager::Instance();

    LOGI("disk read volume metadata");
    std::string volumeId = volume->CreateVolume(GetId(), dev);
    volumeId_.push_back(volumeId);
    return E_OK;
}

int DiskInfo::Partition()
{
    std::vector<std::string> cmd;
    int res;

    Destroy();
    cmd.push_back(sgdiskPath);
    cmd.push_back(sgdiskZapCmd);
    cmd.push_back(devPath_);
    res = ForkExec(cmd);
    if (res != E_OK) {
        LOGW("sgdisk: zap fail");
    }

    cmd.clear();
    cmd.push_back(sgdiskPath);
    cmd.push_back(sgdiskPartCmd);
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