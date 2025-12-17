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

#include "volume/external_volume_info.h"

#include <cstring>
#include <fcntl.h>
#include <future>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>
#include <sys/sysmacros.h>

#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "volume/process.h"

#define STORAGE_MANAGER_IOC_CHK_BUSY _IOR(0xAC, 77, int)
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr const char *NTFS_LABEL_DESC_PREFIX = "Volume label :  ";
using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t WAIT_THREAD_TIMEOUT_S = 60;
constexpr int32_t FILE_NOT_EXIST = 2;
constexpr int UID_FILE_MANAGER = 1006;
int32_t ExternalVolumeInfo::ReadMetadata()
{
    int32_t ret = OHOS::StorageDaemon::ReadMetadata(devPath_, fsUuid_, fsType_, fsLabel_);
    if (fsType_ == "ntfs" && (fsLabel_.find('?') != std::string::npos || fsLabel_ == "")) {
        std::vector<std::string> cmd;
        cmd = {
            "ntfslabel",
            "-v",
            devPath_
        };
        fsLabel_ = GetVolDescByNtfsLabel(cmd);
    }
    return ret;
}

std::string ExternalVolumeInfo::GetFsType()
{
    return fsType_;
}

std::string ExternalVolumeInfo::GetFsUuid()
{
    return fsUuid_;
}

std::string ExternalVolumeInfo::GetFsLabel()
{
    return fsLabel_;
}

std::string ExternalVolumeInfo::GetMountPath()
{
    return mountPath_;
}

bool ExternalVolumeInfo::GetDamagedFlag()
{
    return isDamaged_;
}

int32_t ExternalVolumeInfo::DoCreate(dev_t dev)
{
    int32_t ret = 0;
    string id = VolumeInfo::GetVolumeId();

    device_ = dev;
    devPath_ = StringPrintf(devPathDir_.c_str(), (id).c_str());

    ret = mknod(devPath_.c_str(), S_IFBLK, dev);
    if (ret) {
        LOGE("External volume DoCreate error.");
        return E_ERR;
    }
    return E_OK;
}

std::string ExternalVolumeInfo::GetFsTypeByDev(dev_t dev)
{
    std::string volId = StringPrintf("vol-%u-%u", major(dev), minor(dev));
    std::string devPath = "/dev/block" + volId;
    std::string fsType;
    std::string uuid;
    std::string label;
    OHOS::StorageDaemon::ReadMetadata(devPath, uuid, fsType, label);
    LOGI("GetFsTypeByDev. devPath: %{public}s,fsType: %{public}s", devPath.c_str(), fsType.c_str());
    return fsType;
}

