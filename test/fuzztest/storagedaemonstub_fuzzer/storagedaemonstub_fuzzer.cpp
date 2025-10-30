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
#include "storagedaemonstub_fuzzer.h"

#include "ipc/storage_daemon_provider.h"
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

bool StorageDaemonCreateRecoveryKeyFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data,
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
    uint8_t value12 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t secretSize = 1;
    uint8_t value13 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint32(userType);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value12);
    datas.WriteInt32(secretSize);
    datas.WriteUint8(value13);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_CREATE_RECOVER_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonSetRecoveryKeyFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data,
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
    uint8_t value14 = TypeCast<uint8_t>(data, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteInt32(keySize);
    datas.WriteUint8(value14);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_SET_RECOVER_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonResetSecretWithRecoveryKeyFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
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
    uint8_t value16 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint32(rkType);
    datas.WriteInt32(keySize);
    datas.WriteUint8(value16);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_RESET_SECRET_WITH_RECOVERY_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonIsFileCopyedFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data,
    size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    int32_t inputListSize = 1;
    int len = size / 4;
    u16string path(reinterpret_cast<const char16_t *>(data + pos), len);
    u16string value17(reinterpret_cast<const char16_t *>(data + pos + len + len), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(path);
    datas.WriteInt32(inputListSize);
    datas.WriteString16(value17);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_IS_FILE_OCCUPIED);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CreateUserDirFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_CREATE_USER_DIR);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DeleteUserDirFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_DELETE_USER_DIR);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    StorageDaemonCreateRecoveryKeyFuzzTest(daemon, data, size);
    StorageDaemonSetRecoveryKeyFuzzTest(daemon, data, size);
    StorageDaemonResetSecretWithRecoveryKeyFuzzTest(daemon, data, size);
    StorageDaemonIsFileCopyedFuzzTest(daemon, data, size);
    CreateUserDirFuzzTest(daemon, data, size);
    DeleteUserDirFuzzTest(daemon, data, size);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    auto daemon = OHOS::sptr(new OHOS::StorageDaemon::StorageDaemonProvider());
    if (daemon != nullptr) {
        OHOS::StorageDaemonStubFuzzTest(daemon, data, size);
    } else {
        printf("daemon is nullptr");
    }

    return 0;
}
