/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "openssl_crypto.h"
#include "storage_service_constant.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *SUFFIX_NEED_UPDATE = "/need_update";
constexpr const char *SUFFIX_NEED_RESTORE = "/need_restore";
constexpr const char *PATH_KEY_VERSION = "/version_";
enum UpdateVersion {
    UPDATE_V2 = 2,
    UPDATE_V4 = 4
};

class BaseKey : public std::enable_shared_from_this<BaseKey> {
public:
    BaseKey() = delete;
    BaseKey(const std::string &dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE);
    ~BaseKey() = default;

    /* key operations */
    bool InitKey(bool needGenerateKey);
#ifdef USER_CRYPTO_MIGRATE_KEY
    int32_t StoreKey(const UserAuth &auth, bool needGenerateShield = true);
#else
    int32_t StoreKey(const UserAuth &auth);
#endif
    int32_t UpdateKey(const std::string &keypath = "", bool needSyncCandidate = true);
    int32_t RestoreKey(const UserAuth &auth, bool needSyncCandidate = true);
    int32_t RestoreKey4Nato(const std::string &keyDir, KeyType type);
    virtual int32_t ActiveKey(const KeyBlob &authToken, uint32_t flag,
        const std::string &mnt = std::string(MNT_DATA)) = 0;
    virtual int32_t InactiveKey(uint32_t flag, const std::string &mnt = std::string(MNT_DATA)) = 0;
    virtual int32_t LockUserScreen(uint32_t flag, uint32_t sdpClass,
        const std::string &mnt = std::string(MNT_DATA)) = 0;
    virtual int32_t UnlockUserScreen(const KeyBlob &authToken, uint32_t flag, uint32_t sdpClass,
        const std::string &mnt = std::string(MNT_DATA)) = 0;
    virtual int32_t GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId) = 0;
    virtual int32_t DeleteAppkey(const std::string keyId) = 0;
    virtual int32_t AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status) = 0;
    virtual int32_t DeleteClassEPinCode(uint32_t userId) = 0;
    virtual int32_t DecryptClassE(const UserAuth &auth, bool &isSupport, bool &eBufferStatue, uint32_t user,
                               bool needSyncCandidate) = 0;
    virtual int32_t EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status) = 0;
    virtual int32_t ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId) = 0;
    virtual int32_t UpdateClassEBackUp(uint32_t userId) = 0;
    virtual int32_t LockUece(bool &isFbeSupport) = 0;
    int32_t DoRestoreKey(const UserAuth &auth, const std::string &keypath);
    int32_t EncryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey, KeyBlob &encryptedKey);
    int32_t DecryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey, KeyBlob &decryptedKey);
    bool RenameKeyPath(const std::string &keyPath);
    bool ClearKey(const std::string &mnt = std::string(MNT_DATA));
    void WipingActionDir(std::string &path);
    bool UpgradeKeys();
    bool GetOriginKey(KeyBlob &originKey);
    void SetOriginKey(KeyBlob &originKey);
    bool KeyDescIsEmpty();
    std::string GetKeyDir();
    bool GetHashKey(KeyBlob &hashKey);
    bool GenerateHashKey();
    void ClearKeyInfo();

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
    static bool LoadKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size = 0);
    std::string GetCandidateDir() const;
    std::string dir_ {};

private:
#ifdef USER_CRYPTO_MIGRATE_KEY
    int32_t  DoStoreKey(const UserAuth &auth, bool needGenerateShield = true);
#else
    int32_t  DoStoreKey(const UserAuth &auth);
#endif
    int32_t LoadAndSaveShield(const UserAuth &auth, const std::string &pathShield, bool needGenerateShield,
                           KeyContext &keyCtx);
    bool SaveAndCleanKeyBuff(const std::string &keyPath, KeyContext &keyCtx);
    int32_t DoRestoreKeyCeEceSece(const UserAuth &auth, const std::string &path, const uint32_t keyType);
    int32_t DoRestoreKeyDe(const UserAuth &auth, const std::string &path);
    int32_t DoRestoreKeyOld(const UserAuth &auth, const std::string &keypath);
    int32_t DoUpdateRestore(const UserAuth &auth, const std::string &keyPath);
    int32_t DoUpdateRestoreVx(const UserAuth &auth, const std::string &KeyPath, UpdateVersion update_version);
    static bool GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size);
    static bool GenerateKeyBlob(KeyBlob &blob, const uint32_t size);
    int32_t EncryptDe(const UserAuth &auth, const std::string &path);
    int32_t EncryptEceSece(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx);
    int32_t Decrypt(const UserAuth &auth);
    int32_t DecryptReal(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx);
    bool CheckAndUpdateVersion();
    bool CombKeyCtx(const KeyBlob &nonce, const KeyBlob &rndEnc, const KeyBlob &aad, KeyBlob &keyOut);
    bool SplitKeyCtx(const KeyBlob &keyIn, KeyBlob &nonce, KeyBlob &rndEnc, KeyBlob &aad);
    void CombKeyBlob(const KeyBlob &encAad, const KeyBlob &end, KeyBlob &keyOut);
    void SplitKeyBlob(const KeyBlob &keyIn, KeyBlob &encAad, KeyBlob &nonce, uint32_t start);
    void ClearKeyContext(KeyContext &keyCtx);
    int32_t InitKeyContext(const UserAuth &auth, const std::string &keyPath, KeyContext &keyCtx);
    int GetCandidateVersion() const;
    std::string GetNextCandidateDir() const;
    void SyncKeyDir() const;
    uint32_t GetTypeFromDir();
    uint32_t GetIdFromDir();
    int32_t UpdateOrRollbackKey(const std::string &candidate);

    KeyContext keyContext_ {};
    uint8_t keyLen_ {};
    KeyEncryptType keyEncryptType_;
    std::string KeyEncryptTypeToString(KeyEncryptType keyEncryptType_) const;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_BASEKEY_H
