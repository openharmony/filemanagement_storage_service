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

#include "diskutils_dirparse_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsDirParseFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);

    const std::string dirLines[] = {
        "Directory listing of /mnt/disc", "Directory listing of /path/to/dir",
        "Directory listing of no_slash_here", "not a directory listing line", ""
    };
    uint8_t dlIdx = fdp.ConsumeIntegral<uint8_t>() % 5;
    DiskUtils::ParseDirectoryPath(dirLines[dlIdx]);

    const std::string fileEntries[] = {
        "[ISO9660] filename.txt", "[UDF]   testfile.img", "no_bracket_file",
        "[entry] .", "[entry] ..", "[entry] file;1", "[entry]   trailing  ", ""
    };
    uint8_t feIdx = fdp.ConsumeIntegral<uint8_t>() % 8;
    DiskUtils::ParseFileName(fileEntries[feIdx]);

    const std::string entryLines[] = {
        "-rw-r--r-- file1", "drwxr-xr-x dir1", "lrwxrwxrwx link1", "   ", "", "xrw-r--r-- other"
    };
    uint8_t elIdx = fdp.ConsumeIntegral<uint8_t>() % 6;
    char entryType = ' ';
    DiskUtils::IsFileEntry(entryLines[elIdx], entryType);

    std::map<std::string, std::string> sourceMap;
    std::map<std::string, std::string> discMap;
    int entryCount = fdp.ConsumeIntegral<int32_t>() % 3 + 1;
    for (int i = 0; i < entryCount && fdp.remaining_bytes() > 0; ++i) {
        std::string filename = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
        std::string sourceMd5 = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
        sourceMap[filename] = sourceMd5;
        bool matchInDisc = fdp.ConsumeBool();
        if (matchInDisc) {
            discMap[filename] = sourceMd5;
        } else {
            discMap[filename] = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
        }
    }
    bool addMissingToDisc = fdp.ConsumeBool();
    if (addMissingToDisc) {
        std::string missingFile = "missing_file_" + std::to_string(entryCount);
        sourceMap[missingFile] = "abc123";
    }
    DiskUtils::CompareChecksums(sourceMap, discMap);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsDirParseFuzzTest(data, size);
    return 0;
}
