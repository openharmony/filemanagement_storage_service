/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storagedaemon_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "storage_daemon.h"
#include "storage_daemon_ipc_interface_code.h"
#include "storage_daemon_stub.h"
#include "token_setproc.h"
#include "user_manager.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr uint8_t MAX_CALL_TRANSACTION = 32;
constexpr size_t U32_AT_SIZE = 4;

std::shared_ptr<StorageDaemon::StorageDaemon> storageDaemon = std::make_shared<StorageDaemon::StorageDaemon>();
std::shared_ptr<StorageDaemon::UserManager> userManager = StorageDaemon::UserManager::GetInstance();
uint32_t GetU32Data(const char *ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

enum {
    TOKEN_INDEX_ONE = 0,
    TOKEN_INDEX_TWO,
    TOKEN_INDEX_THREE,
};

void SetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[3];
    perms[TOKEN_INDEX_ONE] = "ohos.permission.STORAGE_MANAGER";
    perms[TOKEN_INDEX_TWO] = "ohos.permission.MOUNT_UNMOUNT_MANAGER";
    perms[TOKEN_INDEX_THREE] = "ohos.permission.MOUNT_FORMAT_MANAGER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "StorageDaemonFuzzTest";
    tokenId = GetAccessTokenId(&infoInstance);
    const uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    tokenId |= systemAppMask;
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

bool StorageDaemonFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    SetNativeToken();
    uint32_t code = GetU32Data(data.get());
    if (code == 0 ||
        code % MAX_CALL_TRANSACTION == static_cast<int32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE) ||
        code % MAX_CALL_TRANSACTION == static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE)) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageDaemon->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);

    return true;
}

bool HandleStartUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleStartUser(datas, reply);
    return true;
}

bool HandleStopUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleStopUser(datas, reply);
    return true;
}

bool HandlePrepareUserDirsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandlePrepareUserDirs(datas, reply);
    return true;
}

bool HandleDestroyUserDirsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleDestroyUserDirs(datas, reply);
    return true;
}

bool HandleMountFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleMount(datas, reply);
    return true;
}

bool HandleUMountFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleUMount(datas, reply);
    return true;
}

bool HandlePartitionFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandlePartition(datas, reply);
    return true;
}

bool HandleCheckFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleCheck(datas, reply);
    return true;
}

bool HandleSetVolDescFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleSetVolDesc(datas, reply);
    return true;
}

bool HandleFormatFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleFormat(datas, reply);
    return true;
}

bool HandleSetBundleQuotaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleSetBundleQuota(datas, reply);
    return true;
}

bool HandleGenerateUserKeysFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleGenerateUserKeys(datas, reply);
    return true;
}

bool HandleDeleteUserKeysFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleDeleteUserKeys(datas, reply);
    return true;
}

bool HandleUpdateUserAuthFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleUpdateUserAuth(datas, reply);
    return true;
}

bool HandleActiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleActiveUserKey(datas, reply);
    return true;
}

bool HandleInactiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleInactiveUserKey(datas, reply);
    return true;
}

bool HandleUpdateKeyContextFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleUpdateKeyContext(datas, reply);
    return true;
}

bool HandleDeleteAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleDeleteAppkey(datas, reply);
    return true;
}

bool HandleGenerateAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleGenerateAppkey(datas, reply);
    return true;
}

bool HandleUnlockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleUnlockUserScreen(datas, reply);
    return true;
}

bool HandleLockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleLockUserScreen(datas, reply);
    return true;
}

bool HandleUpdateMemoryParaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleUpdateMemoryPara(datas, reply);
    return true;
}

bool HandleShutdownFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandleShutdown(datas, reply);
    return true;
}

bool UserManagerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }

    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    uint32_t flag = *(reinterpret_cast<const uint32_t *>(data));
    userManager->PrepareUserDirs(userId, flag);
    userManager->DestroyUserDirs(userId, flag);
    userManager->StartUser(userId);
    userManager->StopUser(userId);
    userManager->CreateBundleDataDir(flag);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }
    OHOS::StorageDaemonFuzzTest(move(str), size);
    OHOS::UserManagerFuzzTest(data, size);
    OHOS::HandleStartUserFuzzTest(data, size);
    OHOS::HandleStopUserFuzzTest(data, size);
    OHOS::HandlePrepareUserDirsFuzzTest(data, size);
    OHOS::HandleDestroyUserDirsFuzzTest(data, size);
    OHOS::HandleMountFuzzTest(data, size);
    OHOS::HandleUMountFuzzTest(data, size);
    OHOS::HandlePartitionFuzzTest(data, size);
    OHOS::HandleCheckFuzzTest(data, size);
    OHOS::HandleSetVolDescFuzzTest(data, size);
    OHOS::HandleFormatFuzzTest(data, size);
    OHOS::HandleSetBundleQuotaFuzzTest(data, size);
    OHOS::HandleGenerateUserKeysFuzzTest(data, size);
    OHOS::HandleDeleteUserKeysFuzzTest(data, size);
    OHOS::HandleUpdateUserAuthFuzzTest(data, size);
    OHOS::HandleActiveUserKeyFuzzTest(data, size);
    OHOS::HandleInactiveUserKeyFuzzTest(data, size);
    OHOS::HandleUpdateKeyContextFuzzTest(data, size);
    OHOS::HandleDeleteAppkeyFuzzTest(data, size);
    OHOS::HandleGenerateAppkeyFuzzTest(data, size);
    OHOS::HandleUnlockUserScreenFuzzTest(data, size);
    OHOS::HandleLockUserScreenFuzzTest(data, size);
    OHOS::HandleUpdateMemoryParaFuzzTest(data, size);
    OHOS::HandleShutdownFuzzTest(data, size);
    return 0;
}
