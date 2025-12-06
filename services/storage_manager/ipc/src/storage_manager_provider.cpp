/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <sys/resource.h>
#include <sys/syscall.h>

#include "utils/storage_radar.h"
#include <singleton.h>
#ifdef STORAGE_STATISTICS_MANAGER
#include <storage/storage_monitor_service.h>
#include <storage/storage_status_service.h>
#include <storage/storage_total_status_service.h>
#include <storage/volume_storage_status_service.h>
#include "storage_rdb_adapter.h"
#endif

#ifdef USER_CRYPTO_MANAGER
#include "account_subscriber/account_subscriber.h"
#include "appspawn.h"
#include "crypto/filesystem_crypto.h"
#include "utils/storage_radar.h"
#endif
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager_service.h"
#include "volume/volume_manager_service.h"
#endif
#include "ipc/storage_manager_provider.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "common_event/storage_common_event_subscriber.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "user/multi_user_manager_service.h"
#include "utils/storage_utils.h"

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "storage/bundle_manager_connector.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
REGISTER_SYSTEM_ABILITY_BY_ID(StorageManagerProvider, STORAGE_MANAGER_MANAGER_ID, true);

constexpr pid_t ACCOUNT_UID = 3058;
constexpr pid_t BACKUP_SA_UID = 1089;
constexpr pid_t FOUNDATION_UID = 5523;
constexpr pid_t DFS_UID = 1009;
constexpr pid_t AOCO_UID = 7558;
constexpr pid_t SPACE_ABILITY_SERVICE_UID = 7014;
constexpr pid_t UPDATE_SERVICE_UID = 6666;
const std::string MEDIALIBRARY_BUNDLE_NAME = "com.ohos.medialibrary.medialibrarydata";
const std::string SCENEBOARD_BUNDLE_NAME = "com.ohos.sceneboard";
const std::string SYSTEMUI_BUNDLE_NAME = "com.ohos.systemui";
const std::string PERMISSION_STORAGE_MANAGER_CRYPT = "ohos.permission.STORAGE_MANAGER_CRYPT";
const std::string PERMISSION_STORAGE_MANAGER = "ohos.permission.STORAGE_MANAGER";
const std::string PERMISSION_MOUNT_MANAGER = "ohos.permission.MOUNT_UNMOUNT_MANAGER";
const std::string PERMISSION_FORMAT_MANAGER = "ohos.permission.MOUNT_FORMAT_MANAGER";
const std::string PROCESS_NAME_FOUNDATION = "foundation";
bool CheckClientPermission(const std::string &permissionStr)
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    auto uid = IPCSkeleton::GetCallingUid();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    int res;
    if (tokenType == Security::AccessToken::TOKEN_NATIVE && uid == ACCOUNT_UID) {
        res = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    } else {
        res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller, permissionStr);
    }

    if (res == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        LOGD("StorageMangaer permissionCheck pass!");
        return true;
    }
    LOGE("StorageManager permissionCheck error, need %{public}s", permissionStr.c_str());
    return false;
}

bool IsSystemApp()
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenType == Security::AccessToken::TOKEN_HAP) {
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        return Security::AccessToken::AccessTokenKit::IsSystemAppByFullTokenID(fullTokenId);
    }
    return true;
}

bool CheckClientPermissionForCrypt(const std::string &permissionStr)
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller, permissionStr);
    if (res == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        LOGD("StorageMangaer permissionCheck pass!");
        return true;
    }
    LOGE("StorageManager permissionCheck error, need %{public}s", permissionStr.c_str());
    return false;
}

bool CheckClientPermissionForShareFile()
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenCaller, nativeInfo);

    auto uid = IPCSkeleton::GetCallingUid();
    if (nativeInfo.processName != PROCESS_NAME_FOUNDATION || uid != FOUNDATION_UID) {
        LOGE("CheckClientPermissionForShareFile error, processName is %{public}s, uid is %{public}d",
             nativeInfo.processName.c_str(), uid);
        return false;
    }

    return true;
}

