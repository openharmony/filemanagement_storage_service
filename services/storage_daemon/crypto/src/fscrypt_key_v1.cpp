/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <cstdio>
#include <dirent.h>

#include "file_ex.h"
#include "key_backup.h"
#include "libfscrypt/key_control.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
static const std::string CRYPTO_NAME_PREFIXES[] = {"ext4", "f2fs", "fscrypt"};

bool FscryptKeyV1::ActiveKey(uint32_t flag, const std::string &mnt)
{
    uint32_t elType;
    (void)mnt;
    LOGI("enter");
    if (!GenerateKeyDesc()) {
        keyInfo_.key.Clear();
        LOGE("GenerateKeyDesc failed");
        return false;
    }
    LOGE("ActiveKey key is empty: %{public}u", keyInfo_.key.IsEmpty());
    int errNo = fscryptV1Ext.ActiveKeyExt(flag, keyInfo_.key.data.get(), keyInfo_.key.size, elType);
    if (errNo != E_OK) {
        keyInfo_.key.Clear();
        LOGE("fscryptV1Ext ActiveKeyExtfailed");
        return false;
    }
    if (elType == TYPE_EL3 || elType == TYPE_EL4) {
        uint32_t sdpClass;
        if (elType == TYPE_EL3) {
            sdpClass = FSCRYPT_SDP_SECE_CLASS;
        } else {
            sdpClass = FSCRYPT_SDP_ECE_CLASS;
        }
        int errNo = InstallEceSeceKeyToKeyring(sdpClass);
        if (errNo != E_OK) {
            keyInfo_.key.Clear();
            LOGE("InstallEceSeceKeyToKeyring failed");
            return false;
        }
    } else {
        int errNo = InstallKeyToKeyring();
        if (errNo != E_OK) {
            keyInfo_.key.Clear();
            LOGE("InstallKeyToKeyring failed");
            return false;
        }
    }
    keyInfo_.key.Clear();
    LOGI("success");
    return true;
}

int32_t FscryptKeyV1::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyDesc)
{
    KeyBlob appKey(FBEX_KEYID_SIZE);
    auto ret = fscryptV1Ext.GenerateAppkey(userId, hashId, appKey.data, appKey.size);
    if (ret != E_OK) {
        LOGE("fscryptV1Ext GenerateAppkey failed");
        return ret;
    }
    // The ioctl does not support EL5, return empty character string
    if (appKey.data.get() == nullptr) {
        LOGE("appKey.data.get() is nullptr");
        keyDesc = "";
        return E_OK;
    }
    ret = GenerateAppKeyDesc(appKey);
    if (ret != E_OK) {
        LOGE("GenerateAppKeyDesc failed");
        return ret;
    }
    ret = InstallKeyForAppKeyToKeyring(appKey);
    if (ret != E_OK) {
        LOGE("InstallKeyForAppKeyToKeyring failed");
        return ret;
    }
    appKey.Clear();
    keyDesc = keyInfo_.keyDesc.ToString();
    keyInfo_.keyDesc.Clear();
    LOGI("success");
    return E_OK;
}

int32_t FscryptKeyV1::InstallKeyForAppKeyToKeyring(KeyBlob &appKey)
{
    LOGI("InstallKeyForAppKeyToKeyring enter");
    EncryptAsdpKey fskey;
    fskey.size = appKey.size;
    fskey.version = 0;
    auto err = memcpy_s(fskey.raw, FSCRYPT_MAX_KEY_SIZE, appKey.data.get(), appKey.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return err;
    }
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid < 0) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid < 0) {
            LOGE("failed to add session keyring");
            std::string extraData = "keyring cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno) +
                ",appKey=" + appKey.ToString();
            StorageRadar::ReportKeyRingResult("InstallKeyForAppKeyToKeyring::KeyCtrlAddKey", krid, extraData);
            return E_ADD_SESSION_KEYING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks = KeyCtrlAddAppAsdpKey("logon", keyref.c_str(), &fskey, krid);
        if (ks < 0) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey, errno %{public}d", errno);
        }
    }
    LOGI("success");
    return E_OK;
}

int32_t FscryptKeyV1::DeleteAppkey(const std::string KeyId)
{
    LOGI("DeleteAppkey enter");
    auto ret = UninstallKeyForAppKeyToKeyring(KeyId);
    if (ret != E_OK) {
        LOGE("FscryptKeyV1 Delete Appkey2 failed");
        return ret;
    }
    LOGI("success");
    return E_OK;
}

