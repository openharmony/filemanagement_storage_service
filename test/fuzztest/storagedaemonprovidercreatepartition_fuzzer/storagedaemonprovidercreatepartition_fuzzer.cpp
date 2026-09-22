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

#include "fuzzer/FuzzedDataProvider.h"
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
    FuzzedDataProvider fdp(data, size);
    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_CREATE_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    std::vector<uint8_t> buffer = fdp.ConsumeBytes<uint8_t>(size);
    datas.WriteBuffer(buffer.data(), buffer.size());
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CreatePartitionFuzzTestWithOpts(const uint8_t *data, size_t size)
{
    // Maximum length for partition type code string
    constexpr size_t maxTypeCodeLength = 32;
    if ((data == nullptr) || (size == 0)) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Parse fuzzed data to create PartitionParams
    PartitionParams partitionParams;
    int32_t partitionNum = fdp.ConsumeIntegral<int32_t>();
    partitionParams.SetPartitionNum(partitionNum);
    uint64_t startSector = fdp.ConsumeIntegral<uint64_t>();
    partitionParams.SetStartSector(startSector);
    uint64_t endSector = fdp.ConsumeIntegral<uint64_t>();
    partitionParams.SetEndSector(endSector);
    std::vector<uint8_t> typeCodeVec = fdp.ConsumeBytes<uint8_t>(maxTypeCodeLength);
    std::string typeCode(typeCodeVec.begin(), typeCodeVec.end());
    partitionParams.SetTypeCode(typeCode);

    // Create diskId from fuzzed data
    std::string diskId = DISK_ID_PREFIX;
    uint8_t minor = fdp.ConsumeIntegral<uint8_t>();
    diskId += std::to_string(minor);

    uint32_t code = static_cast<uint32_t>(IStorageDaemonIpcCode::COMMAND_CREATE_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteString(diskId);
    partitionParams.Marshalling(datas);
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
