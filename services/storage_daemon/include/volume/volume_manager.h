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

#ifndef OHOS_STORAGE_DAEMON_VOLUME_MANAGER_H
#define OHOS_STORAGE_DAEMON_VOLUME_MANAGER_H

#include <list>
#include <map>
#include <mutex>
#include <nocopyable.h>

#include "volume/volume_info.h"

namespace OHOS {
namespace StorageDaemon {
class VolumeManager final {
public:
    virtual ~VolumeManager() = default;
    static VolumeManager &Instance();

    std::string CreateVolume(const std::string &diskId, dev_t device, bool isUserdata, uint32_t partitionNum,
                             const std::string &extraInfo = "");
    int32_t DestroyVolume(const std::string &volId);

    int32_t Check(const std::string &volId);
    int32_t Mount(const std::string &volId, uint32_t flags);
    int32_t UMount(const std::string &volId);
    int32_t MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd);
    int32_t TryToFix(const std::string &volId, uint32_t flags);
    int32_t Format(const std::string &volId, const std::string &fsType);
    int32_t SetVolumeDescription(const std::string &volId, const std::string &description);
    int32_t QueryUsbIsInUse(const std::string &diskPath, bool &isInUse);
    int32_t GetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize);

    //disk crypt api
    int32_t Encrypt(const std::string &volumeId, const std::string &pazzword);
    int32_t GetCryptProgressById(const std::string &volumeId, int32_t &progress);
    int32_t GetCryptUuidById(const std::string &volumeId, std::string &uuid);
    int32_t BindRecoverKeyToPasswd(const std::string &volumeId,
                                const std::string &pazzword,
                                const std::string &recoverKey);
    int32_t UpdateCryptPasswd(const std::string &volumeId,
                            const std::string &pazzword,
                            const std::string &newPazzword);
    int32_t ResetCryptPasswd(const std::string &volumeId,
                            const std::string &recoverKey,
                            const std::string &newPazzword);
    int32_t VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword);
    int32_t Unlock(const std::string &volumeId, const std::string &pazzword);
    int32_t Decrypt(const std::string &volumeId, const std::string &pazzword);

    int32_t GetOpticalDriveOpsProgress(const std::string &volId, uint32_t &progress);
    int32_t Erase(const std::string &volId);
    int32_t CreateIsoImage(const std::string &volId, const std::string &filePath);
    bool IsDiskHasMountedVolume(std::string &diskId);
    std::string GetFsTypeByDiskIdAndPartNum(std::string &diskId, uint32_t partitionNum);
    int32_t Burn(const std::string &volumeId, const BurnParams &params);
    int32_t VerifyBurnData(const std::string &volumeId, uint32_t verType);

private:
    VolumeManager() = default;
    DISALLOW_COPY_AND_MOVE(VolumeManager);

    bool IsMtpDeviceInUse(const std::string &diskPath);
    std::map<std::string, std::shared_ptr<VolumeInfo>> volumes_;
    std::mutex volumesMutex_;
    std::shared_ptr<VolumeInfo> GetVolume(const std::string &volId);
    int32_t CreateMountUsbFusePath(std::string &fsUuid);
    int32_t ReadVolumeUuid(const std::string &volumeId, std::string &fsUuid);
    std::string mountUsbFusePath_;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_VOLUME_MANAGER_H