/*
 * Copyright (C) 2021-2026 Huawei Device Co., Ltd.
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
#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

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

namespace OHOS {
namespace StorageDaemon {
constexpr int32_t DEFAULT_DEV_INDEX = 1;
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr mode_t PUBLIC_DIR_MODE = 02770;
MtpDeviceManager::MtpDeviceManager() {}

MtpDeviceManager::~MtpDeviceManager()
{
    LOGI("MtpDeviceManager Destructor.");
}

int32_t MtpDeviceManager::PrepareMtpMountPath(const std::string &path)
{
    if (!IsDir(path)) {
        LOGI("PrepareMtpMountPath: mtp device mount path directory does not exist, creating it.");
        bool ret = PrepareDir(path, PUBLIC_DIR_MODE, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!ret) {
            LOGE("Prepare directory for mtp device path = %{public}s failed.", path.c_str());
            return E_MTP_PREPARE_DIR_ERR;
        }
    }
    return E_OK;
}

int32_t MtpDeviceManager::MountDevice(const MtpDeviceInfo &device)
{
    LOGI("MountDevice: start mount mtp device, path=%{public}s", device.path.c_str());
    if (isMounting) {
        LOGI("MountDevice: mtp device is mounting, try again later.");
        return E_MTP_IS_MOUNTING;
    }
    isMounting = true;
    int32_t ret = PrepareMtpMountPath(device.path);
    if (ret != E_OK) {
        isMounting = false;
        return ret;
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
        "enable-move",
        "-o",
        "max_idle_threads=10",
        "-o",
        "max_threads=20",
        "-o",
        "context=u:object_r:mnt_external_file:s0",
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
        UmountDevice(device, false, false);
        isMounting = false;
        return err;
    }

    LOGI("Run mtpfs cmd to mount mtp device success.");
    isMounting = false;
    StorageManagerClient client;
    client.NotifyMtpMounted(device.id, device.path, device.vendor, device.uuid);
    return E_OK;
}

int32_t MtpDeviceManager::UmountDevice(const MtpDeviceInfo &device, bool needNotify, bool isBadRemove)
{
    LOGI("MountDevice: start umount mtp device, path=%{public}s", device.path.c_str());
    if (isBadRemove) {
        LOGI("force to umount mtp device");
        int ret = umount2(device.path.c_str(), MNT_DETACH);
        if (ret != 0) {
            LOGW("umount2 failed in force mode, errno %{public}d", errno);
            if (needNotify) {
                StorageManagerClient client;
                client.NotifyMtpUnmounted(device.id, isBadRemove);
            }
            return E_OK;
        }
        ret = remove(device.path.c_str());
        if (ret != 0) {
            LOGW("remove failed in force mode, errno %{public}d", errno);
        }
        if (needNotify) {
            StorageManagerClient client;
            client.NotifyMtpUnmounted(device.id, isBadRemove);
        }

        DelFolder(device.path);
        return E_OK;
    }

    int ret = umount(device.path.c_str());
    if (ret != E_OK) {
        LOGE("umount failed errno %{public}d", errno);
        return E_MTP_UMOUNT_FAILED;
    }
    int err = remove(device.path.c_str());
    if (err != E_OK) {
        LOGE("failed to call remove(%{public}s) error, errno=%{public}d", device.path.c_str(), errno);
        return E_SYS_KERNEL_ERR;
    }
    LOGI("Mtp device unmount success.");
    if (needNotify) {
        StorageManagerClient client;
        client.NotifyMtpUnmounted(device.id, isBadRemove);
    }
    DelFolder(device.path);
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
