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

#ifndef OHOS_STORAGE_MANAGER_USER_INFO_H
#define OHOS_STORAGE_MANAGER_USER_INFO_H

namespace OHOS {
namespace StorageManager {
enum UserState {
    USER_CREATE,
    USER_START
};

class UserInfo {
public:
    UserInfo(int32_t id, UserState state)
    {
        this->userId_ = id;
        this->state_ = state;
    }

    UserInfo(const UserInfo& userInfo)
    {
        this->userId_ = userInfo.userId_;
        this->state_ = userInfo.state_;
    }

    UserInfo& operator=(const UserInfo& userInfo)
    {
        this->userId_ = userInfo.userId_;
        this->state_ = userInfo.state_;

        return *this;
    }

    UserState GetState()
    {
        return this->state_;
    }

    void SetState(UserState state)
    {
        this->state_ = state;
    }

private:
    int32_t userId_;
    UserState state_;
};
} // STORAGE_MANAGER
} // OHOS

#endif // OHOS_STORAGE_MANAGER_USER_INFO_H