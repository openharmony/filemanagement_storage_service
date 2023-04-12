/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <singleton.h>
#include <storage/storage_status_service.h>
#include <storage/storage_total_status_service.h>
#include <storage/volume_storage_status_service.h>

#include "account_subscriber/account_subscriber.h"
#include "crypto/filesystem_crypto.h"
#include "disk/disk_manager_service.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "user/multi_user_manager_service.h"
#include "volume/volume_manager_service.h"

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

int32_t StorageManager::GetFreeSizeOfVolume(std::string volumeUuid, int64_t &freeSize)
{
    LOGI("StorageManger::getFreeSizeOfVolume start, volumeUuid: %{public}s", volumeUuid.c_str());
    std::shared_ptr<VolumeStorageStatusService> volumeStatsManager =
        DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    int32_t err = volumeStatsManager->GetFreeSizeOfVolume(volumeUuid, freeSize);
    return err;
}

int32_t StorageManager::GetTotalSizeOfVolume(std::string volumeUuid, int64_t &totalSize)
{
    LOGI("StorageManger::getTotalSizeOfVolume start, volumeUuid: %{public}s", volumeUuid.c_str());
    std::shared_ptr<VolumeStorageStatusService> volumeStatsManager =
        DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    int32_t err = volumeStatsManager->GetTotalSizeOfVolume(volumeUuid, totalSize);
    return err;
}

int32_t StorageManager::GetBundleStats(std::string pkgName, BundleStats &bundleStats)
{
    LOGI("StorageManger::getBundleStats start, pkgName: %{public}s", pkgName.c_str());
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetBundleStats(pkgName, bundleStats);
    return err;
}

int32_t StorageManager::GetSystemSize(int64_t &systemSize)
{
    LOGI("StorageManger::getSystemSize start");
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetSystemSize(systemSize);
    return err;
}

int32_t StorageManager::GetTotalSize(int64_t &totalSize)
{
    LOGI("StorageManger::getTotalSize start");
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize(totalSize);
    return err;
}

int32_t StorageManager::GetFreeSize(int64_t &freeSize)
{
    LOGI("StorageManger::getFreeSize start");
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetFreeSize(freeSize);
    return err;
}

int32_t StorageManager::GetUserStorageStats(StorageStats &storageStats)
{
    LOGI("StorageManger::GetUserStorageStats start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStats(storageStats);
    return err;
}

int32_t StorageManager::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    LOGI("StorageManger::GetUserStorageStats start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStats(userId, storageStats);
    return err;
}

int32_t StorageManager::GetCurrentBundleStats(BundleStats &bundleStats)
{
    LOGI("StorageManger::GetCurrentBundleStats start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetCurrentBundleStats(bundleStats);
    return err;
}

int32_t StorageManager::NotifyVolumeCreated(VolumeCore vc)
{
    LOGI("StorageManger::NotifyVolumeCreated start, volumeId: %{public}s", vc.GetId().c_str());
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeCreated(vc);
    return E_OK;
}

int32_t StorageManager::NotifyVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
    std::string path, std::string description)
{
    LOGI("StorageManger::NotifyVolumeMounted start");
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
    return E_OK;
}

int32_t StorageManager::NotifyVolumeDestroyed(std::string volumeId)
{
    LOGI("StorageManger::NotifyVolumeDestroyed start");
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeDestroyed(volumeId);
    return E_OK;
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

int32_t StorageManager::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
    LOGI("StorageManger::GetAllVolumes start");
    vecOfVol = DelayedSingleton<VolumeManagerService>::GetInstance()->GetAllVolumes();
    return E_OK;
}

int32_t StorageManager::NotifyDiskCreated(Disk disk)
{
    LOGI("StorageManager::NotifyDiskCreated start, diskId: %{public}s", disk.GetDiskId().c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    diskManager->OnDiskCreated(disk);
    return E_OK;
}

int32_t StorageManager::NotifyDiskDestroyed(std::string diskId)
{
    LOGI("StorageManager::NotifyDiskDestroyed start, diskId: %{public}s", diskId.c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    diskManager->OnDiskDestroyed(diskId);
    return E_OK;
}

int32_t StorageManager::Partition(std::string diskId, int32_t type)
{
    LOGI("StorageManager::Partition start, diskId: %{public}s", diskId.c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    int32_t err = diskManager->Partition(diskId, type);
    return err;
}

int32_t StorageManager::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
    LOGI("StorageManger::GetAllDisks start");
    vecOfDisk = DelayedSingleton<DiskManagerService>::GetInstance()->GetAllDisks();
    return E_OK;
}

int32_t StorageManager::GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc)
{
    LOGI("StorageManger::GetVolumeByUuid start, uuid: %{public}s", fsUuid.c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->GetVolumeByUuid(fsUuid, vc);
    return err;
}

int32_t StorageManager::GetVolumeById(std::string volumeId, VolumeExternal &vc)
{
    LOGI("StorageManger::GetVolumeById start, volId: %{public}s", volumeId.c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->GetVolumeById(volumeId, vc);
    return err;
}

int32_t StorageManager::SetVolumeDescription(std::string fsUuid, std::string description)
{
    LOGI("StorageManger::SetVolumeDescription start, uuid: %{public}s", fsUuid.c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->SetVolumeDescription(fsUuid, description);
    return err;
}

int32_t StorageManager::Format(std::string volumeId, std::string fsType)
{
    LOGI("StorageManger::Format start, volumeId: %{public}s, fsType: %{public}s", volumeId.c_str(), fsType.c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->Format(volumeId, fsType);
    return err;
}

int32_t StorageManager::GetDiskById(std::string diskId, Disk &disk)
{
    LOGI("StorageManger::GetDiskById start, diskId: %{public}s", diskId.c_str());
    int32_t err = DelayedSingleton<DiskManagerService>::GetInstance()->GetDiskById(diskId, disk);
    return err;
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

int32_t StorageManager::UpdateUserAuth(uint32_t userId,
                                       const std::vector<uint8_t> &token,
                                       const std::vector<uint8_t> &oldSecret,
                                       const std::vector<uint8_t> &newSecret)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->UpdateUserAuth(userId, token, oldSecret, newSecret);
    return err;
}

int32_t StorageManager::ActiveUserKey(uint32_t userId,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &secret)
{
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->ActiveUserKey(userId, token, secret);
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

int32_t StorageManager::CreateShareFile(std::string uri, uint32_t tokenId, uint32_t flag)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->CreateShareFile(uri, tokenId, flag);
}

int32_t StorageManager::DeleteShareFile(uint32_t tokenId, std::vector<std::string> sharePathList)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->DeleteShareFile(tokenId, sharePathList);
}
}
}
