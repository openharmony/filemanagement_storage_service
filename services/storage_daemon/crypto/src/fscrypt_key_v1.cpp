/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <unistd.h>
#include <dirent.h>
#include <regex>

#include "file_ex.h"
#include "key_backup.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
static const std::string CRYPTO_NAME_PREFIXES[] = {"ext4", "f2fs", "fscrypt"};
constexpr uint32_t USER_UNLOCK = 0x2;

int32_t FscryptKeyV1::ActiveKey(const KeyBlob &authToken, uint32_t flag, const std::string &mnt)
{
    uint32_t elType;
    (void)mnt;
    LOGI("[L5:FscryptKeyV1] ActiveKey: >>> ENTER <<< flag=%{public}u, dir=%{public}s", flag, dir_.c_str());
    int32_t ret = GenerateKeyDesc();
    if (ret != E_OK) {
        keyInfo_.key.Clear();
        LOGE("[L5:FscryptKeyV1] ActiveKey: <<< EXIT FAILED <<< dir=%{public}s, GenerateKeyDesc failed", dir_.c_str());
        return ret;
    }
    LOGE("[L5:FscryptKeyV1] ActiveKey: key is empty check, isEmpty=%{public}u", keyInfo_.key.IsEmpty());
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int errNo = fscryptV1Ext.ActiveKeyExt(flag, keyInfo_.key, elType, authToken);
    if (errNo != E_OK) {
        keyInfo_.key.Clear();
        LOGE("[L5:FscryptKeyV1] ActiveKey: <<< EXIT FAILED <<< dir=%{public}s, ActiveKeyExt failed", dir_.c_str());
        return errNo;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("HUKS: ACTIVE KEY EXT", startTime);
    LOGI("SD_DURATION: HUKS: ACTIVE KEY EXT: delay time = %{public}s", delay.c_str());
    startTime = StorageService::StorageRadar::RecordCurrentTime();
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
            LOGE("[L5:FscryptKeyV1] ActiveKey: <<< EXIT FAILED <<< dir=%{public}s, InstallEceSeceKeyToKeyring failed",
                 dir_.c_str());
            return errNo;
        }
    } else {
        int errNo = InstallKeyToKeyring();
        if (errNo != E_OK) {
            keyInfo_.key.Clear();
            LOGE("[L5:FscryptKeyV1] ActiveKey: <<< EXIT FAILED <<< dir=%{public}s, InstallKeyToKeyring failed",
                 dir_.c_str());
            return errNo;
        }
    }
    delay = StorageService::StorageRadar::ReportDuration("INSTALL KEY TO KEYRING", startTime);
    LOGI("SD_DURATION: INSTALL KEY TO KEYRING: delay time = %{public}s", delay.c_str());
    keyInfo_.key.Clear();
    LOGI("[L5:FscryptKeyV1] ActiveKey: <<< EXIT SUCCESS <<< dir=%{public}s", dir_.c_str());
    return E_OK;
}

