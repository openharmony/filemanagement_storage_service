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

#ifndef OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_H
#define OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_H

#include <map>
#include <mutex>
#include "burn_params.h"
#include "volume_external.h"

namespace OHOS {
namespace StorageManager {
class VolumeManagerService {
public:
    static VolumeManagerService &GetInstance(void)
    {
        static VolumeManagerService instance;
        return instance;
    }
    void SetUsbDescription(void);
    int32_t Mount(std::string volumeId);
    int32_t Unmount(std::string volumeId);
    int32_t MountUsbFuse(const std::string &volumeId);
    bool IsUsbFuseByType(const std::string &fsType);
    int32_t TryToFix(std::string volumeId);
    void OnVolumeCreated(VolumeCore vc);
    void OnVolumeMounted(const VolumeInfoStr &volumeInfoStr);
    void OnVolumeDamaged(const VolumeInfoStr &volumeInfoStr);
    void OnVolumeStateChanged(std::string volumeId, VolumeState state);
    std::vector<VolumeExternal> GetAllVolumes();
    std::shared_ptr<VolumeExternal> GetVolumeByUuid(std::string volumeUuid);
    int32_t GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc);
    int32_t GetVolumeById(std::string volumeId, VolumeExternal &vc);
    int32_t SetVolumeDescription(std::string fsUuid, std::string description);
    int32_t Format(std::string volumeId, std::string fsType);
    void NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                          const std::string &uuid, const std::string &fsType);
    void NotifyMtpUnmounted(const std::string &id, const bool isBadRemove);
    void NotifyEncryptVolumeStateChanged(const VolumeInfoStr &volumeInfoStr);

    int32_t Eject(const std::string &volumeId);
    int32_t GetOpticalDriveOpsProgress(const std::string &volumeId, uint32_t &progress);
    int32_t Erase(const std::string &volumeId);
    int32_t CreateIsoImage(const std::string &volumeId, const std::string &filePath);
    int32_t Burn(const std::string &volumeId, const BurnParams &params);
    int32_t VerifyBurnData(const std::string &volumeId, uint32_t verType);
private:
    VolumeManagerService();
    ~VolumeManagerService();
    VolumeManagerService(const VolumeManagerService &) = delete;
    VolumeManagerService &operator=(const VolumeManagerService &) = delete;
    VolumeManagerService(const VolumeManagerService &&) = delete;
    VolumeManagerService &operator=(const VolumeManagerService &&) = delete;
    std::map<std::string, std::shared_ptr<VolumeExternal>> volumeMap_;
    std::mutex volumeMapMutex_;
    void VolumeStateNotify(VolumeState state, std::shared_ptr<VolumeExternal> volume);
    int32_t Check(std::string volumeId);
    void SaveVolumeFreeSize(std::shared_ptr<VolumeExternal> volume);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_H
