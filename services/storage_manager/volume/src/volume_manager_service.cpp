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

#include "volume/volume_manager_service.h"

#include "disk.h"
#include "disk/disk_manager_service.h"
#include "safe_map.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "volume/notification.h"

using namespace std;

namespace OHOS {
namespace StorageManager {
    VolumeManagerService::VolumeManagerService() {}
    VolumeManagerService::~VolumeManagerService() {}

    void VolumeManagerService::VolumeStateNotify(int32_t state, string volumeId, string diskId,
        string fsUuid, string path)
    {
        DelayedSingleton<Notification>::GetInstance()->NotifyVolumeChange(state, volumeId, diskId, fsUuid, path);
    }

    void VolumeManagerService::OnVolumeCreated(VolumeCore vc)
    {
        auto volumePtr = make_shared<VolumeExternal>(vc);
        volumeMap_.Insert(volumePtr->GetId(), volumePtr);
        Mount(volumePtr->GetId());
    }

    void VolumeManagerService::OnVolumeDestroyed(string volumeId)
    {
        if (!volumeMap_.Contains(volumeId)) {
            LOGE("VolumeManagerService::OnVolumeDestroyed volumeId %{public}s not exists", volumeId.c_str());
            return;
        }
        std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[volumeId];
        int32_t state = VOLUME_REMOVED;
        if (volumePtr->GetState() != VolumeState::UNMOUNTED) {
            state =  VOLUME_BAD_REMOVAL;
        }
        VolumeStateNotify(state, volumeId, volumePtr->GetDiskId(), "", "");
        volumeMap_.Erase(volumeId);
    }

    void VolumeManagerService::OnVolumeMounted(std::string volumeId, int fsType, std::string fsUuid,
        std::string path, std::string description)
    {
        if (!volumeMap_.Contains(volumeId)) {
            LOGE("VolumeManagerService::OnVolumeMounted volumeId %{public}s not exists", volumeId.c_str());
            return;
        }
        std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[volumeId];
        volumePtr->SetFsType(fsType);
        volumePtr->SetFsUuid(fsUuid);
        volumePtr->SetPath(path);
        std::string des = description;
        if (des == "") {
            auto disk = DelayedSingleton<DiskManagerService>::GetInstance()->GetDiskById(volumePtr->GetDiskId());
            if (disk != nullptr) {
                if (disk->GetFlag() == SD_FLAG) {
                    des = "MySDCard";
                } else if (disk->GetFlag() == USB_FLAG) {
                    des = "MyUSB";
                } else {
                    des = "Default";
                }
            }
        }
        volumePtr->SetDescription(des);
        volumePtr->SetState(VolumeState::MOUNTED);
        VolumeStateNotify(VOLUME_MOUNTED, volumeId, volumePtr->GetDiskId(), fsUuid, path);
    }

    int32_t VolumeManagerService::Mount(std::string volumeId)
    {
        if (!volumeMap_.Contains(volumeId)) {
            LOGE("VolumeManagerService::Mount volumeId %{public}s not exists", volumeId.c_str());
            return E_NON_EXIST;
        }
        std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[volumeId];
        if (volumePtr->GetState() != VolumeState::UNMOUNTED) {
            LOGE("VolumeManagerService::The type of volume(Id %{public}s) is not unmounted", volumeId.c_str());
            return E_MOUNT;
        }
        std::shared_ptr<StorageDaemonCommunication> sdCommunication;
        sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
        int32_t result = Check(volumePtr->GetId());
        if (result == E_OK) {
            result = sdCommunication->Mount(volumeId, 0);
            if (result != E_OK) {
                volumePtr->SetState(VolumeState::UNMOUNTED);
            }
        } else {
            volumePtr->SetState(VolumeState::UNMOUNTED);
        }
        return result;
    }

