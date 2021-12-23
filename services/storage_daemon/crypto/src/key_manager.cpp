/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "key_manager.h"

#include <string>

#include "directory_ex.h"
#include "file_ex.h"

#include "utils/log.h"

namespace OHOS {
namespace StorageDaemon {
const UserAuth NULL_KEY_AUTH = {
    .token = ""
};

int KeyManager::GenerateDeviceKey(const std::string &dir)
{
    globalEl1Key_ = std::make_unique<BaseKey>(dir);

    if (globalEl1Key_->InitKey() == false) {
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (globalEl1Key_->StoreKey(NULL_KEY_AUTH) == false) {
        LOGE("global security key store failed");
        return -EFAULT;
    }

    if (globalEl1Key_->ActiveKey() == false) {
        // should clear saved file ?
        LOGE("global security key active failed");
        return -EFAULT;
    }

    // GetFileEncryptPattern
    // SaveFileEncryptPatternToFile
    return 0;
}

int KeyManager::InitGlobalDeviceKey(void)
{
    LOGD("start");
    return 0;
}

int KeyManager::InitGlobalUserKeys(void)
{
    LOGD("start");
    return 0;
}

int KeyManager::CreateUserKeys(unsigned int user, bool isSave)
{
    LOGD("start, user:%{public}d", user);
    return 0;
}

int KeyManager::DeleteUserKeys(unsigned int user)
{
    LOGD("start, user:%{public}d", user);
    return 0;
}

int KeyManager::UpdateUserAuth(unsigned int user, const std::string &token)
{
    LOGD("start, user:%{public}d", user);
    return 0;
}

int KeyManager::ActiveUserKey(unsigned int user)
{
    LOGD("start");
    return 0;
}

int KeyManager::InActiveUserKey(unsigned int user)
{
    LOGD("start");
    return 0;
}

int KeyManager::UpdateKeyContext(unsigned int user)
{
    LOGD("start");
    return 0;
}

int KeyManager::PrepareUserSpace(unsigned int user)
{
    LOGD("start");
    return 0;
}

std::string KeyManager::GetKeyDesc(unsigned int user)
{
    LOGD("start");
    return "";
}
} // namespace StorageDaemon
} // namespace OHOS
