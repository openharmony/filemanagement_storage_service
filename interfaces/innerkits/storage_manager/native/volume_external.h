/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_VOLUME_EXTERNAL_H
#define OHOS_STORAGE_MANAGER_VOLUME_EXTERNAL_H

#include "volume_core.h"

namespace OHOS {
namespace StorageManager {
enum FsType {
    UNDEFINED = -1,
    EXT2,
    EXT3,
    EXT4,
    NTFS,
    EXFAT,
    VFAT
};
class VolumeExternal : public VolumeCore {
public:
    VolumeExternal();
    VolumeExternal(VolumeCore vc);

    void SetFsType(int32_t fsType);
    void SetFsUuid(std::string fsUuid);
    void SetPath(std::string path);
    void SetDescription(std::string description);
    int32_t GetFsType();
    std::string GetUuid();
    std::string GetPath();
    std::string GetDescription();
    void Reset();

    bool Marshalling(Parcel &parcel) const override;
    static std::unique_ptr<VolumeExternal> Unmarshalling(Parcel &parcel);
private:
    int32_t fsType_ { UNDEFINED };
    std::string fsUuid_;
    std::string path_;
    std::string description_;
};
} // OHOS
} // StorageManager

#endif // OHOS_STORAGE_MANAGER_VOLUME_EXTERNAL_H