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
#include "utils/storage_radar.h"
#include "utils/storage_utils.h"
#include "volume/notification.h"

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
VolumeManagerService::VolumeManagerService() {}
VolumeManagerService::~VolumeManagerService() {}

void VolumeManagerService::VolumeStateNotify(VolumeState state, std::shared_ptr<VolumeExternal> volume)
{
    DelayedSingleton<Notification>::GetInstance()->NotifyVolumeChange(state, volume);
}

void VolumeManagerService::OnVolumeCreated(VolumeCore vc)
{
    auto volumePtr = make_shared<VolumeExternal>(vc);
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        volumeMap_.insert(make_pair(volumePtr->GetId(), volumePtr));
    }
    Mount(volumePtr->GetId());
}

void VolumeManagerService::OnVolumeStateChanged(string volumeId, VolumeState state)
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    if (volumeMap_.find(volumeId) == volumeMap_.end()) {
        LOGE("VolumeManagerService::OnVolumeStateChanged volumeId %{public}s not exists", volumeId.c_str());
        return;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[volumeId];
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return;
    }
    VolumeStateNotify(state, volumePtr);
    if (state == VolumeState::REMOVED || state == VolumeState::BAD_REMOVAL) {
        volumeMap_.erase(volumeId);
    }
}

void VolumeManagerService::OnVolumeMounted(std::string volumeId, int fsType, std::string fsUuid,
    std::string path, std::string description)
{
    std::shared_ptr<VolumeExternal> volumePtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        if (volumeMap_.find(volumeId) == volumeMap_.end()) {
            LOGE("VolumeManagerService::OnVolumeMounted volumeId %{public}s not exists", volumeId.c_str());
            return;
        }
        volumePtr = volumeMap_[volumeId];
    }
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return;
    }
    volumePtr->SetFsType(fsType);
    volumePtr->SetFsUuid(fsUuid);
    volumePtr->SetPath(path);
    std::string des = description;
    auto disk = DelayedSingleton<DiskManagerService>::GetInstance()->GetDiskById(volumePtr->GetDiskId());
    if (disk != nullptr) {
        if (des == "") {
            if (disk->GetFlag() == SD_FLAG) {
                des = "MySDCard";
            } else if (disk->GetFlag() == USB_FLAG) {
                des = "MyUSB";
            } else {
                des = "Default";
            }
        }
        volumePtr->SetFlags(disk->GetFlag());
    }
    volumePtr->SetDescription(des);
    volumePtr->SetState(VolumeState::MOUNTED);
    VolumeStateNotify(VolumeState::MOUNTED, volumePtr);
}

int32_t VolumeManagerService::Mount(std::string volumeId)
{
    std::shared_ptr<VolumeExternal> volumePtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        if (volumeMap_.find(volumeId) == volumeMap_.end()) {
            LOGE("VolumeManagerService::Mount volumeId %{public}s not exists", volumeId.c_str());
            return E_NON_EXIST;
        }
        volumePtr = volumeMap_[volumeId];
    }

    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_VOLUMEEX_IS_NULLPTR;
    }
    if (volumePtr->GetState() != VolumeState::UNMOUNTED) {
        LOGE("VolumeManagerService::The type of volume(Id %{public}s) is not unmounted", volumeId.c_str());
        return E_VOL_MOUNT_ERR;
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
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Check", result);
    }
    return result;
}

int32_t VolumeManagerService::Unmount(std::string volumeId)
{
    std::shared_ptr<VolumeExternal> volumePtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        if (volumeMap_.find(volumeId) == volumeMap_.end()) {
            LOGE("VolumeManagerService::Unmount volumeId %{public}s not exists", volumeId.c_str());
            return E_NON_EXIST;
        }
        volumePtr = volumeMap_[volumeId];
    }

    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_VOLUMEEX_IS_NULLPTR;
    }
    if (volumePtr->GetState() != VolumeState::MOUNTED) {
        LOGE("VolumeManagerService::The type of volume(Id %{public}s) is not mounted", volumeId.c_str());
        return E_VOL_UMOUNT_ERR;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    volumePtr->SetState(VolumeState::EJECTING);
    int32_t result = sdCommunication->Unmount(volumeId);
    if (result == E_OK) {
        volumePtr->SetState(VolumeState::UNMOUNTED);
        volumePtr->Reset();
    } else {
        volumePtr->SetState(VolumeState::MOUNTED);
    }
    return result;
}

int32_t VolumeManagerService::Check(std::string volumeId)
{
    std::shared_ptr<VolumeExternal> volumePtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        if (volumeMap_.find(volumeId) == volumeMap_.end()) {
            LOGE("VolumeManagerService::Check volumeId %{public}s not exists", volumeId.c_str());
            return E_NON_EXIST;
        }
        volumePtr = volumeMap_[volumeId];
    }
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_VOLUMEEX_IS_NULLPTR;
    }
    volumePtr->SetState(VolumeState::CHECKING);
    if (volumePtr->GetFsType() == FsType::MTP) {
        return E_OK;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = sdCommunication->Check(volumeId);
    return result;
}

