/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifdef STORAGE_STATISTICS_MANAGER
#include <storage/storage_status_service.h>
#include <storage/storage_total_status_service.h>
#include <storage/volume_storage_status_service.h>
#include "account_subscriber/account_subscriber.h"
#endif

#ifdef USER_CRYPTO_MANAGER
#include "crypto/filesystem_crypto.h"
#endif
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager_service.h"
#include "volume/volume_manager_service.h"
#endif
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "utils/storage_utils.h"
#include "user/multi_user_manager_service.h"

namespace OHOS {
namespace StorageManager {
REGISTER_SYSTEM_ABILITY_BY_ID(StorageManager, STORAGE_MANAGER_MANAGER_ID, true);

void StorageManager::OnStart()
{
    LOGI("StorageManager::OnStart Begin");
    bool res = SystemAbility::Publish(this);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    LOGI("StorageManager::OnStart End, res = %{public}d", res);
}

void StorageManager::OnStop()
{
    LOGI("StorageManager::Onstop Done");
}

void StorageManager::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("StorageManager::OnAddSystemAbility SA id %{public}d added", systemAbilityId);
#ifdef STORAGE_STATISTICS_MANAGER
    AccountSubscriber::Subscriber();
#endif
}

void StorageManager::ResetUserEventRecord(int32_t userId)
{
#ifdef STORAGE_STATISTICS_MANAGER
    AccountSubscriber::ResetUserEventRecord(userId);
#endif
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
    if (err != E_USERID_RANGE) {
        ResetUserEventRecord(userId);
    }
    return err;
}

int32_t StorageManager::GetFreeSizeOfVolume(std::string volumeUuid, int64_t &freeSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getFreeSizeOfVolume start, volumeUuid: %{public}s",
        GetAnonyString(volumeUuid).c_str());
    std::shared_ptr<VolumeStorageStatusService> volumeStatsManager =
        DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    int32_t err = volumeStatsManager->GetFreeSizeOfVolume(volumeUuid, freeSize);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetTotalSizeOfVolume(std::string volumeUuid, int64_t &totalSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getTotalSizeOfVolume start, volumeUuid: %{public}s",
        GetAnonyString(volumeUuid).c_str());
    std::shared_ptr<VolumeStorageStatusService> volumeStatsManager =
        DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    int32_t err = volumeStatsManager->GetTotalSizeOfVolume(volumeUuid, totalSize);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetBundleStats(std::string pkgName, BundleStats &bundleStats)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getBundleStats start, pkgName: %{public}s", pkgName.c_str());
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetBundleStats(pkgName, bundleStats);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetSystemSize(int64_t &systemSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getSystemSize start");
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetSystemSize(systemSize);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetTotalSize(int64_t &totalSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getTotalSize start");
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize(totalSize);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetFreeSize(int64_t &freeSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getFreeSize start");
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetFreeSize(freeSize);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetUserStorageStats(StorageStats &storageStats)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::GetUserStorageStats start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStats(storageStats);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::GetUserStorageStats start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStats(userId, storageStats);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetCurrentBundleStats(BundleStats &bundleStats)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::GetCurrentBundleStats start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetCurrentBundleStats(bundleStats);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::NotifyVolumeCreated(VolumeCore vc)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyVolumeCreated start, volumeId: %{public}s", vc.GetId().c_str());
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeCreated(vc);
#endif

    return E_OK;
}

int32_t StorageManager::NotifyVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
    std::string path, std::string description)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyVolumeMounted start");
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
#endif

    return E_OK;
}

int32_t StorageManager::NotifyVolumeStateChanged(std::string volumeId, VolumeState state)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyVolumeStateChanged start");
    DelayedSingleton<VolumeManagerService>::GetInstance()->OnVolumeStateChanged(volumeId, state);
#endif

    return E_OK;
}

int32_t StorageManager::Mount(std::string volumeId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::Mount start");
    int result = DelayedSingleton<VolumeManagerService>::GetInstance()->Mount(volumeId);
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManager::Unmount(std::string volumeId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::Unmount start");
    int result = DelayedSingleton<VolumeManagerService>::GetInstance()->Unmount(volumeId);
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetAllVolumes start");
    vecOfVol = DelayedSingleton<VolumeManagerService>::GetInstance()->GetAllVolumes();
#endif

    return E_OK;
}

int32_t StorageManager::NotifyDiskCreated(Disk disk)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManager::NotifyDiskCreated start, diskId: %{public}s", disk.GetDiskId().c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    diskManager->OnDiskCreated(disk);
#endif

    return E_OK;
}

