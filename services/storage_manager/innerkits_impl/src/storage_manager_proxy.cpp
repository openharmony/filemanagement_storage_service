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

#include "storage_manager_proxy.h"
#include "hitrace_meter.h"
#include "storage_manager_ipc_interface_code.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_utils.h"

namespace OHOS {
namespace StorageManager {
int32_t StorageManagerProxy::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManagerProxy::PrepareAddUser, userId:%{public}d", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteInt32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUint32(flags)) {
        LOGE("StorageManagerProxy::PrepareAddUser, WriteUint32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_ADD_USER), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageManagerProxy::RemoveUser, userId:%{public}d", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::RemoveUser, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::RemoveUser, WriteInt32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUint32(flags)) {
        LOGE("StorageManagerProxy::RemoveUser, WriteUint32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::REMOVE_USER), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::PrepareStartUser(int32_t userId)
{
    LOGI("StorageManagerProxy::PrepareStartUser, userId:%{public}d", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::PrepareStartUser, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::PrepareStartUser, WriteInt32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_START_USER), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::StopUser(int32_t userId)
{
    LOGI("StorageManagerProxy::StopUser, userId:%{public}d", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::StopUser, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("StorageManagerProxy::StopUser, WriteInt32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::STOP_USER), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadUint32();
}

int32_t StorageManagerProxy::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("user ID: %{public}u, flags: %{public}u", userId, flags);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUint32(flags)) {
        LOGE("Write key flags failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::CREATE_USER_KEYS), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::DeleteUserKeys(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_USER_KEYS), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &oldSecret,
                                            const std::vector<uint8_t> &newSecret)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUint64(secureUid)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUInt8Vector(token)) {
        LOGE("Write token failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUInt8Vector(oldSecret)) {
        LOGE("Write oldSecret failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUInt8Vector(newSecret)) {
        LOGE("Write newSecret failed");
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::ActiveUserKey(uint32_t userId,
                                           const std::vector<uint8_t> &token,
                                           const std::vector<uint8_t> &secret)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUInt8Vector(token)) {
        LOGE("Write token failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteUInt8Vector(secret)) {
        LOGE("Write secret failed");
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::ACTIVE_USER_KEY), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::InactiveUserKey(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::INACTIVE_USER_KEY), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::LockUserScreen(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::LOCK_USER_SCREEN), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::UnlockUserScreen(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::UNLOCK_USER_SCREEN), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::LOCK_SCREEN_STATUS), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    lockScreenStatus = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::UpdateKeyContext(uint32_t userId)
{
    LOGI("user ID: %{public}u", userId);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(userId)) {
        LOGE("Write user ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_KEY_CONTEXT), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GenerateAppkey(uint32_t appUid, std::string &keyId)
{
    LOGI("appUid ID: %{public}u", appUid);
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteUint32(appUid)) {
        LOGE("Write appUid failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GENERATE_APP_KEY), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    keyId = reply.ReadString();
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::DeleteAppkey(const std::string keyId)
{
    LOGI("DeleteAppkey enter ");
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteString(keyId)) {
        LOGE("Write key ID failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_APP_KEY), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetFreeSizeOfVolume(std::string volumeUuid, int64_t &freeSize)
{
    LOGI("StorageManagerProxy::GetFreeSizeOfVolume, volumeUuid:%{public}s",
        GetAnonyString(volumeUuid).c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetFreeSizeOfVolume, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeUuid)) {
        LOGE("StorageManagerProxy::GetFreeSizeOfVolume, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_FREE), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    freeSize = reply.ReadInt64();
    return E_OK;
}

int32_t StorageManagerProxy::GetTotalSizeOfVolume(std::string volumeUuid, int64_t &totalSize)
{
    LOGI("StorageManagerProxy::GetTotalSizeOfVolume, volumeUuid:%{public}s",
        GetAnonyString(volumeUuid).c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetTotalSizeOfVolume, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeUuid)) {
        LOGE("StorageManagerProxy::GetTotalSizeOfVolume, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_TOTAL), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    totalSize = reply.ReadInt64();
    return E_OK;
}

int32_t StorageManagerProxy::GetBundleStats(std::string pkgName, BundleStats &bundleStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(pkgName)) {
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATUS), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    bundleStats = *BundleStats::Unmarshalling(reply);
    return E_OK;
}

int32_t StorageManagerProxy::NotifyVolumeCreated(VolumeCore vc)
{
    LOGI("StorageManagerProxy::NotifyVolumeCreated, volumeUuid:%{public}s",
        GetAnonyString(vc.GetId()).c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyVolumeCreated, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!vc.Marshalling(data)) {
        LOGE("StorageManagerProxy::NotifyVolumeCreated, WriteVolumeInfo failed");
        return E_WRITE_PARCEL_ERR;
    }

    return SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_CREATED), data, reply, option);
}

int32_t StorageManagerProxy::NotifyVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
    std::string path, std::string description)
{
    LOGI("StorageManagerProxy::NotifyVolumeMounted, volumeUuid:%{public}s",
        GetAnonyString(volumeId).c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteInt32(fsType)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteInt32 failed");
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(fsUuid)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(path)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(description)) {
        LOGE("StorageManagerProxy::NotifyVolumeMounted, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }

    return SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_MOUNTED), data, reply, option);
}

int32_t StorageManagerProxy::NotifyVolumeStateChanged(std::string volumeId, VolumeState state)
{
    LOGI("StorageManagerProxy::NotifyVolumeStateChanged, volumeId:%{public}s", volumeId.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyVolumeStateChanged, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::NotifyVolumeStateChanged, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteInt32(state)) {
        LOGE("StorageManagerProxy::NotifyVolumeStateChanged, WriteInt failed");
        return E_WRITE_PARCEL_ERR;
    }

    return SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_STATE_CHANGED), data, reply,
        option);
}

int32_t StorageManagerProxy::Mount(std::string volumeId)
{
    LOGI("StorageManagerProxy::Mount, volumeId:%{public}s", volumeId.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::Mount, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::Mount, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::MOUNT), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::Unmount(std::string volumeId)
{
    LOGI("StorageManagerProxy::Unmount, volumeId:%{public}s", volumeId.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::Unmount, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeId)) {
        LOGE("StorageManagerProxy::Unmount, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::UNMOUNT), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
    LOGI("StorageManagerProxy::GetAllVolumes");
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetAllVolumes, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_ALL_VOLUMES), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    uint size = reply.ReadUint32();
    if (size == 0) {
        return reply.ReadInt32();
    }
    for (uint i = 0; i < size; i++) {
        std::unique_ptr<VolumeExternal> ve = VolumeExternal::Unmarshalling(reply);
        LOGI("StorageManagerProxy::GetAllVolumes push %{public}s", ve->GetId().c_str());
        vecOfVol.push_back(*ve);
    }
    return E_OK;
}

int32_t StorageManagerProxy::NotifyDiskCreated(Disk disk)
{
    LOGI("StorageManagerProxy::NotifyDiskCreate, diskId:%{public}s", disk.GetDiskId().c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyDiskCreate, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!disk.Marshalling(data)) {
        LOGE("StorageManagerProxy::NotifyDiskCreate, WriteDiskInfo failed");
        return E_WRITE_PARCEL_ERR;
    }

    return SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_CREATED), data, reply, option);
}

int32_t StorageManagerProxy::NotifyDiskDestroyed(std::string diskId)
{
    LOGI("StorageManagerProxy::NotifyDiskDestroyed, diskId:%{public}s", diskId.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::NotifyDiskDestroyed, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteString(diskId)) {
        LOGE("StorageManagerProxy::NotifyDiskDestroyed, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }
    return SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_DESTROYED), data, reply, option);
}

int32_t StorageManagerProxy::Partition(std::string diskId, int32_t type)
{
    LOGI("StorageManagerProxy::Partition, diskId:%{public}s", diskId.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::Partition, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteString(diskId)) {
        LOGE("StorageManagerProxy::Partition, WriteString failed");
        return E_WRITE_PARCEL_ERR;
    }
    if (!data.WriteInt32(type)) {
        LOGE("StorageManagerProxy::Partition WriteInt32 failed");
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::PARTITION), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetAllDisks(std::vector<Disk> &vecOfDisk)
{
    LOGI("StorageManagerProxy::GetAllDisks");
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetAllDisks, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_ALL_DISKS), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    uint size = reply.ReadUint32();
    if (size == 0) {
        return reply.ReadInt32();
    }
    for (uint i = 0; i < size; i++) {
        std::unique_ptr<Disk> disk = Disk::Unmarshalling(reply);
        LOGI("StorageManagerProxy::GetAllDisks push %{public}s", disk->GetDiskId().c_str());
        vecOfDisk.push_back(*disk);
    }
    return E_OK;
}

int32_t StorageManagerProxy::GetSystemSize(int64_t &systemSize)
{
    LOGI("StorageManagerProxy::GetSystemSize");
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetSystemSize WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_SYSTEM_SIZE), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    systemSize = reply.ReadInt64();
    return E_OK;
}

int32_t StorageManagerProxy::GetTotalSize(int64_t &totalSize)
{
    LOGI("StorageManagerProxy::GetTotalSize");
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetTotalSize WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_TOTAL_SIZE), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    totalSize = reply.ReadInt64();
    return E_OK;
}

int32_t StorageManagerProxy::GetFreeSize(int64_t &freeSize)
{
    LOGI("StorageManagerProxy::GetFreeSize");
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::GetFreeSize WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_FREE_SIZE), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    freeSize = reply.ReadInt64();
    return E_OK;
}

int32_t StorageManagerProxy::GetUserStorageStats(StorageStats &storageStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    StorageStats result;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_CURR_USER_STATS), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    storageStats = *StorageStats::Unmarshalling(reply);
    return E_OK;
}

int32_t StorageManagerProxy::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    StorageStats result;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteInt32(userId)) {
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_STATS), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    storageStats = *StorageStats::Unmarshalling(reply);
    return E_OK;
}

int32_t StorageManagerProxy::GetCurrentBundleStats(BundleStats &bundleStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    BundleStats result;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_CURR_BUNDLE_STATS), data, reply,
        option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    bundleStats = *BundleStats::Unmarshalling(reply);
    return E_OK;
}

int32_t StorageManagerProxy::GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(fsUuid)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_VOL_BY_UUID), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    vc = *VolumeExternal::Unmarshalling(reply);
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetVolumeById(std::string volumeId, VolumeExternal &vc)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeId)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_VOL_BY_ID), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    vc = *VolumeExternal::Unmarshalling(reply);
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::SetVolumeDescription(std::string fsUuid, std::string description)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(fsUuid)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(description)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::SET_VOL_DESC), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::Format(std::string volumeId, std::string fsType)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(volumeId)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(fsType)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::FORMAT), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    return reply.ReadInt32();
}

