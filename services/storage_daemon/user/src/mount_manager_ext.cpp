/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "user/mount_manager.h"

#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>

#include "parameter.h"
#include "user/mount_constant.h"
#include "utils/mount_argument_utils.h"
#include "utils/storage_radar.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace OHOS::StorageService;
const string PROC_MOUNTS = "/proc/mounts";
const string REMOTE_SHARE_PATH_DIR = "/.remote_share";
static constexpr int SHARE_FILE_0771 = 0771;
constexpr int32_t PATH_MAX_FOR_LINK = 4096;

void MountManager::CheckSymlinkForMulti(const std::string &fdPath, const std::string &path,
    std::set<std::string> &occupyFiles)
{
    char realPath[PATH_MAX_FOR_LINK];
    int res = readlink(fdPath.c_str(), realPath, sizeof(realPath) - 1);
    if (res < 0) {
        LOGE("readlink failed for multi, errno is %{public}d.", errno);
        return;
    }
    realPath[res] = '\0';
    std::string realPathStr(realPath);
    if (realPathStr.find(UN_REACHABLE) == 0) {
        realPathStr = realPathStr.substr(strlen(UN_REACHABLE)) + FILE_SEPARATOR_CHAR;
    }
    if (realPathStr.find(path) == 0) {
        LOGE("find a fd from link, %{public}s", realPathStr.c_str());
        realPathStr = realPathStr.substr(path.size());
        if (realPathStr.empty()) {
            return;
        }
        if (path == FILE_MGR_ROOT_PATH) {
            occupyFiles.insert(realPathStr);
            return;
        }
        std::string::size_type point = realPathStr.find(FILE_SEPARATOR_CHAR);
        if (point != std::string::npos) {
            realPathStr = realPathStr.substr(0, point);
        }
        occupyFiles.insert(realPathStr);
    }
}

int32_t MountManager::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    std::lock_guard<std::mutex> lock(mountDisMutex_);
    LOGI("mount share file start.");
    std::map<std::string, std::string> notMountPaths = shareFiles;
    FilterNotMountedPath(notMountPaths);
    for (const auto &item: notMountPaths) {
        std::string dstPath = item.first;
        std::string srcPath = item.second;
        if (!IsDir(srcPath)) {
            LOGE("mount share file, src path invalid, errno is %{public}d", errno);
            return E_NON_EXIST;
        }
        if (HandleDisDstPath(dstPath) != E_OK) {
            return E_MOUNT_SHARE_FILE;
        }
        int32_t ret = Mount(srcPath, dstPath, nullptr, MS_BIND, nullptr);
        if (ret != 0) {
            LOGE("mount share file failed, errno is %{public}d", errno);
            std::string extraData = "src=" + srcPath + ",dst=" + dstPath + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("MountDisShareFile", userId, E_MOUNT_SHARE_FILE, extraData);
            return E_MOUNT_SHARE_FILE;
        }
    }
    LOGI("mount share file success.");
    return E_OK;
}

