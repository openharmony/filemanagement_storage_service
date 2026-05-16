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

#include "disk.h"

namespace OHOS {
namespace StorageManager {
Disk::Disk() {}

Disk::Disk(const std::string &diskId, int64_t sizeBytes, const std::string &sysPath, const std::string &vendor,
           int32_t diskType)
    : diskId_(diskId), sizeBytes_(sizeBytes), sysPath_(sysPath), vendor_(vendor), diskType_(diskType) {}

std::string Disk::GetDiskId() const
{
    return diskId_;
}

std::string Disk::GetDiskName() const
{
    return diskName_;
}

int64_t Disk::GetSizeBytes() const
{
    return sizeBytes_;
}

std::string Disk::GetSysPath() const
{
    return sysPath_;
}

std::string Disk::GetVendor() const
{
    return vendor_;
}

int32_t Disk::GetDiskType() const
{
    return diskType_;
}

int32_t Disk::GetMediaType() const
{
    return mediaType_;
}

int32_t Disk::GetRemovable() const
{
    return removable_;
}

std::string Disk::GetExtraInfo() const
{
    return extraInfo_;
}

void Disk::SetDiskType(int32_t diskType)
{
    diskType_ = diskType;
}

bool Disk::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(diskId_)) {
        return false;
    }

    if (!parcel.WriteInt64(sizeBytes_)) {
        return false;
    }

    if (!parcel.WriteString(sysPath_)) {
        return false;
    }

    if (!parcel.WriteString(vendor_)) {
        return false;
    }

    if (!parcel.WriteInt32(diskType_)) {
        return false;
    }
    return true;
}

Disk *Disk::Unmarshalling(Parcel &parcel)
{
    Disk* obj = new (std::nothrow) Disk();
    if (!obj) {
        return nullptr;
    }
    obj->diskId_ = parcel.ReadString();
    obj->sizeBytes_ = parcel.ReadInt64();
    obj->sysPath_ = parcel.ReadString();
    obj->vendor_ = parcel.ReadString();
    obj->diskType_ = parcel.ReadInt32();
    return obj;
}
}
}
