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

#include <string>
#include <memory>

#include "securec.h"
#include "hks_type.h"

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t CRYPTO_KEY_NONCE_SIZE = 16384;
constexpr uint32_t CRYPTO_KEY_SALT_SIZE = 16;
constexpr uint32_t CRYPTO_KEY_ALIAS_SIZE = 8;
constexpr uint32_t CRYPTO_AES_256_LEN = 256;
constexpr uint32_t CRYPTO_AES_AAD_LEN = 16;
constexpr uint32_t FS_AES_256_XTS_KEY_SIZE = 64;
static const std::string CRYPTO_NAME_PREFIXES[] = {"ext4", "f2fs", "fscrypt"};

struct KeyBlob {
    bool Alloc(uint32_t len)
    {
        if (len > CRYPTO_KEY_NONCE_SIZE) {
            return false;
        }
        data = std::make_unique<uint8_t[]>(len);
        size = len;
        memset_s(data.get(), size, 0, size);
        return true;
    }
    void Clear()
    {
        memset_s(data.get(), size, 0, size);
        size = 0;
        data.reset(nullptr);
    }
    bool IsEmpty() const
    {
        return size == 0 || data == nullptr;
    }
    std::string ToString() const
    {
        std::string hex;
        const char *hexMap = "0123456789abcdef";
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
    KeyBlob key;
    KeyBlob keyDesc;
};

struct KeyContext {
    std::string version;
    KeyBlob nonce;
    KeyBlob salt; // used when have password
    KeyBlob encrypted;
};

struct EncryptMode {
    std::string fileName;
    std::string content;
};

struct UserAuth {
    std::string token;
    std::string secret;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEY_UTILS_H