int32_t MountManager::HandleDisDstPath(const std::string &dstPath)
{
    std::error_code errorCode;
    if (!std::filesystem::exists(dstPath, errorCode)) {
        if (!MkDirRecurse(dstPath, SHARE_FILE_0771)) {
            std::string extraData = "mkdir failed,errCode=" + std::to_string(errno);
            StorageRadar::ReportUserManager("HandleDisDstPath", DEFAULT_USERID, E_MOUNT_SHARE_FILE, extraData);
            LOGE("mount share file, dst path mkdir failed, errno is %{public}d", errno);
            return E_ERR;
        }
        return E_OK;
    }
    std::string extraData;
    std::filesystem::directory_iterator iter(dstPath);
    std::filesystem::directory_iterator endIter;
    if (iter != endIter) {
        std::filesystem::path entryName = iter->path().filename();
        struct stat st;
        if (stat(iter->path().c_str(), &st) == 0) {
            LOGE("dir not empty, uid is %{public}d, gid is %{public}d, mode is %{public}d",
                st.st_uid, st.st_gid, st.st_mode);
            extraData = "uid=" + std::to_string(st.st_uid) + ",gid=" + std::to_string(st.st_gid) + ",mode="
                + std::to_string(st.st_mode) + ",name=" + entryName.string();
            StorageRadar::ReportUserManager("HandleDisDstPath", DEFAULT_USERID, E_MOUNT_SHARE_FILE, extraData);
        }
        return E_ERR;
    }
    auto pos = dstPath.find(REMOTE_SHARE_PATH_DIR);
    if (pos == std::string::npos) {
        LOGE("REMOTE_SHARE_PATH_DIR not found in dstPath");
        return E_ERR;
    }
    std::string tempPath = dstPath.substr(0, pos + REMOTE_SHARE_PATH_DIR.size());
    if (!RmDirRecurse(tempPath)) {
        extraData = "rm dst path failed,errCode=" + std::to_string(errno);
        StorageRadar::ReportUserManager("HandleDisDstPath", DEFAULT_USERID, E_MOUNT_SHARE_FILE, extraData);
        LOGE("mount share file, remove dst path failed, errno is %{public}d", errno);
        return E_ERR;
    }
    if (!MkDirRecurse(dstPath, SHARE_FILE_0771)) {
        extraData = "mkdir failed after remove,errCode=" + std::to_string(errno);
        StorageRadar::ReportUserManager("HandleDisDstPath", DEFAULT_USERID, E_MOUNT_SHARE_FILE, extraData);
        LOGE("mount share file, dst path mkdir failed after remove, errno is %{public}d", errno);
        return E_ERR;
    }
    return E_OK;
}

int32_t MountManager::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    std::lock_guard<std::mutex> lock(mountDisMutex_);
    LOGI("umount share file, userId is %{public}d, networkId is %{private}s.", userId, networkId.c_str());
    std::list<std::string> mounts;
    FindMountsByNetworkId(networkId, mounts);
    for (const std::string &item: mounts) {
        auto pos = item.find(REMOTE_SHARE_PATH_DIR);
        if (pos == std::string::npos) {
            continue;
        }
        int32_t ret = UMount2(item, MNT_DETACH);
        if (ret != E_OK && errno != ENOENT && errno != EINVAL) {
            LOGE("umount share file failed, errno is %{public}d.", errno);
            std::string extraData = "networkId=" + networkId + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("UMountDisShareFile", userId, E_UMOUNT_SHARE_FILE, extraData);
            continue;
        }
        std::string path = item.substr(0, pos + REMOTE_SHARE_PATH_DIR.size());
        RmDirRecurse(path);
    }
    LOGI("umount share file end.");
    return E_OK;
}

int32_t MountManager::FindMountsByNetworkId(const std::string &networkId, std::list<std::string> &mounts)
{
    std::ifstream inputStream(PROC_MOUNTS.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open /proc/mounts, errno is %{public}d", errno);
        return E_UMOUNT_PROC_MOUNTS_OPEN;
    }
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        if (tmpLine.empty()) {
            continue;
        }
        std::stringstream ss(tmpLine);
        std::string dst;
        if ((ss >> dst >> dst) && dst.find(networkId) != std::string::npos) {
            mounts.push_front(dst);
        }
    }
    return E_OK;
}

int32_t MountManager::FilterNotMountedPath(std::map<std::string, std::string> &notMountPaths)
{
    std::ifstream inputStream(PROC_MOUNTS.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open /proc/mounts, errno is %{public}d", errno);
        return E_UMOUNT_PROC_MOUNTS_OPEN;
    }
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        if (tmpLine.empty()) {
            continue;
        }
        std::stringstream ss(tmpLine);
        std::string dst;
        if ((ss >> dst >> dst) && notMountPaths.find(dst) != notMountPaths.end()) {
            notMountPaths.erase(dst);
        }
    }
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS