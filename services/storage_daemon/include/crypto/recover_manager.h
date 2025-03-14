/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef STORAGE_DAEMON_CRYPTO_KEY_RECOVER_MANAGER_H
#define STORAGE_DAEMON_CRYPTO_KEY_RECOVER_MANAGER_H

#include "fbex.h"
#include "key_blob.h"
#include "libfscrypt/key_control.h"

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
#include "tee_client_api.h"
#endif

namespace OHOS {
namespace StorageDaemon {
constexpr int RND_AND_KEY2_LEN = 64;
constexpr int RND_AND_KEY2_NUMS = 6; // 0-Device EL1, 1-Global User EL1, 2-User EL1, 3-User EL2, 4-User EL3, 5-User EL4
constexpr int AUTH_TOKEN_LEN = 280;

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
static TEEC_UUID recoverTaUuid = { 0x9449c83f, 0x2fa6, 0x485b, { 0x83, 0xff, 0x77, 0x14, 0x4f, 0xff, 0x66, 0xb4 } };
#endif

struct CreateRecoverKeyStr {
    uint32_t userType;
    uint32_t userId;
    uint32_t authTokenLen;
    uint8_t authToken[AUTH_TOKEN_LEN];
    uint8_t rndToTee[RND_AND_KEY2_NUMS][RND_AND_KEY2_LEN];
};

struct SetRecoverKeyStr {
    uint8_t rndFromTee[RND_AND_KEY2_NUMS][RND_AND_KEY2_LEN];
    uint8_t key2FromTee[RND_AND_KEY2_NUMS][RND_AND_KEY2_LEN];
};

enum TaCmdId {
    RK_CMD_ID_GEN_RECOVERY_KEY = 0x1000A003,
    RK_CMD_ID_DECRYPT_CLASS_KEY = 0x1000A008,
};

const size_t DEVICE_EL1 = 0;
const size_t GLOBAL_USER_EL1 = 1;
const size_t USER_EL1 = 2;
const size_t USER_EL2 = 3;
const size_t USER_EL3 = 4;
const size_t USER_EL4 = 5;

class RecoveryManager {
public:
    static RecoveryManager &GetInstance()
    {
        static RecoveryManager instance;
        return instance;
    }

    int CreateRecoverKey(uint32_t userId,
                         uint32_t userType,
                         const std::vector<uint8_t> &token,
                         const std::vector<uint8_t> &secret,
                         const std::vector<KeyBlob> &originIv);
    int SetRecoverKey(const std::vector<uint8_t> &key);

private:
    RecoveryManager();
    ~RecoveryManager();
    RecoveryManager(const RecoveryManager &) = delete;
    RecoveryManager &operator=(const RecoveryManager &) = delete;

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    bool OpenSession(TEEC_Context &context, TEEC_Session &session);
    void CloseSession(TEEC_Context &context, TEEC_Session &session);
#endif

    int SetRecoverKeyToTee(const std::vector<uint8_t> &key, SetRecoverKeyStr &setRecoverKeyStr);
    int32_t GenerateKeyDesc(const KeyBlob &key2Blob, KeyBlob &originKey2);
    int32_t InstallKeyDescToKeyring(size_t keyType, const KeyBlob &key2Blob, const KeyBlob &keyDesc);
    int32_t InstallDeCe(const KeyBlob &key2Blob, const KeyBlob &keyDesc);
    int32_t InstallEceSece(uint32_t sdpClass, const KeyBlob &key2Blob, const KeyBlob &keyDesc);

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    TEEC_UUID *recoverUuid_ = &recoverTaUuid;
#endif

    bool isSessionOpened;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif //STORAGE_DAEMON_CRYPTO_KEY_RECOVER_MANAGER_H
