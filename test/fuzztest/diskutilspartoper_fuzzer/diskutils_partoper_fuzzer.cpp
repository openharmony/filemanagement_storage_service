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

#include "diskutils_partoper_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>


#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsPartOperFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    std::string diskPath = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    std::string partitionType = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    DiskUtils::Partition(diskPath, partitionType);

    int32_t partitionNum = fdp.ConsumeIntegral<int32_t>();
    int64_t startSector = fdp.ConsumeIntegral<int64_t>();
    int64_t endSector = fdp.ConsumeIntegral<int64_t>();
    std::string typeCode = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    DiskUtils::CreatePartition(diskPath, partitionNum, startSector, endSector, typeCode);

    std::string diskId = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    DiskUtils::DeletePartitionInfo(diskPath, diskId, partitionNum);

    std::string fsType = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    std::string volumeName = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    bool quickFormat = fdp.ConsumeBool();
    DiskUtils::FormatPartition(diskPath, fsType, volumeName, quickFormat);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsPartOperFuzzTest(data, size);
    return 0;
}
