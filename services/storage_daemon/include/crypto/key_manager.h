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

#include "key_blob.h"
#include "base_key.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t GLOBAL_USER_ID = 0;

static const std::string EL1 = "el1";
static const std::string EL2 = "el2";
enum KeyType {
    EL1_KEY = 1,
    EL2_KEY = 2,
};
static std::map<std::string, KeyType> EL_DIR_MAP = {
    {EL1, EL1_KEY},
    {EL2, EL2_KEY},
};

class KeyManager {
public:
    static KeyManager *GetInstance(void)
    {
        static KeyManager instance;
        return &instance;
    }
    int InitGlobalDeviceKey(void);
    int InitGlobalUserKeys(void);
    int GenerateUserKeys(unsigned int user, uint32_t flags);
    int DeleteUserKeys(unsigned int user);
    int UpdateUserAuth(unsigned int user,
                       const std::vector<uint8_t> &token,
                       const std::vector<uint8_t> &oldSecret,
                       const std::vector<uint8_t> &newSecret);
    int ActiveUserKey(unsigned int user, const std::vector<uint8_t> &token,
                      const std::vector<uint8_t> &secret);
    int InActiveUserKey(unsigned int user);
    int SetDirectoryElPolicy(unsigned int user, KeyType type,
                             const std::vector<FileList> &vec);
    int UpdateKeyContext(uint32_t userId);

private:
    KeyManager()
    {
        hasGlobalDeviceKey_ = false;
    }
    ~KeyManager() {}
    int GenerateAndInstallDeviceKey(const std::string &dir);
    int RestoreDeviceKey(const std::string &dir);
    int GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type);
    int RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type);
    int LoadAllUsersEl1Key(void);
    int InitUserElkeyStorageDir(void);
    bool HasElkey(uint32_t userId, KeyType type);
    int DoDeleteUserKeys(unsigned int user);
    std::shared_ptr<BaseKey> GetBaseKey(const std::string& dir);

    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl1Key_;
    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl2Key_;
    std::shared_ptr<BaseKey> globalEl1Key_ { nullptr };

    std::mutex keyMutex_;
    bool hasGlobalDeviceKey_;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYMANAGER_H
