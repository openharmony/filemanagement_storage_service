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
    LOGI("[L3:VolumeInfo] Create: >>> ENTER <<< volId=%{public}s, diskId=%{public}s, isUserdata=%{public}d",
        volId.c_str(), diskId.c_str(), isUserdata);

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
        LOGE("[L3:VolumeInfo] Create: <<< EXIT FAILED <<< DoCreate failed, err=%{public}d", err);
        return err;
    }
    LOGI("[L3:VolumeInfo] Create: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
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

void VolumeInfo::SetState(VolumeState mountState)
{
    mountState_ = mountState;
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
    LOGI("[L3:VolumeInfo] Destroy: >>> ENTER <<< volId=%{public}s", id_.c_str());

    VolumeState state = REMOVED;
    if (mountState_ == REMOVED || mountState_ == BADREMOVABLE) {
        LOGI("[L3:VolumeInfo] Destroy: <<< EXIT SUCCESS <<< already in REMOVED state");
        return E_OK;
    }
    StorageManagerClient client;
    if (mountState_ != UNMOUNTED) {
        // force umount
        UMount(true);
        state = BADREMOVABLE;
        if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::BAD_REMOVAL) != E_OK) {
            LOGE("[L3:VolumeInfo] Destroy: Notify Bad Removal failed");
        }
    } else {
        if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::REMOVED) != E_OK) {
            LOGE("[L3:VolumeInfo] Destroy: Notify Removed failed");
        }
    }

    int32_t err = DoDestroy();
    if (err) {
        LOGE("[L3:VolumeInfo] Destroy: <<< EXIT FAILED <<< DoDestroy failed, err=%{public}d", err);
        return err;
    }
    mountState_ = state;
    LOGI("[L3:VolumeInfo] Destroy: <<< EXIT SUCCESS <<< volId=%{public}s", id_.c_str());
    return E_OK;
}

void VolumeInfo::DestroyUsbFuse()
{
    LOGI("[L3:VolumeInfo] DestroyUsbFuse: >>> ENTER <<< volId=%{public}s", id_.c_str());

    StorageManagerClient client;
    UMountUsbFuse();
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::FUSE_REMOVED) != E_OK) {
        LOGE("[L3:VolumeInfo] DestroyUsbFuse: Notify FUSE Removed failed");
    }
    LOGI("[L3:VolumeInfo] DestroyUsbFuse: <<< EXIT SUCCESS <<<");
}

