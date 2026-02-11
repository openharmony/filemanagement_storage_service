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

#include "ipc/storage_manager_stub.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "storage/bundle_manager_connector.h"
#include "storage_manager_ipc_interface_code.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageManager {
using namespace std;

constexpr pid_t ACCOUNT_UID = 3058;
constexpr pid_t BACKUP_SA_UID = 1089;
constexpr pid_t FOUNDATION_UID = 5523;
constexpr pid_t DFS_UID = 1009;
const std::string MEDIALIBRARY_BUNDLE_NAME = "com.ohos.medialibrary.medialibrarydata";
const std::string SCENEBOARD_BUNDLE_NAME = "com.ohos.sceneboard";
const std::string SYSTEMUI_BUNDLE_NAME = "com.ohos.systemui";
const std::string PERMISSION_STORAGE_MANAGER_CRYPT = "ohos.permission.STORAGE_MANAGER_CRYPT";
const std::string PERMISSION_STORAGE_MANAGER = "ohos.permission.STORAGE_MANAGER";
const std::string PERMISSION_MOUNT_MANAGER = "ohos.permission.MOUNT_UNMOUNT_MANAGER";
const std::string PERMISSION_FORMAT_MANAGER = "ohos.permission.MOUNT_FORMAT_MANAGER";
const std::string PROCESS_NAME_FOUNDATION = "foundation";

bool CheckClientPermission(const std::string& permissionStr)
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

bool CheckClientPermissionForCrypt(const std::string& permissionStr)
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

