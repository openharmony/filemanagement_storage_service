/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <sys/syscall.h>
#include <sys/resource.h>

#include <singleton.h>
#include "utils/storage_radar.h"
#ifdef STORAGE_STATISTICS_MANAGER
#include <storage/storage_monitor_service.h>
#include <storage/storage_status_manager.h>
#include <storage/storage_total_status_service.h>
#include <storage/volume_storage_status_service.h>
#include "account_subscriber/account_subscriber.h"
#endif

#ifdef USER_CRYPTO_MANAGER
#include "crypto/filesystem_crypto.h"
#include "appspawn.h"
#include "utils/storage_radar.h"
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

constexpr bool DECRYPTED = false;
constexpr bool ENCRYPTED = true;

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {

void StorageManager::ResetUserEventRecord(int32_t userId)
{
#ifdef STORAGE_STATISTICS_MANAGER
    AccountSubscriber::GetInstance().ResetUserEventRecord(userId);
#endif
}

int32_t StorageManager::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManager::PrepareAddUser start, userId: %{public}d", userId);
    return MultiUserManagerService::GetInstance().PrepareAddUser(userId, flags);
}

int32_t StorageManager::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManger::RemoveUser start, userId: %{public}d", userId);
    return MultiUserManagerService::GetInstance().RemoveUser(userId, flags);
}

int32_t StorageManager::PrepareStartUser(int32_t userId)
{
    LOGI("StorageManger::PrepareStartUser start, userId: %{public}d", userId);
    return MultiUserManagerService::GetInstance().PrepareStartUser(userId);
}

int32_t StorageManager::StopUser(int32_t userId)
{
    LOGI("StorageManger::StopUser start, userId: %{public}d", userId);
    int32_t err = MultiUserManagerService::GetInstance().StopUser(userId);
    if (err != E_USERID_RANGE) {
        ResetUserEventRecord(userId);
    }
    return err;
}

int32_t StorageManager::CompleteAddUser(int32_t userId)
{
    LOGI("StorageManger::CompleteAddUser start, userId: %{public}d", userId);
    return MultiUserManagerService::GetInstance().CompleteAddUser(userId);
}

int32_t StorageManager::GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getFreeSizeOfVolume start, volumeUuid: %{public}s",
        GetAnonyString(volumeUuid).c_str());
    VolumeStorageStatusService& volumeStatsManager =
        VolumeStorageStatusService::GetInstance();
    int32_t err = volumeStatsManager.GetFreeSizeOfVolume(volumeUuid, freeSize);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("VolumeStorageStatusService::GetFreeSizeOfVolume", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::getTotalSizeOfVolume start, volumeUuid: %{public}s",
        GetAnonyString(volumeUuid).c_str());
    VolumeStorageStatusService& volumeStatsManager =
        VolumeStorageStatusService::GetInstance();
    int32_t err = volumeStatsManager.GetTotalSizeOfVolume(volumeUuid, totalSize);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("VolumeStorageStatusService::GetTotalSizeOfVolume", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetBundleStats(const std::string &pkgName, BundleStats &bundleStats,
                                       int32_t appIndex, uint32_t statFlag)
{
#ifdef STORAGE_STATISTICS_MANAGER
    int32_t err = StorageStatusManager::GetInstance().GetBundleStats(pkgName, bundleStats,
        appIndex, statFlag);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetBundleStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::GetSystemSize(int64_t &systemSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManger::getSystemSize start");
    int32_t err = StorageTotalStatusService::GetInstance().GetSystemSize(systemSize);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageTotalStatusService::GetSystemSize", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::GetTotalSize(int64_t &totalSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManger::getTotalSize start");
    int32_t err = StorageTotalStatusService::GetInstance().GetTotalSize(totalSize);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageTotalStatusService::GetTotalSize", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::GetFreeSize(int64_t &freeSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManger::getFreeSize start");
    int32_t err = StorageTotalStatusService::GetInstance().GetFreeSize(freeSize);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageTotalStatusService::GetFreeSize", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::GetUserStorageStats(StorageStats &storageStats)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManger::GetUserStorageStats start");
    int32_t err = StorageStatusManager::GetInstance().GetUserStorageStats(storageStats);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetUserStorageStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManger::GetUserStorageStats start");
    int32_t err = StorageStatusManager::GetInstance().GetUserStorageStats(userId, storageStats);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetUserStorageStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManger::GetCurrentBundleStats start");
    int32_t err = StorageStatusManager::GetInstance().GetCurrentBundleStats(bundleStats, statFlag);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetCurrentBundleStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::NotifyVolumeCreated(const VolumeCore& vc)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyVolumeCreated start, volumeId: %{public}s", vc.GetId().c_str());
    VolumeManagerService::GetInstance().OnVolumeCreated(vc);
