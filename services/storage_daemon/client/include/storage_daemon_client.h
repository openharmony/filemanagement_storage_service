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

#ifndef STORAGE_DAEMON_CILENT_H
#define STORAGE_DAEMON_CILENT_H

#include <mutex>
#include <iostream>
#include <vector>
#include <sys/types.h>

#include "iremote_proxy.h"
#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon.h"
#include "ipc/storage_daemon_proxy.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonClient {
public:
    static int32_t PrepareUserDirs(int32_t userId, uint32_t flags);
    static int32_t DestroyUserDirs(int32_t userId, uint32_t flags);
    static int32_t StartUser(int32_t userId);
    static int32_t StopUser(int32_t userId);
    static int32_t PrepareUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags);
    static int32_t DestroyUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags);
    static int32_t InitGlobalKey(void);
    static int32_t InitGlobalUserKeys(void);
    static int32_t GenerateUserKeys(uint32_t userId, uint32_t flags);
    static int32_t DeleteUserKeys(uint32_t userId);
    static int32_t UpdateUserAuth(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &oldSecret,
                                  const std::vector<uint8_t> &newSecret);
    static int32_t ActiveUserKey(uint32_t userId,
                                 const std::vector<uint8_t> &token,
                                 const std::vector<uint8_t> &secret);
    static int32_t InactiveUserKey(uint32_t userId);
    static int32_t FscryptEnable(const std::string &fscryptOptions);
    static int32_t UpdateKeyContext(uint32_t userId);

private:
    static sptr<IStorageDaemon> GetStorageDaemonProxy(void);
    static bool CheckServiceStatus(uint32_t serviceFlags);
};
} // StorageDaemon
} // OHOS

#endif