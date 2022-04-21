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

#include "ipc/storage_manager.h"
#include <storage/storage_status_service.h>
#include <storage/storage_total_status_service.h>
#include <singleton.h>
#include "system_ability_definition.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "account_subscriber/account_subscriber.h"
#include "user/multi_user_manager_service.h"
#include "volume/volume_manager_service.h"
#include "disk/disk_manager_service.h"
#include "crypto/filesystem_crypto.h"


namespace OHOS {
namespace StorageManager {
REGISTER_SYSTEM_ABILITY_BY_ID(StorageManager, STORAGE_MANAGER_MANAGER_ID, true);

void StorageManager::OnStart()
{
    LOGI("StorageManager::OnStart Begin");
    bool res = SystemAbility::Publish(this);
    AccountSubscriber::Subscriber();
    LOGI("StorageManager::OnStart End, res = %{public}d", res);
}

void StorageManager::OnStop()
{
    LOGI("StorageManager::Onstop Done");
}

int32_t StorageManager::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManager::PrepareAddUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->PrepareAddUser(userId, flags);
    return err;
}

int32_t StorageManager::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManger::RemoveUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->RemoveUser(userId, flags);
    return err;
}

int32_t StorageManager::PrepareStartUser(int32_t userId)
{
    LOGI("StorageManger::PrepareStartUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->PrepareStartUser(userId);
    return err;
}

int32_t StorageManager::StopUser(int32_t userId)
{
    LOGI("StorageManger::StopUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->StopUser(userId);
    return err;
}

int64_t StorageManager::GetFreeSizeOfVolume(std::string volumeUuid)
{
    LOGI("StorageManger::getFreeSizeOfVolume start, volumeUuid: %{public}s", volumeUuid.c_str());
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetFreeSizeOfVolume(volumeUuid);
    return result;
}

int64_t StorageManager::GetTotalSizeOfVolume(std::string volumeUuid)
{
    LOGI("StorageManger::getTotalSizeOfVolume start, volumeUuid: %{public}s", volumeUuid.c_str());
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSizeOfVolume(volumeUuid);
    return result;
}

BundleStats StorageManager::GetBundleStats(std::string pkgName)
{
    LOGI("StorageManger::getBundleStats start, pkgName: %{public}s", pkgName.c_str());
    BundleStats result = DelayedSingleton<StorageStatusService>::GetInstance()->GetBundleStats(pkgName);
    return result;
}

int64_t StorageManager::GetSystemSize()
{
    LOGI("StorageManger::getSystemSize start");
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetSystemSize();
    return result;
}

int64_t StorageManager::GetTotalSize()
{
    LOGI("StorageManger::getTotalSize start");
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize();
    return result;
}

int64_t StorageManager::GetFreeSize()
{
    LOGI("StorageManger::getFreeSize start");
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetFreeSize();
    return result;
}

StorageStats StorageManager::GetUserStorageStats()
{
    LOGI("StorageManger::GetUserStorageStats start");
    StorageStats result = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStats();
    return result;
}

StorageStats StorageManager::GetUserStorageStats(int32_t userId)
{
    LOGI("StorageManger::GetUserStorageStats start");
    StorageStats result = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStats(userId);
    return result;
}

BundleStats StorageManager::GetCurrentBundleStats()
{
    LOGI("StorageManger::GetCurrentBundleStats start");
    BundleStats result = DelayedSingleton<StorageStatusService>::GetInstance()->GetCurrentBundleStats();
    return result;
}

void StorageManager::NotifyVolumeCreated(VolumeCore vc)
{
    LOGI("StorageManger::NotifyVolumeCreated start, volumeId: %{public}s", vc.GetId().c_str());
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeCreated(vc);
}

void StorageManager::NotifyVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
    std::string path, std::string description)
{
    LOGI("StorageManger::NotifyVolumeMounted start");
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
}

void StorageManager::NotifyVolumeDestroyed(std::string volumeId)
{
    LOGI("StorageManger::NotifyVolumeDestroyed start");
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeDestroyed(volumeId);
}

int32_t StorageManager::Mount(std::string volumeId)
{
    LOGI("StorageManger::Mount start");
    int result = DelayedSingleton<VolumeManagerService>::GetInstance()->Mount(volumeId);
    return result;
}

int32_t StorageManager::Unmount(std::string volumeId)
{
    LOGI("StorageManger::Unmount start");
    int result = DelayedSingleton<VolumeManagerService>::GetInstance()->Unmount(volumeId);
    return result;
}

std::vector<VolumeExternal> StorageManager::GetAllVolumes()
{
    LOGI("StorageManger::GetAllVolumes start");
    std::vector<VolumeExternal> result = DelayedSingleton<VolumeManagerService>::GetInstance()->GetAllVolumes();
    return result;
}

void StorageManager::NotifyDiskCreated(Disk disk)
{
    LOGI("StorageManager::NotifyDiskCreated start, diskId: %{public}s", disk.GetDiskId().c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    diskManager->OnDiskCreated(disk);
}

void StorageManager::NotifyDiskDestroyed(std::string diskId)
{
    LOGI("StorageManager::NotifyDiskDestroyed start, diskId: %{public}s", diskId.c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    diskManager->OnDiskDestroyed(diskId);
}

int32_t StorageManager::Partition(std::string diskId, int32_t type)
{
    LOGI("StorageManager::Partition start, diskId: %{public}s", diskId.c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    int32_t err = diskManager->Partition(diskId, type);
    return err;
}

std::vector<Disk> StorageManager::GetAllDisks()
{
    LOGI("StorageManger::GetAllDisks start");
    std::vector<Disk> result = DelayedSingleton<DiskManagerService>::GetInstance()->GetAllDisks();
    return result;
}

int32_t StorageManager::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("UserId: %{public}u, flags:  %{public}u", userId, flags);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->GenerateUserKeys(userId, flags);
    return err;
}

int32_t StorageManager::DeleteUserKeys(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->DeleteUserKeys(userId);
    return err;
}

int32_t StorageManager::UpdateUserAuth(uint32_t userId, std::string auth, std::string compSecret)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->UpdateUserAuth(userId, auth, compSecret);
    return err;
}

int32_t StorageManager::ActiveUserKey(uint32_t userId, std::string auth, std::string compSecret)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->ActiveUserKey(userId, auth, compSecret);
    return err;
}

int32_t StorageManager::InactiveUserKey(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->InactiveUserKey(userId);
    return err;
}

int32_t StorageManager::UpdateKeyContext(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->UpdateKeyContext(userId);
    return err;
}
}
}