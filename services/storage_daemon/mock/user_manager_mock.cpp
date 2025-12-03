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

#include "mock/user_manager_mock.h"

namespace OHOS {
namespace StorageDaemon {
UserManager &UserManager::GetInstance()
{
    static UserManager instance_;
    return instance_;
}

int32_t UserManager::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    if (UserManagerMock::iUserManagerMock_ == nullptr) {
        return -1;
    }
    return UserManagerMock::iUserManagerMock_->PrepareUserDirs(userId, flags);
}

int32_t UserManager::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    if (UserManagerMock::iUserManagerMock_ == nullptr) {
        return -1;
    }
    return UserManagerMock::iUserManagerMock_->DestroyUserDirs(userId, flags);
}

void UserManager::CreateElxBundleDataDir(uint32_t userId, uint8_t elx)
{
}

} // namespace StorageDaemon
} // namespace OHOS
