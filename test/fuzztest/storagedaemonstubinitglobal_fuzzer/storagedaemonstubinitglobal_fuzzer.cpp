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

    int pos = 0;
    int32_t idType = TypeCast<int32_t>(data, &pos);
    int32_t id = TypeCast<int32_t>(data + pos, &pos);

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

    int len = size / 4;
    u16string path(reinterpret_cast<const char16_t *>(data), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    int32_t userId = static_cast<int32_t>(size);
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

    int len = size / 4;
    u16string networkId(reinterpret_cast<const char16_t *>(data), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    int32_t userId = static_cast<int32_t>(size);
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