int32_t StorageManagerProvider::CheckUserIdRange(int32_t userId)
{
    if (userId < StorageService::START_USER_ID || userId > StorageService::MAX_USER_ID) {
        LOGE("StorageManagerProvider: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

void StorageManagerProvider::OnStart()
{
    LOGI("StorageManager::OnStart Begin");
    bool res = SystemAbility::Publish(this);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    (void)SetPriority();
#ifdef STORAGE_STATISTICS_MANAGER
    StorageMonitorService::GetInstance().StartStorageMonitorTask();
    std::thread([]() { OHOS::StorageManager::StorageRdbAdapter::GetInstance().Init(); }).detach();
#endif
    LOGI("StorageManager::OnStart End, res = %{public}d", res);
}

void StorageManagerProvider::OnStop()
{
#ifdef STORAGE_STATISTICS_MANAGER
    OHOS::StorageManager::StorageRdbAdapter::GetInstance().UnInit();
#endif
    LOGI("StorageManager::OnStop Done");
}

void StorageManagerProvider::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility: sysId: %{public}d", systemAbilityId);
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        StorageCommonEventSubscriber::SubscribeCommonEvent();
    }
}

void StorageManagerProvider::ResetUserEventRecord(int32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    AccountSubscriber::GetInstance().ResetUserEventRecord(userId);
#endif
}

void StorageManagerProvider::SetPriority()
{
    int tid = syscall(SYS_gettid);
    if (setpriority(PRIO_PROCESS, tid, PRIORITY_LEVEL) != 0) {
        LOGE("failed to set priority");
    }
    LOGW("set storage_manager priority: %{public}d", tid);
}

int32_t StorageManagerProvider::PrepareAddUser(int32_t userId, uint32_t flags)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().PrepareAddUser(userId, flags);
}

int32_t StorageManagerProvider::RemoveUser(int32_t userId, uint32_t flags)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().RemoveUser(userId, flags);
}

int32_t StorageManagerProvider::PrepareStartUser(int32_t userId)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().PrepareStartUser(userId);
}

int32_t StorageManagerProvider::StopUser(int32_t userId)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().StopUser(userId);
}

int32_t StorageManagerProvider::CompleteAddUser(int32_t userId)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().CompleteAddUser(userId);
}

int32_t StorageManagerProvider::GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetFreeSizeOfVolume(volumeUuid, freeSize);
}

int32_t StorageManagerProvider::GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetTotalSizeOfVolume(volumeUuid, totalSize);
}

int32_t StorageManagerProvider::GetBundleStats(const std::string &pkgName,
                                               BundleStats &bundleStats,
                                               int32_t appIndex,
                                               uint32_t statFlag)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
}

int32_t StorageManagerProvider::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().ListUserdataDirInfo(scanDirs);
}

int32_t StorageManagerProvider::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    if (IsFilePathInvalid(dirPath)) {
        return E_PARAMS_INVALID;
    }
    return StorageManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, level);
}

int32_t StorageManagerProvider::GetSystemSize(int64_t &systemSize)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetSystemSize(systemSize);
}

int32_t StorageManagerProvider::GetTotalSize(int64_t &totalSize)
{
    return StorageManager::GetInstance().GetTotalSize(totalSize);
}

int32_t StorageManagerProvider::GetFreeSize(int64_t &freeSize)
{
    return StorageManager::GetInstance().GetFreeSize(freeSize);
}

int32_t StorageManagerProvider::GetUserStorageStats(StorageStats &storageStats)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetUserStorageStats(storageStats);
}

int32_t StorageManagerProvider::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetUserStorageStats(userId, storageStats);
}

int32_t StorageManagerProvider::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
    return StorageManager::GetInstance().GetCurrentBundleStats(bundleStats, statFlag);
}

int32_t StorageManagerProvider::NotifyVolumeCreated(const VolumeCore &vc)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyVolumeCreated(vc);
}

int32_t StorageManagerProvider::NotifyVolumeMounted(const VolumeInfoStr &volumeInfoStr)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyVolumeMounted(volumeInfoStr);
}

int32_t StorageManagerProvider::NotifyVolumeDamaged(const VolumeInfoStr &volumeInfoStr)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyVolumeDamaged(volumeInfoStr);
}

int32_t StorageManagerProvider::NotifyVolumeStateChanged(const std::string &volumeId, uint32_t state)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyVolumeStateChanged(volumeId, state);
}

int32_t StorageManagerProvider::Mount(const std::string &volumeId)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().Mount(volumeId);
}

int32_t StorageManagerProvider::Unmount(const std::string &volumeId)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().Unmount(volumeId);
}

int32_t StorageManagerProvider::TryToFix(const std::string &volumeId)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().TryToFix(volumeId);
}

int32_t StorageManagerProvider::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetAllVolumes(vecOfVol);
}

int32_t StorageManagerProvider::NotifyDiskCreated(const Disk &disk)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyDiskCreated(disk);
}

