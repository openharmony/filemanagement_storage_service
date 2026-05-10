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

#include "partition_options.h"

namespace OHOS {
namespace StorageManager {
PartitionOptions::PartitionOptions() {}

int32_t PartitionOptions::GetPartitionNum() const
{
    return partitionNum_;
}

uint64_t PartitionOptions::GetStartSector() const
{
    return startSector_;
}

uint64_t PartitionOptions::GetEndSector() const
{
    return endSector_;
}

std::string PartitionOptions::GetTypeCode() const
{
    return typeCode_;
}

void PartitionOptions::SetPartitionNum(int32_t partitionNum)
{
    partitionNum_ = partitionNum;
}

void PartitionOptions::SetStartSector(uint64_t startSector)
{
    startSector_ = startSector;
}

void PartitionOptions::SetEndSector(uint64_t endSector)
{
    endSector_ = endSector;
}

void PartitionOptions::SetTypeCode(std::string typeCode)
{
    typeCode_ = typeCode;
}

bool PartitionOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(partitionNum_)) {
        return false;
    }
    if (!parcel.WriteUint64(startSector_)) {
        return false;
    }
    if (!parcel.WriteUint64(endSector_)) {
        return false;
    }
    if (!parcel.WriteString(typeCode_)) {
        return false;
    }
    return true;
}

PartitionOptions *PartitionOptions::Unmarshalling(Parcel &parcel)
{
    PartitionOptions* obj = new (std::nothrow) PartitionOptions();
    if (!obj) {
        return nullptr;
    }
    obj->partitionNum_ = parcel.ReadInt32();
    obj->startSector_ = parcel.ReadUint64();
    obj->endSector_ = parcel.ReadUint64();
    obj->typeCode_ = parcel.ReadString();
    return obj;
}
}
}