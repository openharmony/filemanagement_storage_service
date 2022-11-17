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
#include "redactionutils_fuzzer.h"

#include "utils/redaction_utils.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
bool RedactionUtilsFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(int32_t))) {
        return false;
    }
    bool result = false;
    int32_t state32 = *(reinterpret_cast<const int32_t *>(data));
    StorageDaemon::RedactionUtils::MountRedactionFs(state32);
    StorageDaemon::RedactionUtils::UMountRedactionFs(state32);
    StorageDaemon::RedactionUtils::CheckRedactionFsMounted(state32);
    StorageDaemon::RedactionUtils::SupportedRedactionFs();
    result = true;
    return result;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::RedactionUtilsFuzzTest(data, size);
    return 0;
}
