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
#include "utils/storage_manager_errno.h" 
#include "utils/storage_manager_log.h"

namespace OHOS {
namespace StorageManager {

int32_t StorageManagerStub::OnRemoteRequest(uint32_t code, 
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
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
    LOGI("StorageManagerStub::HandlePrepareAddUser, userId:%{public}d", userId);
    int err = PrepareAddUser(userId);
    if (!reply.WriteUint32(err)) {
        LOGE("StorageManagerStub::HandlePrepareAddUser call PrepareAddUser failed");
        return  E_IPC_ERROR;
    }
    return E_OK;
}

int32_t StorageManagerStub::HandleRemoveUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    LOGI("StorageManagerStub::HandleRemoveUser, userId:%{public}d", userId);
    int err = RemoveUser(userId);
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
} // StorageManager
} // OHOS