/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef STORAGE_DAEMON_CRYPTO_KEYMANAGER_EXT_H
#define STORAGE_DAEMON_CRYPTO_KEYMANAGER_EXT_H

#include <mutex>

#include "nocopyable.h"

#include "base_key.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

class UserkeyExtInterface {
public:
    virtual ~UserkeyExtInterface() = default;
    virtual int32_t GenerateUserKey(int32_t userId, const std::vector<uint8_t>& keyInfo) = 0;
    virtual int32_t ActiveUserKey(int32_t userId, const std::vector<uint8_t>& keyInfo,
        const std::vector<uint8_t>& token) = 0;
    virtual int32_t InactiveUserKey(int32_t userId) = 0;
    virtual int32_t DeleteUserKey(int32_t userId) = 0;
    virtual int32_t SetFilePathPolicy(int32_t userId) = 0;
    virtual int32_t UpdateUserPublicDirPolicy(int32_t userId) = 0;
    virtual int32_t SetRecoverKey(int32_t userId, const std::vector<uint8_t>& keyInfo) = 0;
};

class KeyManagerExt {
public:
    static KeyManagerExt &GetInstance(void)
    {
        static KeyManagerExt instance;
        return instance;
    }

    int GenerateUserKeys(uint32_t userId, uint32_t flags);
    int DeleteUserKeys(uint32_t userId);
    int ActiveUserKey(uint32_t userId, const std::vector<uint8_t>& token,
                      const std::vector<uint8_t>& secret);
    int InActiveUserKey(uint32_t userId);
    int UpdateUserPublicDirPolicy(uint32_t userId);
    int SetRecoverKey(uint32_t userId, uint32_t keyType, const KeyBlob& ivBlob);

private:
    KeyManagerExt();
    ~KeyManagerExt();

    int GetHashKey(uint32_t userId, KeyType type, KeyBlob& hashKey);
    void Init();
    void UnInit();
    int DoDeleteUserKeys(uint32_t userId);
    int DoActiveUserKey(uint32_t userId,
                        const std::vector<uint8_t>& token,
                        const std::vector<uint8_t>& secret);
    int DoInactiveUserKey(uint32_t userId);
    int GenerateAndInstallUserKey(uint32_t userId);
    bool IsServiceExtSoLoaded() { return service_ != nullptr; }

    std::mutex keyMutex_;
    UserkeyExtInterface* service_ = nullptr;
    void *handler_ = nullptr;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYMANAGER_EXT_H
