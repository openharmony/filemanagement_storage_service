/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_DISK_H
#define OHOS_STORAGE_MANAGER_DISK_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
enum {
    SD_FLAG = 1,
    USB_FLAG = 2,
    CD_FLAG = 3,
};
class Disk : public Parcelable {
public:
    Disk();
    Disk(std::string diskId, int64_t sizeBytes, std::string sysPath, std::string vendor, int32_t flag);

    std::string GetDiskId() const;
    int64_t GetSizeBytes();
    std::string GetSysPath();
    std::string GetVendor();
    int32_t GetFlag();
    void SetFlag(int32_t flag);

    bool Marshalling(Parcel &parcel) const override;
    static Disk *Unmarshalling(Parcel &parcel);
private:
    std::string diskId_;
    int64_t sizeBytes_ {};
    std::string sysPath_;
    std::string vendor_;
    int32_t flag_ {};
};
} // STORAGE_MANAGER
} // OHOS

#endif // OHOS_STORAGE_MANAGER_DISK_H