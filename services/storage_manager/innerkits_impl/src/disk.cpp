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
constexpr unsigned int MAX_VOLUME_COUNT = 256;
Disk::Disk() {}

Disk::Disk(const std::string &diskId, int64_t sizeBytes, int32_t diskType, bool removable_,
            const std::list<std::string> &volumeIds, const std::string &extraInfo, const std::string &vendor,
            const std::string &sysPath)
    : diskId_(diskId), sizeBytes_(sizeBytes), diskType_(diskType), removable_(removable_), volumeIds_(volumeIds),
    extraInfo_(extraInfo), vendor_(vendor), sysPath_(sysPath) {}

std::string Disk::GetDiskId() const
{
    return diskId_;
}

int64_t Disk::GetSizeBytes() const
{
    return sizeBytes_;
}

int32_t Disk::GetDiskType() const
{
    return diskType_;
}

bool Disk::GetRemovable() const
{
    return removable_;
}

std::list<std::string> Disk::GetVolumeIds() const
{
    return volumeIds_;
}

std::string Disk::GetExtraInfo() const
{
    return extraInfo_;
}

std::string Disk::GetVendor() const
{
    return vendor_;
}

std::string Disk::GetSysPath() const
{
    return sysPath_;
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
    if (!parcel.WriteInt32(diskType_)) {
        return false;
    }
    if (!parcel.WriteBool(removable_)) {
        return false;
    }
    if (!parcel.WriteString(extraInfo_)) {
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(volumeIds_.size()))) {
        return false;
    }
    for (const auto &item: volumeIds_) {
        if (!parcel.WriteString(item)) {
            return false;
        }
    }
    return true;
}

Disk *Disk::Unmarshalling(Parcel &parcel)
{
    Disk* obj = new (std::nothrow) Disk();
    if (!obj) {
        return nullptr;
    }
    if (!parcel.ReadString(obj->diskId_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadInt64(obj->sizeBytes_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadInt32(obj->diskType_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadBool(obj->removable_)) {
        delete obj;
        return nullptr;
    }
    if (!parcel.ReadString(obj->extraInfo_)) {
        delete obj;
        return nullptr;
    }
    uint32_t volSize;
    if (!parcel.ReadUint32(volSize)) {
        delete obj;
        return nullptr;
    }
    if (volSize >= MAX_VOLUME_COUNT) {
        delete obj;
        return nullptr;
    }
    for (uint32_t i = 0; i < volSize; i++) {
        std::string volId;
        if (!parcel.ReadString(volId)) {
            delete obj;
            return nullptr;
        }
        obj->volumeIds_.push_back(volId);
    }
    return obj;
}
}
}
