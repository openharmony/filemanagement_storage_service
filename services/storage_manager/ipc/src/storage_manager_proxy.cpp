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
#include "utils/storage_manager_errno.h"
#include "utils/storage_manager_log.h"

namespace OHOS {
namespace StorageManager {

int32_t StorageManagerProxy::PrepareAddUser(int32_t userId)
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
    int err = Remote()->SendRequest(PREPARE_ADD_USER, data, reply, option);
    if (err != E_OK) {
        LOGE("StorageManagerProxy::PrepareAddUser, SendRequest failed");
        return E_IPC_ERROR;
    }
    return reply.ReadUint32();
}
    
int32_t StorageManagerProxy::RemoveUser(int32_t userId)
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
} // StorageManager
} // OHOS

