/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_MOUNT_MANAGER_H
#define OHOS_STORAGE_DAEMON_MOUNT_MANAGER_H

#include <fstream>
#include <list>
#include <map>
#include <nocopyable.h>
#include <set>

#include "utils/file_utils.h"
#include "user/user_path_resolver.h"

namespace OHOS {
namespace StorageDaemon {

class MountManager final {
public:
    static MountManager &GetInstance();
    int32_t MountByUser(int32_t userId);
    int32_t UmountByUser(int32_t userId);
    int32_t MountCryptoPathAgain(uint32_t userId);
    int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    void GetAllUserId(std::vector<int32_t> &userIds);
    int32_t PrepareAppdataDir(int32_t userId);

    int32_t UMountAllPath(int32_t userId, std::list<std::string> &unMountFailList);
    int32_t UMountByList(std::list<std::string> &list, std::list<std::string> &unMountFailList);
    int32_t UMountByListWithDetach(std::list<std::string> &list);
    void SetMediaObserverState(bool active);
    int32_t FindMountPointsToMap(std::map<std::string, std::list<std::string>> &mountMap, int32_t userId);
    void MountPointToList(std::list<std::string> &hmdfsList, std::list<std::string> &hmfsList,
        std::list<std::string> &sharefsList, std::string &line, int32_t userId);
    bool CheckMaps(const std::string &path, std::list<std::string> &mountFailList);
    bool CheckSymlink(const std::string &path, std::list<std::string> &mountFailList);
    bool GetProcessInfo(const std::string &filename, ProcessInfo &info);
    bool PidUsingFlag(std::string &pidPath, std::list<std::string> &mountFailList);
    void MountSandboxPath(uint32_t userId, const std::vector<MountNodeInfo> &sandboxMountNodeInfo,
        const std::string &bundleName);
    bool CheckMountFileByUser(int32_t userId);
    bool CloudAndFuseDirFlag(const std::string &path);
    bool MediaFuseDirFlag(const std::string &path);
    int32_t MountMediaFuse(int32_t userId, int32_t &devFd);
    int32_t UMountMediaFuse(int32_t userId);
    int32_t FindAndKillProcess(int32_t userId, std::list<std::string> &unMountFailList, int32_t radar);
    int32_t FindAndKillProcessWithoutRadar(int32_t userId, std::list<std::string> &killList);
    int32_t CheckProcessUserId(int32_t userId, std::vector<ProcessInfo> &proInfos,
                               std::vector<ProcessInfo> &processKillInfos);
    int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd);
    int32_t UMountFileMgrFuse(int32_t userId, const std::string &path);
    int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy);
    int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles);
    int32_t UMountDisShareFile(int32_t userId, const std::string &networkId);
    int32_t ClearSecondMountPoint(uint32_t userId, const std::string &bundleName);

private:
    MountManager() = default;
    ~MountManager() = default;
    int32_t MountHmdfs(int32_t userId);
    int32_t MountSharefs(int32_t userId);

    bool SupportHmdfs();
    int32_t CreateVirtualDirs(int32_t userId);
    int32_t LocalMount(int32_t userId, const std::vector<MountNodeInfo> &hmdfsMountNodeList);
    int32_t LocalUMount(int32_t userId);
    bool CheckPathValid(const std::string &bundleNameStr, uint32_t userId);
    int32_t MountAppdata(int32_t userId, bool beforeStartup);
    bool DirExist(const std::string &dir);
    int32_t PrepareAppdataDirByUserId(int32_t userId);
    int32_t UmountMntUserTmpfs(int32_t userId);
    int32_t UmountFileSystem(int32_t userId);
    int32_t MountFileSystem(int32_t userId);
    int32_t FindProcess(std::list<std::string> &unMountFailList, std::vector<ProcessInfo> &proInfos,
        std::list<std::string> &excludeProcess);
    int32_t FindSaFd(int32_t userId);
    int32_t UMountHmdfsByList(int32_t userId, std::list<std::string> &list, std::list<std::string> &unMountFailList);
    bool IsSysMountPoint(int32_t userId, std::string &path);
    bool CheckSysFs(int32_t userId);
    bool IsSysFsInUse(std::string &path);
    void ForbidOpen(int32_t userId);
    int32_t OpenProcForPath(const std::string &path, bool &isOccupy, bool isDir);
    int32_t OpenProcForMulti(const std::string &path, std::set<std::string> &occupyFiles);
    bool FindProcForPath(const std::string &pidPath, const std::string &path, bool isDir);
    void FindProcForMulti(const std::string &pidPath, const std::string &path, std::set<std::string> &occupyFiles);
    bool CheckSymlinkForPath(const std::string &fdPath, const std::string &path, bool isDir);
    void CheckSymlinkForMulti(const std::string &fdPath, const std::string &path, std::set<std::string> &occupyFiles);
    int32_t FindMountsByNetworkId(const std::string &networkId, std::list<std::string> &mounts);
    int32_t FilterNotMountedPath(std::map<std::string, std::string> &notMountPaths);
    int32_t HandleDisDstPath(const std::string &dstPath);
    bool IsBundleNeedClear(uint32_t userId, const std::string &bundleName);
    int32_t InitSecondMountBundleName(uint32_t userId);
    void RemoveBundleNameFromMap(uint32_t userId, const std::string &bundleName);
    void ClearSecondMountMap(uint32_t userId);

    DISALLOW_COPY_AND_MOVE(MountManager);

    std::mutex mountDisMutex_;
    std::mutex secondMountMutex_;
    std::map<uint32_t, std::vector<std::string>> secondMountBundleNameMap_;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_USER_MANAGER_H
