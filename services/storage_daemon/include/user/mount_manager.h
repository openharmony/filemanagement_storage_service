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

#include <string>
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
constexpr uid_t OID_USER_DATA_RW = 1008;
constexpr uid_t OID_DFS = 1009;
constexpr uid_t OID_BACKUP = 1089;
constexpr uid_t OID_DFS_SHARE = 3822;

class MountManager final {
public:
    MountManager();
    virtual ~MountManager() = default;
    static std::shared_ptr<MountManager> GetInstance();
    int32_t MountByUser(int32_t userId);
    int32_t UmountByUser(int32_t userId);
    int32_t PrepareHmdfsDirs(int32_t userId);
    int32_t DestroyHmdfsDirs(int32_t userId);

private:
    bool SupportHmdfs();
    int32_t CreateVirtualDirs(int32_t userId);
    int32_t HmdfsMount(int32_t userId);
    int32_t HmdfsMount(int32_t userId, std::string relativePath);
    int32_t HmdfsTwiceMount(int32_t userId, std::string relativePath);
    int32_t HmdfsUMount(int32_t userId);
    int32_t HmdfsUMount(int32_t userId, std::string relativePath);
    int32_t SharefsMount(int32_t userId);
    int32_t SharefsUMount(int32_t userId);
    int32_t HmdfsTwiceUMount(int32_t userId, std::string relativePath);
    int32_t LocalMount(int32_t userId);
    int32_t LocalUMount(int32_t userId);

    DISALLOW_COPY_AND_MOVE(MountManager);

    static std::shared_ptr<MountManager> instance_;
    const std::vector<DirInfo> hmdfsDirVec_;
    const std::vector<DirInfo> virtualDir_;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_USER_MANAGER_H