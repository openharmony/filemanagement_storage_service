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
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

#include "parameter.h"
#include "user/mount_constant.h"
#include "utils/mount_argument_utils.h"
#include "utils/storage_radar.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_utils.h"

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
    LOGI("[L2:MountManager] CheckSymlinkForMulti: >>> ENTER <<< fdPath=%{public}s, path=%{public}s",
        fdPath.c_str(), path.c_str());
    char realPath[PATH_MAX_FOR_LINK];
    int res = readlink(fdPath.c_str(), realPath, sizeof(realPath) - 1);
    if (res < 0) {
        LOGE("[L2:MountManager] CheckSymlinkForMulti: <<< EXIT FAILED <<< readlink failed, errno=%{public}d", errno);
        return;
    }
    realPath[res] = '\0';
    std::string realPathStr(realPath);
    if (realPathStr.find(UN_REACHABLE) == 0) {
        realPathStr = realPathStr.substr(strlen(UN_REACHABLE)) + FILE_SEPARATOR_CHAR;
    }
    if (realPathStr.find(path) == 0) {
        LOGE("[L2:MountManager] CheckSymlinkForMulti: find a fd from link, %{public}s", realPathStr.c_str());
        realPathStr = realPathStr.substr(path.size());
        if (realPathStr.empty()) {
            LOGI("[L2:MountManager] CheckSymlinkForMulti: <<< EXIT SUCCESS <<< realPathStr is empty");
            return;
        }
        if (path == FILE_MGR_ROOT_PATH) {
            occupyFiles.insert(realPathStr);
            LOGI("[L2:MountManager] CheckSymlinkForMulti: <<< EXIT SUCCESS <<< inserted to occupyFiles");
            return;
        }
        std::string::size_type point = realPathStr.find(FILE_SEPARATOR_CHAR);
        if (point != std::string::npos) {
            realPathStr = realPathStr.substr(0, point);
        }
        occupyFiles.insert(realPathStr);
    }
    LOGI("[L2:MountManager] CheckSymlinkForMulti: <<< EXIT SUCCESS <<< occupyFiles.size()=%{public}zu",
         occupyFiles.size());
}

int32_t MountManager::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    LOGI("[L2:MountManager] MountDisShareFile: >>> ENTER <<< userId=%{public}d, shareFiles.size()=%{public}zu",
        userId, shareFiles.size());
    std::lock_guard<std::mutex> lock(mountDisMutex_);
    LOGI("[L2:MountManager] MountDisShareFile: mount share file start.");
    std::map<std::string, std::string> notMountPaths = shareFiles;
    int32_t ret = FilterNotMountedPath(notMountPaths);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] MountDisShareFile: <<< EXIT FAILED <<< FilterNotMountedPath failed, ret=%{public}d",
            ret);
        return ret;
    }
    for (const auto &item: notMountPaths) {
        std::string dstPath = item.first;
        std::string srcPath = item.second;
        if (!IsDir(srcPath)) {
            LOGE("[L2:MountManager] MountDisShareFile: <<< EXIT FAILED <<< src path invalid, errno=%{public}d", errno);
            return E_NON_EXIST;
        }
        if (!MatchesDisSharePath(dstPath)) {
            LOGE("[L2:MountManager] MountDisShareFile: mount share file, dst path invalid, errno is %{public}d", errno);
            return E_NON_EXIST;
        }
        if (!IsDir(dstPath) && !MkDirRecurse(dstPath, SHARE_FILE_0771)) {
            LOGE("[L2:MountManager] MountDisShareFile: mount share file, dst path mkdir failed,"
                "errno is %{public}d", errno);
            return E_NON_EXIST;
        }
        ret = Mount(srcPath, dstPath, nullptr, MS_BIND, nullptr);
        if (ret != E_OK) {
            LOGE("[L2:MountManager] MountDisShareFile: mount share file failed, errno is %{public}d", errno);
            std::string extraData = "src=" + srcPath + ",dst=" + dstPath + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("MountDisShareFile", userId, E_MOUNT_SHARE_FILE, extraData);
            return E_MOUNT_SHARE_FILE;
        }
    }
    LOGI("[L2:MountManager] MountDisShareFile: mount share file success.");
    return E_OK;
}

int32_t MountManager::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    LOGI("[L2:MountManager] UMountDisShareFile: >>> ENTER <<< userId=%{public}d, networkId=%{private}s",
        userId, networkId.c_str());
    std::lock_guard<std::mutex> lock(mountDisMutex_);
    LOGI("[L2:MountManager] UMountDisShareFile: umount share file start.");
    std::list<std::string> mounts;
    int32_t ret = FindMountsByNetworkId(networkId, mounts);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] UMountDisShareFile: <<< EXIT FAILED <<< FindMountsByNetworkId failed, ret=%{public}d",
            ret);
        return ret;
    }
    for (const std::string &item: mounts) {
        if (!MatchesDisSharePath(item)) {
            continue;
        }
        ret = UMount2(item, MNT_DETACH);
        if (ret != E_OK && errno != ENOENT && errno != EINVAL) {
            LOGE("[L2:MountManager] UMountDisShareFile: umount share file failed, errno is %{public}d.", errno);
            std::string extraData = "networkId=" + networkId + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("UMountDisShareFile", userId, E_UMOUNT_SHARE_FILE, extraData);
            continue;
        }
        RemoveDisSharePath(item, networkId);
    }
    LOGI("[L2:MountManager] UMountDisShareFile: umount share file end.");
    return E_OK;
}

int32_t MountManager::FindMountsByNetworkId(const std::string &networkId, std::list<std::string> &mounts)
{
    LOGI("[L2:MountManager] FindMountsByNetworkId: >>> ENTER <<< networkId=%{private}s", networkId.c_str());
    std::ifstream inputStream(PROC_MOUNTS.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("[L2:MountManager] FindMountsByNetworkId: <<< EXIT FAILED <<< unable to open /proc/mounts,"
            "errno=%{public}d", errno);
        return E_UMOUNT_PROC_MOUNTS_OPEN;
    }
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        if (tmpLine.empty()) {
            continue;
        }
        std::stringstream ss(tmpLine);
        std::string dst;
        ss >> dst >> dst;
        if (dst.empty()) {
            continue;
        }
        if (dst.find(networkId) != std::string::npos) {
            mounts.push_front(dst);
        }
    }
    LOGI("[L2:MountManager] FindMountsByNetworkId: <<< EXIT SUCCESS <<< mounts.size()=%{public}zu", mounts.size());
    return E_OK;
}

int32_t MountManager::FilterNotMountedPath(std::map<std::string, std::string> &notMountPaths)
{
    LOGI("[L2:MountManager] FilterNotMountedPath: >>> ENTER <<< notMountPaths.size()=%{public}zu",
        notMountPaths.size());
    std::ifstream inputStream(PROC_MOUNTS.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("[L2:MountManager] FilterNotMountedPath: <<< EXIT FAILED <<< unable to open /proc/mounts,"
            "errno=%{public}d", errno);
        return E_UMOUNT_PROC_MOUNTS_OPEN;
    }
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        if (tmpLine.empty()) {
            continue;
        }
        std::stringstream ss(tmpLine);
        std::string dst;
        ss >> dst >> dst;
        if (dst.empty()) {
            continue;
        }
        if (notMountPaths.find(dst) != notMountPaths.end()) {
            notMountPaths.erase(dst);
        }
    }
    LOGI("[L2:MountManager] FilterNotMountedPath: <<< EXIT SUCCESS <<< remaining paths=%{public}zu",
        notMountPaths.size());
    return E_OK;
}

bool MountManager::MatchesDisSharePath(const std::string &dstPath)
{
    if (dstPath.empty()) {
        return false;
    }
    std::string userIdPattern = R"((0|[1-9][0-9]{0,4}))";
    std::string bundlePattern = R"(.{1,100})";
    std::string networkIdPattern = R"([0-9a-zA-Z]{1,65})";
    std::string levelPattern = R"([0-9a-zA-Z]{1,3})";
    std::string mediaPattern = R"(^/data/service/el2/)" + userIdPattern + R"(/hmdfs/account/data/)" + bundlePattern +
        R"(/\.remote_share/)" + networkIdPattern + R"(/Photo$)";
    std::string otherPattern = R"(^/data/service/el2/)" + userIdPattern + R"(/hmdfs/account/data/)" + bundlePattern +
        R"(/\.remote_share/)" + networkIdPattern + R"(/data/storage/)" + levelPattern + R"(/base$)";
    std::regex mediaRegex(mediaPattern);
    std::regex otherRegex(otherPattern);
    return std::regex_match(dstPath, mediaRegex) || std::regex_match(dstPath, otherRegex);
}

void MountManager::RemoveDisSharePath(const std::string &dstPath, const std::string &networkId)
{
    if (dstPath.empty() || networkId.empty()) {
        return;
    }
    std::string separator = "/";
    std::string dstPathTmp = dstPath;
    std::vector<std::string> parts = SplitLine(dstPathTmp, separator);
    auto count = static_cast<int32_t>(parts.size());
    for (int32_t i = count - 1; i >= 0; --i) {
        if (parts[i].empty()) {
            continue;
        }
        std::string currentPath;
        for (int32_t j = 0; j <= i; ++j) {
            if (parts[j].empty()) {
                continue;
            }
            currentPath += separator + parts[j];
        }
        std::error_code errCode;
        if (!std::filesystem::exists(currentPath, errCode)) {
            LOGE("path not exists, errno is %{public}d, errCode is %{public}s, path is %{public}s.", errno,
                 errCode.message().c_str(), currentPath.c_str());
            break;
        }
        if (!std::filesystem::is_directory(currentPath) || !std::filesystem::is_empty(currentPath)) {
            LOGE("path is not dir or not empty, %{public}s.", currentPath.c_str());
            break;
        }
        if (rmdir(currentPath.c_str()) != 0) {
            LOGE("del share file failed, errno is %{public}d.", errno);
            std::string extraData = "path=" + currentPath + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("RemoveDisSharePath", GLOBAL_USER_ID, E_UMOUNT_SHARE_FILE, extraData);
            break;
        }
        if (parts[i] == networkId) {
            break;
        }
    }
}
} // namespace StorageDaemon
} // namespace OHOS