int32_t VolumeInfo::Mount(uint32_t flags)
{
    LOGI("[L3:VolumeInfo] Mount: >>> ENTER <<< volId=%{public}s, flags=%{public}u, mountState=%{public}d,"
        "isDamaged=%{public}d", id_.c_str(), flags, mountState_, isDamaged_);

    int32_t err = 0;

    if (system::GetParameter("persist.edm.external_storage_card_disable", "") == "true") {
        LOGW("[L3:VolumeInfo] Mount: <<< EXIT FAILED <<< External Storage prohibited");
        return E_VOL_MOUNT_ERR;
    }

    if (mountState_ == MOUNTED || mountState_ == DAMAGED_MOUNTED) {
        LOGI("[L3:VolumeInfo] Mount: <<< EXIT SUCCESS <<< already mounted");
        return E_OK;
    }
    if (mountState_ != CHECKING) {
        LOGE("[L3:VolumeInfo] Mount: <<< EXIT FAILED <<< invalid state=%{public}d, need CHECK first", mountState_);
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
    LOGI("[L3:VolumeInfo] Mount: external volume mount start");
    err = DoMount(mountFlags_);
    if (err) {
        mountState_ = UNMOUNTED;
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoMount", err);
        LOGE("[L3:VolumeInfo] Mount: <<< EXIT FAILED <<< DoMount failed, err=%{public}d", err);
        return err;
    }
    mountState_ = (isDamaged_ == true ? DAMAGED_MOUNTED : MOUNTED);
    LOGI("[L3:VolumeInfo] Mount: <<< EXIT SUCCESS <<< volId=%{public}s, mountState=%{public}d",
        id_.c_str(), mountState_);
    return E_OK;
}

int32_t VolumeInfo::UMount(bool force)
{
    LOGI("[L3:VolumeInfo] UMount: >>> ENTER <<< volId=%{public}s, force=%{public}d, mountState=%{public}d",
        id_.c_str(), force, mountState_);

    int32_t err = 0;

    if (mountState_ == REMOVED || mountState_ == BADREMOVABLE) {
        LOGE("[L3:VolumeInfo] UMount: <<< EXIT FAILED <<< volId=%{public}s in REMOVED state", id_.c_str());
        return E_VOL_STATE;
    }

    if (mountState_ == UNMOUNTED) {
        LOGI("[L3:VolumeInfo] UMount: <<< EXIT SUCCESS <<< already unmounted");
        return E_OK;
    }

    if (mountState_ == CHECKING) {
        mountState_ = UNMOUNTED;
        LOGI("[L3:VolumeInfo] UMount: <<< EXIT SUCCESS <<< was in CHECKING state");
        return E_OK;
    }

    if (mountState_ == EJECTING && !force) {
        LOGE("[L3:VolumeInfo] UMount: <<< EXIT FAILED <<< already EJECTING and not force");
        return E_UMOUNT_BUSY;
    }

    LOGI("[L3:VolumeInfo] UMount: Volume %{public}s start to unmount, now mount"
        "state is %{public}d.", GetVolumeId().c_str(), mountState_);
    mountState_ = EJECTING;
    StorageManagerClient client;
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::EJECTING) != E_OK) {
        LOGE("[L3:VolumeInfo] UMount: Notify Ejecting failed");
    }

    err = DoUMount(force);
    if (!force && err) {
        mountState_ = (isDamaged_ == true ? DAMAGED_MOUNTED : MOUNTED);
        LOGE("[L3:VolumeInfo] UMount: <<< EXIT FAILED <<< DoUMount failed, err=%{public}d", err);
        return err;
    }

    mountState_ = UNMOUNTED;
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::UNMOUNTED) != E_OK) {
        LOGE("[L3:VolumeInfo] UMount: Notify Unmounted failed");
    }
    LOGI("[L3:VolumeInfo] UMount: <<< EXIT SUCCESS <<< volId=%{public}s", id_.c_str());
    return E_OK;
}

