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
#include "storagedaemonproviderstubupdateuserauth_fuzzer.h"

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

bool StorageDaemonUpdateUserAuthFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data,
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
    uint8_t value1 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t oldSecretSize = 1;
    uint8_t value2 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t newSecretSize = 1;
    uint8_t value3 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteUint64(secureUid);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value1);
    datas.WriteInt32(oldSecretSize);
    datas.WriteUint8(value2);
    datas.WriteInt32(newSecretSize);
    datas.WriteUint8(value3);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UPDATE_USER_AUTH);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    StorageDaemonUpdateUserAuthFuzzTest(daemon, data, size);
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