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

#include "volume/volume_manager.h"

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "ipc/storage_manager_client.h"
#include "mtp/mtp_device_monitor.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "volume/external_volume_info.h"

const int32_t MTP_QUERY_RESULT_LEN = 10;
const std::string MTP_PATH_PREFIX = "/mnt/data/external/mtp";
#define STORAGE_MANAGER_IOC_CHK_BUSY _IOR(0xAC, 77, int)
using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {

VolumeManager &VolumeManager::Instance()
{
    static VolumeManager instance_;
    return instance_;
}

std::shared_ptr<VolumeInfo> VolumeManager::GetVolume(const std::string &volId)
{
    std::lock_guard<std::mutex> lock(volumesMutex_);
    if (volumes_.find(volId) == volumes_.end()) {
        LOGE("VolumeManager::GetVolume the volume %{public}s doesn't exist", volId.c_str());
        return nullptr;
    }
    return volumes_[volId];
}

std::string VolumeManager::CreateVolume(const std::string &diskId, dev_t device, bool isUserdata)
{
    LOGI("[L2:VolumeManager] CreateVolume: >>> ENTER <<< diskId=%{public}s, device=%u:%u, isUserdata=%{public}d",
        diskId.c_str(), major(device), minor(device), isUserdata);
    std::string volId = StringPrintf("vol-%u-%u", major(device), minor(device));

    LOGI("[L2:VolumeManager] CreateVolume: create volume %{public}s.", volId.c_str());

    std::shared_ptr<VolumeInfo> tmp = GetVolume(volId);
    if (tmp != nullptr) {
        LOGE("[L2:VolumeManager] CreateVolume: <<< EXIT FAILED <<< volId=%{public}s already exists", volId.c_str());
        return "";
    }

    auto info = std::make_shared<ExternalVolumeInfo>();
    int32_t ret = info->Create(volId, diskId, device, isUserdata);
    if (ret) {
        LOGE("[L2:VolumeManager] CreateVolume: <<< EXIT FAILED <<< Create failed, ret=%{public}d", ret);
        return "";
    }

    {
        std::lock_guard<std::mutex> lock(volumesMutex_);
        volumes_.insert(make_pair(volId, info));
    }

    StorageManagerClient client;
    ret = client.NotifyVolumeCreated(info);
    if (ret != E_OK) {
        LOGE("[L2:VolumeManager] CreateVolume: Notify Created failed");
    }

    LOGI("[L2:VolumeManager] CreateVolume: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return volId;
}

int32_t VolumeManager::DestroyVolume(const std::string &volId)
{
    LOGI("[L2:VolumeManager] DestroyVolume: >>> ENTER <<< volId=%{public}s", volId.c_str());

    std::shared_ptr<VolumeInfo> destroyNode = GetVolume(volId);

    if (destroyNode == nullptr) {
        LOGE("[L2:VolumeManager] DestroyVolume: <<< EXIT FAILED <<< volId=%{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
    }

    destroyNode->DestroyCrypt(volId);

    int32_t ret = destroyNode->Destroy();
    if (ret) {
        LOGE("[L2:VolumeManager] DestroyVolume: <<< EXIT FAILED <<< Destroy failed, ret=%{public}d", ret);
        return ret;
    }
    destroyNode->DestroyUsbFuse();

    {
        std::lock_guard<std::mutex> lock(volumesMutex_);
        volumes_.erase(volId);
    }
    destroyNode.reset();

    LOGI("[L2:VolumeManager] DestroyVolume: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::Check(const std::string &volId)
{
    LOGI("[L2:VolumeManager] Check: >>> ENTER <<< volId=%{public}s", volId.c_str());

    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] Check: <<< EXIT FAILED <<< volId=%{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->Check();
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Check: <<< EXIT FAILED <<< Check failed, err=%{public}d", err);
        return err;
    }
    LOGI("[L2:VolumeManager] Check: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::Mount(const std::string &volId, uint32_t flags)
{
    LOGI("[L2:VolumeManager] Mount: >>> ENTER <<< volId=%{public}s, flags=%{public}u", volId.c_str(), flags);

    int err = E_OK;
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
        return OHOS::StorageDaemon::MtpDeviceMonitor::GetInstance().Mount(volId);
#else
        LOGE("[L2:VolumeManager] Mount: <<< EXIT FAILED <<< volId=%{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
#endif
    }
    StorageManagerClient client;
    string fsTypeBase = info->GetFsTypeBase();
    if (fsTypeBase == "crypt_LUKS") {
        LOGI("[L2:VolumeManager] Mount: Volume is LUKS encrypted.");
        info->SetState(ENCRYPTED_AND_LOCKED);
        err = client.NotifyEncryptVolumeStateChanged(info);
        if (err != E_OK) {
            LOGE("[L2:VolumeManager] Mount: the volume %{public}s notify failed.", volId.c_str());
        }
        return E_OK;
    }
    LOGI("[L2:VolumeManager] Mount: Before Mount in VolumeManager::Mount");
    err = info->Mount(flags);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Mount: <<< EXIT FAILED <<< Mount failed, err=%{public}d", err);
        return err;
    }
    err = client.NotifyVolumeMounted(info);
    if (err) {
        LOGE("[L2:VolumeManager] Mount: Notify Mounted failed");
    }
    LOGI("[L2:VolumeManager] Mount: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd)
{
    LOGI("[L2:VolumeManager] MountUsbFuse: >>> ENTER <<< volumeId=%{public}s", volumeId.c_str());

    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] MountUsbFuse: <<< EXIT FAILED <<< volumeId=%{public}s does not exist",
            volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t result = ReadVolumeUuid(volumeId, fsUuid);
    if (result != E_OK) {
        LOGE("[L2:VolumeManager] MountUsbFuse: <<< EXIT FAILED <<< ReadVolumeUuid failed, ret=%{public}d", result);
        return result;
    }
    result = CreateMountUsbFusePath(fsUuid);
    if (result != E_OK) {
        LOGE("[L2:VolumeManager] MountUsbFuse: <<< EXIT FAILED <<< CreateMountUsbFusePath failed, ret=%{public}d",
            result);
        return result;
    }
    fuseFd = open("/dev/fuse", O_RDWR);
    if (fuseFd < 0) {
        LOGE("[L2:VolumeManager] MountUsbFuse: <<< EXIT FAILED <<< open /dev/fuse failed, errno=%{public}d", errno);
        return E_OPEN_FUSE;
    }
    LOGI("[L2:VolumeManager] MountUsbFuse: open fuse end.");
    string opt = StringPrintf("fd=%i,"
        "rootmode=40000,"
        "default_permissions,"
        "allow_other,"
        "user_id=0,group_id=0,"
        "context=\"u:object_r:mnt_external_file:s0\","
        "fscontext=u:object_r:mnt_external_file:s0",
        fuseFd);

    int ret = StorageDaemon::Mount("/dev/fuse", mountUsbFusePath_.c_str(),
                                   "fuse", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opt.c_str());
    if (ret) {
        LOGE("[L2:VolumeManager] MountUsbFuse: <<< EXIT FAILED <<< mount fuse failed, ret=%{public}d, errno=%{public}d",
            ret, errno);
        close(fuseFd);
        std::string extraData = "dstPath=" + mountUsbFusePath_ + ",kernelCode=" + to_string(errno);
        return E_MOUNT_FILE_MGR_FUSE;
    }
    LOGI("[L2:VolumeManager] MountUsbFuse: <<< EXIT SUCCESS <<< volumeId=%{public}s", volumeId.c_str());
    return E_OK;
}
 
int32_t VolumeManager::ReadVolumeUuid(const std::string &volumeId, std::string &fsUuid)
{
    std::string devPath = StringPrintf("/dev/block/%s", (volumeId).c_str());
    int32_t ret = OHOS::StorageDaemon::ReadVolumeUuid(devPath, fsUuid);
    return ret;
}
 
int32_t VolumeManager::CreateMountUsbFusePath(std::string &fsUuid)
{
    LOGI("[L2:VolumeManager] CreateMountUsbFusePath: >>> ENTER <<< fsUuid=%{private}s",
        GetAnonyString(fsUuid).c_str());

    if (fsUuid.find("..") != std::string::npos || fsUuid.find("/") != std::string::npos) {
        LOGE("[L2:VolumeManager] CreateMountUsbFusePath: <<< EXIT FAILED <<< invalid fsUuid");
        return E_PARAMS_INVALID;
    }
    struct stat statbuf;
    mountUsbFusePath_ = StringPrintf("/mnt/data/external/%s", fsUuid.c_str());
    if (!lstat(mountUsbFusePath_.c_str(), &statbuf)) {
        LOGE("[L2:VolumeManager] CreateMountUsbFusePath: path exists, removing");
        remove(mountUsbFusePath_.c_str());
        return E_SYS_KERNEL_ERR;
    }
    if (mkdir(mountUsbFusePath_.c_str(), S_IRWXU | S_IRWXG | S_IXOTH)) {
        LOGE("[L2:VolumeManager] CreateMountUsbFusePath: <<< EXIT FAILED <<< mkdir failed, errno=%{public}d", errno);
        return E_MKDIR_MOUNT;
    }
    LOGI("[L2:VolumeManager] CreateMountUsbFusePath: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t VolumeManager::TryToFix(const std::string &volId, uint32_t flags)
{
    LOGI("[L2:VolumeManager] TryToFix: >>> ENTER <<< volId=%{public}s, flags=%{public}u", volId.c_str(), flags);

    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] TryToFix: <<< EXIT FAILED <<< volId=%{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
    }
    LOGI("[L2:VolumeManager] TryToFix: Try to fix %{public}s in VolumeManager::TryToFix", volId.c_str());
    int32_t currentState = info->GetState();
    if (currentState == DAMAGED_MOUNTED || currentState == MOUNTED) {
        int32_t errUmount = info->UMount();
        if (errUmount != E_OK) {
            LOGE("[L2:VolumeManager] TryToFix: <<< EXIT FAILED <<< UMount failed, ret=%{public}d", errUmount);
            return errUmount;
        }
    }
    LOGI("[L2:VolumeManager] TryToFix: Mount %{public}s done in VolumeManager::TryToFix", volId.c_str());
    int32_t errFix = info->TryToFix();
    LOGE("[L2:VolumeManager] TryToFix: The volume result: %{public}d.", errFix);
    if (errFix != E_OK) {
        LOGE("[L2:VolumeManager] TryToFix: the volume %{public}s fix failed, ret: %{public}d.", volId.c_str(), errFix);
    }

    LOGI("[L2:VolumeManager] TryToFix: After TryToFix %{public}s in VolumeManager::TryToFix", volId.c_str());
    int32_t errMount = info->Check();
    if (errMount != E_OK) {
        LOGE("[L2:VolumeManager] TryToFix: <<< EXIT FAILED <<< Check failed, err=%{public}d", errMount);
        return errMount;
    }
    LOGI("[L2:VolumeManager] TryToFix: Check done %{public}s in VolumeManager::TryToFix", volId.c_str());
    errMount = info->Mount(flags);
    if (errMount != E_OK) {
        LOGE("[L2:VolumeManager] TryToFix: <<< EXIT FAILED <<< Mount failed, err=%{public}d", errMount);
        return errMount;
    }
    LOGI("[L2:VolumeManager] TryToFix: Mount done %{public}s in VolumeManager::TryToFix", volId.c_str());
    StorageManagerClient client;
    errMount = client.NotifyVolumeMounted(info);
    if (errMount) {
        LOGE("[L2:VolumeManager] TryToFix: Notify Mounted failed");
    }
    LOGI("[L2:VolumeManager] TryToFix: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::UMount(const std::string &volId)
{
    LOGI("[L2:VolumeManager] UMount: >>> ENTER <<< volId=%{public}s", volId.c_str());

    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
        return OHOS::StorageDaemon::MtpDeviceMonitor::GetInstance().Umount(volId);
#else
        LOGE("[L2:VolumeManager] UMount: <<< EXIT FAILED <<< volId=%{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
#endif
    }

    int32_t err = info->UMount();
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] UMount: <<< EXIT FAILED <<< UMount failed, err=%{public}d", err);
        return err;
    }
    LOGI("[L2:VolumeManager] UMount: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::Format(const std::string &volId, const std::string &fsType)
{
    LOGI("[L2:VolumeManager] Format: >>> ENTER <<< volId=%{public}s, fsType=%{public}s", volId.c_str(), fsType.c_str());

    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] Format: <<< EXIT FAILED <<< volId=%{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->Format(fsType);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Format: <<< EXIT FAILED <<< Format failed, err=%{public}d", err);
        StorageRadar::ReportVolumeOperation("VolumeInfo::Format", err);
        return err;
    }

    LOGI("[L2:VolumeManager] Format: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::SetVolumeDescription(const std::string &volId, const std::string &description)
{
    LOGI("[L2:VolumeManager] SetVolumeDescription: >>> ENTER <<< volId=%{public}s", volId.c_str());

    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] SetVolumeDescription: the volume %{public}s does not exist.", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->SetVolumeDescription(description);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] SetVolumeDescription: <<< EXIT FAILED <<< failed, err=%{public}d", err);
        StorageRadar::ReportVolumeOperation("VolumeInfo::SetVolumeDescription", err);
        return err;
    }

    LOGI("[L2:VolumeManager] SetVolumeDescription: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t VolumeManager::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
    LOGI("[L2:VolumeManager] QueryUsbIsInUse: >>> ENTER <<< diskPath=%{public}s", diskPath.c_str());

    isInUse = true;
    if (diskPath.empty()) {
        LOGE("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT FAILED <<< diskPath is empty");
        return E_PARAMS_NULLPTR_ERR;
    }
    char realPath[PATH_MAX] = { 0 };
    if (realpath(diskPath.c_str(), realPath) == nullptr) {
        LOGE("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT FAILED <<< realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    if (strncmp(realPath, diskPath.c_str(), diskPath.size()) != 0) {
        LOGE("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT FAILED <<< path mismatch");
        return E_PARAMS_INVALID;
    }
    if (IsMtpDeviceInUse(diskPath)) {
        isInUse = true;
        LOGI("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT SUCCESS <<< MTP device in use");
        return E_OK;
    }
    int fd = open(realPath, O_RDONLY);
    if (fd < 0) {
        LOGE("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        return E_OPEN_FAILED;
    }
    int inUse = -1;
    if (ioctl(fd, STORAGE_MANAGER_IOC_CHK_BUSY, &inUse) < 0) {
        LOGE("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT FAILED <<< ioctl failed, errno=%{public}d", errno);
        close(fd);
        return E_IOCTL_FAILED;
    }

    if (inUse) {
        LOGI("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT SUCCESS <<< inUse=%{public}d", inUse);
        close(fd);
        isInUse = true;
        return E_OK;
    }
    LOGI("[L2:VolumeManager] QueryUsbIsInUse: usb not inUse");
    isInUse = false;
    close(fd);
    LOGI("[L2:VolumeManager] QueryUsbIsInUse: <<< EXIT SUCCESS <<< not in use");
    return E_OK;
}

int32_t VolumeManager::GetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] GetOddCapacity: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->GetOddCapacity(volumeId, totalSize, freeSize);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] GetOddCapacity: the volume %{public}s GetOddCapacity failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::GetOddCapacity", err);
        return err;
    }
    return E_OK;
}

bool VolumeManager::IsMtpDeviceInUse(const std::string &diskPath)
{
    if (diskPath.rfind(MTP_PATH_PREFIX, 0) != 0) {
        return false;
    }

    std::string key = "user.queryMtpIsInUse";
    char value[MTP_QUERY_RESULT_LEN] = { 0 };
    int32_t len = getxattr(diskPath.c_str(), key.c_str(), value, MTP_QUERY_RESULT_LEN);
    if (len < 0) {
        LOGE("[L2:VolumeManager] IsMtpDeviceInUse: Failed to getxattr for diskPath = %{public}s", diskPath.c_str());
        return false;
    }

    if ("true" == std::string(value)) {
        LOGI("[L2:VolumeManager] IsMtpDeviceInUse: MTP device in use for diskPath=%{public}s", diskPath.c_str());
        return true;
    }
    return false;
}

int32_t VolumeManager::Encrypt(const std::string &volumeId, const std::string &pazzword)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] Encrypt: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->Encrypt(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Encrypt: the volume %{public}s Encrypt failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::Encrypt", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::GetCryptProgressById(const std::string &volumeId, int32_t &progress)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] GetCryptProgressById: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->GetCryptProgressById(volumeId, progress);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] GetCryptProgressById: the volume %{public}s"
            "GetCryptProgressById failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::GetCryptProgressById", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::GetCryptUuidById(const std::string &volumeId, std::string &uuid)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] GetCryptUuidById: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->GetCryptUuidById(volumeId, uuid);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] GetCryptUuidById: the volume %{public}s GetCryptUuidById failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::GetCryptUuidById", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::BindRecoverKeyToPasswd(const std::string &volumeId,
                                              const std::string &pazzword,
                                              const std::string &recoverKey)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] BindRecoverKeyToPasswd: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->BindRecoverKeyToPasswd(volumeId, pazzword, recoverKey);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] BindRecoverKeyToPasswd: the volume %{public}s"
            "BindRecoverKeyToPasswd failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::BindRecoverKeyToPasswd", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::UpdateCryptPasswd(const std::string &volumeId,
                                         const std::string &pazzword,
                                         const std::string &newPazzword)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] UpdateCryptPasswd: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->UpdateCryptPasswd(volumeId, pazzword, newPazzword);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] UpdateCryptPasswd: the volume %{public}s UpdateCryptPasswd failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::UpdateCryptPasswd", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::ResetCryptPasswd(const std::string &volumeId,
                                        const std::string &recoverKey,
                                        const std::string &newPazzword)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] ResetCryptPasswd: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->ResetCryptPasswd(volumeId, recoverKey, newPazzword);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] ResetCryptPasswd: the volume %{public}s ResetCryptPasswd failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::ResetCryptPasswd", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] VerifyCryptPasswd: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->VerifyCryptPasswd(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] VerifyCryptPasswd: the volume %{public}s VerifyCryptPasswd failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::VerifyCryptPasswd", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::Unlock(const std::string &volumeId, const std::string &pazzword)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] Unlock: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->Unlock(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Unlock: the volume %{public}s Unlock failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::Unlock", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::Decrypt(const std::string &volumeId, const std::string &pazzword)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volumeId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] Decrypt: the volume %{public}s does not exist.", volumeId.c_str());
        return E_NON_EXIST;
    }
    int32_t err = info->Decrypt(volumeId, pazzword);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Decrypt: the volume %{public}s Decrypt failed.", volumeId.c_str());
        StorageRadar::ReportVolumeOperation("VolumeInfo::Decrypt", err);
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::Eject(const std::string &volId)
{
    LOGI("[L2:VolumeManager] Eject: >>> ENTER <<< volId=%{public}s", volId.c_str());
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] Eject:<<< EXIT FAILED <<< %{public}s does not exist.", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->Eject(volId);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] Eject:<<< EXIT FAILED <<< %{public}s failed err: %{public}d", volId.c_str(), err);
        StorageRadar::ReportVolumeOperation("VolumeInfo::Eject", err);
        return err;
    }
    LOGI("[L2:VolumeManager] Eject: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t VolumeManager::GetOpticalDriveOpsProgress(const std::string &volId, uint32_t &progress)
{
    LOGI("[L2:VolumeManager] GetOpticalDriveOpsProgress: >>> ENTER <<< volId=%{public}s", volId.c_str());
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("[L2:VolumeManager] GetOpticalDriveOpsProgress :<<< EXIT FAILED <<< %{public}s does not exist.", 
            volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->GetOpticalDriveOpsProgress(volId, progress);
    if (err != E_OK) {
        LOGE("[L2:VolumeManager] GetOpticalDriveOpsProgress :<<< EXIT FAILED <<< %{public}s failed err: %{public}d", 
            volId.c_str(), err);
        StorageRadar::ReportVolumeOperation("VolumeInfo::GetOpticalDriveOpsProgress", err);
        return err;
    }
    LOGI("[L2:VolumeManager] GetOpticalDriveOpsProgress: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}
} // StorageDaemon
} // OHOS
