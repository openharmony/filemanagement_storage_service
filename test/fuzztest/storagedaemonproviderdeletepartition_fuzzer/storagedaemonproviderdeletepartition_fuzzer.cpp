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
#include "storagedaemonproviderdeletepartition_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_parcel.h"
#include "storage_daemon_provider.h"
#include "storage_daemon_stub.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {

std::shared_ptr<StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemonProvider>();

bool DeletePartitionFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_DELETE_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DeletePartitionFuzzTestWithParams(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t) + sizeof(uint8_t))) {
        return false;
    }

    // Create diskId from fuzzed data
    std::string diskId = "disk-8-";
    uint8_t minor = data[0];
    diskId += std::to_string(minor);

    // Extract partition number
    size_t offset = 1;
    uint32_t partitionNum = 0;
    if (offset + sizeof(uint32_t) <= size) {
        partitionNum = *reinterpret_cast<const uint32_t*>(data + offset);
    }

    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_DELETE_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteString(diskId);
    datas.WriteUint32(partitionNum);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DeletePartitionFuzzTestBoundary(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint8_t))) {
        return false;
    }

    // Test with various boundary partition numbers
    uint8_t minor = data[0];
    std::string diskId = "disk-8-" + std::to_string(minor);

    // Test edge cases: 0, 1, 15, 128, MAX_UINT32
    uint32_t boundaryValues[] = {0, 1, 15, 128, UINT32_MAX};
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_DELETE_PARTITION);

    for (uint32_t partitionNum : boundaryValues) {
        MessageParcel datas;
        datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
        datas.WriteString(diskId);
        datas.WriteUint32(partitionNum);
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;

        storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    }
    return true;
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DeletePartitionFuzzTest(data, size);
    OHOS::DeletePartitionFuzzTestWithParams(data, size);
    OHOS::DeletePartitionFuzzTestBoundary(data, size);
    return 0;
}