int32_t StorageManagerProvider::NotifyDiskDestroyed(const std::string &diskId)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyDiskDestroyed(diskId);
}

int32_t StorageManagerProvider::Partition(const std::string &diskId, int32_t type)
{
    if (!CheckClientPermission(PERMISSION_FORMAT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().Partition(diskId, type);
}

int32_t StorageManagerProvider::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetAllDisks(vecOfDisk);
}

int32_t StorageManagerProvider::GetVolumeByUuid(const std::string &fsUuid, VolumeExternal &vc)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetVolumeByUuid(fsUuid, vc);
}

int32_t StorageManagerProvider::GetVolumeById(const std::string &volumeId, VolumeExternal &vc)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetVolumeById(volumeId, vc);
}

int32_t StorageManagerProvider::SetVolumeDescription(const std::string &fsUuid, const std::string &description)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().SetVolumeDescription(fsUuid, description);
}

int32_t StorageManagerProvider::Format(const std::string &volumeId, const std::string &fsType)
{
    if (!CheckClientPermission(PERMISSION_FORMAT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().Format(volumeId, fsType);
}

int32_t StorageManagerProvider::GetDiskById(const std::string &diskId, Disk &disk)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetDiskById(diskId, disk);
}

int32_t StorageManagerProvider::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(diskPath)) {
        return E_PARAMS_INVALID;
    }
    isInUse = true;
    return StorageManager::GetInstance().QueryUsbIsInUse(diskPath, isInUse);
}

int32_t StorageManagerProvider::DeleteUserKeys(uint32_t userId)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().DeleteUserKeys(userId);
}

int32_t StorageManagerProvider::EraseAllUserEncryptedKeys()
{
    LOGI("StorageManagerProvider::EraseAllUserEncryptedKeys Begin");
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != UPDATE_SERVICE_UID) {
        LOGE("Permission check failed, the UID is not in the trustlist, uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().EraseAllUserEncryptedKeys();
}

int32_t StorageManagerProvider::UpdateUserAuth(uint32_t userId,
                                               uint64_t secureUid,
                                               const std::vector<uint8_t> &token,
                                               const std::vector<uint8_t> &oldSecret,
                                               const std::vector<uint8_t> &newSecret)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
}

int32_t StorageManagerProvider::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                             const std::vector<uint8_t> &newSecret,
                                                             uint64_t secureUid,
                                                             uint32_t userId,
                                                             const std::vector<std::vector<uint8_t>> &plainText)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid,
        userId, plainText);
}

int32_t StorageManagerProvider::ActiveUserKey(uint32_t userId,
                                              const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &secret)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().ActiveUserKey(userId, token, secret);
}

int32_t StorageManagerProvider::InactiveUserKey(uint32_t userId)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().InactiveUserKey(userId);
}

int32_t StorageManagerProvider::LockUserScreen(uint32_t userId)
{
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        StorageService::StorageRadar::ReportBundleMgrResult(
            "StorageManagerStub::HandleLockUserScreen", E_BUNDLEMGR_ERROR, userId, "pkgName=" + SCENEBOARD_BUNDLE_NAME);
        return E_BUNDLEMGR_ERROR;
    }

    if (bundleName != SCENEBOARD_BUNDLE_NAME && bundleName != SYSTEMUI_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d), should be %{public}s", bundleName.c_str(), uid,
             SCENEBOARD_BUNDLE_NAME.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().LockUserScreen(userId);
}

bool StorageManagerProvider::IsFilePathInvalid(const std::string &filePath)
{
    size_t pos = filePath.find(PATH_INVALID_FLAG1);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("Relative path is not allowed, path contain ../");
            return true;
        }
        pos = filePath.find(PATH_INVALID_FLAG1, pos + PATH_INVALID_FLAG_LEN);
    }
    pos = filePath.rfind(PATH_INVALID_FLAG2);
    if ((pos != std::string::npos) && (filePath.size() - pos == PATH_INVALID_FLAG_LEN)) {
        LOGE("Relative path is not allowed, path tail is /..");
        return true;
    }
    return false;
}
int32_t StorageManagerProvider::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    isEncrypted = true;
    return StorageManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
}

int32_t StorageManagerProvider::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    needActive = false;
    return StorageManager::GetInstance().GetUserNeedActiveStatus(userId, needActive);
}

int32_t StorageManagerProvider::UnlockUserScreen(uint32_t userId,
                                                 const std::vector<uint8_t> &token,
                                                 const std::vector<uint8_t> &secret)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UnlockUserScreen(userId, token, secret);
}

