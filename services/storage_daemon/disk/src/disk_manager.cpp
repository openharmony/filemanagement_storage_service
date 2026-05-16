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

#include "disk/disk_manager.h"

#include <sys/sysmacros.h>
#include <cinttypes>

#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

DiskManager &DiskManager::Instance(void)
{
    static DiskManager instance;
    return instance;
}

DiskManager::~DiskManager()
{
    LOGI("[L2:DiskManager] ~DiskManager: >>> ENTER <<<");
}

void DiskManager::HandleDiskEvent(NetlinkData *data)
{
    LOGI("[L2:DiskManager] HandleDiskEvent: >>> ENTER <<<");
    if (data == nullptr) {
        LOGE("[L2:DiskManager] HandleDiskEvent: <<< EXIT FAILED <<< data is nullptr");
        return;
    }
    std::string devType = data->GetParam("DEVTYPE");
    if (devType != "disk") {
        LOGD("[L2:DiskManager] HandleDiskEvent: devType=%{public}s, not disk, skip", devType.c_str());
        return;
    }

    unsigned int major = (unsigned int) std::atoi((data->GetParam("MAJOR")).c_str());
    unsigned int minor = (unsigned int) std::atoi((data->GetParam("MINOR")).c_str());
    dev_t device = makedev(major, minor);

    switch (data->GetAction()) {
        case NetlinkData::Actions::ADD: {
            auto diskInfo = MatchConfig(data);
            if (diskInfo == nullptr) {
                LOGI("[L2:DiskManager] HandleDiskEvent: Can't match config, devPath=%{public}s",
                     data->GetDevpath().c_str());
            } else {
                CreateDisk(diskInfo);
                LOGI("[L2:DiskManager] HandleDiskEvent: <<< EXIT SUCCESS <<< action=ADD");
            }
            break;
        }
        case NetlinkData::Actions::CHANGE: {
            ChangeDisk(device, data);
            LOGI("[L2:DiskManager] HandleDiskEvent: <<< EXIT SUCCESS <<< action=CHANGE");
            break;
        }
        case NetlinkData::Actions::REMOVE: {
            DestroyDisk(device);
            LOGI("[L2:DiskManager] HandleDiskEvent: <<< EXIT SUCCESS <<< action=REMOVE");
            break;
        }
        default: {
            LOGW("[L2:DiskManager] HandleDiskEvent: unexpected event, action=%{public}d", data->GetAction());
            break;
        }
    }
    LOGI("[L2:DiskManager] HandleDiskEvent: <<< EXIT SUCCESS <<<");
}

std::shared_ptr<DiskInfo> DiskManager::MatchConfig(NetlinkData *data)
{
    LOGI("[L2:DiskManager] MatchConfig: >>> ENTER <<<");
    if (data == nullptr) {
        LOGE("[L2:DiskManager] MatchConfig: <<< EXIT FAILED <<< data is nullptr");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(lock_);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = (unsigned int) std::atoi((data->GetParam("MAJOR")).c_str());
    unsigned int minor = (unsigned int) std::atoi((data->GetParam("MINOR")).c_str());
    dev_t device = makedev(major, minor);

    for (auto config : diskConfig_) {
        if ((config != nullptr) && config->IsMatch(devPath)) {
            uint32_t flag = static_cast<uint32_t>(config->GetFlag());
            if (major == DISK_MMC_MAJOR) {
                flag |= DiskInfo::DiskType::SD_CARD;
            } else if (major == DISK_CD_MAJOR) {
                flag |= DiskInfo::DiskType::CD_DVD_BD;
            } else {
                flag |= DiskInfo::DiskType::USB_FLASH;
            }
            std::string diskName = data->GetDiskName();
            auto diskInfo =  std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, static_cast<int>(flag));
            LOGI("[L2:DiskManager] MatchConfig: <<< EXIT SUCCESS <<< devPath=%{public}s, matched", devPath.c_str());
            return diskInfo;
        }
    }

    LOGI("[L2:DiskManager] MatchConfig: <<< EXIT SUCCESS <<< No matching configuration found");
    return nullptr;
}

