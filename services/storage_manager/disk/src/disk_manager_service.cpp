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

#include "disk/disk_manager_service.h"

#include "disk/disk_notification.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_utils.h"
#include "utils/file_utils.h"
#include "volume/volume_manager_service.h"

namespace OHOS {
namespace StorageManager {
const std::string UNDEFINED_FS_TYPE = "undefined";

DiskManagerService::DiskManagerService() {}

DiskManagerService::~DiskManagerService() {}

DiskManagerService &DiskManagerService::GetInstance()
{
    static DiskManagerService instance;
    return instance;
}

std::shared_ptr<Disk> DiskManagerService::GetDiskById(const std::string &diskId)
{
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    if (diskMap_.find(diskId) == diskMap_.end()) {
        LOGE("DiskManagerService::GetDiskById id %{public}s not exists", diskId.c_str());
        return nullptr;
    }
    auto diskPtr = diskMap_[diskId];
    if (diskPtr == nullptr) {
        return nullptr;
    }
    return diskPtr;
}

void DiskManagerService::OnDiskCreated(const Disk &disk)
{
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    if (diskMap_.find(disk.GetDiskId()) != diskMap_.end()) {
        LOGE("DiskManagerService::OnDiskCreated the disk %{public}s already exists",
            GetAnonyString(disk.GetDiskId()).c_str());
        return;
    }
    auto diskPtr = std::make_shared<Disk>(disk);
    diskMap_.insert(make_pair(diskPtr->GetDiskId(), diskPtr));
    DiskNotification::GetInstance().NotifyDiskChange(StorageDaemon::DiskInfo::DiskState::MOUNTED, diskPtr);
}

void DiskManagerService::OnDiskDestroyed(const std::string &diskId)
{
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    if (diskMap_.find(diskId) == diskMap_.end()) {
        LOGE("DiskManagerService::OnDiskDestroyed the disk %{public}s doesn't exist", GetAnonyString(diskId).c_str());
        return;
    }
    auto diskPtr = diskMap_[diskId];
    DiskNotification::GetInstance().NotifyDiskChange(StorageDaemon::DiskInfo::DiskState::REMOVED, diskPtr);
    diskMap_.erase(diskId);
}

int32_t DiskManagerService::Partition(const std::string &diskId, int32_t type)
{
    bool isUsbFuseByType = VolumeManagerService::GetInstance().IsUsbFuseByType(UNDEFINED_FS_TYPE);
    if (isUsbFuseByType) {
        LOGE("DiskManagerService::The disk %{public}s is fuse, not support", GetAnonyString(diskId).c_str());
        return E_NOT_SUPPORT;
    }
    {
        std::lock_guard<std::mutex> lock(diskMapMutex_);
        if (diskMap_.find(diskId) == diskMap_.end()) {
            LOGE("DiskManagerService::Partition the disk %{public}s doesn't exist", GetAnonyString(diskId).c_str());
            return E_NON_EXIST;
        }
    }

    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->Partition(diskId, type);
    return err;
}

std::vector<Disk> DiskManagerService::GetAllDisks()
{
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    std::vector<Disk> result;
    for (auto it = diskMap_.begin(); it != diskMap_.end(); ++it) {
        Disk disk = *(it->second);
        result.push_back(disk);
    }
    return result;
}

int32_t DiskManagerService::GetDiskById(const std::string &diskId, Disk &disk)
{
    std::lock_guard<std::mutex> lock(diskMapMutex_);
    if (diskMap_.find(diskId) != diskMap_.end()) {
        disk = *diskMap_[diskId];
        return E_OK;
    }
    LOGE("DiskManagerService::GetDiskById the disk %{public}s doesn't exist", diskId.c_str());
    return E_NON_EXIST;
}

int32_t DiskManagerService::GetPartitionTable(const std::string &diskId, PartitionTableInfo &partitionTableInfo)
{
    {
        std::lock_guard<std::mutex> lock(diskMapMutex_);
        if (diskMap_.find(diskId) == diskMap_.end()) {
            LOGE("the disk %{public}s not exist", diskId.c_str());
            return E_NON_EXIST;
        }
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->GetPartitionTable(diskId, partitionTableInfo);
}

int32_t DiskManagerService::CreatePartition(const std::string &diskId, const PartitionOptions &partitionOption)
{
    {
        std::lock_guard<std::mutex> lock(diskMapMutex_);
        if (diskMap_.find(diskId) == diskMap_.end()) {
            LOGE("the disk %{public}s not exist", diskId.c_str());
            return E_NON_EXIST;
        }
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->CreatePartition(diskId, partitionOption);
}

int32_t DiskManagerService::DeletePartition(const std::string &diskId, uint32_t partitionNum)
{
    {
        std::lock_guard<std::mutex> lock(diskMapMutex_);
        if (diskMap_.find(diskId) == diskMap_.end()) {
            LOGE("the disk %{public}s not exist", diskId.c_str());
            return E_NON_EXIST;
        }
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->DeletePartition(diskId, partitionNum);
}
}
}