int32_t FscryptKeyV1::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyDesc)
{
    LOGI("[L5:FscryptKeyV1] GenerateAppkey: >>> ENTER <<< userId=%{public}u, hashId=%{public}u", userId, hashId);
    KeyBlob appKey(FBEX_KEYID_SIZE);
    auto ret = fscryptV1Ext.GenerateAppkey(userId, hashId, appKey.data, appKey.size);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, GenerateAppkey failed", userId);
        return ret;
    }
    // The ioctl does not support EL5, return empty character string
    if (appKey.data.get() == nullptr) {
        LOGE("[L5:FscryptKeyV1] GenerateAppkey: appKey data is nullptr, return empty");
        keyDesc = "";
        return E_OK;
    }
    ret = GenerateAppKeyDesc(appKey);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, GenerateAppKeyDesc failed",
             userId);
        return ret;
    }
    ret = InstallKeyForAppKeyToKeyring(appKey);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, InstallKeyForAppKeyToKeyring"
             "failed", userId);
        return ret;
    }
    appKey.Clear();
    keyDesc = keyInfo_.keyDesc.ToString();
    LOGI("[L5:FscryptKeyV1] GenerateAppkey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1::InstallKeyForAppKeyToKeyring(KeyBlob &appKey)
{
    LOGI("[L5:FscryptKeyV1] InstallKeyForAppKeyToKeyring: >>> ENTER <<<");
    EncryptAsdpKey fskey;
    fskey.size = appKey.size;
    fskey.version = 0;
    auto err = memcpy_s(fskey.raw, FSCRYPT_MAX_KEY_SIZE, appKey.data.get(), appKey.size);
    if (err != EOK) {
        LOGE("[L5:FscryptKeyV1] InstallKeyForAppKeyToKeyring: <<< EXIT FAILED <<< memcpy failed, err=%{public}d", err);
        return err;
    }
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid < 0) {
        LOGI("[L5:FscryptKeyV1] InstallKeyForAppKeyToKeyring: no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid < 0) {
            LOGE("[L5:FscryptKeyV1] InstallKeyForAppKeyToKeyring: <<< EXIT FAILED <<< add session keyring failed,"
                 "errno=%{public}d", errno);
            std::string extraData = "keyring cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno) +
                ",appKey=" + GetAnonyString(appKey.ToString());
            StorageRadar::ReportKeyRingResult("InstallKeyForAppKeyToKeyring::KeyCtrlAddKey", krid, extraData);
            return E_ADD_SESSION_KEYRING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks = KeyCtrlAddAppAsdpKey("logon", keyref.c_str(), &fskey, krid);
        if (ks < 0) {
            // Addkey failed, need to process the error
            LOGE("[L5:FscryptKeyV1] InstallKeyForAppKeyToKeyring: Failed to AddKey, errno %{public}d", errno);
        }
    }
    LOGI("[L5:FscryptKeyV1] InstallKeyForAppKeyToKeyring: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::DeleteAppkey(const std::string keyId)
{
    LOGI("[L5:FscryptKeyV1] DeleteAppkey: >>> ENTER <<< keyIdLen=%{public}zu", keyId.size());
    auto ret = UninstallKeyForAppKeyToKeyring(keyId);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] DeleteAppkey: <<< EXIT FAILED <<< UninstallKeyForAppKeyToKeyring failed");
        return ret;
    }
    LOGI("[L5:FscryptKeyV1] DeleteAppkey: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::UninstallKeyForAppKeyToKeyring(const std::string keyId)
{
    LOGI("[L5:FscryptKeyV1] UninstallKeyForAppKeyToKeyring: >>> ENTER <<< keyIdLen=%{public}zu", keyId.size());
    if (keyId.length() == 0) {
        LOGE("[L5:FscryptKeyV1] UninstallKeyForAppKeyToKeyring: <<< EXIT FAILED <<< keyId is null");
        return E_KEY_TYPE_INVALID;
    }
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGE("[L5:FscryptKeyV1] UninstallKeyForAppKeyToKeyring: <<< EXIT FAILED <<< Error searching session keyring");
        std::string extraData =
            "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno) + ",keyIdLen=" + std::to_string(keyId.size());
        StorageRadar::ReportKeyRingResult("UninstallKeyForAppKeyToKeyring::KeyCtrlSearch", krid, extraData);
        return E_SEARCH_SESSION_KEYING_ERROR;
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyId;
        key_serial_t ks = KeyCtrlSearch(krid, "logon", keyref.c_str(), 0);
        if (KeyCtrlUnlink(ks, krid) != 0) {
            LOGE("[L5:FscryptKeyV1] UninstallKeyForAppKeyToKeyring: Failed to unlink key");
        }
    }
    LOGI("[L5:FscryptKeyV1] UninstallKeyForAppKeyToKeyring: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::UnlockUserScreen(const KeyBlob &authToken, uint32_t flag,
    uint32_t sdpClass, const std::string &mnt)
{
    (void)mnt;
    LOGI("[L5:FscryptKeyV1] UnlockUserScreen: >>> ENTER <<< flag=%{public}u, sdpClass=%{public}u", flag, sdpClass);
    int32_t ret = GenerateKeyDesc();
    if (ret != E_OK) {
        keyInfo_.key.Clear();
        LOGE("[L5:FscryptKeyV1] UnlockUserScreen: <<< EXIT FAILED <<< GenerateKeyDesc failed");
        return ret;
    }
    LOGI("[L5:FscryptKeyV1] UnlockUserScreen: keyInfo empty: %{public}u", keyInfo_.key.IsEmpty());
    ret = fscryptV1Ext.UnlockUserScreenExt(flag, keyInfo_.key.data.get(), keyInfo_.key.size, authToken);
    if (ret != E_OK) {
        keyInfo_.key.Clear();
        LOGE("[L5:FscryptKeyV1] UnlockUserScreen: <<< EXIT FAILED <<< UnlockUserScreenExt failed");
        return ret;
    }
    if (sdpClass == FSCRYPT_SDP_ECE_CLASS) {
        int errNo = InstallEceSeceKeyToKeyring(sdpClass);
        if (errNo != E_OK) {
            keyInfo_.key.Clear();
            LOGE("[L5:FscryptKeyV1] UnlockUserScreen: <<< EXIT FAILED <<< InstallKeyToKeyring failed");
            return errNo;
        }
    }
    keyInfo_.key.Clear();
    LOGI("[L5:FscryptKeyV1] UnlockUserScreen: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    LOGI("[L5:FscryptKeyV1] AddClassE: >>> ENTER <<< status=%{public}u", status);
    auto ret = fscryptV1Ext.AddClassE(isNeedEncryptClassE, isSupport, status);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] AddClassE: <<< EXIT FAILED <<< AddClassE failed");
        return ret;
    }
    keyInfo_.keyDesc.Alloc(CRYPTO_KEY_DESC_SIZE);
    auto err = memset_s(keyInfo_.keyDesc.data.get(), keyInfo_.keyDesc.size, status, CRYPTO_KEY_DESC_SIZE);
    if (err != 0) {
        LOGE("[L5:FscryptKeyV1] AddClassE: memset_s failed ret: %{public}d", err);
    }
    LOGW("[L5:FscryptKeyV1] AddClassE: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::DeleteClassEPinCode(uint32_t userId)
{
    LOGI("[L5:FscryptKeyV1] DeleteClassEPinCode: >>> ENTER <<< userId=%{public}u", userId);
    auto ret = fscryptV1Ext.DeleteClassEPinCode(userId);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] DeleteClassEPinCode: <<< EXIT FAILED <<< failed");
        return ret;
    }
    LOGW("[L5:FscryptKeyV1] DeleteClassEPinCode: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1::ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId)
{
    LOGI("[L5:FscryptKeyV1] ChangePinCodeClassE: >>> ENTER <<< userId=%{public}u", userId);
    auto ret = fscryptV1Ext.ChangePinCodeClassE(userId, isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] ChangePinCodeClassE: <<< EXIT FAILED <<< failed");
        return ret;
    }
    LOGW("[L5:FscryptKeyV1] ChangePinCodeClassE: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1::UpdateClassEBackUp(uint32_t userId)
{
    LOGI("[L5:FscryptKeyV1] UpdateClassEBackUp: >>> ENTER <<< userId=%{public}u", userId);
    auto ret = fscryptV1Ext.UpdateClassEBackUp(userId);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] UpdateClassEBackUp: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
             userId, ret);
        return ret;
    }
    LOGW("[L5:FscryptKeyV1] UpdateClassEBackUp: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1::DoDecryptClassE(const UserAuth &auth, KeyBlob &eSecretFBE, KeyBlob &decryptedKey,
                                      bool needSyncCandidate)
{
    LOGI("[L5:FscryptKeyV1] DoDecryptClassE: >>> ENTER <<<");
    auto candidate = GetCandidateDir();
    if (candidate.empty()) {
        // no candidate dir, just restore from the latest
        return KeyBackup::GetInstance().TryRestoreUeceKey(shared_from_this(), auth, eSecretFBE, decryptedKey);
    }
    auto ret = DecryptKeyBlob(auth, candidate, eSecretFBE, decryptedKey);
    if (ret == E_OK) {
        // update the latest with the candidate
        UpdateKey("", needSyncCandidate);
        LOGI("[L5:FscryptKeyV1] DoDecryptClassE: <<< EXIT SUCCESS <<<");
        return E_OK;
    }

    LOGE("[L5:FscryptKeyV1] DoDecryptClassE: DoRestoreKey with %{public}s failed", candidate.c_str());
    // try to restore from other versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    std::sort(files.begin(), files.end(), [&](const std::string &a, const std::string &b) {
        if (a.length() != b.length() ||
            a.length() < strlen(PATH_KEY_VERSION) ||
            b.length() < strlen(PATH_KEY_VERSION)) {
            return a.length() > b.length();
        }
        std::regex pattern("^version_\\d+$");
        if (!std::regex_search(a, pattern)) return false;
        if (!std::regex_search(b, pattern)) return true;

        // make sure a and b is version_\d+
        auto a_len = std::atoi(a.substr(strlen(PATH_KEY_VERSION) - 1).c_str());
        auto b_len = std::atoi(b.substr(strlen(PATH_KEY_VERSION) - 1).c_str());
        return a_len > b_len;
    });
    for (const auto &it: files) {
        if (it != candidate) {
            auto ret = DecryptKeyBlob(auth, dir_ + "/" + it, eSecretFBE, decryptedKey);
            if (ret == E_OK) {
                UpdateKey((dir_ + "/" + it), needSyncCandidate);
                LOGI("[L5:FscryptKeyV1] DoDecryptClassE: <<< EXIT SUCCESS <<<");
                return E_OK;
            }
        }
    }
    LOGE("[L5:FscryptKeyV1] DoDecryptClassE: <<< EXIT FAILED <<< all attempts failed");
    return ret;
}

int32_t FscryptKeyV1::DecryptClassE(const UserAuth &auth, bool &isSupport, bool &eBufferStatue,
                                    uint32_t user, bool needSyncCandidate)
{
    LOGI("[L5:FscryptKeyV1] DecryptClassE: >>> ENTER <<< userId=%{public}u", user);
    KeyBlob eSecretFBE(AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES);
    bool isFbeSupport = true;
    auto authToken = auth.token;
    auto ret = fscryptV1Ext.ReadClassE(USER_UNLOCK, eSecretFBE, authToken, isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] DecryptClassE: <<< EXIT FAILED <<< ReadClassE failed");
        return ret;
    }
    if ((auth.token.IsEmpty() && auth.secret.IsEmpty()) || eSecretFBE.IsEmpty()) {
        LOGE("[L5:FscryptKeyV1] DecryptClassE: Token and secret is invalid");
        eBufferStatue = eSecretFBE.IsEmpty();
        eSecretFBE.Clear();
        return E_OK;
    }
    if (!isFbeSupport) {
        LOGE("[L5:FscryptKeyV1] DecryptClassE: fbe not support uece");
        isSupport = false;
        return E_OK;
    }
    LOGI("[L5:FscryptKeyV1] DecryptClassE: Decrypt keyPath is %{public}s", (dir_ + PATH_LATEST).c_str());
    KeyBlob decryptedKey(AES_256_HASH_RANDOM_SIZE);
    ret = DoDecryptClassE(auth, eSecretFBE, decryptedKey, needSyncCandidate);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] DecryptClassE: <<< EXIT FAILED <<< DecryptKeyBlob failed");
        eSecretFBE.Clear();
        std::string extraData =
            "cmd=DoDecryptClassE,ret=" + std::to_string(ret);
        StorageRadar::ReportKeyRingResult("DecryptClassE::DoDecryptClassE", ret, extraData);
        return ret;
    }
    keyInfo_.key.Alloc(eSecretFBE.size);
    auto err = memcpy_s(keyInfo_.key.data.get(), keyInfo_.key.size, eSecretFBE.data.get(), eSecretFBE.size);
    if (err != 0) {
        LOGE("[L5:FscryptKeyV1] DecryptClassE: memcpy_s failed ret: %{public}d", err);
    }
    eSecretFBE.Clear();
    LOGI("[L5:FscryptKeyV1] DecryptClassE: Decrypt end");
    ret = fscryptV1Ext.WriteClassE(USER_UNLOCK, decryptedKey.data.get(), decryptedKey.size);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] DecryptClassE: <<< EXIT FAILED <<< WriteClassE failed");
        return ret;
    }
    GenerateKeyDesc();
    keyInfo_.key.Clear();
    decryptedKey.Clear();
    LOGI("[L5:FscryptKeyV1] DecryptClassE: <<< EXIT SUCCESS <<< userId=%{public}u", user);
    return E_OK;
}

