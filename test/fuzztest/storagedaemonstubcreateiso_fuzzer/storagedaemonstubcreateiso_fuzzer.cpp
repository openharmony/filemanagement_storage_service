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
#include "storagedaemonstubcreateiso_fuzzer.h"

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

bool StorageDaemonCreateIsoImageFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int len = size / 8;
    u16string devPath(reinterpret_cast<const char16_t *>(data), len);
    u16string filePath(reinterpret_cast<const char16_t *>(data + len), len);
    u16string fsType(reinterpret_cast<const char16_t *>(data + len * 2), len);
    u16string mountPath(reinterpret_cast<const char16_t *>(data + len * 3), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(devPath);
    datas.WriteString16(filePath);
    datas.WriteString16(fsType);
    datas.WriteString16(mountPath);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_CREATE_ISO_IMAGE);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonBurnFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int len = size / 6;
    u16string devPath(reinterpret_cast<const char16_t *>(data), len);
    u16string burnOptions(reinterpret_cast<const char16_t *>(data + len), len);
    u16string fsType(reinterpret_cast<const char16_t *>(data + len * 2), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(devPath);
    datas.WriteString16(burnOptions);
    datas.WriteString16(fsType);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_BURN);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonGetVolumeOpProcessFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int len = size / 4;
    u16string volumeId(reinterpret_cast<const char16_t *>(data), len);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(volumeId);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_GET_VOLUME_OP_PROCESS);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StorageDaemonVerifyBurnDataFuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return true;
    }

    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int pos = 0;
    int len = size / 4;
    u16string devPath(reinterpret_cast<const char16_t *>(data), len);
    int32_t verType = TypeCast<int32_t>(data, &pos);

    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteString16(devPath);
    datas.WriteInt32(verType);
    datas.RewindRead(0);

    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_VERIFY_BURN_DATA);
    daemon->OnRemoteRequest(code, datas, reply, option);
    return true;
}

void StorageDaemonStubUncovered3FuzzTest(sptr<StorageDaemon::StorageDaemonProvider>& daemon,
    const uint8_t *data, size_t size)
{
    StorageDaemonCreateIsoImageFuzzTest(daemon, data, size);
    StorageDaemonBurnFuzzTest(daemon, data, size);
    StorageDaemonGetVolumeOpProcessFuzzTest(daemon, data, size);
    StorageDaemonVerifyBurnDataFuzzTest(daemon, data, size);
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    auto daemon = OHOS::sptr(new OHOS::StorageDaemon::StorageDaemonProvider());
    if (daemon != nullptr) {
        OHOS::StorageDaemonStubUncovered3FuzzTest(daemon, data, size);
    } else {
        printf("daemon is nullptr");
    }
    return 0;
}
