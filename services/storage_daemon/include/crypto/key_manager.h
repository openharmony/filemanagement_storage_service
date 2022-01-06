/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef STORAGE_DAEMON_CRYPTO_KEYMANAGER_H
#define STORAGE_DAEMON_CRYPTO_KEYMANAGER_H

#include <iostream>
#include <map>
#include <memory>
#include <mutex>

#include "key_utils.h"
#include "base_key.h"

namespace OHOS {
namespace StorageDaemon {
class KeyManager {
public:
    static KeyManager *GetInstance(void)
    {
        static KeyManager instance;
        return &instance;
    }
    int InitGlobalDeviceKey(void);
    int InitGlobalUserKeys(void);
    int CreateUserKeys(unsigned int user, bool isSave);
    int DeleteUserKeys(unsigned int user);
    int UpdateUserAuth(unsigned int user, const std::string &token);
    int ActiveUserKey(unsigned int user);
    int InActiveUserKey(unsigned int user);
    int UpdateKeyContext(unsigned int user);
    std::string GetKeyDesc(unsigned int user);
    int PrepareUserSpace(unsigned int user);

private:
    KeyManager()
    {
        hasGlobalDeviceKey_ = false;
    }
    ~KeyManager() {}
    int GenerateDeviceKey(const std::string &dir);

    std::map<unsigned int, std::unique_ptr<BaseKey>> userEl1Key_;
    std::map<unsigned int, std::unique_ptr<BaseKey>> userEl2Key_;
    std::unique_ptr<BaseKey> globalEl1Key_ { nullptr };

    std::mutex keyMutex_;
    bool hasGlobalDeviceKey_;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYMANAGER_H
