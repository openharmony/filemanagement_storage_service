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

#ifndef OHOS_STORAGE_DAEMON_VOLUME_MANAGER_H
#define OHOS_STORAGE_DAEMON_VOLUME_MANAGER_H

#include <nocopyable.h>
#include <sys/types.h>
#include <string>
#include <map>
#include <memory>
#include "volume/volume_info.h"

namespace OHOS {
namespace StorageDaemon {
class VolumeManager final {
public:
    virtual ~VolumeManager() = default;
    static VolumeManager* Instance();

    std::string CreateVolume(const std::string diskId, dev_t device);
    int32_t DestroyVolume(const std::string volId);

    int32_t Check(const std::string volId);
    int32_t Mount(const std::string volId, uint32_t flags);
    int32_t UMount(const std::string volId);
    int32_t Format(const std::string volId, const std::string fsType);
    int32_t SetVolumeDescription(const std::string volId, const std::string description);

private:
    VolumeManager() = default;
    DISALLOW_COPY_AND_MOVE(VolumeManager);

    static VolumeManager* instance_;
    std::map<std::string, std::shared_ptr<VolumeInfo>> volumes_;

    std::shared_ptr<VolumeInfo> GetVolume(const std::string volId);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_VOLUME_MANAGER_H