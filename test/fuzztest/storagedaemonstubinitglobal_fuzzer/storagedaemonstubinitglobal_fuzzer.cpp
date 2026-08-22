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
#include "storagedaemonstubinitglobal_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "ipc/storage_daemon_provider.h"
#include "system_ability_definition.h"

namespace OHOS {
using namespace std;

bool StorageDaemonInitGlobalUserKeysFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_INIT_GLOBAL_USER_KEYS);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonGetOccupiedSpaceFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(int32_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    FuzzedDataProvider provider(data, size);
    int32_t idType = provider.ConsumeIntegral<int32_t>();
    int32_t id = provider.ConsumeIntegral<int32_t>();

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteInt32(idType);
    datas.WriteInt32(id);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_GET_OCCUPIED_SPACE);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonUMountFileMgrFuseFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    FuzzedDataProvider provider(data, size);
    string str = provider.ConsumeRandomLengthString();
    u16string path(reinterpret_cast<const char16_t*>(str.c_str()), str.size());

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    int32_t userId = provider.ConsumeIntegral<int32_t>();
    datas.WriteInt32(userId);
    datas.WriteString16(path);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UMOUNT_FILE_MGR_FUSE);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonUMountDisShareFileFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    FuzzedDataProvider provider(data, size);
    string str = provider.ConsumeRandomLengthString();
    u16string networkId(reinterpret_cast<const char16_t*>(str.c_str()), str.size());

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    int32_t userId = provider.ConsumeIntegral<int32_t>();
    datas.WriteInt32(userId);
    datas.WriteString16(networkId);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_UMOUNT_DIS_SHARE_FILE);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubUncovered1FuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    StorageDaemonInitGlobalUserKeysFuzzTest(daemon, data, size);
    StorageDaemonGetOccupiedSpaceFuzzTest(daemon, data, size);
    StorageDaemonUMountFileMgrFuseFuzzTest(daemon, data, size);
    StorageDaemonUMountDisShareFileFuzzTest(daemon, data, size);
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    auto daemon = OHOS::sptr(new OHOS::StorageDaemon::StorageDaemonProvider());
    if (daemon != nullptr) {
        OHOS::StorageDaemonStubUncovered1FuzzTest(daemon, data, size);
    } else {
        printf("daemon is nullptr");
    }
    return 0;
}