int32_t StorageManagerProvider::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    lockScreenStatus = false;
    return StorageManager::GetInstance().GetLockScreenStatus(userId, lockScreenStatus);
}

int32_t StorageManagerProvider::GenerateAppkey(uint32_t hashId, uint32_t userId, std::string &keyId, bool needReSet)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GenerateAppkey(hashId, userId, keyId, needReSet);
}

int32_t StorageManagerProvider::DeleteAppkey(const std::string &keyId)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().DeleteAppkey(keyId);
}

int32_t StorageManagerProvider::CreateRecoverKey(uint32_t userId,
                                                 uint32_t userType,
                                                 const std::vector<uint8_t> &token,
                                                 const std::vector<uint8_t> &secret)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().CreateRecoverKey(userId, userType, token, secret);
}

int32_t StorageManagerProvider::SetRecoverKey(const std::vector<uint8_t> &key)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().SetRecoverKey(key);
}

int32_t StorageManagerProvider::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UpdateKeyContext(userId, needRemoveTmpKey);
}

int32_t StorageManagerProvider::CreateShareFile(const StorageFileRawData &rawData,
                                                uint32_t tokenId,
                                                uint32_t flag,
                                                std::vector<int32_t> &funcResult)
{
    if (!CheckClientPermissionForShareFile()) {
        return E_PERMISSION_DENIED;
    }

    funcResult = StorageManager::GetInstance().CreateShareFile(rawData, tokenId, flag);
    LOGI("StorageManagerProvider::CreateShareFile end. result is %{public}zu", funcResult.size());
    return E_OK;
}

int32_t StorageManagerProvider::DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData)
{
    if (!CheckClientPermissionForShareFile()) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().DeleteShareFile(tokenId, rawData);
}

int32_t StorageManagerProvider::SetBundleQuota(const std::string &bundleName,
                                               int32_t uid,
                                               const std::string &bundleDataDirPath,
                                               int32_t limitSizeMb)
{
    if (IsFilePathInvalid(bundleDataDirPath)) {
        return E_PARAMS_INVALID;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageManagerProvider::GetUserStorageStatsByType(int32_t userId,
                                                          StorageStats &storageStats,
                                                          const std::string &type)
{
    if (IPCSkeleton::GetCallingUid() != BACKUP_SA_UID) {
        LOGE("StorageManager permissionCheck error, calling uid is invalid, need backup_sa uid.");
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().GetUserStorageStatsByType(userId, storageStats, type);
}

int32_t StorageManagerProvider::UpdateMemoryPara(int32_t size, int32_t &oldSize)
{
    if (IPCSkeleton::GetCallingUid() != BACKUP_SA_UID) {
        LOGE("StorageManager permissionCheck error, calling uid is invalid, need backup_sa uid.");
        return E_PERMISSION_DENIED;
    }
    oldSize = 0;
    return StorageManager::GetInstance().UpdateMemoryPara(size, oldSize);
}

int32_t StorageManagerProvider::MountDfsDocs(int32_t userId,
                                             const std::string &relativePath,
                                             const std::string &networkId,
                                             const std::string &deviceId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::MountDfsDocs userId %{public}d out of range", userId);
        return err;
    }

    if (IsFilePathInvalid(relativePath)) {
        return E_PARAMS_INVALID;
    }
    // Only for dfs create device dir and bind mount from DFS Docs.
    if (IPCSkeleton::GetCallingUid() != DFS_UID) {
        LOGE("HandleMountDfsDocs permissionCheck error, calling uid now is %{public}d, should be DFS_UID: %{public}d",
             IPCSkeleton::GetCallingUid(), DFS_UID);
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageManagerProvider::UMountDfsDocs(int32_t userId,
                                              const std::string &relativePath,
                                              const std::string &networkId,
                                              const std::string &deviceId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UMountDfsDocs userId %{public}d out of range", userId);
        return err;
    }

    if (IsFilePathInvalid(relativePath)) {
        return E_PARAMS_INVALID;
    }
    // Only for dfs create device dir and bind mount from DFS Docs.
    if (IPCSkeleton::GetCallingUid() != DFS_UID) {
        LOGE("HandleUMountDfsDocs permissionCheck error, calling uid now is %{public}d, should be DFS_UID: %{public}d",
             IPCSkeleton::GetCallingUid(), DFS_UID);
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageManagerProvider::NotifyMtpMounted(const std::string &id,
                                                 const std::string &path,
                                                 const std::string &desc,
                                                 const std::string &uuid)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyMtpMounted(id, path, desc, uuid);
}

int32_t StorageManagerProvider::NotifyMtpUnmounted(const std::string &id, const std::string &path, bool isBadRemove)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().NotifyMtpUnmounted(id, path, isBadRemove);
}

int32_t StorageManagerProvider::MountMediaFuse(int32_t userId, int32_t &devFd)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::MountMediaFuse userId %{public}d out of range", userId);
        return err;
    }
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageManagerProvider::MountMediaFuse start.");

    // Only for medialibrary to mount fuse.
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return E_BUNDLEMGR_ERROR;
    }
    if (bundleName != MEDIALIBRARY_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d), should be %{public}s", bundleName.c_str(), uid,
             MEDIALIBRARY_BUNDLE_NAME.c_str());
        return E_PERMISSION_DENIED;
    }
    devFd = -1;
    return StorageManager::GetInstance().MountMediaFuse(userId, devFd);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::UMountMediaFuse(int32_t userId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UMountMediaFuse userId %{public}d out of range", userId);
        return err;
    }
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageManagerStub::HandleUMountMediaFuse start.");

    // Only for medialibrary to mount fuse.
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return E_BUNDLEMGR_ERROR;
    }
    if (bundleName != MEDIALIBRARY_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d), should be %{public}s", bundleName.c_str(), uid,
             MEDIALIBRARY_BUNDLE_NAME.c_str());
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UMountMediaFuse(userId);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::MountFileMgrFuse userId %{public}d out of range", userId);
        return err;
    }

    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    fuseFd = -1;
    return StorageManager::GetInstance().MountFileMgrFuse(userId, path, fuseFd);
}

