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

#include "volume/external_volume_info.h"

#include <sys/stat.h>
#include <sys/wait.h>
#include <future>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "volume/process.h"

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t WAIT_THREAD_TIMEOUT_S = 15;
constexpr int32_t FILE_NOT_EXIST = 2;
constexpr int UID_FILE_MANAGER = 1006;
int32_t ExternalVolumeInfo::ReadMetadata()
{
    int32_t ret = OHOS::StorageDaemon::ReadMetadata(devPath_, fsUuid_, fsType_, fsLabel_);
    if (fsType_ == "ntfs" && (fsLabel_.find('?') != std::string::npos || fsLabel_ == "")) {
        std::vector<std::string> cmd;
        cmd = {
            "ntfslabel",
            devPath_
        };
        fsLabel_ = GetBlkidDataByCmd(cmd);
    }
    return ret;
}

int32_t ExternalVolumeInfo::GetFsType()
{
    for (uint32_t i = 0; i < supportMountType_.size(); i++) {
        if (supportMountType_[i].compare(fsType_) == 0) {
            return i;
        }
    }
    return -1;
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
    mode_t mode = 0777;
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, "");
    if (ret) {
        return E_EXT_MOUNT;
    }
    TravelChmod(mountPath_, mode);
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Hmfs(uint32_t mountFlags)
{
    mode_t mode = 0777;
    const char *fsType = "hmfs";
    auto mountData = StringPrintf("context=u:object_r:mnt_external_file:s0");
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType, mountFlags, mountData.c_str());
    if (!ret) {
        TravelChmod(mountPath_, mode);
        StorageRadar::ReportVolumeOperation("ExternalVolumeInfo::DoMount4Hmfs", ret);
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Ntfs(uint32_t mountFlags)
{
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    auto mountData = StringPrintf("rw,big_writes,uid=%d,gid=%d,dmask=0007,fmask=0007",
        UID_FILE_MANAGER, UID_FILE_MANAGER);
#else
    auto mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
#endif
    if (mountFlags & MS_RDONLY) {
        mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    }

    std::vector<std::string> cmd = {
        "mount.ntfs",
        devPath_,
        mountPath_,
        "-o",
        mountData.c_str()
    };
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        return E_NTFS_MOUNT;
    }
    return E_OK;
#else
    if (ForkExec(cmd) != E_OK) {
        return E_NTFS_MOUNT;
    }
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4Exfat(uint32_t mountFlags)
{
    auto mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    if (mountFlags & MS_RDONLY) {
        mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    }

    std::vector<std::string> cmd = {
        "mount.exfat",
        "-o",
        mountData.c_str(),
        devPath_,
        mountPath_,
    };
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        return E_EXFAT_MOUNT;
    }
    return E_OK;
#else
    if (ForkExec(cmd) != E_OK) {
        return E_EXFAT_MOUNT;
    }
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4OtherType(uint32_t mountFlags)
{
    mountFlags |= MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, mountData.c_str());
    if (ret) {
        return E_OTHER_MOUNT;
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Vfat(uint32_t mountFlags)
{
    mountFlags |= MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0007,fmask=0007,utf8", UID_FILE_MANAGER, UID_FILE_MANAGER);
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

    struct stat statbuf;
    mountPath_ = StringPrintf(mountPathDir_.c_str(), fsUuid_.c_str());
    if (!lstat(mountPath_.c_str(), &statbuf)) {
        LOGE("volume mount path %{public}s exists, please remove first", GetMountPath().c_str());
        remove(mountPath_.c_str());
        return E_SYS_KERNEL_ERR;
    }

    if (mkdir(mountPath_.c_str(), S_IRWXU | S_IRWXG | S_IXOTH)) {
        LOGE("the volume %{public}s create path %{public}s failed", GetVolumeId().c_str(), GetMountPath().c_str());
        return E_MKDIR_MOUNT;
    }

    if (fsType_ == "hmfs" || fsType_ == "f2fs") ret = DoMount4Hmfs(mountFlags);
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
        if (fsType_ == "ext2" || fsType_ == "ext3" || fsType_ == "ext4") retValue = DoMount4Ext(mountFlags);
        else if (fsType_ == "ntfs") retValue = DoMount4Ntfs(mountFlags);
        else if (fsType_ == "exfat") retValue = DoMount4Exfat(mountFlags);
        else if (fsType_ == "vfat" || fsType_ == "fat32") retValue = DoMount4Vfat(mountFlags);
        else if (fsType_ != "hmfs" && fsType_ != "f2fs") retValue = DoMount4OtherType(mountFlags);
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
    LOGI("external volume mount success");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMount(bool force)
{
    if (force) {
        LOGI("External volume start force to unmount.");
        Process ps(mountPath_);
        ps.UpdatePidAndKill(SIGKILL);
        umount2(mountPath_.c_str(), MNT_DETACH);
        remove(mountPath_.c_str());
        LOGI("External volume force to unmount success.");
        return E_OK;
    }
    LOGI("External volume start to unmount.");
    int ret = umount(mountPath_.c_str());
    int err = remove(mountPath_.c_str());
    if (err && ret) {
        LOGE("External volume DoUmount error.");
        return E_VOL_UMOUNT_ERR;
    }

    if (err && errno != FILE_NOT_EXIST) {
        LOGE("failed to call remove(%{public}s) error, errno = %{public}d", mountPath_.c_str(), errno);
        return E_RMDIR_MOUNT;
    }
    LOGI("External volume unmount success.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoCheck()
{
    int32_t ret = ExternalVolumeInfo::ReadMetadata();
    if (ret) {
        LOGE("External volume uuid=%{public}s DoCheck failed.", GetAnonyString(GetFsUuid()).c_str());
        return E_CHECK;
    }

    // check fstype
    if (GetFsType() == -1) {
        LOGE("External Volume type not support.");
        return E_NOT_SUPPORT;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoFormat(std::string type)
{
    int32_t err = 0;
    std::map<std::string, std::string>::iterator iter = supportFormatType_.find(type);
    if (iter == supportFormatType_.end()) {
        LOGE("External volume format not support.");
        return E_NOT_SUPPORT;
    }

    if (type == "vfat") {
        std::vector<std::string> cmd = {
            iter->second,
            "-A",
            devPath_
        };
        err = ForkExec(cmd);
    } else {
        std::vector<std::string> cmd = {
            iter->second,
            devPath_
        };
        err = ForkExec(cmd);
    }

    if (err == E_NO_CHILD) {
        err = E_OK;
    }

    ReadMetadata();
    return err;
}

int32_t ExternalVolumeInfo::DoSetVolDesc(std::string description)
{
    int32_t err = 0;
    if (fsType_ == "ntfs") {
        std::vector<std::string> fixCmd = {
            "ntfsfix",
            "-d",
            devPath_
        };
        err = ForkExec(fixCmd);
        std::vector<std::string> labelCmd = {
            "ntfslabel",
            devPath_,
            description
        };
        err = ForkExec(labelCmd);
    } else if (fsType_ == "exfat") {
        std::vector<std::string> cmd = {
            "exfatlabel",
            devPath_,
            description
        };
        err = ForkExec(cmd);
    } else if (fsType_ == "hmfs") {
        std::vector<std::string> cmd = {
            "hmfslabel",
            devPath_,
            description
        };
        err = ForkExec(cmd);
    } else {
        LOGE("SetVolumeDescription fsType not support.");
        return E_NOT_SUPPORT;
    }

    ReadMetadata();
    return err;
}
} // StorageDaemon
} // OHOS
