/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
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
    std::vector<std::string> cmdVec = {
        "mtpfs",
        "-o",
        "uid=" + std::to_string(FILE_MANAGER_UID),
        "-o",
        "gid=" + std::to_string(FILE_MANAGER_GID),
        "-o",
        "allow_other",
        "-o",
        "default_permissions",
        "--device",
        std::to_string(DEFAULT_DEV_INDEX),
        device.path,
    };
    std::vector<std::string> result;
    int32_t err = ForkExec(cmdVec, &result);
    for (auto str : result) {
        LOGI("MountDevice result: %{public}s", str.c_str());
    }
    if ((err != 0) || (result.size() != 0)) {
        LOGE("Run mtpfs cmd to mount mtp device failed.");
        UmountDevice(device, false);
        isMounting = false;
        return E_MTP_MOUNT_FAILED;
    }

    LOGI("Run mtpfs cmd to mount mtp device success.");
    isMounting = false;
    StorageManagerClient client;
    client.NotifyMtpMounted(device.id, device.path, device.vendor);
    
    return E_OK;
}

int32_t MtpDeviceManager::UmountDevice(const MtpDeviceInfo &device, bool needNotify)
{
    LOGI("MountDevice: start umount mtp device, path=%{public}s", device.path.c_str());
    int ret = umount(device.path.c_str());
    int err = remove(device.path.c_str());
    if (err && ret) {
        LOGE("umount mtp device error.");
        return E_MTP_UMOUNT_FAILED;
    }
    if (err) {
        LOGE("failed to call remove(%{public}s) error, errno=%{public}d", device.path.c_str(), errno);
        return E_SYS_CALL;
    }
    LOGI("Mtp device unmount success.");
    if (needNotify) {
        StorageManagerClient client;
        client.NotifyMtpUnmounted(device.id, device.path);
    }
    DelFolder(device.path);
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS