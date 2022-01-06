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
#include "base_key.h"

#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "directory_ex.h"
#include "utils/log.h"
#include "huks_master.h"
#include "key_ctrl.h"

namespace OHOS {
namespace StorageDaemon {
static bool g_isHuksMasterInit = false;
BaseKey::BaseKey(std::string dir, uint8_t keyLen) : dir_(dir), keyLen_(keyLen)
{
    if (!g_isHuksMasterInit && (HuksMaster::Init() == 0)) {
        g_isHuksMasterInit = true;
    }
}

bool BaseKey::InitKey()
{
    LOGD("enter");
    if (!keyInfo_.key.IsEmpty()) {
        LOGE("key is not empty");
        return false;
    }
    if (!GenerateKeyBlob(keyInfo_.key, keyLen_)) {
        LOGE("GenerateKeyBlob failed");
        return false;
    }
    if (!GenerateKeyDesc()) {
        LOGE("GenerateKeyDesc failed");
        return false;
    }
    if (!HuksMaster::GenerateKey(keyInfo_.keyDesc)) {
        LOGE("HuksMaster::GenerateKey failed");
        return false;
    }

    return true;
}

bool BaseKey::GenerateKeyBlob(KeyBlob &blob, const uint32_t size)
{
    if (!blob.Alloc(size)) {
        return false;
    }
    if (!HuksMaster::GenerateRandomKey(blob)) {
        blob.Clear();
        return false;
    }
    return true;
}

bool BaseKey::SaveKeyBlob(const KeyBlob &blob, const std::string &name)
{
    LOGD("enter %{public}s, size=%{public}d", name.c_str(), blob.size);
    if (blob.IsEmpty()) {
        return false;
    }
    std::string path = dir_ + "/" + name;
    std::ofstream file(path, std::ios::binary);
    if (file.fail()) {
        LOGE("path:%{public}s fail", path.c_str());
        return false;
    }
    file.write(reinterpret_cast<char *>(blob.data.get()), blob.size);
    file.flush();
    return true;
}

bool BaseKey::GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &name, const uint32_t size)
{
    if (!GenerateKeyBlob(blob, size)) {
        return false;
    }
    return SaveKeyBlob(blob, name);
}

bool BaseKey::LoadKeyBlob(KeyBlob &blob, const std::string &name, const uint32_t size = 0)
{
    LOGD("enter %{public}s, size=%{public}d", name.c_str(), size);
    std::string path = dir_ + "/" + name;
    std::ifstream file(path, std::ios::binary);
    if (file.fail()) {
        LOGE("path:%{public}s fail", path.c_str());
        return false;
    }

    file.seekg(0, std::ios::end);
    uint32_t length = file.tellg();
    // zero size means use the file length.
    if ((size != 0) && (length != size)) {
        LOGE("file:%{public}s size error, real len %{public}d !=  expected %{public}d", name.c_str(), length, size);
        return false;
    }
    if (!blob.Alloc(length)) {
        return false;
    }

    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char *>(blob.data.get()), length)) {
        LOGE("read fail"); // print what?
        return false;
    }
    return true;
}

bool BaseKey::GenerateKeyDesc()
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

    static_assert(SHA512_DIGEST_LENGTH >= CRYPTO_KEY_ALIAS_SIZE, "Hash too short for descriptor");
    keyInfo_.keyDesc.Alloc(CRYPTO_KEY_ALIAS_SIZE);
    auto err = memcpy_s(keyInfo_.keyDesc.data.get(), keyInfo_.keyDesc.size, keyRef2, CRYPTO_KEY_ALIAS_SIZE);
    if (err) {
        LOGE("memcpy failed ret %{public}d", err);
        return false;
    }
    return true;
}

bool BaseKey::StoreKey(const UserAuth &auth)
{
    LOGD("enter");
    std::string originalDir = dir_;
    dir_ += ".tmp";
    if (DoStoreKey(auth)) {
        /*
         * if originalDir
         * del original alias of key, and alias should be different.
         */
        OHOS::ForceRemoveDirectory(originalDir);
        if (rename(dir_.c_str(), originalDir.c_str()) != 0) {
            LOGE("rename fail return %{public}d", errno);
            OHOS::ForceRemoveDirectory(dir_);
            dir_ = originalDir;
            return false;
        }
    }
    dir_ = originalDir;
    return true;
}

bool BaseKey::DoStoreKey(const UserAuth &auth)
{
    OHOS::ForceCreateDirectory(dir_);
    if (!SaveKeyBlob(keyInfo_.keyDesc, "alias")) {
        return false;
    }
    if (!GenerateAndSaveKeyBlob(keyContext_.secDiscard, "sec_discard", CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob sec_discard failed");
        return false;
    }
    if (!EncryptKey(auth)) {
        return false;
    }
    if (!SaveKeyBlob(keyContext_.encrypted, "encrypted")) {
        return false;
    }
    LOGD("encrypted len:%{public}d, data(hex):%{private}s", keyContext_.encrypted.size,
        keyContext_.encrypted.ToString().c_str());
    return true;
}

bool BaseKey::EncryptKey(const UserAuth &auth)
{
    auto ret = HuksMaster::EncryptKey(keyContext_, auth, keyInfo_);
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    return ret;
}

bool BaseKey::RestoreKey(const UserAuth &auth)
{
    LOGD("enter");
    if (!LoadKeyBlob(keyContext_.encrypted, "encrypted")) {
        return false;
    }
    LOGD("encrypted len:%{public}d, data(hex):%{private}s", keyContext_.encrypted.size,
        keyContext_.encrypted.ToString().c_str());

    if (!LoadKeyBlob(keyInfo_.keyDesc, "alias", CRYPTO_KEY_ALIAS_SIZE)) {
        keyInfo_.keyDesc.Clear();
        return false;
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, "sec_discard", CRYPTO_KEY_SECDISC_SIZE)) {
        keyContext_.encrypted.Clear();
        keyInfo_.keyDesc.Clear();
        return false;
    }
    return DecryptKey(auth);
}

bool BaseKey::DecryptKey(const UserAuth &auth)
{
    auto ret = HuksMaster::DecryptKey(keyContext_, auth, keyInfo_);
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();

    return ret;
}

bool BaseKey::ActiveKey()
{
    LOGD("enter");
    if (keyInfo_.keyDesc.IsEmpty()) {
        LOGE("keyDesc is null");
        return false;
    }
    if (keyInfo_.key.IsEmpty()) {
        LOGE("rawkey is null");
        return false;
    }

    fscrypt_key fskey;
    fskey.mode = FS_ENCRYPTION_MODE_AES_256_XTS;
    fskey.size = keyInfo_.key.size;
    auto err = memcpy_s(fskey.raw, FS_MAX_KEY_SIZE, keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err) {
        LOGE("memcpy failed ret %{public}d", err);
        return false;
    }

    key_serial_t krid = KeyCtrl::Search(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrl::AddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("failed to add session keyring");
            return false;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks =
            KeyCtrl::AddKey("logon", keyref, fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey %{public}s into keyring %{public}d, errno %{public}d", keyref.c_str(), krid,
                errno);
        }
    }
    keyInfo_.key.Clear();
    LOGD("success");
    return true;
}

bool BaseKey::ClearKey()
{
    LOGD("enter");
    if (keyInfo_.keyDesc.IsEmpty()) {
        LOGE("keyDesc is null, key not installed?");
        return false;
    }
    HuksMaster::DeleteKey(keyInfo_.keyDesc);

    key_serial_t krid = KeyCtrl::Search(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGE("Error searching session keyring for fscrypt-provisioning key for fscrypt");
        return false;
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks = KeyCtrl::Search(krid, "logon", keyref, 0);
        if (KeyCtrl::Unlink(ks, krid) != 0) {
            LOGE("Failed to unlink key with serial %{public}d ref %{public}s", krid, keyref.c_str());
        }
    }

    OHOS::ForceRemoveDirectory(dir_);
    keyInfo_.key.Clear();
    keyInfo_.keyDesc.Clear();
    LOGD("success");
    return true;
}
} // namespace StorageDaemon
} // namespace OHOS
