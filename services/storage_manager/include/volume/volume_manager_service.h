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

#ifndef OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_H
#define OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_H

#include <singleton.h>
#include <nocopyable.h>
#include "volume_core.h"
#include "volume_external.h"
#include "utils/storage_rl_map.h"

namespace OHOS {
namespace StorageManager {
class VolumeManagerService : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(VolumeManagerService);
public:
    int32_t Mount(std::string volumeId);
    int32_t Unmount(std::string volumeId);
    void OnVolumeCreated(VolumeCore vc);
    void OnVolumeMounted(std::string volumeId, int32_t fsType, std::string fsUuid,
        std::string path, std::string description);
    void OnVolumeDestroyed(std::string volumeId);
    std::vector<VolumeExternal> GetAllVolumes();
    std::shared_ptr<VolumeExternal> GetVolumeByUuid(std::string volumeUuid);
    int32_t GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc);
    int32_t GetVolumeById(std::string volumeId, VolumeExternal &vc);
    int32_t SetVolumeDescription(std::string fsUuid, std::string description);
    int32_t Format(std::string volumeId, std::string fsType);
private:
    StorageRlMap<std::string, std::shared_ptr<VolumeExternal>> volumeMap_;
    void VolumeStateNotify(int32_t state, std::string volumeId,
        std::string diskId, std::string fsUuid, std::string path);
    int32_t Check(std::string volumeId);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_H
