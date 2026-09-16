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

#include "disk_manager/volume/ext4_operator.h"
#include "parameters.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

#include <sys/mount.h>
#include <unistd.h>

namespace OHOS {
namespace StorageDaemon {
const std::string KEY_CUST = "const.cust.custPath";
const std::string CUST_TOBBASIC = "tobbasic";

int32_t Ext4Operator::DoMount(const std::string& devPath,
                              const std::string& mountPath,
                              unsigned long mountFlags,
                              const std::string& mountData)
{
#ifdef PC_EXT4_ENABLE
    std::string custParam = system::GetParameter(KEY_CUST, "");
    bool cond = (custParam.find(CUST_TOBBASIC) != std::string::npos);
    if (!cond) {
        LOGE("PC is not tobbasic, exit. custParam=%{public}s", custParam.c_str());
        return E_NOT_SUPPORT;
    }

    LOGI("Ext4Operator::Mount devPath=%{public}s, mountPath=%{public}s", devPath.c_str(),
         GetAnonyString(mountPath).c_str());

    if (devPath.empty() || mountPath.empty()) {
        LOGE("Ext4Operator::Mount invalid parameters");
        return E_PARAMS_INVALID;
    }

    std::string data = mountData.empty() ? "context=u:object_r:mnt_external_file:s0" : mountData;
    int32_t ret = mount(devPath.c_str(), mountPath.c_str(), "ext4", mountFlags, data.c_str());
    if (ret != E_OK) {
        LOGE("Ext4Operator::Mount failed, errno=%{public}d", errno);
        return E_EXT_MOUNT;
    }

    LOGI("Ext4Operator::Mount success");
    return E_OK;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t Ext4Operator::Format(const std::string& devPath,
                             const std::string& diskPath,
                             const std::string& partitionType,
                             const int32_t partitionNum)
{
#ifdef PC_EXT4_ENABLE
    LOGI("Ext4Operator::Format devPath=%{public}s", devPath.c_str());

    if (devPath.empty()) {
        LOGE("Ext4Operator::Format devPath is empty");
        return E_PARAMS_INVALID;
    }

    std::vector<std::string> cmd = {
        "mke2fs",
        "-F",
        "-t",
        "ext4",
        devPath
    };

    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("Ext4Operator::Format output: %{public}s", GetAnonyString(str).c_str());
    }

    if (err == E_NO_CHILD) {
        err = E_OK;
    }

    if (err != E_OK) {
        LOGE("Ext4Operator::Format failed, err=%{public}d", err);
        return err;
    }
    if (partitionNum == 0) {
        LOGI("Ext4Operator::Format success, partitionNum is 0, no need to fix");
        return E_OK;
    }
    err = FixTypeIdentifier(diskPath, partitionType, partitionNum);
    if (err != E_OK) {
        LOGE("Ext4Operator::Format FixTypeIdentifier failed, err=%{public}d", err);
        return err;
    }
    LOGI("Ext4Operator::Format success");
    return E_OK;
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t Ext4Operator::FixTypeIdentifier(const std::string& diskPath,
                                        const std::string& partitionType,
                                        const int32_t partitionNum)
{
    LOGI("Ext4Operator::FixTypeIdentifier diskPath=%{public}s, partitionType=%{public}s, partitionNum=%{public}d",
         diskPath.c_str(), partitionType.c_str(), partitionNum);
    std::string typeIdentifier;
    std::vector<std::string> cmd;
    if (partitionType == "mbr") {
        typeIdentifier = std::to_string(partitionNum) + ":" + "0x83";
        cmd = {"ohos_fixparts", "-t", typeIdentifier, diskPath};
    } else if (partitionType == "gpt") {
        typeIdentifier = std::to_string(partitionNum) + ":" + "0x8300";
        cmd = {"sgdisk", "-t", typeIdentifier, diskPath};
    } else {
        LOGE("Ext4Operator::FixTypeIdentifier failed, unknown partitionType=%{public}s", partitionType.c_str());
        return E_NOT_SUPPORT;
    }
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("Ext4Operator::FixTypeIdentifier output: %{public}s", GetAnonyString(str).c_str());
    }
    if (ret != E_OK) {
        LOGE("Ext4Operator::FixTypeIdentifier failed, ret=%{public}d", ret);
        return ret;
    }
    LOGI("Ext4Operator::FixTypeIdentifier success");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS
