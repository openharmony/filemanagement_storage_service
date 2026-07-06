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

#include "disk_manager/volume/hmfs_operator.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

#include <cerrno>
#include <sys/mount.h>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

namespace OHOS {
namespace StorageDaemon {

constexpr const char* HMFS_MOUNT_CONTEXT = "context=u:object_r:mnt_external_file:s0,noacl";
constexpr unsigned long MOUNT_FLAG_MIGRATION_RO = 0x8000;
constexpr uid_t ROOT_UID = 0;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr mode_t MOUNT_DIR_MODE = 0775;

int32_t HmfsOperator::DoMount(const std::string& devPath,
    const std::string& mountPath,
    unsigned long mountFlags,
    const std::string& mountData)
{
    LOGI("HmfsOperator::DoMount devPath=%{public}s, mountPath=%{public}s, mountFlags=%{public}lu",
         devPath.c_str(), GetAnonyString(mountPath).c_str(), mountFlags);

    if (devPath.empty() || mountPath.empty()) {
        LOGE("HmfsOperator::DoMount invalid parameters");
        return E_PARAMS_INVALID;
    }

    int32_t ret;
    if (mountFlags == MOUNT_FLAG_MIGRATION_RO) {
        ret = mount(devPath.c_str(), mountPath.c_str(), "hmfs", MS_RDONLY, HMFS_MOUNT_CONTEXT);
        if (ret != E_OK) {
            LOGE("HmfsOperator::DoMount mount RO failed, errno=%{public}d", errno);
            return E_HMFS_MOUNT;
        }
    } else {
        ret = mount(devPath.c_str(), mountPath.c_str(), "hmfs", mountFlags, mountData.c_str());
        if (ret != E_OK) {
            LOGE("HmfsOperator::DoMount mount failed, errno=%{public}d", errno);
            return E_HMFS_MOUNT;
        }
    }

    if (mountFlags != MOUNT_FLAG_MIGRATION_RO) {
        if (chmod(mountPath.c_str(), S_ISGID | MOUNT_DIR_MODE) != 0) {
            LOGE("HmfsOperator::DoMount chmod failed on %{public}s, errno=%{public}d",
                 mountPath.c_str(), errno);
        }
        if (chown(mountPath.c_str(), ROOT_UID, FILE_MANAGER_GID) != 0) {
            LOGE("HmfsOperator::DoMount chown failed on %{public}s, errno=%{public}d",
                 mountPath.c_str(), errno);
        }
    }

    LOGI("HmfsOperator::DoMount success");
    return E_OK;
}

int32_t HmfsOperator::Format(const std::string& devPath)
{
    LOGI("HmfsOperator::Format devPath=%{public}s", devPath.c_str());

    if (devPath.empty()) {
        LOGE("HmfsOperator::Format invalid devPath");
        return E_PARAMS_INVALID;
    }

    std::vector<std::string> cmd = {
        "mkfs.f2fs",
        "-d1",
        "-O", "encrypt",
        "-O", "verity",
        "-O", "sb_checksum",
        devPath
    };

    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("HmfsOperator::Format output: %{public}s", str.c_str());
    }

    if (ret == E_NO_CHILD) {
        ret = E_OK;
    }

    if (ret == E_OK) {
        LOGI("HmfsOperator::Format success");
    } else {
        LOGE("HmfsOperator::Format failed, ret=%{public}d", ret);
    }

    return ret;
}

int32_t HmfsOperator::Check(const std::string& devPath)
{
    LOGI("HmfsOperator::Check devPath=%{public}s", devPath.c_str());

    if (devPath.empty()) {
        LOGE("HmfsOperator::Check invalid devPath");
        return E_PARAMS_INVALID;
    }

    std::vector<std::string> cmd = {
        "fsck.f2fs",
        "--dry-run",
        "-p1",
        devPath
    };

    int32_t execRet = ForkExecWithExit(cmd);
    if (execRet != E_OK) {
        LOGE("HmfsOperator::Check need fix, execRet=%{public}d", execRet);
        return E_VOL_NEED_FIX;
    }

    LOGI("HmfsOperator::Check success");
    return E_OK;
}

int32_t HmfsOperator::Repair(const std::string& devPath)
{
    LOGI("HmfsOperator::Repair devPath=%{public}s", devPath.c_str());

    if (devPath.empty()) {
        LOGE("HmfsOperator::Repair invalid devPath");
        return E_PARAMS_INVALID;
    }

    std::vector<std::string> cmd = {
        "fsck.f2fs",
        "-p1",
        devPath
    };

    int exitStatus = 0;
    int32_t forkExecRes = ForkExecWithExit(cmd, &exitStatus);
    LOGI("HmfsOperator::Repair forkExecRes=%{public}d, exitStatus=%{public}d",
         forkExecRes, exitStatus);

    if (exitStatus != 1 || forkExecRes != E_OK) {
        LOGE("HmfsOperator::Repair failed, exitStatus=%{public}d, res=%{public}d",
             exitStatus, forkExecRes);
        return E_VOL_FIX_FAILED;
    }

    LOGI("HmfsOperator::Repair success");
    return E_OK;
}

int32_t HmfsOperator::SetLabel(const std::string& devPath,
    const std::string& label)
{
    LOGI("HmfsOperator::SetLabel devPath=%{public}s, label=%{public}s",
         devPath.c_str(), label.c_str());

    if (devPath.empty()) {
        LOGE("HmfsOperator::SetLabel invalid parameters");
        return E_PARAMS_INVALID;
    }

    std::vector<std::string> cmd = {
        "f2fslabel",
        devPath,
        label
    };

    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("HmfsOperator::SetLabel output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("HmfsOperator::SetLabel failed, ret=%{public}d", ret);
        return ret;
    }

    LOGI("HmfsOperator::SetLabel success");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS