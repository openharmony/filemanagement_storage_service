/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "fileutils_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "utils/file_utils.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    unsigned int ustate = fdp.ConsumeIntegral<unsigned int >();
    uint32_t state32 = fdp.ConsumeIntegral<uint32_t>();
    std::string metaData(fdp.ConsumeRandomLengthString(size));
    struct StorageDaemon::FileList list = {ustate, metaData};
    std::vector<std::string> metaData2;
    metaData2.push_back(metaData);
    std::vector<StorageDaemon::FileList> metaData4;
    metaData4.push_back(list);
    std::string metaData5;
    StorageDaemon::IsDir(metaData);
    StorageDaemon::StringToUint32(metaData, state32);
    StorageDaemon::GetSubDirs(metaData, metaData2);
    StorageDaemon::ReadDigitDir(metaData, metaData4);
    StorageDaemon::ReadFile(metaData, &metaData5);
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