int32_t StorageManagerProvider::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UMountFileMgrFuse userId %{public}d out of range", userId);
        return err;
    }
    
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    return StorageManager::GetInstance().UMountFileMgrFuse(userId, path);
}

int32_t StorageManagerProvider::IsFileOccupied(const std::string &path,
                                               const std::vector<std::string> &inputList,
                                               std::vector<std::string> &outputList,
                                               bool &isOccupy)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    isOccupy = false;
    return StorageManager::GetInstance().IsFileOccupied(path, inputList, outputList, isOccupy);
}

int32_t StorageManagerProvider::ResetSecretWithRecoveryKey(uint32_t userId,
                                                           uint32_t rkType,
                                                           const std::vector<uint8_t> &key)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key);
}

int32_t StorageManagerProvider::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid != DFS_UID) {
        LOGE("MountDisShareFile permissionCheck error, calling uid is %{public}d", uid);
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().MountDisShareFile(userId, shareFiles);
}

int32_t StorageManagerProvider::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid != DFS_UID) {
        LOGE("UMountDisShareFile permissionCheck error, calling uid is %{public}d", uid);
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UMountDisShareFile(userId, networkId);
}

int32_t StorageManagerProvider::InactiveUserPublicDirKey(uint32_t userId)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) ||
		IPCSkeleton::GetCallingUid() != SPACE_ABILITY_SERVICE_UID) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().InactiveUserPublicDirKey(userId);
}

int32_t StorageManagerProvider::UpdateUserPublicDirPolicy(uint32_t userId)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) ||
		IPCSkeleton::GetCallingUid() != SPACE_ABILITY_SERVICE_UID) {
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UpdateUserPublicDirPolicy(userId);
}

int32_t StorageManagerProvider::RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().RegisterUeceActivationCallback(ueceCallback);
}

int32_t StorageManagerProvider::UnregisterUeceActivationCallback()
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }
    return StorageManager::GetInstance().UnregisterUeceActivationCallback();
}

int32_t StorageManagerProvider::CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != AOCO_UID) {
        LOGE("Permission check failed, the UID is not in the trustlist, uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(path)) {
        LOGE("The path: %{public}s is invalid.", path.c_str());
        return E_PARAMS_INVALID;
    }

    auto ret = MultiUserManagerService::GetInstance().CreateUserDir(path, mode, uid, gid);
    LOGW("CreateUserDir end, uid: %{public}d, ret: %{public}d", callingUid, ret);

    std::string extraData = "path=" + path + "callingUid=" + std::to_string(callingUid);
    StorageRadar::ReportUserManager("CreateUserDir", 0, ret, extraData);
    return ret;
}

