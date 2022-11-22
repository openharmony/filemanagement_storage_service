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

#ifndef STORAGE_MANAGER_CILENT_H
#define STORAGE_MANAGER_CILENT_H

#include "iremote_proxy.h"
#include "istorage_manager.h"
#include "ipc/storage_manager.h"
#include "ipc/storage_manager_proxy.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerClient {
public:
    static int32_t PrepareAddUser(uint32_t userId, uint32_t flags);
    static int32_t RemoveUser(uint32_t userId, uint32_t flags);
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
    static int32_t UpdateKeyContext(uint32_t userId);

private:
    static sptr<IStorageManager> GetStorageManagerProxy(void);
};
}
}
#endif