    int32_t VolumeManagerService::Unmount(std::string volumeId)
    {
        if (!volumeMap_.Contains(volumeId)) {
            LOGE("VolumeManagerService::Unmount volumeId %{public}s not exists", volumeId.c_str());
            return E_NON_EXIST;
        }
        std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[volumeId];
        if (volumePtr->GetState() != VolumeState::MOUNTED) {
            LOGE("VolumeManagerService::The type of volume(Id %{public}s) is not mounted", volumeId.c_str());
            return E_UMOUNT;
        }
        std::shared_ptr<StorageDaemonCommunication> sdCommunication;
        sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
        volumePtr->SetState(VolumeState::EJECTING);
        VolumeStateNotify(VOLUME_EJECT, volumeId, volumePtr->GetDiskId(), "", "");
        int32_t result = sdCommunication->Unmount(volumeId);
        if (result == E_OK) {
            volumePtr->SetState(VolumeState::UNMOUNTED);
            volumePtr->Reset();
            VolumeStateNotify(VOLUME_UNMOUNTED, volumeId, volumePtr->GetDiskId(), "", "");
        } else {
            volumePtr->SetState(VolumeState::MOUNTED);
        }
        return result;
    }

    int32_t VolumeManagerService::Check(std::string volumeId)
    {
        std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[volumeId];
        std::shared_ptr<StorageDaemonCommunication> sdCommunication;
        sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
        volumePtr->SetState(VolumeState::CHECKING);
        int32_t result = sdCommunication->Check(volumeId);
        return result;
    }

    vector<VolumeExternal> VolumeManagerService::GetAllVolumes()
    {
        vector<VolumeExternal> result;
        for (auto it = volumeMap_.Begin(); it != volumeMap_.End(); ++it) {
            VolumeExternal vc = *(it->second);
            result.push_back(vc);
        }
        return result;
    }

    std::shared_ptr<VolumeExternal> VolumeManagerService::GetVolumeByUuid(std::string volumeUuid)
    {
        for (auto it = volumeMap_.Begin(); it != volumeMap_.End(); ++it) {
            auto vc = it->second;
            if (vc->GetUuid() == volumeUuid) {
                LOGE("VolumeManagerService::GetVolumeByUuid volumeUuid %{public}s exists", volumeUuid.c_str());
                return vc;
            }
        }
        return nullptr;
    }

    int32_t VolumeManagerService::GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc)
    {
        for (auto it = volumeMap_.Begin(); it != volumeMap_.End(); ++it) {
            auto volume = it->second;
            if (volume->GetUuid() == fsUuid) {
                LOGI("VolumeManagerService::GetVolumeByUuid volumeUuid %{public}s exists", fsUuid.c_str());
                vc = *volume;
                return E_OK;
            }
        }
        return E_NON_EXIST;
    }

    int32_t VolumeManagerService::GetVolumeById(std::string volumeId, VolumeExternal &vc)
    {
        if (volumeMap_.Contains(volumeId)) {
            vc = *volumeMap_[volumeId];
            return E_OK;
        }
        return E_NON_EXIST;
    }

    int32_t VolumeManagerService::SetVolumeDescription(std::string fsUuid, std::string description)
    {
        for (auto it = volumeMap_.Begin(); it != volumeMap_.End(); ++it) {
            auto volume = it->second;
            if (volume->GetUuid() == fsUuid) {
                LOGI("VolumeManagerService::SetVolumeDescription volumeUuid %{public}s exists", fsUuid.c_str());
                if (volume->GetState() != VolumeState::UNMOUNTED) {
                    LOGE("VolumeManagerService::SetVolumeDescription volume state is not unmounted!");
                    return E_VOL_STATE;
                }
                std::shared_ptr<StorageDaemonCommunication> sdCommunication;
                sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
                return sdCommunication->SetVolumeDescription(volume->GetId(), description);
            }
        }
        return E_NON_EXIST;
    }

    int32_t VolumeManagerService::Format(std::string volumeId, std::string fsType)
    {
        if (volumeMap_.Find(volumeId) == volumeMap_.End()) {
            return E_NON_EXIST;
        }
        if (volumeMap_[volumeId]->GetState() != VolumeState::UNMOUNTED) {
            LOGE("VolumeManagerService::SetVolumeDescription volume state is not unmounted!");
            return E_VOL_STATE;
        }
        // check fstype!!!!
        std::shared_ptr<StorageDaemonCommunication> sdCommunication;
        sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
        return sdCommunication->Format(volumeId, fsType);
    }
} // StorageManager
} // OHOS
