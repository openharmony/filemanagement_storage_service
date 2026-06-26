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
#include "storagedaemonproviderumountdissharefileext_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "fuzzer/FuzzedDataProvider.h"
#include "message_parcel.h"
#include "storage_daemon_provider.h"
#include "storage_daemon_stub.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {

static const int32_t MAX_DIR_STRING_LENGTH = 64;
static const int32_t MIN_DIR_COUNT = 0;
static const int32_t MAX_DIR_COUNT = 8;

std::shared_ptr<StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemonProvider>();

bool UMountDisShareFileExtFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_UMOUNT_DIS_SHARE_FILE_IN_STRING_VECTOR);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    std::vector<std::string> distributeDirs;
    size_t dirCount = fdp.ConsumeIntegralInRange<size_t>(MIN_DIR_COUNT, MAX_DIR_COUNT);
    for (size_t i = 0; i < dirCount; i++) {
        distributeDirs.push_back(fdp.ConsumeRandomLengthString(MAX_DIR_STRING_LENGTH));
    }
    datas.WriteStringVector(distributeDirs);
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
    OHOS::UMountDisShareFileExtFuzzTest(data, size);
    return 0;
}
