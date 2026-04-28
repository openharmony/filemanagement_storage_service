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

#include <climits>
#include <cstdio>
#include <libgen.h>

#include "disk_manager/volume/volume_utils.h"

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include "storage_service_errno.h"
#include "securec.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

constexpr const char *MOUNT_PATH_PREFIX = "/mnt/data/external/";

static bool IsValidMountPath(const std::string& mountPath)
{
    char realPath[PATH_MAX] = {0};
    if (realpath(mountPath.c_str(), realPath) == nullptr) {
        char pathBuf[PATH_MAX] = {0};
        if (strncpy_s(pathBuf, PATH_MAX, mountPath.c_str(), PATH_MAX - 1) != EOK) {
            LOGE("IsValidMountPath: strncpy_s failed");
            return false;
        }
        std::string dirPath = dirname(pathBuf);
        if (realpath(dirPath.c_str(), realPath) == nullptr) {
            LOGE("IsValidMountPath parent realpath failed, errno=%{public}d", errno);
            return false;
        }
        std::string resolvedParent(realPath);
        if (resolvedParent != "/mnt/data/external" && resolvedParent.find(MOUNT_PATH_PREFIX) != 0) {
            LOGE("IsValidMountPath invalid parent path prefix");
            return false;
        }
    } else {
        if (std::string(realPath).find(MOUNT_PATH_PREFIX) != 0) {
            LOGE("IsValidMountPath invalid mountPath prefix");
            return false;
        }
    }
    return true;
}

static std::string ExtractLabelFromLine(const std::string& line, const std::string& prefix)
{
    size_t pos = line.find(prefix);
    if (pos == std::string::npos) {
        return "";
    }
    size_t colonPos = line.find(':', pos + prefix.length());
    if (colonPos == std::string::npos) {
        return "";
    }
    std::string label = line.substr(colonPos + 1);
    size_t start = label.find_first_not_of(" \t");
    size_t end = label.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos) {
        label = label.substr(start, end - start + 1);
    }
    return label;
}

static std::string GetNtfsLabelFallback(const std::string& devPath)
{
    LOGI("VolumeUtils::GetNtfsLabelFallback devPath=%{public}s", devPath.c_str());
    std::vector<std::string> cmd = {"ntfslabel", "-v", devPath};
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);
    if (ret != E_OK) {
        LOGW("VolumeUtils::GetNtfsLabelFallback ForkExec failed, ret=%{public}d", ret);
        return "";
    }

    const std::string prefix = "Volume label";
    for (const auto& line : output) {
        std::string label = ExtractLabelFromLine(line, prefix);
        if (!label.empty()) {
            LOGI("VolumeUtils::GetNtfsLabelFallback label=%{public}s", label.c_str());
            return label;
        }
    }
    LOGW("VolumeUtils::GetNtfsLabelFallback no label found in output");
    return "";
}

int32_t VolumeUtils::ReadMetadata(const std::string& devPath,
                                  std::string& uuid,
                                  std::string& type,
                                  std::string& label)
{
    LOGI("VolumeUtils::ReadMetadata devPath=%{public}s", devPath.c_str());

    if (devPath.empty() || devPath.length() >= PATH_MAX) {
        LOGE("VolumeUtils::ReadMetadata invalid devPath");
        return E_PARAMS_INVALID;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(devPath.c_str(), realPath) == nullptr) {
        LOGE("VolumeUtils::ReadMetadata realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    if (std::string(realPath).find("/dev/block/") != 0) {
        LOGE("VolumeUtils::ReadMetadata invalid devPath prefix");
        return E_PARAMS_INVALID;
    }
    uuid = GetBlkidData(realPath, "UUID");
    type = GetBlkidData(realPath, "TYPE");
    if (type.empty()) {
        LOGE("VolumeUtils::ReadMetadata failed to get type");
        return E_READMETADATA;
    }
    label = GetBlkidData(realPath, "LABEL");
    if (type == "ntfs" && (label.find('?') != std::string::npos || label.empty())) {
        LOGI("VolumeUtils::ReadMetadata using ntfslabel fallback for NTFS");
        label = GetNtfsLabelFallback(realPath);
    }
    if (!IsAcceptableUuid(uuid)) {
        LOGE("VolumeUtils::ReadMetadata invalid UUID");
        return E_READMETADATA;
    }

    LOGI("VolumeUtils::ReadMetadata success - uuid=%{public}s, type=%{public}s, label=%{public}s",
         GetAnonyString(uuid).c_str(), type.c_str(), GetAnonyString(label).c_str());
    return E_OK;
}

static int32_t EnsureFuseMountPath(const char* path)
{
    struct stat statbuf;
    if (lstat(path, &statbuf) == 0) {
        if (!S_ISDIR(statbuf.st_mode)) {
            if (remove(path) != 0) {
                LOGE("MountFuseDevice remove failed, errno=%{public}d", errno);
                return E_SYS_KERNEL_ERR;
            }
        }
    }
    mode_t originalUmask = umask(0);
    int mkdirErr = mkdir(path, S_IRWXU | S_IRWXG | S_IXOTH);
    umask(originalUmask);
    if (mkdirErr != 0 && errno != EEXIST) {
        LOGE("MountFuseDevice mkdir failed, errno=%{public}d", errno);
        return E_MKDIR_MOUNT;
    }
    return E_OK;
}

int32_t VolumeUtils::MountFuseDevice(const std::string& mountPath,
                                     int& fuseFd)
{
    LOGI("VolumeUtils::MountFuseDevice mountPath=%{public}s", mountPath.c_str());

    if (mountPath.empty() || mountPath.length() >= PATH_MAX) {
        LOGE("VolumeUtils::MountFuseDevice invalid mountPath, len=%{public}zu", mountPath.length());
        return E_PARAMS_INVALID;
    }
    if (!IsValidMountPath(mountPath)) {
        LOGE("VolumeUtils::MountFuseDevice invalid mountPath");
        return E_PARAMS_INVALID;
    }

    int32_t ret = EnsureFuseMountPath(mountPath.c_str());
    if (ret != E_OK) {
        return ret;
    }

    fuseFd = open("/dev/fuse", O_RDWR);
    if (fuseFd < 0) {
        LOGE("VolumeUtils::MountFuseDevice open /dev/fuse failed, errno=%{public}d", errno);
        return E_OPEN_FAILED;
    }
    std::string fuseOptions = StringPrintf(
        "fd=%d,"
        "rootmode=40000,"
        "default_permissions,"
        "allow_other,"
        "user_id=0,group_id=0,"
        "context=u:object_r:mnt_external_file:s0",
        fuseFd);
    int mret = mount("/dev/fuse", mountPath.c_str(), "fuse",
                     MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME,
                     fuseOptions.c_str());
    if (mret != 0) {
        LOGE("VolumeUtils::MountFuseDevice mount failed, errno=%{public}d", errno);
        close(fuseFd);
        fuseFd = -1;
        return E_EXT_MOUNT;
    }

    LOGI("VolumeUtils::MountFuseDevice success, fuseFd=%{public}d", fuseFd);
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
