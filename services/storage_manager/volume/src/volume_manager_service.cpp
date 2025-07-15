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
#include "volume/volume_manager_service_ext.h"

#include "disk.h"
#include "disk/disk_manager_service.h"
#include "safe_map.h"
#include "securec.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include <sys/xattr.h>
#include "utils/storage_radar.h"
#include "utils/storage_utils.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "volume/notification.h"

using namespace std;
using namespace OHOS::StorageService;
const int32_t MTP_DEVICE_NAME_LEN = 512;
namespace OHOS {
namespace StorageManager {
VolumeManagerService::VolumeManagerService() {}
VolumeManagerService::~VolumeManagerService() {}

void VolumeManagerService::VolumeStateNotify(VolumeState state, std::shared_ptr<VolumeExternal> volume)
{
    if (DelayedSingleton<Notification>::GetInstance() != nullptr) {
        DelayedSingleton<Notification>::GetInstance()->NotifyVolumeChange(state, volume);
    }
}

void VolumeManagerService::OnVolumeCreated(VolumeCore vc)
{
    auto volumePtr = make_shared<VolumeExternal>(vc);
    volumeMap_.Insert(volumePtr->GetId(), volumePtr);
    Mount(volumePtr->GetId());
}

void VolumeManagerService::OnVolumeStateChanged(string volumeId, VolumeState state)
{
    if (state == VolumeState::FUSE_REMOVED) {
        int32_t result = VolumeManagerServiceExt::GetInstance()->NotifyUsbFuseUMount(volumeId);
        if (result != E_OK) {
            LOGE("VolumeManagerServiceExt NotifyUsbFuseUMount failed, error = %{public}d", result);
        }
        return;
    }
    if (!volumeMap_.Contains(volumeId)) {
        LOGE("VolumeManagerService::OnVolumeDestroyed volumeId %{public}s not exists", volumeId.c_str());
        return;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
    VolumeStateNotify(state, volumePtr);
    if (state == VolumeState::REMOVED || state == VolumeState::BAD_REMOVAL) {
        volumeMap_.Erase(volumeId);
    }
}

void VolumeManagerService::OnVolumeMounted(const std::string &volumeId, const std::string &fsTypeStr,
                                           const std::string &fsUuid, const std::string &path,
                                           const std::string &description)
{
    if (!volumeMap_.Contains(volumeId)) {
        LOGE("VolumeManagerService::OnVolumeMounted volumeId %{public}s not exists", volumeId.c_str());
        return;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return;
    }
    volumePtr->SetFsType(volumePtr->GetFsTypeByStr(fsTypeStr));
    volumePtr->SetFsUuid(fsUuid);
    volumePtr->SetPath(path);
    std::string des = description;
    std::shared_ptr<Disk> disk = nullptr;
    if (DelayedSingleton<DiskManagerService>::GetInstance() != nullptr) {
        disk = DelayedSingleton<DiskManagerService>::GetInstance()->GetDiskById(volumePtr->GetDiskId());
    }
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

void VolumeManagerService::OnVolumeDamaged(const std::string &volumeId, const std::string &fsTypeStr,
                                           const std::string &fsUuid, const std::string &path,
                                           const std::string &description)
{
    if (!volumeMap_.Contains(volumeId)) {
        LOGE("VolumeManagerService::OnVolumeDamaged volumeId %{public}s not exists", volumeId.c_str());
        return;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return;
    }
    volumePtr->SetFsType(volumePtr->GetFsTypeByStr(fsTypeStr));
    volumePtr->SetFsUuid(fsUuid);
    volumePtr->SetPath(path);
    std::string des = description;
    std::shared_ptr<Disk> disk = nullptr;
    if (DelayedSingleton<DiskManagerService>::GetInstance() != nullptr) {
        disk = DelayedSingleton<DiskManagerService>::GetInstance()->GetDiskById(volumePtr->GetDiskId());
    }
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
    VolumeStateNotify(VolumeState::DAMAGED, volumePtr);
}

int32_t VolumeManagerService::Mount(std::string volumeId)
{
    if (!volumeMap_.Contains(volumeId)) {
        LOGE("VolumeManagerService::Mount volumeId %{public}s not exists", volumeId.c_str());
        return E_NON_EXIST;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
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
    if (result != E_OK || sdCommunication == nullptr) {
        volumePtr->SetState(VolumeState::UNMOUNTED);
        StorageRadar::ReportVolumeOperation("VolumeManagerService::Check", result);
        return result;
    }
    std::string mountUsbFusePath = StorageDaemon::StringPrintf("/mnt/data/external/%s", volumePtr->GetUuid().c_str());
    if (StorageDaemon::IsFuse() && !StorageDaemon::IsPathMounted(mountUsbFusePath)) {
        result = MountUsbFuse(volumeId);
        if (result != E_OK) {
            volumePtr->SetState(VolumeState::UNMOUNTED);
            return result;
        }
    }
    
    result = sdCommunication->Mount(volumeId, 0);
    if (result != E_OK) {
        volumePtr->SetState(VolumeState::UNMOUNTED);
    }
    return result;
}

int32_t VolumeManagerService::MountUsbFuse(const std::string &volumeId)
{
    LOGI("VolumeManagerService::MountUsbFuse in");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        return E_COMMUNICATION_IS_NULLPTR;
    }

    std::string fsUuid;
    int32_t fuseFd;
    int32_t result = sdCommunication->MountUsbFuse(volumeId, fsUuid, fuseFd);
    if (result == E_OK) {
        result = VolumeManagerServiceExt::GetInstance()->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    }
    LOGI("VolumeManagerService::MountUsbFuse out");
    return result;
}

int32_t VolumeManagerService::Unmount(std::string volumeId)
{
    if (!volumeMap_.Contains(volumeId)) {
        LOGE("VolumeManagerService::Unmount volumeId %{public}s not exists", volumeId.c_str());
        return E_NON_EXIST;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
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
    if (sdCommunication == nullptr) {
        LOGE("sdCommunication is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    int32_t result = sdCommunication->Unmount(volumeId);
    if (result == E_OK) {
        volumePtr->SetState(VolumeState::UNMOUNTED);
        volumePtr->Reset();
    } else {
        volumePtr->SetState(VolumeState::MOUNTED);
    }
    return result;
}

int32_t VolumeManagerService::TryToFix(std::string volumeId)
{
    if (!volumeMap_.Contains(volumeId)) {
        LOGE("VolumeManagerService::TryToFix volumeId %{public}s not exists", volumeId.c_str());
        return E_NON_EXIST;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for volumeId");
        return E_VOLUMEEX_IS_NULLPTR;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = Check(volumePtr->GetId());
    if (result == E_OK && sdCommunication != nullptr) {
        result = sdCommunication->TryToFix(volumeId, 0);
    } else {
        volumePtr->Reset();
        StorageRadar::ReportVolumeOperation("VolumeManagerService::DAMAGED", result);
    }
    return result;
}

int32_t VolumeManagerService::Check(std::string volumeId)
{
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
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
    if (sdCommunication == nullptr) {
        LOGE("sdCommunication is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
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
            LOGE("VolumeManagerService::GetVolumeByUuid volumeUuid %{public}s exists",
                GetAnonyString(volumeUuid).c_str());
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
    if (volumeMap_.Contains(volumeId)) {
        vc = *volumeMap_.ReadVal(volumeId);
        return E_OK;
    }
    return E_NON_EXIST;
}

int32_t VolumeManagerService::SetVolumeDescription(std::string fsUuid, std::string description)
{
    for (auto it = volumeMap_.Begin(); it != volumeMap_.End(); ++it) {
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
            if (sdCommunication == nullptr) {
                LOGE("sdCommunication is nullptr");
                return E_PARAMS_NULLPTR_ERR;
            }
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
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(volumeId);
    if ((volumePtr != nullptr) && (volumePtr->GetFsType() == FsType::MTP)) {
        LOGE("MTP device not support to format.");
        return E_NOT_SUPPORT;
    }
    if (volumeMap_.ReadVal(volumeId)->GetState() != VolumeState::UNMOUNTED) {
        LOGE("VolumeManagerService::SetVolumeDescription volume state is not unmounted!");
        return E_VOL_STATE;
    }
    // check fstype!!!!
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        return E_COMMUNICATION_IS_NULLPTR;
    }
    int32_t result = sdCommunication->Format(volumeId, fsType);
    if (result != E_OK) {
        return result;
    }

    if (StorageDaemon::IsFuse()) {
        result = VolumeManagerServiceExt::GetInstance()->NotifyUsbFuseUMount(volumeId);
    }
    return result;
}

void VolumeManagerService::NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                            const std::string &uuid)
{
    LOGI("VolumeManagerService NotifyMtpMounted");
    std::string key = "user.getfriendlyname";
    char *value = (char *)malloc(sizeof(char) * MTP_DEVICE_NAME_LEN);
    int32_t len = 0;
    if (value != nullptr) {
        len = getxattr(path.c_str(), key.c_str(), value, MTP_DEVICE_NAME_LEN);
        if (len >= 0 && len < MTP_DEVICE_NAME_LEN) {
            value[len] = '\0';
            LOGI("MTP get namelen=%{public}d, name=%{public}s", len, value);
        }
    }

    VolumeCore core(id, 0, "");
    auto volumePtr = make_shared<VolumeExternal>(core);
    volumePtr->SetPath(path);
    volumePtr->SetFsType(volumePtr->GetFsTypeByStr("mtp"));
    volumePtr->SetDescription(desc);
    if (len > 0) {
        LOGI("set MTP device name:%{public}s", value);
        volumePtr->SetDescription(std::string(value));
    }
    if (value != nullptr) {
        free(value);
        value = nullptr;
    }
    volumePtr->SetState(MOUNTED);
    volumePtr->SetFsUuid(uuid);
    volumeMap_.Insert(volumePtr->GetId(), volumePtr);
    VolumeStateNotify(VolumeState::MOUNTED, volumePtr);
}

void VolumeManagerService::NotifyMtpUnmounted(const std::string &id, const std::string &path, const bool isBadRemove)
{
    LOGI("VolumeManagerService NotifyMtpUnmounted");
    if (!volumeMap_.Contains(id)) {
        LOGE("VolumeManagerService::Unmount id %{public}s not exists", id.c_str());
        return;
    }
    std::shared_ptr<VolumeExternal> volumePtr = volumeMap_.ReadVal(id);
    if (volumePtr == nullptr) {
        LOGE("volumePtr is nullptr for id");
        return;
    }
    if (!isBadRemove) {
        VolumeStateNotify(VolumeState::UNMOUNTED, volumePtr);
    } else {
        VolumeStateNotify(VolumeState::BAD_REMOVAL, volumePtr);
    }

    volumeMap_.Erase(id);
}
} // StorageManager
} // OHOS