#endif

    return E_OK;
}

int32_t StorageManager::NotifyVolumeMounted(const VolumeInfoStr &volumeInfoStr)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyVolumeMounted start, fsType is %{public}s.", volumeInfoStr.fsTypeStr.c_str());
    VolumeManagerService::GetInstance().OnVolumeMounted(volumeInfoStr);
#endif
    return E_OK;
}

int32_t StorageManager::NotifyVolumeDamaged(const VolumeInfoStr &volumeInfoStr)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("NotifyVolumeDamaged start, fsType is %{public}s, fsU is %{public}s.",
        volumeInfoStr.fsTypeStr.c_str(), volumeInfoStr.fsUuid.c_str());
    VolumeManagerService::GetInstance().OnVolumeDamaged(volumeInfoStr);
#endif
    return E_OK;
}

OHOS::StorageManager::VolumeState StorageManager::UintToState(uint32_t state)
{
    switch (state) {
        case UNMOUNTED:
            return OHOS::StorageManager::VolumeState::UNMOUNTED;
        case CHECKING:
            return OHOS::StorageManager::VolumeState::CHECKING;
        case MOUNTED:
            return OHOS::StorageManager::VolumeState::MOUNTED;
        case EJECTING:
            return OHOS::StorageManager::VolumeState::EJECTING;
        case REMOVED:
            return OHOS::StorageManager::VolumeState::REMOVED;
        case BAD_REMOVAL:
            return OHOS::StorageManager::VolumeState::BAD_REMOVAL;
        case FUSE_REMOVED:
            return OHOS::StorageManager::VolumeState::FUSE_REMOVED;
        case DAMAGED_MOUNTED:
            return OHOS::StorageManager::VolumeState::DAMAGED_MOUNTED;
        case DAMAGED:
            return OHOS::StorageManager::VolumeState::DAMAGED;
        default:
            return OHOS::StorageManager::VolumeState::UNMOUNTED;
    }
}

int32_t StorageManager::NotifyVolumeStateChanged(const std::string& volumeId, uint32_t state)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyVolumeStateChanged start");
    OHOS::StorageManager::VolumeState stateService = UintToState(state);
    VolumeManagerService::GetInstance().OnVolumeStateChanged(volumeId, stateService);
#endif

    return E_OK;
}

int32_t StorageManager::Mount(const std::string &volumeId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::Mount start");
    int result = VolumeManagerService::GetInstance().Mount(volumeId);
    if (result != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Mount", result);
    }
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManager::Unmount(const std::string &volumeId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::Unmount start");
    int result = VolumeManagerService::GetInstance().Unmount(volumeId);
    if (result != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Unmount", result);
    }
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManager::TryToFix(const std::string &volumeId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::TryToFix start");
    int result = VolumeManagerService::GetInstance().TryToFix(volumeId);
    if (result != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::TryToFix", result);
    }
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetAllVolumes start");
    vecOfVol = VolumeManagerService::GetInstance().GetAllVolumes();
#endif

    return E_OK;
}

int32_t StorageManager::NotifyDiskCreated(const Disk& disk)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManager::NotifyDiskCreated start, diskId: %{public}s", disk.GetDiskId().c_str());
    DiskManagerService& diskManager = DiskManagerService::GetInstance();
    diskManager.OnDiskCreated(disk);
#endif

    return E_OK;
}

int32_t StorageManager::NotifyDiskDestroyed(const std::string &diskId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManager::NotifyDiskDestroyed start, diskId: %{public}s", diskId.c_str());
    DiskManagerService& diskManager = DiskManagerService::GetInstance();
    diskManager.OnDiskDestroyed(diskId);
#endif

    return E_OK;
}

int32_t StorageManager::Partition(const std::string &diskId, int32_t type)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManager::Partition start, diskId: %{public}s", diskId.c_str());
    DiskManagerService& diskManager = DiskManagerService::GetInstance();
    int32_t err = diskManager.Partition(diskId, type);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("DiskManagerService::Partition", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetAllDisks start");
    vecOfDisk = DiskManagerService::GetInstance().GetAllDisks();
#endif

    return E_OK;
}

int32_t StorageManager::GetVolumeByUuid(const std::string &fsUuid, VolumeExternal &vc)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetVolumeByUuid start, uuid: %{public}s",
        GetAnonyString(fsUuid).c_str());
    int32_t err = VolumeManagerService::GetInstance().GetVolumeByUuid(fsUuid, vc);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::GetVolumeByUuid", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetVolumeById(const std::string &volumeId, VolumeExternal &vc)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetVolumeById start, volId: %{public}s", volumeId.c_str());
    int32_t err = VolumeManagerService::GetInstance().GetVolumeById(volumeId, vc);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::GetVolumeById", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::SetVolumeDescription(const std::string &fsUuid, const std::string &description)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::SetVolumeDescription start, uuid: %{public}s",
        GetAnonyString(fsUuid).c_str());
    int32_t err = VolumeManagerService::GetInstance().SetVolumeDescription(fsUuid, description);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::SetVolumeDescription", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::Format(const std::string &volumeId, const std::string &fsType)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::Format start, volumeId: %{public}s, fsType: %{public}s", volumeId.c_str(), fsType.c_str());
    int32_t err = VolumeManagerService::GetInstance().Format(volumeId, fsType);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Format", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetDiskById(const std::string &diskId, Disk &disk)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::GetDiskById start, diskId: %{public}s", diskId.c_str());
    int32_t err = DiskManagerService::GetInstance().GetDiskById(diskId, disk);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("DiskManagerService::GetDiskById", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::QueryUsbIsInUse diskPath: %{public}s", diskPath.c_str());
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->QueryUsbIsInUse(diskPath, isInUse);
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManager::DeleteUserKeys(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().DeleteUserKeys(userId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::EraseAllUserEncryptedKeys()
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("StorageManager::EraseAllUserEncryptedKeys start");
    return FileSystemCrypto::GetInstance().EraseAllUserEncryptedKeys();
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
    return FileSystemCrypto::GetInstance().UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                     const std::vector<uint8_t> &newSecret,
                                                     uint64_t secureUid,
                                                     uint32_t userId,
                                                     const std::vector<std::vector<uint8_t>> &plainText)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId,
        plainText);
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
    auto err = FileSystemCrypto::GetInstance().ActiveUserKey(userId, token, secret);
    if (err == E_OK) {
        int32_t ret = -1;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ret = AppSpawnClientSendUserLockStatus(userId, DECRYPTED);
        }
        LOGE("Send DECRYPTED status: userId: %{public}d, err is %{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("AppSpawnClientSendUserLockStatus:DECRYPT", userId, ret, "EL2-EL5");
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::InactiveUserKey(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    auto err = FileSystemCrypto::GetInstance().InactiveUserKey(userId);
    int32_t ret = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ret = AppSpawnClientSendUserLockStatus(userId, ENCRYPTED);
    }
    LOGE("send encrypted status: userId: %{public}d, err is %{public}d", userId, ret);
    StorageRadar::ReportActiveUserKey("AppSpawnClientSendUserLockStatus:ENCRYPT", userId, ret, "EL2-EL5");
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManager::LockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().LockUserScreen(userId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().GetUserNeedActiveStatus(userId, needActive);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UnlockUserScreen(uint32_t userId,
                                         const std::vector<uint8_t> &token,
                                         const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().UnlockUserScreen(userId, token, secret);
#else
    return E_OK;
#endif
}

int32_t StorageManager::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().GetLockScreenStatus(userId, lockScreenStatus);
#else
    return E_OK;
#endif
}

int32_t StorageManager::GenerateAppkey(uint32_t hashId, uint32_t userId, std::string &keyId, bool needReSet)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("hashId: %{public}u", hashId);
    return FileSystemCrypto::GetInstance().GenerateAppkey(hashId, userId, keyId, needReSet);
#else
    return E_OK;
#endif
}

int32_t StorageManager::DeleteAppkey(const std::string &keyId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("DeleteAppkey enter");
    LOGI("keyId :  %{public}s", keyId.c_str());
    return FileSystemCrypto::GetInstance().DeleteAppkey(keyId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::CreateRecoverKey(uint32_t userId,
                                         uint32_t userType,
                                         const std::vector<uint8_t> &token,
                                         const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("CreateRecoverKey enter");
    LOGI("UserId :  %{public}u", userId);
    return FileSystemCrypto::GetInstance().CreateRecoverKey(userId, userType, token, secret);
#else
    return E_OK;
#endif
}

int32_t StorageManager::SetRecoverKey(const std::vector<uint8_t> &key)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("SetRecoverKey enter");
    return FileSystemCrypto::GetInstance().SetRecoverKey(key);
#else
    return E_OK;
#endif
}

int32_t StorageManager::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("ResetSecretWithRecoveryKey UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    return FileSystemCrypto::GetInstance().UpdateKeyContext(userId, needRemoveTmpKey);
#else
    return E_OK;
#endif
}

std::vector<int32_t> StorageManager::CreateShareFile(const StorageFileRawData &rawData,
    uint32_t tokenId, uint32_t flag)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->CreateShareFile(rawData, tokenId, flag);
}

int32_t StorageManager::DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->DeleteShareFile(tokenId, rawData);
}

