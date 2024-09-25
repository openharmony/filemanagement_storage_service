/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
const uint32_t USER_UNLOCK = 0x2;
const uint32_t USER_ADD_AUTH = 0x0;
const uint32_t USER_CHANGE_AUTH = 0x1;
const std::string SUFFIX_NEED_UPDATE = "/need_update";
const std::string SUFFIX_NEED_RESTORE = "/need_restore";
const std::vector<uint8_t> NULL_SECRET = { '!' };
class BaseKey : public std::enable_shared_from_this<BaseKey> {
public:
    BaseKey() = delete;
    BaseKey(const std::string &dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE);
    ~BaseKey() = default;

    /* key operations */
    bool InitKey(bool needGenerateKey);
#ifdef USER_CRYPTO_MIGRATE_KEY
    bool StoreKey(const UserAuth &auth, bool needGenerateShield = true);
#else
    bool StoreKey(const UserAuth &auth);
#endif
    bool UpdateKey(const std::string &keypath = "");
    bool RestoreKey(const UserAuth &auth);
    virtual bool ActiveKey(uint32_t flag, const std::string &mnt = MNT_DATA) = 0;
    virtual bool InactiveKey(uint32_t flag, const std::string &mnt = MNT_DATA) = 0;
    virtual bool LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt = MNT_DATA) = 0;
    virtual bool UnlockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt = MNT_DATA) = 0;
    virtual bool GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId) = 0;
    virtual bool DeleteAppkey(const std::string keyId) = 0;
    virtual bool AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status) = 0;
    virtual bool DeleteClassEPinCode(uint32_t userId) = 0;
    virtual bool DecryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status) = 0;
    virtual bool EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status) = 0;
    virtual bool ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId) = 0;
    virtual bool LockUece(bool &isFbeSupport) = 0;
    bool DoRestoreKeyEx(const UserAuth &auth, const std::string &keypath);
    bool EncryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey, KeyBlob &encryptedKey);
    bool DecryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey, KeyBlob &decryptedKey);
    bool RenameKeyPath(const std::string &keyPath);
    bool ClearKey(const std::string &mnt = MNT_DATA);
    void ClearMemoryKeyCtx();
    void WipingActionDir(std::string &path);
    bool UpgradeKeys();
    bool GetOriginKey(KeyBlob &originKey);
    void SetOriginKey(KeyBlob &originKey);
    
    KeyInfo keyInfo_;
    std::string GetDir() const
    {
        return dir_;
    }
    enum class KeyEncryptType {
        KEY_CRYPT_HUKS,
        KEY_CRYPT_OPENSSL,
        KEY_CRYPT_HUKS_OPENSSL
    };
    
protected:
    static bool SaveKeyBlob(const KeyBlob &blob, const std::string &path);
    std::string dir_ {};

private:
#ifdef USER_CRYPTO_MIGRATE_KEY
    bool DoStoreKey(const UserAuth &auth, bool needGenerateShield = true);
#else
    bool DoStoreKey(const UserAuth &auth);
#endif
    bool LoadAndSaveShield(const UserAuth &auth, const std::string &pathShield, bool needGenerateShield,
                           KeyContext &keyCtx);
    bool SaveAndCleanKeyBuff(const std::string &keyPath, KeyContext &keyCtx);
    bool DoRestoreKey(const UserAuth &auth, const std::string &keypath);
    bool DoRestoreKeyCeEceSece(const UserAuth &auth, const std::string &path, const uint32_t keyType);
    bool DoRestoreKeyDe(const UserAuth &auth, const std::string &path);
    bool DoRestoreKeyOld(const UserAuth &auth, const std::string &keypath);
    bool DoUpdateRestore(const UserAuth &auth, const std::string &keyPath);
    static bool GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size);
    static bool GenerateKeyBlob(KeyBlob &blob, const uint32_t size);
    static bool LoadKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size);
    bool EncryptDe(const UserAuth &auth, const std::string &path);
    bool EncryptEceSece(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx);
    bool Decrypt(const UserAuth &auth);
    bool DecryptReal(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx);
    bool CheckAndUpdateVersion();
    bool CombKeyCtx(const KeyBlob &nonce, const KeyBlob &rndEnc, const KeyBlob &aad, KeyBlob &keyOut);
    bool SplitKeyCtx(const KeyBlob &keyIn, KeyBlob &nonce, KeyBlob &rndEnc, KeyBlob &aad);
    void CombKeyBlob(const KeyBlob &encAad, const KeyBlob &end, KeyBlob &keyOut);
    void SplitKeyBlob(const KeyBlob &keyIn, KeyBlob &encAad, KeyBlob &nonce, uint32_t start);
    void ClearKeyContext(KeyContext &keyCtx);
    bool InitKeyContext(const UserAuth &auth, const std::string &keyPath, KeyContext &keyCtx);
    int GetCandidateVersion() const;
    std::string GetCandidateDir() const;
    std::string GetNextCandidateDir() const;
    void SyncKeyDir() const;
    uint32_t GetTypeFromDir();
    uint32_t GetIdFromDir();

    KeyContext keyContext_ {};
    uint8_t keyLen_ {};
    KeyEncryptType keyEncryptType_;
    std::string KeyEncryptTypeToString(KeyEncryptType keyEncryptType_) const;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_BASEKEY_H
