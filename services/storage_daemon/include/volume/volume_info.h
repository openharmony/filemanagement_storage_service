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

#ifndef OHOS_STORAGE_DAEMON_VOLUME_INFO_H
#define OHOS_STORAGE_DAEMON_VOLUME_INFO_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
enum VolumeState {
    UNMOUNTED,
    CHECKING,
    MOUNTED,
    EJECTING,
    REMOVED,
    BADREMOVABLE,
    DAMAGED,
    FUSE_REMOVED,
    DAMAGED_MOUNTED,
};

enum VolumeType {
    EXTERNAL,
};

class VolumeInfo {
public:
    VolumeInfo() = default;
    virtual ~VolumeInfo() = default;

    int32_t Create(const std::string volId, const std::string diskId, dev_t device, bool isUserdata);
    int32_t Destroy();
    void DestroyUsbFuse();
    int32_t Mount(uint32_t flags);
    int32_t UMount(bool force = false);
    int32_t UMountUsbFuse();
    int32_t Check();
    int32_t Format(const std::string type);
    int32_t SetVolumeDescription(const std::string description);
    int32_t TryToCheck();
    int32_t TryToFix();

    std::string GetVolumeId();
    int32_t GetVolumeType();
    std::string GetDiskId();
    int32_t GetState();
    bool GetIsUserdata();

protected:
    virtual int32_t DoCreate(dev_t dev) = 0;
    virtual int32_t DoDestroy() = 0;
    virtual int32_t DoMount(uint32_t mountFlags) = 0;
    virtual int32_t DoUMount(bool force) = 0;
    virtual int32_t DoUMountUsbFuse() = 0;
    virtual int32_t DoCheck() = 0;
    virtual int32_t DoFormat(std::string type) = 0;
    virtual int32_t DoSetVolDesc(std::string description) = 0;
    virtual int32_t DoTryToCheck() = 0;
    virtual int32_t DoTryToFix() = 0;

private:
    std::string id_;
    std::string diskId_;
    VolumeType type_;
    VolumeState mountState_;
    uint32_t mountFlags_;
    int32_t userIdOwner_;
    bool isUserdata_;
protected:
    bool isDamaged_ = false;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_VOLUME_INFO_H