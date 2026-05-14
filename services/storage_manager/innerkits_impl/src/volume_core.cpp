/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "volume_core.h"

namespace OHOS {
namespace StorageManager {
VolumeCore::VolumeCore() {}

VolumeCore::VolumeCore(const std::string &id, int32_t type, const std::string &diskId)
{
    id_ = id;
    type_ = type;
    diskId_ = diskId;
}

VolumeCore::VolumeCore(const std::string &id, int32_t type, const std::string &diskId, int32_t state)
{
    id_ = id;
    type_ = type;
    diskId_ = diskId;
    state_ = state;
}

VolumeCore::VolumeCore(const std::string &id, int32_t type, const std::string &diskId, int32_t state,
                       const std::string &fsType, const std::string &extraInfo)
{
    id_ = id;
    type_ = type;
    diskId_ = diskId;
    state_ = state;
    fsType_ = fsType;
    extraInfo_ = extraInfo;
}

void VolumeCore::SetState(int32_t state)
{
    state_ = state;
}

void VolumeCore::SetFsType(std::string fsType)
{
    fsType_ = fsType;
}

std::string VolumeCore::GetId() const
{
    return id_;
}

int VolumeCore::GetType() const
{
    return type_;
}

std::string VolumeCore::GetDiskId() const
{
    return diskId_;
}

int32_t VolumeCore::GetState() const
{
    return state_;
}

std::string VolumeCore::GetFsType() const
{
    return fsType_;
}

std::string VolumeCore::GetExtraInfo() const
{
    return extraInfo_;
}

bool VolumeCore::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(id_)) {
        return false;
    }

    if (!parcel.WriteInt32(type_)) {
        return false;
    }

    if (!parcel.WriteString(diskId_)) {
        return false;
    }

    if (!parcel.WriteInt32(state_)) {
        return false;
    }

    if (!parcel.WriteBool(errorFlag_)) {
        return false;
    }

    if (!parcel.WriteString(fsType_)) {
        return false;
    }
    if (!parcel.WriteString(extraInfo_)) {
        return false;
    }

    return true;
}

bool VolumeInfoStr::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(volumeId)) {
        return false;
    }

    if (!parcel.WriteString(fsTypeStr)) {
        return false;
    }

    if (!parcel.WriteString(fsUuid)) {
        return false;
    }

    if (!parcel.WriteString(path)) {
        return false;
    }

    if (!parcel.WriteString(description)) {
        return false;
    }

    if (!parcel.WriteBool(isDamaged)) {
        return false;
    }

    return true;
}

VolumeCore *VolumeCore::Unmarshalling(Parcel &parcel)
{
    VolumeCore* obj = new (std::nothrow) VolumeCore();
    if (!obj) {
        return nullptr;
    }
    obj->id_ = parcel.ReadString();
    obj->type_ = parcel.ReadInt32();
    obj->diskId_ = parcel.ReadString();
    obj->state_ = parcel.ReadInt32();
    obj->errorFlag_ = parcel.ReadBool();
    obj->fsType_ = parcel.ReadString();
    obj->extraInfo_ = parcel.ReadString();
    return obj;
}

VolumeInfoStr *VolumeInfoStr::Unmarshalling(Parcel &parcel)
{
    VolumeInfoStr* obj = new (std::nothrow) VolumeInfoStr();
    if (!obj) {
        return nullptr;
    }
    obj->volumeId = parcel.ReadString();
    obj->fsTypeStr = parcel.ReadString();
    obj->fsUuid = parcel.ReadString();
    obj->path = parcel.ReadString();
    obj->description = parcel.ReadString();
    obj->isDamaged = parcel.ReadBool();
    return obj;
}
}
}
