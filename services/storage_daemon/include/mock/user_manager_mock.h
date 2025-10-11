/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_USER_MANAGER_MOCK_H
#define STORAGE_DAEMON_USER_MANAGER_MOCK_H

#include <gmock/gmock.h>

#include "user/user_manager.h"

namespace OHOS {
namespace StorageDaemon {
class IUserManagerMock {
public:
    virtual ~IUserManagerMock() = default;
public:
    virtual int32_t PrepareUserDirs(int32_t userId, uint32_t flags) = 0;
    virtual int32_t DestroyUserDirs(int32_t userId, uint32_t flags) = 0;

public:
    static inline std::shared_ptr<IUserManagerMock> iUserManagerMock_ = nullptr;
};

class UserManagerMock : public IUserManagerMock {
public:
    MOCK_METHOD(int32_t, PrepareUserDirs, (int32_t, uint32_t));
    MOCK_METHOD(int32_t, DestroyUserDirs, (int32_t, uint32_t));
};
}
}
#endif // STORAGE_DAEMON_USER_MANAGER_MOCK_H