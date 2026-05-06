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

#ifndef OHOS_STORAGE_MANAGER_PARTITION_INFO_H
#define OHOS_STORAGE_MANAGER_PARTITION_INFO_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class PartitionInfo : public Parcelable {
public:
    PartitionInfo();

    uint32_t GetPartitionNum() const;
    std::string GetDiskId() const;
    uint64_t GetStartSector() const;
    uint64_t GetEndSector() const;
    uint64_t GetSizeBytes() const;
    std::string GetFsType() const;

    void SetPartitionNum(uint32_t partitionNum);
    void SetDiskId(const std::string &diskId);
    void SetStartSector(uint64_t startSector);
    void SetEndSector(uint64_t endSector);
    void SetSizeBytes(uint64_t sizeBytes);
    void SetFsType(const std::string &fsType);

    bool Marshalling(Parcel &parcel) const override;
    static PartitionInfo *Unmarshalling(Parcel &parcel);

private:
    uint32_t partitionNum_ {};
    std::string diskId_;
    uint64_t startSector_ {};
    uint64_t endSector_ {};
    uint64_t sizeBytes_ {};
    std::string fsType_;
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_PARTITION_INFO_H