void DiskManager::CreateDisk(std::shared_ptr<DiskInfo> &diskInfo)
{
    LOGI("[L2:DiskManager] CreateDisk: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(lock_);
    if (diskInfo == nullptr) {
        LOGE("[L2:DiskManager] CreateDisk: <<< EXIT FAILED <<< diskInfo is nullptr");
        return;
    }

    int ret = diskInfo->Create();
    if (ret != E_OK) {
        LOGE("[L2:DiskManager] CreateDisk: <<< EXIT FAILED <<< Create DiskInfo failed, err=%{public}d", ret);
        return;
    }
    disk_.push_back(diskInfo);
    LOGI("[L2:DiskManager] CreateDisk: <<< EXIT SUCCESS <<<");
}

void DiskManager::ChangeDisk(dev_t device, NetlinkData *data)
{
    LOGI("[L2:DiskManager] ChangeDisk: >>> ENTER <<< device=%{public}" PRIu64, device);
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto &diskInfo : disk_) {
            if ((diskInfo != nullptr) && (diskInfo->GetDevice() == device)) {
                diskInfo->ReadMetadata();
                diskInfo->ReadPartition(data->GetEjectRequest());
                LOGI("[L2:DiskManager] ChangeDisk: <<< EXIT SUCCESS <<< disk found and updated");
                return;
            }
        }
    }
    auto diskInfo = MatchConfig(data);
    if (diskInfo == nullptr) {
        LOGI("[L2:DiskManager] ChangeDisk: Can't match config, devPath=%{public}s",
             data->GetDevpath().c_str());
    } else {
        CreateDisk(diskInfo);
        LOGI("[L2:DiskManager] ChangeDisk: <<< EXIT SUCCESS <<< new disk created");
    }
}

void DiskManager::DestroyDisk(dev_t device)
{
    LOGI("[L2:DiskManager] DestroyDisk: >>> ENTER <<< device=%{public}" PRIu64, device);
    int ret;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end();) {
        if (*i != nullptr && (*i)->GetDevice() == device) {
            ret = (*i)->Destroy();
            if (ret != E_OK) {
                LOGE("[L2:DiskManager] DestroyDisk: <<< EXIT FAILED <<< Destroy DiskInfo failed, err=%{public}d", ret);
                return;
            }

            StorageManagerClient client;
            ret = client.NotifyDiskDestroyed((*i)->GetDiskId());
            if (ret != E_OK) {
                LOGI("[L2:DiskManager] DestroyDisk: Notify Disk Destroyed failed, err=%{public}d", ret);
            }
            i = disk_.erase(i);
        } else {
            i++;
        }
    }
    LOGI("[L2:DiskManager] DestroyDisk: <<< EXIT SUCCESS <<<");
}

void DiskManager::AddDiskConfig(std::shared_ptr<DiskConfig> &diskConfig)
{
    LOGI("[L2:DiskManager] AddDiskConfig: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(lock_);
    if (diskConfig != nullptr) {
        diskConfig_.push_back(diskConfig);
    }
    LOGI("[L2:DiskManager] AddDiskConfig: <<< EXIT SUCCESS <<<");
}

void DiskManager::ReplayUevent()
{
    LOGI("[L2:DiskManager] ReplayUevent: >>> ENTER <<<");
    TraverseDirUevent(sysBlockPath_, true);
    LOGI("[L2:DiskManager] ReplayUevent: <<< EXIT SUCCESS <<<");
}

int32_t DiskManager::HandlePartition(const std::string &diskId)
{
    LOGI("[L2:DiskManager] HandlePartition: >>> ENTER <<< diskId=%{public}s", diskId.c_str());
    int32_t ret = E_NON_EXIST;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }

        if ((*i)->GetDiskId() == diskId) {
            ret = (*i)->Partition();
            break;
        }
    }

    if (ret == E_OK) {
        LOGI("[L2:DiskManager] HandlePartition: <<< EXIT SUCCESS <<< diskId=%{public}s", diskId.c_str());
    } else {
        LOGE("[L2:DiskManager] HandlePartition: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId.c_str(), ret);
    }
    return ret;
}

