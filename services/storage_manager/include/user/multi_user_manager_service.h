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

#ifndef OHOS_STORAGE_MANAGER_MULTI_USER_MANAGER_SERVICE_H
#define OHOS_STORAGE_MANAGER_MULTI_USER_MANAGER_SERVICE_H

#include <cstdint>

namespace OHOS {
namespace StorageManager {
class MultiUserManagerService {
public:
    static MultiUserManagerService &GetInstance()
    {
        static MultiUserManagerService instance;
        return instance;
    }
    int32_t PrepareAddUser(int32_t userId, uint32_t flags);
    int32_t RemoveUser(int32_t userId, uint32_t flags);
    int32_t PrepareStartUser(int32_t userId);
    int32_t StopUser(int32_t userId);
    int32_t CompleteAddUser(int32_t userId);

private:
    MultiUserManagerService();
    ~MultiUserManagerService();
    MultiUserManagerService(const MultiUserManagerService &) = delete;
    MultiUserManagerService &operator=(const MultiUserManagerService &) = delete;
    MultiUserManagerService(const MultiUserManagerService &&) = delete;
    MultiUserManagerService &operator=(const MultiUserManagerService &&) = delete;
    int32_t CheckUserIdRange(int32_t userId);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_MULTI_USER_MANAGER_SERVICE_H