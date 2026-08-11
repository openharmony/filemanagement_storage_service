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

#include "diskutils_checksum_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
namespace StorageDaemon {
std::string GetOpticalDriveNode(const std::string &devPath);
}

bool DiskUtilsChecksumFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);

    const std::string checksumLines[] = {
        "d41d8cd98f00b204e9800998ecf8427e  /mnt/disc/file1.txt\n",
        "098f6bcd4621d373cade4e832627b4f6  /mnt/disc/file2.txt\n",
        "invalid_line_no_double_space\n",
        "\n",
        "abc123  /path/to/file\n",
        ""
    };
    std::string checksumContent;
    int lineCount = fdp.ConsumeIntegral<int32_t>() % 4 + 1;
    for (int i = 0; i < lineCount && fdp.remaining_bytes() > 0; ++i) {
        uint8_t clIdx = fdp.ConsumeIntegral<uint8_t>() % 6;
        checksumContent += checksumLines[clIdx];
    }
    const std::string basePaths[] = {"/mnt/disc", "/path/to", "/wrong", ""};
    uint8_t bpIdx = fdp.ConsumeIntegral<uint8_t>() % 4;
    DiskUtils::ParseChecksumFile(checksumContent, basePaths[bpIdx]);

    const std::string dirPaths[] = {"/data/local/tmp", "/mnt/disc", "/tmp/fuzz", "invalid_path"};
    const std::string ckstPaths[] = {"/data/local/tmp/checksum.md5", "/tmp/ck.txt", "/mnt/disc/sum.txt"};
    uint8_t dpIdx = fdp.ConsumeIntegral<uint8_t>() % 4;
    uint8_t cpIdx = fdp.ConsumeIntegral<uint8_t>() % 3;
    DiskUtils::GenerateChecksums(dirPaths[dpIdx], ckstPaths[cpIdx]);

    const std::string volIds[] = {"vol-123", "public:123:456", "", "usb-789"};
    uint8_t viIdx = fdp.ConsumeIntegral<uint8_t>() % 4;
    int32_t progressPct = 0;
    DiskUtils::GetVolumeOpProcess(volIds[viIdx], progressPct);

    const std::string devPaths[] = {
        "/dev/block/sr0-11:5:0",
        "/dev/block/sda-8:0:0",
        "/dev/sr0-11:0:0:0",
        "/dev/block/nodisk",
        "no-dash-path"
    };
    uint8_t odIdx = fdp.ConsumeIntegral<uint8_t>() % 5;
    GetOpticalDriveNode(devPaths[odIdx]);
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsChecksumFuzzTest(data, size);
    return 0;
}
