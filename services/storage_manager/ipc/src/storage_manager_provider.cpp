/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <regex>
#ifdef STORAGE_STATISTICS_MANAGER
#include "storage/storage_monitor_service.h"
#include "storage/storage_status_manager.h"
#include "storage/storage_total_status_service.h"
#include "storage/volume_storage_status_service.h"
#include "file_cache_adapter.h"
#endif

#ifdef USER_CRYPTO_MANAGER
#include "account_subscriber/account_subscriber.h"
#include "appspawn.h"
#include "utils/storage_radar.h"
#endif
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager_service.h"
#include "volume/volume_manager_service.h"
#endif
#include "ipc/storage_manager_provider.h"
#include "os_account_manager.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "common_event/storage_common_event_subscriber.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "utils/memory_reclaim_manager.h"
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
constexpr pid_t ROOT_UID = 0;
constexpr pid_t SPACE_ABILITY_SERVICE_UID = 7014;
constexpr pid_t UPDATE_SERVICE_UID = 6666;
constexpr bool DECRYPTED = false;
constexpr bool ENCRYPTED = true;
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
#ifdef STORAGE_STATISTICS_MANAGER
    StorageMonitorService::GetInstance().StartStorageMonitorTask();
    (void)SetPriority();
#endif
    LOGI("StorageManager::OnStart End, res = %{public}d", res);
}

void StorageManagerProvider::OnStop()
{
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

OHOS::StorageManager::VolumeState UintToState(uint32_t state)
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

int32_t StorageManagerProvider::PrepareAddUser(int32_t userId, uint32_t flags)
{
    StorageRadar::ReportFucBehavior("PrepareAddUser", userId, "PrepareAddUser Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerProvider::PrepareAddUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageManagerProvider::PrepareAddUser userId %{public}d out of range", userId);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareAddUser::CheckUserIdRange", userId, err, extraData);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->PrepareAddUser(userId, flags);
    StorageRadar::ReportFucBehavior("PrepareAddUser", userId, "PrepareAddUser End", err);
    return err;
}

int32_t StorageManagerProvider::RemoveUser(int32_t userId, uint32_t flags)
{
    StorageRadar::ReportFucBehavior("RemoveUser", userId, "RemoveUser Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerProvider::RemoveUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageManagerProvider::RemoveUser userId %{public}d out of range", userId);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("RemoveUser::CheckUserIdRange", userId, err, extraData);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->RemoveUser(userId, flags);
    StorageRadar::ReportFucBehavior("RemoveUser", userId, "RemoveUser End", err);
    return err;
}

int32_t StorageManagerProvider::PrepareStartUser(int32_t userId)
{
    StorageRadar::ReportFucBehavior("PrepareStartUser", userId, "PrepareStartUser Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerProvider::PrepareStartUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageManagerProvider::PrepareStartUser userId %{public}d out of range", userId);
        StorageRadar::ReportUserManager("PrepareStartUser::CheckUserIdRange", userId, err, "");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->PrepareStartUser(userId);
    StorageRadar::ReportFucBehavior("PrepareStartUser", userId, "PrepareStartUser End", err);
    return err;
}

int32_t StorageManagerProvider::StopUser(int32_t userId)
{
    StorageRadar::ReportFucBehavior("StopUser", userId, "StopUser Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerProvider::StopUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageManagerProvider::StopUser userId %{public}d out of range", userId);
        StorageRadar::ReportUserManager("StopUser::CheckUserIdRange", userId, err, "");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->StopUser(userId);
    StorageRadar::ReportFucBehavior("StopUser", userId, "StopUser End", err);
    if (err != E_USERID_RANGE) {
        ResetUserEventRecord(userId);
    }
    return err;
}

int32_t StorageManagerProvider::CompleteAddUser(int32_t userId)
{
    StorageRadar::ReportFucBehavior("CompleteAddUser", userId, "CompleteAddUser Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerProvider::CompleteAddUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageManagerProvider::CompleteAddUser userId %{public}d out of range", userId);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->CompleteAddUser(userId);
    StorageRadar::ReportFucBehavior("CompleteAddUser", userId, "CompleteAddUser End", err);
    return err;
}

int32_t StorageManagerProvider::GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManagerProvider::getFreeSizeOfVolume start, volumeUuid: %{public}s",
        GetAnonyString(volumeUuid).c_str());
    VolumeStorageStatusService& volumeStatsManager = VolumeStorageStatusService::GetInstance();
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

int32_t StorageManagerProvider::GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManagerProvider::getTotalSizeOfVolume start, volumeUuid: %{public}s",
        GetAnonyString(volumeUuid).c_str());
    VolumeStorageStatusService& volumeStatsManager = VolumeStorageStatusService::GetInstance();
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

