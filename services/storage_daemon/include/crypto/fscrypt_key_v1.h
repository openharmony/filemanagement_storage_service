/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV1_H
#define STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV1_H

#include "base_key.h"
#include "fscrypt_key_v1_ext.h"
#include "libfscrypt/key_control.h"
#include "fbex.h"

namespace OHOS {
namespace StorageDaemon {
class FscryptKeyV1 final : public BaseKey {
public:
    FscryptKeyV1() = delete;
    FscryptKeyV1(const std::string &dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE) : BaseKey(dir, keyLen)
    {
        keyInfo_.version = FSCRYPT_V1;
        fscryptV1Ext.SetDir(dir);
    }
    ~FscryptKeyV1() = default;

    int32_t ActiveKey(const KeyBlob &authToken, uint32_t flag = 0, const std::string &mnt = std::string(MNT_DATA));
    int32_t InactiveKey(uint32_t flag = 0, const std::string &mnt = std::string(MNT_DATA));
    int32_t LockUserScreen(uint32_t flag = 0, uint32_t sdpClass = 0, const std::string &mnt = std::string(MNT_DATA));
    int32_t UnlockUserScreen(const KeyBlob &authToken, uint32_t flag = 0, uint32_t sdpClass = 0,
        const std::string &mnt = std::string(MNT_DATA));
    int32_t GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId);
    int32_t DeleteAppkey(const std::string keyId);
    void DropCachesIfNeed();
    int32_t AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status = 0);
    int32_t DeleteClassEPinCode(uint32_t userId = 0);
    int32_t ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId = 0);
    int32_t DecryptClassE(const UserAuth &auth, bool &isSupport, bool &eBufferStatue, uint32_t user = 0,
                          bool needSyncCandidate = true);
    int32_t EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user = 0, uint32_t status = 0);
    int32_t LockUece(bool &isFbeSupport);

private:
    int32_t GenerateKeyDesc();
    int32_t InstallKeyToKeyring();
    int32_t InstallEceSeceKeyToKeyring(uint32_t sdpClass);
    int32_t UninstallKeyToKeyring();
    int32_t InstallKeyForAppKeyToKeyring(KeyBlob &appKey);
    int32_t UninstallKeyForAppKeyToKeyring(const std::string keyId);
    int32_t GenerateAppKeyDesc(KeyBlob appKey);
    int32_t DoDecryptClassE(const UserAuth &auth, KeyBlob &eSecretFBE, KeyBlob &decryptedKey,
                         bool needSyncCandidate = true);
    FscryptKeyV1Ext fscryptV1Ext;
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV1_H
