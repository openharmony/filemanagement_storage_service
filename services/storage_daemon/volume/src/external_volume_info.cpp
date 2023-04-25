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

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "volume/process.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
int32_t ExternalVolumeInfo::ReadMetadata()
{
    int32_t ret = OHOS::StorageDaemon::ReadMetadata(devPath_, fsUuid_, fsType_, fsLabel_);
    if (fsType_ == "ntfs") {
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

int32_t ExternalVolumeInfo::DoMount(uint32_t mountFlags)
{
    int32_t ret = 0;
    mode_t mode = 0777;
    struct stat statbuf;

    if (GetFsType() == -1) {
        return E_NOT_SUPPORT;
    }

    ret = ReadMetadata();
    if (ret) {
        LOGE("External volume ReadMetadata failed.");
        return E_ERR;
    }
    mountPath_ = StringPrintf(mountPathDir_.c_str(), fsUuid_.c_str());

    // check if dir exists
    ret = lstat(mountPath_.c_str(), &statbuf);
    if (!ret) {
        LOGE("volume mount path %{public}s exists, please remove first", GetMountPath().c_str());
        return E_MOUNT;
    }

    ret = mkdir(mountPath_.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    if (ret) {
        LOGE("the volume %{public}s create mount file %{public}s failed",
             GetVolumeId().c_str(), GetMountPath().c_str());
        return E_MOUNT;
    }

    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    if (fsType_ == "ext2" || fsType_ == "ext3" || fsType_ == "ext4") {
        ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, "");
        if (!ret) {
            TravelChmod(mountPath_, mode);
        }
    } else if (fsType_ == "ntfs") {
        if (mountFlags & MS_RDONLY) {
            mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
        } else {
            mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0007,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
        }
        std::vector<std::string> cmd = {
            "mount.ntfs",
            devPath_,
            mountPath_,
            "-o",
            mountData.c_str()
        };
        ret = ForkExec(cmd);
    } else {
        mountFlags |= MS_MGC_VAL;
        ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, mountData.c_str());
    }

    if (ret) {
        LOGE("External volume DoMount error.");
        remove(mountPath_.c_str());
        return E_MOUNT;
    }

    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMount(bool force)
{
    if (force) {
        LOGI("External volume start force to unmount.");
        Process ps(mountPath_);
        ps.UpdatePidByPath();
        ps.KillProcess(SIGKILL);
        umount2(mountPath_.c_str(), MNT_DETACH);
        remove(mountPath_.c_str());
        return E_OK;
    }

    int ret = umount(mountPath_.c_str());
    int err = remove(mountPath_.c_str());
    if (err && ret) {
        LOGE("External volume DoUmount error.");
        return E_UMOUNT;
    }

    if (err) {
        LOGE("failed to call remove(%{public}s) error, errno = %{public}d", mountPath_.c_str(), errno);
        return E_SYS_CALL;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoCheck()
{
    int32_t ret = ExternalVolumeInfo::ReadMetadata();
    if (ret) {
        LOGE("External volume uuid=%{public}s DoCheck failed.", GetFsUuid().c_str());
        return E_ERR;
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
        std::vector<std::string> cmd = {
            "ntfslabel",
            devPath_,
            description
        };
        err = ForkExec(cmd);
    } else if (fsType_ == "exfat") {
        std::vector<std::string> cmd = {
            "exfatlabel",
            devPath_,
            description
        };
        err = ForkExec(cmd);
    } else if (fsType_ == "vfat") {
        std::vector<std::string> cmd = {
            "newfs_msdos",
            "-L",
            description,
            devPath_
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
