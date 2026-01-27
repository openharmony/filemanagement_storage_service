/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <map>
#include "volume_core.h"

namespace OHOS {
namespace StorageManager {
enum FsType {
    UNDEFINED = -1,
    NTFS,
    EXFAT,
    VFAT,
    HMFS,
    F2FS,
    MTP,
    UDF,
    ISO9660
};

static std::map<int32_t, std::string> FS_TYPE_MAP = {
    {NTFS, "ntfs"},
    {EXFAT, "exfat"},
    {VFAT, "vfat"},
    {HMFS, "hmfs"},
    {F2FS, "f2fs"},
    {MTP, "mtp"},
    {UDF, "udf"},
    {ISO9660, "iso9660"},
};

class VolumeExternal : public VolumeCore {
public:
    VolumeExternal();
    VolumeExternal(VolumeCore vc);

    void SetFlags(int32_t flags);
    void SetFsType(int32_t fsType);
    void SetFsUuid(std::string fsUuid);
    void SetPath(std::string path);
    void SetDescription(std::string description);
    int32_t GetFlags();
    int32_t GetFsType();
    std::string GetFsTypeString();
    std::string GetUuid();
    std::string GetPath();
    std::string GetDescription();
    int32_t GetFsTypeByStr(const std::string &fsTypeStr);
    void Reset();

    bool Marshalling(Parcel &parcel) const override;
    static VolumeExternal *Unmarshalling(Parcel &parcel);
private:
    int32_t fsType_ { UNDEFINED };
    int32_t flags_ {};
    std::string fsUuid_;
    std::string path_;
    std::string description_;
};
} // OHOS
} // StorageManager

#endif // OHOS_STORAGE_MANAGER_VOLUME_EXTERNAL_H