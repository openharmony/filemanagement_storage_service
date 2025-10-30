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
#include "storagedaemonprovideronremoterequest_fuzzer.h"

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

bool StorageDaemonOnRemoteRequestFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data,
    size_t size)
{
    const uint32_t maxCode = 64;
    for (uint32_t code = 1; code < maxCode; code++) {
        if (code == static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_INIT_GLOBAL_USER_KEYS) ||
            code == static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UMOUNT_DIS_SHARE_FILE) ||
            code == static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_GET_OCCUPIED_SPACE) ||
            code == static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UMOUNT_FILE_MGR_FUSE) ||
            code == static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_QUERY_OCCUPIED_SPACE_FOR_SA)) {
            continue;
        }
        MessageParcel datas;
        MessageParcel reply;
        MessageOption option;

        datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
        datas.WriteBuffer(data, size);
        datas.RewindRead(0);

        daemon->OnRemoteRequest(code, datas, reply, option);
    }
    return true;
}

void StorageDaemonStubFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon, const uint8_t *data, size_t size)
{
    StorageDaemonOnRemoteRequestFuzzTest(daemon, data, size);
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