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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_DAEMON_COMMUNICATION_H
#define OHOS_STORAGE_MANAGER_STORAGE_DAEMON_COMMUNICATION_H

#include "istorage_manager.h"
#include "disk/disk_info.h"
#include "volume/volume_info.h"

namespace OHOS {
namespace StorageDaemon {
class StorageManagerClient final {
public:
    StorageManagerClient() = default;

    int32_t NotifyDiskCreated(DiskInfo &diskInfo);
    int32_t NotifyDiskDestroyed(std::string id);

    int32_t NotifyVolumeCreated(std::shared_ptr<VolumeInfo> info);
    int32_t NotifyVolumeMounted(std::shared_ptr<VolumeInfo> volumeInfo);
    int32_t NotifyVolumeDamaged(std::shared_ptr<VolumeInfo> volumeInfo);
    int32_t NotifyVolumeStateChanged(std::string volId, StorageManager::VolumeState state);

    int32_t NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                             const std::string &uuid);
    int32_t NotifyMtpUnmounted(const std::string &id, const std::string &path, const bool isBadRemove);

    int32_t IsUsbFuseByType(const std::string &fsType, bool &enabled);

private:
    DISALLOW_COPY_AND_MOVE(StorageManagerClient);

    int32_t GetClient();

    sptr<OHOS::StorageManager::IStorageManager> storageManager_;
};
} // StorageManager
} // OHOS

#endif