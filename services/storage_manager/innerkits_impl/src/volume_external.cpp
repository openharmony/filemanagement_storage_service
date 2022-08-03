/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "volume_external.h"

namespace OHOS {
namespace StorageManager {
VolumeExternal::VolumeExternal() {}

VolumeExternal::VolumeExternal(VolumeCore vc)
    : VolumeExternal::VolumeCore(vc.GetId(), vc.GetType(), vc.GetDiskId(), vc.GetState()) {}

void VolumeExternal::SetFsType(int32_t fsType)
{
    fsType_ = fsType;
}

void VolumeExternal::SetFsUuid(std::string fsUuid)
{
    fsUuid_ = fsUuid;
}

void VolumeExternal::SetPath(std::string path)
{
    path_ = path;
}

void VolumeExternal::SetDescription(std::string description)
{
    description_ = description;
}

int32_t VolumeExternal::GetFsType()
{
    return fsType_;
}

std::string VolumeExternal::GetUuid()
{
    return fsUuid_;
}

std::string VolumeExternal::GetPath()
{
    return path_;
}

std::string VolumeExternal::GetDescription()
{
    return description_;
}

void VolumeExternal::Reset()
{
    path_ = "";
}

bool VolumeExternal::Marshalling(Parcel &parcel) const
{
    if (!VolumeCore::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteInt32(fsType_)) {
        return false;
    }

    if (!parcel.WriteString(fsUuid_)) {
        return false;
    }

    if (!parcel.WriteString(path_)) {
        return false;
    }

    if (!parcel.WriteString(description_)) {
        return false;
    }

    return true;
}

std::unique_ptr<VolumeExternal> VolumeExternal::Unmarshalling(Parcel &parcel)
{
    auto obj = std::make_unique<VolumeExternal>(*VolumeCore::Unmarshalling(parcel));
    obj->fsType_ = parcel.ReadInt32();
    obj->fsUuid_ = parcel.ReadString();
    obj->path_ = parcel.ReadString();
    obj->description_ = parcel.ReadString();
    return obj;
}
}
}