int32_t StorageManagerProvider::GetBundleStats(const std::string &pkgName,
                                               BundleStats &bundleStats,
                                               int32_t appIndex,
                                               uint32_t statFlag)
{
    StorageRadar::ReportFucBehavior("GetBundleStats", DEFAULT_USERID, "GetBundleStats Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    int32_t err = StorageStatusManager::GetInstance().GetBundleStats(pkgName, bundleStats,
        appIndex, statFlag);
    StorageRadar::ReportFucBehavior("GetBundleStats", DEFAULT_USERID, "GetBundleStats End", err);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetBundleStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManagerProvider::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    StorageRadar::ReportFucBehavior("ListUserdataDirInfo", DEFAULT_USERID, "ListUserdataDirInfo Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->ListUserdataDirInfo(scanDirs);
    StorageRadar::ReportFucBehavior("ListUserdataDirInfo", DEFAULT_USERID, "ListUserdataDirInfo End", err);
    return err;
}

int32_t StorageManagerProvider::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    StorageRadar::ReportFucBehavior("SetDirEncryptionPolicy", userId, "SetDirEncryptionPolicy Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    if (IsFilePathInvalid(dirPath)) {
        return E_PARAMS_INVALID;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("StorageManagerProvider::SetDirEncryptionPolicy start");
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->SetDirEncryptionPolicy(userId, dirPath, level);
    StorageRadar::ReportFucBehavior("SetDirEncryptionPolicy", userId, "SetDirEncryptionPolicy End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetSystemSize(int64_t &systemSize)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManagerProvider::getSystemSize start");
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

int32_t StorageManagerProvider::GetTotalSize(int64_t &totalSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManagerProvider::getTotalSize start");
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

int32_t StorageManagerProvider::GetFreeSize(int64_t &freeSize)
{
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManagerProvider::getFreeSize start");
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

int32_t StorageManagerProvider::GetUserStorageStats(StorageStats &storageStats)
{
    StorageRadar::ReportFucBehavior("GetUserStorageStats", DEFAULT_USERID, "GetUserStorageStats Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManagerProvider::GetUserStorageStats start");
    int32_t err = StorageStatusManager::GetInstance().GetUserStorageStats(storageStats);
    StorageRadar::ReportFucBehavior("GetUserStorageStats", DEFAULT_USERID, "GetUserStorageStats End", err);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetUserStorageStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManagerProvider::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    StorageRadar::ReportFucBehavior("GetUserStorageStats", userId, "GetUserStorageStats Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGD("StorageManagerProvider::GetUserStorageStats start");
    int32_t err = StorageStatusManager::GetInstance().GetUserStorageStats(userId, storageStats);
    StorageRadar::ReportFucBehavior("GetUserStorageStats", userId, "GetUserStorageStats End", err);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetUserStorageStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
#ifdef STORAGE_STATISTICS_MANAGER
    StorageRadar::ReportFucBehavior("GetCurrentBundleStats", DEFAULT_USERID, "GetCurrentBundleStats Begin", E_OK);
    LOGD("StorageManagerProvider::GetCurrentBundleStats start");
    int32_t err = StorageStatusManager::GetInstance().GetCurrentBundleStats(bundleStats, statFlag);
    StorageRadar::ReportFucBehavior("GetCurrentBundleStats", DEFAULT_USERID, "GetCurrentBundleStats End", err);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetCurrentBundleStats", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManagerProvider::NotifyVolumeCreated(const VolumeCore &vc)
{
    StorageRadar::ReportFucBehavior("NotifyVolumeCreated", DEFAULT_USERID, "NotifyVolumeCreated Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyVolumeCreated start, volumeId: %{public}s", vc.GetId().c_str());
    VolumeManagerService::GetInstance().OnVolumeCreated(vc);
    StorageRadar::ReportFucBehavior("NotifyVolumeCreated", DEFAULT_USERID, "NotifyVolumeCreated End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::NotifyVolumeMounted(const VolumeInfoStr &volumeInfoStr)
{
    StorageRadar::ReportFucBehavior("NotifyVolumeMounted", DEFAULT_USERID, "NotifyVolumeMounted Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyVolumeMounted start, fsType is %{public}s.", volumeInfoStr.fsTypeStr.c_str());
    VolumeManagerService::GetInstance().OnVolumeMounted(volumeInfoStr);
    StorageRadar::ReportFucBehavior("NotifyVolumeMounted", DEFAULT_USERID, "NotifyVolumeMounted End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::NotifyVolumeDamaged(const VolumeInfoStr &volumeInfoStr)
{
    StorageRadar::ReportFucBehavior("NotifyVolumeDamaged", DEFAULT_USERID, "NotifyVolumeDamaged Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("NotifyVolumeDamaged start, fsType is %{public}s, fsU is %{public}s.",
        volumeInfoStr.fsTypeStr.c_str(), volumeInfoStr.fsUuid.c_str());
    VolumeManagerService::GetInstance().OnVolumeDamaged(volumeInfoStr);
    StorageRadar::ReportFucBehavior("NotifyVolumeDamaged", DEFAULT_USERID, "NotifyVolumeDamaged End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::NotifyVolumeStateChanged(const std::string &volumeId, uint32_t state)
{
    StorageRadar::ReportFucBehavior("NotifyVolumeStateChanged", DEFAULT_USERID, "NotifyVolumeStateChanged Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyVolumeStateChanged start");
    OHOS::StorageManager::VolumeState stateService = UintToState(state);
    VolumeManagerService::GetInstance().OnVolumeStateChanged(volumeId, stateService);
    StorageRadar::ReportFucBehavior("NotifyVolumeStateChanged", DEFAULT_USERID, "NotifyVolumeStateChanged End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::Mount(const std::string &volumeId)
{
    StorageRadar::ReportFucBehavior("Mount", DEFAULT_USERID, "Mount Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::Mount start");
    int result = VolumeManagerService::GetInstance().Mount(volumeId);
    StorageRadar::ReportFucBehavior("Mount", DEFAULT_USERID, "Mount End", result);
    if (result != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Mount", result);
    }
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::Unmount(const std::string &volumeId)
{
    StorageRadar::ReportFucBehavior("Unmount", DEFAULT_USERID, "Unmount Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::Unmount start");
    int result = VolumeManagerService::GetInstance().Unmount(volumeId);
    StorageRadar::ReportFucBehavior("Unmount", DEFAULT_USERID, "Unmount End", result);
    if (result != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Unmount", result);
    }
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::TryToFix(const std::string &volumeId)
{
    StorageRadar::ReportFucBehavior("TryToFix", DEFAULT_USERID, "TryToFix Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::TryToFix start");
    int result = VolumeManagerService::GetInstance().TryToFix(volumeId);
    StorageRadar::ReportFucBehavior("TryToFix", DEFAULT_USERID, "TryToFix End", result);
    if (result != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::TryToFix", result);
    }
    return result;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::GetAllVolumes start");
    vecOfVol = VolumeManagerService::GetInstance().GetAllVolumes();
#endif
    return E_OK;
}

int32_t StorageManagerProvider::NotifyDiskCreated(const Disk &disk)
{
    StorageRadar::ReportFucBehavior("NotifyDiskCreated", DEFAULT_USERID, "NotifyDiskCreated Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyDiskCreated start, diskId: %{public}s", disk.GetDiskId().c_str());
    DiskManagerService& diskManager = DiskManagerService::GetInstance();
    diskManager.OnDiskCreated(disk);
    StorageRadar::ReportFucBehavior("NotifyDiskCreated", DEFAULT_USERID, "NotifyDiskCreated End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::NotifyDiskDestroyed(const std::string &diskId)
{
    StorageRadar::ReportFucBehavior("NotifyDiskDestroyed", DEFAULT_USERID, "NotifyDiskDestroyed Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyDiskDestroyed start, diskId: %{public}s", diskId.c_str());
    DiskManagerService& diskManager = DiskManagerService::GetInstance();
    diskManager.OnDiskDestroyed(diskId);
    StorageRadar::ReportFucBehavior("NotifyDiskDestroyed", DEFAULT_USERID, "NotifyDiskDestroyed End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::Partition(const std::string &diskId, int32_t type)
{
    StorageRadar::ReportFucBehavior("Partition", DEFAULT_USERID, "Partition Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_FORMAT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::Partition start, diskId: %{public}s", diskId.c_str());
    DiskManagerService& diskManager = DiskManagerService::GetInstance();
    int32_t err = diskManager.Partition(diskId, type);
    StorageRadar::ReportFucBehavior("Partition", DEFAULT_USERID, "Partition End", err);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("DiskManagerService::Partition", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::GetAllDisks start");
    vecOfDisk = DiskManagerService::GetInstance().GetAllDisks();
#endif
    return E_OK;
}

int32_t StorageManagerProvider::GetVolumeByUuid(const std::string &fsUuid, VolumeExternal &vc)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::GetVolumeByUuid start, uuid: %{public}s",
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

int32_t StorageManagerProvider::GetVolumeById(const std::string &volumeId, VolumeExternal &vc)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::GetVolumeById start, volId: %{public}s", volumeId.c_str());
    int32_t err = VolumeManagerService::GetInstance().GetVolumeById(volumeId, vc);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::GetVolumeById", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::SetVolumeDescription(const std::string &fsUuid, const std::string &description)
{
    StorageRadar::ReportFucBehavior("SetVolumeDescription", DEFAULT_USERID, "SetVolumeDescription Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::SetVolumeDescription start, uuid: %{public}s",
        GetAnonyString(fsUuid).c_str());
    int32_t err = VolumeManagerService::GetInstance().SetVolumeDescription(fsUuid, description);
    StorageRadar::ReportFucBehavior("SetVolumeDescription", DEFAULT_USERID, "SetVolumeDescription End", err);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::SetVolumeDescription", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::Format(const std::string &volumeId, const std::string &fsType)
{
    StorageRadar::ReportFucBehavior("Format", DEFAULT_USERID, "Format Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_FORMAT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::Format start, volumeId: %{public}s, fsType: %{public}s", volumeId.c_str(),
        fsType.c_str());
    int32_t err = VolumeManagerService::GetInstance().Format(volumeId, fsType);
    StorageRadar::ReportFucBehavior("Format", DEFAULT_USERID, "Format End", err);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Format", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetDiskById(const std::string &diskId, Disk &disk)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::GetDiskById start, diskId: %{public}s", diskId.c_str());
    int32_t err = DiskManagerService::GetInstance().GetDiskById(diskId, disk);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("DiskManagerService::GetDiskById", err);
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
    StorageRadar::ReportFucBehavior("QueryUsbIsInUse", DEFAULT_USERID, "QueryUsbIsInUse Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(diskPath)) {
        return E_PARAMS_INVALID;
    }
    isInUse = true;
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::QueryUsbIsInUse diskPath: %{public}s", diskPath.c_str());
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->QueryUsbIsInUse(diskPath, isInUse);
    StorageRadar::ReportFucBehavior("QueryUsbIsInUse", DEFAULT_USERID, "QueryUsbIsInUse End", err);
    return err;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageManagerProvider::EraseAllUserEncryptedKeys()
{
    StorageRadar::ReportFucBehavior("EraseAllUserEncryptedKeys", DEFAULT_USERID, "EraseAllUserEncryptedKeys Begin",
                                    E_OK);
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
#ifdef USER_CRYPTO_MANAGER
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->EraseAllUserEncryptedKeys();
    StorageRadar::ReportFucBehavior("EraseAllUserEncryptedKeys", DEFAULT_USERID, "EraseAllUserEncryptedKeys End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::UpdateUserAuth(uint32_t userId,
                                               uint64_t secureUid,
                                               const std::vector<uint8_t> &token,
                                               const std::vector<uint8_t> &oldSecret,
                                               const std::vector<uint8_t> &newSecret)
{
    StorageRadar::ReportFucBehavior("UpdateUserAuth", userId, "UpdateUserAuth Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    StorageRadar::ReportFucBehavior("UpdateUserAuth", userId, "UpdateUserAuth End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                             const std::vector<uint8_t> &newSecret,
                                                             uint64_t secureUid,
                                                             uint32_t userId,
                                                             const std::vector<std::vector<uint8_t>> &plainText)
{
    StorageRadar::ReportFucBehavior("UpdateUseAuthWithRecoveryKey", userId, "UpdateUseAuthWithRecoveryKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    StorageRadar::ReportFucBehavior("UpdateUseAuthWithRecoveryKey", userId, "UpdateUseAuthWithRecoveryKey End", err);
    return err;
    #else
        return E_OK;
#endif
}

int32_t StorageManagerProvider::ActiveUserKey(uint32_t userId,
                                              const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &secret)
{
    StorageRadar::ReportFucBehavior("ActiveUserKey", userId, "ActiveUserKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->ActiveUserKey(userId, token, secret);
    StorageRadar::ReportFucBehavior("ActiveUserKey", userId, "ActiveUserKey End", err);
    if (err == E_OK) {
        int32_t ret = -1;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ret = AppSpawnClientSendUserLockStatus(userId, DECRYPTED);
        }
        LOGI("Send DECRYPTED status: userId: %{public}d, err is %{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("AppSpawnClientSendUserLockStatus:DECRYPT", userId, ret, "EL2-EL5");
    }
    StorageDaemon::MemoryReclaimManager::ScheduleReclaimCurrentProcess(StorageDaemon::ACTIVE_USER_KEY_DELAY_SECOND);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::InactiveUserKey(uint32_t userId)
{
    StorageRadar::ReportFucBehavior("InactiveUserKey", userId, "InactiveUserKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->InactiveUserKey(userId);
    StorageRadar::ReportFucBehavior("InactiveUserKey", userId, "InactiveUserKey End", err);
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

int32_t StorageManagerProvider::LockUserScreen(uint32_t userId)
{
    StorageRadar::ReportFucBehavior("LockUserScreen", userId, "LockUserScreen Begin", E_OK);
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
#ifdef USER_CRYPTO_MANAGER
    StorageDaemon::MemoryReclaimManager::ScheduleReclaimCurrentProcess(StorageDaemon::LOCK_USER_SCREEN_DELAY_SECOND);
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->LockUserScreen(userId);
    StorageRadar::ReportFucBehavior("LockUserScreen", userId, "LockUserScreen End", err);
    return err;
#else
    return E_OK;
#endif
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
    StorageRadar::ReportFucBehavior("GetFileEncryptStatus", userId, "GetFileEncryptStatus Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    isEncrypted = true;
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    StorageRadar::ReportFucBehavior("GetFileEncryptStatus", userId, "GetFileEncryptStatus End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    StorageRadar::ReportFucBehavior("GetUserNeedActiveStatus", userId, "GetUserNeedActiveStatus Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    needActive = false;
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GetUserNeedActiveStatus(userId, needActive);
    StorageRadar::ReportFucBehavior("GetUserNeedActiveStatus", userId, "GetUserNeedActiveStatus End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::UnlockUserScreen(uint32_t userId,
                                                 const std::vector<uint8_t> &token,
                                                 const std::vector<uint8_t> &secret)
{
    StorageRadar::ReportFucBehavior("UnlockUserScreen", userId, "UnlockUserScreen Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UnlockUserScreen(userId, token, secret);
    StorageRadar::ReportFucBehavior("UnlockUserScreen", userId, "UnlockUserScreen End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    StorageRadar::ReportFucBehavior("GetLockScreenStatus", userId, "GetLockScreenStatus Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    lockScreenStatus = false;
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GetLockScreenStatus(userId, lockScreenStatus);
    StorageRadar::ReportFucBehavior("GetLockScreenStatus", userId, "GetLockScreenStatus End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::GenerateAppkey(uint32_t hashId, uint32_t userId, std::string &keyId, bool needReSet)
{
    StorageRadar::ReportFucBehavior("GenerateAppkey", userId, "GenerateAppkey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("hashId: %{public}u", hashId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GenerateAppkey(userId, hashId, keyId, needReSet);
    StorageRadar::ReportFucBehavior("GenerateAppkey", userId, "GenerateAppkey End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::DeleteAppkey(const std::string &keyId)
{
    StorageRadar::ReportFucBehavior("DeleteAppkey", DEFAULT_USERID, "DeleteAppkey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("keyId :  %{public}s", keyId.c_str());
    std::vector<int32_t> ids;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("Query active userid failed, ret = %{public}u", ret);
        StorageRadar::ReportOsAccountResult("DeleteAppkey::QueryActiveOsAccountIds", ret, DEFAULT_USERID);
        return ret;
    }
    int32_t userId = ids[0];
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->DeleteAppkey(userId, keyId);
    StorageRadar::ReportFucBehavior("DeleteAppkey", DEFAULT_USERID, "DeleteAppkey End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::CreateRecoverKey(uint32_t userId,
                                                 uint32_t userType,
                                                 const std::vector<uint8_t> &token,
                                                 const std::vector<uint8_t> &secret)
{
    StorageRadar::ReportFucBehavior("CreateRecoverKey", userId, "CreateRecoverKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    LOGI("UserId :  %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->CreateRecoverKey(userId, userType, token, secret);
    StorageRadar::ReportFucBehavior("CreateRecoverKey", userId, "CreateRecoverKey End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::SetRecoverKey(const std::vector<uint8_t> &key)
{
    StorageRadar::ReportFucBehavior("SetRecoverKey", DEFAULT_USERID, "SetRecoverKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    LOGI("SetRecoverKey enter");
    std::vector<int32_t> ids;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("Query active userid failed, ret = %{public}u", ret);
        StorageRadar::ReportOsAccountResult("SetRecoverKey::QueryActiveOsAccountIds", ret, DEFAULT_USERID);
        return ret;
    }
    int32_t userId = ids[0];
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->SetRecoverKey(key);
    StorageRadar::ReportFucBehavior("SetRecoverKey", DEFAULT_USERID, "SetRecoverKey End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    StorageRadar::ReportFucBehavior("UpdateKeyContext", userId, "UpdateKeyContext Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#ifdef USER_CRYPTO_MANAGER
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateKeyContext(userId, needRemoveTmpKey);
    StorageRadar::ReportFucBehavior("UpdateKeyContext", userId, "UpdateKeyContext End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::CreateShareFile(const StorageFileRawData &rawData,
                                                uint32_t tokenId,
                                                uint32_t flag,
                                                std::vector<int32_t> &funcResult)
{
    StorageRadar::ReportFucBehavior("CreateShareFile", DEFAULT_USERID, "CreateShareFile Begin", E_OK);
    if (!CheckClientPermissionForShareFile()) {
        return E_PERMISSION_DENIED;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    funcResult = sdCommunication->CreateShareFile(rawData, tokenId, flag);
    StorageRadar::ReportFucBehavior("CreateShareFile", DEFAULT_USERID, "CreateShareFile End", E_OK);
    return E_OK;
}

int32_t StorageManagerProvider::DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData)
{
    StorageRadar::ReportFucBehavior("DeleteShareFile", DEFAULT_USERID, "DeleteShareFile Begin", E_OK);
    if (!CheckClientPermissionForShareFile()) {
        return E_PERMISSION_DENIED;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->DeleteShareFile(tokenId, rawData);
    StorageRadar::ReportFucBehavior("DeleteShareFile", DEFAULT_USERID, "DeleteShareFile End", err);
    return err;
}

int32_t StorageManagerProvider::SetBundleQuota(const std::string &bundleName,
                                               int32_t uid,
                                               const std::string &bundleDataDirPath,
                                               int32_t limitSizeMb)
{
    StorageRadar::ReportFucBehavior("SetBundleQuota", DEFAULT_USERID, "SetBundleQuota Begin", E_OK);
    if (IsFilePathInvalid(bundleDataDirPath)) {
        return E_PARAMS_INVALID;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    StorageRadar::ReportFucBehavior("SetBundleQuota", DEFAULT_USERID, "SetBundleQuota End", err);
    return err;
}

int32_t StorageManagerProvider::GetUserStorageStatsByType(int32_t userId,
                                                          StorageStats &storageStats,
                                                          const std::string &type)
{
    StorageRadar::ReportFucBehavior("GetUserStorageStatsByType", userId, "GetUserStorageStatsByType Begin", E_OK);
    if (IPCSkeleton::GetCallingUid() != BACKUP_SA_UID) {
        LOGE("StorageManager permissionCheck error, calling uid is invalid, need backup_sa uid.");
        return E_PERMISSION_DENIED;
    }
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("StorageManagerProvider::GetUserStorageStatsByType start");
    int32_t err = StorageStatusManager::GetInstance().GetUserStorageStatsByType(userId,
        storageStats, type);
    StorageRadar::ReportFucBehavior("GetUserStorageStatsByType", userId, "GetUserStorageStatsByType End", err);
    if (err != E_OK) {
        StorageRadar::ReportGetStorageStatus("StorageStatusManager::GetUserStorageStatsByType", DEFAULT_USERID, err,
            "setting");
    }
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::MountDfsDocs(int32_t userId,
                                             const std::string &relativePath,
                                             const std::string &networkId,
                                             const std::string &deviceId)
{
    StorageRadar::ReportFucBehavior("MountDfsDocs", userId, "MountDfsDocs Begin", E_OK);
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
    LOGI("StorageManagerProvider::MountDfsDocs start.");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->MountDfsDocs(userId, relativePath, networkId, deviceId);
    StorageRadar::ReportFucBehavior("MountDfsDocs", userId, "MountDfsDocs End", err);
    return err;
}

int32_t StorageManagerProvider::UMountDfsDocs(int32_t userId,
                                              const std::string &relativePath,
                                              const std::string &networkId,
                                              const std::string &deviceId)
{
    StorageRadar::ReportFucBehavior("UMountDfsDocs", userId, "UMountDfsDocs Begin", E_OK);
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
    LOGI("StorageManagerProvider::UMountDfsDocs start.");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    StorageRadar::ReportFucBehavior("UMountDfsDocs", userId, "UMountDfsDocs End", err);
    return err;
}

int32_t StorageManagerProvider::NotifyMtpMounted(const std::string &id,
                                                 const std::string &path,
                                                 const std::string &desc,
                                                 const std::string &uuid)
{
    StorageRadar::ReportFucBehavior("NotifyMtpMounted", DEFAULT_USERID, "NotifyMtpMounted Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyMtpMounted start, id: %{public}s, path: %{public}s, uuid: %{public}s",
        id.c_str(), path.c_str(), GetAnonyString(uuid).c_str());
    VolumeManagerService::GetInstance().NotifyMtpMounted(id, path, desc, uuid);
    StorageRadar::ReportFucBehavior("NotifyMtpMounted", DEFAULT_USERID, "NotifyMtpMounted End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::IsUsbFuseByType(const std::string &fsType, bool &enabled)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    enabled = VolumeManagerService::GetInstance().IsUsbFuseByType(fsType);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::NotifyMtpUnmounted(const std::string &id, bool isBadRemove)
{
    StorageRadar::ReportFucBehavior("NotifyMtpUnmounted", DEFAULT_USERID, "NotifyMtpUnmounted Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("StorageManagerProvider::NotifyMtpUnmounted start, id: %{public}s", id.c_str());
    VolumeManagerService::GetInstance().NotifyMtpUnmounted(id, isBadRemove);
    StorageRadar::ReportFucBehavior("NotifyMtpUnmounted", DEFAULT_USERID, "NotifyMtpUnmounted End", E_OK);
#endif
    return E_OK;
}

int32_t StorageManagerProvider::MountMediaFuse(int32_t userId, int32_t &devFd)
{
    StorageRadar::ReportFucBehavior("MountMediaFuse", userId, "MountMediaFuse Begin", E_OK);
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
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->MountMediaFuse(userId, devFd);
    StorageRadar::ReportFucBehavior("MountMediaFuse", userId, "MountMediaFuse End", err);
    return err;
#endif
    return E_OK;
}

int32_t StorageManagerProvider::UMountMediaFuse(int32_t userId)
{
    StorageRadar::ReportFucBehavior("UMountMediaFuse", userId, "UMountMediaFuse Begin", E_OK);
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
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UMountMediaFuse(userId);
    StorageRadar::ReportFucBehavior("UMountMediaFuse", userId, "UMountMediaFuse End", err);
    return err;
#endif
    return E_OK;
}

int32_t StorageManagerProvider::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    StorageRadar::ReportFucBehavior("MountFileMgrFuse", userId, "MountFileMgrFuse Begin", E_OK);
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
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->MountFileMgrFuse(userId, path, fuseFd);
    StorageRadar::ReportFucBehavior("MountFileMgrFuse", userId, "MountFileMgrFuse End", err);
    return err;
}

int32_t StorageManagerProvider::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    StorageRadar::ReportFucBehavior("UMountFileMgrFuse", userId, "UMountFileMgrFuse Begin", E_OK);
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
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UMountFileMgrFuse(userId, path);
    StorageRadar::ReportFucBehavior("UMountFileMgrFuse", userId, "UMountFileMgrFuse End", err);
    return err;
}

int32_t StorageManagerProvider::IsFileOccupied(const std::string &path,
                                               const std::vector<std::string> &inputList,
                                               std::vector<std::string> &outputList,
                                               bool &isOccupy)
{
    StorageRadar::ReportFucBehavior("IsFileOccupied", DEFAULT_USERID, "IsFileOccupied Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    isOccupy = false;
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->IsFileOccupied(path, inputList, outputList, isOccupy);
    StorageRadar::ReportFucBehavior("IsFileOccupied", DEFAULT_USERID, "IsFileOccupied End", err);
    return err;
}

int32_t StorageManagerProvider::ResetSecretWithRecoveryKey(uint32_t userId,
                                                           uint32_t rkType,
                                                           const std::vector<uint8_t> &key)
{
    StorageRadar::ReportFucBehavior("ResetSecretWithRecoveryKey", userId, "ResetSecretWithRecoveryKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    LOGI("ResetSecretWithRecoveryKey UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->ResetSecretWithRecoveryKey(userId, rkType, key);
    StorageRadar::ReportFucBehavior("ResetSecretWithRecoveryKey", userId, "ResetSecretWithRecoveryKey End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    StorageRadar::ReportFucBehavior("MountDisShareFile", userId, "MountDisShareFile Begin", E_OK);
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid != DFS_UID) {
        LOGE("MountDisShareFile permissionCheck error, calling uid is %{public}d", uid);
        return E_PERMISSION_DENIED;
    }
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
    int32_t err = sdCommunication->MountDisShareFile(userId, shareFiles);
    StorageRadar::ReportFucBehavior("MountDisShareFile", userId, "MountDisShareFile End", err);
    return err;
}

int32_t StorageManagerProvider::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    StorageRadar::ReportFucBehavior("UMountDisShareFile", userId, "UMountDisShareFile Begin", E_OK);
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid != DFS_UID) {
        LOGE("UMountDisShareFile permissionCheck error, calling uid is %{public}d", uid);
        return E_PERMISSION_DENIED;
    }
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
    int32_t err = sdCommunication->UMountDisShareFile(userId, networkId);
    StorageRadar::ReportFucBehavior("UMountDisShareFile", userId, "UMountDisShareFile End", err);
    return err;
}

int32_t StorageManagerProvider::InactiveUserPublicDirKey(uint32_t userId)
{
    StorageRadar::ReportFucBehavior("InactiveUserPublicDirKey", userId, "InactiveUserPublicDirKey Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) ||
		IPCSkeleton::GetCallingUid() != SPACE_ABILITY_SERVICE_UID) {
        return E_PERMISSION_DENIED;
    }
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->InactiveUserPublicDirKey(userId);
    LOGI("inactive user public dir key, userId: %{public}d, err: %{public}d", userId, err);
    StorageRadar::ReportFucBehavior("InactiveUserPublicDirKey", userId, "InactiveUserPublicDirKey End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::UpdateUserPublicDirPolicy(uint32_t userId)
{
    StorageRadar::ReportFucBehavior("UpdateUserPublicDirPolicy", userId, "UpdateUserPublicDirPolicy Begin", E_OK);
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) ||
		IPCSkeleton::GetCallingUid() != SPACE_ABILITY_SERVICE_UID) {
        return E_PERMISSION_DENIED;
    }
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateUserPublicDirPolicy(userId);
    LOGI("Update policy userId: %{public}u, err: %{public}d", userId, err);
    StorageRadar::ReportFucBehavior("UpdateUserPublicDirPolicy", userId, "UpdateUserPublicDirPolicy End", err);
    return err;
#else
    return E_OK;
#endif
}

int32_t StorageManagerProvider::RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback)
{
    StorageRadar::ReportFucBehavior("RegisterUeceActivationCallback", DEFAULT_USERID,
                                    "RegisterUeceActivationCallback Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }
    LOGI("Enter RegisterUeceActivationCallback");
    if (ueceCallback == nullptr) {
        LOGE("callback is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->RegisterUeceActivationCallback(ueceCallback);
    StorageRadar::ReportFucBehavior("RegisterUeceActivationCallback", DEFAULT_USERID,
                                    "RegisterUeceActivationCallback End", err);
    return err;
}

int32_t StorageManagerProvider::UnregisterUeceActivationCallback()
{
    StorageRadar::ReportFucBehavior("UnregisterUeceActivationCallback", DEFAULT_USERID,
                                    "UnregisterUeceActivationCallback Begin", E_OK);
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        LOGE("Permission check failed, for storage_manager_crypt");
        return E_PERMISSION_DENIED;
    }
    LOGI("Enter UnregisterUeceActivationCallback");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->UnregisterUeceActivationCallback();
    StorageRadar::ReportFucBehavior("UnregisterUeceActivationCallback", DEFAULT_USERID,
                                    "UnregisterUeceActivationCallback End", err);
    return err;
}

int32_t StorageManagerProvider::CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    StorageRadar::ReportFucBehavior("CreateUserDir", DEFAULT_USERID, "CreateUserDir Begin", E_OK);
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

    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    auto ret = sdCommunication->CreateUserDir(path, mode, uid, gid);
    LOGW("CreateUserDir end, uid: %{public}d, ret: %{public}d", callingUid, ret);

    std::string extraData = "path=" + path + "callingUid=" + std::to_string(callingUid);
    StorageRadar::ReportUserManager("CreateUserDir", 0, ret, extraData);
    return ret;
}

int32_t StorageManagerProvider::DeleteUserDir(const std::string &path)
{
    StorageRadar::ReportFucBehavior("DeleteUserDir", DEFAULT_USERID, "DeleteUserDir Begin", E_OK);
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

    std::shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    auto ret = sdCommunication->DeleteUserDir(path);
    LOGE("DeleteUserDir end, path: %{public}s, uid: %{public}d, ret: %{public}d", path.c_str(), callingUid, ret);
    std::string extraData = "path=" + path + "callingUid=" + std::to_string(callingUid);
    StorageRadar::ReportUserManager("DeleteUserDir", 0, ret, extraData);
    return ret;
}

int32_t StorageManagerProvider::NotifyUserChangedEvent(uint32_t userId, uint32_t eventType)
{
    StorageRadar::ReportFucBehavior("NotifyUserChangedEvent", userId, "NotifyUserChangedEvent Begin", E_OK);
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
#ifdef STORAGE_STATISTICS_MANAGER
    LOGI("NotifyUserChangedEvent UserId: %{public}u, type: %{public}d", userId, enumType);
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, enumType);
    StorageRadar::ReportFucBehavior("NotifyUserChangedEvent", userId, "NotifyUserChangedEvent End", E_OK);
#endif
    LOGI("NotifyUserChangedEvent Not support !");
    return E_OK;
}

int32_t StorageManagerProvider::SetExtBundleStats(uint32_t userId, const ExtBundleStats &stats)
{
    StorageRadar::ReportFucBehavior("SetExtBundleStats", userId, "SetExtBundleStats Begin", E_OK);
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
    int32_t ret = StorageStatusManager::GetInstance().SetExtBundleStats(userId, stats);
    StorageRadar::ReportFucBehavior("SetExtBundleStats", userId, "SetExtBundleStats End", ret);
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
    StorageRadar::ReportFucBehavior("GetExtBundleStats", userId, "GetExtBundleStats Begin", E_OK);
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
    int32_t ret = StorageStatusManager::GetInstance().GetExtBundleStats(userId, stats);
    StorageRadar::ReportFucBehavior("GetExtBundleStats", userId, "GetExtBundleStats End", ret);
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
    StorageRadar::ReportFucBehavior("GetAllExtBundleStats", userId, "GetAllExtBundleStats Begin", E_OK);
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
    int32_t ret = StorageStatusManager::GetInstance().GetAllExtBundleStats(userId, statsVec);
    StorageRadar::ReportFucBehavior("GetAllExtBundleStats", userId, "GetAllExtBundleStats Begin", ret);
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

int32_t StorageManagerProvider::NotifyCreateBundleDataDirWithEl(uint32_t userId, uint8_t elx)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER) || callingUid != ROOT_UID) {
        LOGE("NotifyCreateBundleDataDirWithEl permission denied ! uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }
    LOGI("CreateElxBundleDataDir start: userId %{public}u, elx is %{public}d", userId, elx);
    if (elx == StorageDaemon::EL1_KEY) {
        LOGW("CreateElxBundleDataDir pass: userId %{public}u, elx is %{public}d", userId, elx);
        return E_ERR;
    }
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_PERMISSION_DENIED;
    }
    int32_t ret = bundleMgr->CreateBundleDataDirWithEl(userId, static_cast<OHOS::AppExecFwk::DataDirEl>(elx));
    LOGI("CreateElxBundleDataDir end ret %{public}d", ret);
    return ret;
}

int32_t StorageManagerProvider::QueryActiveOsAccountIds(std::vector<int32_t> &ids)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER) || callingUid != ROOT_UID) {
        LOGE("QueryActiveOsAccountIds permission denied ! uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }
    return AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
}

int32_t StorageManagerProvider::IsOsAccountExists(unsigned int userId, bool &isOsAccountExists)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER) || callingUid != ROOT_UID) {
        LOGE("IsOsAccountExists permission denied ! uid: %{public}d", callingUid);
        return E_PERMISSION_DENIED;
    }
    return AccountSA::OsAccountManager::IsOsAccountExists(userId, isOsAccountExists);
}

int32_t StorageManagerProvider::ClearSecondMountPoint(uint32_t userId, const std::string &bundleName)
{
    StorageRadar::ReportFucBehavior("ClearSecondMountPoint", userId, "ClearSecondMountPoint Begin", E_OK);
    LOGI("clear second mount point start, userId is %{public}d, bundle is %{public}s.", userId, bundleName.c_str());
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER) || uid != FOUNDATION_UID) {
        LOGE("ClearSecondMountPoint permission denied, uid: %{public}d", uid);
        return E_PERMISSION_DENIED;
    }
    if (userId > TOP_USER_ID || bundleName.empty()) {
        LOGE("invalid params, userId: %{public}d.", userId);
        return E_PARAMS_INVALID;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->ClearSecondMountPoint(userId, bundleName);
    LOGI("clear second mount point end, ret is %{public}d.", err);
    StorageRadar::ReportFucBehavior("ClearSecondMountPoint", userId, "ClearSecondMountPoint End", err);
    return err;
}
} // namespace StorageManager
} // namespace OHOS
