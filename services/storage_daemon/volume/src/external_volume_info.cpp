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

#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <sys/mount.h>
#include <csignal>
#include <algorithm>
#include <sys/wait.h>
#include <cstring>

#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/string_utils.h"
#include "volume/process.h"
#include "utils/file_utils.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
std::string ExternalVolumeInfo::GetBlkidData(const std::string type)
{
    std::vector<std::string> output;
    std::vector<std::string> cmd = {
        "blkid",
        "-s",
        type,
        "-o",
        "value",
        devPath_
    };

    int32_t err = ForkExec(cmd, &output);
    if (err) {
        return "";
    }

    if (output.size() > 0) {
        size_t sep = string::npos;
        sep = output[0].find_first_of("\n");
        if (sep != string::npos)
            output[0].resize(sep);
        return output[0];
    }
    return "";
}

int32_t ExternalVolumeInfo::ReadMetadata()
{
    fsUuid_ = GetBlkidData("UUID");
    fsType_ = GetBlkidData("TYPE");
    fsLabel_ = GetBlkidData("LABEL");

    if (fsUuid_.empty() || fsType_.empty()) {
        LOGE("External volume ReadMetadata error.");
        return E_ERR;
    }
    LOGI("ReadMetadata, fsUuid=%{public}s, fsType=%{public}d, fsLabel=%{public}s.",
         GetFsUuid().c_str(), GetFsType(), GetFsLabel().c_str());
    return E_OK;
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

    ret = ReadMetadata();
    if (ret) {
        LOGE("External volume ReadMetadata failed.");
        remove(devPath_.c_str());
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

int32_t ExternalVolumeInfo::DoMount(const std::string mountPath, uint32_t mountFlags)
{
    int32_t ret = 0;
    mode_t mode = 0777;

    if (GetFsType() == -1) {
        return E_NOT_SUPPORT;
    }

    if (fsType_ == "ext2" || fsType_ == "ext3" || fsType_ == "ext4") {
        ret = mount(devPath_.c_str(), mountPath.c_str(), fsType_.c_str(), mountFlags, "");
        if (!ret) {
            TravelChmod(mountPath, mode);
        }
    } else {
        ret = mount(devPath_.c_str(), mountPath.c_str(), fsType_.c_str(), MS_MGC_VAL, "fmask=000,dmask=000");
    }

    if (ret) {
        LOGE("External volume DoMount error.");
        return E_MOUNT;
    }

    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMount(const std::string mountPath, bool force)
{
    if (force) {
        LOGI("External volume start force to unmount.");
        Process ps(mountPath);
        ps.UpdatePidByPath();
        ps.KillProcess(SIGKILL);
        umount2(mountPath.c_str(), MNT_DETACH);
        remove(mountPath.c_str());
        return E_OK;
    }

    int ret = umount(mountPath.c_str());
    int err = remove(mountPath.c_str());
    if (err && ret) {
        LOGE("External volume DoUmount error.");
        return E_UMOUNT;
    }

    if (err) {
        LOGE("failed to call remove(%{public}s) error, errno = %{public}d", mountPath.c_str(), errno);
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

    if (type == "ext2" || type == "ext3" || type == "ext4") {
        std::vector<std::string> cmd = {
            iter->second,
            "-F",
            "-t",
            type,
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
} // StorageDaemon
} // OHOS