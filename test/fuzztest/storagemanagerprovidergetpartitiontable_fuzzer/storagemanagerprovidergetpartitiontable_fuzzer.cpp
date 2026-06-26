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
#include "storagemanagerprovidergetpartitiontable_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "ipc/storage_manager_provider.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "securec.h"
#include "storage_manager_stub.h"

using namespace OHOS::StorageManager;
namespace OHOS::StorageManager {
std::shared_ptr<StorageManagerProvider> storageManagerProvider =
    std::make_shared<StorageManagerProvider>(STORAGE_MANAGER_MANAGER_ID);

// ipccode 107: GetPartitionTable([in] String diskId, [out] PartitionTableInfo) — IStorageManager.idl
constexpr uint32_t CODE_GET_PARTITION_TABLE = 107;
constexpr size_t MAX_DISK_ID_LENGTH = 64;

bool GetPartitionTableFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    std::string diskId = fdp.ConsumeRandomLengthString(MAX_DISK_ID_LENGTH);

    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteString(diskId);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(CODE_GET_PARTITION_TABLE, datas, reply, option);
    return true;
}
} // namespace OHOS::StorageManager

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::StorageManager::GetPartitionTableFuzzTest(data, size);
    return 0;
}
