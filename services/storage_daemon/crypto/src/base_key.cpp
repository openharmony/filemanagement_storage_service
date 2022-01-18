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
#include "string_ex.h"
#include "file_ex.h"
#include "utils/log.h"
#include "huks_master.h"

namespace OHOS {
namespace StorageDaemon {
static bool g_isHuksMasterInit = false;
BaseKey::BaseKey(std::string dir, uint8_t keyLen) : dir_(dir), keyLen_(keyLen)
{
    if (!g_isHuksMasterInit && (HuksMaster::Init() == 0)) {
        g_isHuksMasterInit = true;
    }
}

bool BaseKey::InitKey(uint8_t version)
{
    LOGD("enter");
    if (version == FSCRYPT_INVALID || version > KeyCtrl::GetFscryptVersion()) {
        LOGE("invalid version %{public}d", version);
        return false;
    }
    if (!keyInfo_.key.IsEmpty()) {
        LOGE("key is not empty");
        return false;
    }
    if (!GenerateKeyBlob(keyInfo_.key, keyLen_)) {
        LOGE("GenerateKeyBlob raw key failed");
        return false;
    }
    if (!GenerateKeyDesc()) {
        LOGE("GenerateKeyDesc failed");
        keyInfo_.key.Clear();
        return false;
    }

    keyInfo_.version = version;
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

    static_assert(SHA512_DIGEST_LENGTH >= CRYPTO_KEY_DESC_SIZE, "Hash too short for descriptor");
    keyInfo_.keyDesc.Alloc(CRYPTO_KEY_DESC_SIZE);
    auto err = memcpy_s(keyInfo_.keyDesc.data.get(), keyInfo_.keyDesc.size, keyRef2, CRYPTO_KEY_DESC_SIZE);
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
    } else {
        OHOS::ForceRemoveDirectory(dir_);
    }
    dir_ = originalDir;
    return true;
}

bool BaseKey::DoStoreKey(const UserAuth &auth)
{
    OHOS::ForceCreateDirectory(dir_);
    OHOS::SaveStringToFile(dir_ + "/version", std::to_string(keyInfo_.version));
    if (!GenerateAndSaveKeyBlob(keyContext_.alias, "alias", CRYPTO_KEY_ALIAS_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob alias failed");
        return false;
    }
    if (!HuksMaster::GenerateKey(keyContext_.alias)) {
        LOGE("HuksMaster::GenerateKey failed");
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
    keyContext_.encrypted.Clear();
    return true;
}

bool BaseKey::EncryptKey(const UserAuth &auth)
{
    auto ret = HuksMaster::EncryptKey(keyContext_, auth, keyInfo_);
    keyContext_.alias.Clear();
    keyContext_.secDiscard.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    return ret;
}

bool BaseKey::RestoreKey(const UserAuth &auth)
{
    LOGD("enter");
    auto ver = KeyCtrl::LoadVersion(dir_);
    if (ver == FSCRYPT_INVALID) {
        LOGE("RestoreKey fail. bad version");
        return false;
    }
    keyInfo_.version = static_cast<uint8_t>(ver);

    if (!LoadKeyBlob(keyContext_.encrypted, "encrypted")) {
        return false;
    }
    if (!LoadKeyBlob(keyContext_.alias, "alias", CRYPTO_KEY_ALIAS_SIZE)) {
        keyContext_.alias.Clear();
        return false;
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, "sec_discard", CRYPTO_KEY_SECDISC_SIZE)) {
        keyContext_.encrypted.Clear();
        keyContext_.alias.Clear();
        return false;
    }
    return DecryptKey(auth);
}

bool BaseKey::DecryptKey(const UserAuth &auth)
{
    auto ret = HuksMaster::DecryptKey(keyContext_, auth, keyInfo_);
    keyContext_.encrypted.Clear();
    keyContext_.alias.Clear();
    keyContext_.secDiscard.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    if (ret && !GenerateKeyDesc()) {
        LOGE("GenerateKeyDesc failed");
        return false;
    }
    return ret;
}

bool BaseKey::ActiveKey(const std::string& mnt)
{
    if (keyInfo_.version == FSCRYPT_V1) {
        return ActiveKeyLegacy();
    } else if (keyInfo_.version == FSCRYPT_V2) {
        return ActiveKeyV2(mnt);
    }

    LOGE("ActiveKey fail. bad keyinfo.version %{public}d", keyInfo_.version);
    return false;
}

bool BaseKey::ClearKey()
{
    // rmdir and remove huks keyalias
    return InactiveKey();
}

bool BaseKey::InactiveKey(const std::string& mnt)
{
    if (keyInfo_.version == FSCRYPT_V1) {
        return InactiveKeyLegacy();
    } else if (keyInfo_.version == FSCRYPT_V2) {
        return InactiveKeyV2(mnt);
    }

    LOGE("InactiveKey fail. bad keyinfo.version %{public}d", keyInfo_.version);
    return false;
}

// The legacy kernel interface, add the key to kernel keyring.
bool BaseKey::ActiveKeyLegacy()
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
    if (!SaveKeyBlob(keyInfo_.keyDesc, "key_desc")) {
        return false;
    }
    keyInfo_.key.Clear();
    LOGD("success");
    return true;
}

// rmdir after it if needed.
bool BaseKey::InactiveKeyLegacy()
{
    LOGD("enter");
    if (keyInfo_.keyDesc.IsEmpty()) {
        LOGE("keyDesc is null, key not installed?");
        return false;
    }
    if (keyContext_.alias.IsEmpty()) {
        LOGE("alias is null");
        return false;
    }
    HuksMaster::DeleteKey(keyContext_.alias);

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

    keyInfo_.key.Clear();
    keyInfo_.keyDesc.Clear();
    LOGD("success");
    return true;
}

// v2 interface, use keyId as the later setpolicy param.
bool BaseKey::ActiveKeyV2(const std::string& mnt)
{
    LOGD("enter");
    if (keyInfo_.key.IsEmpty()) {
        LOGE("rawkey is null");
        return false;
    }

    auto buf = std::make_unique<char[]>(sizeof(fscrypt_add_key_arg) + FSCRYPT_MAX_KEY_SIZE);
    auto arg = reinterpret_cast<fscrypt_add_key_arg *>(buf.get());
    (void)memset_s(arg, sizeof(fscrypt_add_key_arg) + FSCRYPT_MAX_KEY_SIZE, 0, sizeof(fscrypt_add_key_arg) +
        FSCRYPT_MAX_KEY_SIZE);
    arg->key_spec.type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;
    arg->raw_size = keyInfo_.key.size;
    auto err = memcpy_s(arg->raw, FSCRYPT_MAX_KEY_SIZE, keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err) {
        LOGE("memcpy failed ret %{public}d", err);
        return false;
    }

    auto ret = KeyCtrl::InstallKey(mnt, *arg);
    if (ret == false) {
        LOGE("InstallKey failed");
        return false;
    }

    keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE);
    (void)memcpy_s(keyInfo_.keyId.data.get(), FSCRYPT_KEY_IDENTIFIER_SIZE, arg->key_spec.u.identifier,
        FSCRYPT_KEY_IDENTIFIER_SIZE);
    LOGD("success. key_id len:%{public}d, data(hex):%{private}s", keyInfo_.keyId.size,
        keyInfo_.keyId.ToString().c_str());
    if (!SaveKeyBlob(keyInfo_.keyId, "key_id")) {
        // do some cleanup
        return false;
    }

    keyInfo_.key.Clear();
    LOGD("success");
    return true;
}

// rmdir after it if needed.
bool BaseKey::InactiveKeyV2(const std::string& mnt)
{
    LOGD("enter");
    if (keyInfo_.keyId.size != FSCRYPT_KEY_IDENTIFIER_SIZE) {
        LOGE("keyId is invalid, %{public}u", keyInfo_.keyId.size);
        return false;
    }

    fscrypt_remove_key_arg arg;
    memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.key_spec.type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;
    (void)memcpy_s(arg.key_spec.u.identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, keyInfo_.keyId.data.get(),
        keyInfo_.keyId.size);

    auto ret = KeyCtrl::RemoveKey(mnt, arg);
    if (ret == false) {
        return false;
    }
    if (arg.removal_status_flags & FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS) {
        LOGE("Other users still have this key added");
    } else if (arg.removal_status_flags & FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY) {
        // is the key fully removed?
        LOGE("Some files using this key are still in-use");
    } else {
        LOGD("RemoveKey success");
    }

    keyInfo_.keyDesc.Clear();
    keyInfo_.keyId.Clear();

    return true;
}
} // namespace StorageDaemon
} // namespace OHOS
