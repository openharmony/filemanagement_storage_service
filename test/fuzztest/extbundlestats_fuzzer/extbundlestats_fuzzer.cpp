/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "extbundlestats_fuzzer.h"
#include "ext_bundle_stats.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
template<typename T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint64_t)) {
        return false;
    }

    int pos = 0;
    uint64_t businessSize = 0;
    const uint64_t maxBusinessSize = 500 * 1024 * 1024;
    if (pos + sizeof(uint64_t) <= size) {
        businessSize = TypeCast<uint64_t>(data, &pos);
        businessSize = businessSize % (maxBusinessSize);
    } else {
        return false;
    }
    bool showFlag = false;
    std::string businessName = "data";
    Parcel parcel;
    StorageManager::ExtBundleStats extbundlestats(businessName, businessSize, showFlag);
    extbundlestats.Marshalling(parcel);
    extbundlestats.Unmarshalling(parcel);
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