vector<VolumeExternal> VolumeManagerService::GetAllVolumes()
{
    vector<VolumeExternal> result;
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        for (auto it = volumeMap_.begin(); it != volumeMap_.end(); ++it) {
            VolumeExternal vc = *(it->second);
            result.push_back(vc);
        }
    }
    return result;
}

std::shared_ptr<VolumeExternal> VolumeManagerService::GetVolumeByUuid(std::string volumeUuid)
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    for (auto it = volumeMap_.begin(); it != volumeMap_.end(); ++it) {
        auto vc = it->second;
        if (vc->GetUuid() == volumeUuid) {
            LOGE("VolumeManagerService::GetVolumeByUuid volumeUuid %{public}s exists",
                GetAnonyString(volumeUuid).c_str());
            return vc;
        }
    }
    return nullptr;
}

int32_t VolumeManagerService::GetVolumeByUuid(std::string fsUuid, VolumeExternal &vc)
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    for (auto it = volumeMap_.begin(); it != volumeMap_.end(); ++it) {
        auto volume = it->second;
        if (volume->GetUuid() == fsUuid) {
            LOGI("VolumeManagerService::GetVolumeByUuid volumeUuid %{public}s exists",
                GetAnonyString(fsUuid).c_str());
            vc = *volume;
            return E_OK;
        }
    }
    return E_NON_EXIST;
}

int32_t VolumeManagerService::GetVolumeById(std::string volumeId, VolumeExternal &vc)
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    if (volumeMap_.find(volumeId) != volumeMap_.end()) {
        vc = *volumeMap_[volumeId];
        return E_OK;
    }
    LOGE("VolumeManagerService::GetVolumeById volumeId %{public}s not exists", volumeId.c_str());
    return E_NON_EXIST;
}

int32_t VolumeManagerService::SetVolumeDescription(std::string fsUuid, std::string description)
{
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    for (auto it = volumeMap_.begin(); it != volumeMap_.end(); ++it) {
        auto volume = it->second;
        if (volume->GetUuid() == fsUuid) {
            LOGI("VolumeManagerService::SetVolumeDescription volumeUuid %{public}s exists",
                GetAnonyString(fsUuid).c_str());
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
    std::shared_ptr<VolumeExternal> volumePtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        if (volumeMap_.find(volumeId) == volumeMap_.end()) {
            LOGE("VolumeManagerService::Format volumeId %{public}s not exists", volumeId.c_str());
            return E_NON_EXIST;
        }
        volumePtr = volumeMap_[volumeId];
    }
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_PARAMS_INVALID;
    }
    if (volumePtr->GetFsType() == FsType::MTP) {
        LOGE("MTP device not support to format.");
        return E_NOT_SUPPORT;
    }
    if (volumePtr->GetState() != VolumeState::UNMOUNTED) {
        LOGE("VolumeManagerService::SetVolumeDescription volume state is not unmounted!");
        return E_VOL_STATE;
    }
    // check fstype!!!!
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->Format(volumeId, fsType);
}

void VolumeManagerService::NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                            const std::string &uuid)
{
    LOGI("VolumeManagerService NotifyMtpMounted");
    VolumeCore core(id, 0, "");
    auto volumePtr = make_shared<VolumeExternal>(core);
    volumePtr->SetPath(path);
    volumePtr->SetFsType(FsType::MTP);
    volumePtr->SetDescription(desc);
    volumePtr->SetState(MOUNTED);
    volumePtr->SetFsUuid(uuid);
    {
        std::lock_guard<std::mutex> lock(volumeMapMutex_);
        volumeMap_.insert(make_pair(volumePtr->GetId(), volumePtr));
    }
    VolumeStateNotify(VolumeState::MOUNTED, volumePtr);
}

void VolumeManagerService::NotifyMtpUnmounted(const std::string &id, const std::string &path, const bool isBadRemove)
{
    LOGI("VolumeManagerService NotifyMtpUnmounted");
    std::lock_guard<std::mutex> lock(volumeMapMutex_);
    if (volumeMap_.find(id) == volumeMap_.end()) {
        LOGE("VolumeManagerService::Unmount id %{public}s not exists", id.c_str());
        return;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_[id];
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for id");
        return;
    }
    if (!isBadRemove) {
        VolumeStateNotify(VolumeState::UNMOUNTED, volumePtr);
    } else {
        VolumeStateNotify(VolumeState::BAD_REMOVAL, volumePtr);
    }
    volumeMap_.erase(id);
}
} // StorageManager
} // OHOS