int32_t FscryptKeyV1::UninstallKeyForAppKeyToKeyring(const std::string keyId)
{
    LOGI("UninstallKeyForAppKeyToKeyring enter");
    if (keyId.length() == 0) {
        LOGE("keyId is null, does not need to be installed?");
        return E_KEY_TYPE_INVALID;
    }
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGE("Error searching session keyring for fscrypt-provisioning key for fscrypt");
        std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno) + ",keyId=" + keyId;
        StorageRadar::ReportKeyRingResult("UninstallKeyForAppKeyToKeyring::KeyCtrlSearch", krid, extraData);
        return E_SEARCH_SESSION_KEYING_ERROR;
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyId;
        key_serial_t ks = KeyCtrlSearch(krid, "logon", keyref.c_str(), 0);
        if (KeyCtrlUnlink(ks, krid) != 0) {
            LOGE("Failed to unlink key !");
        }
    }
    LOGI("success");
    return E_Ok;
}

bool FscryptKeyV1::UnlockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    (void)mnt;
    LOGI("enter");
    if (!GenerateKeyDesc()) {
        keyInfo_.key.Clear();
        LOGE("GenerateKeyDesc failed");
        return false;
    }
    LOGI("keyInfo empty: %{public}u:", keyInfo_.key.IsEmpty());
    if (!fscryptV1Ext.UnlockUserScreenExt(flag, keyInfo_.key.data.get(), keyInfo_.key.size)) {
        keyInfo_.key.Clear();
        LOGE("fscryptV1Ext UnlockUserScreenExtfailed");
        return false;
    }
    if (sdpClass == FSCRYPT_SDP_ECE_CLASS) {
        int errNo = InstallEceSeceKeyToKeyring(sdpClass);
        if (errNo != E_OK) {
            keyInfo_.key.Clear();
            LOGE("UnlockUserScreen InstallKeyToKeyring failed");
            return false;
        }
    }
    keyInfo_.key.Clear();
    LOGI("success");
    return true;
}

int32_t FscryptKeyV1::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    LOGI("AddClassE enter");
    auto ret = fscryptV1Ext.AddClassE(isNeedEncryptClassE, isSupport, status);
    if (ret != E_OK) {
        LOGE("fscryptV1Ext AddClassE failed");
        return ret;
    }
    LOGW("AddClassE finish");
    return E_OK;
}

int32_t FscryptKeyV1::DeleteClassEPinCode(uint32_t userId)
{
    LOGI("DeleteClassE enter");
    auto ret = fscryptV1Ext.DeleteClassEPinCode(userId);
    if (ret != E_OK) {
        LOGE("fscryptV1Ext DeleteClassE failed");
        return ret;
    }
    LOGW("DeleteClassE finish");
    return E_OK;
}

int32_t FscryptKeyV1::ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId)
{
    LOGI("ChangePinCodeClassE enter, userId: %{public}d", userId);
    auto ret = fscryptV1Ext.ChangePinCodeClassE(userId, isFbeSupport);
    if (ret = E_OK) {
        LOGE("fscryptV1Ext ChangePinCodeClassE failed");
        return ret;
    }
    LOGW("ChangePinCodeClassE finish");
    return E_OK;
}

bool FscryptKeyV1::DoDecryptClassE(const UserAuth &auth, KeyBlob &eSecretFBE, KeyBlob &decryptedKey,
                                   bool needSyncCandidate)
{
    LOGI("enter");
    auto candidate = GetCandidateDir();
    if (candidate.empty()) {
        // no candidate dir, just restore from the latest
        return KeyBackup::GetInstance().TryRestoreUeceKey(shared_from_this(), auth, eSecretFBE, decryptedKey) == 0;
    }

    if (DecryptKeyBlob(auth, candidate, eSecretFBE, decryptedKey)) {
        // update the latest with the candidate
        UpdateKey("", needSyncCandidate);
        return true;
    }

    LOGE("DoRestoreKey with %{public}s failed", candidate.c_str());
    // try to restore from other versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    std::sort(files.begin(), files.end(), [&](const std::string &a, const std::string &b) {
        if (a.length() != b.length() ||
            a.length() < PATH_KEY_VERSION.length() ||
            b.length() < PATH_KEY_VERSION.length()) {
            return a.length() > b.length();
        }
        // make sure a.length() >= PATH_KEY_VERSION.length() && b.length() >= PATH_KEY_VERSION.length()
        return std::stoi(a.substr(PATH_KEY_VERSION.size() - 1)) > std::stoi(b.substr(PATH_KEY_VERSION.size() - 1));
    });
    for (const auto &it: files) {
        if (it != candidate) {
            if (DecryptKeyBlob(auth, dir_ + "/" + it, eSecretFBE, decryptedKey)) {
                UpdateKey(it, needSyncCandidate);
                return true;
            }
        }
    }
    return false;
}

