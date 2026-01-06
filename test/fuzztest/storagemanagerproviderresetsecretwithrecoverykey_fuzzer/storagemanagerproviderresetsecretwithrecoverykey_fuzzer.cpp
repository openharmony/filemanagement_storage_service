/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "storagemanagerproviderresetsecretwithrecoverykey_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
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

bool ResetSecretWithRecoveryKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_RESET_SECRET_WITH_RECOVERY_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}
} // namespace OHOS::StorageManager


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    OHOS::StorageManager::ResetSecretWithRecoveryKeyFuzzTest(data, size);
    return 0;
}
