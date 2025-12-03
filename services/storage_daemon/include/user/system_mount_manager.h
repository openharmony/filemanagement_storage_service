/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_SYSTEM_MOUNT_MANAGER_H
#define OHOS_STORAGE_DAEMON_SYSTEM_MOUNT_MANAGER_H

#include <nocopyable.h>
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

namespace OHOS {
namespace StorageDaemon {

class SystemMountManager final {
public:
    static SystemMountManager &GetInstance();
    int32_t MountCloudByUserId(int32_t userId);
    int32_t UMountCloudByUserId(int32_t userId);
    void SetCloudState(bool active);

private:
    SystemMountManager() = default;
    ~SystemMountManager() = default;
    DISALLOW_COPY_AND_MOVE(SystemMountManager);

    int32_t CloudTwiceMount(int32_t userId);
    int32_t CloudTwiceUMount(int32_t userId);
    void MountCloudForUsers(void);
    void UMountCloudForUsers(void);
    int32_t CloudMount(int32_t userId, const std::string &path);
    

    std::mutex mountMutex_;
    std::vector<int32_t> fuseToMountUsers_;
    std::vector<int32_t> fuseMountedUsers_;
    bool cloudReady_{false};
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_SYSTEM_MOUNT_MANAGER_H
