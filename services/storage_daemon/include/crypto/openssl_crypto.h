/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_OPENSSL_CRYPTO_H
#define STORAGE_DAEMON_CRYPTO_OPENSSL_CRYPTO_H

#include "key_blob.h"

namespace OHOS {
namespace StorageDaemon {
class OpensslCrypto {
public:
    static OpensslCrypto &GetInstance()
    {
        static OpensslCrypto instance;
        return instance;
    }
    bool DecryptWithoutHuks(const KeyBlob &preKey, const KeyBlob &cipherText,
                            KeyBlob &plainText, KeyBlob &shield, KeyBlob &secDiscard);
    bool EncryptWithoutHuks(const KeyBlob &preKey, const KeyBlob &plainText,
                            KeyBlob &cipherText, KeyBlob &shield, KeyBlob &secDiscard);
    KeyBlob HashAndClip(const KeyBlob &prefix, const KeyBlob &payload, uint32_t length);
    void MkdirVersionCheck(const std::string &pathtemp);
    enum KeyEncryptType {
        KEY_CRYPT_HUKS,
        KEY_CRYPT_OPENSSL
    };
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_CRYPTO_OPENSSL_CRYPTO_H