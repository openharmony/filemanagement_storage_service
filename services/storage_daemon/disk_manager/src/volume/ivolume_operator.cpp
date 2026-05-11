/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "disk_manager/volume/ivolume_operator.h"

#include <climits>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

constexpr const char *MOUNT_PATH_PREFIX = "/mnt/data/";

int32_t IVolumeOperator::EnsureMountPath(const std::string& mountPath)
{
    struct stat statbuf;
    if (lstat(mountPath.c_str(), &statbuf) == 0) {
        LOGI("IVolumeOperator::EnsureMountPath path exists, removing");
        if (remove(mountPath.c_str()) != 0) {
            LOGE("IVolumeOperator::EnsureMountPath remove failed, errno=%{public}d", errno);
            return E_SYS_KERNEL_ERR;
        }
    }
    mode_t originalUmask = umask(0);
    int err = mkdir(mountPath.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    umask(originalUmask);
    if (err != 0) {
        LOGE("IVolumeOperator::EnsureMountPath mkdir failed, errno=%{public}d", errno);
        return E_MKDIR_MOUNT;
    }
    return E_OK;
}

int32_t IVolumeOperator::RemoveMountPath(const std::string& mountPath)
{
    int err = rmdir(mountPath.c_str());
    if (err != 0 && errno != ENOENT) {
        LOGE("IVolumeOperator::RemoveMountPath rmdir failed, errno=%{public}d", errno);
        return E_ERR;
    }
    return E_OK;
}

int32_t IVolumeOperator::ReadMetadata(const std::string& devPath,
                                      std::string& uuid,
                                      std::string& type,
                                      std::string& label)
{
    LOGI("IVolumeOperator::ReadMetadata devPath=%{public}s", devPath.c_str());

    if (devPath.empty() || devPath.length() >= PATH_MAX) {
        LOGE("IVolumeOperator::ReadMetadata invalid devPath");
        return E_PARAMS_INVALID;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(devPath.c_str(), realPath) == nullptr) {
        LOGE("IVolumeOperator::ReadMetadata realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    if (std::string(realPath).find("/dev/block/") != 0) {
        LOGE("IVolumeOperator::ReadMetadata invalid devPath prefix");
        return E_PARAMS_INVALID;
    }

    uuid = GetBlkidData(realPath, "UUID");
    type = GetBlkidData(realPath, "TYPE");
    if (type.empty()) {
        LOGE("IVolumeOperator::ReadMetadata failed to get type");
        return E_READMETADATA;
    }
    label = GetBlkidData(realPath, "LABEL");

    if (!IsAcceptableUuid(uuid)) {
        LOGE("IVolumeOperator::ReadMetadata invalid UUID");
        return E_READMETADATA;
    }

    LOGI("IVolumeOperator::ReadMetadata success - uuid=%{public}s, type=%{public}s, label=%{public}s",
         GetAnonyString(uuid).c_str(), type.c_str(), GetAnonyString(label).c_str());
    return E_OK;
}

int32_t IVolumeOperator::Mount(const std::string& devPath,
                               const std::string& mountPath,
                               unsigned long mountFlags)
{
    LOGI("IVolumeOperator::Mount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), mountPath.c_str());

    if (devPath.empty() || devPath.length() >= PATH_MAX) {
        LOGE("IVolumeOperator::Mount invalid devPath");
        return E_PARAMS_INVALID;
    }
    if (devPath.find("/dev/block/") != 0) {
        LOGE("IVolumeOperator::Mount invalid devPath prefix");
        return E_PARAMS_INVALID;
    }

    if (mountPath.empty() || mountPath.length() >= PATH_MAX) {
        LOGE("IVolumeOperator::Mount invalid mountPath, len=%{public}zu", mountPath.length());
        return E_PARAMS_INVALID;
    }
    if (IsFilePathInvalid(mountPath)) {
        LOGE("IVolumeOperator::Mount mountPath contains invalid path segments");
        return E_PARAMS_INVALID;
    }
    if (mountPath.find(MOUNT_PATH_PREFIX) != 0) {
        LOGE("IVolumeOperator::Mount invalid mountPath prefix");
        return E_PARAMS_INVALID;
    }

    int32_t ret = EnsureMountPath(mountPath);
    if (ret != E_OK) {
        LOGE("IVolumeOperator::Mount EnsureMountPath failed, ret=%{public}d", ret);
        return ret;
    }
    ret = DoMount(devPath, mountPath, mountFlags);
    if (ret != E_OK) {
        LOGE("IVolumeOperator::Mount DoMount failed, ret=%{public}d", ret);
        RemoveMountPath(mountPath);
        return ret;
    }

    LOGI("IVolumeOperator::Mount success");
    return E_OK;
}

int32_t IVolumeOperator::Unmount(const std::string& mountPath, const std::string& fsType, bool force)
{
    LOGI("IVolumeOperator::Unmount mountPath=%{public}s, fsType=%{public}s, force=%{public}d",
         mountPath.c_str(), fsType.c_str(), force);

    if (mountPath.empty() || mountPath.length() >= PATH_MAX) {
        LOGE("IVolumeOperator::Unmount invalid path, len=%{public}zu", mountPath.length());
        return E_PARAMS_INVALID;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(mountPath.c_str(), realPath) == nullptr) {
        LOGE("IVolumeOperator::Unmount realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    std::string resolvedPath(realPath);
    if (resolvedPath.find(MOUNT_PATH_PREFIX) != 0) {
        LOGE("IVolumeOperator::Unmount invalid mountPath prefix");
        return E_PARAMS_INVALID;
    }

    int flags = force ? MNT_DETACH : 0;
    int ret = umount2(realPath, flags);
    if (ret != 0) {
        LOGE("IVolumeOperator::Unmount failed, errno=%{public}d", errno);
        return E_VOL_UMOUNT_ERR;
    }

    RemoveMountPath(realPath);
    LOGI("IVolumeOperator::Unmount success");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