int32_t StorageManager::NotifyDiskDestroyed(std::string diskId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManager::NotifyDiskDestroyed start, diskId: %{public}s", diskId.c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    diskManager->OnDiskDestroyed(diskId);
#endif

    return E_OK;
}

int32_t StorageManager::Partition(std::string diskId, int32_t type)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManager::Partition start, diskId: %{public}s", diskId.c_str());
    std::shared_ptr<DiskManagerService> diskManager = DelayedSingleton<DiskManagerService>::GetInstance();
    int32_t err = diskManager->Partition(diskId, type);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetAllDisks start");
    vecOfDisk = DelayedSingleton<DiskManagerService>::GetInstance()->GetAllDisks();
#endif

    return E_OK;
}

int32_t StorageManager::GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetVolumeByUuid start, uuid: %{public}s",
        GetAnonyString(fsUuid).c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->GetVolumeByUuid(fsUuid, vc);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetVolumeById(std::string volumeId, VolumeExternal &vc)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetVolumeById start, volId: %{public}s", volumeId.c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->GetVolumeById(volumeId, vc);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::SetVolumeDescription(std::string fsUuid, std::string description)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::SetVolumeDescription start, uuid: %{public}s",
        GetAnonyString(fsUuid).c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->SetVolumeDescription(fsUuid, description);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::Format(std::string volumeId, std::string fsType)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::Format start, volumeId: %{public}s, fsType: %{public}s", volumeId.c_str(), fsType.c_str());
    int32_t err = DelayedSingleton<VolumeManagerService>::GetInstance()->Format(volumeId, fsType);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetDiskById(std::string diskId, Disk &disk)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetDiskById start, diskId: %{public}s", diskId.c_str());
    int32_t err = DelayedSingleton<DiskManagerService>::GetInstance()->GetDiskById(diskId, disk);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u, flags:  %{public}u", userId, flags);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->GenerateUserKeys(userId, flags);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::DeleteUserKeys(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->DeleteUserKeys(userId);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                       const std::vector<uint8_t> &token,
                                       const std::vector<uint8_t> &oldSecret,
                                       const std::vector<uint8_t> &newSecret)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::ActiveUserKey(uint32_t userId,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->ActiveUserKey(userId, token, secret);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::InactiveUserKey(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->InactiveUserKey(userId);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::LockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    return fsCrypto->LockUserScreen(userId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UnlockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    return fsCrypto->UnlockUserScreen(userId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    return fsCrypto->GetLockScreenStatus(userId, lockScreenStatus);
#else
    return E_OK;
#endif
}

int32_t StorageManager::GenerateAppkey(uint32_t appUid, std::string &keyId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("appUid: %{public}u", appUid);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    return fsCrypto->GenerateAppkey(appUid, keyId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::DeleteAppkey(const std::string keyId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("DeleteAppkey enter");
    LOGI("keyId :  %{public}s", keyId.c_str());
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    return fsCrypto->DeleteAppkey(keyId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UpdateKeyContext(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    std::shared_ptr<FileSystemCrypto> fsCrypto = DelayedSingleton<FileSystemCrypto>::GetInstance();
    int32_t err = fsCrypto->UpdateKeyContext(userId);
    return err;
#else
    return E_OK;
#endif
}

std::vector<int32_t> StorageManager::CreateShareFile(const std::vector<std::string> &uriList,
                                                     uint32_t tokenId, uint32_t flag)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->CreateShareFile(uriList, tokenId, flag);
}

int32_t StorageManager::DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &uriList)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->DeleteShareFile(tokenId, uriList);
}

int32_t StorageManager::SetBundleQuota(const std::string &bundleName, int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageManager::GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
    const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::GetBundleStatsForIncrease start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetBundleStatsForIncrease(userId,
        bundleNames, incrementalBackTimes, pkgFileSizes);
    return err;
#else
    return E_OK;
#endif
}


int32_t StorageManager::GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::GetUserStorageStatsByType start");
    int32_t err = DelayedSingleton<StorageStatusService>::GetInstance()->GetUserStorageStatsByType(userId,
        storageStats, type);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::UpdateMemoryPara(int32_t size, int32_t &oldSize)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->UpdateMemoryPara(size, oldSize);
}

int32_t StorageManager::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageManager::MountDfsDocs start.");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->MountDfsDocs(userId, relativePath, networkId, deviceId);
}
}
}