int32_t StorageManager::SetBundleQuota(int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageManager::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->ListUserdataDirInfo(scanDirs);
}

int32_t StorageManager::GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, const std::string &type)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManger::GetUserStorageStatsByType start");
    int32_t err = StorageStatusManager::GetInstance().GetUserStorageStatsByType(userId,
        storageStats, type);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetUserStorageStatsByType", DEFAULT_USERID, err,
            "setting");
    }
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

int32_t StorageManager::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageManager::UMountDfsDocs start.");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageManager::NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                         const std::string &uuid)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyMtpMounted start, id: %{public}s, path: %{public}s, uuid: %{public}s", id.c_str(),
         path.c_str(), GetAnonyString(uuid).c_str());
    VolumeManagerService::GetInstance().NotifyMtpMounted(id, path, desc, uuid);
#endif
    return E_OK;
}

int32_t StorageManager::IsUsbFuseByType(const std::string &fsType, bool &enabled)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    enabled = VolumeManagerService::GetInstance().IsUsbFuseByType(fsType);
#endif
    return E_OK;
}

int32_t StorageManager::NotifyMtpUnmounted(const std::string &id, const std::string &path, bool isBadRemove)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManger::NotifyMtpUnmounted start, id: %{public}s, path: %{public}s", id.c_str(), path.c_str());
    VolumeManagerService::GetInstance().NotifyMtpUnmounted(id, path, isBadRemove);
#endif
    return E_OK;
}

int32_t StorageManager::MountMediaFuse(int32_t userId, int32_t &devFd)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->MountMediaFuse(userId, devFd);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UMountMediaFuse(int32_t userId)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->UMountMediaFuse(userId);
#else
    return E_OK;
#endif
}

int32_t StorageManager::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->MountFileMgrFuse(userId, path, fuseFd);
}

int32_t StorageManager::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->UMountFileMgrFuse(userId, path);
}

int32_t StorageManager::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->IsFileOccupied(path, inputList, outputList, isOccupy);
}

int32_t StorageManager::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    if (userId <= 0) {
        LOGE("mount share file, userId %{public}d is invalid.", userId);
        return E_PARAMS_INVALID;
    }
    for (const auto &item : shareFiles) {
        if (item.first.find("..") != std::string::npos || item.second.find("..") != std::string::npos) {
            LOGE("mount share file, shareFiles is invalid.");
            return E_PARAMS_INVALID;
        }
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->MountDisShareFile(userId, shareFiles);
}

int32_t StorageManager::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    if (userId <= 0) {
        LOGE("umount share file, userId %{public}d is invalid.", userId);
        return E_PARAMS_INVALID;
    }
    if (networkId.find("..") != std::string::npos) {
        LOGE("umount share file, networkId is invalid.");
        return E_PARAMS_INVALID;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->UMountDisShareFile(userId, networkId);
}

int32_t StorageManager::InactiveUserPublicDirKey(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    auto ret = FileSystemCrypto::GetInstance().InactiveUserPublicDirKey(userId);
    LOGI("inactive user public dir key, userId: %{public}d, ret: %{public}d", userId, ret);
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageManager::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("StorageManger::SetDirEncryptionPolicy start");
    return FileSystemCrypto::GetInstance().SetDirEncryptionPolicy(userId, dirPath, level);
#else
    return E_OK;
#endif
}

int32_t StorageManager::UpdateUserPublicDirPolicy(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    auto ret = FileSystemCrypto::GetInstance().UpdateUserPublicDirPolicy(userId);
    LOGI("Update policy userId: %{public}u, ret: %{public}d", userId, ret);
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageManager::RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback)
{
    return FileSystemCrypto::GetInstance().RegisterUeceActivationCallback(ueceCallback);
}

int32_t StorageManager::UnregisterUeceActivationCallback()
{
    return FileSystemCrypto::GetInstance().UnregisterUeceActivationCallback();
}

void StorageManager::NotifyUserChangedEvent(uint32_t userId, StorageService::UserChangedEventType eventType)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("NotifyUserChangedEvent UserId: %{public}u, type: %{public}d", userId, eventType);
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, eventType);
    return;
#endif
    LOGI("NotifyUserChangedEvent Not support !");
}
}
}