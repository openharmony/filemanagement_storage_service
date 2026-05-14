/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STORAGE_MANAGER_BURN_PARAMS_H
#define STORAGE_MANAGER_BURN_PARAMS_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace StorageManager {
struct BurnParams : public Parcelable {
    std::string diskName;
    std::string burnPath;
    std::string fsType;
    uint32_t burnSpeed;
    bool isIsoImage;
    bool isIncBurnSupport;

    BurnParams() : burnSpeed(0), isIsoImage(false), isIncBurnSupport(false) {}
    BurnParams(const std::string &diskName, const std::string &burnPath, const std::string &fsType,
               uint32_t burnSpeed, bool isIsoImage, bool isIncBurnSupport)
        : diskName(diskName),
          burnPath(burnPath),
          fsType(fsType),
          burnSpeed(burnSpeed),
          isIsoImage(isIsoImage),
          isIncBurnSupport(isIncBurnSupport) {}

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(diskName) &&
               parcel.WriteString(burnPath) &&
               parcel.WriteString(fsType) &&
               parcel.WriteUint32(burnSpeed) &&
               parcel.WriteBool(isIsoImage) &&
               parcel.WriteBool(isIncBurnSupport);
    }

    static BurnParams *Unmarshalling(Parcel &parcel)
    {
        auto *params = new (std::nothrow) BurnParams();
        if (params == nullptr) {
            return nullptr;
        }
        if (!parcel.ReadString(params->diskName) ||
            !parcel.ReadString(params->burnPath) ||
            !parcel.ReadString(params->fsType) ||
            !parcel.ReadUint32(params->burnSpeed) ||
            !parcel.ReadBool(params->isIsoImage) ||
            !parcel.ReadBool(params->isIncBurnSupport)) {
            delete params;
            return nullptr;
        }
        return params;
    }
};
} // namespace StorageManager
} // namespace OHOS

#endif