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
#include "partitiontableinfo_fuzzer.h"
#include "partition_table_info.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <cstddef>
#include <cstdint>
#include <memory>

namespace OHOS {
bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return true;
    }
    FuzzedDataProvider fdp(data, size);

    uint64_t totalSector = fdp.ConsumeIntegral<uint64_t>();
    uint32_t partitionCount = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sectorSize = fdp.ConsumeIntegral<uint32_t>();
    uint32_t alignSector = fdp.ConsumeIntegral<uint32_t>();
    std::string diskId = fdp.ConsumeRandomLengthString(256);
    std::string tableType = fdp.ConsumeRandomLengthString(256);

    Parcel parcel;
    StorageManager::PartitionTableInfo partitionTableInfo;
    partitionTableInfo.SetDiskId(diskId);
    partitionTableInfo.SetTableType(tableType);
    partitionTableInfo.SetPartitionCount(partitionCount);
    partitionTableInfo.SetTotalSector(totalSector);
    partitionTableInfo.SetSectorSize(sectorSize);
    partitionTableInfo.SetAlignSector(alignSector);
    partitionTableInfo.GetDiskId();
    partitionTableInfo.GetTableType();
    partitionTableInfo.GetPartitionCount();
    partitionTableInfo.GetTotalSector();
    partitionTableInfo.GetSectorSize();
    partitionTableInfo.GetAlignSector();
    partitionTableInfo.GetPartitions();
    partitionTableInfo.Marshalling(parcel);
    auto unmarshallingPartitionTableInfo =
        std::unique_ptr<StorageManager::PartitionTableInfo>(partitionTableInfo.Unmarshalling(parcel));
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FileUtilFuzzTest(data, size);
    return 0;
}
