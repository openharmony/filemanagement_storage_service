/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_KEYCTL_H_
#define STORAGE_DAEMON_CRYPTO_KEYCTL_H_
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <linux/fs.h>
#include <linux/keyctl.h>
#include <linux/fscrypt.h>

namespace OHOS {
namespace StorageDaemon {
using key_serial_t = int;
constexpr uint32_t CRYPTO_KEY_DESC_SIZE = FSCRYPT_KEY_DESCRIPTOR_SIZE;

enum {
    FSCRYPT_INVALID = 0,
    FSCRYPT_V1 = 1,
    FSCRYPT_V2 = 2,
};

union FscryptPolicy{
    fscrypt_policy_v1 v1;
    fscrypt_policy_v2 v2;
};

class KeyCtrl {
public:
    // ------ fscrypt legacy ------
    static key_serial_t AddKey(const std::string &type, const std::string &description, const key_serial_t ringId);
    static key_serial_t AddKey(const std::string &type, const std::string &description, fscrypt_key &fsKey,
        const key_serial_t ringId);
    static key_serial_t GetKeyring(key_serial_t id, int create);
    static long Revoke(key_serial_t id);
    static long Search(key_serial_t ringId, const std::string &type, const std::string &description,
        key_serial_t destRingId);
    static long SetPermission(key_serial_t id, int permissions);
    static long Unlink(key_serial_t key, key_serial_t keyring);
    static long RestrictKeyring(key_serial_t keyring, const std::string &type, const std::string &restriction);
    static long GetSecurity(key_serial_t key, std::string &buffer);

    // ------ fscrypt v2 ------
    static bool InstallKey(const std::string &mnt, fscrypt_add_key_arg &arg);
    static bool RemoveKey(const std::string &mnt, fscrypt_remove_key_arg &arg);
    static bool GetKeyStatus(const std::string &mnt, fscrypt_get_key_status_arg &arg);

    static bool SetPolicy(const std::string &path, FscryptPolicy &policy);
    static bool GetPolicy(const std::string &path, fscrypt_get_policy_ex_arg &options);
    static bool LoadAndSetPolicy(const std::string &keyPath, const std::string &policyFile,
        const std::string &toEncrypt);

    static uint8_t GetFscryptVersion(const std::string &mnt = "/data");
    static uint8_t GetEncryptedVersion(const std::string &dir);
};

struct EncryptPolicy {
    std::string version;
    std::string fileName;
    std::string content;
    std::string flags;
    bool hwWrappedKey;
};

static const EncryptPolicy DEFAULT_POLICY = {
    .version = "2",
    .fileName = "aes-256-cts",
    .content = "aes-256-xts",
    .flags = "padding-32",
    .hwWrappedKey = false,
};

static const auto CONTENTS_MODES = std::map<std::string, uint8_t> {
    {"aes-256-xts", FSCRYPT_MODE_AES_256_XTS},
    {"aes-128-cbc", FSCRYPT_MODE_AES_128_CBC},
    {"adiantum", FSCRYPT_MODE_ADIANTUM},
};

static const auto FILENAME_MODES = std::map<std::string, uint8_t> {
    {"aes-256-cts", FSCRYPT_MODE_AES_256_CTS},
    {"aes-128-cts", FSCRYPT_MODE_AES_128_CTS},
    {"adiantum", FSCRYPT_MODE_ADIANTUM},
};

static const auto POLICY_FLAGS = std::map<std::string, uint8_t> {
    {"padding-4", FSCRYPT_POLICY_FLAGS_PAD_4},
    {"padding-8", FSCRYPT_POLICY_FLAGS_PAD_8},
    {"padding-16", FSCRYPT_POLICY_FLAGS_PAD_16},
    {"padding-32", FSCRYPT_POLICY_FLAGS_PAD_32},
    {"direct-key", FSCRYPT_POLICY_FLAG_DIRECT_KEY}, // use with adiantum
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYCTL_H_
