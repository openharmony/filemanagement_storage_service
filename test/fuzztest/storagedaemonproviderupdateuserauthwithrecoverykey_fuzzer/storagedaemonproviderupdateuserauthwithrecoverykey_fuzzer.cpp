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
#include "storagedaemonproviderupdateuserauthwithrecoverykey_fuzzer.h"

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

bool StorageDaemonUpdateUserAuthWithRecoverykeyFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
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
    uint8_t value4 = TypeCast<uint8_t>(data, &pos);
    int32_t newSecretSize = 1;
    uint8_t value5 = TypeCast<uint8_t>(data + pos, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos, &pos);
    uint32_t userId = TypeCast<uint32_t>(data + pos, &pos);
    int32_t plainTextSize = 1;
    int32_t value6Size = 1;
    uint8_t value7 = TypeCast<uint8_t>(data + pos, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteInt32(authTokenSize);
    datas.WriteUint8(value4);
    datas.WriteInt32(newSecretSize);
    datas.WriteUint8(value5);
    datas.WriteUint64(secureUid);
    datas.WriteUint32(userId);
    datas.WriteInt32(plainTextSize);
    datas.WriteInt32(value6Size);
    datas.WriteUint8(value7);
    datas.RewindRead(0);

    uint32_t code =
        static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UPDATE_USE_AUTH_WITH_RECOVERY_KEY);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    StorageDaemonUpdateUserAuthWithRecoverykeyFuzzTest(daemon, data, size);
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