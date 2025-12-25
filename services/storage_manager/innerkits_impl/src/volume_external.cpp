/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
    : VolumeExternal::VolumeCore(vc.GetId(), vc.GetType(), vc.GetDiskId(), vc.GetState(), vc.GetFsType()) {}

void VolumeExternal::SetFlags(int32_t flags)
{
    flags_ = flags;
}

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

int32_t VolumeExternal::GetFlags()
{
    return flags_;
}

int32_t VolumeExternal::GetFsType()
{
    return fsType_;
}

std::string VolumeExternal::GetFsTypeString()
{
    auto it = FS_TYPE_MAP.find(fsType_);
    if (it == FS_TYPE_MAP.end()) {
        return "undefined";
    }
    return FS_TYPE_MAP[fsType_];
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

int32_t VolumeExternal::GetFsTypeByStr(const std::string &fsTypeStr)
{
    for (uint32_t i = 0; i < FS_TYPE_MAP.size(); i++) {
        if (FS_TYPE_MAP[i].compare(fsTypeStr) == 0) {
            return i;
        }
    }
    return -1;
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

    if (!parcel.WriteInt32(flags_)) {
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

VolumeExternal *VolumeExternal::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<VolumeCore> volumeCorePtr(VolumeCore::Unmarshalling(parcel));
    VolumeExternal* obj = new (std::nothrow) VolumeExternal(*volumeCorePtr);
    if (!obj) {
        return nullptr;
    }
    obj->flags_ = parcel.ReadInt32();
    obj->fsType_ = parcel.ReadInt32();
    obj->fsUuid_ = parcel.ReadString();
    obj->path_ = parcel.ReadString();
    obj->description_ = parcel.ReadString();
    return obj;
}
}
}
