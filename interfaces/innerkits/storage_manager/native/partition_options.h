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

#ifndef OHOS_STORAGE_MANAGER_PARTITION_OPTIONS_H
#define OHOS_STORAGE_MANAGER_PARTITION_OPTIONS_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class PartitionOptions : public Parcelable {
public:
    PartitionOptions();

    int32_t GetPartitionNum() const;
    uint64_t GetStartSector() const;
    uint64_t GetEndSector() const;
    std::string GetTypeCode() const;

    void SetPartitionNum(int32_t partitionNum);
    void SetStartSector(uint64_t startSector);
    void SetEndSector(uint64_t endSector);
    void SetTypeCode(const std::string &typeCode);

    bool Marshalling(Parcel &parcel) const override;
    static PartitionOptions *Unmarshalling(Parcel &parcel);

private:
    int32_t partitionNum_ {};
    uint64_t startSector_ {};
    uint64_t endSector_ {};
    std::string typeCode_;
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_PARTITION_OPTIONS_H