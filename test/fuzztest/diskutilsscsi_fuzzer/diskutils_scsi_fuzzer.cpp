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

#include "diskutils_scsi_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <cstring>
constexpr int32_t SCSI_BUF_LEN = 64;

#include "fuzzer/FuzzedDataProvider.h"
#include "disk_manager/disk/disk_utils.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t MAX_STR_LEN = 256;

bool DiskUtilsScsiFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    int fd = fdp.ConsumeIntegral<int>();
    uint8_t cdb[SCSI_BUF_LEN] = {0};
    fdp.ConsumeData(&cdb, sizeof(cdb));
    int cdbLen = fdp.ConsumeIntegral<int>();
    uint8_t buf[SCSI_BUF_LEN] = {0};
    int dxferLen = fdp.ConsumeIntegral<int>();
    ExecuteScsiCmd(fd, cdb, cdbLen, buf, dxferLen);

    std::string diskPath = fdp.ConsumeRandomLengthString(MAX_STR_LEN);
    int32_t cmdIndex = fdp.ConsumeIntegral<int32_t>();
    ReadCDDiscInfo(diskPath, cmdIndex, buf, sizeof(buf));
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DiskUtilsScsiFuzzTest(data, size);
    return 0;
}
