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

#include "storage_stats.h"

namespace OHOS {
namespace StorageManager {
bool StorageStats::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(total_)) {
        return false;
    }

    if (!parcel.WriteInt64(audio_)) {
        return false;
    }

    if (!parcel.WriteInt64(video_)) {
        return false;
    }

    if (!parcel.WriteInt64(image_)) {
        return false;
    }

    if (!parcel.WriteInt64(file_)) {
        return false;
    }

    if (!parcel.WriteInt64(app_)) {
        return false;
    }

    return true;
}

std::unique_ptr<StorageStats> StorageStats::Unmarshalling(Parcel &parcel)
{
    auto obj = std::make_unique<StorageStats>();
    obj->total_ = parcel.ReadInt64();
    obj->audio_ = parcel.ReadInt64();
    obj->video_ = parcel.ReadInt64();
    obj->image_ = parcel.ReadInt64();
    obj->file_ = parcel.ReadInt64();
    obj->app_ = parcel.ReadInt64();
    return obj;
}
}
}
