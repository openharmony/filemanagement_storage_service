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

MtpDeviceManager::MtpDeviceManager()
{
    LOGI("[L2:MtpDeviceManager] MtpDeviceManager: >>> ENTER <<<");
}

MtpDeviceManager::~MtpDeviceManager()
{
    LOGI("[L2:MtpDeviceManager] ~MtpDeviceManager: >>> ENTER <<<");
}

int32_t MtpDeviceManager::PrepareMtpMountPath(const std::string &path)
{
    LOGI("[L2:MtpDeviceManager] PrepareMtpMountPath: >>> ENTER <<< path=%{public}s", path.c_str());
    if (!IsDir(path)) {
        LOGI("[L2:MtpDeviceManager] PrepareMtpMountPath: directory does not exist, creating");
        bool ret = PrepareDir(path, PUBLIC_DIR_MODE, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!ret) {
            LOGE("[L2:MtpDeviceManager] PrepareMtpMountPath: <<< EXIT FAILED <<< path=%{public}s, PrepareDir failed",
                path.c_str());
            return E_MTP_PREPARE_DIR_ERR;
        }
    }
    LOGI("[L2:MtpDeviceManager] PrepareMtpMountPath: <<< EXIT SUCCESS <<< path=%{public}s", path.c_str());
    return E_OK;
}

int32_t MtpDeviceManager::MountDevice(const MtpDeviceInfo &device)
{
    LOGI("[L2:MtpDeviceManager] MountDevice: >>> ENTER <<< id=%{public}s, path=%{public}s",
        device.id.c_str(), device.path.c_str());
    if (isMounting_) {
        LOGI("MountDevice: mtp device is mounting, try again later.");
        return E_MTP_IS_MOUNTING;
    }
    isMounting_ = true;
    int32_t ret = PrepareMtpMountPath(device.path);
    if (ret != E_OK) {
        isMounting_ = false;
        LOGE("[L2:MtpDeviceManager] MountDevice: <<< EXIT FAILED <<< PrepareMtpMountPath failed, err=%{public}d", ret);
        return ret;
    }
    std::vector<std::string> cmdVec = {
        device.type,
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
        LOGI("[L2:MtpDeviceManager] MountDevice: result=%{public}s", str.c_str());
    }
    if ((err != 0) || (result.size() != 0)) {
        LOGE("[L2:MtpDeviceManager] MountDevice: <<< EXIT FAILED <<< mtpfs cmd failed, err=%{public}d", err);
        rmdir(device.path.c_str());
        isMounting_ = false;
        return err != 0 ? err : E_MTP_MOUNT_FAILED;
    }

    LOGI("[L2:MtpDeviceManager] MountDevice: <<< EXIT SUCCESS <<< id=%{public}s", device.id.c_str());
    isMounting_ = false;
    StorageManagerClient client;
    client.NotifyMtpMounted(device.id, device.path, device.vendor, device.uuid, device.type);
    return E_OK;
}

int32_t MtpDeviceManager::UmountDevice(const MtpDeviceInfo &device, bool needNotify, bool isBadRemove)
{
    LOGI("[L2:MtpDeviceManager] UmountDevice: >>> ENTER <<< id=%{public}s, path=%{public}s, needNotify=%{public}d,"
        "isBadRemove=%{public}d", device.id.c_str(), device.path.c_str(), needNotify, isBadRemove);
    if (isBadRemove) {
        LOGI("[L2:MtpDeviceManager] UmountDevice: force umount mode");
        int ret = umount2(device.path.c_str(), MNT_DETACH);
        if (ret != 0) {
            LOGW("[L2:MtpDeviceManager] UmountDevice: umount2 failed in force mode, errno=%{public}d", errno);
            if (needNotify) {
                StorageManagerClient client;
                client.NotifyMtpUnmounted(device.id, isBadRemove);
            }
            LOGI("[L2:MtpDeviceManager] UmountDevice: <<< EXIT SUCCESS <<< force mode completed");
            return E_OK;
        }
        ret = rmdir(device.path.c_str());
        if (ret != 0) {
            LOGW("[L2:MtpDeviceManager] UmountDevice: remove failed in force mode, errno=%{public}d", errno);
        }
        if (needNotify) {
            StorageManagerClient client;
            client.NotifyMtpUnmounted(device.id, isBadRemove);
        }
        rmdir(device.path.c_str());
        LOGI("[L2:MtpDeviceManager] UmountDevice: <<< EXIT SUCCESS <<< force umount completed");
        return E_OK;
    }

    int ret = umount(device.path.c_str());
    if (ret != E_OK) {
        LOGE("[L2:MtpDeviceManager] UmountDevice: <<< EXIT FAILED <<< umount failed, errno=%{public}d", errno);
        return E_MTP_UMOUNT_FAILED;
    }
    int err = rmdir(device.path.c_str());
    if (err != E_OK) {
        LOGE("[L2:MtpDeviceManager] UmountDevice: <<< EXIT FAILED <<< remove failed, path=%{public}s, errno=%{public}d",
            device.path.c_str(), errno);
        return E_SYS_KERNEL_ERR;
    }
    LOGI("[L2:MtpDeviceManager] UmountDevice: unmount success");
    if (needNotify) {
        StorageManagerClient client;
        client.NotifyMtpUnmounted(device.id, isBadRemove);
    }
    rmdir(device.path.c_str());
    LOGI("[L2:MtpDeviceManager] UmountDevice: <<< EXIT SUCCESS <<< id=%{public}s", device.id.c_str());
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
