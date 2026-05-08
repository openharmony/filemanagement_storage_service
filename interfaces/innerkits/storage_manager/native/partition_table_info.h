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

#ifndef OHOS_STORAGE_MANAGER_PARTITION_TABLE_INFO_H
#define OHOS_STORAGE_MANAGER_PARTITION_TABLE_INFO_H

#include "parcel.h"
#include "partition_info.h"
#include <vector>

namespace OHOS {
namespace StorageManager {
class PartitionTableInfo : public Parcelable {
public:
    PartitionTableInfo();

    std::string GetDiskId() const;
    std::string GetTableType() const;
    uint32_t GetPartitionCount() const;
    uint64_t GetTotalSector() const;
    uint32_t GetSectorSize() const;
    uint32_t GetAlignSector() const;
    std::vector<PartitionInfo> GetPartitions() const;

    void SetDiskId(const std::string &diskId);
    void SetTableType(const std::string &tableType);
    void SetPartitionCount(uint32_t partitionCount);
    void SetTotalSector(uint64_t totalSector);
    void SetSectorSize(uint32_t sectorSize);
    void SetAlignSector(uint32_t alignSector);
    void SetPartitions(const std::vector<PartitionInfo> &partitions);

    bool Marshalling(Parcel &parcel) const override;
    static PartitionTableInfo *Unmarshalling(Parcel &parcel);

private:
    std::string diskId_;
    std::string tableType_;
    uint32_t partitionCount_ {};
    uint64_t totalSector_ {};
    uint32_t sectorSize_ {};
    uint32_t alignSector_ {};
    std::vector<PartitionInfo> partitions_;
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_PARTITION_TABLE_INFO_H