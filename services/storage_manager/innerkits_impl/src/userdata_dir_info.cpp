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

#include "userdata_dir_info.h"

namespace OHOS {
namespace StorageManager {
bool UserdataDirInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path_)) {
        return false;
    }
    if (!parcel.WriteInt64(totalSize_)) {
        return false;
    }
    if (!parcel.WriteInt32(totalCnt_)) {
        return false;
    }
    return true;
}

UserdataDirInfo *UserdataDirInfo::Unmarshalling(Parcel &parcel)
{
    UserdataDirInfo* obj = new (std::nothrow) UserdataDirInfo();
    if (!obj) {
        return nullptr;
    }
    obj->path_ = parcel.ReadString();
    obj->totalSize_ = parcel.ReadInt64();
    obj->totalCnt_ = parcel.ReadInt32();
    return obj;
}
}
}
