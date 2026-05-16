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

#include "format_params.h"

namespace OHOS {
namespace StorageManager {
FormatParams::FormatParams() {}

std::string FormatParams::GetFsType() const
{
    return fsType_;
}

bool FormatParams::GetQuickFormat() const
{
    return quickFormat_;
}

std::string FormatParams::GetVolumeName() const
{
    return volumeName_;
}

void FormatParams::SetFsType(const std::string &fsType)
{
    fsType_ = fsType;
}

void FormatParams::SetQuickFormat(bool quickFormat)
{
    quickFormat_ = quickFormat;
}

void FormatParams::SetVolumeName(const std::string &volumeName)
{
    volumeName_ = volumeName;
}

bool FormatParams::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(fsType_)) {
        return false;
    }
    if (!parcel.WriteBool(quickFormat_)) {
        return false;
    }
    if (!parcel.WriteString(volumeName_)) {
        return false;
    }
    return true;
}

FormatParams *FormatParams::Unmarshalling(Parcel &parcel)
{
    FormatParams* obj = new (std::nothrow) FormatParams();
    if (!obj) {
        return nullptr;
    }
    obj->fsType_ = parcel.ReadString();
    obj->quickFormat_ = parcel.ReadBool();
    obj->volumeName_ = parcel.ReadString();
    return obj;
}
} // namespace StorageManager
} // namespace OHOS