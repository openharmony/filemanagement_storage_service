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

#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

DiskManager::~DiskManager()
{
    LOGI("Destroy DiskManager");
}

void DiskManager::HandleDiskEvent(NetlinkData *data)
{
    if (data == nullptr) {
        LOGE("data is nullptr");
        return;
    }
    std::string devType = data->GetParam("DEVTYPE");
    if (devType != "disk") {
        return;
    }

    unsigned int major = (unsigned int) std::atoi((data->GetParam("MAJOR")).c_str());
    unsigned int minor = (unsigned int) std::atoi((data->GetParam("MINOR")).c_str());
    dev_t device = makedev(major, minor);

    switch (data->GetAction()) {
        case NetlinkData::Actions::ADD: {
            auto diskInfo = MatchConfig(data);
            if (diskInfo == nullptr) {
                LOGI("Can't match config, devPath is %{public}s", data->GetDevpath().c_str());
            } else {
                CreateDisk(diskInfo);
                LOGI("Handle Disk Add Event");
            }
            break;
        }
        case NetlinkData::Actions::CHANGE: {
            ChangeDisk(device, data);
            LOGI("Handle Disk Change Event");
            break;
        }
        case NetlinkData::Actions::REMOVE: {
            DestroyDisk(device);
            LOGI("Handle Disk Remove Event");
            break;
        }
        default: {
            LOGW("Cannot handle unexpected disk event %{public}d", data->GetAction());
            break;
        }
    }
}

std::shared_ptr<DiskInfo> DiskManager::MatchConfig(NetlinkData *data)
{
    if (data == nullptr) {
        LOGE("data is nullptr");
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
                flag |= DiskInfo::DeviceFlag::SD_FLAG;
            } else if (major == DISK_CD_MAJOR) {
                flag |= DiskInfo::DeviceFlag::CD_FLAG;
            } else {
                flag |= DiskInfo::DeviceFlag::USB_FLAG;
            }
            auto diskInfo =  std::make_shared<DiskInfo>(sysPath, devPath, device, static_cast<int>(flag));
            return diskInfo;
        }
    }

    LOGI("No matching configuration found");
    return nullptr;
}

void DiskManager::CreateDisk(std::shared_ptr<DiskInfo> &diskInfo)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (diskInfo == nullptr) {
        LOGE("diskInfo is nullptr");
        return;
    }

    int ret = diskInfo->Create();
    if (ret != E_OK) {
        LOGE("Create DiskInfo failed");
        return;
    }
    disk_.push_back(diskInfo);
}

void DiskManager::ChangeDisk(dev_t device, NetlinkData *data)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto &diskInfo : disk_) {
            if ((diskInfo != nullptr) && (diskInfo->GetDevice() == device)) {
                diskInfo->ReadMetadata();
                diskInfo->ReadPartition();
                return;
            }
        }
    }
    auto diskInfo = MatchConfig(data);
    if (diskInfo == nullptr) {
        LOGI("Can't match config, devPath is %{public}s", data->GetDevpath().c_str());
    } else {
        CreateDisk(diskInfo);
        LOGI("Handle Disk Add Event");
    }
}

void DiskManager::DestroyDisk(dev_t device)
{
    int ret;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end();) {
        if (*i != nullptr && (*i)->GetDevice() == device) {
            ret = (*i)->Destroy();
            if (ret != E_OK) {
                LOGE("Destroy DiskInfo failed");
                return;
            }

            StorageManagerClient client;
            ret = client.NotifyDiskDestroyed((*i)->GetId());
            if (ret != E_OK) {
                LOGI("Notify Disk Destroyed failed");
            }
            i = disk_.erase(i);
        } else {
            i++;
        }
    }
}

void DiskManager::AddDiskConfig(std::shared_ptr<DiskConfig> &diskConfig)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (diskConfig != nullptr) {
        diskConfig_.push_back(diskConfig);
    }
}

void DiskManager::ReplayUevent()
{
    TraverseDirUevent(sysBlockPath_, true);
}

int32_t DiskManager::HandlePartition(std::string diskId)
{
    int32_t ret = E_NON_EXIST;
    std::lock_guard<std::mutex> lock(lock_);
    for (auto i = disk_.begin(); i != disk_.end(); i++) {
        if (*i == nullptr) {
            continue;
        }

        if ((*i)->GetId() == diskId) {
            ret = (*i)->Partition();
            break;
        }
    }

    return ret;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
