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
#include "storagedaemonprovidersetrecoverkey_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sstream>

#include "message_parcel.h"
#include "storage_daemon_stub.h"
#include "storage_daemon_provider.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {

std::shared_ptr<StorageDaemon::StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemon::StorageDaemonProvider>();

bool StorageDaemonProviderSetRecoverKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_CREATE_RECOVER_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
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
    OHOS::StorageDaemonProviderSetRecoverKeyFuzzTest(data, size);
    return 0;
}