int32_t ExternalVolumeInfo::DoDestroy()
{
    int err = remove(devPath_.c_str());
    if (err) {
        LOGE("External volume DoDestroy error.");
        return E_ERR;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoMount4Ext(uint32_t mountFlags)
{
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, "");
    if (ret) {
        return E_EXT_MOUNT;
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Hmfs(uint32_t mountFlags)
{
    const char *fsType = "hmfs";
    auto mountData = StringPrintf("context=u:object_r:mnt_external_file:s0,errors=continue");
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType, mountFlags, mountData.c_str());
    if (!ret) {
        StorageRadar::ReportVolumeOperation("ExternalVolumeInfo::DoMount4Hmfs", ret);
    } else {
        LOGE("initial mount failed errno %{public}d", errno);
        return ret;
    }
 
    ret = umount(mountPath_.c_str());
    if (ret != E_OK) {
        LOGE("umount failed errno %{public}d", errno);
        return ret;
    }
 
    auto mountDataWithoutOpt = StringPrintf("context=u:object_r:mnt_external_file:s0");
    ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType, MS_RDONLY, mountDataWithoutOpt.c_str());
    if (ret != E_OK) {
        LOGE("mount read only failed errno %{public}d", errno);
        return ret;
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Ntfs(uint32_t mountFlags)
{
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    auto mountData = StringPrintf("rw,big_writes,uid=%d,gid=%d,dmask=0006,fmask=0007",
        UID_FILE_MANAGER, UID_FILE_MANAGER);
#else
    auto mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
#endif
    if (mountFlags & MS_RDONLY) {
        mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    }

    std::vector<std::string> cmd = {
        "mount.ntfs",
        devPath_,
        mountPath_,
        "-o",
        mountData.c_str()
    };
    std::vector<std::string> output;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        LOGE("ext exec mount for ntfs failed, errno is %{public}d.", errno);
        return E_NTFS_MOUNT;
    }
    return E_OK;
#else
    if (ForkExec(cmd, &output) != E_OK) {
        LOGE("exec mount for ntfs failed, errno is %{public}d.", errno);
        return E_NTFS_MOUNT;
    }
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4Exfat(uint32_t mountFlags)
{
    auto mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    if (mountFlags & MS_RDONLY) {
        mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    }

    std::vector<std::string> cmd = {
        "mount.exfat",
        "-o",
        mountData.c_str(),
        devPath_,
        mountPath_,
    };
    std::vector<std::string> output;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        LOGE("ext exec mount for exfat failed, errno is %{public}d.", errno);
        return E_EXFAT_MOUNT;
    }
    return E_OK;
#else
    if (ForkExec(cmd, &output) != E_OK) {
        LOGE("exec mount for exfat failed, errno is %{public}d.", errno);
        return E_EXFAT_MOUNT;
    }
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoCheck4Exfat()
{
    std::vector<std::string> cmd = {
        "fsck.exfat",
        "-n",
        devPath_,
    };
    int execRet = ForkExecWithExit(cmd);
    LOGI("execRet: %{public}d", execRet);
    if (execRet != E_OK) {
        return E_VOL_NEED_FIX;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoCheck4Ntfs()
{
    std::vector<std::string> cmd = {
        "fsck.ntfs",
        devPath_,
    };
    int exitStatus = 0;
    int execRet = ForkExecWithExit(cmd, &exitStatus);
    LOGI("execRet: %{public}d, exitStatus: %{public}d", execRet, exitStatus);
    if (exitStatus != 1) {
        return E_VOL_NEED_FIX;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoFix4Ntfs()
{
    LOGE("DoFix4Ntfs");
    std::vector<std::string> cmd = {
        "ntfsfix",
        "-d",
        devPath_
    };
    std::vector<std::string> output;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        LOGE("ext exec fix for ntfs failed, errno is %{public}d.", errno);
        return E_VOL_FIX_FAILED;
    }
    return E_OK;
#else
    if (ForkExec(cmd, &output) != E_OK) {
        LOGE("exec fix for ntfs failed, errno is %{public}d.", errno);
        return E_VOL_FIX_FAILED;
    }
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoFix4Exfat()
{
    LOGI("DoFix4Exfat");
    LOGI("devPath_ is %{public}s", devPath_.c_str());
    std::vector<std::string> cmd = {
        "fsck.exfat",
        "-p",
        devPath_,
    };
    std::vector<std::string> output;
    int exitStatus = 0;
    int forkExecRes = 0;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    forkExecRes = ExtStorageMountForkExec(cmd, &exitStatus);
    LOGI("ExtStorageMountForkExec is %{public}d, exitStatus is %{public}d", forkExecRes, exitStatus);
    if (exitStatus != 1 || forkExecRes != E_OK) {
        LOGE("ext exec fix for exfat failed, errno is %{public}d, forkExecRes is %{public}d.", errno, forkExecRes);
        return E_VOL_FIX_FAILED;
    }
    return E_OK;
#else
    forkExecRes = ForkExec(cmd, &output, &exitStatus);
    LOGI("forkExecRes is %{public}d, exitStatus is %{public}d", forkExecRes, exitStatus);
    if (exitStatus != 1 || forkExecRes != E_OK) {
        LOGE("exec fix for exfat failed, errno is %{public}d, forkExecRes is %{public}d.", errno, forkExecRes);
        return E_VOL_FIX_FAILED;
    }
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4OtherType(uint32_t mountFlags)
{
    mountFlags |= MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, mountData.c_str());
    if (ret) {
        return E_OTHER_MOUNT;
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Vfat(uint32_t mountFlags)
{
    mountFlags |= MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0006,fmask=0007,utf8", UID_FILE_MANAGER, UID_FILE_MANAGER);
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, mountData.c_str());
    if (ret) {
        return E_FAT_MOUNT;
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoMount(uint32_t mountFlags)
{
    int32_t ret = DoCheck();
    if (ret != E_OK) {
        LOGE("External volume uuid=%{public}s check failed.", GetAnonyString(GetFsUuid()).c_str());
        return E_DOCHECK_MOUNT;
    }
    if (IsUsbFuseByTypeClient(fsType_)) {
        ret = CreateFuseMountPath();
    } else {
        ret = CreateMountPath();
    }
    if (ret != E_OK) {
        return ret;
    }
    if ((fsType_ == "hmfs" || fsType_ == "f2fs") && GetIsUserdata()) {
        ret = DoMount4Hmfs(mountFlags);
    }
    if (ret) {
        LOGE("External volume DoMount error, errno = %{public}d", errno);
        remove(mountPath_.c_str());
        return E_HMFS_MOUNT;
    }
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread mountThread ([this, mountFlags, p = std::move(promise)]() mutable {
        LOGI("Ready to mount: volume fstype is %{public}s, mountflag is %{public}d", fsType_.c_str(), mountFlags);
        int retValue = E_OK;
        if (fsType_ == "ntfs") retValue = DoMount4Ntfs(mountFlags);
        else if (fsType_ == "exfat") retValue = DoMount4Exfat(mountFlags);
        else if (fsType_ == "vfat" || fsType_ == "fat32") retValue = DoMount4Vfat(mountFlags);
        else if ((fsType_ == "hmfs" || fsType_ == "f2fs") && GetIsUserdata()) retValue = E_OK;
        else retValue = E_OTHER_MOUNT;
        p.set_value(retValue);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("Mount timed out");
        remove(mountPath_.c_str());
        mountThread.detach();
        return E_TIMEOUT_MOUNT;
    }
    ret = future.get();
    mountThread.join();
    if (ret) {
        LOGE("External volume DoMount error, errno = %{public}d", errno);
        remove(mountPath_.c_str());
        return ret;
    }
    mountPath_ = mountBackupPath_;
    return E_OK;
}

bool ExternalVolumeInfo::IsUsbFuseByTypeClient(std::string fsType)
{
    StorageManagerClient client;
    bool isUsbFuseByType = true;
    client.IsUsbFuseByType(fsType, isUsbFuseByType);
    return isUsbFuseByType;
}

int32_t ExternalVolumeInfo::IsUsbInUse(int fd)
{
    int32_t inUse = -1;
    if (ioctl(fd, STORAGE_MANAGER_IOC_CHK_BUSY, &inUse) < 0) {
        LOGE("ioctl check in use failed errno %{public}d", errno);
        return E_IOCTL_FAILED;
    }
    if (inUse) {
        LOGI("usb inuse number is %{public}d", inUse);
        return E_USB_IN_USE;
    }
    LOGI("usb not inUse");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMount(bool force)
{
    StorageManagerClient client;
    bool isUsbFuseByType = true;
    client.IsUsbFuseByType(fsType_, isUsbFuseByType);
    if (force && !isUsbFuseByType) {
        LOGI("External volume start force to unmount.");
        Process ps(mountPath_);
        ps.UpdatePidByPath();
        ps.KillProcess(SIGKILL);
        umount2(mountPath_.c_str(), MNT_DETACH);
        remove(mountPath_.c_str());
        LOGI("External volume force to unmount success.");
        return E_OK;
    }
    if (isUsbFuseByType) {
        mountPath_ = mountUsbFusePath_;
    }
    int fd = open(mountPath_.c_str(), O_RDONLY);
    if (fd < 0) {
        LOGE("open file fail mountPath %{public}s, errno %{public}d", GetAnonyString(mountPath_).c_str(), errno);
    }
    if (fd >= 0) {
        IsUsbInUse(fd);
    }

    LOGI("External volume start to unmount mountPath: %{public}s.", GetAnonyString(mountPath_).c_str());
    int ret = umount2(mountPath_.c_str(), MNT_DETACH);
    if (ret != E_OK) {
        LOGE("umount2 failed errno %{public}d", errno);
    }
    if (fd >= 0) {
        IsUsbInUse(fd);
        close(fd);
    }

    int err = remove(mountPath_.c_str());
    if (err && ret) {
        LOGE("External volume DoUmount error.");
        return E_VOL_UMOUNT_ERR;
    }
    if (err && errno != FILE_NOT_EXIST) {
        LOGE("failed to call remove(%{public}s) error, errno = %{public}d", GetAnonyString(mountPath_).c_str(), errno);
        return E_RMDIR_MOUNT;
    }
    mountPath_ = mountBackupPath_;

    LOGI("External volume unmount success.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMountUsbFuse()
{
    LOGI("DoUMountUsbFuse in.");
    int ret = umount2(mountPath_.c_str(), MNT_DETACH);
    if (ret != E_OK) {
        LOGE("umount2 mountUsbFusePath failed errno %{public}d", errno);
    }
 
    int err = rmdir(mountPath_.c_str());
    if (err) {
        LOGE("External volume DoUMountUsbFuse error: rmdir failed, errno %{public}d", errno);
        return E_RMDIR_MOUNT;
    }
    LOGI("DoUMountUsbFuse success.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoTryToCheck()
{
    int32_t ret = DoCheck();
    if (ret != E_OK) {
        LOGE("External volume uuid=%{public}s check failed.", GetAnonyString(GetFsUuid()).c_str());
        return E_DOCHECK_MOUNT;
    }

    if (fsType_ != "ntfs" && fsType_ != "exfat") {
        LOGE("Volume type %{public}s, is not support fix", fsType_.c_str());
        return E_VOL_FIX_NOT_SUPPORT;
    }

    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread mountThread ([this, p = std::move(promise)]() mutable {
        LOGI("Ready to DoTryToCheck: volume fstype is %{public}s", fsType_.c_str());
        int retValue = E_OK;
        if (fsType_ == "ntfs") retValue = DoCheck4Ntfs();
        else if (fsType_ == "exfat") retValue = DoCheck4Exfat();
        else retValue = E_VOL_FIX_NOT_SUPPORT;
        LOGI("DoTryToCheck cmdRet:%{public}d", retValue);
        p.set_value(retValue);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("DoTryToCheck timed out");
        mountThread.detach();
        return E_TIMEOUT_MOUNT;
    }

    ret = future.get();
    mountThread.join();
    if (ret != E_OK) {
        LOGE("External volume DoTryToCheck error, maybe need fix, errno = %{public}d", ret);
        return E_VOL_NEED_FIX;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoTryToFix()
{
    int32_t ret = DoCheck();
    if (ret != E_OK) {
        LOGE("External volume uuid=%{public}s check failed.", GetAnonyString(GetFsUuid()).c_str());
        return E_DOCHECK_MOUNT;
    }

    if (fsType_ != "ntfs" && fsType_ != "exfat") {
        LOGE("Volume type %{public}s, is not support fix", fsType_.c_str());
        return E_VOL_FIX_NOT_SUPPORT;
    }
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread mountThread ([this, p = std::move(promise)]() mutable {
        LOGI("Ready to mount: volume fstype is %{public}s", fsType_.c_str());
        int retValue = E_OK;
        if (fsType_ == "ntfs") retValue = DoFix4Ntfs();
        else if (fsType_ == "exfat") retValue = DoFix4Exfat();
        p.set_value(retValue);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("Fix timed out");
        mountThread.detach();
        return E_TIMEOUT_MOUNT;
    }

    ret = future.get();
    mountThread.join();
    LOGI("DoTryToFix: volume fstype is %{public}s, ret: %{public}d, errno: %{public}d", fsType_.c_str(), ret, errno);
    return ret;
}

int32_t ExternalVolumeInfo::DoCheck()
{
    int32_t ret = ExternalVolumeInfo::ReadMetadata();
    if (ret) {
        LOGE("External volume uuid=%{public}s DoCheck failed.", GetAnonyString(GetFsUuid()).c_str());
        return E_CHECK;
    }

    // check fstype
    for (std::string item : supportMountType_) {
        if (item == fsType_) {
            return E_OK;
        }
    }
    LOGE("External Volume type not support.");
    return E_NOT_SUPPORT;
}

int32_t ExternalVolumeInfo::DoFormat(std::string type)
{
    int32_t err = 0;
    StorageManagerClient client;
    bool isUsbFuseByType = true;
    client.IsUsbFuseByType(fsType_, isUsbFuseByType);
    if (isUsbFuseByType && IsPathMounted(mountPath_)) {
        err = DoUMountUsbFuse();
    }
    if (err != E_OK) {
        return err;
    }
    
    std::map<std::string, std::string>::iterator iter = supportFormatType_.find(type);
    if (iter == supportFormatType_.end()) {
        LOGE("External volume format not support.");
        return E_NOT_SUPPORT;
    }
    std::vector<std::string> output;
    if (type == "vfat") {
        std::vector<std::string> cmd = {
            iter->second,
            "-A",
            devPath_
        };
        err = ForkExec(cmd, &output);
    } else {
        std::vector<std::string> cmd = {
            iter->second,
            devPath_
        };
        err = ForkExec(cmd, &output);
    }

    if (err == E_NO_CHILD) {
        err = E_OK;
    }
    ReadMetadata();
    LOGI("do format end, err is %{public}d.", err);
    return err;
}

int32_t ExternalVolumeInfo::DoSetVolDesc(std::string description)
{
    int32_t err = 0;
    std::vector<std::string> output;
    if (fsType_ == "ntfs") {
        std::vector<std::string> fixCmd = {
            "ntfsfix",
            "-d",
            devPath_
        };
        err = ForkExec(fixCmd, &output);
        std::vector<std::string> labelCmd = {
            "ntfslabel",
            devPath_,
            description
        };
        output.clear();
        err = ForkExec(labelCmd, &output);
    } else if (fsType_ == "exfat") {
        std::vector<std::string> cmd = {
            "exfatlabel",
            devPath_,
            description
        };
        err = ForkExec(cmd, &output);
    } else if (fsType_ == "hmfs") {
        std::vector<std::string> cmd = {
            "hmfslabel",
            devPath_,
            description
        };
        err = ForkExec(cmd, &output);
    } else {
        LOGE("SetVolumeDescription fsType not support.");
        return E_NOT_SUPPORT;
    }
    ReadMetadata();
    LOGI("do set vol desc end, err is %{public}d.", err);
    return err;
}

int32_t ExternalVolumeInfo::CreateFuseMountPath()
{
    struct stat statbuf;
    mountBackupPath_ = StringPrintf(mountPathDir_.c_str(), fsUuid_.c_str());
    mountUsbFusePath_ = StringPrintf(mountFusePathDir_.c_str(), fsUuid_.c_str());
    std::string mountFusePath = "/mnt/data/external_fuse";
    bool ret = true;
    if (lstat(mountFusePath.c_str(), &statbuf)) {
        ret = PrepareDir("/mnt/data/external_fuse", S_IRWXU | S_IRWXG | S_IXOTH, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!ret) {
            LOGE("create path %{public}s failed", mountFusePath.c_str());
            return E_MKDIR_MOUNT;
        }
    }
 
    if (!lstat(mountUsbFusePath_.c_str(), &statbuf)) {
        LOGE("volume mount path %{public}s exists, please remove first", GetAnonyString(mountUsbFusePath_).c_str());
        remove(mountUsbFusePath_.c_str());
        return E_SYS_KERNEL_ERR;
    }
    ret = PrepareDir(mountUsbFusePath_, S_IRWXU | S_IRWXG | S_IXOTH, FILE_MANAGER_UID, FILE_MANAGER_GID);
    if (!ret) {
        LOGE("the volume %{public}s create path %{public}s failed",
             GetVolumeId().c_str(), GetAnonyString(mountUsbFusePath_).c_str());
        return E_MKDIR_MOUNT;
    }
    mountPath_ = mountUsbFusePath_;
    return E_OK;
}
 
int32_t ExternalVolumeInfo::CreateMountPath()
{
    struct stat statbuf;
    mountBackupPath_ = StringPrintf(mountPathDir_.c_str(), fsUuid_.c_str());
    if (!lstat(mountBackupPath_.c_str(), &statbuf)) {
        LOGE("volume mount path %{public}s exists, please remove first", mountBackupPath_.c_str());
        remove(mountBackupPath_.c_str());
        return E_SYS_KERNEL_ERR;
    }
    mode_t originalUmask = umask(0);
    if (mkdir(mountBackupPath_.c_str(), S_IRWXU | S_IRWXG | S_IXOTH)) {
        LOGE("the volume %{public}s create path %{public}s failed", GetVolumeId().c_str(), mountBackupPath_.c_str());
        return E_MKDIR_MOUNT;
    }
    umask(originalUmask);
    mountPath_ = mountBackupPath_;
    return E_OK;
}

std::string ExternalVolumeInfo::GetVolDescByNtfsLabel(std::vector<std::string> &cmd)
{
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);
    if (ret != E_OK) {
        LOGE("exec ntfs label failed, ret is: %{public}d, output size is %{public}zu", ret, output.size());
        StorageRadar::ReportVolumeOperation("ForkExec", ret);
        return "";
    }
    return SplitOutputIntoLines(output);
}

std::string ExternalVolumeInfo::SplitOutputIntoLines(std::vector<std::string> &output)
{
    std::vector<std::string> lines;
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        lines.insert(lines.end(), split.begin(), split.end());
    }
    if (lines.empty()) {
        LOGE("lines is empty");
        return "";
    }
    std::string volDesc;
    for (size_t i = lines.size() - 1; i < lines.size(); --i) {
        std::string line = lines[i];
        std::string::size_type index = line.find(NTFS_LABEL_DESC_PREFIX);
        if (index == std::string::npos) {
            continue;
        }
        volDesc = line.substr(index + std::strlen(NTFS_LABEL_DESC_PREFIX));
        break;
    }
    LOGE("exec ntfs label success, label is %{public}s", volDesc.c_str());
    return volDesc;
}
} // StorageDaemon
} // OHOS