int32_t FscryptKeyV1::EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status)
{
    LOGI("[L5:FscryptKeyV1] EncryptClassE: >>> ENTER <<< userId=%{public}u, status=%{public}u", user, status);
    KeyBlob eSecretFBE(AES_256_HASH_RANDOM_SIZE);
    bool isFbeSupport = true;
    auto authToken = auth.token;
    auto ret = fscryptV1Ext.ReadClassE(status, eSecretFBE, authToken, isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] EncryptClassE: <<< EXIT FAILED <<< ReadClassE failed");
        return ret;
    }
    if (!isFbeSupport) {
        LOGE("[L5:FscryptKeyV1] EncryptClassE: fbe not support E type");
        isSupport = false;
        return E_OK;
    }
    KeyBlob encryptedKey(AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES);
    ret = EncryptKeyBlob(auth, dir_ + PATH_LATEST, eSecretFBE, encryptedKey);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] EncryptClassE: <<< EXIT FAILED <<< EncryptKeyBlob failed");
        eSecretFBE.Clear();
        return ret;
    }
    eSecretFBE.Clear();
    if (!RenameKeyPath(dir_ + PATH_LATEST)) {
        LOGE("[L5:FscryptKeyV1] EncryptClassE: <<< EXIT FAILED <<< RenameKeyPath failed");
        return E_RENAME_KEY_PATH;
    }
    LOGI("[L5:FscryptKeyV1] EncryptClassE: encrypt end");
    ret = fscryptV1Ext.WriteClassE(status, encryptedKey.data.get(), encryptedKey.size);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] EncryptClassE: <<< EXIT FAILED <<< WriteClassE failed");
        return ret;
    }
    encryptedKey.Clear();
    LOGI("[L5:FscryptKeyV1] EncryptClassE: <<< EXIT SUCCESS <<< userId=%{public}u", user);
    return E_OK;
}

