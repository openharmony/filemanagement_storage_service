/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_STATS_H
#define OHOS_STORAGE_MANAGER_STORAGE_STATS_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class StorageStats final : public Parcelable {
public:
    StorageStats() {}
    StorageStats(int64_t total, int64_t audio, int64_t video, int64_t image, int64_t file, int64_t app)
        : total_(total), audio_(audio), video_(video), image_(image), file_(file), app_(app) {}
    ~StorageStats() {}

    int64_t total_ {0};
    int64_t audio_ {0};
    int64_t video_ {0};
    int64_t image_ {0};
    int64_t file_ {0};
    int64_t app_ {0};

    bool Marshalling(Parcel &parcel) const override;
    static StorageStats *Unmarshalling(Parcel &parcel);
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_STATS_H