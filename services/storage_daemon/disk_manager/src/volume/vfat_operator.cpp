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

#include "disk_manager/volume/vfat_operator.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "utils/disk_utils.h"

#include <cerrno>
#include <sys/mount.h>
#include <unistd.h>

namespace OHOS {
namespace StorageDaemon {
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;

int32_t VfatOperator::DoMount(const std::string& devPath,
                              const std::string& mountPath,
                              unsigned long mountFlags,
                              const std::string& mountData)
{
    LOGI("VfatOperator::DoMount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), GetAnonyString(mountPath).c_str());

    unsigned long flags = mountFlags | MS_MGC_VAL;
    std::string data = mountData.empty() ?
        StringPrintf("uid=%d,gid=%d,dmask=0006,fmask=0007,utf8", FILE_MANAGER_UID, FILE_MANAGER_GID) :
        mountData;

    int32_t ret = mount(devPath.c_str(), mountPath.c_str(), "vfat", flags, data.c_str());
    if (ret != E_OK) {
        LOGE("VfatOperator::DoMount failed, errno=%{public}d", errno);
        return E_FAT_MOUNT;
    }

    LOGI("VfatOperator::DoMount success");
    return E_OK;
}

int32_t VfatOperator::Format(const std::string& devPath,
                             const std::string& diskPath,
                             const std::string& partitionType,
                             const int32_t partitionNum)
{
    LOGI("VfatOperator::Format devPath=%{public}s", devPath.c_str());

    std::vector<std::string> cmd = {
        "newfs_msdos",
        "-A",
        devPath
    };

    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("VfatOperator::Format output: %{public}s", str.c_str());
    }

    if (err == E_NO_CHILD) {
        err = E_OK;
    }

    if (err != E_OK) {
        LOGE("VfatOperator::Format failed, err=%{public}d", err);
        return err;
    }

    err = FixTypeIdentifier(diskPath, partitionType, partitionNum);
    if (err != E_OK) {
        LOGE("VfatOperator::Format FixTypeIdentifier failed, err=%{public}d", err);
        return err;
    }
    LOGI("VfatOperator::Format success");
    return E_OK;
}

int32_t VfatOperator::FixTypeIdentifier(const std::string& diskPath,
                                        const std::string& partitionType,
                                        const int32_t partitionNum)
{
    LOGI("VfatOperator::FixTypeIdentifier diskPath=%{public}s, partitionType=%{public}s, partitionNum=%{public}d",
         diskPath.c_str(), partitionType.c_str(), partitionNum);
    std::string typeIdentifier;
    std::vector<std::string> cmd;
    if (partitionType == "mbr") {
        typeIdentifier = std::to_string(partitionNum) + ":" + "0x0c";
        cmd = {"ohos_fixparts", "-t", typeIdentifier, diskPath};
    } else if (partitionType == "gpt") {
        typeIdentifier = std::to_string(partitionNum) + ":" + "0x0700";
        cmd = {"sgdisk", "-t", typeIdentifier, diskPath};
    } else {
        LOGE("VfatOperator::FixTypeIdentifier failed, unknown partitionType=%{public}s", partitionType.c_str());
        return E_NOT_SUPPORT;
    }
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("VfatOperator::FixTypeIdentifier output: %{public}s", GetAnonyString(str).c_str());
    }
    if (ret != E_OK) {
        LOGE("VfatOperator::FixTypeIdentifier failed, ret=%{public}d", ret);
        return ret;
    }
    LOGI("VfatOperator::FixTypeIdentifier success");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
