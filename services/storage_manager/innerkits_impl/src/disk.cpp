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

#include "disk.h"

namespace OHOS {
namespace StorageManager {
Disk::Disk() {}

Disk::Disk(std::string diskId, int64_t sizeBytes, std::string sysPath, std::string vendor, int32_t flag)
    : diskId_(diskId), sizeBytes_(sizeBytes), sysPath_(sysPath), vendor_(vendor), flag_(flag) {}

std::string Disk::GetDiskId()
{
    return diskId_;
}

int64_t Disk::GetSizeBytes()
{
    return sizeBytes_;
}

std::string Disk::GetSysPath()
{
    return sysPath_;
}

std::string Disk::GetVendor()
{
    return vendor_;
}

int32_t Disk::GetFlag()
{
    return flag_;
}

void Disk::SetFlag(int32_t flag)
{
    flag_ = flag;
}

bool Disk::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(diskId_)) {
        return false;
    }

    if (!parcel.WriteInt32(sizeBytes_)) {
        return false;
    }

    if (!parcel.WriteString(sysPath_)) {
        return false;
    }

    if (!parcel.WriteString(vendor_)) {
        return false;
    }

    if (!parcel.WriteInt32(flag_)) {
        return false;
    }

    return true;
}

std::unique_ptr<Disk> Disk::Unmarshalling(Parcel &parcel)
{
    auto obj = std::make_unique<Disk>();
    obj->diskId_ = parcel.ReadString();
    obj->sizeBytes_ = parcel.ReadInt32();
    obj->sysPath_ = parcel.ReadString();
    obj->vendor_ = parcel.ReadString();
    obj->flag_ = parcel.ReadInt32();
    return obj;
}
}
}
