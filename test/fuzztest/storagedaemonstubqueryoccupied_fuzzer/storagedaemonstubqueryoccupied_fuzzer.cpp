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
#include "storagedaemonstubqueryoccupied_fuzzer.h"

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

bool StorageDaemonQueryOccupiedSpaceForSaFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int32_t bundleNameAndUidSize = 1;

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteInt32(bundleNameAndUidSize);
    int32_t key = static_cast<int32_t>(size);
    datas.WriteInt32(key);
    int len = size / 4;
    u16string value(reinterpret_cast<const char16_t *>(data), len);
    datas.WriteString16(value);
    int32_t type = static_cast<int32_t>(size);
    datas.WriteInt32(type);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_QUERY_OCCUPIED_SPACE_FOR_SA);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonGetDataSizeByPathFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int len = size / 4;
    u16string path(reinterpret_cast<const char16_t *>(data), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(path);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_GET_DATA_SIZE_BY_PATH);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonGetRmgResourceSizeFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int len = size / 4;
    u16string rgmName(reinterpret_cast<const char16_t *>(data), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(rgmName);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_GET_RMG_RESOURCE_SIZE);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonListUserdataDirInfoFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_LIST_USERDATA_DIR_INFO);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubUncovered2FuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    StorageDaemonQueryOccupiedSpaceForSaFuzzTest(daemon, data, size);
    StorageDaemonGetDataSizeByPathFuzzTest(daemon, data, size);
    StorageDaemonGetRmgResourceSizeFuzzTest(daemon, data, size);
    StorageDaemonListUserdataDirInfoFuzzTest(daemon, data, size);
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    auto daemon = OHOS::sptr(new OHOS::StorageDaemon::StorageDaemonProvider());
    if (daemon != nullptr) {
        OHOS::StorageDaemonStubUncovered2FuzzTest(daemon, data, size);
    } else {
        printf("daemon is nullptr");
    }
    return 0;
}
