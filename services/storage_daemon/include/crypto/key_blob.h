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
#ifndef STORAGE_DAEMON_CRYPTO_KEY_UTILS_H
#define STORAGE_DAEMON_CRYPTO_KEY_UTILS_H

#include <memory>
#include <string>

#include "hks_type.h"
#include "securec.h"

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t CRYPTO_KEY_SECDISC_SIZE = 16384;
constexpr uint32_t CRYPTO_KEY_ALIAS_SIZE = 16;
constexpr uint32_t CRYPTO_AES_AAD_LEN = 16;
constexpr uint32_t CRYPTO_AES_256_XTS_KEY_SIZE = 64;
constexpr uint32_t CRYPTO_KEY_SHIELD_SIZE = 300;
constexpr uint32_t CRYPTO_AES_256_KEY_ENCRYPTED_SIZE = 80;

class KeyBlob {
public:
    KeyBlob() = default;
    ~KeyBlob()
    {
        Clear();
    }
    KeyBlob(uint32_t len)
    {
        Alloc(len);
        // may fail, need check IsEmpty() if needed
    }
    KeyBlob(KeyBlob &&right)
    {
        data = std::move(right.data);
        size = right.size;
    }
    KeyBlob& operator=(KeyBlob &&right)
    {
        data = std::move(right.data);
        size = right.size;
        return *this;
    }

    bool Alloc(uint32_t len)
    {
        if (len > CRYPTO_KEY_SECDISC_SIZE) {
            return false;
        }
        if (!IsEmpty()) {
            Clear();
        }

        data = std::make_unique<uint8_t[]>(len);
        size = len;
        (void)memset_s(data.get(), size, 0, size);
        return true;
    }
    void Clear()
    {
        if (data != nullptr && size != 0) {
            (void)memset_s(data.get(), size, 0, size);
        }
        size = 0;
        data.reset(nullptr);
    }
    bool IsEmpty() const
    {
        return size == 0 || data.get() == nullptr;
    }
    std::string ToString() const
    {
        std::string hex;
        const char *hexMap = "0123456789abcdef";
        static_assert(sizeof(data[0]) == sizeof(char));
        for (size_t i = 0; i < size; i++) {
            hex = hex + hexMap[(data[i] & 0xF0) >> 4] + hexMap[data[i] & 0x0F]; // higher 4 bits
        }
        return hex;
    }
    HksBlob ToHksBlob() const
    {
        return {size, data.get()};
    }
    uint32_t size { 0 };
    std::unique_ptr<uint8_t[]> data { nullptr };
};

struct KeyInfo {
    uint8_t version { 0 };
    KeyBlob key;
    // the legacy interface use key_spec.u.descriptor
    KeyBlob keyDesc;
    // the v2 interface use the key_spec.u.identifier
    KeyBlob keyId;
};

struct KeyContext {
    // secure discardable keyblob
    KeyBlob secDiscard;
    // encrypted huks key for encrypt/decrypt
    KeyBlob shield;
    // encrypted blob of rawkey
    KeyBlob encrypted;
    // aes_gcm tags
    KeyBlob nonce;
    KeyBlob aad;
};

struct UserAuth {
    std::string token;
    // synthetic
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEY_UTILS_H