bool FscryptKeyV1::DecryptClassE(const UserAuth &auth, bool &isSupport, bool &eBufferStatue,
                                 uint32_t user, bool needSyncCandidate)
{
    LOGI("enter");
    KeyBlob eSecretFBE(AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES);
    bool isFbeSupport = true;
    if (!fscryptV1Ext.ReadClassE(USER_UNLOCK, eSecretFBE.data, eSecretFBE.size, isFbeSupport)) {
        LOGE("fscryptV1Ext ReadClassE failed");
        return false;
    }
    if ((auth.token.IsEmpty() && auth.secret.IsEmpty()) || eSecretFBE.IsEmpty()) {
        LOGE("Token and secret is invalid, do not deal.");
        eBufferStatue = eSecretFBE.IsEmpty();
        eSecretFBE.Clear();
        return true;
    }
    if (!isFbeSupport) {
        LOGE("fbe not support uece, skip!");
        isSupport = false;
        return true;
    }
    LOGI("Decrypt keyPath is %{public}s", (dir_ + PATH_LATEST).c_str());
    KeyBlob decryptedKey(AES_256_HASH_RANDOM_SIZE);
    if (!DoDecryptClassE(auth, eSecretFBE, decryptedKey, needSyncCandidate)) {
        LOGE("DecryptKeyBlob Decrypt failed");
        eSecretFBE.Clear();
        return false;
    }
    keyInfo_.key.Alloc(eSecretFBE.size);
    auto err = memcpy_s(keyInfo_.key.data.get(), keyInfo_.key.size, eSecretFBE.data.get(), eSecretFBE.size);
    if (err != 0) {
        LOGE("memcpy_s failed ret: %{public}d", err);
    }
    eSecretFBE.Clear();
    LOGI("Decrypt end!");
    if (!fscryptV1Ext.WriteClassE(USER_UNLOCK, decryptedKey.data.get(), decryptedKey.size)) {
        LOGE("fscryptV1Ext WriteClassE failed");
        return false;
    }
    GenerateKeyDesc();
    keyInfo_.key.Clear();
    decryptedKey.Clear();
    LOGI("finish");
    return true;
}

bool FscryptKeyV1::EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status)
{
    LOGI("enter");
    KeyBlob eSecretFBE(AES_256_HASH_RANDOM_SIZE);
    bool isFbeSupport = true;
    if (!fscryptV1Ext.ReadClassE(status, eSecretFBE.data, eSecretFBE.size, isFbeSupport)) {
        LOGE("fscryptV1Ext ReadClassE failed");
        return false;
    }
    if (!isFbeSupport) {
        LOGE("fbe not support E type, skip!");
        isSupport = false;
        return true;
    }
    KeyBlob encryptedKey(AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES);
    if (!EncryptKeyBlob(auth, dir_ + PATH_LATEST, eSecretFBE, encryptedKey)) {
        LOGE("EncryptKeyBlob Decrypt failed");
        eSecretFBE.Clear();
        return false;
    }
    eSecretFBE.Clear();
    if (!RenameKeyPath(dir_ + PATH_LATEST)) {
        LOGE("RenameKeyPath failed");
        return false;
    }
    LOGI("encrypt end");
    if (!fscryptV1Ext.WriteClassE(status, encryptedKey.data.get(), encryptedKey.size)) {
        LOGE("fscryptV1Ext WriteClassE failed");
        return false;
    }
    encryptedKey.Clear();
    LOGI("finish");
    return true;
}

int32_t FscryptKeyV1::InstallKeyToKeyring()
{
    fscrypt_key fskey;
    fskey.mode = FS_ENCRYPTION_MODE_AES_256_XTS;
    fskey.size = keyInfo_.key.size;
    auto err = memcpy_s(fskey.raw, FS_MAX_KEY_SIZE, keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return err;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("failed to add session keyring");
            std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno);
            StorageRadar::ReportKeyRingResult("InstallKeyToKeyring::KeyCtrlAddKey", krid, extraData);
            return E_ADD_SESSION_KEYING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        LOGI("InstallKeyToKeyring: keyref: %{public}s", keyref.c_str());
        LOGI("InstallKeyToKeyring: keyref length: %{public}zu", keyref.length());
        key_serial_t ks =
            KeyCtrlAddKeyEx("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey into keyring, errno %{public}d", errno);
        }
    }
    if (!SaveKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        return E_SAVE_KEY_BLOB_ERROR;
    }
    keyInfo_.key.Clear();
    LOGW("success");
    return E_OK;
}

