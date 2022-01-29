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

#include "ipc/storage_daemon_proxy.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
StorageDaemonProxy::StorageDaemonProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IStorageDaemon>(impl)
{}

int32_t StorageDaemonProxy::Shutdown()
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    return Remote()->SendRequest(SHUTDOWN, data, reply, option);
}

int32_t StorageDaemonProxy::Mount(std::string volId, uint32_t flags)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UMount(std::string volId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Check(std::string volId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Format(std::string voldId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteInt32(userId)) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(flags)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(PREPARE_USER_DIRS, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteInt32(userId)) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(flags)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(DESTROY_USER_DIRS, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::StartUser(int32_t userId)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteInt32(userId)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(START_USER, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::StopUser(int32_t userId)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteInt32(userId)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(STOP_USER, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::InitGlobalKey(void)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(INIT_GLOBAL_KEY, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::InitGlobalUserKeys(void)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);
    LOGI("start");
    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(INIT_GLOBAL_USER_KEYS, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    LOGI("start");
    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(userId)) {
        return E_IPC_ERROR;
    }
    if (!data.WriteUint32(flags)) {
        return E_IPC_ERROR;
    }

    int err = Remote()->SendRequest(CREATE_USER_KEYS, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadUint32();
}

int32_t StorageDaemonProxy::DeleteUserKeys(uint32_t userId)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(userId)) {
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(DELETE_USER_KEYS, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

int32_t StorageDaemonProxy::UpdateUserAuth(uint32_t userId, std::string auth, std::string compSecret)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(userId)) {
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(UPDATE_USER_AUTH, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}

 int32_t StorageDaemonProxy::ActiveUserKey(uint32_t userId, std::string auth, std::string compSecret)
 {
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(userId)) {
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(ACTIVE_USER_KEY, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
 }

int32_t StorageDaemonProxy::InactiveUserKey(uint32_t userId)
{
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor())) {
        return E_IPC_ERROR;
    }

    if (!data.WriteUint32(userId)) {
        return E_IPC_ERROR;
    }
    int err = Remote()->SendRequest(INACTIVE_USER_KEY, data, reply, option);
    if (err != E_OK) {
        return E_IPC_ERROR;
    }

    return reply.ReadInt32();
}
} // StorageDaemon
} // OHOS