int32_t VolumeInfo::UMountUsbFuse()
{
    LOGD("[L3:VolumeInfo] UMountUsbFuse: >>> ENTER <<< volId=%{public}s", id_.c_str());
    
    int32_t ret = DoUMountUsbFuse();
    if (ret == E_OK) {
        LOGD("[L3:VolumeInfo] UMountUsbFuse: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L3:VolumeInfo] UMountUsbFuse: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

int32_t VolumeInfo::Check()
{
    LOGI("[L3:VolumeInfo] Check: >>> ENTER <<< Volume enter Check, mountState_ is %{public}d.", mountState_);
    if ((mountState_ != UNMOUNTED) &&
        (mountState_ != ENCRYPTING) &&
        (mountState_ != DECRYPTING) &&
        (mountState_ != ENCRYPTED_AND_LOCKED) &&
        (mountState_ != ENCRYPTED_AND_UNLOCKED)) {
        LOGE("[L3:VolumeInfo] Check: the volume %{public}s is not in UNMOUNT state, "
            "in %{public}d state.", GetVolumeId().c_str(), mountState_);
        return E_VOL_STATE;
    }

    if (mountState_ == CHECKING) {
        mountState_ = UNMOUNTED;
    }

    int32_t err = DoCheck();
    if (err) {
        LOGE("[L3:VolumeInfo] Check: <<< EXIT FAILED <<< DoCheck failed, err=%{public}d", err);
        return err;
    }
    mountState_ = CHECKING;
    LOGI("[L3:VolumeInfo] Check: <<< EXIT SUCCESS <<< volId=%{public}s", id_.c_str());
    return E_OK;
}

int32_t VolumeInfo::TryToCheck()
{
    LOGD("[L3:VolumeInfo] TryToCheck: >>> ENTER <<< volId=%{public}s", id_.c_str());

    int32_t checkRet = DoTryToCheck();
    if (checkRet == E_VOL_NEED_FIX) {
        LOGE("[L3:VolumeInfo] TryToCheck: <<< EXIT FAILED <<< need fix");
        isDamaged_ = true;
    } else if (checkRet == E_VOL_FIX_NOT_SUPPORT) {
        LOGE("[L3:VolumeInfo] TryToCheck: <<< EXIT FAILED <<< fix not supported");
    } else {
        LOGD("[L3:VolumeInfo] TryToCheck: <<< EXIT SUCCESS <<<");
    }
    return checkRet;
}

int32_t VolumeInfo::TryToFix()
{
    LOGI("[L3:VolumeInfo] TryToFix: >>> ENTER <<< volId=%{public}s", id_.c_str());

    int32_t errFix = DoTryToFix();
    if (errFix != E_OK) {
        LOGE("[L3:VolumeInfo] TryToFix: <<< EXIT FAILED <<< err=%{public}d", errFix);
        isDamaged_ = true;
        return errFix;
    }
    isDamaged_ = false;
    LOGI("[L3:VolumeInfo] TryToFix: <<< EXIT SUCCESS <<< volId=%{public}s", id_.c_str());
    return E_OK;
}

int32_t VolumeInfo::Format(std::string type)
{
    LOGI("[L3:VolumeInfo] Format: >>> ENTER <<< volId=%{public}s, type=%{public}s", id_.c_str(), type.c_str());
    if ((mountState_ != UNMOUNTED) &&
        (mountState_ != ENCRYPTED_AND_LOCKED) &&
        (mountState_ != ENCRYPTED_AND_UNLOCKED)) {
        LOGE("[L3:VolumeInfo] Format: Please unmount the volume %{public}s first", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    int32_t err = DoFormat(type);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoFormat", err);
        LOGE("[L3:VolumeInfo] Format: <<< EXIT FAILED <<< DoFormat failed, err=%{public}d", err);
    } else {
        LOGI("[L3:VolumeInfo] Format: <<< EXIT SUCCESS <<< volId=%{public}s", id_.c_str());
    }
    fsTypeBase_ = type;
    return err;
}

int32_t VolumeInfo::SetVolumeDescription(const std::string description)
{
    LOGI("[L3:VolumeInfo] SetVolumeDescription: >>> ENTER <<< volId=%{public}s, desc=%{public}s",
        id_.c_str(), description.c_str());
    if ((mountState_ != UNMOUNTED) &&
        (mountState_ != ENCRYPTED_AND_LOCKED) &&
        (mountState_ != ENCRYPTED_AND_UNLOCKED)) {
        LOGE("[L3:VolumeInfo] SetVolumeDescription: <<< EXIT FAILED <<< not unmounted, state=%{public}d", mountState_);
        return E_VOL_STATE;
    }

    int32_t err = DoSetVolDesc(description);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoSetVolDesc", err);
        LOGE("[L3:VolumeInfo] SetVolumeDescription: <<< EXIT FAILED <<< err=%{public}d", err);
    } else {
        LOGI("[L3:VolumeInfo] SetVolumeDescription: <<< EXIT SUCCESS <<<");
    }
    return err;
}

bool VolumeInfo::IsUsbFuseByType(std::string fsType)
{
    StorageManagerClient client;
    bool isUsbFuseByType = true;
    client.IsUsbFuseByType(fsType, isUsbFuseByType);
    return isUsbFuseByType;
}

int32_t VolumeInfo::GetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize)
{
    int32_t err = DoGetOddCapacity(volumeId, totalSize, freeSize);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] DoGetOddCapacity: Volume DoGetOddCapacity failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::DestroyCrypt(const std::string &volumeId)
{
    int32_t err = DoDestroyCrypt(volumeId);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] DestroyCrypt: Volume DoCloseCrypt failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::Encrypt(const std::string &volumeId, const std::string &pazzword)
{
    mountState_ = ENCRYPTING;
    StorageManagerClient client;
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::ENCRYPTING) != E_OK) {
        LOGE("[L3:VolumeInfo] Encrypt: Volume Notify Encryption failed");
    }
    int32_t err = DoEncrypt(volumeId, pazzword);
    if (err != E_OK) {
        mountState_ = UNMOUNTED;
        LOGE("[L3:VolumeInfo] Encrypt: Volume DoEncrypt failed, err: %{public}d", err);
        return err;
    }
    mountState_ = ENCRYPTED_AND_LOCKED;
    if (client.NotifyVolumeStateChanged(id_, StorageManager::VolumeState::ENCRYPTED_AND_LOCKED) != E_OK) {
        LOGE("[L3:VolumeInfo] Encrypt: Volume Notify Encryption failed");
    }
    return E_OK;
}

