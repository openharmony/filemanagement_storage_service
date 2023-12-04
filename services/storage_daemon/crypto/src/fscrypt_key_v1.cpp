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

#include "fscrypt_key_v1.h"

#include <openssl/sha.h>

#include "libfscrypt/key_control.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
static const std::string CRYPTO_NAME_PREFIXES[] = {"ext4", "f2fs", "fscrypt"};

bool FscryptKeyV1::ActiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    LOGD("enter");
    if (!GenerateKeyDesc()) {
        LOGE("GenerateKeyDesc failed");
        return false;
    }

    if (!fscryptV1Ext.ActiveKeyExt(flag, keyInfo_.key.data.get(), keyInfo_.key.size)) {
        LOGE("fscryptV1Ext ActiveKeyExtfailed");
        return false;
    }
    if (!InstallKeyToKeyring()) {
        LOGE("InstallKeyToKeyring failed");
        return false;
    }
    LOGD("success");
    return true;
}

bool FscryptKeyV1::InstallKeyToKeyring()
{
    fscrypt_key fskey;
    fskey.mode = FS_ENCRYPTION_MODE_AES_256_XTS;
    fskey.size = keyInfo_.key.size;
    auto err = memcpy_s(fskey.raw, FS_MAX_KEY_SIZE, keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return false;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("failed to add session keyring");
            return false;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks =
            KeyCtrlAddKeyEx("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey %{public}s into keyring %{public}d, errno %{public}d", keyref.c_str(), krid,
                errno);
        }
    }
    if (!SaveKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        return false;
    }
    keyInfo_.key.Clear();
    LOGD("success");
    return true;
}

bool FscryptKeyV1::InactiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    LOGD("enter");
    bool ret = true;

    if (!UninstallKeyToKeyring()) {
        LOGE("UninstallKeyToKeyring failed");
        ret = false;
    }
    if (!fscryptV1Ext.InactiveKeyExt(flag)) {
        LOGE("fscryptV1Ext InactiveKeyExt failed");
        ret = false;
    }
    LOGD("finish");
    return ret;
}

bool FscryptKeyV1::UninstallKeyToKeyring()
{
    if (keyInfo_.keyDesc.IsEmpty()) {
        LOGE("keyDesc is null, key not installed?");
        return false;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGE("Error searching session keyring for fscrypt-provisioning key for fscrypt");
        return false;
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks = KeyCtrlSearch(krid, "logon", keyref.c_str(), 0);
        if (KeyCtrlUnlink(ks, krid) != 0) {
            LOGE("Failed to unlink key with serial %{public}d ref %{public}s", krid, keyref.c_str());
        }
    }

    LOGD("success");
    return true;
}

bool FscryptKeyV1::GenerateKeyDesc()
{
    if (keyInfo_.key.IsEmpty()) {
        LOGE("key is empty");
        return false;
    }
    SHA512_CTX c;

    SHA512_Init(&c);
    SHA512_Update(&c, keyInfo_.key.data.get(), keyInfo_.key.size);
    uint8_t keyRef1[SHA512_DIGEST_LENGTH] = { 0 };
    SHA512_Final(keyRef1, &c);

    SHA512_Init(&c);
    SHA512_Update(&c, keyRef1, SHA512_DIGEST_LENGTH);
    uint8_t keyRef2[SHA512_DIGEST_LENGTH] = { 0 };
    SHA512_Final(keyRef2, &c);

    static_assert(SHA512_DIGEST_LENGTH >= CRYPTO_KEY_DESC_SIZE, "Hash too short for descriptor");
    keyInfo_.keyDesc.Alloc(CRYPTO_KEY_DESC_SIZE);
    auto err = memcpy_s(keyInfo_.keyDesc.data.get(), keyInfo_.keyDesc.size, keyRef2, CRYPTO_KEY_DESC_SIZE);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return false;
    }
    return true;
}
} // namespace StorageDaemon
} // namespace OHOS
