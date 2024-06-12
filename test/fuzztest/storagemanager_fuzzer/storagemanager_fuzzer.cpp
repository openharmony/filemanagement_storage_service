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
#include "storagemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_parcel.h"
#include "storage_manager_stub.h"
#include "storage_manager.h"
#include "securec.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"

using namespace OHOS::StorageManager;

namespace OHOS::StorageManager {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr uint8_t MAX_CALL_TRANSACTION = 64;
constexpr size_t U32_AT_SIZE = 4;
constexpr int32_t SERVICE_ID = 5003;

std::shared_ptr<StorageManager> storageManagerPtr =
    std::make_shared<StorageManager>(SERVICE_ID, true);

uint32_t GetU32Data(const char* ptr)
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

    infoInstance.processName = "StorageManagerFuzzTest";
    tokenId = GetAccessTokenId(&infoInstance);
    const uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    tokenId |= systemAppMask;
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

bool StorageManagerFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    SetNativeToken();
    uint32_t code = GetU32Data(data.get());
    if (code == 0) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageManagerPtr->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);

    return true;
}

bool HandlePrepareAddUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandlePrepareAddUser(datas, reply);
    return true;
}

bool HandleRemoveUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleRemoveUser(datas, reply);
    return true;
}

bool HandlePrepareStartUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandlePrepareStartUser(datas, reply);
    return true;
}
bool HandleStopUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleStopUser(datas, reply);
    return true;
}
bool HandleGetCurrentBundleStatsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleGetCurrentBundleStats(datas, reply);
    return true;
}

bool HandleMountFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleMount(datas, reply);
    return true;
}

bool HandleUnmountFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleUnmount(datas, reply);
    return true;
}

bool HandlePartitionFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandlePartition(datas, reply);
    return true;
}

bool HandleFormatFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleFormat(datas, reply);
    return true;
}

bool HandleGenerateUserKeysFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleGenerateUserKeys(datas, reply);
    return true;
}

bool HandleDeleteUserKeysFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleDeleteUserKeys(datas, reply);
    return true;
}

bool HandleUpdateUserAuthFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleUpdateUserAuth(datas, reply);
    return true;
}

bool HandleActiveUserKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleActiveUserKey(datas, reply);
    return true;
}

bool HandleInactiveUserKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleInactiveUserKey(datas, reply);
    return true;
}

bool HandleLockUserScreenFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleLockUserScreen(datas, reply);
    return true;
}

bool HandleUnlockUserScreenFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleUnlockUserScreen(datas, reply);
    return true;
}

bool HandleUpdateKeyContextFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleUpdateKeyContext(datas, reply);
    return true;
}

bool HandleCreateShareFileFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleCreateShareFile(datas, reply);
    return true;
}

bool HandleDeleteShareFileFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleDeleteShareFile(datas, reply);
    return true;
}

bool HandleSetBundleQuotaFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;

    storageManagerPtr->HandleSetBundleQuota(datas, reply);
    return true;
}
} // namespace OHOS::StorageManager

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::StorageManager::U32_AT_SIZE || size > OHOS::StorageManager::FOO_MAX_LEN) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }

    OHOS::StorageManager::StorageManagerFuzzTest(move(str), size);
    OHOS::StorageManager::HandlePrepareAddUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandleRemoveUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandlePrepareStartUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandleStopUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandleGetCurrentBundleStatsFuzzTest(move(str), size);
    OHOS::StorageManager::HandleMountFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUnmountFuzzTest(move(str), size);
    OHOS::StorageManager::HandlePartitionFuzzTest(move(str), size);
    OHOS::StorageManager::HandleFormatFuzzTest(move(str), size);
    OHOS::StorageManager::HandleGenerateUserKeysFuzzTest(move(str), size);
    OHOS::StorageManager::HandleDeleteUserKeysFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUpdateUserAuthFuzzTest(move(str), size);
    OHOS::StorageManager::HandleActiveUserKeyFuzzTest(move(str), size);
    OHOS::StorageManager::HandleInactiveUserKeyFuzzTest(move(str), size);
    OHOS::StorageManager::HandleLockUserScreenFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUnlockUserScreenFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUpdateKeyContextFuzzTest(move(str), size);
    OHOS::StorageManager::HandleCreateShareFileFuzzTest(move(str), size);
    OHOS::StorageManager::HandleDeleteShareFileFuzzTest(move(str), size);
    OHOS::StorageManager::HandleSetBundleQuotaFuzzTest(move(str), size);
    return 0;
}
