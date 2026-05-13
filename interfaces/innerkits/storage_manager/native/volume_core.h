/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_VOLUME_CORE_H
#define OHOS_STORAGE_MANAGER_VOLUME_CORE_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
enum VolumeType {
    EMULATED = 1,
    EXTERNAL,
};
enum VolumeState {
    UNMOUNTED = 0,
    CHECKING,
    MOUNTED,
    EJECTING,
    REMOVED,
    BAD_REMOVAL,
    DAMAGED,
    FUSE_REMOVED,
    DAMAGED_MOUNTED,
    ENCRYPTING,
    ENCRYPTED_AND_LOCKED,
    ENCRYPTED_AND_UNLOCKED,
    DECRYPTING,
};

class VolumeInfoStr : public Parcelable {
public:
    VolumeInfoStr() {};
    
    VolumeInfoStr(const std::string &volumeId_, const std::string &fsTypeStr_, const std::string &fsUuid_,
                  const std::string &path_, const std::string &description_, bool isDamaged_)
        : volumeId(volumeId_),
          fsTypeStr(fsTypeStr_),
          fsUuid(fsUuid_),
          path(path_),
          description(description_),
          isDamaged(isDamaged_) {};
    
    std::string volumeId;
    std::string fsTypeStr;
    std::string fsUuid;
    std::string path;
    std::string description;
    bool isDamaged = false;

    bool Marshalling(Parcel &parcel) const override;
    static VolumeInfoStr *Unmarshalling(Parcel &parcel);
};

class VolumeCore : public Parcelable {
public:
    VolumeCore();
    VolumeCore(const std::string &id, int32_t type, const std::string &diskId);
    VolumeCore(const std::string &id, int32_t type, const std::string &diskId, int32_t state);
    VolumeCore(const std::string &id, int32_t type, const std::string &diskId, int32_t state,
               const std::string &fsType);

    std::string GetId() const;
    int32_t GetType() const;
    std::string GetDiskId() const;
    int32_t GetState() const;
    std::string GetFsType() const;
    void SetState(int32_t state);
    void SetFsType(std::string fsType);

    bool Marshalling(Parcel &parcel) const override;
    static VolumeCore *Unmarshalling(Parcel &parcel);
private:
    std::string id_;
    int32_t type_ {};
    std::string diskId_;
    int32_t state_ = UNMOUNTED;
    bool errorFlag_ = false;
    std::string fsType_;
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_VOLUME_CORE_H