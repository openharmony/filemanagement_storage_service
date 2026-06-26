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
#include "storagemanagerprovidercreatepartition_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "ipc/storage_manager_provider.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "partition_params.h"
#include "securec.h"
#include "storage_manager_stub.h"

using namespace OHOS::StorageManager;
namespace OHOS::StorageManager {
std::shared_ptr<StorageManagerProvider> storageManagerProvider =
    std::make_shared<StorageManagerProvider>(STORAGE_MANAGER_MANAGER_ID);

// ipccode 108: CreatePartition([in] String diskId, [in] PartitionParams) — IStorageManager.idl
constexpr uint32_t CODE_CREATE_PARTITION = 108;
constexpr const char *DISK_ID_PREFIX = "disk-8-";
constexpr size_t MAX_TYPE_CODE_LENGTH = 32;

bool CreatePartitionFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(CODE_CREATE_PARTITION, datas, reply, option);
    return true;
}

bool CreatePartitionFuzzTestWithOpts(const uint8_t *data, size_t size)
{
    // partitionNum(4) + startSector(8) + endSector(8) + at least 1 char for typeCode
    constexpr size_t minTypeCodeChars = 1;
    constexpr size_t minDataSize = sizeof(int32_t) + sizeof(uint64_t) * 2 + minTypeCodeChars;
    if ((data == nullptr) || (size < minDataSize)) {
        return false;
    }

    PartitionParams partitionParams;
    size_t offset = 0;

    if (offset + sizeof(int32_t) <= size) {
        int32_t partitionNum = *reinterpret_cast<const int32_t *>(data + offset);
        partitionParams.SetPartitionNum(partitionNum);
        offset += sizeof(int32_t);
    }

    if (offset + sizeof(uint64_t) <= size) {
        uint64_t startSector = *reinterpret_cast<const uint64_t *>(data + offset);
        partitionParams.SetStartSector(startSector);
        offset += sizeof(uint64_t);
    }

    if (offset + sizeof(uint64_t) <= size) {
        uint64_t endSector = *reinterpret_cast<const uint64_t *>(data + offset);
        partitionParams.SetEndSector(endSector);
        offset += sizeof(uint64_t);
    }

    if (offset < size) {
        size_t typeCodeLen = std::min(size - offset, MAX_TYPE_CODE_LENGTH);
        std::string typeCode(reinterpret_cast<const char *>(data + offset), typeCodeLen);
        partitionParams.SetTypeCode(typeCode);
    }

    std::string diskId = DISK_ID_PREFIX;
    uint8_t minor = data[0];
    diskId += std::to_string(minor);

    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteString(diskId);
    partitionParams.Marshalling(datas);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(CODE_CREATE_PARTITION, datas, reply, option);
    return true;
}
} // namespace OHOS::StorageManager

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::StorageManager::CreatePartitionFuzzTest(data, size);
    OHOS::StorageManager::CreatePartitionFuzzTestWithOpts(data, size);
    return 0;
}