int32_t DiskManager::HandleGetPartitionTable(const std::string &diskId,
    OHOS::StorageManager::PartitionTableInfo &partitionTableInfo)
{
    LOGI("[L2:DiskManager] HandleGetPartitionTable: >>> ENTER <<< diskId=%{public}s", diskId.c_str());
    int32_t ret = E_NON_EXIST;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }
        if ((*i)->GetDiskId() == diskId) {
            ret = (*i)->GetPartitionTable(partitionTableInfo);
            break;
        }
    }
    if (ret == E_OK) {
        LOGI("[L2:DiskManager] HandleGetPartitionTable: <<< EXIT SUCCESS <<< diskId=%{public}s, "
             "partitionCount=%{public}u", diskId.c_str(), partitionTableInfo.GetPartitionCount());
    } else {
        LOGE("[L2:DiskManager] HandleGetPartitionTable: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId.c_str(), ret);
    }
    return ret;
}

int32_t DiskManager::HandleCreatePartition(const std::string &diskId,
    const OHOS::StorageManager::PartitionParams &partitionParams)
{
    LOGI("[L2:DiskManager] HandleCreatePartition: >>> ENTER <<< diskId=%{public}s", diskId.c_str());
    int32_t ret = E_NON_EXIST;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }
        if ((*i)->GetDiskId() == diskId) {
            ret = (*i)->CreatePartition(partitionParams);
            break;
        }
    }
    if (ret == E_OK) {
        LOGI("[L2:DiskManager] HandleCreatePartition: <<< EXIT SUCCESS <<< diskId=%{public}s", diskId.c_str());
    } else {
        LOGE("[L2:DiskManager] HandleCreatePartition: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId.c_str(), ret);
    }
    return ret;
}

int32_t DiskManager::HandleDeletePartition(const std::string &diskId, uint32_t partitionNum)
{
    LOGI("[L2:DiskManager] HandleDeletePartition: >>> ENTER <<< diskId=%{public}s, partitionNum=%{public}u",
         diskId.c_str(), partitionNum);
    int32_t ret = E_NON_EXIST;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }
        if ((*i)->GetDiskId() == diskId) {
            ret = (*i)->DeletePartition(partitionNum);
            break;
        }
    }
    if (ret == E_OK) {
        LOGI("[L2:DiskManager] HandleDeletePartition: <<< EXIT SUCCESS <<< diskId=%{public}s, partitionNum=%{public}u",
             diskId.c_str(), partitionNum);
    } else {
        LOGE("[L2:DiskManager] HandleDeletePartition: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId.c_str(), ret);
    }
    return ret;
}

int32_t DiskManager::HandleFormatPartition(const std::string &diskId, uint32_t partitionNum,
    const OHOS::StorageManager::FormatParams &formatParams)
{
    LOGI("[L2:DiskManager] HandleFormatPartition: >>> ENTER <<< diskId=%{public}s, partitionNum=%{public}u",
         diskId.c_str(), partitionNum);
    int32_t ret = E_NON_EXIST;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }
        if ((*i)->GetDiskId() == diskId) {
            ret = (*i)->FormatPartition(partitionNum, formatParams);
            break;
        }
    }
    if (ret == E_OK) {
        LOGI("[L2:DiskManager] HandleFormatPartition: <<< EXIT SUCCESS <<< diskId=%{public}s, partitionNum=%{public}u",
             diskId.c_str(), partitionNum);
    } else {
        LOGE("[L2:DiskManager] HandleFormatPartition: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId.c_str(), ret);
    }
    return ret;
}

int32_t DiskManager::HandleEject(const std::string &diskId)
{
    LOGI("[L2:DiskManager] HandleEject: >>> ENTER <<< diskId=%{public}s", diskId.c_str());
    int32_t ret = E_NON_EXIST;
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }

        if ((*i)->GetDiskId() == diskId) {
            ret = (*i)->EjectDisk();
            break;
        }
    }

    if (ret == E_OK) {
        LOGI("[L2:DiskManager] HandleEject: <<< EXIT SUCCESS <<< diskId=%{public}s", diskId.c_str());
    } else {
        LOGE("[L2:DiskManager] HandleEject: <<< EXIT FAILED <<< diskId=%{public}s, err=%{public}d",
             diskId.c_str(), ret);
    }
    return ret;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
