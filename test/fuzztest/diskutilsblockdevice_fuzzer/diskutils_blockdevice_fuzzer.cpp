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

#include "diskutils_blockdevice_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>


#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsBlockDeviceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    std::string devPath = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    uint32_t mode = fdp.ConsumeIntegral<uint32_t>();
    int32_t major = fdp.ConsumeIntegral<int32_t>();
    int32_t minor = fdp.ConsumeIntegral<int32_t>();
    DiskUtils::CreateBlockDeviceNode(devPath, mode, major, minor);

    DiskUtils::DestroyBlockDeviceNode(devPath);

    std::string output;
    int32_t maxVolume = 0;
    DiskUtils::ReadPartitionTable(devPath, output, maxVolume);

    std::string execRet;
    DiskUtils::GetPartitionTableInfo(devPath, execRet);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsBlockDeviceFuzzTest(data, size);
    return 0;
}
