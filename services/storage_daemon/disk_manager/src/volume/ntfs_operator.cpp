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

#include "disk_manager/volume/ntfs_operator.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "utils/disk_utils.h"

#include <cerrno>
#include <climits>
#include <sys/mount.h>
#include <vector>

namespace OHOS {
namespace StorageDaemon {
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;

constexpr const char *NTFS_LABEL_PREFIX = "Volume label :  ";

static std::string GetNtfsLabelFallback(const std::string& devPath)
{
    LOGI("NtfsOperator::GetNtfsLabelFallback devPath=%{public}s", devPath.c_str());
    std::vector<std::string> cmd = {"ntfslabel", "-v", devPath};
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("NtfsOperator::GetNtfsLabelFallback output: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGW("NtfsOperator::GetNtfsLabelFallback ForkExec failed, ret=%{public}d", ret);
        return "";
    }

    // Split output by '\n' into individual lines, same as old SplitOutputIntoLines
    std::vector<std::string> lines;
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        lines.insert(lines.end(), split.begin(), split.end());
    }
    if (lines.empty()) {
        LOGW("NtfsOperator::GetNtfsLabelFallback no lines in output");
        return "";
    }

    // Iterate backwards to find last occurrence of label prefix, same as old logic
    std::string volDesc;
    for (size_t i = lines.size() - 1; i < lines.size(); --i) {
        std::string &line = lines[i];
        std::string::size_type index = line.find(NTFS_LABEL_PREFIX);
        if (index == std::string::npos) {
            continue;
        }
        volDesc = line.substr(index + strlen(NTFS_LABEL_PREFIX));
        break;
    }
    LOGI("NtfsOperator::GetNtfsLabelFallback label=%{public}s", volDesc.c_str());
    return volDesc;
}

int32_t NtfsOperator::ReadMetadata(const std::string& devPath,
                                   std::string& uuid,
                                   std::string& type,
                                   std::string& label)
{
    int32_t ret = IVolumeOperator::ReadMetadata(devPath, uuid, type, label);
    if (ret != E_OK) {
        return ret;
    }

    if (label.find('?') != std::string::npos || label.empty()) {
        LOGI("NtfsOperator::ReadMetadata using ntfslabel fallback");
        char realPath[PATH_MAX] = {0};
        if (realpath(devPath.c_str(), realPath) != nullptr &&
            std::string(realPath).find("/dev/block/") == 0) {
            label = GetNtfsLabelFallback(realPath);
        }
    }

    return E_OK;
}

int32_t NtfsOperator::DoMount(const std::string& devPath,
                              const std::string& mountPath,
                              unsigned long mountFlags,
                              const std::string& mountData)
{
    LOGI("NtfsOperator::DoMount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), GetAnonyString(mountPath).c_str());

    std::string options;
    if (!mountData.empty()) {
        options = mountData;
    } else if (mountFlags & MS_RDONLY) {
        options = StringPrintf("ro,uid=%d,gid=%d,dmask=0006,fmask=0007", FILE_MANAGER_UID, FILE_MANAGER_GID);
    } else {
#ifdef EXTERNAL_STORAGE_QOS_TRANS
        options = StringPrintf("rw,big_writes,uid=%d,gid=%d,dmask=0006,fmask=0007",
            FILE_MANAGER_UID, FILE_MANAGER_GID);
#else
        options = StringPrintf("rw,uid=%d,gid=%d,dmask=0006,fmask=0007", FILE_MANAGER_UID, FILE_MANAGER_GID);
#endif
    }

#ifdef CDC_STORAGE
    options += ",nodev,noexec";
#endif
    std::vector<std::string> cmd = {
        "mount.ntfs",
        devPath,
        mountPath,
        "-o",
        options
    };

#ifdef EXTERNAL_STORAGE_QOS_TRANS
    int32_t ret = ExtStorageMountForkExec(cmd);
    if (ret != E_OK) {
        LOGE("NtfsOperator::DoMount failed, ret=%{public}d, errno=%{public}d", ret, errno);
        return E_NTFS_MOUNT;
    }
#else
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);
    for (auto& str : output) {
        LOGI("NtfsOperator::DoMount output: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("NtfsOperator::DoMount failed, ret=%{public}d, errno=%{public}d", ret, errno);
        return E_NTFS_MOUNT;
    }
#endif

    LOGI("NtfsOperator::DoMount success");
    return E_OK;
}

int32_t NtfsOperator::Check(const std::string& devPath)
{
    LOGI("NtfsOperator::Check devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "fsck.ntfs",
        devPath
    };

    int exitStatus = 0;
    int execRet = ForkExecWithExit(cmd, &exitStatus);
    LOGI("NtfsOperator::Check execRet=%{public}d, exitStatus=%{public}d", execRet, exitStatus);

    if (exitStatus != 1) {
        LOGE("NtfsOperator::Check need fix, exitStatus=%{public}d", exitStatus);
        return E_VOL_NEED_FIX;
    }

    LOGI("NtfsOperator::Check success");
    return E_OK;
}

int32_t NtfsOperator::Repair(const std::string& devPath)
{
    LOGI("NtfsOperator::Repair devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "ntfsfix",
        "-d",
        devPath
    };

    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("NtfsOperator::Repair output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("NtfsOperator::Repair failed, ret=%{public}d", ret);
        return E_VOL_FIX_FAILED;
    }

    LOGI("NtfsOperator::Repair success");
    return E_OK;
}

int32_t NtfsOperator::SetLabel(const std::string& devPath,
                               const std::string& label)
{
    LOGI("NtfsOperator::SetLabel devPath=%{public}s", devPath.c_str());

    std::vector<std::string> fixCmd = {
        "ntfsfix",
        "-d",
        devPath
    };
    std::vector<std::string> fixOutput;
    int32_t fixRet = ForkExec(fixCmd, &fixOutput);
    for (auto& str : fixOutput) {
        LOGI("NtfsOperator::SetLabel ntfsfix: %{public}s", str.c_str());
    }
    if (fixRet != E_OK) {
        LOGW("NtfsOperator::SetLabel ntfsfix failed, ret=%{public}d, continue to set label", fixRet);
    }

    std::vector<std::string> labelCmd = {
        "ntfslabel",
        devPath,
        label
    };
    std::vector<std::string> output;
    int32_t ret = ForkExec(labelCmd, &output);

    for (auto& str : output) {
        LOGI("NtfsOperator::SetLabel output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("NtfsOperator::SetLabel failed, ret=%{public}d", ret);
        return ret;
    }

    LOGI("NtfsOperator::SetLabel success");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
