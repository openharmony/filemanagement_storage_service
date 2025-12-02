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

#include "ext_bundle_stats.h"

namespace OHOS {
namespace StorageManager {
bool ExtBundleStats::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(businessName_)) {
        return false;
    }
    if (!parcel.WriteUint64(businessSize_)) {
        return false;
    }
    if (!parcel.WriteBool(showFlag_)) {
        return false;
    }
    return true;
}

ExtBundleStats *ExtBundleStats::Unmarshalling(Parcel &parcel)
{
    ExtBundleStats* obj = new (std::nothrow) ExtBundleStats();
    if (!obj) {
        return nullptr;
    }
    obj->businessName_ = parcel.ReadString();
    obj->businessSize_ = parcel.ReadUint64();
    obj->showFlag_ = parcel.ReadBool();
    return obj;
}
}
}
