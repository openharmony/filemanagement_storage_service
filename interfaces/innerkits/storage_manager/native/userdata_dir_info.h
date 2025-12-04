/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_USERDATA_DIR_INFO_H
#define OHOS_STORAGE_MANAGER_USERDATA_DIR_INFO_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class UserdataDirInfo final : public Parcelable {
public:
    UserdataDirInfo() {}
    UserdataDirInfo(std::string path, int64_t totalSize, int totalCnt)
        : path_(path), totalSize_(totalSize), totalCnt_(totalCnt) {}
    ~UserdataDirInfo() {}

    std::string path_{""};
    int64_t totalSize_{0};
    int32_t totalCnt_{0};

    bool Marshalling(Parcel &parcel) const override;
    static UserdataDirInfo *Unmarshalling(Parcel &parcel);
};
} // StorageMangaer
} // OHOS

#endif // OHOS_STORAGE_MANAGER_USERDATA_DIR_INFO_H
