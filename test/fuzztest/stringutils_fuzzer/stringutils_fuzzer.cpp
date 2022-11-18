/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "stringutils_fuzzer.h"
#include "utils/string_utils.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
bool StringUtilsFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(char))) {
        return false;
    }
    bool result = false;
    std::string metaData(reinterpret_cast<const char *>(data), size);
    const char *character = "test";
    StorageDaemon::IsEndWith(metaData, metaData);
    StorageDaemon::StringPrintf(character);
    StorageDaemon::WriteFileSync(character, data, size);
    StorageDaemon::SaveStringToFileSync(metaData, metaData);
    result = true;
    return result;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StringUtilsFuzzTest(data, size);
    return 0;
}
