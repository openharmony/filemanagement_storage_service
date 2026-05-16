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

#ifndef OHOS_STORAGE_MANAGER_FORMAT_PARAMS_H
#define OHOS_STORAGE_MANAGER_FORMAT_PARAMS_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class FormatParams : public Parcelable {
public:
    FormatParams();

    std::string GetFsType() const;
    bool GetQuickFormat() const;
    std::string GetVolumeName() const;

    void SetFsType(const std::string &fsType);
    void SetQuickFormat(bool quickFormat);
    void SetVolumeName(const std::string &volumeName);

    bool Marshalling(Parcel &parcel) const override;
    static FormatParams *Unmarshalling(Parcel &parcel);

private:
    std::string fsType_ {"hmfs"};
    bool quickFormat_ {true};
    std::string volumeName_;
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_FORMAT_PARAMS_H