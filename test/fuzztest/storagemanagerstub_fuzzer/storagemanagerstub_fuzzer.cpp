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
#include "storagemanagerstub_fuzzer.h"

#include "ipc/storage_manager_provider.h"
#include "system_ability_definition.h"

namespace OHOS {
using namespace std;
template<typename T>
T TypeCast(const uint8_t *data, int *pos)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool StorageManagerOnRemoteRequestFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t * data,
    size_t size)
{
    const uint32_t maxCode = 88;
    for (uint32_t code = 1; code < maxCode; code++) {
        if (code == static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_UMOUNT_DIS_SHARE_FILE) ||
            code == static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_UMOUNT_FILE_MGR_FUSE)) {
            continue;
        }
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;

        datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
        datas.WriteBuffer(data, size);
        datas.RewindRead(0);

        manager->OnRemoteRequest(code, datas, reply, option);
    }
    return true;
}

bool StorageManagerUpdateUserAuthFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t *data,
    size_t size)
{
    if (data == nullptr ||
        size < sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos, &pos);
    int32_t tokenSize = 1;
    uint8_t value3 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t oldSecretSize = 1;
    uint8_t value4 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t newSecretSize = 1;
    uint8_t value5 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint64(secureUid);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value3);
    datas.WriteInt32(oldSecretSize);
    datas.WriteUint8(value4);
    datas.WriteInt32(newSecretSize);
    datas.WriteUint8(value5);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_UPDATE_USER_AUTH);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerActiveUserKeyFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t *data,
    size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    int32_t tokenSize = 1;
    uint8_t value6 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t secretSize = 1;
    uint8_t value7 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value6);
    datas.WriteInt32(secretSize);
    datas.WriteUint8(value7);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_ACTIVE_USER_KEY);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerUnlockUserKeyScreenFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    int32_t tokenSize = 1;
    uint8_t value8 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t secretSize = 1;
    uint8_t value9 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value8);
    datas.WriteInt32(secretSize);
    datas.WriteUint8(value9);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_UNLOCK_USER_SCREEN);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerCreateRecoveryKeyFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t *data,
    size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t userType = TypeCast<uint32_t>(data + pos, &pos);
    int32_t tokenSize = 1;
    uint8_t value10 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t secretSize = 1;
    uint8_t value11 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint32(userType);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value10);
    datas.WriteInt32(secretSize);
    datas.WriteUint8(value11);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_CREATE_RECOVER_KEY);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerSetRecoveryKeyFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t *data,
    size_t size)
{
    if (data == nullptr || size < sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    int32_t keySize = 1;
    uint8_t value12 = TypeCast<uint8_t>(data, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteInt32(keySize);
    datas.WriteUint8(value12);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_SET_RECOVER_KEY);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerUpdateUserAuthWithRecoverykeyFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager,
    const uint8_t *data, size_t size)
{
    if (data == nullptr ||
        size < sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    int32_t authTokenSize  = 1;
    uint8_t value14 = TypeCast<uint8_t>(data, &pos);
    int32_t newSecretSize = 1;
    uint8_t value15 = TypeCast<uint8_t>(data + pos, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos, &pos);
    uint32_t userId = TypeCast<uint32_t>(data + pos, &pos);
    int32_t plainTextSize = 1;
    int32_t value16Size = 1;
    uint8_t value17 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteInt32(authTokenSize);
    datas.WriteUint8(value14);
    datas.WriteInt32(newSecretSize);
    datas.WriteUint8(value15);
    datas.WriteUint64(secureUid);
    datas.WriteUint32(userId);
    datas.WriteInt32(plainTextSize);
    datas.WriteInt32(value16Size);
    datas.WriteUint8(value17);
    datas.RewindRead(0);

    uint32_t code =
        static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_UPDATE_USE_AUTH_WITH_RECOVERY_KEY);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerIsFileCopyedFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t *data,
    size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int32_t inputListSize = 1;
    int len = size / 4;
    u16string path(reinterpret_cast<const char16_t *>(data), len);
    u16string value18(reinterpret_cast<const char16_t *>(data + len + len), len);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteString16(path);
    datas.WriteInt32(inputListSize);
    datas.WriteString16(value18);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_IS_FILE_OCCUPIED);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageManagerResetSecretWithRecoveryKeyFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t rkType = TypeCast<uint32_t>(data + pos, &pos);
    int32_t keySize = 1;
    uint8_t value20 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageManager::StorageManagerStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint32(rkType);
    datas.WriteInt32(keySize);
    datas.WriteUint8(value20);
    datas.RewindRead(0);

    uint32_t code =
        static_cast<uint32_t>(StorageManager::IStorageManagerIpcCode::COMMAND_RESET_SECRET_WITH_RECOVERY_KEY);
    manager->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageManagerStubFuzzTest(sptr<StorageManager::StorageManagerProvider>& manager, const uint8_t *data, size_t size)
{
    StorageManagerOnRemoteRequestFuzzTest(manager, data, size);
    StorageManagerUpdateUserAuthFuzzTest(manager, data, size);
    StorageManagerActiveUserKeyFuzzTest(manager, data, size);
    StorageManagerUnlockUserKeyScreenFuzzTest(manager, data, size);
    StorageManagerCreateRecoveryKeyFuzzTest(manager, data, size);
    StorageManagerSetRecoveryKeyFuzzTest(manager, data, size);
    StorageManagerUpdateUserAuthWithRecoverykeyFuzzTest(manager, data, size);
    StorageManagerIsFileCopyedFuzzTest(manager, data, size);
    StorageManagerResetSecretWithRecoveryKeyFuzzTest(manager, data, size);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    auto manager = OHOS::sptr(new OHOS::StorageManager::StorageManagerProvider(OHOS::STORAGE_MANAGER_MANAGER_ID, true));
    if (manager != nullptr) {
        OHOS::StorageManagerStubFuzzTest(manager, data, size);
    } else {
        printf("manager is nullptr");
    }

    return 0;
}
