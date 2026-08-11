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

#include "diskutils_string_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsStringFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    std::string input = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    char delimiter = static_cast<char>(fdp.ConsumeIntegral<uint8_t>());
    DiskUtils::SplitString(input, delimiter);

    std::string baseDir = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    DiskUtils::GetRelativePath(input, baseDir);

    std::vector<std::string> lines;
    int lists = 4;
    for (int i = 0; i < lists && fdp.remaining_bytes() > 0; ++i) {
        lines.push_back(fdp.ConsumeRandomLengthString(MAX_STR_LEN));
    }
    DiskUtils::MergeOutputLines(lines);

    DiskUtils::ParseDirectoryPath(input);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsStringFuzzTest(data, size);
    return 0;
}
