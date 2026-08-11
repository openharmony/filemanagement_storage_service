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

#include "diskutils_fmtcmdstr_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsFmtCmdStrFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);

    const std::string fsTypes[] = {"vfat", "ext4", "exfat", "hmfs", "unknown", ""};
    uint8_t fsIdx = fdp.ConsumeIntegral<uint8_t>() % 6;
    std::string fsType = fsTypes[fsIdx];
    std::string devPath = "/dev/block/sda1";
    bool hasVolName = fdp.ConsumeBool();
    std::string volName = hasVolName ? fdp.ConsumeRandomLengthString(MAX_STR_LEN) : "";
    DiskUtils::GetFormatCMD(fsType, devPath, volName);

    const std::string splitInputs[] = {"a,b,c", "path/to/file", "one||two||three", "", "single"};
    const char delimiters[] = {',', '/', '|', ' ', '\n'};
    uint8_t sIdx = fdp.ConsumeIntegral<uint8_t>() % 5;
    uint8_t dIdx = fdp.ConsumeIntegral<uint8_t>() % 5;
    DiskUtils::SplitString(splitInputs[sIdx], delimiters[dIdx]);

    const std::string fullPaths[] = {"/data/base/file", "/mnt/media/usb", "relative/path", "/dev/block/sda1"};
    const std::string baseDirs[] = {"/data/base", "/mnt/media", "/wrong", "", "/data"};
    uint8_t fpIdx = fdp.ConsumeIntegral<uint8_t>() % 4;
    uint8_t bdIdx = fdp.ConsumeIntegral<uint8_t>() % 5;
    DiskUtils::GetRelativePath(fullPaths[fpIdx], baseDirs[bdIdx]);

    const std::string linePatterns[] = {
        "-rw-r--r-- file1",
        "drwxr-xr-x dir1",
        "  continuation line",
        "Directory listing of /mnt/disc",
        "[ISO9660] entry",
        "",
        "normal line",
        "another continuation"
    };
    std::vector<std::string> mergeLines;
    int mergeCount = fdp.ConsumeIntegral<int32_t>() % 5 + 1;
    for (int i = 0; i < mergeCount && fdp.remaining_bytes() > 0; ++i) {
        uint8_t lpIdx = fdp.ConsumeIntegral<uint8_t>() % 8;
        mergeLines.push_back(linePatterns[lpIdx]);
    }
    DiskUtils::MergeOutputLines(mergeLines);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsFmtCmdStrFuzzTest(data, size);
    return 0;
}
