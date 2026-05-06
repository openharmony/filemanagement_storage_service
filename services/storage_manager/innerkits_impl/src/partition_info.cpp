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

#include "partition_info.h"

namespace OHOS {
namespace StorageManager {
PartitionInfo::PartitionInfo() {}

uint32_t PartitionInfo::GetPartitionNum() const
{
    return partitionNum_;
}

std::string PartitionInfo::GetDiskId() const
{
    return diskId_;
}

uint64_t PartitionInfo::GetStartSector() const
{
    return startSector_;
}

uint64_t PartitionInfo::GetEndSector() const
{
    return endSector_;
}

uint64_t PartitionInfo::GetSizeBytes() const
{
    return sizeBytes_;
}

std::string PartitionInfo::GetFsType() const
{
    return fsType_;
}

void PartitionInfo::SetPartitionNum(uint32_t partitionNum)
{
    partitionNum_ = partitionNum;
}

void PartitionInfo::SetDiskId(const std::string &diskId)
{
    diskId_ = diskId;
}

void PartitionInfo::SetStartSector(uint64_t startSector)
{
    startSector_ = startSector;
}

void PartitionInfo::SetEndSector(uint64_t endSector)
{
    endSector_ = endSector;
}

void PartitionInfo::SetSizeBytes(uint64_t sizeBytes)
{
    sizeBytes_ = sizeBytes;
}

void PartitionInfo::SetFsType(const std::string &fsType)
{
    fsType_ = fsType;
}

bool PartitionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(partitionNum_)) {
        return false;
    }
    if (!parcel.WriteString(diskId_)) {
        return false;
    }
    if (!parcel.WriteUint64(startSector_)) {
        return false;
    }
    if (!parcel.WriteUint64(endSector_)) {
        return false;
    }
    if (!parcel.WriteUint64(sizeBytes_)) {
        return false;
    }
    if (!parcel.WriteString(fsType_)) {
        return false;
    }
    return true;
}

PartitionInfo *PartitionInfo::Unmarshalling(Parcel &parcel)
{
    PartitionInfo* obj = new (std::nothrow) PartitionInfo();
    if (!obj) {
        return nullptr;
    }
    obj->partitionNum_ = parcel.ReadUint32();
    obj->diskId_ = parcel.ReadString();
    obj->startSector_ = parcel.ReadUint64();
    obj->endSector_ = parcel.ReadUint64();
    obj->sizeBytes_ = parcel.ReadUint64();
    obj->fsType_ = parcel.ReadString();
    return obj;
}
}
}