StorageManagerStub::StorageManagerStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::PREPARE_ADD_USER)] =
        &StorageManagerStub::HandlePrepareAddUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::REMOVE_USER)] =
        &StorageManagerStub::HandleRemoveUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::PREPARE_START_USER)] =
        &StorageManagerStub::HandlePrepareStartUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::STOP_USER)] =
        &StorageManagerStub::HandleStopUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_TOTAL)] =
        &StorageManagerStub::HandleGetTotal;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_FREE)] =
        &StorageManagerStub::HandleGetFree;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_SYSTEM_SIZE)] =
        &StorageManagerStub::HandleGetSystemSize;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_TOTAL_SIZE)] =
        &StorageManagerStub::HandleGetTotalSize;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_FREE_SIZE)] =
        &StorageManagerStub::HandleGetFreeSize;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_CURR_USER_STATS)] =
        &StorageManagerStub::HandleGetCurrUserStorageStats;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_USER_STATS)] =
        &StorageManagerStub::HandleGetUserStorageStats;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_USER_STATS_BY_TYPE)] =
        &StorageManagerStub::HandleGetUserStorageStatsByType;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_CURR_BUNDLE_STATS)] =
        &StorageManagerStub::HandleGetCurrentBundleStats;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATUS)] =
        &StorageManagerStub::HandleGetBundleStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_CREATED)] =
        &StorageManagerStub::HandleNotifyVolumeCreated;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_MOUNTED)] =
        &StorageManagerStub::HandleNotifyVolumeMounted;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_STATE_CHANGED)] =
        &StorageManagerStub::HandleNotifyVolumeStateChanged;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT)] =
        &StorageManagerStub::HandleMount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UNMOUNT)] =
        &StorageManagerStub::HandleUnmount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_ALL_VOLUMES)] =
        &StorageManagerStub::HandleGetAllVolumes;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_CREATED)] =
        &StorageManagerStub::HandleNotifyDiskCreated;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_DESTROYED)] =
        &StorageManagerStub::HandleNotifyDiskDestroyed;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::PARTITION)] =
        &StorageManagerStub::HandlePartition;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_ALL_DISKS)] =
        &StorageManagerStub::HandleGetAllDisks;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_VOL_BY_UUID)] =
        &StorageManagerStub::HandleGetVolumeByUuid;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_VOL_BY_ID)] =
        &StorageManagerStub::HandleGetVolumeById;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::SET_VOL_DESC)] =
        &StorageManagerStub::HandleSetVolDesc;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::FORMAT)] =
        &StorageManagerStub::HandleFormat;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_DISK_BY_ID)] =
        &StorageManagerStub::HandleGetDiskById;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_USER_KEYS)] =
        &StorageManagerStub::HandleGenerateUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::DELETE_USER_KEYS)] =
        &StorageManagerStub::HandleDeleteUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH)] =
        &StorageManagerStub::HandleUpdateUserAuth;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH_RECOVER_KEY)] =
        &StorageManagerStub::HandleUpdateUseAuthWithRecoveryKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::ACTIVE_USER_KEY)] =
        &StorageManagerStub::HandleActiveUserKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::INACTIVE_USER_KEY)] =
        &StorageManagerStub::HandleInactiveUserKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::LOCK_USER_SCREEN)] =
        &StorageManagerStub::HandleLockUserScreen;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UNLOCK_USER_SCREEN)] =
        &StorageManagerStub::HandleUnlockUserScreen;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::LOCK_SCREEN_STATUS)] =
        &StorageManagerStub::HandleGetLockScreenStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_KEY_CONTEXT)] =
        &StorageManagerStub::HandleUpdateKeyContext;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_SHARE_FILE)] =
        &StorageManagerStub::HandleCreateShareFile;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::DELETE_SHARE_FILE)] =
        &StorageManagerStub::HandleDeleteShareFile;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::SET_BUNDLE_QUOTA)] =
        &StorageManagerStub::HandleSetBundleQuota;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_MEM_PARA)] =
        &StorageManagerStub::HandleUpdateMemoryPara;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATS_INCREASE)] =
        &StorageManagerStub::HandleGetBundleStatsForIncrease;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GENERATE_APP_KEY)] =
        &StorageManagerStub::HandleGenerateAppkey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::DELETE_APP_KEY)] =
        &StorageManagerStub::HandleDeleteAppkey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT_DFS_DOCS)] =
        &StorageManagerStub::HandleMountDfsDocs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UMOUNT_DFS_DOCS)] =
        &StorageManagerStub::HandleUMountDfsDocs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_FILE_ENCRYPT_STATUS)] =
        &StorageManagerStub::HandleGetFileEncryptStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_RECOVER_KEY)] =
        &StorageManagerStub::HandleCreateRecoverKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::GET_USER_NEED_ACTIVE_STATUS)] =
        &StorageManagerStub::HandleGetUserNeedActiveStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::SET_RECOVER_KEY)] =
        &StorageManagerStub::HandleSetRecoverKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_MTP_MOUNT)] =
        &StorageManagerStub::HandleNotifyMtpMount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_MTP_UNMOUNT)] =
        &StorageManagerStub::HandleNotifyMtpUnmount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT_MEDIA_FUSE)] =
        &StorageManagerStub::HandleMountMediaFuse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UMOUNT_MEDIA_FUSE)] =
        &StorageManagerStub::HandleUMountMediaFuse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT_FILE_MGR_FUSE)] =
            &StorageManagerStub::HandleMountFileMgrFuse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageManagerInterfaceCode::UMOUNT_FILE_MGR_FUSE)] =
            &StorageManagerStub::HandleUMountFileMgrFuse;
}

