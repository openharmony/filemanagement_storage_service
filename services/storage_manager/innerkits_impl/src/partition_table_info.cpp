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

#include "partition_table_info.h"

namespace OHOS {
namespace StorageManager {
PartitionTableInfo::PartitionTableInfo() {}

std::string PartitionTableInfo::GetDiskId() const
{
    return diskId_;
}

std::string PartitionTableInfo::GetTableType() const
{
    return tableType_;
}

uint32_t PartitionTableInfo::GetPartitionCount() const
{
    return partitionCount_;
}

uint64_t PartitionTableInfo::GetTotalSector() const
{
    return totalSector_;
}

uint32_t PartitionTableInfo::GetSectorSize() const
{
    return sectorSize_;
}

uint32_t PartitionTableInfo::GetAlignSector() const
{
    return alignSector_;
}

std::vector<PartitionInfo> PartitionTableInfo::GetPartitions() const
{
    return partitions_;
}

void PartitionTableInfo::SetDiskId(const std::string &diskId)
{
    diskId_ = diskId;
}

void PartitionTableInfo::SetTableType(const std::string &tableType)
{
    tableType_ = tableType;
}

void PartitionTableInfo::SetPartitionCount(uint32_t partitionCount)
{
    partitionCount_ = partitionCount;
}

void PartitionTableInfo::SetTotalSector(uint64_t totalSector)
{
    totalSector_ = totalSector;
}

void PartitionTableInfo::SetSectorSize(uint32_t sectorSize)
{
    sectorSize_ = sectorSize;
}

void PartitionTableInfo::SetAlignSector(uint32_t alignSector)
{
    alignSector_ = alignSector;
}

void PartitionTableInfo::SetPartitions(const std::vector<PartitionInfo> &partitions)
{
    partitions_ = partitions;
}

bool PartitionTableInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(diskId_)) {
        return false;
    }
    if (!parcel.WriteString(tableType_)) {
        return false;
    }
    if (!parcel.WriteUint32(partitionCount_)) {
        return false;
    }
    if (!parcel.WriteUint64(totalSector_)) {
        return false;
    }
    if (!parcel.WriteUint32(sectorSize_)) {
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(partitions_.size()))) {
        return false;
    }
    for (const auto &partition : partitions_) {
        if (!partition.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

PartitionTableInfo *PartitionTableInfo::Unmarshalling(Parcel &parcel)
{
    PartitionTableInfo* obj = new (std::nothrow) PartitionTableInfo();
    if (!obj) {
        return nullptr;
    }
    obj->diskId_ = parcel.ReadString();
    obj->tableType_ = parcel.ReadString();
    obj->partitionCount_ = parcel.ReadUint32();
    obj->totalSector_ = parcel.ReadUint64();
    obj->sectorSize_ = parcel.ReadUint32();
    obj->alignSector_ = parcel.ReadUint32();

    uint32_t partitionSize = parcel.ReadUint32();
    for (uint32_t i = 0; i < partitionSize; i++) {
        PartitionInfo* partition = PartitionInfo::Unmarshalling(parcel);
        if (partition == nullptr) {
            delete obj;
            return nullptr;
        }
        obj->partitions_.push_back(*partition);
        delete partition;
    }
    return obj;
}
}
}