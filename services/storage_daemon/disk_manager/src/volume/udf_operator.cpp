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

#include "disk_manager/volume/udf_operator.h"
#include "storage_service_log.h"
#include "disk_manager/disk/disk_utils.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"

#include <cerrno>
#include <sys/mount.h>
#include <vector>

namespace OHOS {
namespace StorageDaemon {
constexpr int UID_FILE_MANAGER = 1006;
constexpr const char* MNT_EXTERNAL_FILE_CONTEXT = "context=u:object_r:mnt_external_file:s0";

int32_t UdfOperator::DoMount(const std::string& devPath,
                             const std::string& mountPath,
                             unsigned long mountFlags,
                             const std::string& mountData)
{
    LOGI("UdfOperator::DoMount devPath=%{public}s, mountPath=%{public}s",
         devPath.c_str(), mountPath.c_str());

    int32_t cdStatus = 0;
    int32_t ret = DiskUtils::QueryCDStatus(devPath, cdStatus);
    if (ret == E_OK && (cdStatus & 0x01) != 0 && (cdStatus & 0x02) != 0) {
        LOGI("UdfOperator::DoMount empty disc, skip mount");
        return E_OK;
    }

    std::string options = "rw,uid=1006,gid=1006,dmask=0006,fmask=0007";

    auto mountUdfData = StringPrintf("ro,uid=%d,gid=%d,%s,mode=0770,dmode=0751",
        UID_FILE_MANAGER, UID_FILE_MANAGER, MNT_EXTERNAL_FILE_CONTEXT);
    std::vector<std::string> cmd = {
        "mount",
        "-t",
        "udf",
        "-o",
        mountUdfData,
        devPath,
        mountPath,
    };

    std::vector<std::string> output;
    ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("UdfOperator::DoMount output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("UdfOperator::DoMount failed, ret=%{public}d, errno=%{public}d", ret, errno);
        return E_UDF_MOUNT;
    }

    LOGI("UdfOperator::DoMount success");
    return E_OK;
}

int32_t UdfOperator::ReadMetadata(const std::string& devPath,
                                  std::string& uuid,
                                  std::string& type,
                                  std::string& label)
{
    LOGI("UdfOperator::ReadMetadata devPath=%{public}s", devPath.c_str());

    if (devPath.empty() || devPath.length() >= PATH_MAX) {
        LOGE("UdfOperator::ReadMetadata invalid devPath");
        return E_PARAMS_INVALID;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(devPath.c_str(), realPath) == nullptr) {
        LOGE("UdfOperator::ReadMetadata realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    if (std::string(realPath).find("/dev/block/") != 0) {
        LOGE("UdfOperator::ReadMetadata invalid devPath prefix");
        return E_PARAMS_INVALID;
    }

    uuid = GetBlkidData(realPath, "UUID");
    std::string offset = devPath + "/" + type;
    if (uuid.empty()) {
        uuid = GenerateRandomUuid(realPath, offset);
    }

    label = GetBlkidData(realPath, "LABEL");
    if (label.empty()) {
        label = GetCDType(realPath);
    }

    LOGI("UdfOperator::ReadMetadata success - uuid=%{public}s, type=%{public}s, label=%{public}s",
         GetAnonyString(uuid).c_str(), type.c_str(), GetAnonyString(label).c_str());
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS