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
#include <vector>
#include <linux/keyctl.h>
#include <linux/version.h>

#include "hks_type.h"
#include "securec.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/fscrypt.h>
#define SUPPORT_FSCRYPT_V2
#else
#include "libfscrypt/fscrypt_uapi.h"
#endif

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t CRYPTO_KEY_SECDISC_SIZE = 16384;
constexpr uint32_t CRYPTO_KEY_ALIAS_SIZE = 16;
constexpr uint32_t CRYPTO_AES_AAD_LEN = 16;
constexpr uint32_t CRYPTO_AES_NONCE_LEN = 64;
constexpr uint32_t CRYPTO_AES_256_XTS_KEY_SIZE = 64;
constexpr uint32_t CRYPTO_KEY_SHIELD_MAX_SIZE = 2048;
constexpr uint32_t CRYPTO_AES_256_KEY_ENCRYPTED_SIZE = 80;
constexpr uint32_t CRYPTO_TOKEN_SIZE = TOKEN_CHALLENGE_LEN; // 32

using key_serial_t = int;
constexpr uint32_t CRYPTO_KEY_DESC_SIZE = FSCRYPT_KEY_DESCRIPTOR_SIZE;
static const std::string MNT_DATA = "/data";
static const std::string PATH_LATEST = "/latest";
static const std::string PATH_SHIELD = "/shield";
static const std::string PATH_SECDISC = "/sec_discard";
static const std::string PATH_ENCRYPTED = "/encrypted";
static const std::string PATH_KEYID = "/key_id";
static const std::string PATH_KEYDESC = "/key_desc";

const std::string DATA_EL0_DIR = std::string() + "/data/service/el0";
const std::string STORAGE_DAEMON_DIR = DATA_EL0_DIR + "/storage_daemon";
const std::string DEVICE_EL1_DIR = STORAGE_DAEMON_DIR + "/sd";

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
    KeyBlob(const std::vector<uint8_t> &vec)
    {
        if (Alloc(vec.size())) {
            auto ret = memcpy_s(data.get(), size, vec.data(), vec.size());
            if (ret != EOK) {
                Clear();
            }
        }
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
    // when secure access enabled, token is needed to authenticate the user
    KeyBlob token;
    KeyBlob secret;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEY_UTILS_H
