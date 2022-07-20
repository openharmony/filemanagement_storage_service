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

#include "ipc/storage_manager_proxy.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
int32_t StorageManagerProxy::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManagerProxy::PrepareAddUser, userId:%{public}d", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteInt32 failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(flags)) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteUint32 failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(PREPARE_ADD_USER, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::PrepareAddUser, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManagerProxy::RemoveUser, userId:%{public}d", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::RemoveUser, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::RemoveUser, WriteInt32 failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(flags)) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteUint32 failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(REMOVE_USER, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::RemoveUser, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::PrepareStartUser(int32_t userId)
{
    LOGI("StorageManagerProxy::PrepareStartUser, userId:%{public}d", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::PrepareStartUser, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::PrepareStartUser, WriteInt32 failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(PREPARE_START_USER, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::PrepareStartUser, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::StopUser(int32_t userId)
{
    LOGI("StorageManagerProxy::StopUser, userId:%{public}d", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::StopUser, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::StopUser, WriteInt32 failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(STOP_USER, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::StopUser, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("user ID: %{public}u, flags: %{public}u", userId, flags);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(flags)) {
        LOGE("Write key flags failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(CREATE_USER_KEYS, data, reply, option);
    if (err != E_OK) {
        LOGE("SendRequest failed");
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::DeleteUserKeys(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(DELETE_USER_KEYS, data, reply, option);
    if (err != E_OK) {
        LOGE("SendRequest failed");
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::UpdateUserAuth(uint32_t userId,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &oldSecret,
                                            const std::vector<uint8_t> &newSecret)
{
    LOGI("user ID: %{public}u", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUInt8Vector(token)) {
        LOGE("Write token failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUInt8Vector(oldSecret)) {
        LOGE("Write oldSecret failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUInt8Vector(newSecret)) {
        LOGE("Write newSecret failed");
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(UPDATE_USER_AUTH, data, reply, option);
    if (err != E_OK) {
        LOGE("SendRequest failed");
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::ActiveUserKey(uint32_t userId,
                                           const std::vector<uint8_t> &token,
                                           const std::vector<uint8_t> &secret)
{
    LOGI("user ID: %{public}u", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUInt8Vector(token)) {
        LOGE("Write token failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUInt8Vector(secret)) {
        LOGE("Write secret failed");
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(ACTIVE_USER_KEY, data, reply, option);
    if (err != E_OK) {
        LOGE("SendRequest failed");
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::InactiveUserKey(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(INACTIVE_USER_KEY, data, reply, option);
    if (err != E_OK) {
        LOGE("SendRequest failed");
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::UpdateKeyContext(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(UPDATE_KEY_CONTEXT, data, reply, option);
    if (err != E_OK) {
        LOGE("SendRequest failed");
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int64_t StorageManagerProxy::GetFreeSizeOfVolume(std::string volumeUuid)
{
    LOGI("StorageManagerProxy::GetFreeSizeOfVolume, volumeUuid:%{public}s", volumeUuid.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetFreeSizeOfVolume, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    if (!data.WriteString(volumeUuid)) {
        LOGE("StorageManagerProxy::GetFreeSizeOfVolume, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(GET_FREE, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetFreeSizeOfVolume, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt64();
}

int64_t StorageManagerProxy::GetTotalSizeOfVolume(std::string volumeUuid)
{
    LOGI("StorageManagerProxy::GetTotalSizeOfVolume, volumeUuid:%{public}s", volumeUuid.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetTotalSizeOfVolume, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    if (!data.WriteString(volumeUuid)) {
        LOGE("StorageManagerProxy::GetTotalSizeOfVolume, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(GET_TOTAL, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetTotalSizeOfVolume, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt64();
}

BundleStats StorageManagerProxy::GetBundleStats(std::string pkgName)
{
    BundleStats result;
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return result;
    }

    if (!data.WriteString(pkgName)) {
        return result;
    }
    int err = Remote()->SendRequest(GET_BUNDLE_STATUS, data, reply, option);
    if (err != E_OK) {
        return result;
    }
    result = *BundleStats::Unmarshalling(reply);
    return result;
}

void StorageManagerProxy::NotifyVolumeCreated(VolumeCore vc)
{
    LOGI("StorageManagerProxy::NotifyVolumeCreated, volumeUuid:%{public}s", vc.GetId().c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyVolumeCreated, WriteInterfaceToken failed");
        return;
    }

    vc.Marshalling(data);
    int err = Remote()->SendRequest(NOTIFY_VOLUME_CREATED, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::NotifyVolumeCreated, SendRequest failed");
    }
}

void StorageManagerProxy::NotifyVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
                                              std::string path, std::string description)
{
    LOGI("StorageManagerProxy::NotifyVolumeMounted, volumeUuid:%{public}s", volumeId.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(fsType)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(fsUuid)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(path)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(description)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return;
    }

    int err = Remote()->SendRequest(NOTIFY_VOLUME_MOUNTED, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, SendRequest failed");
    }
}
void StorageManagerProxy::NotifyVolumeDestroyed(std::string volumeId)
{
    LOGI("StorageManagerProxy::NotifyVolumedestroyed, volumeId:%{public}s", volumeId.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyVolumedestroyed, WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::NotifyVolumedestroyed, WriteInterfaceToken failed");
        return;
    }
    int err = Remote()->SendRequest(NOTIFY_VOLUME_DESTROYED, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::NotifyVolumeDestroyed, SendRequest failed");
    }
}

int32_t StorageManagerProxy::Mount(std::string volumeId)
{
    LOGI("StorageManagerProxy::Mount, volumeId:%{public}s", volumeId.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::Mount, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::Mount, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(MOUNT, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::Mount, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::Unmount(std::string volumeId)
{
    LOGI("StorageManagerProxy::Unmount, volumeId:%{public}s", volumeId.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::Unmount, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::Unmount, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(UNMOUNT, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::Unmount, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt32();
}

std::vector<VolumeExternal> StorageManagerProxy::GetAllVolumes()
{
    std::vector<VolumeExternal> result = {};
    LOGI("StorageManagerProxy::GetAllVolumes");
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetAllVolumes, WriteInterfaceToken failed");
        return result;
    }

    int err = Remote()->SendRequest(GET_ALL_VOLUMES, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetAllVolumes, SendRequest failed");
        return result;
    }
    uint size = reply.ReadUint32();
    if (size == 0) {
        return result;
    }
    for (uint i = 0; i < size; i++) {
        std::unique_ptr<VolumeExternal> ve = VolumeExternal::Unmarshalling(reply);
        LOGI("StorageManagerProxy::GetAllVolumes push %{public}s", ve->GetId().c_str());
        result.push_back(*ve);
    }
    return result;
}

void StorageManagerProxy::NotifyDiskCreated(Disk disk)
{
    LOGI("StorageManagerProxy::NotifyDiskCreate, diskId:%{public}s", disk.GetDiskId().c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyDiskCreate, WriteInterfaceToken failed");
        return;
    }
    disk.Marshalling(data);
    int err = Remote()->SendRequest(NOTIFY_DISK_CREATED, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::NotifyDiskCreate, SendRequest failed");
        return;
    }
}

void StorageManagerProxy::NotifyDiskDestroyed(std::string diskId)
{
    LOGI("StorageManagerProxy::NotifyDiskDestroyed, diskId:%{public}s", diskId.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyDiskDestroyed, WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteString(diskId)) {
        LOGE("StorageManagerProxy::NotifyDiskDestroyed, WriteString failed");
        return;
    }
    int err = Remote()->SendRequest(NOTIFY_DISK_DESTROYED, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::NotifyDiskDestroyed, SendRequest failed");
        return;
    }
}

int32_t StorageManagerProxy::Partition(std::string diskId, int32_t type)
{
    LOGI("StorageManagerProxy::Partition, diskId:%{public}s", diskId.c_str());
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::Partition, WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteString(diskId)) {
        LOGE("StorageManagerProxy::Partition, WriteString failed");
        return E_IPC_ERROR;
    }
    if (!data.WriteInt32(type)) {
        LOGE("StorageManagerProxy::Partition WriteInt32 failed");
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(PARTITION, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::Partition, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt32();
}

std::vector<Disk> StorageManagerProxy::GetAllDisks()
{
    LOGI("StorageManagerProxy::GetAllDisks");
    std::vector<Disk> result = {};
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetAllDisks, WriteInterfaceToken failed");
        return result;
    }

    int err = Remote()->SendRequest(GET_ALL_DISKS, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetAllDisks, SendRequest failed");
        return result;
    }
    uint size = reply.ReadUint32();
    if (size == 0) {
        return result;
    }
    for (uint i = 0; i < size; i++) {
        std::unique_ptr<Disk> disk = Disk::Unmarshalling(reply);
        LOGI("StorageManagerProxy::GetAllDisks push %{public}s", disk->GetDiskId().c_str());
        result.push_back(*disk);
    }
    return result;
}

int64_t StorageManagerProxy::GetSystemSize()
{
    LOGI("StorageManagerProxy::GetSystemSize");
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetSystemSize WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(GET_SYSTEM_SIZE, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetSystemSize SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt64();
}

int64_t StorageManagerProxy::GetTotalSize()
{
    LOGI("StorageManagerProxy::GetTotalSize");
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetTotalSize WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(GET_TOTAL_SIZE, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetTotalSize SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt64();
}

int64_t StorageManagerProxy::GetFreeSize()
{
    LOGI("StorageManagerProxy::GetFreeSize");
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetFreeSize WriteInterfaceToken failed");
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(GET_FREE_SIZE, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::GetFreeSize SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadInt64();
}

StorageStats StorageManagerProxy::GetUserStorageStats()
{
    StorageStats result;
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return result;
    }

    int err = Remote()->SendRequest(GET_CURR_USER_STATS, data, reply, option);
    if (err != E_OK) {
        return result;
    }
    result = *StorageStats::Unmarshalling(reply);
    return result;
}

StorageStats StorageManagerProxy::GetUserStorageStats(int32_t userId)
{
    StorageStats result;
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return result;
    }

    if (!data.WriteInt32(userId)) {
        return result;
    }
    int err = Remote()->SendRequest(GET_USER_STATS, data, reply, option);
    if (err != E_OK) {
        return result;
    }
    result = *StorageStats::Unmarshalling(reply);
    return result;
}

BundleStats StorageManagerProxy::GetCurrentBundleStats()
{
    BundleStats result;
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return result;
    }

    int err = Remote()->SendRequest(GET_CURR_BUNDLE_STATS, data, reply, option);
    if (err != E_OK) {
        return result;
    }
    result = *BundleStats::Unmarshalling(reply);
    return result;
}

int32_t StorageManagerProxy::GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(fsUuid)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(GET_VOL_BY_UUID, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    } else {
        vc = *VolumeExternal::Unmarshalling(reply);
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetVolumeById(std::string volumeId, VolumeExternal &vc)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(volumeId)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(GET_VOL_BY_ID, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    } else {
        vc = *VolumeExternal::Unmarshalling(reply);
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::SetVolumeDescription(std::string fsUuid, std::string description)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(fsUuid)) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(description)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(SET_VOL_DESC, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }
    return reply.ReadInt32();
}
int32_t StorageManagerProxy::Format(std::string volumeId, std::string fsType)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(volumeId)) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(fsType)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(FORMAT, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }
    return reply.ReadInt32();
}
int32_t StorageManagerProxy::GetDiskById(std::string diskId, Disk &disk)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteString(diskId)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(GET_DISK_BY_ID, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    } else {
        disk = *Disk::Unmarshalling(reply);
    }
    return reply.ReadInt32();
}
} // StorageManager
} // OHOS
