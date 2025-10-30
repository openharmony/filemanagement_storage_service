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
#include "storagedaemonproviderstubactiveuserkey_fuzzer.h"

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

bool StorageDaemonActiveUserKeyFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data,
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
    uint8_t value8 = TypeCast<uint8_t>(data + pos, &pos);
    int32_t secretSize = 1;
    uint8_t value9 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteUint32(userId);
    datas.WriteInt32(tokenSize);
    datas.WriteUint8(value8);
    datas.WriteInt32(secretSize);
    datas.WriteUint8(value9);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_ACTIVE_USER_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    StorageDaemonActiveUserKeyFuzzTest(daemon, data, size);
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