int32_t FscryptKeyV1::InstallKeyToKeyring()
{
    LOGI("[L5:FscryptKeyV1] InstallKeyToKeyring: >>> ENTER <<< dir=%{public}s", dir_.c_str());
    fscrypt_key fskey;
    fskey.mode = FS_ENCRYPTION_MODE_AES_256_XTS;
    fskey.size = keyInfo_.key.size;
    auto err = memcpy_s(fskey.raw, FS_MAX_KEY_SIZE, keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err != EOK) {
        LOGE("[L5:FscryptKeyV1] InstallKeyToKeyring: <<< EXIT FAILED <<< memcpy failed, err=%{public}d", err);
        return err;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("[L5:FscryptKeyV1] InstallKeyToKeyring: no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("[L5:FscryptKeyV1] InstallKeyToKeyring: <<< EXIT FAILED <<< add session keyring failed,"
                 "errno=%{public}d", errno);
            std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno);
            StorageRadar::ReportKeyRingResult("InstallKeyToKeyring::KeyCtrlAddKey", krid, extraData);
            return E_ADD_SESSION_KEYRING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks =
            KeyCtrlAddKeyEx("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("[L5:FscryptKeyV1] InstallKeyToKeyring: Failed to AddKey into keyring, errno %{public}d", errno);
        }
    }
    if (!SaveKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        LOGE("[L5:FscryptKeyV1] InstallKeyToKeyring: <<< EXIT FAILED <<< SaveKeyBlob failed");
        return E_SAVE_KEY_BLOB_ERROR;
    }
    keyInfo_.key.Clear();
    LOGW("[L5:FscryptKeyV1] InstallKeyToKeyring: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::InstallEceSeceKeyToKeyring(uint32_t sdpClass)
{
    LOGI("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: >>> ENTER <<< sdpClass=%{public}u", sdpClass);
    EncryptionKeySdp fskey;
    if (keyInfo_.key.size != sizeof(fskey.raw)) {
        LOGE("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: <<< EXIT FAILED <<< Wrong key size=%{public}d",
             keyInfo_.key.size);
        return E_KEY_SIZE_ERROR;
    }
    fskey.mode = EXT4_ENCRYPTION_MODE_AES_256_XTS;
    auto err = memcpy_s(fskey.raw, sizeof(fskey.raw), keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err != EOK) {
        LOGE("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: <<< EXIT FAILED <<< memcpy failed, err=%{public}d", err);
        return err;
    }
    fskey.size = EXT4_AES_256_XTS_KEY_SIZE_TO_KEYRING;
    fskey.sdpClass = sdpClass;
    fskey.version = 0;
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: <<< EXIT FAILED <<< add session keyring failed,"
                 "errno=%{public}d", errno);
            std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno) +
                ",sdpClass=" + std::to_string(sdpClass);
            StorageRadar::ReportKeyRingResult("InstallEceSeceKeyToKeyring::KeyCtrlAddKey", krid, extraData);
            return E_ADD_SESSION_KEYRING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks =
                KeyCtrlAddKeySdp("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: Failed to AddKey into keyring, errno %{public}d",
                 errno);
        }
    }
    if (!SaveKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        LOGE("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: <<< EXIT FAILED <<< SaveKeyBlob failed");
        return E_SAVE_KEY_BLOB_ERROR;
    }
    LOGW("[L5:FscryptKeyV1] InstallEceSeceKeyToKeyring: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::InactiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    LOGI("[L5:FscryptKeyV1] InactiveKey: >>> ENTER <<< flag=%{public}u, dir=%{public}s", flag, dir_.c_str());
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    DropCachesIfNeed();
    auto delay = StorageService::StorageRadar::ReportDuration("DROP CACHE", startTime,
        StorageService::DELAY_TIME_THRESH_HIGH, StorageService::DEFAULT_USERID);
    LOGI("SD_DURATION: DROP CACHE: delay time = %{public}s", delay.c_str());

    int32_t ret = E_OK;
    if (!keyInfo_.keyDesc.IsEmpty()) {
        int errNo = UninstallKeyToKeyring();
        if (errNo != E_OK) {
            LOGE("[L5:FscryptKeyV1] InactiveKey: UninstallKeyToKeyring failed, ret=%{public}d", errNo);
            ret = errNo;
        }
    }
    ret = fscryptV1Ext.InactiveKeyExt(flag);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] InactiveKey: <<< EXIT FAILED <<< InactiveKeyExt failed");
        return ret;
    }
    startTime = StorageService::StorageRadar::RecordCurrentTime();
    DropCachesIfNeed();
    delay = StorageService::StorageRadar::ReportDuration("DROP CACHE", startTime,
        StorageService::DELAY_TIME_THRESH_HIGH, StorageService::DEFAULT_USERID);
    LOGI("[L5:FscryptKeyV1] InactiveKey: <<< EXIT SUCCESS <<< dir=%{public}s", dir_.c_str());
    return ret;
}