int32_t VolumeInfo::GetCryptProgressById(const std::string &volumeId, int32_t &progress)
{
    int32_t err = DoGetCryptProgressById(volumeId, progress);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] GetCryptProgressById: Volume DoGetCryptProgressById failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::GetCryptUuidById(const std::string &volumeId, std::string &uuid)
{
    int32_t err = DoGetCryptUuidById(volumeId, uuid);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] GetCryptUuidById: Volume GetCryptUuidById failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::BindRecoverKeyToPasswd(const std::string &volumeId,
                                           const std::string &pazzword,
                                           const std::string &recoverKey)
{
    int32_t err = DoBindRecoverKeyToPasswd(volumeId, pazzword, recoverKey);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] BindRecoverKeyToPasswd: Volume DoBindRecoverKeyToPasswd failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::UpdateCryptPasswd(const std::string &volumeId,
                                      const std::string &pazzword,
                                      const std::string &newPazzword)
{
    int32_t err = DoUpdateCryptPasswd(volumeId, pazzword, newPazzword);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] UpdateCryptPasswd: Volume DoUpdateCryptPasswd failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::ResetCryptPasswd(const std::string &volumeId,
                                     const std::string &recoverKey,
                                     const std::string &newPazzword)
{
    int32_t err = DoResetCryptPasswd(volumeId, recoverKey, newPazzword);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] ResetCryptPasswd: Volume DoResetCryptPasswd failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword)
{
    int32_t err = DoVerifyCryptPasswd(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] VerifyCryptPasswd: Volume DoVerifyCryptPasswd failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::Unlock(const std::string &volumeId, const std::string &pazzword)
{
    int32_t err = DoUnlock(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] Unlock: Volume DoUnlock failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::Decrypt(const std::string &volumeId, const std::string &pazzword)
{
    int32_t err = DoDecrypt(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L3:VolumeInfo] Decrypt: Volume DoDecrypt failed, err: %{public}d", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeInfo::Eject(const std::string &volId)
{
    if (volId.empty()) {
        LOGE("Eject volId is empty");
        return E_NON_EXIST;
    }
    if (volId != GetVolumeId()) {
        LOGE("Eject volId is invalid, volId: %{public}s, current volume id: %{public}s", 
                volId.c_str(), GetVolumeId().c_str());
        return E_PARAMS_INVALID;
    }

    int32_t err = DoEject(volId);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeInfo::Doeject", err);
        LOGE("VolumeInfo::DoEject failed, err: %{public}d", err);
    }
    return err;
}

int32_t VolumeInfo::GetOpticalDriveOpsProgress(const std::string &volId, uint32_t &progress)
{
    if (volId.empty()) {
        LOGE("GetOpticalDriveOpsProgress volId is empty");
        return E_NON_EXIST;
    }
    if (volId != GetVolumeId()) {
        LOGE("GetOpticalDriveOpsProgress volId is invalid, volId: %{public}s, current volume id: %{public}s", 
                volId.c_str(), GetVolumeId().c_str());
        return E_PARAMS_INVALID;
    }

    uint32_t progressDefaultValue = 0;
    int32_t err = DoGetOpticalDriveOpsProgress(volId, progressDefaultValue);
    if (err != E_OK) {
        StorageRadar::ReportVolumeOperation("VolumeInfo::DoGetOpticalDriveOpsProgress", err);
    } else {
        progress = progressDefaultValue;
    }
    return err;
}
} // StorageDaemon
} // OHOS
