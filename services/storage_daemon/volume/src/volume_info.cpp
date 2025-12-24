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

#include "volume/volume_info.h"

#include <sys/mount.h>

#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"
#include "parameter.h"
#include "utils/file_utils.h"
#include "parameters.h"

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t TRUE_LEN = 5;
constexpr int32_t RD_ENABLE_LENGTH = 255;
constexpr const char *PERSIST_FILEMANAGEMENT_USB_READONLY = "persist.filemanagement.usb.readonly";
const std::string UNDEFINED_FS_TYPE = "undefined";

int32_t VolumeInfo::Create(const std::string volId, const std::string diskId, dev_t device, bool isUserdata)
{
    id_ = volId;
    diskId_ = diskId;
    type_ = EXTERNAL;
    mountState_ = UNMOUNTED;
    mountFlags_ = 0;
    userIdOwner_ = 0;
    isUserdata_ = isUserdata;
    isDamaged_ = false;

    int32_t err = DoCreate(device);
    fsTypeBase_ = GetFsTypeByDev(device);
    if (err) {
        return err;
    }
    return E_OK;
}

std::string VolumeInfo::GetVolumeId()
{
    return id_;
}

int32_t VolumeInfo::GetVolumeType()
{
    return type_;
}

std::string VolumeInfo::GetDiskId()
{
    return diskId_;
}

int32_t VolumeInfo::GetState()
{
    return mountState_;
}

bool VolumeInfo::GetIsUserdata()
{
    return isUserdata_;
}

std::string VolumeInfo::GetFsTypeBase()
{
    return fsTypeBase_;
}

int32_t VolumeInfo::Destroy()
{
    VolumeState state = REMOVED;
    if (mountState_ == REMOVED || mountState_ == BADREMOVABLE) {
        return E_OK;
    }
    StorageManagerClient client;
    if (mountState_ != UNMOUNTED) {
        // force umount
        UMount(true);
        state = BADREMOVABLE;
        if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::BAD_REMOVAL) != E_OK) {
            LOGE("Volume Notify Bad Removal failed");
        }
    } else {
        if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::REMOVED) != E_OK) {
            LOGE("Volume Notify Removed failed");
        }
    }

    int32_t err = DoDestroy();
    if (err) {
        return err;
    }
    mountState_ = state;
    return E_OK;
}

void VolumeInfo::DestroyUsbFuse()
{
    LOGI("DestroyUsbFuse in");
    StorageManagerClient client;
    UMountUsbFuse();
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::FUSE_REMOVED) != E_OK) {
        LOGE("Volume Notify Removed failed");
    }
    LOGI("DestroyUsbFuse out");
}

