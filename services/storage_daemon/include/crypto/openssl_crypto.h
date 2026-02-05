/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

namespace OHOS {
namespace StorageDaemon {
constexpr size_t GCM_MAC_BYTES = 16;
constexpr size_t GCM_NONCE_BYTES = 12;
constexpr int AES_256_HASH_RANDOM_SIZE = 32;
constexpr int OPENSSL_SUCCESS_FLAG = 1;
class OpensslCrypto {
public:
    static int32_t AESDecrypt(const KeyBlob &preKey, KeyContext &keyContext_, KeyBlob &plainText);
    static int32_t AESEncrypt(const KeyBlob &preKey, const KeyBlob &plainText, KeyContext &keyContext_);
    static KeyBlob HashWithPrefix(const KeyBlob &prefix, const KeyBlob &payload, uint32_t length);

private:
    static int32_t DoGCMDecryptInit(EVP_CIPHER_CTX *ctx, const KeyBlob &shield,
                                     const KeyContext &keyContext_, KeyBlob &plainText);
    static int32_t DoGCMDecryptFinal(EVP_CIPHER_CTX *ctx, const KeyBlob &cipherText, KeyBlob &plainText);
    static int32_t DoGCMEncryptFinal(EVP_CIPHER_CTX *ctx, KeyBlob &cipherText, const KeyBlob &plainText);
    static void CleanupShield(KeyBlob &shield);
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_CRYPTO_OPENSSL_CRYPTO_H