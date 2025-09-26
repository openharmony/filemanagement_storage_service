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
#include "storagedaemonprovider_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_parcel.h"
#include "storage_daemon.h"
#include "storage_daemon_provider.h"
#include "storage_daemon_stub.h"
#include "user_manager.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {

std::shared_ptr<StorageDaemon::StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemon::StorageDaemonProvider>();
StorageDaemon::UserManager &userManager = StorageDaemon::UserManager::GetInstance();

bool StartUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_START_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StopUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_STOP_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool PrepareUserDirsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_PREPARE_USER_DIRS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DestroyUserDirsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_DESTROY_USER_DIRS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool MountFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_MOUNT);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UMountFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_UMOUNT);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool PartitionFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CheckFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_CHECK);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool SetVolDescFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_SET_VOLUME_DESCRIPTION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool QueryUsbIsInUseFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_QUERY_USB_IS_IN_USE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool InactiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_INACTIVE_USER_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UpdateKeyContextFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_UPDATE_KEY_CONTEXT);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DeleteAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_DELETE_APPKEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GenerateAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_GENERATE_APPKEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UnlockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_UNLOCK_USER_SCREEN);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UserManagerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(uint32_t))) {
        return false;
    }

    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    uint32_t flag = *(reinterpret_cast<const uint32_t *>(data + sizeof(uint32_t)));
    userManager.PrepareUserDirs(userId, flag);
    userManager.DestroyUserDirs(userId, flag);
    userManager.StartUser(userId);
    userManager.StopUser(userId);
    userManager.CreateElxBundleDataDir(userId, static_cast<uint8_t>(flag));

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::UserManagerFuzzTest(data, size);
    OHOS::StartUserFuzzTest(data, size);
    OHOS::StopUserFuzzTest(data, size);
    OHOS::PrepareUserDirsFuzzTest(data, size);
    OHOS::DestroyUserDirsFuzzTest(data, size);
    OHOS::MountFuzzTest(data, size);
    OHOS::UMountFuzzTest(data, size);
    OHOS::PartitionFuzzTest(data, size);
    OHOS::CheckFuzzTest(data, size);
    OHOS::SetVolDescFuzzTest(data, size);
    OHOS::QueryUsbIsInUseFuzzTest(data, size);
    OHOS::InactiveUserKeyFuzzTest(data, size);
    OHOS::UpdateKeyContextFuzzTest(data, size);
    OHOS::DeleteAppkeyFuzzTest(data, size);
    OHOS::GenerateAppkeyFuzzTest(data, size);
    OHOS::UnlockUserScreenFuzzTest(data, size);
    return 0;
}