void FscryptKeyV1::DropCachesIfNeed()
{
    LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: >>> ENTER <<<");
    DIR *dir = opendir(MNT_DATA);
    if (dir != nullptr) {
        int fd = dirfd(dir);
        LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: open /data dir fd success");
        if (fd < 0 || syncfs(fd)) {
            LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: syncfs failed, errno=%{public}d", errno);
            sync();
        }
        (void)closedir(dir);
    } else {
        sync();
        LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: Failed to open directory");
    }
    LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: syncfs success");
    if (!SaveStringToFile("/proc/sys/vm/drop_caches", "2")) {
        LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: Failed to drop cache during key eviction");
    }
    LOGE("[L5:FscryptKeyV1] DropCachesIfNeed: <<< EXIT SUCCESS <<<");
}

int32_t FscryptKeyV1::LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    LOGI("[L5:FscryptKeyV1] LockUserScreen: >>> ENTER <<< flag=%{public}u, sdpClass=%{public}u", flag, sdpClass);
    // uninstall KeyRing
    int errNo = UninstallKeyToKeyring();
    if (errNo != E_OK) {
        LOGE("[L5:FscryptKeyV1] LockUserScreen: <<< EXIT FAILED <<< UninstallKeyToKeyring failed");
        return errNo;
    }

    // uninstall FBE
    uint32_t elType;
    int32_t ret = fscryptV1Ext.LockUserScreenExt(flag, elType);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] LockUserScreen: <<< EXIT FAILED <<< LockUserScreenExt failed");
        return ret;
    }
    LOGI("[L5:FscryptKeyV1] LockUserScreen: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::LockUece(bool &isFbeSupport)
{
    LOGD("[L5:FscryptKeyV1] LockUece: >>> ENTER <<<");
    int32_t ret = fscryptV1Ext.LockUeceExt(isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L5:FscryptKeyV1] LockUece: <<< EXIT FAILED <<< LockUeceExt failed");
        return ret;
    }
    LOGI("[L5:FscryptKeyV1] LockUece: <<< EXIT SUCCESS <<<");
    return ret;
}

