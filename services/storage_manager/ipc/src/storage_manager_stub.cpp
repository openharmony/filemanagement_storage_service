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

#include "ipc/storage_manager_stub.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
static bool GetClientUid(int &uid)
{
    uid = IPCSkeleton::GetCallingUid();
    return true;
}

bool CheckClientPermission(const std::string& permissionStr)
{
    int uid = -1;
    if (!GetClientUid(uid)) {
        LOGE("GetClientUid: fail");
    }
    LOGI("uid: %{public}d", uid);
    if (uid == UID_ACCOUNTMGR || uid == UID_SYSTEM || uid == UID_ROOT) {
        LOGI("StorageManager permissionCheck pass!");
        return true;
    }
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller,
        permissionStr);
    if (res == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        LOGI("Have media permission");
        return true;
    }
    return false;
}
int32_t StorageManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        return E_PERMISSION_DENIED;
    }

    std::string permission = "ohos.permission.READ_MEDIA";
    if (!CheckClientPermission(permission)) {
        LOGE("StorageManager checkPermission error");
        return E_PERMISSION_DENIED;
    }
    int err = 0;
    switch (code) {
        case PREPARE_ADD_USER:
            HandlePrepareAddUser(data, reply);
            break;
        case REMOVE_USER:
            HandleRemoveUser(data, reply);
            break;
        case PREPARE_START_USER:
            HandlePrepareStartUser(data, reply);
            break;
        case STOP_USER:
            HandleStopUser(data, reply);
            break;
        case GET_TOTAL:
            HandleGetTotal(data, reply);
            break;
        case GET_FREE:
            HandleGetFree(data, reply);
            break;
        case GET_BUNDLE_STATUS:
            HandleGetBundleStatus(data, reply);
            break;
        case NOTIFY_VOLUME_CREATED:
            HandleNotifyVolumeCreated(data, reply);
            break;
        case NOTIFY_VOLUME_MOUNTED:
            HandleNotifyVolumeMounted(data, reply);
            break;
        case NOTIFY_VOLUME_DESTROYED:
            HandleNotifyVolumeDestroyed(data, reply);
            break;
        case MOUNT:
            HandleMount(data, reply);
            break;
        case UNMOUNT:
            HandleUnmount(data, reply);
            break;
        case GET_ALL_VOLUMES:
            HandleGetAllVolumes(data, reply);
            break;
        case NOTIFY_DISK_CREATED:
            HandleNotifyDiskCreated(data, reply);
            break;
        case NOTIFY_DISK_DESTROYED:
            HandleNotifyDiskDestroyed(data, reply);
            break;
        case PARTITION:
            HandlePartition(data, reply);
            break;
        case GET_ALL_DISKS:
            HandleGetAllDisks(data, reply);
            break;
        case CREATE_USER_KEYS:
            HandleGenerateUserKeys(data, reply);
            break;
        case DELETE_USER_KEYS:
            HandleDeleteUserKeys(data, reply);
            break;
        case UPDATE_USER_AUTH:
            HandleUpdateUserAuth(data, reply);
            break;
        case ACTIVE_USER_KEY:
            HandleActiveUserKey(data, reply);
            break;
        case INACTIVE_USER_KEY:
            HandleInactiveUserKey(data, reply);
            break;
        case UPDATE_KEY_CONTEXT:
            HandleUpdateKeyContext(data, reply);
            break;
        default: {
            LOGI("use IPCObjectStub default OnRemoteRequest");
            err = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return err;
}

int32_t StorageManagerStub::HandlePrepareAddUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();
    LOGI("StorageManagerStub::HandlePrepareAddUser, userId:%{public}d", userId);
    int err = PrepareAddUser(userId, flags);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePrepareAddUser call PrepareAddUser failed");
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleRemoveUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();
    LOGI("StorageManagerStub::HandleRemoveUser, userId:%{public}d", userId);
    int err = RemoveUser(userId, flags);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleRemoveUser call RemoveUser failed");
        return E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandlePrepareStartUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    LOGI("StorageManagerStub::HandlePrepareStartUser, userId:%{public}d", userId);
    int err = PrepareStartUser(userId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePrepareStartUser call PrepareStartUser failed");
        return E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleStopUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    LOGI("StorageManagerStub::HandleStopUser, userId:%{public}d", userId);
    int err = StopUser(userId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleStopUser call StopUser failed");
        return E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetTotal(MessageParcel &data, MessageParcel &reply)
{
    LOGE("StorageManagerStub::HandleGetTotal Begin.");
    std::string volumeId = data.ReadString();
    int64_t totalSize = GetTotalSizeOfVolume(volumeId);
    if (!reply.WriteInt64(totalSize)) {
        LOGE("StorageManagerStub::HandleGetTotal call OnUserDelete failed");
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetFree(MessageParcel &data, MessageParcel &reply)
{
    std::string volumeId = data.ReadString();
    int64_t freeSize = GetFreeSizeOfVolume(volumeId);
    if (!reply.WriteInt64(freeSize)) {
        LOGE("StorageManagerStub::HandleGetFree call OnUserDelete failed");
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetBundleStatus(MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::vector<int64_t> bundleStats = GetBundleStats(pkgName);
    if (!reply.WriteInt64Vector(bundleStats)) {
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetAllVolumes(MessageParcel &data, MessageParcel &reply)
{
    LOGE("StorageManagerStub::HandleGetAllVolumes Begin.");
    std::vector<VolumeExternal> ve = GetAllVolumes();
    uint size = ve.size();
    if (size == 0) {
        LOGE("StorageManagerStub::No volume.");
        if (!reply.WriteUint32(0)) {
            return  E_IPC_ERROR;
        }
        return E_OK;
    }
    if (!reply.WriteUint32(ve.size())) {
        return  E_IPC_ERROR;
    }
    for (uint i = 0; i < size; i++) {
        if (!ve[i].Marshalling(reply)) {
            return  E_IPC_ERROR;
        }
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyVolumeCreated(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<VolumeCore> vc = VolumeCore::Unmarshalling(data);
    NotifyVolumeCreated(*vc);
    LOGI("StorageManagerStub::HandleNotifyVolumeCreated");
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyVolumeMounted(MessageParcel &data, MessageParcel &reply)
{
    std::string volumeId = data.ReadString();
    int32_t fsType = data.ReadInt32();
    std::string fsUuid = data.ReadString();
    std::string path = data.ReadString();
    std::string description = data.ReadString();
    NotifyVolumeMounted(volumeId, fsType, fsUuid, path, description);
    LOGI("StorageManagerStub::HandleNotifyVolumeMounted");
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyVolumeDestroyed(MessageParcel &data, MessageParcel &reply)
{
    std::string volumeId = data.ReadString();
    NotifyVolumeDestroyed(volumeId);
    LOGI("StorageManagerStub::HandleNotifyVolumeDestroyed");
    return E_OK;
}

int32_t StorageManagerStub::HandleMount(MessageParcel &data, MessageParcel &reply)
{
    LOGE("StorageManagerStub::HandleMount Begin.");
    std::string volumeId = data.ReadString();
    int err = Mount(volumeId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleMount call Mount failed");
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleUnmount(MessageParcel &data, MessageParcel &reply)
{
    LOGE("StorageManagerStub::HandleUnmount Begin.");
    std::string volumeId = data.ReadString();
    int err = Unmount(volumeId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandleUnmount call Mount failed");
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyDiskCreated(MessageParcel &data, MessageParcel &reply)
{
    auto disk = Disk::Unmarshalling(data);
    LOGI("zwd, %{public}s", disk->GetDiskId().c_str());
    NotifyDiskCreated(*disk);
    return E_OK;
}

int32_t StorageManagerStub::HandleNotifyDiskDestroyed(MessageParcel &data, MessageParcel &reply)
{
    std::string diskId = data.ReadString();
    NotifyDiskDestroyed(diskId);
    return E_OK;
}

int32_t StorageManagerStub::HandlePartition(MessageParcel &data, MessageParcel &reply)
{
    std::string diskId = data.ReadString();
    int32_t type = data.ReadInt32();
    int err = Partition(diskId, type);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePartition call Partition failed");
        return E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGetAllDisks(MessageParcel &data, MessageParcel &reply)
{
    LOGE("StorageManagerStub::HandleGetAllDisk Begin.");
    std::vector<Disk> disks = GetAllDisks();
    uint size = disks.size();
    if (size == 0) {
        LOGE("StorageManagerStub::No Disk.");
        if (!reply.WriteUint32(0)) {
            return  E_IPC_ERROR;
        }
        return E_OK;
    }
    if (!reply.WriteUint32(disks.size())) {
        return  E_IPC_ERROR;
    }
    for (uint i = 0; i < size; i++) {
        if (!disks[i].Marshalling(reply)) {
            return  E_IPC_ERROR;
        }
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t flags = data.ReadUint32();
    int32_t err = GenerateUserKeys(userId, flags);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_IPC_ERROR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int32_t err = DeleteUserKeys(userId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_IPC_ERROR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    std::string auth = data.ReadString();
    std::string compSecret = data.ReadString();
    int32_t err = UpdateUserAuth(userId, auth, compSecret);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_IPC_ERROR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleActiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    std::string auth = data.ReadString();
    std::string compSecret = data.ReadString();
    int32_t err = ActiveUserKey(userId, auth, compSecret);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_IPC_ERROR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int32_t err = InactiveUserKey(userId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_IPC_ERROR;
    }

    return E_OK;
}

int32_t StorageManagerStub::HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int32_t err = UpdateKeyContext(userId);
    if (!reply.WriteInt32(err)) {
        LOGE("Write reply error code failed");
        return E_IPC_ERROR;
    }

    return E_OK;
}
} // StorageManager
} // OHOS