int32_t StorageManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_PERMISSION_DENIED;
    }
    switch (code) {
        case static_cast<uint32_t>(StorageManagerInterfaceCode::PREPARE_ADD_USER):
            return HandlePrepareAddUser(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::REMOVE_USER):
            return HandleRemoveUser(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::PREPARE_START_USER):
            return HandlePrepareStartUser(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::STOP_USER):
            return HandleStopUser(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::COMPLETE_ADD_USER):
            return HandleCompleteAddUser(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_TOTAL):
            return HandleGetTotal(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_FREE):
            return HandleGetFree(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_SYSTEM_SIZE):
            return HandleGetSystemSize(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_TOTAL_SIZE):
            return HandleGetTotalSize(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_FREE_SIZE):
            return HandleGetFreeSize(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_CURR_USER_STATS):
            return HandleGetCurrUserStorageStats(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_USER_STATS):
            return HandleGetUserStorageStats(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_USER_STATS_BY_TYPE):
            return HandleGetUserStorageStatsByType(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_CURR_BUNDLE_STATS):
            return HandleGetCurrentBundleStats(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATUS):
            return HandleGetBundleStatus(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_CREATED):
            return HandleNotifyVolumeCreated(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_MOUNTED):
            return HandleNotifyVolumeMounted(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_STATE_CHANGED):
            return HandleNotifyVolumeStateChanged(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT):
            return HandleMount(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UNMOUNT):
            return HandleUnmount(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_ALL_VOLUMES):
            return HandleGetAllVolumes(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_CREATED):
            return HandleNotifyDiskCreated(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_DESTROYED):
            return HandleNotifyDiskDestroyed(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::PARTITION):
            return HandlePartition(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_ALL_DISKS):
            return HandleGetAllDisks(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_VOL_BY_UUID):
            return HandleGetVolumeByUuid(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_VOL_BY_ID):
            return HandleGetVolumeById(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::SET_VOL_DESC):
            return HandleSetVolDesc(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::FORMAT):
            return HandleFormat(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_DISK_BY_ID):
            return HandleGetDiskById(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_USER_KEYS):
            return HandleGenerateUserKeys(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::DELETE_USER_KEYS):
            return HandleDeleteUserKeys(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH):
            return HandleUpdateUserAuth(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH_RECOVER_KEY):
            return HandleUpdateUseAuthWithRecoveryKey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::ACTIVE_USER_KEY):
            return HandleActiveUserKey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::INACTIVE_USER_KEY):
            return HandleInactiveUserKey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::LOCK_USER_SCREEN):
            return HandleLockUserScreen(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UNLOCK_USER_SCREEN):
            return HandleUnlockUserScreen(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::LOCK_SCREEN_STATUS):
            return HandleGetLockScreenStatus(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_KEY_CONTEXT):
            return HandleUpdateKeyContext(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_SHARE_FILE):
            return HandleCreateShareFile(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::DELETE_SHARE_FILE):
            return HandleDeleteShareFile(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::SET_BUNDLE_QUOTA):
            return HandleSetBundleQuota(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UPDATE_MEM_PARA):
            return HandleUpdateMemoryPara(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATS_INCREASE):
            return HandleGetBundleStatsForIncrease(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT_DFS_DOCS):
            return HandleMountDfsDocs(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UMOUNT_DFS_DOCS):
            return HandleUMountDfsDocs(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GENERATE_APP_KEY):
            return HandleGenerateAppkey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::DELETE_APP_KEY):
            return HandleDeleteAppkey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_FILE_ENCRYPT_STATUS):
            return HandleGetFileEncryptStatus(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::GET_USER_NEED_ACTIVE_STATUS):
            return HandleGetUserNeedActiveStatus(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_RECOVER_KEY):
            return HandleCreateRecoverKey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::SET_RECOVER_KEY):
            return HandleSetRecoverKey(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_MTP_MOUNT):
            return HandleNotifyMtpMount(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::NOTIFY_MTP_UNMOUNT):
            return HandleNotifyMtpUnmount(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT_MEDIA_FUSE):
            return HandleMountMediaFuse(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UMOUNT_MEDIA_FUSE):
            return HandleUMountMediaFuse(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::MOUNT_FILE_MGR_FUSE):
            return HandleMountFileMgrFuse(data, reply);
        case static_cast<uint32_t>(StorageManagerInterfaceCode::UMOUNT_FILE_MGR_FUSE):
            return HandleUMountFileMgrFuse(data, reply);
        default:
            LOGE("Cannot response request %{public}d: unknown tranction", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t StorageManagerStub::HandlePrepareAddUser(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();
    LOGI("StorageManagerStub::HandlePrepareAddUser, userId:%{public}d", userId);
    int err = PrepareAddUser(userId, flags);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePrepareAddUser call PrepareAddUser failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleRemoveUser(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();
    LOGI("StorageManagerStub::HandleRemoveUser, userId:%{public}d", userId);
    int err = RemoveUser(userId, flags);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleRemoveUser call RemoveUser failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandlePrepareStartUser(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    int32_t userId = data.ReadInt32();
    LOGI("StorageManagerStub::HandlePrepareStartUser, userId:%{public}d", userId);
    int err = PrepareStartUser(userId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePrepareStartUser call PrepareStartUser failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleStopUser(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    int32_t userId = data.ReadInt32();
    LOGI("StorageManagerStub::HandleStopUser, userId:%{public}d", userId);
    int err = StopUser(userId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleStopUser call StopUser failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleCompleteAddUser(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    int32_t userId = data.ReadInt32();
    LOGI("StorageManagerStub::HandleCompleteAddUser, userId:%{public}d", userId);
    int err = CompleteAddUser(userId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleCompleteAddUser call CompleteAddUser failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetTotal(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    LOGE("StorageManagerStub::HandleGetTotal Begin.");
    std::string volumeId = data.ReadString();
    int64_t totalSize;
    int32_t err = GetTotalSizeOfVolume(volumeId, totalSize);
    if (!reply.WriteInt32(err)) {
        LOGE("StorageManagerStub::HandleGetTotal call OnUserDGetTotalSizeOfVolume failed");
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(totalSize)) {
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetFree(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volumeId = data.ReadString();
    int64_t freeSize;
    int32_t err = GetFreeSizeOfVolume(volumeId, freeSize);
    if (!reply.WriteInt32(err)) {
        LOGE("StorageManagerStub::HandleGetFree call GetFreeSizeOfVolume failed");
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(freeSize)) {
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetBundleStatus(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string pkgName = data.ReadString();
    int32_t appIndex = data.ReadInt32();
    uint32_t statFlag = data.ReadUint32();
    BundleStats bundleStats;
    int32_t err = GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!bundleStats.Marshalling(reply)) {
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetSystemSize(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    int64_t systemSize;
    int32_t err = GetSystemSize(systemSize);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(systemSize)) {
        LOGE("StorageManagerStub::HandleGetFree call GetSystemSize failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetTotalSize(MessageParcel &data, MessageParcel &reply)
{
    int64_t totalSize;
    int32_t err = GetTotalSize(totalSize);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(totalSize)) {
        LOGE("StorageManagerStub::HandleGetFree call GetTotalSize failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetFreeSize(MessageParcel &data, MessageParcel &reply)
{
    int64_t freeSize;
    int32_t err = GetFreeSize(freeSize);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(freeSize)) {
        LOGE("StorageManagerStub::HandleGetFree call GetFreeSize failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetCurrUserStorageStats(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    StorageStats storageStats;
    int32_t err = GetUserStorageStats(storageStats);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!storageStats.Marshalling(reply)) {
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetUserStorageStats(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    int32_t userId = data.ReadInt32();
    StorageStats storageStats;
    int32_t err = GetUserStorageStats(userId, storageStats);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!storageStats.Marshalling(reply)) {
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetCurrentBundleStats(MessageParcel &data, MessageParcel &reply)
{
    BundleStats bundleStats;
    uint32_t statFlag = data.ReadUint32();
    int32_t err = GetCurrentBundleStats(bundleStats, statFlag);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    if (!bundleStats.Marshalling(reply)) {
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetAllVolumes(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::vector<VolumeExternal> ve;
    int32_t err = GetAllVolumes(ve);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }
    uint size = ve.size();
    if (size == 0) {
        LOGE("StorageManagerStub::No volume.");
        if (!reply.WriteUint32(0)) {
            return  E_WRITE_REPLY_ERR;
        }
        return E_OK;
    }
    if (!reply.WriteUint32(ve.size())) {
        return  E_WRITE_REPLY_ERR;
    }
    for (uint i = 0; i < size; i++) {
        if (!ve[i].Marshalling(reply)) {
            return  E_WRITE_REPLY_ERR;
        }
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyVolumeCreated(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::unique_ptr<VolumeCore> vc = VolumeCore::Unmarshalling(data);
    NotifyVolumeCreated(*vc);
    LOGI("StorageManagerStub::HandleNotifyVolumeCreated");
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyVolumeMounted(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volumeId = data.ReadString();
    int32_t fsType = data.ReadInt32();
    std::string fsUuid = data.ReadString();
    std::string path = data.ReadString();
    std::string description = data.ReadString();
    NotifyVolumeMounted(volumeId, fsType, fsUuid, path, description);
    LOGI("StorageManagerStub::HandleNotifyVolumeMounted");
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyVolumeStateChanged(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volumeId = data.ReadString();
    VolumeState state = VolumeState(data.ReadInt32());
    NotifyVolumeStateChanged(volumeId, state);
    LOGI("StorageManagerStub::HandleNotifyVolumeStateChanged");
    return E_OK;
}

int32_t StorageManagerStub::HandleMount(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volumeId = data.ReadString();
    int err = Mount(volumeId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleMount call Mount failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUnmount(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volumeId = data.ReadString();
    int err = Unmount(volumeId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleUnmount call Mount failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyDiskCreated(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    auto disk = Disk::Unmarshalling(data);
    NotifyDiskCreated(*disk);
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyDiskDestroyed(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string diskId = data.ReadString();
    NotifyDiskDestroyed(diskId);
    return E_OK;
}

int32_t StorageManagerStub::HandlePartition(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_FORMAT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string diskId = data.ReadString();
    int32_t type = data.ReadInt32();
    int err = Partition(diskId, type);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePartition call Partition failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetAllDisks(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::vector<Disk> disks;
    int32_t err = GetAllDisks(disks);
    if (!reply.WriteUint32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    uint size = disks.size();
    if (size == 0) {
        LOGE("StorageManagerStub::No Disk.");
        if (!reply.WriteUint32(0)) {
            return  E_WRITE_REPLY_ERR;
        }
        return E_OK;
    }
    if (!reply.WriteUint32(disks.size())) {
        return  E_WRITE_REPLY_ERR;
    }
    for (uint i = 0; i < size; i++) {
        if (!disks[i].Marshalling(reply)) {
            return  E_WRITE_REPLY_ERR;
        }
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetVolumeByUuid(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string fsUuid = data.ReadString();
    VolumeExternal vc;
    int err = GetVolumeByUuid(fsUuid, vc);
    if (!vc.Marshalling(reply)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleGetVolumeByUuid call GetVolumeByUuid failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetVolumeById(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volId = data.ReadString();
    VolumeExternal vc;
    int err = GetVolumeById(volId, vc);
    if (!vc.Marshalling(reply)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleGetVolumeById call GetVolumeById failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleSetVolDesc(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_MOUNT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string fsUuid = data.ReadString();
    std::string desc = data.ReadString();
    int err = SetVolumeDescription(fsUuid, desc);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleSetVolDesc call SetVolumeDescription failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleFormat(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_FORMAT_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volId = data.ReadString();
    std::string fsType = data.ReadString();
    int err = Format(volId, fsType);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleFormat call Format failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetDiskById(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    std::string volId = data.ReadString();
    Disk disk;
    int err = GetDiskById(volId, disk);
    if (!disk.Marshalling(reply)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleGetDiskById call GetDiskById failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    uint32_t flags = data.ReadUint32();
    int32_t err = GenerateUserKeys(userId, flags);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    int32_t err = DeleteUserKeys(userId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    uint64_t secureUid = data.ReadUint64();

    std::vector<uint8_t> token;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&oldSecret);
    data.ReadUInt8Vector(&newSecret);

    int32_t err = UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUpdateUseAuthWithRecoveryKey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    uint64_t secureUid = data.ReadUint64();

    std::vector<uint8_t> token;
    std::vector<uint8_t> newSecret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&newSecret);
    std::vector<std::vector<uint8_t>> plainText;
    const int CKEY_NUMS = 6;
    for (uint32_t i = 0; i < CKEY_NUMS; i++) {
        std::vector<uint8_t> iv;
        data.ReadUInt8Vector(&iv);
        plainText.push_back(iv);
    }

    int32_t err = UpdateUseAuthWithRecoveryKey(token, newSecret, secureUid, userId, plainText);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleActiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT) &&
        IPCSkeleton::GetCallingUid() != ACCOUNT_UID) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int32_t err = ActiveUserKey(userId, token, secret);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    int32_t err = InactiveUserKey(userId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleLockUserScreen(MessageParcel &data, MessageParcel &reply)
{
    // Only for sceneboard
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    uint32_t userId = data.ReadUint32();
    auto bundleMgr = DelayedSingleton<BundleMgrConnector>::GetInstance()->GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        StorageService::StorageRadar::ReportBundleMgrResult("StorageManagerStub::HandleLockUserScreen",
            E_BUNDLEMGR_ERROR, userId, "pkgName="+SCENEBOARD_BUNDLE_NAME);
        return E_BUNDLEMGR_ERROR;
    }

    if (bundleName != SCENEBOARD_BUNDLE_NAME && bundleName != SYSTEMUI_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d), should be %{public}s",
            bundleName.c_str(), uid, SCENEBOARD_BUNDLE_NAME.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }

    int32_t err = LockUserScreen(userId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleGetFileEncryptStatus(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    bool isEncrypted = true;
    uint32_t userId = data.ReadUint32();
    bool needCheckDirMount = data.ReadBool();
    int32_t err = GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    if (!reply.WriteBool(isEncrypted)) {
        LOGE("Write reply isEncrypted failed");
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetUserNeedActiveStatus(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    bool needActive = false;
    uint32_t userId = data.ReadUint32();
    int32_t err = GetUserNeedActiveStatus(userId, needActive);
    if (!reply.WriteBool(needActive)) {
        LOGE("Write needActive failed");
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUnlockUserScreen(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int32_t err = UnlockUserScreen(userId, token, secret);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetLockScreenStatus(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    bool lockScreenStatus = false;
    int32_t err = GetLockScreenStatus(userId, lockScreenStatus);
    if (!reply.WriteBool(lockScreenStatus)) {
        LOGE("Write reply lockScreenStatus failed");
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGenerateAppkey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t hashId = data.ReadUint32();
    uint32_t userId = data.ReadUint32();
    std::string keyId;
    int32_t err = GenerateAppkey(hashId, userId, keyId);
    if (!reply.WriteString(keyId)) {
        LOGE("Write reply lockScreenStatus failed");
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleDeleteAppkey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    std::string keyId = data.ReadString();
    int32_t err = DeleteAppkey(keyId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleCreateRecoverKey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    uint32_t userType = data.ReadUint32();
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int32_t err = CreateRecoverKey(userId, userType, token, secret);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleSetRecoverKey(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    std::vector<uint8_t> key;
    data.ReadUInt8Vector(&key);

    int32_t err = SetRecoverKey(key);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForCrypt(PERMISSION_STORAGE_MANAGER_CRYPT)) {
        return E_PERMISSION_DENIED;
    }
    uint32_t userId = data.ReadUint32();
    bool needRemoveTmpKey = data.ReadBool();
    int32_t err = UpdateKeyContext(userId, needRemoveTmpKey);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleCreateShareFile(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForShareFile()) {
        return E_PERMISSION_DENIED;
    }

    std::vector<std::string> uriList;
    if (!data.ReadStringVector(&uriList)) {
        return E_WRITE_REPLY_ERR;
    }
    uint32_t tokenId = data.ReadUint32();
    uint32_t flag = data.ReadUint32();
    std::vector<int32_t> retList = CreateShareFile(uriList, tokenId, flag);
    if (!reply.WriteInt32Vector(retList)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermissionForShareFile()) {
        return E_PERMISSION_DENIED;
    }

    uint32_t tokenId = data.ReadUint32();
    std::vector<std::string> uriList;
    if (!data.ReadStringVector(&uriList)) {
        return E_WRITE_REPLY_ERR;
    }

    int err = DeleteShareFile(tokenId, uriList);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleSetBundleQuota(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    std::string bundleName = data.ReadString();
    int32_t uid = data.ReadInt32();
    std::string bundleDataDirPath = data.ReadString();
    int32_t limitSizeMb = data.ReadInt32();
    int err = SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetBundleStatsForIncrease(MessageParcel &data, MessageParcel &reply)
{
    if (IPCSkeleton::GetCallingUid() != BACKUP_SA_UID) {
        LOGE("StorageManager permissionCheck error, calling uid is invalid, need backup_sa uid.");
        return E_PERMISSION_DENIED;
    }

    uint32_t userId = data.ReadUint32();
    std::vector<std::string> bundleNames;
    if (!data.ReadStringVector(&bundleNames)) {
        return E_WRITE_REPLY_ERR;
    }
    std::vector<int64_t> incrementalBackTimes;
    if (!data.ReadInt64Vector(&incrementalBackTimes)) {
        return E_WRITE_REPLY_ERR;
    }

    std::vector<int64_t> pkgFileSizes;
    std::vector<int64_t> incPkgFileSizes;
    int32_t err = GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes, incPkgFileSizes);
    if (!reply.WriteUint32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64Vector(pkgFileSizes)) {
        LOGE("StorageManagerStub::HandleGetBundleStatsForIncrease call GetBundleStatsForIncrease failed");
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64Vector(incPkgFileSizes)) {
        LOGE("StorageManagerStub::HandleGetBundleStatsForIncrease call GetBundleStatsForIncrease failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}


int32_t StorageManagerStub::HandleGetUserStorageStatsByType(MessageParcel &data, MessageParcel &reply)
{
    if (IPCSkeleton::GetCallingUid() != BACKUP_SA_UID) {
        LOGE("StorageManager permissionCheck error, calling uid is invalid, need backup_sa uid.");
        return E_PERMISSION_DENIED;
    }

    int32_t userId = data.ReadInt32();
    std::string type = data.ReadString();
    StorageStats storageStats;
    int32_t err = GetUserStorageStatsByType(userId, storageStats, type);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!storageStats.Marshalling(reply)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUpdateMemoryPara(MessageParcel &data, MessageParcel &reply)
{
    if (IPCSkeleton::GetCallingUid() != BACKUP_SA_UID) {
        LOGE("StorageManager permissionCheck error, calling uid is invalid, need backup_sa uid.");
        return E_PERMISSION_DENIED;
    }

    int32_t size = data.ReadInt32();
    int32_t oldSize = 0;
    int err = UpdateMemoryPara(size, oldSize);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(oldSize)) {
        LOGE("StorageManagerStub::HandleUpdateMemoryPara call UpdateMemoryPara failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleMountDfsDocs(MessageParcel &data, MessageParcel &reply)
{
    // Only for dfs create device dir and bind mount from DFS Docs.
    if (IPCSkeleton::GetCallingUid() != DFS_UID) {
        LOGE("HandleMountDfsDocs permissionCheck error, calling uid now is %{public}d, should be DFS_UID: %{public}d",
            IPCSkeleton::GetCallingUid(), DFS_UID);
        return E_PERMISSION_DENIED;
    }

    int32_t userId = data.ReadInt32();
    std::string relativePath = data.ReadString();
    std::string networkId = data.ReadString();
    std::string deviceId = data.ReadString();

    int32_t err = MountDfsDocs(userId, relativePath, networkId, deviceId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUMountDfsDocs(MessageParcel &data, MessageParcel &reply)
{
    // Only for dfs create device dir and bind mount from DFS Docs.
    if (IPCSkeleton::GetCallingUid() != DFS_UID) {
        LOGE("HandleUMountDfsDocs permissionCheck error, calling uid now is %{public}d, should be DFS_UID: %{public}d",
             IPCSkeleton::GetCallingUid(), DFS_UID);
        return E_PERMISSION_DENIED;
    }

    int32_t userId = data.ReadInt32();
    std::string relativePath = data.ReadString();
    std::string networkId = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t err = UMountDfsDocs(userId, relativePath, networkId, deviceId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyMtpMount(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    std::string id = data.ReadString();
    std::string path = data.ReadString();
    std::string desc = data.ReadString();
    std::string uuid = data.ReadString();
    int32_t err = NotifyMtpMounted(id, path, desc, uuid);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }

    LOGI("StorageManagerStub::HandleNotifyMtpMount");
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyMtpUnmount(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER)) {
        return E_PERMISSION_DENIED;
    }

    std::string id = data.ReadString();
    std::string path = data.ReadString();
    bool isBadRemove = data.ReadBool();
    int32_t err = NotifyMtpUnmounted(id, path, isBadRemove);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_WRITE_REPLY_ERR;
    }

    LOGI("StorageManagerStub::HandleNotifyMtpUnmount");
    return E_OK;
}

int32_t StorageManagerStub::HandleMountMediaFuse(MessageParcel &data, MessageParcel &reply)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageManagerStub::HandleMountMediaFuse start.");

    // Only for medialibrary to mount fuse.
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = DelayedSingleton<BundleMgrConnector>::GetInstance()->GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return E_BUNDLEMGR_ERROR;
    }
    if (bundleName != MEDIALIBRARY_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d), should be %{public}s",
            bundleName.c_str(), uid, MEDIALIBRARY_BUNDLE_NAME.c_str());
        return E_PERMISSION_DENIED;
    }

    int32_t userId = data.ReadInt32();
    int32_t fd = -1;
    int32_t ret = MountMediaFuse(userId, fd);
    if (!reply.WriteInt32(ret)) {
        LOGE("Write reply error code failed");
        if (ret == E_OK) {
            close(fd);
        }
        return E_WRITE_REPLY_ERR;
    }
    if (ret == E_OK) {
        if (!reply.WriteFileDescriptor(fd)) {
            LOGE("Write reply fd failed");
            close(fd);
            return E_WRITE_REPLY_ERR;
        }
        close(fd);
    }
#endif
    return E_OK;
}

int32_t StorageManagerStub::HandleUMountMediaFuse(MessageParcel &data, MessageParcel &reply)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageManagerStub::HandleUMountMediaFuse start.");

    // Only for medialibrary to mount fuse.
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = DelayedSingleton<BundleMgrConnector>::GetInstance()->GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return E_BUNDLEMGR_ERROR;
    }
    if (bundleName != MEDIALIBRARY_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d), should be %{public}s",
            bundleName.c_str(), uid, MEDIALIBRARY_BUNDLE_NAME.c_str());
        return E_PERMISSION_DENIED;
    }

    int32_t userId = data.ReadInt32();
    int32_t ret = UMountMediaFuse(userId);
    if (!reply.WriteInt32(ret)) {
        return E_WRITE_REPLY_ERR;
    }
#endif
    return E_OK;
}

int32_t StorageManagerStub::HandleMountFileMgrFuse(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER) || !IsCalledByFileMgr()) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerStub::HandleMountFileMgrFuse start.");
    int32_t userId = data.ReadInt32();
    std::string path = data.ReadString();
    if (!IsPathStartWithFileMgr(userId, path)) {
        return E_PARAMS_INVALID;
    }
    int32_t fuseFd = -1;
    int32_t ret = MountFileMgrFuse(userId, path, fuseFd);
    if (!reply.WriteInt32(ret)) {
        LOGE("Write reply error code failed");
        if (ret == E_OK) {
            close(fuseFd);
        }
        return E_WRITE_REPLY_ERR;
    }
    if (ret == E_OK) {
        if (!reply.WriteFileDescriptor(fuseFd)) {
            LOGE("Write reply fuseFd failed");
            close(fuseFd);
            return E_WRITE_REPLY_ERR;
        }
        close(fuseFd);
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUMountFileMgrFuse(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckClientPermission(PERMISSION_STORAGE_MANAGER) || !IsCalledByFileMgr()) {
        return E_PERMISSION_DENIED;
    }
    LOGI("StorageManagerStub::HandleUMountFileMgrFuse start.");
    int32_t userId = data.ReadInt32();
    std::string path = data.ReadString();
    if (!IsPathStartWithFileMgr(userId, path)) {
        return E_PARAMS_INVALID;
    }
    int32_t ret = UMountFileMgrFuse(userId, path);
    if (!reply.WriteInt32(ret)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

bool StorageManagerStub::IsCalledByFileMgr()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return false;
    }
    std::string bundleName;
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return false;
    }
    if (bundleName != FILEMGR_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d)", bundleName.c_str(), uid);
        return false;
    }
    return true;
}

bool StorageManagerStub::IsPathStartWithFileMgr(int32_t userId, const std::string &path)
{
    const std::string prefix = "/mnt/data/" + std::to_string(userId) + "/userExternal/";
    if (path.size() <= prefix.size()) {
        return false;
    }
    return path.compare(0, prefix.length(), prefix) == 0;
}
} // StorageManager
} // OHOS
