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
constexpr uid_t OID_DDMS = 3012;
constexpr uid_t OID_DLP_CREDENTIAL = 3553;
constexpr uid_t USER_ID_BASE = 200000;

class MountManager final {
public:
    MountManager();
    virtual ~MountManager() = default;
    static std::shared_ptr<MountManager> GetInstance();
    int32_t MountByUser(int32_t userId);
    int32_t UmountByUser(int32_t userId);
    int32_t PrepareHmdfsDirs(int32_t userId);
    int32_t PrepareFileManagerDirs(int32_t userId);
    int32_t DestroyHmdfsDirs(int32_t userId);
    int32_t DestroyFileManagerDirs(int32_t userId);
    int32_t DestroySystemServiceDirs(int32_t userId);
    int32_t CloudMount(int32_t userId, const std::string& path);
    int32_t CloudTwiceMount(int32_t userId);
    int32_t MountCryptoPathAgain(uint32_t userId);
    int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    int32_t UMountAllPath(int32_t userId);
    void SetCloudState(bool active);
    int32_t RestoreconSystemServiceDirs(int32_t userId);
    int32_t FindMountPointsToMap(std::map<std::string, std::list<std::string>> &mountMap, int32_t userId);
    void MountPointToList(std::list<std::string> &hmdfsList, std::list<std::string> &hmfsList,
        std::list<std::string> &sharefsList, std::string &line, int32_t userId);

private:
    bool SupportHmdfs();
    int32_t CreateVirtualDirs(int32_t userId);
    int32_t HmdfsMount(int32_t userId);
    int32_t HmdfsMount(int32_t userId, std::string relativePath, bool mountCloudDisk = false);
    int32_t HmdfsTwiceMount(int32_t userId, std::string relativePath);
    int32_t HmdfsUMount(int32_t userId, std::string relativePath);
    int32_t SharefsMount(int32_t userId);
    int32_t LocalMount(int32_t userId);
    int32_t LocalUMount(int32_t userId);
    int32_t SetFafQuotaProId(int32_t userId);
    int32_t CreateSystemServiceDirs(int32_t userId);
    void MountCloudForUsers(void);
    void UMountCloudForUsers(void);
    void PrepareFileManagerDir(int32_t userId);
    int32_t CloudUMount(int32_t userId);

    DISALLOW_COPY_AND_MOVE(MountManager);

    static std::shared_ptr<MountManager> instance_;
    const std::vector<DirInfo> hmdfsDirVec_;
    const std::vector<DirInfo> virtualDir_;
    const std::vector<DirInfo> systemServiceDir_;
    const std::vector<DirInfo> fileManagerDir_;
    std::mutex mountMutex_;
    std::vector<int32_t> fuseToMountUsers_;
    std::vector<int32_t> fuseMountedUsers_;
    bool cloudReady_{false};
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_USER_MANAGER_H