int32_t FscryptKeyV1::UninstallKeyToKeyring()
{
    LOGI("[L5:FscryptKeyV1] UninstallKeyToKeyring: >>> ENTER <<<");
    if (keyInfo_.keyDesc.IsEmpty() && !LoadKeyBlob(keyInfo_.keyDesc, dir_ + PATH_KEYDESC)) {
        LOGE("[L5:FscryptKeyV1] UninstallKeyToKeyring: <<< EXIT FAILED <<< Load keyDesc failed");
        return E_KEY_LOAD_ERROR;
    }
    if (keyInfo_.keyDesc.IsEmpty()) {
        DropCachesIfNeed();
        LOGE("[L5:FscryptKeyV1] UninstallKeyToKeyring: keyDesc is null, key not installed");
        return E_KEY_EMPTY_ERROR;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGE("[L5:FscryptKeyV1] UninstallKeyToKeyring: Error searching session keyring");
        std::string extraData = "cmd=KEY_SPEC_SESSION_KEYRING,errno=" + std::to_string(errno);
        StorageRadar::ReportKeyRingResult("UninstallKeyToKeyring::KeyCtrlSearch", krid, extraData);
        return E_SEARCH_SESSION_KEYING_ERROR;
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyInfo_.keyDesc.ToString();
        key_serial_t ks = KeyCtrlSearch(krid, "logon", keyref.c_str(), 0);
        if (KeyCtrlUnlink(ks, krid) != 0) {
            LOGE("[L5:FscryptKeyV1] UninstallKeyToKeyring: Failed to unlink key");
        }
    }

    LOGW("[L5:FscryptKeyV1] UninstallKeyToKeyring: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::GenerateKeyDesc()
{
    LOGD("[L5:FscryptKeyV1] GenerateKeyDesc: >>> ENTER <<<");
    if (keyInfo_.key.IsEmpty()) {
        LOGE("[L5:FscryptKeyV1] GenerateKeyDesc: <<< EXIT FAILED <<< key is empty");
        return E_KEY_EMPTY_ERROR;
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
        LOGE("[L5:FscryptKeyV1] GenerateKeyDesc: <<< EXIT FAILED <<< memcpy failed, err=%{public}d", err);
        return err;
    }
    LOGI("[L5:FscryptKeyV1] GenerateKeyDesc: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1::GenerateAppKeyDesc(KeyBlob appKey)
{
    LOGI("[L5:FscryptKeyV1] GenerateAppKeyDesc: >>> ENTER <<<");
    if (appKey.IsEmpty()) {
        LOGE("[L5:FscryptKeyV1] GenerateAppKeyDesc: <<< EXIT FAILED <<< key is empty");
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
        LOGE("[L5:FscryptKeyV1] GenerateAppKeyDesc: <<< EXIT FAILED <<< memcpy failed, err=%{public}d", err);
        return err;
    }
    LOGE("[L5:FscryptKeyV1] GenerateAppKeyDesc: keyDescLen=%{public}u, <<< EXIT SUCCESS <<<", keyInfo_.keyDesc.size);
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS

