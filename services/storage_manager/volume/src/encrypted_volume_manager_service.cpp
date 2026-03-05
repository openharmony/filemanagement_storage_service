/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "volume/encrypted_volume_manager_service.h"
#include "volume/volume_manager_service.h"

#include "disk.h"
#include "disk/disk_manager_service.h"
#include "parameters.h"
#include "safe_map.h"
#include "securec.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include <sys/xattr.h>
#include "utils/storage_radar.h"
#include "utils/storage_utils.h"
#include "utils/file_utils.h"
#include "utils/disk_utils.h"
#include "utils/string_utils.h"
#include "volume/notification.h"

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {

EncryptedVolumeManagerService::EncryptedVolumeManagerService() {}
EncryptedVolumeManagerService::~EncryptedVolumeManagerService() {}


int32_t EncryptedVolumeManagerService::Encrypt(const std::string &volumeId, const std::string &pazzword)
{
    VolumeExternal volumePtr;
    int32_t result = VolumeManagerService::GetInstance().GetVolumeById(volumeId, volumePtr);
    if (result != E_OK) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_VOLUMEEX_IS_NULLPTR;
    }
    if (volumePtr.GetState() != VolumeState::MOUNTED) {
        LOGE("EncryptedVolumeManagerService::The type of volume(Id %{public}s) is not mounted", volumeId.c_str());
        return E_VOL_MOUNT_ERR;
    }

    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("sdCommunication is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    volumePtr.SetState(VolumeState::ENCRYPTING);
    result = sdCommunication->Encrypt(volumeId, pazzword);
    if (result != E_OK) {
        volumePtr.SetState(VolumeState::MOUNTED);
        return result;
    }
    return result;
}

int32_t EncryptedVolumeManagerService::GetCryptProgressById(const std::string &volumeId, int32_t &progress)
{
    VolumeExternal volumePtr;
    int32_t result = VolumeManagerService::GetInstance().GetVolumeById(volumeId, volumePtr);
    if (result != E_OK) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_VOLUMEEX_IS_NULLPTR;
    }
    if ((volumePtr.GetState() != VolumeState::ENCRYPTING) &&
        (volumePtr.GetState() != VolumeState::DECRYPTING)) {
        LOGE("EncryptedVolumeManagerService::The type of volume(Id %{public}s) is not encrypting or decrypting",
            volumeId.c_str());
        return E_VOL_MOUNT_ERR;
    }

    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("sdCommunication is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    result = sdCommunication->GetCryptProgressById(volumeId, progress);
    if (result != E_OK) {
        return result;
    }
    return result;
}
} // StorageManager
} // OHOS
