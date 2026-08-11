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

#include "diskutils_volume_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>


#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsVolumeFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    std::string volId = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    int32_t progressPct = 0;
    DiskUtils::GetVolumeOpProcess(volId, progressPct);

    std::string devPath = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    int32_t verifyType = fdp.ConsumeIntegral<int32_t>();
    DiskUtils::VerifyBurnData(devPath, verifyType);

    int cmdFd = fdp.ConsumeIntegral<int32_t>();
    std::string discType = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    DiskUtils::GetDiscCapacity(cmdFd, discType);

    int64_t totalSize = 0;
    int64_t usedSize = 0;
    DiskUtils::AdjustBlankDiscCapacity(devPath, discType, totalSize, usedSize);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsVolumeFuzzTest(data, size);
    return 0;
}
