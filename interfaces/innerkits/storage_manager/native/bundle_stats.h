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

#ifndef OHOS_STORAGE_MANAGER_BUNDLE_STATS_H
#define OHOS_STORAGE_MANAGER_BUNDLE_STATS_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class BundleStats final : public Parcelable {
public:
    BundleStats() {}
    BundleStats(int64_t appSize, int64_t cacheSize, int64_t dataSize)
        : appSize_(appSize), cacheSize_(cacheSize), dataSize_(dataSize) {}
    ~BundleStats() {}

    int64_t appSize_ {0};
    int64_t cacheSize_ {0};
    int64_t dataSize_ {0};

    bool Marshalling(Parcel &parcel) const override;
    static std::unique_ptr<BundleStats> Unmarshalling(Parcel &parcel);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_BUNDLE_STATS_H