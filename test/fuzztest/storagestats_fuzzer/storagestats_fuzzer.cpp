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
#include "storagestats_fuzzer.h"
#include "storage_stats.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
constexpr size_t NUM_PARA = 6;
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

    int64_t total = TypeCast<int64_t>(data, nullptr);
    int64_t audio = TypeCast<int64_t>(data + sizeof(int64_t), nullptr);
    int64_t video = TypeCast<int64_t>(data + sizeof(int64_t) * 2, nullptr);
    int64_t image = TypeCast<int64_t>(data + sizeof(int64_t) * 3, nullptr);
    int64_t file = TypeCast<int64_t>(data + sizeof(int64_t) * 4, nullptr);
    int64_t app = TypeCast<int64_t>(data + sizeof(int64_t) * 5, nullptr);

    Parcel parcel;
    StorageManager::StorageStats storagestats(total, audio, video, image, file, app);
    storagestats.Marshalling(parcel);
    auto unmarshallingStorageStats = std::unique_ptr<StorageManager::StorageStats>(storagestats.Unmarshalling(parcel));
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
