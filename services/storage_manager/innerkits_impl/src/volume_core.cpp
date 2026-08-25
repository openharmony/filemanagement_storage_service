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
                       const std::string &fsType, const std::string &extraInfo, uint32_t partitionNum)
{
    id_ = id;
    type_ = type;
    diskId_ = diskId;
    state_ = state;
    fsType_ = fsType;
    extraInfo_ = extraInfo;
    partitionNum_ = partitionNum;
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

void VolumeCore::SetId(const std::string volId)
{
    id_ = volId;
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

uint32_t VolumeCore::GetPartitionNum() const
{
    return partitionNum_;
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
    if (!parcel.WriteUint32(partitionNum_)) {
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
    if (!parcel.ReadString(obj->id_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadInt32(obj->type_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->diskId_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadInt32(obj->state_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadBool(obj->errorFlag_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->fsType_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->extraInfo_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadUint32(obj->partitionNum_)) {
        delete obj;
        return nullptr;
    }
    return obj;
}

VolumeInfoStr *VolumeInfoStr::Unmarshalling(Parcel &parcel)
{
    VolumeInfoStr* obj = new (std::nothrow) VolumeInfoStr();
    if (!obj) {
        return nullptr;
    }
    if (!parcel.ReadString(obj->volumeId)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->fsTypeStr)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->fsUuid)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->path)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->description)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadBool(obj->isDamaged)) {
        delete obj;
        return nullptr;
    }
    return obj;
}
}
}
