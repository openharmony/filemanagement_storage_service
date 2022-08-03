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

#include "bundle_stats.h"

namespace OHOS {
namespace StorageManager {
bool BundleStats::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(appSize_)) {
        return false;
    }

    if (!parcel.WriteInt64(cacheSize_)) {
        return false;
    }

    if (!parcel.WriteInt64(dataSize_)) {
        return false;
    }

    return true;
}

std::unique_ptr<BundleStats> BundleStats::Unmarshalling(Parcel &parcel)
{
    auto obj = std::make_unique<BundleStats>();
    obj->appSize_ = parcel.ReadInt64();
    obj->cacheSize_ = parcel.ReadInt64();
    obj->dataSize_ = parcel.ReadInt64();
    return obj;
}
}
}
