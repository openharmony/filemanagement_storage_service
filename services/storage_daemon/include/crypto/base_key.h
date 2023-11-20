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
#ifndef STORAGE_DAEMON_CRYPTO_BASEKEY_H
#define STORAGE_DAEMON_CRYPTO_BASEKEY_H

#include <string>

#include "key_blob.h"
#include "openssl_crypto.h"

namespace OHOS {
namespace StorageDaemon {
const uint8_t RETRIEVE_KEY = 0x0;
const uint8_t FIRST_CREATE_KEY = 0x6c;
const uint8_t USER_LOGOUT = 0x0;
const uint8_t USER_DESTROY = 0x1;
constexpr size_t GCM_MAC_BYTES = 16;
constexpr size_t GCM_NONCE_BYTES = 12;
const int RANDOM_NUMBER_SIZE = 32;
const int OPENSSL_SUCCESS_FLAG = 1;
class BaseKey {
public:
    BaseKey() = delete;
    BaseKey(const std::string &dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE);
    ~BaseKey() = default;

    /* key operations */
    bool InitKey();
#ifdef USER_CRYPTO_MIGRATE_KEY
    bool StoreKey(const UserAuth &auth, bool needGenerateShield = true);
#else
    bool StoreKey(const UserAuth &auth);
#endif
    bool UpdateKey(const std::string &keypath = "");
    bool RestoreKey(const UserAuth &auth);
    virtual bool ActiveKey(uint32_t flag, const std::string &mnt = MNT_DATA) = 0;
    virtual bool InactiveKey(uint32_t flag, const std::string &mnt = MNT_DATA) = 0;
    bool ClearKey(const std::string &mnt = MNT_DATA);
    bool UpgradeKeys();
    KeyInfo keyInfo_;
    OpensslCrypto::KeyEncryptType keyEncryptType_;
    std::string GetDir() const
    {
        return dir_;
    }

protected:
    static bool SaveKeyBlob(const KeyBlob &blob, const std::string &path);
    std::string dir_ {};

private:
#ifdef USER_CRYPTO_MIGRATE_KEY
    bool DoStoreKey(const UserAuth &auth, bool needGenerateShield = true);
#else
    bool DoStoreKey(const UserAuth &auth);
#endif
#ifdef USER_CRYPTO_MIGRATE_KEY
    bool LoadAndSaveShield(const UserAuth &auth, const std::string &pathTemp, bool needGenerateShield = true);
#else
    bool LoadAndSaveShield(const UserAuth &auth, const std::string &pathTemp);
#endif
    bool DoRestoreKey(const UserAuth &auth, const std::string &keypath);
    static bool GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size);
    static bool GenerateKeyBlob(KeyBlob &blob, const uint32_t size);
    static bool LoadKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size);
    bool Encrypt(const UserAuth &auth);
    bool Decrypt(const UserAuth &auth);
    bool LoadAndSaveStringToFile();
    int GetCandidateVersion() const;
    std::string GetCandidateDir() const;
    std::string GetNextCandidateDir() const;
    void SyncKeyDir() const;

    KeyContext keyContext_ {};
    uint8_t keyLen_ {};
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_BASEKEY_H
