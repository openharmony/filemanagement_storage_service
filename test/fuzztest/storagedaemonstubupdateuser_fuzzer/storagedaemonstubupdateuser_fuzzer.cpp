/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "storagedaemonstubupdateuser_fuzzer.h"

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

static const int32_t VECTOR_OVERFLOW_SIZE = 200000;

bool StorageDaemonUpdateUserAuthOverflowFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint64_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint64(secureUid);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UPDATE_USER_AUTH);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonActiveUserKeyOverflowFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_ACTIVE_USER_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonUnlockUserScreenOverflowFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t user = TypeCast<uint32_t>(data, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(user);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UNLOCK_USER_SCREEN);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonCreateRecoverKeyOverflowFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t userType = TypeCast<uint32_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint32(userType);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.WriteInt32(VECTOR_OVERFLOW_SIZE);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_CREATE_RECOVER_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubUncovered4FuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    StorageDaemonUpdateUserAuthOverflowFuzzTest(daemon, data, size);
    StorageDaemonActiveUserKeyOverflowFuzzTest(daemon, data, size);
    StorageDaemonUnlockUserScreenOverflowFuzzTest(daemon, data, size);
    StorageDaemonCreateRecoverKeyOverflowFuzzTest(daemon, data, size);
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    auto daemon = OHOS::sptr(new OHOS::StorageDaemon::StorageDaemonProvider());
    if (daemon != nullptr) {
        OHOS::StorageDaemonStubUncovered4FuzzTest(daemon, data, size);
    } else {
        printf("daemon is nullptr");
    }
    return 0;
}
