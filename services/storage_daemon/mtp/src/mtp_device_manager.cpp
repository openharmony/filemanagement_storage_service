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
#include <algorithm>
#include <config.h>
#include <dirent.h>
#include <iostream>
#include <csignal>
#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "ipc/storage_manager_client.h"
#include "simple-mtpfs-fuse.h"
#include "simple-mtpfs-util.h"
#include "storage_service_log.h"
#include "mtp/mtp_device_manager.h"
#include "utils/cmd_utils.h"
#include "utils/file_utils.h"
using namespace std;
namespace OHOS {
namespace StorageDaemon {
bool MtpDeviceManager::MountMtp(std::string id, std::string devlinks, std::string path)
{
    LOGI("Mtp  MountMtp id:%{public}s devlinks:%{public}s path:%{public}s", id.c_str(), devlinks.c_str(), path.c_str());
    bool success = false;
    if (!isMounting) {
        isMounting = true;
        if (PrepareDir(path,PUBLIC_DIR_MODE,FILE_MANAGER_UID,FILE_MANAGER_GID)) {
            string cmd = "exec-simple-mtpfs ";
            cmd.append("-o uid=")
            .append(std::to_string(FILE_MANAGER_UID))
            .append(" ")
            .append("-o gid=")
            .append(std::to_string(FILE_MANAGER_GID))
            .append(" ")
            .append("-o allow_other")
            .append(" ")
            .append("-o enable-move")
            .append(" ")
            .append("-o hard_remove")
            .append(" ")   
            .append(devlinks)
            .append(" ")
            .append(path);
            LOGI("MountMtp cmd: %{public}s", cmd.c_str());
            vector<string> result;
            bool cmdSuccess = CmdUtils::GetInstance().RunCmd(cmd, result);
            for (auto str : result) {
                LOGI("MountMtp result: %{public}s", str.c_str());
            }
            if (cmdSuccess && (result.size() == 0)) {
                success = true;
            }
        }
        if(!success){
            DelFolder(path);
        }
    }
    StorageManagerClient client;
    client.NotifyMtpMounted(id, devlinks, path, success);
    isMounting = false;
    return success;
}

bool MtpDeviceManager::UnMountMtp(std::string id, string path)
{
    LOGI("Mtp  UnMountMtp id:%{public}s path:%{public}s", id.c_str(), path.c_str());
    bool success = false;
    string cmd = "umount " + path;
    vector<string> result;
    bool runSuccess = CmdUtils::GetInstance().RunCmd(cmd, result);
    if (runSuccess && result.size() == 0) {
        success = DelFolder(path);
    }
    StorageManagerClient client;
    client.NofifyMtpUnMounted(id, path, success);
    return success;
}

bool MtpDeviceManager::UnMountMtpAll(string path)
{
    LOGI("Mtp  UnMountMtpAll path:%{public}s", path.c_str());
    DIR *dir;
    if ((dir = opendir(path.c_str())) == nullptr) {
        return false;
    }
    for (dirent *dp = readdir(dir); dp != nullptr; dp = readdir(dir)) {
        std::string fileName = dp->d_name;
        if (fileName == "." || fileName == "..") {
            continue;
        }
        if ((dp->d_type == DT_DIR) && (fileName.find("mtp-") == 0)) {
            MtpDeviceManager::GetInstance().UnMountMtp("", path + fileName);
        }
    }
    closedir(dir);
    StorageManagerClient client;
    client.NofityMtpUMountAll();
    return true;
}
} // namespace StorageDaemon
} // namespace OHOS