int32_t StorageManagerProxy::GetDiskById(std::string diskId, Disk &disk)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(diskId)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_DISK_BY_ID), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    disk = *Disk::Unmarshalling(reply);
    return reply.ReadInt32();
}

std::vector<int32_t> StorageManagerProxy::CreateShareFile(const std::vector<std::string> &uriList,
                                                          uint32_t tokenId, uint32_t flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return std::vector<int32_t>{E_WRITE_DESCRIPTOR_ERR};
    }

    if (!data.WriteStringVector(uriList)) {
        return std::vector<int32_t>{E_WRITE_PARCEL_ERR};
    }

    if (!data.WriteUint32(tokenId)) {
        return std::vector<int32_t>{E_WRITE_PARCEL_ERR};
    }

    if (!data.WriteUint32(flag)) {
        return std::vector<int32_t>{E_WRITE_PARCEL_ERR};
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::CREATE_SHARE_FILE), data, reply, option);
    if (err != E_OK) {
        return std::vector<int32_t>{err};
    }

    std::vector<int32_t> retList;
    if (!reply.ReadInt32Vector(&retList)) {
        return std::vector<int32_t>{E_WRITE_PARCEL_ERR};
    };
    return retList;
}

int32_t StorageManagerProxy::DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &uriList)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteUint32(tokenId)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteStringVector(uriList)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_SHARE_FILE), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}
int32_t StorageManagerProxy::SetBundleQuota(const std::string &bundleName, int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteString(bundleName)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteInt32(uid)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(bundleDataDirPath)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteInt32(limitSizeMb)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::SET_BUNDLE_QUOTA), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}


