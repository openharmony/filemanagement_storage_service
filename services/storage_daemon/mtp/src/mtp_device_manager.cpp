/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mtp/mtp_device_manager.h"

#include <config.h>
#include <dirent.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/cmd_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
MtpDeviceManager::MtpDeviceManager() {}

MtpDeviceManager::~MtpDeviceManager()
{
    LOGI("MtpDeviceManager Destructor.");
}

int32_t MtpDeviceManager::MountDevice(const MtpDeviceInfo &device)
{
    LOGI("MountDevice: start mount mtp device, path=%{public}s", device.path.c_str());
    if (isMounting) {
        LOGI("MountDevice: mtp device is mounting, try again later.");
        return E_MTP_IS_MOUNTING;
    }
    isMounting = true;
    if (!IsDir(device.path)) {
        LOGI("MountDevice: mtp device mount path directory is not exist, create it first.");
        bool ret = PrepareDir(device.path, PUBLIC_DIR_MODE, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!ret) {
            LOGE("Prepare directory for mtp device path = %{public}s failed.", device.path.c_str());
            return E_MTP_PREPARE_DIR_ERR;
        }
    }

    std::string cmd = "mtpfs";
    cmd.append("-o uid=")
    .append(std::to_string(FILE_MANAGER_UID))
    .append(" ")
    .append("-o gid=")
    .append(std::to_string(FILE_MANAGER_GID))
    .append(" ")
    .append("--device ")
    .append("1")
    .append(" ")
    .append(device.path);
    LOGI("MountDevice: append cmd: %{public}s", cmd.c_str());

    std::vector<std::string> result;
    bool success = CmdUtils::GetInstance().RunCmd(cmd, result);
    for (auto str : result) {
        LOGI("MountDevice result: %{public}s", str.c_str());
    }
    if (!success || (result.size() != 0)) {
        LOGE("Run mtpfs cmd to mount mtp device failed.");
        DelFolder(device.path);
        isMounting = false;
        return E_MTP_MOUNT_FAILED;
    }

    LOGI("Run mtpfs cmd to mount mtp device success.");
    isMounting = false;
    StorageManagerClient client;
    client.NotifyMtpMounted(device.id, device.path);
    return E_OK;
}

int32_t MtpDeviceManager::UmountDevice(const MtpDeviceInfo &device)
{
    LOGI("MountDevice: start umount mtp device, path=%{public}s", device.path.c_str());
    std::string cmd = "umount " + device.path;
    std::vector<std::string> result;
    bool success = CmdUtils::GetInstance().RunCmd(cmd, result);
    if (!success || (result.size() != 0)) {
        LOGE("Run mtpfs cmd to umount mtp device failed.");
        return E_MTP_UMOUNT_FAILED;
    }

    LOGI("Run mtpfs cmd to umount mtp device success.");
    StorageManagerClient client;
    client.NotifyMtpUnmounted(device.id, device.path);
    DelFolder(device.path);
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS