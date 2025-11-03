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
        if (IsDir(dstPath)) {
            RmDirRecurse(dstPath);
        }
        if (!MkDirRecurse(dstPath, SHARE_FILE_0771)) {
            LOGE("mount share file, dst path mkdir failed, errno is %{public}d", errno);
            return E_NON_EXIST;
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

int32_t MountManager::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    LOGI("umount share file, userId is %{public}d, networkId is %{private}s.", userId, networkId.c_str());
    std::list<std::string> mounts;
    FindMountsByNetworkId(networkId, mounts);
    for (const std::string &item: mounts) {
        int32_t ret = UMount2(item, MNT_DETACH);
        if (ret != E_OK && errno != ENOENT && errno != EINVAL) {
            LOGE("umount share file failed, errno is %{public}d.", errno);
            std::string extraData = "networkId=" + networkId + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("UMountDisShareFile", userId, E_UMOUNT_SHARE_FILE, extraData);
        }
        std::string path = item.substr(0, item.find(networkId) + networkId.size());
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