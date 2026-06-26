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
#include "storagemanagerproviderdeletepartition_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "ipc/storage_manager_provider.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "securec.h"
#include "storage_manager_stub.h"

using namespace OHOS::StorageManager;
namespace OHOS::StorageManager {
std::shared_ptr<StorageManagerProvider> storageManagerProvider =
    std::make_shared<StorageManagerProvider>(STORAGE_MANAGER_MANAGER_ID);

// ipccode 109: DeletePartition([in] String diskId, [in] unsigned int partitionNum) — IStorageManager.idl
constexpr uint32_t CODE_DELETE_PARTITION = 109;

bool DeletePartitionFuzzTest(const uint8_t *data, size_t size)
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

    storageManagerProvider->OnRemoteRequest(CODE_DELETE_PARTITION, datas, reply, option);
    return true;
}

bool DeletePartitionFuzzTestWithParams(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t) + sizeof(uint8_t))) {
        return false;
    }

    std::string diskId = "disk-8-";
    uint8_t minor = data[0];
    diskId += std::to_string(minor);

    size_t offset = 1;
    uint32_t partitionNum = 0;
    if (offset + sizeof(uint32_t) <= size) {
        partitionNum = *reinterpret_cast<const uint32_t *>(data + offset);
    }

    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteString(diskId);
    datas.WriteUint32(partitionNum);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(CODE_DELETE_PARTITION, datas, reply, option);
    return true;
}

bool DeletePartitionFuzzTestBoundary(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint8_t))) {
        return false;
    }

    uint8_t minor = data[0];
    std::string diskId = "disk-8-" + std::to_string(minor);

    // Cover edge cases: 0, 1, 15, 128, MAX_UINT32
    uint32_t boundaryValues[] = { 0, 1, 15, 128, UINT32_MAX };
    for (uint32_t partitionNum : boundaryValues) {
        MessageParcel datas;
        datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
        datas.WriteString(diskId);
        datas.WriteUint32(partitionNum);
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;

        storageManagerProvider->OnRemoteRequest(CODE_DELETE_PARTITION, datas, reply, option);
    }
    return true;
}
} // namespace OHOS::StorageManager

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::StorageManager::DeletePartitionFuzzTest(data, size);
    OHOS::StorageManager::DeletePartitionFuzzTestWithParams(data, size);
    OHOS::StorageManager::DeletePartitionFuzzTestBoundary(data, size);
    return 0;
}
