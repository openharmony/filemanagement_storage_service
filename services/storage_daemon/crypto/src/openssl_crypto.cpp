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
#include "base_key.h"
#include "err.h"
#include "key_manager.h"
#include "key_blob.h"
#include "openssl_crypto.h"
#include "openssl/err.h"
#include <openssl/sha.h>
#include "storage_service_log.h"
#include "third_party/openssl/include/openssl/evp.h"

namespace OHOS {
namespace StorageDaemon {
bool OpensslCrypto::DecryptWithoutHuks(const KeyBlob &preKey, const KeyBlob &cipherText,
                                       KeyBlob &plainText, KeyBlob &shield, KeyBlob &secDiscard)
{
    shield = HashAndClip(preKey, secDiscard, RANDOM_NUMBER_SIZE);
    if (cipherText.size < GCM_NONCE_BYTES + GCM_MAC_BYTES) {
        LOGE("GCM cipherText too small: %{public}u ", cipherText.size);
        return false;
    }
    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!ctx) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL,
                           reinterpret_cast<const uint8_t*>(shield.data.get()),
                           reinterpret_cast<const uint8_t*>(cipherText.data.get())) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    plainText = KeyBlob(cipherText.size - GCM_NONCE_BYTES - GCM_MAC_BYTES);
    int outlen;
    if (EVP_DecryptUpdate(ctx.get(), reinterpret_cast<uint8_t*>(plainText.data.get()), &outlen,
                          reinterpret_cast<const uint8_t*>(cipherText.data.get() + GCM_NONCE_BYTES),
                          plainText.size) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    if (static_cast<int>(plainText.size) != outlen) {
        LOGE("GCM plainText length should be %{private}u, was %{public}d", plainText.size, outlen);
        return false;
    }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, GCM_MAC_BYTES,
                            const_cast<void*>(reinterpret_cast<const void*>(
                            cipherText.data.get() + GCM_NONCE_BYTES + plainText.size))) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    if (EVP_DecryptFinal_ex(ctx.get(),
                            reinterpret_cast<uint8_t*>(plainText.data.get() + plainText.size),
                            &outlen) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    if (outlen != 0) {
        LOGE("GCM EncryptFinal should be 0, was %{public}d ", outlen);
        return false;
    }
    LOGI("Enhance decrypt key success");
    return true;
}

bool OpensslCrypto::EncryptWithoutHuks(const KeyBlob &preKey, const KeyBlob &plainText,
                                       KeyBlob &cipherText, KeyBlob &shield, KeyBlob &secDiscard)
{
    shield = HashAndClip(preKey, secDiscard, RANDOM_NUMBER_SIZE);
    auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!ctx) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    cipherText = KeyBlob(GCM_NONCE_BYTES + plainText.size + GCM_MAC_BYTES);
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL,
                           reinterpret_cast<const uint8_t*>(shield.data.get()),
                           reinterpret_cast<const uint8_t*>(cipherText.data.get())) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    int outlen;
    if (EVP_EncryptUpdate(ctx.get(), reinterpret_cast<uint8_t*>(cipherText.data.get() + GCM_NONCE_BYTES),
                          &outlen, reinterpret_cast<const uint8_t*>(plainText.data.get()), plainText.size) !=
                          OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    if (static_cast<int>(plainText.size) != outlen) {
        LOGE("GCM cipherText length should be %{private}d, was %{public}u", plainText.size, outlen);
        return false;
    }
    if (EVP_EncryptFinal_ex(ctx.get(),
                            reinterpret_cast<uint8_t*>(cipherText.data.get() + GCM_NONCE_BYTES + plainText.size),
                            &outlen) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    if (outlen != 0) {
        LOGE("GCM EncryptFinal should be 0 , was %{public}u", outlen);
        return false;
    }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, GCM_MAC_BYTES,
                            reinterpret_cast<uint8_t*> (cipherText.data.get() +
                            GCM_NONCE_BYTES + plainText.size)) != OPENSSL_SUCCESS_FLAG) {
        LOGE("Openssl error: %{public}lu ", ERR_get_error());
        return false;
    }
    LOGI("Enhance encrypt key success");
    return true;
}

KeyBlob OpensslCrypto::HashAndClip(const KeyBlob &prefix, const KeyBlob &payload, uint32_t length)
{
    KeyBlob res(SHA512_DIGEST_LENGTH);
    SHA512_CTX c;
    SHA512_Init(&c);
    SHA512_Update(&c, prefix.data.get(), prefix.size);
    if (!payload.IsEmpty()) {
        SHA512_Update(&c, payload.data.get(), payload.size);
    }
    SHA512_Final(res.data.get(), &c);

    res.size = length;
    return res;
}

void OpensslCrypto::MkdirVersionCheck(const std::string &pathTemp)
{
    const std::string NEED_UPDATE_PATH = pathTemp + SUFFIX_NEED_UPDATE;
    if (!IsDir(NEED_UPDATE_PATH)) {
        int ret = MkDir(NEED_UPDATE_PATH, 0700);
        if (ret && errno != EEXIST) {
            LOGE("create NEED_UPDATE_PATH dir error");
        }
    }
}

} // namespace StorageDaemon
} // namespace OHOS