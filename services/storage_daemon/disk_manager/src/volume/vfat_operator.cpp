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

#include <cerrno>
#include <sys/mount.h>
#include <unistd.h>

namespace OHOS {
namespace StorageDaemon {
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;

int32_t VfatOperator::DoMount(const std::string& devPath,
                              const std::string& mountPath,
                              unsigned long mountFlags)
{
    LOGI("VfatOperator::DoMount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), mountPath.c_str());

    unsigned long flags = mountFlags | MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0006,fmask=0007,utf8", FILE_MANAGER_UID, FILE_MANAGER_GID);

    int32_t ret = mount(devPath.c_str(), mountPath.c_str(), "vfat", flags, mountData.c_str());
    if (ret != E_OK) {
        LOGE("VfatOperator::DoMount failed, errno=%{public}d", errno);
        return E_FAT_MOUNT;
    }

    LOGI("VfatOperator::DoMount success");
    return E_OK;
}

int32_t VfatOperator::Format(const std::string& devPath)
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

    if (err == E_OK) {
        LOGI("VfatOperator::Format success");
    } else {
        LOGE("VfatOperator::Format failed, err=%{public}d", err);
    }

    return err;
}

} // namespace StorageDaemon
} // namespace OHOS
