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
#include "storagemanagerproviderumountdissharefilevector_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
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

// ipccode 113: UMountDisShareFile([in] String[] distributeDirs) — IStorageManager.idl
// Note: distinct from the ipccode-66 overload UMountDisShareFile([in] int userId, [in] String networkId);
// the codegen-disambiguated enum name for this String[] overload is not verifiable from checked-in
// source, so the explicit ipccode integer (which the stub dispatches on) is used.
constexpr uint32_t CODE_UMOUNT_DIS_SHARE_FILE_BY_DIRS = 113;
constexpr size_t MAX_DIR_COUNT = 32;
constexpr size_t MAX_DIR_LENGTH = 128;

bool UMountDisShareFileVectorFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_DIR_COUNT;
    std::vector<std::string> distributeDirs;
    for (uint8_t i = 0; i < count; i++) {
        distributeDirs.push_back(fdp.ConsumeRandomLengthString(MAX_DIR_LENGTH));
    }

    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteStringVector(distributeDirs);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(CODE_UMOUNT_DIS_SHARE_FILE_BY_DIRS, datas, reply, option);
    return true;
}
} // namespace OHOS::StorageManager

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::StorageManager::UMountDisShareFileVectorFuzzTest(data, size);
    return 0;
}
