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
#ifndef STORAGE_DAEMON_CRYPTO_KEY_UTILS_H
#define STORAGE_DAEMON_CRYPTO_KEY_UTILS_H

#include <string>
#include <linux/version.h>

#include "hks_type.h"
#ifdef HUKS_IDL_ENVIRONMENT
#include "v1_1/ihuks_types.h"
#endif
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
constexpr uint32_t CRYPTO_AES_256_XTS_KEY_SIZE = 64;

using key_serial_t = int;
constexpr uint32_t CRYPTO_KEY_DESC_SIZE = FSCRYPT_KEY_DESCRIPTOR_SIZE;
constexpr const char *MNT_DATA = "/data";
constexpr const char *PATH_LATEST = "/latest";
constexpr const char *PATH_SHIELD = "/shield";
constexpr const char *PATH_SECDISC = "/sec_discard";
constexpr const char *PATH_ENCRYPTED = "/encrypted";
constexpr const char *PATH_KEYID = "/key_id";
constexpr const char *PATH_KEYDESC = "/key_desc";

constexpr const char *DATA_EL0_DIR = "/data/service/el0";
constexpr const char *STORAGE_DAEMON_DIR = "/data/service/el0/storage_daemon";
constexpr const char *DEVICE_EL1_DIR = "/data/service/el0/storage_daemon/sd";
constexpr const char *MAINTAIN_DEVICE_EL1_DIR = "/mnt/data_old/service/el0/storage_daemon/sd";

class KeyBlob {
public:
    KeyBlob() = default;
    KeyBlob(KeyBlob const &blob)
    {
        Alloc(blob.size);
        if (!blob.data) {
            Clear();
            return;
        }
        auto ret = memcpy_s(data.get(), size, blob.data.get(), blob.size);
        if (ret != EOK) {
            Clear();
        }
    }
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
        if (IsEmpty()) {
            return hex;
        }
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
#ifdef HUKS_IDL_ENVIRONMENT
    HuksBlob ToHuksBlob() const
    {
        return {data.get(), size};
    }
#endif
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
    // SHA3-512 hash code: key and specify a prefix string
    KeyBlob keyHash;
};

struct KeyContext {
    // secure discardable keyblob
    KeyBlob secDiscard;
    // encrypted huks key for encrypt/decrypt
    KeyBlob shield;
    // encrypted blob of rawkey
    KeyBlob rndEnc;
    // aes_gcm tags
    KeyBlob nonce;
    KeyBlob aad;
};

struct UserAuth {
    // when secure access enabled, token is needed to authenticate the user
    KeyBlob token;
    KeyBlob secret;
    uint64_t secureUid { 0 };
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEY_UTILS_H