int32_t VolumeInfo::Mount(uint32_t flags)
{
    int32_t err = 0;

    if (system::GetParameter("persist.edm.external_storage_card_disable", "") == "true") {
        LOGW("External Storage is prohibited!");
        return E_VOL_MOUNT_ERR;
    }

    LOGI("Volume mount state is %{public}d, isDamaged_ is %{public}d at mount start.", mountState_, isDamaged_);
    if (mountState_ == MOUNTED || mountState_ == DAMAGED_MOUNTED) {
        return E_OK;
    }
    if (mountState_ != CHECKING) {
        LOGE("please check volume %{public}s first", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    if (!IsUsbFuseByType(UNDEFINED_FS_TYPE)) {
        std::string key = PERSIST_FILEMANAGEMENT_USB_READONLY;
        int handle = static_cast<int>(FindParameter(key.c_str()));
        if (handle != -1) {
            char rdOnlyEnable[RD_ENABLE_LENGTH] = {"false"};
            auto res = GetParameterValue(handle, rdOnlyEnable, RD_ENABLE_LENGTH);
            if (res >= 0 && strncmp(rdOnlyEnable, "true", TRUE_LEN) == 0) {
                mountFlags_ |= MS_RDONLY;
            } else {
                mountFlags_ &= ~MS_RDONLY;
            }
        }
    }

    mountFlags_ |= flags;
    LOGI("external volume mount start");
    err = DoMount(mountFlags_);
    if (err) {
        mountState_ = UNMOUNTED;
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoMount", err);
        return err;
    }
    LOGI("external volume mount success");
    LOGI("Volume mount state is %{public}d, isDamaged_ is %{public}d at mount end.", mountState_, isDamaged_);
    mountState_ = (isDamaged_ == true ? DAMAGED_MOUNTED : MOUNTED);
    return E_OK;
}

int32_t VolumeInfo::UMount(bool force)
{
    int32_t err = 0;

    LOGI("Volume mount state is %{public}d, isDamaged_ is %{public}d at unmount start.", mountState_, isDamaged_);
    if (mountState_ == REMOVED || mountState_ == BADREMOVABLE) {
        LOGE("the volume %{public}s is in REMOVED state", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    if (mountState_ == UNMOUNTED) {
        return E_OK;
    }

    if (mountState_ == CHECKING) {
        mountState_ = UNMOUNTED;
        return E_OK;
    }

    if (mountState_ == EJECTING && !force) {
        return E_UMOUNT_BUSY;
    }

    LOGI("Volume %{public}s start to unmount, now mount state is %{public}d.", GetVolumeId().c_str(), mountState_);
    mountState_ = EJECTING;
    StorageManagerClient client;
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::EJECTING) != E_OK) {
        LOGE("Volume Notify Ejecting failed");
    }

    err = DoUMount(force);
    if (!force && err) {
        mountState_ = (isDamaged_ == true ? DAMAGED_MOUNTED : MOUNTED);
        LOGE("Volume DoUmount failed, err: %{public}d, mountState_: %{public}d", err, mountState_);
        return err;
    }

    mountState_ = UNMOUNTED;
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::UNMOUNTED) != E_OK) {
        LOGE("Volume Notify Unmounted failed");
    }
    return E_OK;
}

int32_t VolumeInfo::UMountUsbFuse()
{
    return DoUMountUsbFuse();
}

int32_t VolumeInfo::Check()
{
    LOGI("Volume enter Check, mountState_ is %{public}d.", mountState_);
    if (mountState_ != UNMOUNTED) {
        LOGE("the volume %{public}s is not in UNMOUNT state, in %{public}d state.", GetVolumeId().c_str(), mountState_);
        return E_VOL_STATE;
    }

    if (mountState_ == CHECKING) {
        mountState_ = UNMOUNTED;
    }

    int32_t err = DoCheck();
    if (err) {
        return err;
    }
    mountState_ = CHECKING;
    return E_OK;
}

int32_t VolumeInfo::TryToCheck()
{
    int32_t checkRet = DoTryToCheck();
    if (checkRet == E_VOL_NEED_FIX) {
        LOGE("External Volume need fix.");
        isDamaged_ = true;
    } else if (checkRet == E_VOL_FIX_NOT_SUPPORT) {
        LOGE("External Volume need fix, but not support.");
    }
    return checkRet;
}

int32_t VolumeInfo::TryToFix()
{
    int32_t errFix = DoTryToFix();
    if (errFix != E_OK) {
        LOGE("Volume TryToFix failed, err: %{public}d", errFix);
        isDamaged_ = true;
        return errFix;
    }
    isDamaged_ = false;
    return E_OK;
}

int32_t VolumeInfo::Format(std::string type)
{
    if (mountState_ != UNMOUNTED) {
        LOGE("Please unmount the volume %{public}s first", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    int32_t err = DoFormat(type);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoFormat", err);
    }
    return err;
}

int32_t VolumeInfo::SetVolumeDescription(const std::string description)
{
    if (mountState_ != UNMOUNTED) {
        LOGE("Please unmount the volume %{public}s first", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    int32_t err = DoSetVolDesc(description);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoSetVolDesc", err);
    }
    return err;
}

bool ExternalVolumeInfo::IsUsbFuseByType(std::string fsType)
{
    StorageManagerClient client;
    bool isUsbFuseByType = true;
    client.IsUsbFuseByType(fsType, isUsbFuseByType);
    return isUsbFuseByType;
}
} // StorageDaemon
} // OHOS
