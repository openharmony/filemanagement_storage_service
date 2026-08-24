/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    if (!parcel.ReadUint64(obj->dqbHardLimit)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbBSoftLimit)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbCurSpace)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbIHardLimit)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbISoftLimit)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbCurInodes)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbBTime)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint64(obj->dqbITime)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint32(obj->dqbValid)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint32(obj->dqbId)) {
        delete obj;
        return nullptr;
    }
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
    if (!parcel.ReadString(info->path)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadUint32(info->uid)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->size)) {
        delete info;
        return nullptr;
    }
    return info;
}
} // StorageManager
} // OHOS

namespace OHOS {
namespace StorageManager {
bool UidSaInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(uid)) {
        return false;
    }
    if (!parcel.WriteString(saName)) {
        return false;
    }
    if (!parcel.WriteInt64(size)) {
        return false;
    }
    if (!parcel.WriteUint64(iNodes)) {
        return false;
    }
    return true;
}

UidSaInfo *UidSaInfo::Unmarshalling(Parcel &parcel)
{
    UidSaInfo *info = new (std::nothrow) UidSaInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadInt32(info->uid)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->saName)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->size)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadUint64(info->iNodes)) {
        delete info;
        return nullptr;
    }
    return info;
}
} // StorageManager
} // OHOS

namespace OHOS {
namespace StorageManager {
bool LargeFileInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path)) {
        return false;
    }
    if (!parcel.WriteInt64(size)) {
        return false;
    }
    return true;
}

LargeFileInfo *LargeFileInfo::Unmarshalling(Parcel &parcel)
{
    LargeFileInfo *info = new (std::nothrow) LargeFileInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadString(info->path)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->size)) {
        delete info;
        return nullptr;
    }
    return info;
}
} // StorageManager
} // OHOS

namespace OHOS {
namespace StorageManager {
bool LargeDirInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path)) {
        return false;
    }
    if (!parcel.WriteInt64(totalSize)) {
        return false;
    }
    return true;
}

LargeDirInfo *LargeDirInfo::Unmarshalling(Parcel &parcel)
{
    LargeDirInfo *info = new (std::nothrow) LargeDirInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadString(info->path)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->totalSize)) {
        delete info;
        return nullptr;
    }
    return info;
}
} // StorageManager
} // OHOS
