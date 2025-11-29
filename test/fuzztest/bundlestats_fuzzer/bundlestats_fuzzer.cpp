/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "bundlestats_fuzzer.h"
#include "bundle_stats.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
constexpr size_t NUM_PARA = 3;
template<typename T>
T TypeCast(const uint8_t *data, int *pos)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint64_t) * NUM_PARA)) {
        return true;
    }

    int64_t appSize = TypeCast<int64_t>(data, nullptr);
    int64_t cacheSize = TypeCast<int64_t>(data + sizeof(int64_t), nullptr);
    int64_t dataSize = TypeCast<int64_t>(data + sizeof(int64_t) * 2, nullptr);

    Parcel parcel;
    StorageManager::BundleStats bundlestats(appSize, cacheSize, dataSize);
    bundlestats.Marshalling(parcel);
    auto unmarshallingBundleStats = std::unique_ptr<StorageManager::BundleStats>(bundlestats.Unmarshalling(parcel));
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
