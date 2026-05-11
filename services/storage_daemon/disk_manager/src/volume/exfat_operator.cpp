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

#include "disk_manager/volume/exfat_operator.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

#include <cerrno>
#include <sys/mount.h>
#include <vector>

namespace OHOS {
namespace StorageDaemon {
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;

int32_t ExfatOperator::DoMount(const std::string& devPath,
                               const std::string& mountPath,
                               unsigned long mountFlags)
{
    LOGI("ExfatOperator::DoMount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), mountPath.c_str());

    std::string options;
    if (mountFlags & MS_RDONLY) {
        options = StringPrintf("ro,uid=%d,gid=%d,dmask=0006,fmask=0007", FILE_MANAGER_UID, FILE_MANAGER_GID);
    } else {
        options = StringPrintf("rw,uid=%d,gid=%d,dmask=0006,fmask=0007", FILE_MANAGER_UID, FILE_MANAGER_GID);
    }

    std::vector<std::string> cmd = {
        "mount.exfat",
        "-o",
        options,
        devPath,
        mountPath
    };

    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("ExfatOperator::DoMount output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("ExfatOperator::DoMount failed, ret=%{public}d, errno=%{public}d", ret, errno);
        return E_EXFAT_MOUNT;
    }

    LOGI("ExfatOperator::DoMount success");
    return E_OK;
}

int32_t ExfatOperator::Format(const std::string& devPath)
{
    LOGI("ExfatOperator::Format devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "mkfs.exfat",
        devPath
    };

    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("ExfatOperator::Format output: %{public}s", str.c_str());
    }

    if (err == E_NO_CHILD) {
        err = E_OK;
    }

    if (err == E_OK) {
        LOGI("ExfatOperator::Format success");
    } else {
        LOGE("ExfatOperator::Format failed, err=%{public}d", err);
    }

    return err;
}

int32_t ExfatOperator::Check(const std::string& devPath)
{
    LOGI("ExfatOperator::Check devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "fsck.exfat",
        "-n",
        devPath
    };

    int exitStatus = 0;
    int execRet = ForkExecWithExit(cmd, &exitStatus);
    LOGI("ExfatOperator::Check execRet=%{public}d, exitStatus=%{public}d", execRet, exitStatus);

    if (execRet != E_OK || exitStatus != 0) {
        LOGE("ExfatOperator::Check need fix, execRet=%{public}d, exitStatus=%{public}d", execRet, exitStatus);
        return E_VOL_NEED_FIX;
    }

    LOGI("ExfatOperator::Check success");
    return E_OK;
}

int32_t ExfatOperator::Repair(const std::string& devPath)
{
    LOGI("ExfatOperator::Repair devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "fsck.exfat",
        "-p",
        devPath
    };

    int exitStatus = 0;
    int forkExecRes = ForkExecWithExit(cmd, &exitStatus);
    LOGI("ExfatOperator::Repair forkExecRes=%{public}d, exitStatus=%{public}d",
         forkExecRes, exitStatus);

    if (exitStatus != 1 || forkExecRes != E_OK) {
        LOGE("ExfatOperator::Repair failed, exitStatus=%{public}d, res=%{public}d",
             exitStatus, forkExecRes);
        return E_VOL_FIX_FAILED;
    }

    LOGI("ExfatOperator::Repair success");
    return E_OK;
}

int32_t ExfatOperator::SetLabel(const std::string& devPath,
                                const std::string& label)
{
    LOGI("ExfatOperator::SetLabel devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "exfatlabel",
        devPath,
        label
    };
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("ExfatOperator::SetLabel output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("ExfatOperator::SetLabel failed, ret=%{public}d", ret);
        return ret;
    }

    LOGI("ExfatOperator::SetLabel success");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
