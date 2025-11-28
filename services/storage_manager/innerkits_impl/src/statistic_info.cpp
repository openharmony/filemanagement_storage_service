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

#include "statistic_info.h"

namespace OHOS {
namespace StorageManager {
bool NextDqBlk::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint64(dqbHardLimit)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbBSoftLimit)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbCurSpace)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbIHardLimit)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbISoftLimit)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbCurInodes)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbBTime)) {
        return false;
    }
    if (!parcel.WriteUint64(dqbITime)) {
        return false;
    }
    if (!parcel.WriteUint32(dqbValid)) {
        return false;
    }
    if (!parcel.WriteUint32(dqbId)) {
        return false;
    }
    return true;
}

NextDqBlk *NextDqBlk::Unmarshalling(Parcel &parcel)
{
    NextDqBlk* obj = new (std::nothrow) NextDqBlk();
    if (!obj) {
        return nullptr;
    }
    obj->dqbHardLimit = parcel.ReadUint64();
    obj->dqbBSoftLimit = parcel.ReadUint64();
    obj->dqbCurSpace = parcel.ReadUint64();
    obj->dqbIHardLimit = parcel.ReadUint64();
    obj->dqbISoftLimit = parcel.ReadUint64();
    obj->dqbCurInodes = parcel.ReadUint64();
    obj->dqbBTime = parcel.ReadUint64();
    obj->dqbITime = parcel.ReadUint64();
    obj->dqbValid = parcel.ReadUint32();
    obj->dqbId = parcel.ReadUint32();
    return obj;
}
} // StorageManager
} // OHOS

namespace OHOS {
namespace StorageManager {
bool DirSpaceInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path)) {
        return false;
    }
    if (!parcel.WriteUint32(uid)) {
        return false;
    }
    if (!parcel.WriteInt64(size)) {
        return false;
    }
    return true;
}

DirSpaceInfo *DirSpaceInfo::Unmarshalling(Parcel &parcel)
{
    DirSpaceInfo *info = new (std::nothrow) DirSpaceInfo();
    if (info == nullptr) {
        return nullptr;
    }
    info->path = parcel.ReadString();
    info->uid = parcel.ReadUint32();
    info->size = parcel.ReadInt64();
    return info;
}
} // StorageManager
} // OHOS