int32_t StorageManagerProxy::GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
    const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteInt32(userId)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteStringVector(bundleNames)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteInt64Vector(incrementalBackTimes)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATS_INCREASE), data, reply,
        option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    if (!reply.ReadInt64Vector(&pkgFileSizes)) {
        LOGE("StorageManagerProxy::SendRequest read pkgFileSizes");
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageManagerProxy::UpdateMemoryPara(int32_t size, int32_t &oldSize)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteInt32(size)) {
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_MEM_PARA), data, reply, option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    oldSize = reply.ReadInt32();

    return E_OK;
}

int32_t StorageManagerProxy::SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr, code = %{public}d", code);
        return E_REMOTE_IS_NULLPTR;
    }

    int32_t result = remote->SendRequest(code, data, reply, option);
    if (result != E_OK) {
        LOGE("failed to SendRequest, code = %{public}d, result = %{public}d", code, result);
        return result;
    }

    return E_OK;
}

int32_t StorageManagerProxy::GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    StorageStats result;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        return E_WRITE_DESCRIPTOR_ERR;
    }

    if (!data.WriteInt32(userId)) {
        return E_WRITE_PARCEL_ERR;
    }

    if (!data.WriteString(type)) {
        return E_WRITE_PARCEL_ERR;
    }
    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_STATS_BY_TYPE), data, reply,
        option);
    if (err != E_OK) {
        return err;
    }
    err = reply.ReadInt32();
    if (err != E_OK) {
        return err;
    }
    storageStats = *StorageStats::Unmarshalling(reply);
    return E_OK;
}

int32_t StorageManagerProxy::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor())) {
        LOGE("StorageManagerProxy::MountDfsDocs, WriteInterfaceToken failed");
        return E_WRITE_DESCRIPTOR_ERR;
    }
    if (!data.WriteInt32(userId) || !data.WriteString(relativePath) ||
        !data.WriteString(networkId) || !data.WriteString(deviceId)) {
        LOGE("StorageManagerProxy::MountDfsDocs, Write failed");
        return E_WRITE_PARCEL_ERR;
    }

    int err = SendRequest(static_cast<int32_t>(StorageManagerInterfaceCode::MOUNT_DFS_DOCS), data, reply, option);
    if (err != E_OK) {
        return err;
    }

    return reply.ReadInt32();
}
} // StorageManager
} // OHOS
