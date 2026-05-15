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
#include "storagedaemonprovidercreatepartition_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_parcel.h"
#include "storage_daemon_provider.h"
#include "storage_daemon_stub.h"

using namespace OHOS::StorageDaemon;
using namespace OHOS::StorageManager;

namespace OHOS {

// Test constants
constexpr const char* DISK_ID_PREFIX = "disk-8-";

std::shared_ptr<StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemonProvider>();

bool CreatePartitionFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_CREATE_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CreatePartitionFuzzTestWithOpts(const uint8_t *data, size_t size)
{
    // Minimum required size: partitionNum(4) + startSector(8) + endSector(8) + at least 1 char for typeCode
    constexpr size_t minTypeCodeChars = 1;
    constexpr size_t minDataSize = sizeof(int32_t) + sizeof(uint64_t) * 2 + minTypeCodeChars;
    // Maximum length for partition type code string
    constexpr size_t maxTypeCodeLength = 32;
    if ((data == nullptr) || (size < minDataSize)) {
        return false;
    }

    // Parse fuzzed data to create PartitionOptions
    PartitionOptions options;
    size_t offset = 0;

    // Extract partition num
    if (offset + sizeof(int32_t) <= size) {
        int32_t partitionNum = *reinterpret_cast<const int32_t*>(data + offset);
        options.SetPartitionNum(partitionNum);
        offset += sizeof(int32_t);
    }

    // Extract start sector
    if (offset + sizeof(uint64_t) <= size) {
        uint64_t startSector = *reinterpret_cast<const uint64_t*>(data + offset);
        options.SetStartSector(startSector);
        offset += sizeof(uint64_t);
    }

    // Extract end sector
    if (offset + sizeof(uint64_t) <= size) {
        uint64_t endSector = *reinterpret_cast<const uint64_t*>(data + offset);
        options.SetEndSector(endSector);
        offset += sizeof(uint64_t);
    }

    // Extract type code (max maxTypeCodeLength chars)
    if (offset < size) {
        size_t typeCodeLen = std::min(size - offset, maxTypeCodeLength);
        std::string typeCode(reinterpret_cast<const char*>(data + offset), typeCodeLen);
        options.SetTypeCode(typeCode);
    }

    // Create diskId from fuzzed data
    std::string diskId = DISK_ID_PREFIX;
    if (size >= sizeof(uint8_t)) {
        uint8_t minor = data[0];
        diskId += std::to_string(minor);
    }

    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_CREATE_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteString(diskId);
    options.Marshalling(datas);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::CreatePartitionFuzzTest(data, size);
    OHOS::CreatePartitionFuzzTestWithOpts(data, size);
    return 0;
}
