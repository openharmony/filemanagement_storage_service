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

#include "ipc/storage_daemon_stub.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"

namespace OHOS {
namespace StorageDaemon {

int32_t StorageDaemonStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                           MessageParcel &reply, MessageOption &option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        return E_PERMISSION_DENIED;
    }

    LOGI("recv remote request code %{public}u", code);
    int err = E_OK;
    switch (code) {
        case SHUTDOWN:
            err = HandleShutdown();
            break;
        case CHECK:
            err = HandleCheck(data, reply);
            break;
        case MOUNT:
            err = HandleMount(data, reply);
            break;
        case UMOUNT:
            err =HandleUMount(data, reply);
            break;
        case PARTITION:
            err = HandlePartition(data, reply);
            break;
        case FORMAT:
            err = HandleFormat(data, reply);
            break;
        case SET_VOL_DESC:
            err = HandleSetVolDesc(data, reply);
            break;
        case PREPARE_USER_DIRS:
            err = HandlePrepareUserDirs(data, reply);
            break;
        case DESTROY_USER_DIRS:
            err = HandleDestroyUserDirs(data, reply);
            break;
        case START_USER:
            err = HandleStartUser(data,  reply);
            break;
        case STOP_USER:
            err = HandleStopUser(data, reply);
            break;
        case INIT_GLOBAL_KEY:
            err = HandleInitGlobalKey(data, reply);
            break;
        case INIT_GLOBAL_USER_KEYS:
            err = HandleInitGlobalUserKeys(data, reply);
            break;
        case CREATE_USER_KEYS:
            err = HandleGenerateUserKeys(data, reply);
            break;
        case DELETE_USER_KEYS:
            err = HandleDeleteUserKeys(data, reply);
            break;
        case UPDATE_USER_AUTH:
            err = HandleUpdateUserAuth(data, reply);
            break;
        case ACTIVE_USER_KEY:
            err = HandleActiveUserKey(data, reply);
            break;
        case INACTIVE_USER_KEY:
            err = HandleInactiveUserKey(data, reply);
            break;
        case UPDATE_KEY_CONTEXT:
            err = HandleUpdateKeyContext(data, reply);
            break;
        case CREATE_SHARE_FILE:
            err = HandleCreateShareFile(data, reply);
            break;
        case DELETE_SHARE_FILE:
            err = HandleDeleteShareFile(data, reply);
            break;
        default: {
            LOGI(" use IPCObjectStub default OnRemoteRequest");
            err = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }

    return err;
}

int32_t StorageDaemonStub::HandleShutdown()
{
    Shutdown();
    return E_OK;
}

int32_t StorageDaemonStub::HandleMount(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    uint32_t flags = data.ReadUint32();

    int err = Mount(volId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUMount(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();

    int err = UMount(volId);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleCheck(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();

    int err = Check(volId);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleFormat(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    std::string fsType = data.ReadString();

    int err = Format(volId, fsType);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandlePartition(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    int32_t type = data.ReadInt32();

    int err = Partition(volId, type);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleSetVolDesc(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    std::string description = data.ReadString();

    int err = SetVolumeDescription(volId, description);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandlePrepareUserDirs(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();

    int err = PrepareUserDirs(userId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDestroyUserDirs(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();

    int err = DestroyUserDirs(userId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleStartUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();

    int32_t err = StartUser(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleStopUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();

    int32_t err = StopUser(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInitGlobalKey(MessageParcel &data, MessageParcel &reply)
{
    int err = InitGlobalKey();
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInitGlobalUserKeys(MessageParcel &data, MessageParcel &reply)
{
    int err = InitGlobalUserKeys();
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t flags = data.ReadUint32();

    int err = GenerateUserKeys(userId, flags);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int err = DeleteUserKeys(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&oldSecret);
    data.ReadUInt8Vector(&newSecret);

    int err = UpdateUserAuth(userId, token, oldSecret, newSecret);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleActiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int err = ActiveUserKey(userId, token, secret);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int err = InactiveUserKey(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int err = UpdateKeyContext(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleCreateShareFile(MessageParcel &data, MessageParcel &reply)
{
    std::string uri = data.ReadString();
    uint32_t tokenId = data.ReadUint32();
    uint32_t flag = data.ReadUint32();
    int err = CreateShareFile(uri, tokenId, flag);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply)
{
    uint32_t tokenId = data.ReadUint32();
    std::vector<std::string> sharePathList;
    if (!data.ReadStringVector(&sharePathList)) {
        return E_WRITE_REPLY_ERR;
    }
    int err = DeleteShareFile(tokenId, sharePathList);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}
} // StorageDaemon
} // OHOS
