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
#include "storagedaemonproviderformatpartition_fuzzer.h"

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
constexpr size_t MAX_FS_TYPE_LENGTH = 16;
constexpr size_t MAX_VOLUME_NAME_LENGTH = 32;
constexpr uint32_t DEFAULT_PARTITION_NUM = 1;

std::shared_ptr<StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemonProvider>();

bool FormatPartitionFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_FORMAT_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool FormatPartitionFuzzTestWithOpts(const uint8_t *data, size_t size)
{
    // Minimum size: uint8_t(minor) + uint32_t(partitionNum) + extra data for fsType
    constexpr size_t minExtraDataSize = 4;
    if ((data == nullptr) || (size < sizeof(uint32_t) + sizeof(uint8_t) + minExtraDataSize)) {
        return false;
    }

    // Create diskId from fuzzed data
    std::string diskId = DISK_ID_PREFIX;
    uint8_t minor = data[0];
    diskId += std::to_string(minor);

    // Extract partition number
    size_t offset = 1;
    uint32_t partitionNum = 0;
    if (offset + sizeof(uint32_t) <= size) {
        partitionNum = *reinterpret_cast<const uint32_t*>(data + offset);
        offset += sizeof(uint32_t);
    }

    // Create FormatParams with fuzzed fs type
    FormatParams formatParams;
    if (offset < size) {
        size_t fsTypeLen = std::min(size - offset, MAX_FS_TYPE_LENGTH);
        std::string fsType(reinterpret_cast<const char*>(data + offset), fsTypeLen);
        formatParams.SetFsType(fsType);
        offset += fsTypeLen;

        // Try to extract volume name if available
        if (offset < size) {
            size_t volNameLen = std::min(size - offset, MAX_VOLUME_NAME_LENGTH);
            std::string volName(reinterpret_cast<const char*>(data + offset), volNameLen);
            formatParams.SetVolumeName(volName);
        }
    }

    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_FORMAT_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteString(diskId);
    datas.WriteUint32(partitionNum);
    formatParams.Marshalling(datas);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool FormatPartitionFuzzTestFSTypes(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint8_t))) {
        return false;
    }

    // Test with various file system types
    const char* fsTypes[] = {"vfat", "ext4", "exfat", "ntfs", "invalid", "", "\0\0\0", nullptr};
    constexpr size_t validFsTypeCount = 6; // Number of valid fs types to test (excluding "" and "\0\0\0")
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_FORMAT_PARTITION);

    uint8_t minor = data[0];
    std::string diskId = std::string(DISK_ID_PREFIX) + std::to_string(minor);
    uint32_t partitionNum = DEFAULT_PARTITION_NUM;

for (size_t i = 0; i < validFsTypeCount; i++) {
        FormatParams formatParams;
        if (fsTypes[i] != nullptr && strlen(fsTypes[i]) > 0) {
            formatParams.SetFsType(std::string(fsTypes[i]));
        }

        MessageParcel datas;
        datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
        datas.WriteString(diskId);
        datas.WriteUint32(partitionNum);
        formatParams.Marshalling(datas);
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
    OHOS::FormatPartitionFuzzTest(data, size);
    OHOS::FormatPartitionFuzzTestWithOpts(data, size);
    OHOS::FormatPartitionFuzzTestFSTypes(data, size);
    return 0;
}
