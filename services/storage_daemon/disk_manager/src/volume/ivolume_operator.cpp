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

#include <chrono>
#include <climits>
#include <csignal>
#include <fcntl.h>
#include <future>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/volume_op_diag.h"
#include "volume/process.h"

#define STORAGE_MANAGER_IOC_CHK_BUSY _IOR(0xAC, 77, int)

namespace OHOS {
namespace StorageDaemon {

constexpr const char *MOUNT_PATH_PREFIX = "/mnt/data/";
constexpr int32_t WAIT_MOUNT_TIMEOUT_S = 60;

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
    mode_t mode = S_IRWXU | S_IRWXG | S_IXOTH;
    if (mountPath == "/mnt/data/voldata" || mountPath.find("/mnt/data/voldata/") == 0) {
        mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    }
    mode_t originalUmask = umask(0);
    int err = mkdir(mountPath.c_str(), mode);
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
    if (std::string(realPath).find("/dev/block/") != 0 && std::string(realPath).find("/dev/mapper/") != 0) {
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

namespace {
struct MountThreadResult {
    int32_t ret = 0;
    std::vector<VolumeOpDiagToolEntry> toolEntries;
};
} // namespace

int32_t IVolumeOperator::ValidateMountRequest(const std::string& devPath, const std::string& mountPath,
                                              const std::string& mountData)
{
    if (devPath.empty() || devPath.length() >= PATH_MAX) {
        LOGE("IVolumeOperator::Mount invalid devPath");
        return E_PARAMS_INVALID;
    }
    if (devPath.find("/dev/block/") != 0 && devPath.find("/dev/mapper/") != 0) {
        LOGE("IVolumeOperator::Mount invalid devPath prefix");
        return E_PARAMS_INVALID;
    }
    if (mountPath.empty() || mountPath.length() >= PATH_MAX) {
        LOGE("IVolumeOperator::Mount invalid mountPath, len=%{public}zu", mountPath.length());
        return E_PARAMS_INVALID;
    }
    if (IsFilePathInvalid(mountPath) || IsMountDataInvalid(mountData)) {
        LOGE("IVolumeOperator::Mount mountPath or mountData contains invalid content");
        return E_PARAMS_INVALID;
    }
    if (mountPath.find(MOUNT_PATH_PREFIX) != 0) {
        LOGE("IVolumeOperator::Mount invalid mountPath prefix");
        return E_PARAMS_INVALID;
    }
    return E_OK;
}

int32_t IVolumeOperator::Mount(const std::string& devPath,
                               const std::string& mountPath,
                               unsigned long mountFlags,
                               const std::string& mountData)
{
    LOGI("IVolumeOperator::Mount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), GetAnonyString(mountPath).c_str());

    int32_t ret = ValidateMountRequest(devPath, mountPath, mountData);
    if (ret != E_OK) {
        return ret;
    }

    ret = EnsureMountPath(mountPath);
    if (ret != E_OK) {
        LOGE("IVolumeOperator::Mount EnsureMountPath failed, ret=%{public}d", ret);
        return ret;
    }

    std::promise<MountThreadResult> promise;
    std::future<MountThreadResult> future = promise.get_future();
    const VolumeOpDiagContext diagCtx = VolumeOpDiagCaptureContext();
    std::thread mountThread([this, devPath, mountPath, mountFlags, mountData, diagCtx,
                             p = std::move(promise)]() mutable {
        VolumeOpDiagAttachContext(diagCtx);
        MountThreadResult result;
        result.ret = DoMount(devPath, mountPath, mountFlags, mountData);
        result.toolEntries = VolumeOpDiagTakeToolEntries();
        p.set_value(std::move(result));
    });

    if (future.wait_for(std::chrono::seconds(WAIT_MOUNT_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("IVolumeOperator::Mount timed out, devPath=%{public}s", devPath.c_str());
        mountThread.detach();
        RemoveMountPath(mountPath);
        return E_TIMEOUT_MOUNT;
    }

    MountThreadResult mountResult = future.get();
    mountThread.join();
    VolumeOpDiagMergeToolEntries(mountResult.toolEntries);
    ret = mountResult.ret;
    if (ret != E_OK) {
        LOGE("IVolumeOperator::Mount DoMount failed, ret=%{public}d", ret);
        RemoveMountPath(mountPath);
        return ret;
    }

    LOGI("IVolumeOperator::Mount success");
    return E_OK;
}

static int32_t IsUsbInUse(int fd)
{
    int32_t inUse = -1;
    if (ioctl(fd, STORAGE_MANAGER_IOC_CHK_BUSY, &inUse) < 0) {
        LOGE("IsUsbInUse: ioctl failed, errno=%{public}d", errno);
        return E_IOCTL_FAILED;
    }
    if (inUse) {
        LOGI("IsUsbInUse: inUse=%{public}d", inUse);
        return E_USB_IN_USE;
    }
    return E_OK;
}

int32_t IVolumeOperator::Unmount(const std::string& mountPath, const std::string& fsType, bool force)
{
    LOGI("IVolumeOperator::Unmount mountPath=%{public}s, fsType=%{public}s, force=%{public}d",
         GetAnonyString(mountPath).c_str(), fsType.c_str(), force);

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

    if (force) {
        Process ps(resolvedPath);
        ps.UpdatePidAndKill(SIGKILL);
        int ret = umount2(resolvedPath.c_str(), MNT_DETACH);
        if (ret != 0) {
            LOGW("IVolumeOperator::Unmount umount2 failed in force mode, errno=%{public}d", errno);
            RemoveMountPath(resolvedPath);
            return E_OK;
        }
        RemoveMountPath(resolvedPath);
        LOGI("IVolumeOperator::Unmount force success");
        return E_OK;
    }

    int fd = open(resolvedPath.c_str(), O_RDONLY);
    if (fd >= 0) {
        IsUsbInUse(fd);
    }
    int ret = umount2(resolvedPath.c_str(), MNT_DETACH);
    if (fd >= 0) {
        IsUsbInUse(fd);
        close(fd);
    }
    if (ret != 0) {
        LOGE("IVolumeOperator::Unmount failed, errno=%{public}d", errno);
        return E_VOL_UMOUNT_ERR;
    }

    RemoveMountPath(resolvedPath);
    LOGI("IVolumeOperator::Unmount success");
    return E_OK;
}

int32_t IVolumeOperator::CreateIsoImage(const std::string& devPath,
                                        const std::string& filePath,
                                        const std::string& mountPath)
{
    LOGI("IVolumeOperator::CreateIsoImage devPath=%{public}s, filePath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), GetAnonyString(filePath).c_str(), GetAnonyString(mountPath).c_str());
    return E_NOT_SUPPORT;
}

int32_t IVolumeOperator::Burn(const std::string &devPath, const BurnOptions &burnOptions)
{
    LOGI("IVolumeOperator::Burn devPath=%{public}s", devPath.c_str());
    return E_NOT_SUPPORT;
}

bool IVolumeOperator::IsMountDataInvalid(const std::string& mountData)
{
    if (mountData.empty()) {
        return false;
    }
    if (mountData.find("uid=0") != std::string::npos ||
        mountData.find("gid=0") != std::string::npos ||
        mountData.find("suid") != std::string::npos) {
        LOGE("IsMountDataInvalid: mountData contains dangerous options");
        return true;
    }
    return false;
}

bool IVolumeOperator::IsShellMetacharPresent(const std::string& str)
{
    static const std::string shellChars = "\"$`\\;|&!(){}<>\n";
    return str.find_first_of(shellChars) != std::string::npos;
}
} // namespace StorageDaemon
} // namespace OHOS
