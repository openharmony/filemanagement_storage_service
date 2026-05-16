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

#include "partition_params.h"

namespace OHOS {
namespace StorageManager {
PartitionParams::PartitionParams() {}

int32_t PartitionParams::GetPartitionNum() const
{
    return partitionNum_;
}

uint64_t PartitionParams::GetStartSector() const
{
    return startSector_;
}

uint64_t PartitionParams::GetEndSector() const
{
    return endSector_;
}

std::string PartitionParams::GetTypeCode() const
{
    return typeCode_;
}

void PartitionParams::SetPartitionNum(int32_t partitionNum)
{
    partitionNum_ = partitionNum;
}

void PartitionParams::SetStartSector(uint64_t startSector)
{
    startSector_ = startSector;
}

void PartitionParams::SetEndSector(uint64_t endSector)
{
    endSector_ = endSector;
}

void PartitionParams::SetTypeCode(const std::string &typeCode)
{
    typeCode_ = typeCode;
}

bool PartitionParams::Marshalling(Parcel &parcel) const
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

PartitionParams *PartitionParams::Unmarshalling(Parcel &parcel)
{
    PartitionParams* obj = new (std::nothrow) PartitionParams();
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