int32_t StorageManagerProvider::DeleteUserDir(const std::string &path)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }

    auto callingUid = IPCSkeleton::GetCallingUid();
    LOGE("DeleteUserDir begin, path: %{public}s, uid: %{public}d", path.c_str(), callingUid);
    if (callingUid != AOCO_UID) {
        LOGE("Permission check failed, the UID is not in the trustlist, uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(path)) {
        LOGE("The path: %{public}s is invalid.", path.c_str());
        return E_PARAMS_INVALID;
    }

    auto ret = MultiUserManagerService::GetInstance().DeleteUserDir(path);
    LOGE("DeleteUserDir end, path: %{public}s, uid: %{public}d, ret: %{public}d", path.c_str(), callingUid, ret);

    std::string extraData = "path=" + path + "callingUid=" + std::to_string(callingUid);
    StorageRadar::ReportUserManager("DeleteUserDir", 0, ret, extraData);
    return ret;
}

int32_t StorageManagerProvider::NotifyUserChangedEvent(uint32_t userId, uint32_t eventType)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) && callingUid != ACCOUNT_UID) {
        LOGE("NotifyUserChangedEvent permission denied ! uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }
    StorageService::UserChangedEventType enumType = static_cast<StorageService::UserChangedEventType>(eventType);
    if (enumType != StorageService::UserChangedEventType::EVENT_USER_UNLOCKED &&
        enumType != StorageService::UserChangedEventType::EVENT_USER_SWITCHED) {
        LOGE("NotifyUserChangedEvent event type invalid ! type: %{public}u", eventType);
        return E_PARAMS_INVALID;
    }
    StorageManager::GetInstance().NotifyUserChangedEvent(userId, enumType);
    return E_OK;
}

int32_t StorageManagerProvider::SetExtBundleStats(uint32_t userId, const ExtBundleStats &stats)
{
    if (!IsSystemApp()) {
        LOGE("the caller is not sysapp");
        return E_SYS_APP_PERMISSION_DENIED;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    if (userId > TOP_USER_ID || stats.businessSize_ >= INT64_MAX || stats.businessName_.empty()) {
        LOGE("invalid params, userId: %{public}d, size: %{public}lld, name: %{public}s", userId,
            static_cast<long long>(stats.businessSize_), stats.businessName_.c_str());
        return E_PARAMS_INVALID;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("SetExtBundleStats start");
    int32_t ret = StorageStatusService::GetInstance().SetExtBundleStats(userId, stats);
    if (ret != E_OK) {
        std::string extraData = "errCode=" + std::to_string(ret);
        StorageRadar::ReportSpaceRadar("SetExtBundleStats", E_SET_EXT_BUNDLE_STATS_ERROR, extraData);
        return E_SET_EXT_BUNDLE_STATS_ERROR;
    }
    return E_OK;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetExtBundleStats(uint32_t userId, ExtBundleStats &stats)
{
    if (!IsSystemApp()) {
        LOGE("the caller is not sysapp");
        return E_SYS_APP_PERMISSION_DENIED;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    if (userId > TOP_USER_ID || stats.businessName_.empty()) {
        LOGE("invalid params, userId: %{public}d, name: %{public}s", userId, stats.businessName_.c_str());
        return E_PARAMS_INVALID;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("GetExtBundleStats start");
    int32_t ret = StorageStatusService::GetInstance().GetExtBundleStats(userId, stats);
    if (ret != E_OK) {
        std::string extraData = "errCode=" + std::to_string(ret);
        StorageRadar::ReportSpaceRadar("GetExtBundleStats", E_GET_EXT_BUNDLE_STATS_ERROR, extraData);
        return E_GET_EXT_BUNDLE_STATS_ERROR;
    }
    return E_OK;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetAllExtBundleStats(uint32_t userId, std::vector<ExtBundleStats> &statsVec)
{
    if (!IsSystemApp()) {
        LOGE("the caller is not sysapp");
        return E_SYS_APP_PERMISSION_DENIED;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    if (userId > TOP_USER_ID) {
        LOGE("invalid params, userId: %{public}d.", userId);
        return E_PARAMS_INVALID;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("GetAllExtBundleStats start");
    int32_t ret = StorageStatusService::GetInstance().GetAllExtBundleStats(userId, statsVec);
    if (ret != E_OK) {
        std::string extraData = "errCode=" + std::to_string(ret);
        StorageRadar::ReportSpaceRadar("GetAllExtBundleStats", E_GET_ALL_EXT_BUNDLE_STATS_ERROR, extraData);
        return E_GET_ALL_EXT_BUNDLE_STATS_ERROR;
    }
    return E_OK;
#else
    return E_OK;
#endif
}

} // namespace StorageManager
} // namespace OHOS