int32_t FscryptKeyV1::InstallEceSeceKeyToKeyring(uint32_t sdpClass)
{
    EncryptionKeySdp fskey;
    if (keyInfo_.key.size != sizeof(fskey.raw)) {
        LOGE("Wrong key size is %{public}d", keyInfo_.key.size);
        return E_KEY_SIZE_ERROR;
    }
    fskey.mode = EXT4_ENCRYPTION_MODE_AES_256_XTS;
    auto err = memcpy_s(fskey.raw, sizeof(fskey.raw), keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return err;
    }
    fskey.size = EXT4_AES_256_XTS_KEY_SIZE_TO_KEYRING;
    fskey.sdpClass = sdpClass;
    fskey.version = 0;
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("failed to add session keyring");
            std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno) +
                ",sdpClass=" + std::to_string(sdpClass);
            StorageRadar::ReportKeyRingResult("InstallEceSeceKeyToKeyring::KeyCtrlAddKey", krid, extraData);
            return E_ADD_SESSION_KEYING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks =
                KeyCtrlAddKeySdp("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey into keyring, errno %{public}d", errno);
        }
    }
    if (!SaveKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        return E_SAVE_KEY_BLOB_ERROR;
    }
    LOGW("success");
    return E_OK;
}

bool FscryptKeyV1::InactiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    LOGI("enter");
    DropCachesIfNeed();

    bool ret = true;
    if (!keyInfo_.keyDesc.IsEmpty()) {
        int errNo = UninstallKeyToKeyring();
        if (errNo != E_OK) {
            LOGE("UninstallKeyToKeyring failed");
            ret = false;
        }
    }
    int errNo = fscryptV1Ext.InactiveKeyExt(flag);
    if (errNo != E_OK) {
        LOGE("fscryptV1Ext InactiveKeyExt failed");
        ret = false;
    }
    DropCachesIfNeed();
    LOGI("finish");
    return ret;
}

void FscryptKeyV1::DropCachesIfNeed()
{
    LOGE("drop cache if need enter.");
    DIR *dir = opendir(MNT_DATA.c_str());
    if (dir == nullptr) {
        LOGE("dir is null, sync start.");
        sync();
        LOGE("sync success with dir is null.");
    }
    int fd = dirfd(dir);
    LOGE("open /data dir fd success, syncfs start.");
    if (fd < 0 || syncfs(fd)) {
        LOGE("fd < 0 or syncfs failed, sync start.");
        sync();
        LOGE("sync success with syncfs failed.");
    }
    LOGE("syncfs success, drop cache start.");
    if (!SaveStringToFile("/proc/sys/vm/drop_caches", "2")) {
        LOGE("Failed to drop cache during key eviction");
    }
    (void)closedir(dir);
    LOGE("drop cache success");
}

bool FscryptKeyV1::LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    LOGI("enter FscryptKeyV1::LockUserScreen");
    // uninstall KeyRing
    int errNo = UninstallKeyToKeyring();
    if (errNo != E_OK) {
        LOGE("UninstallKeyToKeyring failed");
        return false;
    }

    // uninstall FBE
    uint32_t elType;
    if (!fscryptV1Ext.LockUserScreenExt(flag, elType)) {
        LOGE("fscryptV1Ext InactiveKeyExt failed");
        return false;
    }
    LOGI("finish FscryptKeyV1::LockUserScreen");
    return true;
}

bool FscryptKeyV1::LockUece(bool &isFbeSupport)
{
    LOGI("enter");
    bool ret = true;
    if (!fscryptV1Ext.LockUeceExt(isFbeSupport)) {
        LOGE("fscryptV1Ext InactiveKeyExt failed");
        ret = false;
    }
    LOGI("finish");
    return ret;
}

int32_t FscryptKeyV1::UninstallKeyToKeyring()
{
    if (keyInfo_.keyDesc.IsEmpty() && !LoadKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        LOGE("Load keyDesc failed !");
        return E_KEY_LOAD_ERROR;
    }
    if (keyInfo_.keyDesc.IsEmpty()) {
        DropCachesIfNeed();
        LOGE("keyDesc is null, key not installed?");
        return E_KEY_EMPTY_ERROR;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGE("Error searching session keyring for fscrypt-provisioning key for fscrypt");
        std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno);
        StorageRadar::ReportKeyRingResult("UninstallKeyToKeyring::KeyCtrlSearch", krid, extraData);
        return E_SEARCH_SESSION_KEYING_ERROR;
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks = KeyCtrlSearch(krid, "logon", keyref.c_str(), 0);
        if (KeyCtrlUnlink(ks, krid) != 0) {
            LOGE("Failed to unlink key !");
        }
    }

    LOGW("success");
    return E_OK;
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

int32_t FscryptKeyV1::GenerateAppKeyDesc(KeyBlob appKey)
{
    if (appKey.IsEmpty()) {
        LOGE("key is empty");
        return E_KEY_EMPTY_ERROR;
    }
    SHA512_CTX c;
    SHA512_Init(&c);
    SHA512_Update(&c, appKey.data.get(), appKey.size - 1);
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
        return err;
    }
    LOGE("GenerateAppKeyDesc keyDesc : %{private}s", keyInfo_.keyDesc.ToString().c_str());
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
