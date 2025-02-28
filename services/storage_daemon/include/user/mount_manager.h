/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <string>
#include <mutex>
#include <vector>
#include <sys/types.h>
#include <nocopyable.h>

#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
struct DirInfo {
    const std::string path;
    mode_t mode;
    uid_t uid;
    gid_t gid;
};

constexpr uid_t OID_ROOT = 0;
constexpr uid_t OID_SYSTEM = 1000;
constexpr uid_t OID_FILE_MANAGER = 1006;
constexpr uid_t OID_USER_DATA_RW = 1008;
constexpr uid_t OID_DFS = 1009;
constexpr uid_t OID_BACKUP = 1089;
constexpr uid_t OID_DFS_SHARE = 3822;
constexpr uid_t OID_TEE = 6668;
constexpr uid_t OID_DEVICE_AUTH = 3333;
constexpr uid_t OID_HUKS = 3510;
constexpr uid_t OID_ASSET = 6226;
constexpr uid_t OID_DDMS = 3012;
constexpr uid_t OID_HWID = 7008;
constexpr uid_t OID_HEALTH_SPORT = 7259;
constexpr uid_t OID_DLP_CREDENTIAL = 3553;
constexpr uid_t OID_RSS = 1096;
constexpr uid_t OID_HIVIEW = 1201;
constexpr uid_t OID_PARENT_CONTROL = 7007;
constexpr uid_t OID_ACCOUNT = 3058;
constexpr uid_t OID_COLLABORATION_FWK = 5520;
constexpr uid_t OID_CLOUD_BACK = 5206;
constexpr uid_t OID_AV_SESSION = 6700;
constexpr uid_t USER_ID_BASE = 200000;
constexpr uid_t OID_FOUNDATION = 5523;
constexpr uid_t OID_PASTEBOARD = 3816;
constexpr uid_t OID_PRINT = 3823;
constexpr uid_t OID_FINDNETWORK = 7518;

class MountManager final {
public:
    MountManager();
    virtual ~MountManager() = default;
    static std::shared_ptr<MountManager> GetInstance();
    static std::vector<DirInfo> InitHmdfsDirVec();
    static std::vector<DirInfo> InitVirtualDir();
    static std::vector<DirInfo> InitSystemServiceDir();
    static std::vector<DirInfo> InitFileManagerDir();
    static std::vector<DirInfo> InitAppdataDir();
    int32_t MountByUser(int32_t userId);
    int32_t UmountByUser(int32_t userId);
    int32_t PrepareHmdfsDirs(int32_t userId);
    int32_t PrepareFileManagerDirs(int32_t userId);
    int32_t PrepareAppdataDir(int32_t userId);
    int32_t DestroyHmdfsDirs(int32_t userId);
    int32_t DestroyFileManagerDirs(int32_t userId);
    int32_t DestroySystemServiceDirs(int32_t userId);
    int32_t CloudMount(int32_t userId, const std::string &path);
    int32_t CloudTwiceMount(int32_t userId);
    int32_t MountCryptoPathAgain(uint32_t userId);
    int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    int32_t UMountAllPath(int32_t userId, std::list<std::string> &unMountFailList);
    int32_t UMountByList(std::list<std::string> &list, std::list<std::string> &unMountFailList);
    int32_t UMountByListWithDetach(std::list<std::string> &list);
    void SetCloudState(bool active);
    void SetMediaObserverState(bool active);
    int32_t RestoreconSystemServiceDirs(int32_t userId);
    int32_t FindMountPointsToMap(std::map<std::string, std::list<std::string>> &mountMap, int32_t userId);
    void MountPointToList(std::list<std::string> &hmdfsList, std::list<std::string> &hmfsList,
        std::list<std::string> &sharefsList, std::string &line, int32_t userId);
    bool CheckMaps(const std::string &path, std::list<std::string> &mountFailList);
    bool CheckSymlink(const std::string &path, std::list<std::string> &mountFailList);
    bool GetProcessInfo(const std::string &filename, ProcessInfo &info);
    bool PidUsingFlag(std::string &pidPath, std::list<std::string> &mountFailList);
    void MountSandboxPath(const std::vector<std::string> &srcPaths, const std::vector<std::string> &dstPaths,
                          const std::string &bundleName, const std::string &userId);
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

private:
    bool SupportHmdfs();
    int32_t CreateVirtualDirs(int32_t userId);
    int32_t HmdfsMount(int32_t userId, std::string relativePath, bool mountCloudDisk = false);
    int32_t HmdfsTwiceMount(int32_t userId, const std::string &relativePath);
    int32_t SharefsMount(int32_t userId);
    int32_t HmSharefsMount(int32_t userId, std::string &srcPath, std::string &dstPath);
    int32_t LocalMount(int32_t userId);
    int32_t LocalUMount(int32_t userId);
    int32_t SetFafQuotaProId(int32_t userId);
    int32_t CreateSystemServiceDirs(int32_t userId);
    void MountCloudForUsers(void);
    void UMountCloudForUsers(void);
    void PrepareFileManagerDir(int32_t userId);
    int32_t CloudUMount(int32_t userId);
    bool CheckPathValid(const std::string &bundleNameStr, uint32_t userId);
    int32_t MountAppdataAndSharefs(int32_t userId);
    int32_t MountAppdata(const std::string &userId);
    bool DirExist(const std::string &dir);
    void GetAllUserId(std::vector<int32_t> &userIds);
    int32_t PrepareAppdataDirByUserId(int32_t userId);
    int32_t MountSharefsAndNoSharefs(int32_t userId);
    int32_t SharedMount(int32_t userId, const std::string &path);
    int32_t BindAndRecMount(int32_t userId, std::string &srcPath, std::string &dstPath, bool isUseSlave = true);
    int32_t UmountMntUserTmpfs(int32_t userId);
    int32_t UmountFileSystem(int32_t userId);
    int32_t MountFileSystem(int32_t userId);
    int32_t FindProcess(std::list<std::string> &unMountFailList, std::vector<ProcessInfo> &proInfos,
        std::list<std::string> &excludeProcess);
    int32_t FindSaFd(int32_t userId);
    int32_t BindMount(std::string &srcPath, std::string &dstPath);

    DISALLOW_COPY_AND_MOVE(MountManager);

    static std::shared_ptr<MountManager> instance_;
    const std::vector<DirInfo> hmdfsDirVec_;
    const std::vector<DirInfo> virtualDir_;
    const std::vector<DirInfo> systemServiceDir_;
    const std::vector<DirInfo> fileManagerDir_;
    const std::vector<DirInfo> appdataDir_;
    std::mutex mountMutex_;
    std::vector<int32_t> fuseToMountUsers_;
    std::vector<int32_t> fuseMountedUsers_;
    bool cloudReady_{false};
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_USER_MANAGER_H
