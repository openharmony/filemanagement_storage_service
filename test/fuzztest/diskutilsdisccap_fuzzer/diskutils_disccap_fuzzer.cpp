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

#include "diskutils_disccap_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {

bool DiskUtilsDiscCapFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);

    const std::string discTypes[] = {
        "DVD-R", "DVD+R", "DVD+RW", "DVD-RW",
        "CD-ROM", "CD-RW", "CD-R",
        "BD-R", "BD-RE", "BD-ROM",
        "unknown", ""
    };
    uint8_t dtIdx = fdp.ConsumeIntegral<uint8_t>() % 12;
    std::string discType = discTypes[dtIdx];
    int cmdFd = fdp.ConsumeIntegral<int>();
    DiskUtils::GetDiscCapacity(cmdFd, discType);

    const std::string devPaths[] = {
        "/dev/block/sr0", "/dev/block/sda1", "/dev/sr0", "/mnt/data/external/mtp/test", "invalid"
    };
    uint8_t dpIdx = fdp.ConsumeIntegral<uint8_t>() % 5;
    std::string devPath = devPaths[dpIdx];
    int64_t totalSize = 0;
    int64_t usedSize = 0;
    DiskUtils::AdjustBlankDiscCapacity(devPath, discType, totalSize, usedSize);

    int64_t capTotal = 0;
    int64_t capFree = 0;
    DiskUtils::GetCapacity(devPath, capTotal, capFree);

    int32_t verifyType = fdp.ConsumeIntegral<int32_t>();
    DiskUtils::VerifyBurnData(devPath, verifyType);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsDiscCapFuzzTest(data, size);
    return 0;
}
