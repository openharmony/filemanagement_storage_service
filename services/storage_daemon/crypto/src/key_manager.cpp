/*
 * Copyright (C) 2022-2026 Huawei Device Co., Ltd.
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

#include "key_manager.h"

#include <fcntl.h>
#include <future>
#include "directory_ex.h"
#include <dirent.h>
#include "file_ex.h"
#include "fscrypt_key_v1.h"
#include "fscrypt_key_v2.h"
#include "iam_client.h"
#include "libfscrypt/fscrypt_control.h"
#include "recover_manager.h"
#include "storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "user/mount_constant.h"
#include "user/user_manager.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
const UserAuth NULL_KEY_AUTH = {};
constexpr const char *BACKUP_NAME = "_bak";
constexpr const char *DEFAULT_NEED_RESTORE_VERSION = "1";
constexpr const char *DEFAULT_NEED_RESTORE_UPDATE_VERSION = "3";
constexpr const char *UECE_PATH = "/dev/fbex_uece";
constexpr const char *DATA_DIR = "data/app/";
constexpr const char *SERVICE_DIR = "data/service/";
constexpr const char *ENCRYPT_VERSION_DIR = "/latest/encrypted";
constexpr const char *SEC_DISCARD_DIR = "/latest/sec_discard";
constexpr const char *NEED_UPDATE_DIR = "/latest/need_update";
constexpr const char *SHIELD_DIR = "/latest/shield";
constexpr const char *DESC_DIR = "/key_desc";
constexpr const char *EL2_ENCRYPT_TMP_FILE = "/el2_tmp";
constexpr uint32_t KEY_RECOVERY_USER_ID = 300;

constexpr const char *SERVICE_STORAGE_DAEMON_DIR = "/data/service/el1/public/storage_daemon";
constexpr const char *FSCRYPT_EL_DIR = "/data/service/el1/public/storage_daemon/sd";
constexpr uint32_t RECOVERY_TOKEN_CHALLENGE_LENG = 32;

constexpr int LOCK_STATUS_START = 0;
constexpr int LOCK_STATUS_END = 1;
constexpr uint8_t RETRIEVE_KEY = 0x0;
constexpr uint8_t FIRST_CREATE_KEY = 0x6c;
constexpr uint8_t USER_LOGOUT = 0x0;
constexpr uint32_t USER_ADD_AUTH = 0x0;
constexpr uint32_t USER_CHANGE_AUTH = 0x1;

constexpr uint32_t FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG = 0xFBE30034;

#ifdef EL5_FILEKEY_MANAGER
constexpr int32_t WAIT_THREAD_TIMEOUT_MS = 500;
#endif

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
const std::string FILE_BASED_ENCRYPT_SRC_PATH = "/mnt/data_old/service/el1/public/sec_storage_data/fbe3";
const std::string FILE_BASED_ENCRYPT_DST_PATH = "/data/service/el1/public/sec_storage_data/fbe3";
constexpr int32_t DEFAULT_REPAIR_USERID = 10736;
#endif

static bool IsEncryption()
{
#ifdef SUPPORT_RECOVERY_KEY_SERVICE
    static bool isEncryption = RecoveryManager::GetInstance().IsEncryptionEnabled();
    return isEncryption;
#endif
    return true;
}
std::shared_ptr<BaseKey> KeyManager::GetBaseKey(const std::string& dir)
{
    uint8_t versionFromPolicy = GetFscryptVersionFromPolicy();
    uint8_t kernelSupportVersion = KeyCtrlGetFscryptVersion(MNT_DATA);
    if (kernelSupportVersion != FSCRYPT_V1 && kernelSupportVersion != FSCRYPT_V2) {
        LOGE("[L3:KeyManager] GetBaseKey: <<< EXIT FAILED <<< [kernel does not support fscrypt, ret=%{public}d,"
            "errno=%{public}d]", kernelSupportVersion, errno);
        return nullptr;
    }
    if ((versionFromPolicy == kernelSupportVersion) && (kernelSupportVersion == FSCRYPT_V2)) {
        return std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>(dir));
    }
    if (versionFromPolicy != kernelSupportVersion) {
        LOGE("[L3:KeyManager] GetBaseKey: version from policy %{public}u not same as version from kernel %{public}u",
            versionFromPolicy, kernelSupportVersion);
    }
    return std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV1>(dir));
}

int KeyManager::GenerateAndInstallDeviceKey(const std::string &dir)
{
    LOGW("[L3:KeyManager] GenerateAndInstallDeviceKey: >>> ENTER <<< [dir=%{public}s]", dir.c_str());
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] GenerateAndInstallDeviceKey: fscrypt syspara not found or encryption not enabled");
        return E_OK;
    }
    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ == nullptr) {
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_NULLPTR, "EL1", "globalEl1Key_ is null");
        return E_GLOBAL_KEY_NULLPTR;
    }

    if (globalEl1Key_->InitKey(true) == false) {
        globalEl1Key_ = nullptr;
        LOGE("[L3:KeyManager] GenerateAndInstallDeviceKey: <<< EXIT FAILED <<< [global security key initialization"
            "failed]");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_INIT_ERROR, "EL1", "InitKey failed");
        return E_GLOBAL_KEY_INIT_ERROR;
    }
    auto ret = globalEl1Key_->StoreKey(NULL_KEY_AUTH);
    if (ret != E_OK) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("[L3:KeyManager] GenerateAndInstallDeviceKey: <<< EXIT FAILED <<< [global security key storage failed]");
        std::string extraData = "StoreKey ret = " + std::to_string(ret);
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_STORE_ERROR,
            "EL1", extraData);
        return E_GLOBAL_KEY_STORE_ERROR;
    }
    ret = globalEl1Key_->ActiveKey({}, FIRST_CREATE_KEY);
    if (ret != E_OK) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("[L3:KeyManager] GenerateAndInstallDeviceKey: <<< EXIT FAILED <<< [global security key activation"
            "failed]");
        std::string extraData = "ActiveKey ret = " + std::to_string(ret);
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_ACTIVE_ERROR,
            "EL1", extraData);
        return E_GLOBAL_KEY_ACTIVE_ERROR;
    }
    ret = globalEl1Key_->UpdateKey();
    if (ret != E_OK) {
        std::string extraData = "UpdateKey ret = " + std::to_string(ret);
        LOGE("global security key update failed");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_UPDATE_ERROR,
            "EL1", extraData);
    }
    hasGlobalDeviceKey_ = true;
    LOGW("[L3:KeyManager] GenerateAndInstallDeviceKey: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::RestoreDeviceKey(const std::string &dir)
{
    LOGI("[L3:KeyManager] RestoreDeviceKey: >>> ENTER <<< [dir=%{public}s]", dir.c_str());
    if (globalEl1Key_ != nullptr) {
        LOGI("[L3:KeyManager] RestoreDeviceKey: <<< EXIT SUCCESS <<< [device key already exists]");
        return 0;
    }

    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ == nullptr) {
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_NULLPTR, "EL1", "");
        return E_GLOBAL_KEY_NULLPTR;
    }

    if (globalEl1Key_->InitKey(false) == false) {
        globalEl1Key_ = nullptr;
        LOGE("[L3:KeyManager] RestoreDeviceKey: <<< EXIT FAILED <<< [global security key initialization failed]");
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_INIT_ERROR, "EL1", "");
        return E_GLOBAL_KEY_INIT_ERROR;
    }

    auto ret = globalEl1Key_->RestoreKey(NULL_KEY_AUTH);
    if (ret != E_OK) {
        globalEl1Key_ = nullptr;
        LOGE("[L3:KeyManager] RestoreDeviceKey: <<< EXIT FAILED <<< [global security key restore failed]");
        std::string extraData = "RestoreKey ret = " + std::to_string(ret);
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_STORE_ERROR, "EL1", extraData);
        return E_GLOBAL_KEY_STORE_ERROR;
    }
    ret = globalEl1Key_->ActiveKey({}, RETRIEVE_KEY);
    if (ret != E_OK) {
        globalEl1Key_ = nullptr;
        LOGE("[L3:KeyManager] RestoreDeviceKey: <<< EXIT FAILED <<< [global security key activation failed]");
        std::string extraData = "ActiveKey ret = " + std::to_string(ret);
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_ACTIVE_ERROR, "EL1", extraData);
        return E_GLOBAL_KEY_ACTIVE_ERROR;
    }
    hasGlobalDeviceKey_ = true;
    LOGI("[L3:KeyManager] RestoreDeviceKey: <<< EXIT SUCCESS <<< [retval=0]");

    return 0;
}

int KeyManager::InitGlobalDeviceKey(void)
{
    LOGW("[L3:KeyManager] InitGlobalDeviceKey: >>> ENTER <<<");
    int ret = InitFscryptPolicy();
    if (ret < 0) {
        LOGE("[L3:KeyManager] InitGlobalDeviceKey: <<< EXIT FAILED <<< [fscrypt initialization failed, fscrypt will"
            "not be enabled]");
        StorageRadar::ReportUserKeyResult("InitGlobalDeviceKey::InitFscryptPolicy", 0, ret, "EL1", "");
        return ret;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    if (hasGlobalDeviceKey_ || globalEl1Key_ != nullptr) {
        LOGI("[L3:KeyManager] InitGlobalDeviceKey: global device el1 already exists");
        return 0;
    }

    ret = MkDir(STORAGE_DAEMON_DIR, S_IRWXU); // para.0700: root only
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitGlobalDeviceKey: <<< EXIT FAILED <<< [failed to create storage daemon directory]");
        StorageRadar::ReportUserKeyResult("InitGlobalDeviceKey::MkDir", 0, ret, "EL1",
            std::string("errno = ") + std::to_string(errno) + ", path = " + STORAGE_DAEMON_DIR);
        return ret;
    }
    std::error_code errCode;
    if (std::filesystem::exists(DEVICE_EL1_DIR, errCode) && !std::filesystem::is_empty(DEVICE_EL1_DIR)) {
        UpgradeKeys({{0, DEVICE_EL1_DIR}});
        return RestoreDeviceKey(DEVICE_EL1_DIR);
    }
    ret = MkDir(DEVICE_EL1_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitGlobalDeviceKey: <<< EXIT FAILED <<< [failed to create device el1 key directory]");
        StorageRadar::ReportUserKeyResult("InitGlobalDeviceKey::MkDir", 0, ret, "EL1",
            std::string("errno = ") + std::to_string(errno) + ", path = " + DEVICE_EL1_DIR);
        return ret;
    }

    return GenerateAndInstallDeviceKey(DEVICE_EL1_DIR);
}

int KeyManager::GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGW("[L3:KeyManager] GenerateAndInstallUserKey: >>> ENTER <<< [userId=%{public}u, type=%{public}u]", userId, type);
    if (HasElkey(userId, type)) {
        LOGI("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT SUCCESS <<< [key already exists]");
        return 0;
    }
    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        return E_GLOBAL_KEY_NULLPTR;
    }
    if (type == EL5_KEY) {
        return GenerateAndInstallEl5Key(userId, dir, auth);
    }
    if (elKey->InitKey(true) == false) {
        LOGE("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT FAILED <<< [user security key initialization"
            "failed]");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallUserKey", userId, E_ELX_KEY_INIT_ERROR, "",
            "InitKey failed, userId=" + std::to_string(userId) + ", type=" + std::to_string(type));
        return E_ELX_KEY_INIT_ERROR;
    }
    auto ret = elKey->StoreKey(auth);
    if (ret != E_OK) {
        if (!elKey->ClearKey()) {
            LOGE("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT FAILED <<< Failed to clear key"
                "after store error");
        }
        LOGE("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT FAILED <<< [user security key storage failed]");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallUserKey", userId, E_ELX_KEY_STORE_ERROR, "",
            "StoreKey failed, userId=" + std::to_string(userId) + ", type=" + std::to_string(type) +
            ", ret=" + std::to_string(ret));
        return E_ELX_KEY_STORE_ERROR;
    }
    // Generate hashkey for encrypt public directory
    elKey->GenerateHashKey();
    ret = elKey->ActiveKey(auth.token, FIRST_CREATE_KEY);
    if (ret != E_OK) {
        if (!elKey->ClearKey()) {
            LOGE("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT FAILED <<< Failed to clear"
                "key after active error");
        }
        LOGE("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT FAILED <<< [user security key activation failed]");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallUserKey", userId, E_ELX_KEY_ACTIVE_ERROR, "",
            "ActiveKey failed, userId=" + std::to_string(userId) + ", type=" + std::to_string(type) +
            ", ret=" + std::to_string(ret));
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    (void)elKey->UpdateKey();
    if (type >= EL1_KEY && type < EL5_KEY) {
        SaveUserElKey(userId, type, elKey);
    }
    LOGI("[L3:KeyManager] GenerateAndInstallUserKey: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::GenerateAndInstallEl5Key(uint32_t userId, const std::string &dir, const UserAuth &auth)
{
    LOGI("[L3:KeyManager] GenerateAndInstallEl5Key: >>> ENTER <<< [userId=%{public}u]", userId);
    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        LOGE("[L3:KeyManager] GenerateAndInstallEl5Key: <<< EXIT FAILED <<< [failed to get base key]");
        return E_GLOBAL_KEY_NULLPTR;
    }
    bool isNeedEncryptClassE = true;
    saveESecretStatus[userId] = true;
    auto ret = elKey->AddClassE(isNeedEncryptClassE, saveESecretStatus[userId], FIRST_CREATE_KEY);
    if (ret != E_OK) {
        if (elKey->ClearKey() != E_OK) {
            LOGW("[L3:KeyManager] GenerateAndInstallEl5Key: <<< EXIT FAILED <<< user %{public}u"
                "ClearKey failed after AddClassE error", userId);
        }
        StorageRadar::ReportUserKeyResult("GenerateAndInstallEl5Key", userId, E_EL5_ADD_CLASS_ERROR, "",
            "AddClassE failed, userId=" + std::to_string(userId) + ", ret=" + std::to_string(ret));
        LOGE("[L3:KeyManager] GenerateAndInstallEl5Key: <<< EXIT FAILED <<< [failed to create EL5 key for user"
            "%{public}u, error=%{public}d]", userId, ret);
        return E_EL5_ADD_CLASS_ERROR;
    }
    std::string keyDir = GetKeyDirByUserAndType(userId, EL5_KEY);
    if (keyDir == "") {
        LOGE("[L3:KeyManager] GenerateAndInstallEl5Key: <<< EXIT FAILED <<< [invalid key directory]");
        return E_KEY_TYPE_INVALID;
    }
    if (!saveESecretStatus[userId]) {
        OHOS::ForceRemoveDirectory(keyDir);
    }
    saveESecretStatus[userId] = (!auth.secret.IsEmpty() && !auth.token.IsEmpty());
    if (isNeedEncryptClassE) {
        if (!auth.secret.IsEmpty() && !auth.token.IsEmpty()) {
            auto ret = elKey->EncryptClassE(auth, saveESecretStatus[userId], userId, USER_ADD_AUTH);
            if (ret != E_OK) {
                elKey->ClearKey();
                LOGE("[L3:KeyManager] GenerateAndInstallEl5Key: <<< EXIT FAILED <<< [failed to encrypt class E for"
                    "user %{public}u]", userId);
                StorageRadar::ReportUserKeyResult("GenerateAndInstallEl5Key", userId, E_EL5_ENCRYPT_CLASS_ERROR, "",
                    "EncryptClassE failed, userId=" + std::to_string(userId) + ", ret=" + std::to_string(ret));
                return E_EL5_ENCRYPT_CLASS_ERROR;
            }
        }
    } else {
        bool eBufferStatue = false;
        auto ret = elKey->DecryptClassE(auth, saveESecretStatus[userId], eBufferStatue, userId, false);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] GenerateAndInstallEl5Key: failed to decrypt class E for user %{public}u", userId);
            StorageRadar::ReportUserKeyResult("GenerateAndInstallEl5Key", userId, ret, "",
                "DecryptClassE failed, userId=" + std::to_string(userId));
        }
    }
    SaveUserElKey(userId, EL5_KEY, elKey);
    LOGI("[L3:KeyManager] GenerateAndInstallEl5Key: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGI("[L3:KeyManager] RestoreUserKey: >>> ENTER <<< [userId=%{public}u, type=%{public}u]", userId, type);
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (HasElkey(userId, type)) {
        LOGI("[L3:KeyManager] RestoreUserKey: <<< EXIT SUCCESS <<< [key already exists]");
        return E_OK;
    }

    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        LOGE("[L3:KeyManager] RestoreUserKey: <<< EXIT FAILED <<< [failed to get base key]");
        return E_GLOBAL_KEY_NULLPTR;
    }

    if (elKey->InitKey(false) == false) {
        LOGE("[L3:KeyManager] RestoreUserKey: <<< EXIT FAILED <<< [user security key initialization failed]");
        StorageRadar::ReportUserKeyResult("RestoreUserKey", userId, E_ELX_KEY_INIT_ERROR, "",
            "InitKey failed, userId=" + std::to_string(userId) + ", type=" + std::to_string(type));
        return E_ELX_KEY_INIT_ERROR;
    }

    auto ret = elKey->RestoreKey(auth);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] RestoreUserKey: <<< EXIT FAILED <<< [user security key restore failed]");
        StorageRadar::ReportUserKeyResult("RestoreUserKey", userId, E_ELX_KEY_STORE_ERROR, "",
            "RestoreKey failed, userId=" + std::to_string(userId) + ", type=" + std::to_string(type) +
            ", ret=" + std::to_string(ret));
        return E_ELX_KEY_STORE_ERROR;
    }

    ret = elKey->ActiveKey(auth.token, RETRIEVE_KEY);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] RestoreUserKey: <<< EXIT FAILED <<< [user security key activation failed]");
        StorageRadar::ReportUserKeyResult("RestoreUserKey", userId, E_ELX_KEY_ACTIVE_ERROR, "",
            "ActiveKey failed, userId=" + std::to_string(userId) + ", type=" + std::to_string(type) +
            ", ret=" + std::to_string(ret));
        return E_ELX_KEY_ACTIVE_ERROR;
    }

    SaveUserElKey(userId, type, elKey);
    LOGI("[L3:KeyManager] RestoreUserKey: <<< EXIT SUCCESS <<< [retval=0]");

    return E_OK;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int32_t KeyManager::ClearAppCloneUserNeedRestore(unsigned int userId, std::string elNeedRestorePath)
{
    LOGI("[L3:KeyManager] ClearAppCloneUserNeedRestore: >>> ENTER <<< [userId=%{public}u]", userId);
    if (userId < StorageService::START_APP_CLONE_USER_ID || userId >= StorageService::MAX_APP_CLONE_USER_ID) {
        LOGI("[L3:KeyManager] ClearAppCloneUserNeedRestore: userId %{public}d out of range", userId);
        return E_USERID_RANGE;
    }

    LOGE("[L3:KeyManager] ClearAppCloneUserNeedRestore: user %{public}d is app clone user, deleting elx need_restore",
        userId);
    std::error_code errCode;
    if (!std::filesystem::exists(elNeedRestorePath, errCode)) {
        LOGI("[L3:KeyManager] ClearAppCloneUserNeedRestore: need_restore does not exist, no need to delete");
        return E_OK;
    }
    (void)remove(elNeedRestorePath.c_str());
    LOGI("[L3:KeyManager] ClearAppCloneUserNeedRestore: <<< EXIT SUCCESS <<< [need_restore deleted]");
    return E_OK;
}
#endif

bool KeyManager::HasElkey(uint32_t userId, KeyType type)
{
    if (userElKeys_.find(userId) != userElKeys_.end()) {
        if (userElKeys_[userId].find(type) != userElKeys_[userId].end()) {
            LOGI("[L3:KeyManager] HasElkey: user %{public}u el %{public}u already exists", userId, type);
            return true;
        }
    }
    LOGE("[L3:KeyManager] HasElkey: key not found for user %{public}u, type %{public}u", userId, type);
    return false;
}

bool KeyManager::IsNeedClearKeyFile(std::string file)
{
    LOGI("[L3:KeyManager] IsNeedClearKeyFile: >>> ENTER <<< [file=%{public}s]", file.c_str());
    std::error_code errCode;
    if (!std::filesystem::exists(file, errCode)) {
        LOGE("[L3:KeyManager] IsNeedClearKeyFile: <<< EXIT <<< [file does not exist, file=%{private}s]", file.c_str());
        return false;
    }

    std::string version;
    if (!OHOS::LoadStringFromFile(file, version)) {
        LOGE("[L3:KeyManager] IsNeedClearKeyFile: <<< EXIT <<< [failed to load string from file=%{private}s]",
            file.c_str());
        return false;
    }

    if (version != DEFAULT_NEED_RESTORE_VERSION && version != DEFAULT_NEED_RESTORE_UPDATE_VERSION) {
        LOGE("[L3:KeyManager] IsNeedClearKeyFile: <<< EXIT <<< [need to clear, file=%{private}s, version=%{public}s]",
            file.c_str(), version.c_str());
        return true;
    }
    LOGE("[L3:KeyManager] IsNeedClearKeyFile: <<< EXIT SUCCESS <<< [no need to clear, file=%{private}s,"
        "version=%{public}s]", file.c_str(), version.c_str());
    return false;
}

void KeyManager::ClearKeyFilesForPath(const std::string &path)
{
    std::vector<std::string> filesToDelete = {
        path + FSCRYPT_VERSION_DIR,
        path + DESC_DIR,
        path + ENCRYPT_VERSION_DIR,
        path + NEED_UPDATE_DIR,
        path + SEC_DISCARD_DIR,
        path + SHIELD_DIR
    };

    for (const auto &file : filesToDelete) {
        if (remove(file.c_str()) != 0 && errno != ENOENT) {
            LOGE("[L3:KeyManager] ClearKeyFilesForPath:Failed to delete %{public}s,"
                "errno: %{public}d", file.c_str(), errno);
        }
    }
    if (!IsDirRecursivelyEmpty(path.c_str())) {
        return;
    }
    if (remove(path.c_str()) != 0 && errno != ENOENT) {
        LOGE("[L3:KeyManager] ClearKeyFilesForPath:Failed to delete dir %{public}s,"
            "errno: %{public}d", path.c_str(), errno);
    }
}

void KeyManager::ProcUpgradeKey(const std::vector<FileList> &dirInfo)
{
    LOGI("[L3:KeyManager] ProcUpgradeKey: >>> ENTER <<< [count=%{public}zu]", dirInfo.size());
    for (const auto &it : dirInfo) {
        std::string needRestorePath = it.path + "/latest/need_restore";
        if (IsNeedClearKeyFile(needRestorePath)) {
            StorageRadar::ReportUserKeyResult("ProcUpgradeKey::IsNeedClearKeyFile", it.userId,
                E_OK, "ELx", "user elx path: " + it.path);
            ClearKeyFilesForPath(it.path);
        }
    }
}

int KeyManager::LoadAllUsersEl1Key(void)
{
    LOGI("[L3:KeyManager] LoadAllUsersEl1Key: >>> ENTER <<<");
    int ret = E_OK;
    std::vector<FileList> dirInfo;
    ReadDigitDir(USER_EL2_DIR, dirInfo);
    UpgradeKeys(dirInfo);
    dirInfo.clear();
    ReadDigitDir(USER_EL1_DIR, dirInfo);
    UpgradeKeys(dirInfo);
    for (auto &item : dirInfo) {
        ret = RestoreUserKey(item.userId, item.path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] LoadAllUsersEl1Key: failed to restore el1 key for user %{public}u", item.userId);
            StorageRadar::ReportUserKeyResult("LoadAllUsersEl1Key::RestoreUserKey", item.userId,
                ret, "EL1", "user el1 path = " + item.path);
        }
    }

    /* only for el3/el4 upgrade scene */
    dirInfo.clear();
    ReadDigitDir(USER_EL3_DIR, dirInfo);
    ProcUpgradeKey(dirInfo);
    dirInfo.clear();
    ReadDigitDir(USER_EL4_DIR, dirInfo);
    ProcUpgradeKey(dirInfo);
    dirInfo.clear();
    ReadDigitDir(USER_EL5_DIR, dirInfo);
    ProcUpgradeKey(dirInfo);
    LOGI("[L3:KeyManager] LoadAllUsersEl1Key: <<< EXIT SUCCESS <<< [retval=%{public}d]", ret);
    return ret;
}

int KeyManager::InitUserElkeyStorageDir(void)
{
    int ret = MkDir(SERVICE_STORAGE_DAEMON_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create service storage daemon directory");
        return ret;
    }

    ret = MkDir(FSCRYPT_EL_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create fscrypt el directory");
        return ret;
    }

    ret = MkDir(USER_EL1_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create EL1 storage directory");
        return ret;
    }
    ret = MkDir(USER_EL2_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create EL2 storage directory");
        return ret;
    }
    // 0700 means create el3 permissions
    ret = MkDir(USER_EL3_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create EL3 storage directory");
        return ret;
    }
    // 0700 means create el4 permissions
    ret = MkDir(USER_EL4_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create EL4 storage directory");
        return ret;
    }
    // 0700 means create el5 permissions
    ret = MkDir(USER_EL5_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("[L3:KeyManager] InitUserElkeyStorageDir: failed to create EL5 storage directory");
        return ret;
    }
    return 0;
}

int KeyManager::InitGlobalUserKeys(void)
{
    LOGW("[L3:KeyManager] InitGlobalUserKeys: >>> ENTER <<<");
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] InitGlobalUserKeys: fscrypt syspara not found or encryption not enabled");
        return 0;
    }
    int ret = InitUserElkeyStorageDir();
    if (ret) {
        LOGE("[L3:KeyManager] InitGlobalUserKeys: failed to initialize user el storage directory");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::InitUserElkeyStorageDir", GLOBAL_USER_ID,
            ret, "EL1", "");
        return ret;
    }

    std::string globalUserEl1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
    if (IsDir(globalUserEl1Path)) {
        ret = RestoreUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("[L3:KeyManager] InitGlobalUserKeys: failed to restore el1 key");
            StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::RestoreUserKey", GLOBAL_USER_ID,
                ret, "EL1", "global user el1 path = " + globalUserEl1Path);
            return ret;
        }
    } else {
        std::lock_guard<std::mutex> lock(keyMutex_);
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("[L3:KeyManager] InitGlobalUserKeys: failed to generate el1 key");
            StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::GenerateAndInstallUserKey", GLOBAL_USER_ID,
                ret, "EL1", "global user el1 path = " + globalUserEl1Path);
            return ret;
        }
    }

    ret = LoadAllUsersEl1Key();
    if (ret) {
        LOGE("[L3:KeyManager] InitGlobalUserKeys: failed to load all users el1 keys");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::LoadAllUsersEl1Key", GLOBAL_USER_ID,
            ret, "EL1", "Load all users el1 failed");
        return ret;
    }
    LOGW("[L3:KeyManager] InitGlobalUserKeys: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::GenerateUserKeys(unsigned int user, uint32_t flags)
{
    LOGW("[L3:KeyManager] GenerateUserKeys: >>> ENTER <<< [user=%{public}u, flags=%{public}u]", user, flags);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] GenerateUserKeys: fscrypt syspara not found or encryption not enabled");
        return 0;
    }
    if ((!IsDir(USER_EL1_DIR)) || (!IsDir(USER_EL2_DIR)) || (!IsDir(USER_EL3_DIR)) ||
        (!IsDir(USER_EL4_DIR)) || (!IsDir(USER_EL5_DIR))) {
        LOGI("[L3:KeyManager] GenerateUserKeys: <<< EXIT FAILED <<< [el storage directory does not exist]");
        return -ENOENT;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = GenerateElxAndInstallUserKey(user);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] GenerateUserKeys: <<< EXIT FAILED <<< [failed to generate ELX keys]");
        return ret;
    }
    LOGW("[L3:KeyManager] GenerateUserKeys: <<< EXIT SUCCESS <<< [retval=0]");
    return ret;
}

int KeyManager::GenerateElxAndInstallUserKey(unsigned int user)
{
    std::string el1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(user);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(user);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(user);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(user);
    std::string el5Path = std::string(USER_EL5_DIR) + "/" + std::to_string(user);
    if (IsDir(el1Path) || IsDir(el2Path) || IsDir(el3Path) || IsDir(el4Path) || IsDir(el5Path)) {
        return CheckAndFixUserKeyDirectory(user);
    }
    int ret = GenerateAndInstallUserKey(user, el1Path, NULL_KEY_AUTH, EL1_KEY);
    if (ret) {
        LOGE("[L3:KeyManager] GenerateElxAndInstallUserKey: failed to create el1 key for user %{public}u", user);
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL1", "el1Path = " + el1Path);
        return ret;
    }

    ret = GenerateAndInstallUserKey(user, el2Path, NULL_KEY_AUTH, EL2_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("[L3:KeyManager] GenerateElxAndInstallUserKey: failed to create el2 key for user %{public}u", user);
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL2", "el2Path = " + el2Path);
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el3Path, NULL_KEY_AUTH, EL3_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("[L3:KeyManager] GenerateElxAndInstallUserKey: failed to create el3 key for user %{public}u", user);
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL3", "el3Path = " + el3Path);
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el4Path, NULL_KEY_AUTH, EL4_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("[L3:KeyManager] GenerateElxAndInstallUserKey: failed to create el4 key for user %{public}u", user);
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL4", "el4Path = " + el4Path);
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el5Path, NULL_KEY_AUTH, EL5_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("[L3:KeyManager] GenerateElxAndInstallUserKey: failed to create el5 key for user %{public}u", user);
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL5", "el5Path = " + el5Path);
        return ret;
    }
    saveLockScreenStatus[user] = true;
    return ret;
}

int KeyManager::CheckAndFixUserKeyDirectory(unsigned int user)
{
    std::string el1NeedRestorePath = std::string(USER_EL1_DIR) + "/" + std::to_string(user) + RESTORE_DIR;
    std::error_code errCode;
    if (std::filesystem::exists(el1NeedRestorePath, errCode)) {
        LOGE("[L3:KeyManager] CheckAndFixUserKeyDirectory: el1 need_restore file exists, upgrade scene not supported");
        return -EEXIST;
    }
    int ret = GenerateIntegrityDirs(user, EL1_KEY);
    if (ret != -EEXIST) {
        LOGE("[L3:KeyManager] CheckAndFixUserKeyDirectory: failed to generate integrity dirs for el1");
        StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", user, ret, "EL1", "");
    }
    ret = GenerateIntegrityDirs(user, EL2_KEY);
    if (ret != -EEXIST) {
        LOGE("[L3:KeyManager] CheckAndFixUserKeyDirectory: failed to generate integrity dirs for el2");
        StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", user, ret, "EL2", "");
    }
    return ret;
}

int KeyManager::GenerateIntegrityDirs(int32_t userId, KeyType type)
{
    std::string dirType = (type == EL1_KEY) ? EL1 : EL2;
    std::string userDir = std::string(FSCRYPT_EL_DIR) + "/" + dirType;
    uint32_t flag_type = (type == EL1_KEY) ? IStorageDaemonEnum::CRYPTO_FLAG_EL1 : IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    std::string versionElx = userDir + "/" + std::to_string(userId) + FSCRYPT_VERSION_DIR;
    std::string encryptElx = userDir + "/" + std::to_string(userId) + ENCRYPT_VERSION_DIR;
    std::string discardElx = userDir + "/" + std::to_string(userId) + SEC_DISCARD_DIR;
    std::string shieldElx = userDir + "/" + std::to_string(userId) + SHIELD_DIR;
    std::error_code errCode;
    if (!std::filesystem::exists(versionElx, errCode) || !std::filesystem::exists(encryptElx, errCode) ||
        !std::filesystem::exists(shieldElx, errCode) || !std::filesystem::exists(discardElx, errCode) ||
        !IsWorkDirExist(dirType, userId)) {
        LOGE("[L3:KeyManager] GenerateIntegrityDirs: user %{public}u el %{public}d is not integrity, creation failed",
            userId, type);
        std::string extraData =
            "dir is not Integrity , userId =" + std::to_string(userId) + ", type = " + std::to_string(type);
        StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", userId, E_DIR_INTEGRITY_ERR, dirType,
                                          extraData);
        int ret = DoDeleteUserCeEceSeceKeys(userId, userDir, type);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] GenerateIntegrityDirs: failed to delete userId=%{public}d el %{public}d key",
                userId, type);
        }

        ret = GenerateUserKeyByType(userId, type, {}, {});
        if (ret != E_OK) {
            std::string extraData = "GenerateUserKeyByType failed, userId =" + std::to_string(userId) +
                                    ", type = " + std::to_string(type);
            StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", userId, ret, dirType, extraData);
            LOGE("[L3:KeyManager] GenerateIntegrityDirs: upgrade scene - failed to generate user key, userId"
                "%{public}d, KeyType %{public}d", userId, type);
            return ret;
        }

        LOGI("[L3:KeyManager] GenerateIntegrityDirs: attempting to destroy directory first, user %{public}d, Type"
            "%{public}d", userId, type);
        ret = UserManager::GetInstance().DestroyUserDirs(userId, flag_type);
        if (ret != E_OK) {
            std::string extraData = "DestroyUserDirs failed, userId =" + std::to_string(userId) +
                                    ", type = " + std::to_string(flag_type);
            StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", userId, ret, dirType, extraData);
        }
        ret = UserManager::GetInstance().PrepareUserDirs(userId, flag_type);
        if (ret != E_OK) {
            std::string extraData = "PrepareUserDirs failed, userId =" + std::to_string(userId) +
                                    ", type = " + std::to_string(flag_type);
            StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", userId, ret, dirType, extraData);
            LOGE("[L3:KeyManager] GenerateIntegrityDirs: upgrade scene - failed to prepare user dirs, userId"
                "%{public}d, type %{public}d", userId, type);
            return ret;
        }
    }
    LOGI("[L3:KeyManager] GenerateIntegrityDirs: userId=%{public}d el %{public}d directory exists, no need to fix",
        userId, type);
    return -EEXIST;
}

bool KeyManager::IsWorkDirExist(std::string type, int32_t userId)
{
    std::string dataDir = std::string(DATA_DIR) + type + "/" + std::to_string(userId);
    std::string serviceDir = std::string(SERVICE_DIR) + type + "/" + std::to_string(userId);
    std::error_code errCode;
    bool isExist = std::filesystem::exists(dataDir, errCode) && std::filesystem::exists(serviceDir, errCode);
    return isExist;
}

int KeyManager::GenerateUserKeyByType(unsigned int user, KeyType type,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] GenerateUserKeyByType: >>> ENTER <<< [user=%{public}u, type=%{public}u]", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] GenerateUserKeyByType: fscrypt syspara not found or encryption not enabled");
        return 0;
    }

    std::string elPath = GetKeyDirByType(type);
    if (!IsDir(elPath)) {
        LOGI("[L3:KeyManager] GenerateUserKeyByType: <<< EXIT FAILED <<< [el storage directory does not exist]");
        return -ENOENT;
    }

    std::string elUserKeyPath = elPath + "/" + std::to_string(user);
    if (IsDir(elUserKeyPath)) {
        LOGE("[L3:KeyManager] GenerateUserKeyByType: <<< EXIT FAILED <<< [user %{public}u el key already exists]",
            user);
        return -EEXIST;
    }
    uint64_t secureUid = { 0 };
    if (!secret.empty() && !token.empty()) {
        IamClient::GetInstance().GetSecureUid(user, secureUid);
        LOGE("[L3:KeyManager] GenerateUserKeyByType: token exists, secure uid obtained");
    }
    UserAuth auth = { .token = token, .secret = secret, .secureUid = secureUid };
    int ret = GenerateAndInstallUserKey(user, elUserKeyPath, auth, type);
    if (ret) {
        LOGE("[L3:KeyManager] GenerateUserKeyByType: <<< EXIT FAILED <<< [failed to create el key for user"
            "%{public}u, type=%{public}u]", user, type);
        StorageRadar::ReportUserKeyResult("GenerateUserKeyByType::GenerateAndInstallUserKey",
            user, ret, std::to_string(type), "user key path = " + elUserKeyPath);
        return ret;
    }
    LOGI("[L3:KeyManager] GenerateUserKeyByType: <<< EXIT SUCCESS <<< [user el key created, user=%{public}u,"
        "type=%{public}u]", user, type);

    return 0;
}

void KeyManager::DeleteElKey(unsigned int user, KeyType type)
{
    if (userElKeys_.find(user) == userElKeys_.end()) {
        LOGE("[L3:KeyManager] DeleteElKey: user %{public}u does not exist", user);
        return;
    }
    if (userElKeys_[user].find(type) == userElKeys_[user].end()) {
        LOGE("[L3:KeyManager] DeleteElKey: el%{public}u does not exist", type);
        return;
    }
    userElKeys_[user].erase(type);
    if (userElKeys_[user].empty()) {
        userElKeys_.erase(user);
    }
}

int KeyManager::DoDeleteUserCeEceSeceKeys(unsigned int user, const std::string userDir, KeyType type)
{
    LOGI("[L3:KeyManager] DoDeleteUserCeEceSeceKeys: >>> ENTER <<< [user=%{public}u, userDir=%{public}s]",
        user, userDir.c_str());
    int ret = 0;
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (userDir == USER_EL1_DIR) {
        std::string elNeedRestorePath = std::string(USER_EL1_DIR) + "/" + std::to_string(user) + RESTORE_DIR;
        (void)ClearAppCloneUserNeedRestore(user, elNeedRestorePath);
    }
#endif
    if (HasElkey(user, type)) {
        auto elKey = userElKeys_[user][type];
        if (!elKey->ClearKey()) {
            LOGE("[L3:KeyManager] DoDeleteUserCeEceSeceKeys: failed to clear key");
            ret = E_CLEAR_KEY_FAILED;
            StorageRadar::ReportUserKeyResult("DoDeleteUserCeEceSeceKeys", user, E_CLEAR_KEY_FAILED, "",
                "clear key failed");
        }
        DeleteElKey(user, type);
        saveLockScreenStatus.erase(user);
    } else {
        std::string elPath = userDir + "/" + std::to_string(user);
        std::shared_ptr<BaseKey> elKey = GetBaseKey(elPath);
        if (elKey == nullptr) {
            LOGE("[L3:KeyManager] DoDeleteUserCeEceSeceKeys: memory allocation for base key failed");
            StorageRadar::ReportUserKeyResult("DoDeleteUserCeEceSeceKeys", user, E_PARAMS_NULLPTR_ERR, "",
                "Malloc el1 Basekey memory failed");
            return E_PARAMS_NULLPTR_ERR;
        }
        if (!elKey->ClearKey()) {
            LOGE("[L3:KeyManager] DoDeleteUserCeEceSeceKeys: failed to clear key");
            ret = E_CLEAR_KEY_FAILED;
            StorageRadar::ReportUserKeyResult("DoDeleteUserCeEceSeceKeys", user, E_CLEAR_KEY_FAILED, "",
                "clear key failed");
        }
    }
    LOGI("[L3:KeyManager] DoDeleteUserCeEceSeceKeys: <<< EXIT <<< [retval=%{public}d]", ret);
    return ret;
}

int KeyManager::DoDeleteUserKeys(unsigned int user)
{
    int errCode = 0;
    KeyType types[] = { EL1_KEY, EL2_KEY, EL3_KEY, EL4_KEY, EL5_KEY };
    constexpr const char *USER_DIRS[] = {
        USER_EL1_DIR,
        USER_EL2_DIR,
        USER_EL3_DIR,
        USER_EL4_DIR,
        USER_EL5_DIR
    };
    int size = sizeof(types) / sizeof(types[0]);
    std::string el("El");
    const std::string elxPath("elx path=");
    for (int i = 0; i < size; ++i) {
        if (types[i] == EL5_KEY && IsUeceSupportWithErrno() == ENOENT) {
            continue;
        }
        int deleteRet = DoDeleteUserCeEceSeceKeys(user, GetKeyDirByType(types[i]), types[i]);
        if (deleteRet != 0) {
            LOGE("[L3:KeyManager] DoDeleteUserKeys: failed to delete el%{public}d key", i + 1);
            errCode = deleteRet;
            StorageRadar::ReportUserKeyResult("DoDeleteUserKeys", user, errCode,
                "El" + std::to_string(i + 1), elxPath + USER_DIRS[i]);
        }
    }
    return errCode;
}

int KeyManager::DeleteUserKeys(unsigned int user)
{
    LOGI("[L3:KeyManager] DeleteUserKeys: >>> ENTER <<< [user=%{public}u]", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] DeleteUserKeys: fscrypt syspara not found or encryption not enabled");
        return 0;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoDeleteUserKeys(user);
    LOGI("[L3:KeyManager] DeleteUserKeys: <<< EXIT SUCCESS <<< [retval=%{public}d]", ret);

    auto userTask = userLockScreenTask_.find(user);
    if (userTask != userLockScreenTask_.end()) {
        userLockScreenTask_.erase(userTask);
        LOGI("[L3:KeyManager] DeleteUserKeys: deleted user %{public}u task", user);
    }
    return ret;
}

int KeyManager::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    LOGI("[L3:KeyManager] EraseAllUserEncryptedKeys: >>> ENTER <<< [count=%{public}zu]", localIdList.size());
    bool isDeleteFailed = false;
    if (localIdList.empty()) {
        LOGI("[L3:KeyManager] EraseAllUserEncryptedKeys: localIdList is empty");
    }
    for (const auto &localId : localIdList) {
        LOGI("[L3:KeyManager] EraseAllUserEncryptedKeys: deleting keys for user=%{public}d", localId);
        int32_t ret = DeleteUserKeys(localId);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] EraseAllUserEncryptedKeys: failed to erase keys for user=%{public}d", localId);
            StorageRadar::ReportUserKeyResult("KeyManager::EraseAllUserEncryptedKeys", localId, ret,
                "ELx", "");
            isDeleteFailed = true;
        }
    }
    DeleteUserKeys(ZERO_USER);
    DeleteGlobalDeviceKey(DEVICE_EL1_DIR);
    LOGI("[L3:KeyManager] EraseAllUserEncryptedKeys: <<< EXIT <<< [lastError=%{public}d]", isDeleteFailed);
    return isDeleteFailed ? E_ERASE_USER_KEY_ERROR : E_OK;
}

int KeyManager::DeleteGlobalDeviceKey(const std::string &dir)
{
    LOGI("[L3:KeyManager] DeleteGlobalDeviceKey: >>> ENTER <<<");
    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ != nullptr) {
        if (globalEl1Key_->ClearKey() != E_OK) {
            LOGW("[L3:KeyManager] DeleteGlobalDeviceKey:ClearKey failed in DeleteGlobalDeviceKey");
        }
        globalEl1Key_ = nullptr;
        hasGlobalDeviceKey_ = false;
        LOGE("[L3:KeyManager] DeleteGlobalDeviceKey: global security key cleared");
    }
    std::string backupDir = dir + BACKUP_NAME;
    globalEl1Key_ = GetBaseKey(backupDir);
    if (globalEl1Key_ != nullptr) {
        if (globalEl1Key_->ClearKey() != E_OK) {
            LOGW("[L3:KeyManager] DeleteGlobalDeviceKey:ClearKey failed for backup key in DeleteGlobalDeviceKey");
        }
        globalEl1Key_ = nullptr;
        hasGlobalDeviceKey_ = false;
        LOGE("[L3:KeyManager] DeleteGlobalDeviceKey: global security backup key cleared");
    }
    LOGI("[L3:KeyManager] DeleteGlobalDeviceKey: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::string BuildSecretStatus(struct UserTokenSecret &userTokenSecret)
{
    std::string isOldEmy = userTokenSecret.oldSecret.empty() ? "true" : "false";
    std::string isNewEmy = userTokenSecret.newSecret.empty() ? "true" : "false";
    return "oldSecret isEmpty = " + isOldEmy + ", newSecret isEmpty = " + isNewEmy;
}

std::string BuildTimeInfo(int64_t start, int64_t end)
{
    std::string duration = std::to_string(end - start);
    return " start: " + std::to_string(start) + " ,end: " + std::to_string(end) + " ,duration: " + duration;
}

int KeyManager::UpdateUserAuthByKeyType(unsigned int user, struct UserTokenSecret &userTokenSecret, KeyType keyType)
{
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::string secretInfo = BuildSecretStatus(userTokenSecret);
    std::string queryTime = BuildTimeInfo(getLockStatusTime_[LOCK_STATUS_START], getLockStatusTime_[LOCK_STATUS_END]);
    int64_t startTime = StorageService::StorageRadar::RecordCurrentTime();

    LOGW("[L3:KeyManager] UpdateUserAuthByKeyType:enter, user=%{public}d, token=%{public}d, oldSec=%{public}d,"
        "newSec=%{public}d, keyType: %{public}u", user,
        userTokenSecret.token.empty(), userTokenSecret.oldSecret.empty(), userTokenSecret.newSecret.empty(), keyType);
    if (keyType == EL5_KEY) {
        int ret = UpdateESecret(user, userTokenSecret);
        if (ret != 0) {
            LOGE("[L3:KeyManager] UpdateUserAuthByKeyType:user %{public}u UpdateESecret fail", user);
            queryTime += " UpdateUserAuth: " +
                BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
            StorageRadar::ReportUpdateUserAuth("UpdateESecret_ByKeyType", user, ret, "EL5", secretInfo + queryTime);
        }
        return ret;
    }
#ifdef USER_CRYPTO_MIGRATE_KEY
    int ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, keyType, true);
    if (ret != 0) {
        LOGE("user %{public}u UpdateCeEceSeceUserAuth el%{public}u fail", user, keyType);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_ByKeyType_Migrate",
            user, ret, "EL" + std::to_string(keyType), secretInfo + queryTime);
    }
#else
    int ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, keyType);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuthByKeyType:user %{public}u UpdateCeEceSeceUserAuth "
            "el%{public}u fail", user, keyType);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_ByKeyType",
            user, ret, "EL" + std::to_string(keyType), secretInfo + queryTime);
    }
#endif
    return ret;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
                               bool needGenerateShield)
#else
int KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret)
#endif
{
    std::string secretInfo = BuildSecretStatus(userTokenSecret);
    std::string queryTime = BuildTimeInfo(getLockStatusTime_[LOCK_STATUS_START], getLockStatusTime_[LOCK_STATUS_END]);
    int64_t startTime = StorageService::StorageRadar::RecordCurrentTime();

    LOGW("[L3:KeyManager] UpdateUserAuth: >>> ENTER <<< [user=%{public}u, token_len=%{public}zu,"
        "oldSec_len=%{public}zu, newSec_len=%{public}zu, token_empty=%{public}d, oldSec_empty=%{public}d,"
        "newSec_empty=%{public}d]",
        user, userTokenSecret.token.size(), userTokenSecret.oldSecret.size(),
        userTokenSecret.newSecret.size(), userTokenSecret.token.empty(),
        userTokenSecret.oldSecret.empty(), userTokenSecret.newSecret.empty());
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = UpdateESecret(user, userTokenSecret);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, UpdateESecret failed]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateESecret", user, ret, "EL5", secretInfo + queryTime);
        return ret;
    }
#ifdef USER_CRYPTO_MIGRATE_KEY
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL2_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, failed to update el2 key]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_Migrate", user, ret, "EL2", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL3_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, failed to update el3 key]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_Migrate", user, ret, "EL3", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL4_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, failed to update el4 key]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_Migrate", user, ret, "EL4", secretInfo + queryTime);
        return ret;
    }
#else
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL2_KEY);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, failed to update el2 key]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth", user, ret, "EL2", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL3_KEY);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, failed to update el3 key]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth", user, ret, "EL3", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL4_KEY);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateUserAuth: <<< EXIT FAILED <<< [userId=%{public}u, failed to update el4 key]", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth", user, ret, "EL4", secretInfo + queryTime);
        return ret;
    }
#endif
    LOGI("[L3:KeyManager] UpdateUserAuth: <<< EXIT SUCCESS <<< [retval=0]");
    return ret;
}

int32_t KeyManager::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
    const std::vector<uint8_t> &newSecret, uint64_t secureUid, uint32_t userId,
    const std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: >>> ENTER <<< [userId=%{public}d]", userId);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);
    std::vector<std::string> elKeyDirs = {el2Path, el3Path, el4Path};

    uint32_t i = 0;
    for (const auto &elxKeyDir : elKeyDirs) {
        if (!IsDir(elxKeyDir)) {
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< [key directory not found:"
                "%{public}s]", elxKeyDir.c_str());
            return E_KEY_TYPE_INVALID;
        }
        std::shared_ptr<BaseKey> elxKey = GetBaseKey(elxKeyDir);
        if (elxKey == nullptr) {
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< [failed to load elx key,"
                "path=%{public}s]", elxKeyDir.c_str());
            return E_PARAMS_NULLPTR_ERR;
        }

        if (plainText.size() < elKeyDirs.size()) {
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< [plain text size error]");
            return E_PARAMS_INVALID;
        }
        KeyBlob originKey(plainText[i]);
        elxKey->SetOriginKey(originKey);
        i++;
        auto ret = elxKey->StoreKey({authToken, newSecret, secureUid});
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< [failed to store key]");
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    if (IsUeceSupport()) {
        std::shared_ptr<BaseKey> el5Key = GetBaseKey(std::string(USER_EL5_DIR) + "/" + std::to_string(userId));
        if (!el5Key) {
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< [failed to get el5 key]");
            return E_PARAMS_NULLPTR_ERR;
        }
        bool tempUeceSupport = true;
        UserAuth userAuth = {.token = authToken, .secret = newSecret, .secureUid = secureUid};
        auto ret = el5Key->EncryptClassE(userAuth, tempUeceSupport, userId, USER_ADD_AUTH);
        if (ret != E_OK) {
            el5Key->ClearKey();
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< [failed to encrypt class E for"
                "user %{public}u]", userId);
            return E_EL5_ENCRYPT_CLASS_ERROR;
        }
        ret = el5Key->LockUece(tempUeceSupport);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: failed to lock key for user %{public}u", userId);
        }
    }
    LOGI("[L3:KeyManager] UpdateUseAuthWithRecoveryKey: <<< EXIT SUCCESS <<< [retval=0]");
    return E_OK;
}

int KeyManager::UpdateESecret(unsigned int user, struct UserTokenSecret &tokenSecret)
{
    LOGW("[L3:KeyManager] UpdateESecret: >>> ENTER <<< [user=%{public}u]", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] UpdateESecret: fscrypt syspara not found or encryption not enabled");
        return E_OK;
    }
    std::shared_ptr<BaseKey> el5Key = GetUserElKey(user, EL5_KEY);
    std::string el5Path = std::string(USER_EL5_DIR) + "/" + std::to_string(user);
    if (IsUeceSupport() && el5Key == nullptr) {
        if (!MkDirRecurse(el5Path, S_IRWXU)) {
            LOGE("[L3:KeyManager] UpdateESecret: <<< EXIT FAILED <<< [failed to create directory recursively for user"
                "%{public}u]", user);
            return E_CREATE_DIR_RECURSIVE_FAILED;
        }
        LOGI("[L3:KeyManager] UpdateESecret: directory created successfully for user %{public}u", user);
        el5Key = GetUserElKey(user, EL5_KEY);
    }
    if (el5Key == nullptr) {
        LOGE("[L3:KeyManager] UpdateESecret: <<< EXIT FAILED <<< [el5 key not found for user %{public}u]", user);
        return E_PARAMS_NULLPTR_ERR;
    }
    if (tokenSecret.newSecret.empty()) {
        return DoChangerPinCodeClassE(user, el5Key);
    }
    if (!tokenSecret.newSecret.empty() && !tokenSecret.oldSecret.empty()) {
        saveESecretStatus[user] = true;
        auto ret = el5Key->ChangePinCodeClassE(saveESecretStatus[user], user);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] UpdateESecret: <<< EXIT FAILED <<< [failed to change pin code class E for user"
                "%{public}u, error=%{public}d]", user, ret);
            return E_EL5_UPDATE_CLASS_ERROR;
        }
        LOGI("[L3:KeyManager] UpdateESecret: <<< EXIT SUCCESS <<< [retval=0]");
        return 0;
    }
    uint32_t status = tokenSecret.oldSecret.empty() ? USER_ADD_AUTH : USER_CHANGE_AUTH;
    LOGI("[L3:KeyManager] UpdateESecret: status=%{public}u", status);
    UserAuth auth = { .token = tokenSecret.token, .secret = tokenSecret.newSecret, .secureUid = tokenSecret.secureUid };
    saveESecretStatus[user] = true;
    auto ret = el5Key->EncryptClassE(auth, saveESecretStatus[user], user, status);
    if (static_cast<uint32_t>(ret) == FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG) {
        LOGE("[L3:KeyManager] UpdateESecret: user=%{public}d, error=FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG, no need"
            "to add again", user);
    } else if (ret != E_OK) {
        LOGE("[L3:KeyManager] UpdateESecret: <<< EXIT FAILED <<< [failed to encrypt class E for user %{public}u]",
            user);
        return E_EL5_ENCRYPT_CLASS_ERROR;
    }
    el5Key->ClearKeyInfo();
    LOGW("[L3:KeyManager] UpdateESecret: <<< EXIT SUCCESS <<< [retval=0], saveESecretStatus=%{public}u",
        saveESecretStatus[user]);
    return 0;
}

int KeyManager::DoChangerPinCodeClassE(unsigned int user, std::shared_ptr<BaseKey> &el5Key)
{
    auto ret = el5Key->DeleteClassEPinCode(user);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] DoChangerPinCodeClassE: failed to delete class E pin code for user %{public}u,"
            "error=%{public}d", user, ret);
        return E_EL5_DELETE_CLASS_ERROR;
    }
    saveESecretStatus[user] = false;
    return 0;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int KeyManager::UpdateCeEceSeceUserAuth(unsigned int user,
                                        struct UserTokenSecret &userTokenSecret,
                                        KeyType type, bool needGenerateShield)
#else
int KeyManager::UpdateCeEceSeceUserAuth(unsigned int user,
                                        struct UserTokenSecret &userTokenSecret,
                                        KeyType type)
#endif
{
    LOGW("[L3:KeyManager] UpdateCeEceSeceUserAuth: >>> ENTER <<< [user=%{public}u, type=%{public}u]", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] UpdateCeEceSeceUserAuth: fscrypt syspara not found or encryption not enabled");
        return E_OK;
    }
    std::shared_ptr<BaseKey> item = GetUserElKey(user, type);
    if (item == nullptr) {
        LOGE("[L3:KeyManager] UpdateCeEceSeceUserAuth: <<< EXIT FAILED <<< [el key not found for user %{public}u]",
            user);
        return E_PARAMS_NULLPTR_ERR;
    }

    UserAuth auth = { {}, userTokenSecret.oldSecret, userTokenSecret.secureUid };
    UserAuth auth_newSec = { userTokenSecret.token, userTokenSecret.newSecret, userTokenSecret.secureUid };
    LOGW("[L3:KeyManager] UpdateCeEceSeceUserAuth: param status token=%{public}d, oldSec=%{public}d,"
        "newSec=%{public}d", userTokenSecret.token.empty(),
        userTokenSecret.oldSecret.empty(), userTokenSecret.newSecret.empty());
    if (!userTokenSecret.oldSecret.empty()) {
        KeyBlob token(userTokenSecret.token);
        auth.token = std::move(token);
    }
    if ((item->RestoreKey(auth) != E_OK) && (item->RestoreKey(NULL_KEY_AUTH) != E_OK) &&
        (item->RestoreKey(auth_newSec) != E_OK)) {
        LOGE("[L3:KeyManager] UpdateCeEceSeceUserAuth: <<< EXIT FAILED <<< [failed to restore key]");
        return E_RESTORE_KEY_FAILED;
    }
    if (!userTokenSecret.newSecret.empty()) {
        KeyBlob token(userTokenSecret.token);
        KeyBlob newSecret(userTokenSecret.newSecret);
        auth.token = std::move(token);
        auth.secret = std::move(newSecret);
    } else {
        auth.token.Clear();
        auth.secret.Clear();
    }
#ifdef USER_CRYPTO_MIGRATE_KEY
    auto ret = item->StoreKey(auth, needGenerateShield);
#else
    auto ret = item->StoreKey(auth);
#endif
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] UpdateCeEceSeceUserAuth: <<< EXIT FAILED <<< [failed to store key]");
        std::string extraData = "user:" + std::to_string(user) + " type:" + std::to_string(type) +
        " ret:" + std::to_string(ret);
        StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", user, E_ELX_KEY_STORE_ERROR, "", extraData);
        return E_ELX_KEY_STORE_ERROR;
    }

    // Generate hashkey for encrypt public directory
    item->GenerateHashKey();
    item->ClearKeyInfo();
    userPinProtect[user] = !userTokenSecret.newSecret.empty();
    LOGI("[L3:KeyManager] UpdateCeEceSeceUserAuth: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::CheckNeedRestoreVersion(unsigned int user, KeyType type)
{
    std::error_code errCode;
    std::string restore_version;
    std::string need_restore_path = GetKeyDirByUserAndType(user, type) + RESTORE_DIR;
    (void)OHOS::LoadStringFromFile(need_restore_path, restore_version);
    if (std::filesystem::exists(need_restore_path, errCode) &&
        restore_version == DEFAULT_NEED_RESTORE_UPDATE_VERSION && !IsAppCloneUser(user)) {
        LOGI("[L3:KeyManager] NEED_RESTORE path exist: %{public}s, errcode: %{public}d",
            need_restore_path.c_str(), errCode.value());
        return type == EL5_KEY ? -ENONET : -EFAULT;
    }
    return E_OK;
}

std::string KeyManager::GetKeyDirByUserAndType(unsigned int user, KeyType type)
{
    std::string keyDir = "";
    switch (type) {
        case EL1_KEY:
            keyDir = std::string(USER_EL1_DIR) + "/" + std::to_string(user);
            break;
        case EL2_KEY:
            keyDir = std::string(USER_EL2_DIR) + "/" + std::to_string(user);
            break;
        case EL3_KEY:
            keyDir = std::string(USER_EL3_DIR) + "/" + std::to_string(user);
            break;
        case EL4_KEY:
            keyDir = std::string(USER_EL4_DIR) + "/" + std::to_string(user);
            break;
        case EL5_KEY:
            keyDir = std::string(USER_EL5_DIR) + "/" + std::to_string(user);
            break;
        default:
            LOGE("[L3:KeyManager] GetKeyDirByUserAndType: type %{public}u is invalid", type);
            break;
    }
    return keyDir;
}

std::string KeyManager::GetNatoNeedRestorePath(uint32_t userId, KeyType type)
{
    std::string keyDir = "";
    switch (type) {
        case EL2_KEY:
            keyDir = std::string(NATO_EL2_DIR) + "/" + std::to_string(userId);
            break;
        case EL3_KEY:
            keyDir = std::string(NATO_EL3_DIR) + "/" + std::to_string(userId);
            break;
        case EL4_KEY:
            keyDir = std::string(NATO_EL4_DIR) + "/" + std::to_string(userId);
            break;
        default:
            LOGE("[L3:KeyManager] GetNatoNeedRestorePath: type %{public}u is invalid", type);
            break;
    }
    return keyDir;
}

std::string KeyManager::GetKeyDirByType(KeyType type)
{
    std::string keyDir = "";
    switch (type) {
        case EL1_KEY:
            keyDir = USER_EL1_DIR;
            break;
        case EL2_KEY:
            keyDir = USER_EL2_DIR;
            break;
        case EL3_KEY:
            keyDir = USER_EL3_DIR;
            break;
        case EL4_KEY:
            keyDir = USER_EL4_DIR;
            break;
        case EL5_KEY:
            keyDir = USER_EL5_DIR;
            break;
        default:
            LOGE("[L3:KeyManager] GetKeyDirByType: type %{public}u is invalid", type);
            break;
    }
    return keyDir;
}

void KeyManager::SaveUserElKey(unsigned int user, KeyType type, std::shared_ptr<BaseKey> elKey)
{
    if (type >= EL1_KEY && type <= EL5_KEY) {
        userElKeys_[user][type] = elKey;
    } else {
        LOGE("[L3:KeyManager] SaveUserElKey: failed to save el key, type=%{public}d", type);
    }
}

std::shared_ptr<BaseKey> KeyManager::GetUserElKey(unsigned int user, KeyType type, bool isSave)
{
    bool isNeedGenerateBaseKey = false;
    std::shared_ptr<BaseKey> elKey = nullptr;
    if (!HasElkey(user, type)) {
        std::string keyDir = GetKeyDirByUserAndType(user, type);
        if (!IsDir(keyDir)) {
            LOGE("[L3:KeyManager] GetUserElKey: key directory not found for user %{public}u, type %{public}u",
                user, type);
            return nullptr;
        }
        elKey = GetBaseKey(keyDir);
        if (elKey == nullptr) {
            LOGE("[L3:KeyManager] GetUserElKey: base key memory allocation failed");
            return nullptr;
        }
        isNeedGenerateBaseKey = true;
        LOGI("[L3:KeyManager] GetUserElKey: generated new base key, type=%{public}u", type);
    } else {
        elKey = userElKeys_[user][type];
    }
    if (isSave && isNeedGenerateBaseKey) {
        SaveUserElKey(user, type, elKey);
    }
    return elKey;
}

int KeyManager::ActiveCeSceSeceUserKey(unsigned int user,
                                       KeyType type,
                                       const std::vector<uint8_t> &token,
                                       const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] ActiveCeSceSeceUserKey: >>> ENTER <<< [user=%{public}u, type=%{public}u]", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGE("[L3:KeyManager] ActiveCeSceSeceUserKey: fscrypt not enabled or encryption not enabled, skipping"
            "[user=%{public}u]", user);
        return E_OK;
    }
    int ret = CheckNeedRestoreVersion(user, type);
    if (ret == -EFAULT || ret == -ENOENT) {
        LOGE("[L3:KeyManager] ActiveCeSceSeceUserKey: <<< EXIT FAILED <<< [need restore version check failed]");
        return ret;
    }
    if (CheckUserPinProtect(user, token, secret) != E_OK) {
        LOGE("[L3:KeyManager] ActiveCeSceSeceUserKey: <<< EXIT FAILED <<< [IAM & Storage mismatch, waiting for user"
            "pin input]");
        return E_CHECK_USER_PIN_PROTECT_ERR;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (HasElkey(user, type) && HashElxActived(user, type)) {
        return E_ACTIVE_REPEATED;
    }
    std::shared_ptr<DelayHandler> userDelayHandler;
    if (GetUserDelayHandler(user, userDelayHandler)) {
        userDelayHandler->CancelDelayTask();
    }
    std::string keyDir = GetKeyDirByUserAndType(user, type);
    if (keyDir == "") {
        return E_KEY_TYPE_INVALID;
    }
    if (!CheckDir(type, keyDir, user)) {
        return -ENOENT;
    }
    std::shared_ptr<BaseKey> elKey = GetBaseKey(keyDir);
    if (elKey == nullptr) {
        LOGE("[L3:KeyManager] ActiveCeSceSeceUserKey: <<< EXIT FAILED <<< [failed to get base key]");
        return -EOPNOTSUPP;
    }
    if (type == EL5_KEY) {
        return ActiveUece(user, elKey, token, secret);
    }
    if (ActiveElXUserKey(user, token, type, secret, elKey) != 0) {
        LOGE("[L3:KeyManager] ActiveCeSceSeceUserKey: <<< EXIT FAILED <<< [failed to activate ELX user key]");
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    SaveUserElKey(user, type, elKey);
    userPinProtect[user] = !secret.empty();
    saveLockScreenStatus[user] = true;
    LOGI("[L3:KeyManager] ActiveCeSceSeceUserKey: <<< EXIT SUCCESS <<< [user=%{public}u,"
        "saveLockScreenStatus=%{public}d]", user, saveLockScreenStatus[user]);
    return E_OK;
}

int KeyManager::ActiveElxUserKey4Nato(unsigned int user, KeyType type, const KeyBlob &authToken)
{
    LOGW("[L3:KeyManager] ActiveElxUserKey4Nato: >>> ENTER <<< [userId=%{public}u, keyType=%{public}u]", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] ActiveElxUserKey4Nato: fscrypt syspara not found or encryption not enabled");
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::string keyDir = GetNatoNeedRestorePath(user, type);
    if (keyDir == "") {
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey4Nato", user, E_KEY_TYPE_INVALID, "",
            "type=" + std::to_string(type));
        LOGE("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT FAILED <<< [invalid key directory]");
        return E_KEY_TYPE_INVALID;
    }
    if (!CheckDir(type, keyDir, user)) {
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey4Nato", user, E_NATO_CHECK_KEY_DIR_ERROR, "",
            "CheckDir is false keyDir=" + keyDir);
        LOGE("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT FAILED <<< [key directory check failed]");
        return E_NATO_CHECK_KEY_DIR_ERROR;
    }
    std::shared_ptr<BaseKey> elKey = GetBaseKey(keyDir);
    if (elKey == nullptr) {
        LOGE("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT FAILED <<< [failed to get base key for"
            "userId=%{public}d el%{public}u]", user, type);
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey4Nato", user, E_PARAMS_NULLPTR_ERR, "",
            "elKey is nullptr keyDir=" + keyDir);
        return E_PARAMS_NULLPTR_ERR;
    }
    if (!elKey->InitKey(false)) {
        LOGE("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT FAILED <<< [failed to init key for userId=%{public}d"
            "el%{public}u]", user, type);
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey4Nato", user, E_NATO_INIT_USER_KEY_ERROR, "",
            "InitKey failed, keyDir=" + keyDir + ", user=" + std::to_string(user) + ", type=" + std::to_string(type));
        return E_NATO_INIT_USER_KEY_ERROR;
    }
    int32_t ret = elKey->RestoreKey4Nato(keyDir, type);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT FAILED <<< [failed to restore key for userId=%{public}d"
             "el%{public}u]", user, type);
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey4Nato", user, E_NATO_RESTORE_USER_KEY_ERROR, "",
            "RestoreKey failed, keyDir=" + keyDir + ", user=" + std::to_string(user) +
            ", type=" + std::to_string(type) + ", ret=" + std::to_string(ret));
        return E_NATO_RESTORE_USER_KEY_ERROR;
    }
    ret = elKey->ActiveKey(authToken, RETRIEVE_KEY);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT FAILED <<< [failed to activate key for"
             "userId=%{public}d el%{public}u]", user, type);
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey4Nato", user, E_NATO_ACTIVE_EL4_KEY_ERROR, "",
            "ActiveKey failed, keyDir=" + keyDir + ", user=" + std::to_string(user) +
            ", type=" + std::to_string(type) + ", ret=" + std::to_string(ret));
        return E_NATO_ACTIVE_EL4_KEY_ERROR;
    }
    LOGW("[L3:KeyManager] ActiveElxUserKey4Nato: <<< EXIT SUCCESS <<< [userId=%{public}u, keyType=%{public}u]",
        user, type);
    return E_OK;
}

int KeyManager::ActiveUece(unsigned int user,
                           std::shared_ptr<BaseKey> elKey,
                           const std::vector<uint8_t> &token,
                           const std::vector<uint8_t> &secret)
{
    if (ActiveUeceUserKey(user, token, secret, elKey) != 0) {
        LOGE("[L3:KeyManager] ActiveUece: failed to activate UECE user key for user %{public}u", user);
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    return 0;
}

bool KeyManager::CheckDir(KeyType type, std::string keyDir, unsigned int user)
{
    if ((type != EL5_KEY) && !IsDir(keyDir)) {
        LOGE("[L3:KeyManager] CheckDir: el directory not found for user %{public}u", user);
        return false;
    }
    if ((type == EL5_KEY) && CheckAndDeleteEmptyEl5Directory(keyDir, user) != 0) {
        return false;
    }
    return true;
}

bool KeyManager::HashElxActived(unsigned int user, KeyType type)
{
    if (HasElkey(user, type)) {
        auto elKey = userElKeys_[user][type];
        if (elKey == nullptr) {
            LOGI("[L3:KeyManager] HashElxActived: elKey is nullptr");
            return false;
        }

        if (!elKey->KeyDescIsEmpty()) {
            LOGI("[L3:KeyManager] HashElxActived: user el%{public}u key descriptor already exists", type);
            return true;
        }
    }
    return false;
}

bool KeyManager::IsAppCloneUser(unsigned int user)
{
    return user >= START_APP_CLONE_USER_ID && user <= MAX_APP_CLONE_USER_ID;
}

int KeyManager::CheckAndDeleteEmptyEl5Directory(std::string keyDir, unsigned int user)
{
    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(user);
    if (!IsDir(keyDir) || !IsDir(keyUeceDir)) {
        LOGE("[L3:KeyManager] CheckAndDeleteEmptyEl5Directory: el5 directory not found for user %{public}u", user);
        return -ENOENT;
    }

    if (IsDir(keyDir) && std::filesystem::is_empty(keyDir)) {
        OHOS::ForceRemoveDirectory(keyDir);
        LOGE("[L3:KeyManager] CheckAndDeleteEmptyEl5Directory: removed empty el5 key directory for user %{public}u",
            user);
        return -ENOENT;
    }
    return 0;
}

bool KeyManager::GetUserDelayHandler(uint32_t userId, std::shared_ptr<DelayHandler> &delayHandler)
{
    LOGI("[L3:KeyManager] GetUserDelayHandler: >>> ENTER <<< [userId=%{public}u]", userId);
    auto iterTask = userLockScreenTask_.find(userId);
    if (iterTask == userLockScreenTask_.end()) {
        userLockScreenTask_[userId] = std::make_shared<DelayHandler>(userId);
    }
    delayHandler = userLockScreenTask_[userId];
    if (delayHandler == nullptr) {
        LOGE("[L3:KeyManager] GetUserDelayHandler: <<< EXIT FAILED <<< [delayHandler is nullptr for user %{public}u]",
            userId);
        return false;
    }
    LOGI("[L3:KeyManager] GetUserDelayHandler: <<< EXIT SUCCESS <<<");
    return true;
}

int KeyManager::ActiveUeceUserKey(unsigned int user,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey)
{
    LOGI("[L3:KeyManager] ActiveUeceUserKey: >>> ENTER <<< [user=%{public}u]", user);
    saveESecretStatus[user] = !secret.empty();
    LOGW("[L3:KeyManager] ActiveUeceUserKey: userId=%{public}u, token_empty=%{public}d, secret_empty=%{public}d",
        user, token.empty(), secret.empty());
    SaveUserElKey(user, EL5_KEY, elKey);
    UserAuth auth = { .token = token, .secret = secret };
    bool eBufferStatue = false;
    auto ret = elKey->DecryptClassE(auth, saveESecretStatus[user], eBufferStatue, user, true);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] ActiveUeceUserKey: <<< EXIT FAILED <<< [failed to decrypt class E for user %{public}u]",
            user);
        return E_EL5_DELETE_CLASS_ERROR;
    }

    if (!token.empty() && !secret.empty() && eBufferStatue) {
        ret = TryToFixUeceKey(user, token, secret);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] ActiveUeceUserKey: <<< EXIT FAILED <<< [failed to fix UECE key for user %{public}u]",
                user);
            StorageRadar::ReportUserKeyResult("ActiveElxUserKey", user, E_TRY_TO_FIX_USER_KEY_ERR, "",
                "TryToFixUeceKey failed, user=" + std::to_string(user) + ", type=" + std::to_string(EL5_KEY));
            return E_TRY_TO_FIX_USER_KEY_ERR;
        }
    }
    LOGW("[L3:KeyManager] ActiveUeceUserKey: user=%{public}u, saveESecretStatus=%{public}d",
         user, saveESecretStatus[user]);
    LOGI("[L3:KeyManager] ActiveUeceUserKey: <<< EXIT SUCCESS <<<");
    return 0;
}

int KeyManager::ActiveElXUserKey(unsigned int user,
                                 const std::vector<uint8_t> &token, KeyType keyType,
                                 const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey)
{
    LOGI("[L3:KeyManager] ActiveElXUserKey: >>> ENTER <<< [user=%{public}u, keyType=%{public}u]", user, keyType);
    if (elKey->InitKey(false) == false) {
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey", user, E_ELX_KEY_INIT_ERROR, "",
            "Init failed, user=" + std::to_string(user) + ", type=" + std::to_string(keyType));
        LOGE("[L3:KeyManager] ActiveElXUserKey: <<< EXIT FAILED <<< [failed to initialize el key]");
        return E_ELX_KEY_INIT_ERROR;
    }
    UserAuth auth = { token, secret };
    auto keyResult = elKey->RestoreKey(auth);
    bool noKeyResult = (keyResult != E_OK) && (elKey->RestoreKey(NULL_KEY_AUTH) == E_OK);
    // key and no-key situation all failed, include upgrade situation, return err
    if (keyResult != E_OK && !noKeyResult) {
        std::string extraData = "keyResult: " + std::to_string(keyResult);
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey", user, E_RESTORE_KEY_FAILED, "", extraData);
        LOGE("[L3:KeyManager] ActiveElXUserKey: <<< EXIT FAILED <<< [failed to restore el key, type=%{public}u]",
            keyType);
        return E_RESTORE_KEY_FAILED;
    }
    // if device has pwd and decrypt success, continue.otherwise try no pwd and fix situation.
    if (keyResult != E_OK && noKeyResult) {
        int32_t ret = TryToFixUserCeEceSeceKey(user, keyType, token, secret);
        if (ret != E_OK) {
            std::string extraData = "keyResult: " + std::to_string(keyResult) + "fixResult: " + std::to_string(ret);
            StorageRadar::ReportUserKeyResult("ActiveElxUserKey", user, E_TRY_TO_FIX_USER_KEY_ERR, "", extraData);
            LOGE("[L3:KeyManager] ActiveElXUserKey: <<< EXIT FAILED <<< [failed to fix user CE/ECE/SECE key,"
                "type=%{public}u]", keyType);
            return E_TRY_TO_FIX_USER_KEY_ERR;
        }
    }
    std::string NEED_UPDATE_PATH = GetKeyDirByUserAndType(user, keyType) + PATH_LATEST + SUFFIX_NEED_UPDATE;
    std::string NEED_RESTORE_PATH = GetKeyDirByUserAndType(user, keyType) + PATH_LATEST + SUFFIX_NEED_RESTORE;
    if (!FileExists(NEED_RESTORE_PATH) && !FileExists(NEED_UPDATE_PATH)) {
        auto ret = elKey->StoreKey(auth);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] ActiveElXUserKey: <<< EXIT FAILED <<< [failed to store el key]");
            StorageRadar::ReportUserKeyResult("ActiveElxUserKey", user, E_ELX_KEY_STORE_ERROR, "",
                "Store failed, user=" + std::to_string(user) + ", ret=" + std::to_string(ret));
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    // Generate hashkey for encrypt public directory
    elKey->GenerateHashKey();
    int32_t ret = elKey->ActiveKey(auth.token, RETRIEVE_KEY);
    if (ret != E_OK) {
        StorageRadar::ReportUserKeyResult("ActiveElxUserKey", user, E_ELX_KEY_ACTIVE_ERROR, "",
            "Active failed, ret=" + std::to_string(ret));
        LOGE("[L3:KeyManager] ActiveElXUserKey: <<< EXIT FAILED <<< [failed to activate key for user %{public}u]",
            user);
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    LOGI("[L3:KeyManager] ActiveElXUserKey: <<< EXIT SUCCESS <<<");
    return 0;
}

int KeyManager::UnlockUserScreen(uint32_t user, const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] UnlockUserScreen: >>> ENTER <<< [user=%{public}u]", user);
    int64_t startTime = StorageService::StorageRadar::RecordCurrentTime();
    userPinProtect[user] = !secret.empty() || !token.empty();
    std::shared_ptr<DelayHandler> userDelayHandler;
    {
        std::lock_guard<std::mutex> lock(keyMutex_);
        if (GetUserDelayHandler(user, userDelayHandler)) {
            userDelayHandler->CancelDelayTask();
        }
    }
    auto iter = saveLockScreenStatus.find(user);
    if (iter == saveLockScreenStatus.end()) {
        saveLockScreenStatus.insert(std::make_pair(user, false));
    }
    if (!IsUserCeDecrypt(user)) {
        LOGE("[L3:KeyManager] UnlockUserScreen: <<< EXIT <<< [user %{public}u CE not decrypted, skipping]", user);
        return 0;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        saveLockScreenStatus[user] = true;
        LOGI("[L3:KeyManager] UnlockUserScreen: saveLockScreenStatus=%{public}d (fscrypt not enabled)",
            saveLockScreenStatus[user]);
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::string tokenEmy = token.empty() ? "true" : "false";
    std::string secretEmy = secret.empty() ? "true" : "false";
    std::string queryTime = BuildTimeInfo(getLockStatusTime_[LOCK_STATUS_START], getLockStatusTime_[LOCK_STATUS_END]);
    std::string extraData = "token isEmpty = " + tokenEmy + ", secret isEmpty = " + secretEmy + queryTime;
    int ret = 0;
    if ((ret = UnlockEceSece(user, token, secret)) != E_OK) {
        extraData += " UnlockScreen: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UnlockUserScreen::UnlockEceSece", user, ret, "EL4", extraData);
        return ret;
    }
    if ((ret = UnlockUece(user, token, secret)) != E_OK) {
        extraData += " UnlockScreen: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UnlockUserScreen::UnlockUece", user, ret, "EL5", extraData);
        return ret;
    }
    saveLockScreenStatus[user] = true;
    LOGW("[L3:KeyManager] UnlockUserScreen: <<< EXIT SUCCESS <<< [retval=0, saveLockScreenStatus=%{public}d]",
         saveLockScreenStatus[user]);
    return 0;
}

int32_t KeyManager::UnlockEceSece(uint32_t user,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] UnlockEceSece: >>> ENTER <<< [user=%{public}u]", user);
    auto el4Key = GetUserElKey(user, EL4_KEY);
    if (el4Key == nullptr) {
        saveLockScreenStatus[user] = true;
        LOGE("[L3:KeyManager] UnlockEceSece: <<< EXIT FAILED <<< [user %{public}u not activated,"
            "saveLockScreenStatus=%{public}d]", user,
             saveLockScreenStatus[user]);
        return E_NON_EXIST;
    }
    if (el4Key->RestoreKey({ token, secret }, false) != E_OK && el4Key->RestoreKey(NULL_KEY_AUTH, false) != E_OK) {
        LOGE("[L3:KeyManager] UnlockEceSece: <<< EXIT FAILED <<< [failed to restore el4 key for user %{public}u]",
            user);
        return E_RESTORE_KEY_FAILED;
    }
    int32_t ret = el4Key->UnlockUserScreen(token, user, FSCRYPT_SDP_ECE_CLASS);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] UnlockEceSece: <<< EXIT FAILED <<< [failed to unlock screen for user %{public}u]", user);
        StorageRadar::ReportUserKeyResult("UnlockEceSece", user, E_UNLOCK_SCREEN_FAILED, "",
            "Unlock failed, user=" + std::to_string(user) + ", ret=" + std::to_string(ret));
        return E_UNLOCK_SCREEN_FAILED;
    }
    LOGI("[L3:KeyManager] UnlockEceSece: <<< EXIT SUCCESS <<< [user=%{public}u, saveESecretStatus=%{public}d]",
        user, saveESecretStatus[user]);
    return E_OK;
}

int32_t KeyManager::UnlockUece(uint32_t user,
                               const std::vector<uint8_t> &token,
                               const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] UnlockUece: >>> ENTER <<< [user=%{public}u]", user);
    UserAuth auth = {.token = token, .secret = secret};
    saveESecretStatus[user] = !auth.token.IsEmpty();
    auto el5Key = GetUserElKey(user, EL5_KEY);
    bool eBufferStatue = false;
    if (el5Key != nullptr) {
        auto ret = el5Key->DecryptClassE(auth, saveESecretStatus[user], eBufferStatue, user, false);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] UnlockUece: <<< EXIT FAILED <<< [failed to unlock UECE for user %{public}u]", user);
            return ret;
        }
    }
    LOGI("[L3:KeyManager] UnlockUece: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int KeyManager::GetLockScreenStatus(uint32_t user, bool &lockScreenStatus)
{
    getLockStatusTime_[LOCK_STATUS_START] = StorageService::StorageRadar::RecordCurrentTime();
    LOGI("[L3:KeyManager] GetLockScreenStatus: >>> ENTER <<< [user=%{public}u]", user);
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto iter = saveLockScreenStatus.find(user);
    lockScreenStatus = (iter == saveLockScreenStatus.end()) ? false: iter->second;
    LOGW("[L3:KeyManager] GetLockScreenStatus: <<< EXIT SUCCESS <<< [lockScreenStatus=%{public}d]", lockScreenStatus);
    getLockStatusTime_[LOCK_STATUS_END] = StorageService::StorageRadar::RecordCurrentTime();
    return 0;
}

int KeyManager::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    if (!IsUeceSupport()) {
        LOGI("[L3:KeyManager] GenerateAppkey: UECE not supported or encryption not enabled");
        StorageRadar::ReportEl5KeyMgrResult("GenerateAppkey", userId, -ENOTSUP,
            "IsUeceSupport check failed");
        return -ENOTSUP;
    }
    LOGI("[L3:KeyManager] GenerateAppkey: >>> ENTER <<< [userId=%{public}u, hashId=%{public}u, needReSet=%{public}d]",
        userId, hashId, needReSet);
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (needReSet) {
        return GenerateAppkeyWithRecover(userId, hashId, keyId);
    }

    if (userId == KEY_RECOVERY_USER_ID) {
        LOGI("[L3:KeyManager] GenerateAppkey: generating app key in recovery mode");
        auto el5Key = GetBaseKey(GetKeyDirByUserAndType(userId, EL5_KEY));
        if (el5Key == nullptr) {
            LOGE("[L3:KeyManager] GenerateAppkey: <<< EXIT FAILED <<< [el5Key is null]");
            StorageRadar::ReportEl5KeyMgrResult("GenerateAppkey", userId, E_PARAMS_NULLPTR_ERR,
                "GetBaseKey failed, userId=KEY_RECOVERY_USER_ID");
            return E_PARAMS_NULLPTR_ERR;
        }
        auto ret = el5Key->GenerateAppkey(userId, hashId, keyId);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] GenerateAppkey: <<< EXIT FAILED <<< [failed to generate app key, error=%{public}d]",
                ret);
            StorageRadar::ReportEl5KeyMgrResult("GenerateAppkey", userId, E_EL5_GENERATE_APP_KEY_ERR,
                "GenerateAppkey failed, userId=KEY_RECOVERY_USER_ID, ret=" + std::to_string(ret));
            return E_EL5_GENERATE_APP_KEY_ERR;
        }
        LOGI("[L3:KeyManager] GenerateAppkey: <<< EXIT SUCCESS <<< [retval=0]");
        return 0;
    }
    auto el5Key = GetBaseKey(GetKeyDirByUserAndType(userId, EL5_KEY));
    if (el5Key == nullptr) {
        LOGE("[L3:KeyManager] GenerateAppkey: <<< EXIT FAILED <<< [el5Key is null]");
        StorageRadar::ReportEl5KeyMgrResult("GenerateAppkey", userId, E_PARAMS_NULLPTR_ERR,
            "GetBaseKey failed, userId=" + std::to_string(userId));
        return E_PARAMS_NULLPTR_ERR;
    }
    auto ret = el5Key->GenerateAppkey(userId, hashId, keyId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] GenerateAppkey: <<< EXIT FAILED <<< [failed to generate app key, error=%{public}d]", ret);
        StorageRadar::ReportEl5KeyMgrResult("GenerateAppKey", userId, E_EL5_GENERATE_APP_KEY_ERR,
            "GenerateAppkey failed, userId=" + std::to_string(userId) + ", ret=" + std::to_string(ret));
        return E_EL5_GENERATE_APP_KEY_ERR;
    }
    LOGI("[L3:KeyManager] GenerateAppkey: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::GenerateAppkeyWithRecover(uint32_t userId, uint32_t hashId, std::string &keyId)
{
    LOGI("[L3:KeyManager] GenerateAppkeyWithRecover: >>> ENTER <<< [userId=%{public}u, hashId=%{public}u]",
        userId, hashId);
    std::string el5Path = std::string(MAINTAIN_USER_EL5_DIR) + "/" + std::to_string(userId);
    auto el5Key = GetBaseKey(el5Path);
    if (el5Key == nullptr) {
        LOGE("[L3:KeyManager] GenerateAppkeyWithRecover: <<< EXIT FAILED <<< [el5Key is null]");
        return E_PARAMS_NULLPTR_ERR;
    }
    auto ret = el5Key->GenerateAppkey(userId, hashId, keyId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] GenerateAppkeyWithRecover: <<< EXIT FAILED <<< [failed to generate app key,"
            "error=%{public}d]", ret);
        return E_EL5_GENERATE_APP_KEY_WITH_RECOVERY_ERR;
    }
    ret = el5Key->DeleteClassEPinCode(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] GenerateAppkeyWithRecover: <<< EXIT FAILED <<< [failed to delete class E pin code]");
        return E_EL5_DELETE_CLASS_WITH_RECOVERY_ERR;
    }
    saveESecretStatus[userId] = false;
    LOGI("[L3:KeyManager] GenerateAppkeyWithRecover: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int32_t KeyManager::DeleteAppkey(uint32_t user, const std::string &keyId)
{
    LOGI("[L3:KeyManager] DeleteAppkey: >>> ENTER <<< [userId=%{public}u]", user);
    if (!IsUeceSupport()) {
        LOGI("[L3:KeyManager] DeleteAppkey: UECE not supported or encryption not enabled");
        StorageRadar::ReportEl5KeyMgrResult("DeleteAppkey", user, ENOTSUP,
            "IsUeceSupport check failed");
        return -ENOTSUP;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto el5Key = GetBaseKey(GetKeyDirByUserAndType(user, EL5_KEY));
    if (el5Key == nullptr) {
        LOGE("[L3:KeyManager] DeleteAppkey: <<< EXIT FAILED <<< [el5Key is null]");
        StorageRadar::ReportEl5KeyMgrResult("DeleteAppkey", user, E_PARAMS_NULLPTR_ERR,
            "GetBaseKey failed, userId=" + std::to_string(user));
        return E_PARAMS_NULLPTR_ERR;
    }
    if (el5Key->DeleteAppkey(keyId) != E_OK) {
        LOGE("[L3:KeyManager] DeleteAppkey: <<< EXIT FAILED <<< [failed to delete app key]");
        StorageRadar::ReportEl5KeyMgrResult("DeleteAppkey", user, E_EL5_DELETE_APP_KEY_ERR,
            "DeleteAppkey failed, userId=" + std::to_string(user) + ", keyId=" + keyId);
        return E_EL5_DELETE_APP_KEY_ERR;
    }
    LOGI("[L3:KeyManager] DeleteAppkey: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::CreateRecoverKey(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
                                 const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] CreateRecoverKey: >>> ENTER <<< [userId=%{public}u, userType=%{public}u,"
        "token_len=%{public}zu]", userId, userType, token.size());
    std::string globalUserEl1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
    std::string el1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);
    std::vector<std::string> elKeyDirs = { DEVICE_EL1_DIR, globalUserEl1Path, el1Path, el2Path, el3Path, el4Path };
    std::vector<KeyBlob> originKeys;
    for (const auto &elxKeyDir : elKeyDirs) {
        if (!IsDir(elxKeyDir)) {
            LOGE("[L3:KeyManager] CreateRecoverKey: <<< EXIT FAILED <<< [el directory not found: %{public}s]",
                elxKeyDir.c_str());
            return E_KEY_TYPE_INVALID;
        }
        std::shared_ptr<BaseKey> elxKey = GetBaseKey(elxKeyDir);
        if (elxKey == nullptr) {
            LOGE("[L3:KeyManager] CreateRecoverKey: <<< EXIT FAILED <<< [failed to load elx key, path=%{public}s]",
                elxKeyDir.c_str());
            return E_PARAMS_NULLPTR_ERR;
        }
        UserAuth auth = { token, secret };
        if (secret.empty() && token.size() == RECOVERY_TOKEN_CHALLENGE_LENG) {
            LOGW("[L3:KeyManager] CreateRecoverKey: secret is empty, using null token");
            auth = { {}, {}};
        }
        if ((elxKey->RestoreKey(auth, false) != E_OK) && (elxKey->RestoreKey(NULL_KEY_AUTH, false) != E_OK)) {
            LOGE("[L3:KeyManager] CreateRecoverKey: <<< EXIT FAILED <<< [failed to restore el key]");
            return E_RESTORE_KEY_FAILED;
        }
        KeyBlob originKey;
        if (!elxKey->GetOriginKey(originKey)) {
            LOGE("[L3:KeyManager] CreateRecoverKey: <<< EXIT FAILED <<< [failed to get origin key]");
            return -ENOENT;
        }
        originKeys.push_back(std::move(originKey));
    }
    int ret = RecoveryManager::GetInstance().CreateRecoverKey(userId, userType, token, secret, originKeys);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] CreateRecoverKey: <<< EXIT FAILED <<< [failed to create recovery key]");
        return ret;
    }
    originKeys.clear();
    LOGI("[L3:KeyManager] CreateRecoverKey: <<< EXIT SUCCESS <<< [retval=0]");
    return E_OK;
}

int KeyManager::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("[L3:KeyManager] SetRecoverKey: >>> ENTER <<< [key_len=%{public}zu]", key.size());
    std::vector<KeyBlob> originIvs;
    if (RecoveryManager::GetInstance().SetRecoverKey(key) != E_OK) {
        LOGE("[L3:KeyManager] SetRecoverKey: <<< EXIT FAILED <<< [failed to set recovery key]");
        return E_SET_RECOVERY_KEY_ERR;
    }
    LOGI("[L3:KeyManager] SetRecoverKey: <<< EXIT SUCCESS <<< [retval=0]");
    return E_OK;
}

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
int32_t KeyManager::FileBasedEncryptfsMount()
{
    LOGI("[L3:KeyManager] FileBasedEncryptfsMount: >>> ENTER <<<");
    std::string srcPath = FILE_BASED_ENCRYPT_SRC_PATH;
    std::string dstPath = FILE_BASED_ENCRYPT_DST_PATH;
    errno = 0;
    int32_t ret = UMount(dstPath);
    if (ret != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("[L3:KeyManager] FileBasedEncryptfsMount: <<< EXIT FAILED <<< [failed to unmount file based encrypt fs,"
            "err=%{public}d]", errno);
        std::string extraData = "srcPath=" + srcPath + ",dstPath=" + dstPath + ",kernelCode=" + std::to_string(errno);
        StorageRadar::ReportUserManager("FileBasedEncryptfsMount", DEFAULT_REPAIR_USERID, E_UMOUNT_FBE, extraData);
        return E_UMOUNT_FBE;
    }
    errno = 0;
    ret = Mount(srcPath, dstPath, nullptr, MS_BIND, nullptr);
    if (ret != E_OK && errno != EEXIST && errno != EBUSY) {
        LOGE("[L3:KeyManager] FileBasedEncryptfsMount: <<< EXIT FAILED <<< [failed to bind mount file based encrypt"
            "fs, err=%{public}d]", errno);
        std::string extraData = "srcPath=" + srcPath + ",dstPath=" + dstPath + ",kernelCode=" + std::to_string(errno);
        StorageRadar::ReportUserManager("FileBasedEncryptfsMount", DEFAULT_REPAIR_USERID, E_MOUNT_FBE, extraData);
        return E_MOUNT_FBE;
    }
    LOGI("[L3:KeyManager] FileBasedEncryptfsMount: <<< EXIT SUCCESS <<< [retval=0]");
    return E_OK;
}

int32_t KeyManager::InstallEmptyUserKeyForRecovery(uint32_t userId)
{
    LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: >>> ENTER <<< [userId=%{public}u]", userId);
    int32_t ret = E_OK;
    if (userId == KEY_RECOVERY_USER_ID) {
        ret = GenerateElxAndInstallUserKey(userId);
        LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: KEY_RECOVERY_USER ret=%{public}d", ret);
        ret = FileBasedEncryptfsMount();
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] InstallEmptyUserKeyForRecovery: <<< EXIT FAILED <<< [failed to mount file based"
                "encrypt fs, ret=%{public}d]", ret);
            return ret;
        }
        // Since the EL1_KEY corresponding to GLOBAL_USER_ID has already been cached during boot - pointing to the key
        // path under `/data` - attempting to regenerate the key to point to `/mnt/data_old` will be intercepted.
        // Therefore, the cache must be cleared first.
        // In recovery mode, the key pointing to `/data` is actually located in the ramdisk. This has no impact on
        // normal system operation after recovery.
        ret = GenerateAndInstallUserKey(userId, MAINTAIN_DEVICE_EL1_DIR, NULL_KEY_AUTH, EL0_KEY);
        LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: elxDir=%{public}s, ret=%{public}d",
            MAINTAIN_DEVICE_EL1_DIR, ret);
        DeleteElKey(GLOBAL_USER_ID, EL1_KEY);
        std::string globalUserEl1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: elxDir=%{public}s, ret=%{public}d",
            globalUserEl1Path.c_str(), ret);
        LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: <<< EXIT <<< [retval=%{public}d]", ret);
        return ret;
    }
    
    std::string el1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(userId);
    std::string el2Path = std::string(MAINTAIN_USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(MAINTAIN_USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(MAINTAIN_USER_EL4_DIR) + "/" + std::to_string(userId);
    std::string el5Path = std::string(MAINTAIN_USER_EL5_DIR) + "/" + std::to_string(userId);

    const std::vector<std::pair<std::string, KeyType>> keyDirType = {
        {el1Path, EL1_KEY}, {el2Path, EL2_KEY}, {el3Path, EL3_KEY}, {el4Path, EL4_KEY}, {el5Path, EL5_KEY}};
    for (const auto &[elxDir, elxType] : keyDirType) {
        ret = GenerateAndInstallUserKey(userId, elxDir, NULL_KEY_AUTH, elxType);
        LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: elxDir=%{public}s, ret=%{public}d", elxDir.c_str(), ret);
    }
    LOGI("[L3:KeyManager] InstallEmptyUserKeyForRecovery: <<< EXIT <<< [retval=%{public}d]", ret);
    return ret;
}
#endif

int32_t KeyManager::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
    LOGI("[L3:KeyManager] ResetSecretWithRecoveryKey: >>> ENTER <<< [userId=%{public}u, rkType=%{public}u,"
        "key_len=%{public}zu]", userId, rkType, key.size());
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    if (!IsEncryption()) {
        return InstallEmptyUserKeyForRecovery(userId);
    }
    std::vector<KeyBlob> originIvs;
    auto ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key, originIvs);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< [failed to reset secret with recovery"
            "key]");
        return E_RESET_SECRET_WITH_RECOVERY_KEY_ERR;
    }

    std::string globalUserEl1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
    std::string el1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(userId);
    std::string el2Path = std::string(MAINTAIN_USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(MAINTAIN_USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(MAINTAIN_USER_EL4_DIR) + "/" + std::to_string(userId);
    std::vector<std::string> elKeyDirs = {MAINTAIN_DEVICE_EL1_DIR, globalUserEl1Path,
                                          el1Path, el2Path, el3Path, el4Path};

    if (originIvs.size() < elKeyDirs.size()) {
        LOGE("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< [plain text size error]");
        return E_PARAMS_INVALID;
    }

    uint32_t i = 0;
    for (const auto &elxKeyDir : elKeyDirs) {
        if (!IsDir(elxKeyDir)) {
            LOGE("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< [directory not found: %{public}s]",
                elxKeyDir.c_str());
            return E_KEY_TYPE_INVALID;
        }
        std::shared_ptr<BaseKey> elxKey = GetBaseKey(elxKeyDir);
        if (elxKey == nullptr) {
            LOGE("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< [failed to load elx key,"
                "path=%{public}s]", elxKeyDir.c_str());
            return E_PARAMS_NULLPTR_ERR;
        }

        elxKey->SetOriginKey(originIvs[i]);
        i++;
        auto ret = elxKey->StoreKey(NULL_KEY_AUTH);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< [failed to store key]");
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    ret = FileBasedEncryptfsMount();
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< [failed to mount file based encrypt fs]");
        return ret;
    }
#endif
    LOGI("[L3:KeyManager] ResetSecretWithRecoveryKey: <<< EXIT SUCCESS <<< [retval=0]");
    return E_OK;
}

int KeyManager::InActiveUserKey(unsigned int user)
{
    LOGI("[L3:KeyManager] InActiveUserKey: >>> ENTER <<< [user=%{public}u]", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] InActiveUserKey: fscrypt syspara not found or encryption not enabled");
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    KeyType types[] = { EL2_KEY, EL3_KEY, EL4_KEY, EL5_KEY };
    int ret = 0;
    for (KeyType type : types) {
        ret = InactiveUserElKey(user, type);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] InActiveUserKey: <<< EXIT FAILED <<< [failed to inactive el%{public}d key]", type);
            StorageRadar::ReportUserKeyResult("InactiveUserElKey", user, ret, "EL" + std::to_string(type), "");
            return ret;
        }
    }
    auto userTask = userLockScreenTask_.find(user);
    if (userTask != userLockScreenTask_.end()) {
        userLockScreenTask_.erase(userTask);
        LOGI("[L3:KeyManager] InActiveUserKey: inactivated user %{public}u, erased user task", user);
    }
    LOGI("[L3:KeyManager] InActiveUserKey: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

int KeyManager::InactiveUserElKey(unsigned int user, KeyType type)
{
    std::shared_ptr<BaseKey> elKey;
    if (!HasElkey(user, type)) {
        LOGE("[L3:KeyManager] InactiveUserElKey: key not found for user %{public}u type %{public}u", user, type);
        std::string keyDir = GetKeyDirByUserAndType(user, type);
        if (type != EL5_KEY && !IsDir(keyDir)) {
            LOGE("[L3:KeyManager] InactiveUserElKey: directory not found for user %{public}u type %{public}u",
                user, type);
            return E_PARAMS_INVALID;
        }
        elKey = GetBaseKey(keyDir);
    } else {
        elKey = userElKeys_[user][type];
    }
    if (elKey == nullptr) {
        LOGE("[L3:KeyManager] InactiveUserElKey: base key memory allocation failed");
        return E_PARAMS_INVALID;
    }
    int32_t ret = elKey->InactiveKey(USER_LOGOUT);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] InactiveUserElKey: failed to clear key for user %{public}u", user);
        StorageRadar::ReportUserKeyResult("InactiveUserElKey", user, ret, "EL" +
            std::to_string(type), "InactiveKey failed");
        return E_ELX_KEY_INACTIVE_ERROR;
    }
    LOGI("[L3:KeyManager] InactiveUserElKey: removing elx descriptor");
    auto elx = elKey->GetKeyDir();
    if (!elx.empty() && elx != "el1") {
        std::string descElx = std::string(FSCRYPT_EL_DIR) + "/" + elx + "/" + std::to_string(user) + DESC_DIR;
        (void)remove(descElx.c_str());
        LOGI("[L3:KeyManager] InactiveUserElKey: descriptor removed successfully");
    }
    DeleteElKey(user, type);
    LOGI("[L3:KeyManager] InactiveUserElKey: user %{public}u elX inactivated successfully", user);
    return 0;
}

int KeyManager::LockUserScreen(uint32_t user)
{
    LOGI("[L3:KeyManager] LockUserScreen: >>> ENTER <<< [user=%{public}u]", user);
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::error_code errCode;
    if (!IsUserCeDecrypt(user) || std::filesystem::exists(GetNatoNeedRestorePath(user, EL4_KEY), errCode)) {
        LOGE("[L3:KeyManager] LockUserScreen: user %{public}u CE not decrypted, skipping", user);
        return 0;
    }

    auto iter = userPinProtect.find(user);
    if (iter == userPinProtect.end() || iter->second == false) {
        if (!IamClient::GetInstance().HasPinProtect(user)) {
            LOGI("[L3:KeyManager] LockUserScreen: no pin protect, saveLockScreenStatus=%{public}d",
                saveLockScreenStatus[user]);
            return 0;
        }
        userPinProtect.erase(user);
        userPinProtect.insert(std::make_pair(user, true));
        LOGI("[L3:KeyManager] LockUserScreen: user=%{public}u locked screen, saveLockScreenStatus=%{public}d",
            user, saveLockScreenStatus[user]);
    }
    iter = saveLockScreenStatus.find(user);
    if (iter == saveLockScreenStatus.end()) {
        saveLockScreenStatus.insert(std::make_pair(user, false));
        LOGI("[L3:KeyManager] LockUserScreen: user=%{public}u inserted lock screen status,"
            "saveLockScreenStatus=%{public}d", user, saveLockScreenStatus[user]);
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        saveLockScreenStatus[user] = false;
        LOGI("[L3:KeyManager] LockUserScreen: KeyCtrlHasFscryptSyspara is false, saveLockScreenStatus=%{public}d",
            saveLockScreenStatus[user]);
        return 0;
    }
    auto el5Key = GetUserElKey(user, EL5_KEY, false);
    saveESecretStatus[user] = true;
    if (el5Key != nullptr && el5Key->LockUece(saveESecretStatus[user]) != E_OK) {
        LOGE("[L3:KeyManager] LockUserScreen: failed to lock el5 key for user %{public}u", user);
    }
    auto el4Key = GetUserElKey(user, EL4_KEY, false);
    if (el4Key == nullptr) {
        LOGE("[L3:KeyManager] LockUserScreen: el4 key not found for user %{public}u", user);
        StorageRadar::ReportUpdateUserAuth("LockUserScreen::GetUserElKey", user, E_NON_EXIST, "EL4", "not found key");
        return E_NON_EXIST;
    }
    std::shared_ptr<DelayHandler> userDelayHandler;
    if (GetUserDelayHandler(user, userDelayHandler)) {
        userDelayHandler->StartDelayTask(el4Key);
    }

    saveLockScreenStatus[user] = false;
    LOGI("[L3:KeyManager] LockUserScreen: <<< EXIT SUCCESS <<< [retval=0, saveLockScreenStatus=%{public}d]",
        saveLockScreenStatus[user]);
    return 0;
}

int KeyManager::SetDirectoryElPolicy(unsigned int user, KeyType type, const std::vector<FileList> &vec)
{
    LOGI("[L3:KeyManager] SetDirectoryElPolicy: >>> ENTER <<< [user=%{public}u, type=%{public}u, count=%{public}zu]",
        user, type, vec.size());
    if (!KeyCtrlHasFscryptSyspara() || !IsEncryption()) {
        LOGW("[L3:KeyManager] SetDirectoryElPolicy: fscrypt syspara not found or encryption not enabled");
        return 0;
    }
    std::string keyPath;
    std::string eceSeceKeyPath;
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (type == EL1_KEY) {
        int ret = getElxKeyPath(user, EL1_KEY, keyPath);
        if (ret != E_OK) {
            return ret;
        }
    } else if (type == EL2_KEY || type == EL3_KEY || type == EL4_KEY || type == EL5_KEY) {
        int ret = getElxKeyPath(user, EL2_KEY, keyPath);
        if (ret != E_OK) {
            return ret;
        }
    } else {
        LOGE("[L3:KeyManager] SetDirectoryElPolicy: el flags not specified, no need to crypt");
        return 0;
    }
    int32_t getElxKeyPathRet = getElxKeyPath(user, type, eceSeceKeyPath);
    if (getElxKeyPathRet != 0) {
        LOGE("[L3:KeyManager] SetDirectoryElPolicy: failed to get ece/sece key path");
        StorageRadar::ReportUpdateUserAuth("SetDirectoryElPolicy::getElxKeyPath", user, -ENOENT, "EL" +
            std::to_string(type), "getElxKeyPath ret =" + std::to_string(getElxKeyPathRet));
        return -ENOENT;
    }
    for (auto item : vec) {
        int ret = LoadAndSetPolicy(keyPath.c_str(), item.path.c_str());
        if (ret != 0) {
            LOGE("[L3:KeyManager] SetDirectoryElPolicy: failed to set directory el policy, ret=%{public}d,"
                "path=%{public}s", ret, item.path.c_str());
            return E_LOAD_AND_SET_POLICY_ERR;
        }
    }
    if (type == EL3_KEY || type == EL4_KEY) {
        for (auto item : vec) {
            int32_t ret = LoadAndSetEceAndSecePolicy(eceSeceKeyPath.c_str(), item.path.c_str(), static_cast<int>(type));
            if (ret != 0) {
                LOGE("[L3:KeyManager] SetDirectoryElPolicy: failed to set directory ece/sece policy");
                StorageRadar::ReportUpdateUserAuth("SetDirectoryElPolicy::LoadAndSetEceAndSecePolicy", user,
                    E_LOAD_AND_SET_ECE_POLICY_ERR, "EL" + std::to_string(type), "LoadAndSetEceAndSecePolicy ret ="
                    + std::to_string(ret) + " path:" + item.path +  "eceSeceKeyPath:"+ eceSeceKeyPath);
                return E_LOAD_AND_SET_ECE_POLICY_ERR;
            }
        }
    }
    LOGW("[L3:KeyManager] SetDirectoryElPolicy: <<< EXIT SUCCESS <<< [user %{public}u el policy set successfully]",
        user);
    return 0;
}

int32_t KeyManager::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath,
    StorageService::EncryptionLevel level)
{
    LOGI("[L3:KeyManager] SetDirEncryptionPolicy: >>> ENTER <<< [userId=%{public}u, level=%{public}u]", userId, level);
    if (!KeyCtrlHasFscryptSyspara() || !IsEncryption()) {
        LOGW("[L3:KeyManager] SetDirEncryptionPolicy: fscrypt syspara not found or encryption not enabled");
        return E_NOT_SUPPORT;
    }

    bool isCeEncrypt = false;
    auto ret = GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("[L3:KeyManager] SetDirEncryptionPolicy: <<< EXIT FAILED <<< [user el2 not decrypted, userId=%{public}u]",
            userId);
        return E_KEY_NOT_ACTIVED;
    }

    std::string keyPath;
    ret = ((level == EL1_SYS_KEY) || (level == EL1_USER_KEY)) ? getElxKeyPath(userId, EL1_KEY, keyPath)
        : getElxKeyPath(userId, EL2_KEY, keyPath);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] SetDirEncryptionPolicy: <<< EXIT FAILED <<< [failed to get key path, userId=%{public}u,"
            "level=%{public}u]", userId, level);
        return ret;
    }

    ret = LoadAndSetPolicy(keyPath.c_str(), dirPath.c_str());
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] SetDirEncryptionPolicy: <<< EXIT FAILED <<< [failed to set directory encryption policy,"
            "userId=%{public}u, level=%{public}u]", userId, level);
        return ret;
    }

    if (level == EL3_USER_KEY || level == EL4_USER_KEY) {
        std::string eceSeceKeyPath;
        KeyType tempType = level == EL3_USER_KEY ? EL3_KEY : EL4_KEY;
        ret = getElxKeyPath(userId, tempType, eceSeceKeyPath);
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] SetDirEncryptionPolicy: <<< EXIT FAILED <<< [failed to get key path,"
                "userId=%{public}u, level=%{public}u]", userId, level);
            return ret;
        }
        ret = LoadAndSetEceAndSecePolicy(eceSeceKeyPath.c_str(), dirPath.c_str(), static_cast<int>(level));
        if (ret != E_OK) {
            LOGE("[L3:KeyManager] SetDirEncryptionPolicy: <<< EXIT FAILED <<< [failed to set ece/sece policy,"
                "userId=%{public}u, level=%{public}u]", userId, level);
            return ret;
        }
    }
    LOGI("[L3:KeyManager] SetDirEncryptionPolicy: <<< EXIT SUCCESS <<< [retval=0]");
    return E_OK;
}

int KeyManager::getElxKeyPath(unsigned int user, KeyType type, std::string &elxKeyPath)
{
    std::string natoPath = GetNatoNeedRestorePath(user, type);
    std::error_code errCode;
    if (std::filesystem::exists(natoPath, errCode)) {
        LOGW("[L3:KeyManager] type=%{public}d NATO path exists.", type);
        elxKeyPath = natoPath;
        return E_OK;
    }
    if (!HasElkey(user, type) && type != EL5_KEY) {
        return -ENOENT;
    }
    if (type >= EL1_KEY && type <= EL4_KEY) {
        elxKeyPath = userElKeys_[user][type]->GetDir();
    }
    return E_OK;
}

int KeyManager::UpdateCeEceSeceKeyContext(uint32_t userId, KeyType type)
{
    LOGI("[L3:KeyManager] UpdateCeEceSeceKeyContext: >>> ENTER <<< [userId=%{public}u, type=%{public}u]", userId, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("[L3:KeyManager] UpdateCeEceSeceKeyContext: fscrypt syspara not found or encryption not enabled");
        return 0;
    }
    if (HasElkey(userId, type) == false) {
        LOGE("[L3:KeyManager] UpdateCeEceSeceKeyContext: <<< EXIT FAILED <<< [key not found for user %{public}u,"
            "type=%{public}u]", userId, type);
        return E_PARAMS_INVALID;
    }
    std::shared_ptr<BaseKey> elKey = GetUserElKey(userId, type);
    if (elKey == nullptr) {
        LOGE("[L3:KeyManager] UpdateCeEceSeceKeyContext: <<< EXIT FAILED <<< [el%{public}u key not found for user"
            "%{public}u]", type, userId);
        return -ENOENT;
    }
    auto ret = elKey->UpdateKey();
    if (ret != E_OK) {
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId,
            ret, "EL" + std::to_string(type), "Basekey update newest context failed");
        LOGE("[L3:KeyManager] UpdateCeEceSeceKeyContext: <<< EXIT FAILED <<< [failed to update base key context]");
        return E_ELX_KEY_UPDATE_ERROR;
    }
    return 0;
}

int KeyManager::UpdateKeyContextByKeyType(uint32_t userId, KeyType keyType)
{
    LOGI("[L3:KeyManager] UpdateKeyContextByKeyType: UpdateKeyContextByKeyType enter,"
        " userId: %{public}u, keyType: %{public}u", userId, keyType);
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::string strKeyType = "EL" + std::to_string(keyType);
    int ret = E_OK;
    if (keyType != EL5_KEY) {
        ret = UpdateCeEceSeceKeyContext(userId, keyType);
        if (ret != 0) {
            LOGE("[L3:KeyManager] UpdateKeyContextByKeyType:Basekey update"
                "EL%{public}u newest context failed", keyType);
            StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext",
                userId, ret, strKeyType, "");
        }
        return ret;
    }
    if (IsUeceSupport()) {
        ret = UpdateClassEBackUpFix(userId);
        if (ret != 0) {
            LOGE("Inform FBE do update class E backup failed, ret=%{public}d", ret);
            StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateClassEBackUpFix", userId,
                ret, strKeyType, "Inform FBE do update class E backup failed");

            return ret;
        }
        if (saveESecretStatus[userId]) {
            ret = UpdateCeEceSeceKeyContext(userId, keyType);
        }
    }
    if (ret != 0 && ((userId < START_APP_CLONE_USER_ID || userId > MAX_APP_CLONE_USER_ID))) {
        LOGE("Basekey update EL5 newest context failed");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, strKeyType, "");
        return ret;
    }
    LOGI("Basekey update key context success");
    return 0;
}

int KeyManager::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("[L3:KeyManager] UpdateKeyContext: >>> ENTER <<< [userId=%{public}u, needRemoveTmpKey=%{public}d]",
        userId, needRemoveTmpKey);
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = UpdateCeEceSeceKeyContext(userId, EL2_KEY);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateKeyContext: failed to update EL2 key context");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL2", "");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL3_KEY);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateKeyContext: failed to update EL3 key context");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL3", "");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL4_KEY);
    if (ret != 0) {
        LOGE("[L3:KeyManager] UpdateKeyContext: failed to update EL4 key context");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL4", "");
        return ret;
    }
    if (IsUeceSupport()) {
        ret = UpdateClassEBackUpFix(userId);
        if (ret != 0) {
            LOGE("[L3:KeyManager] UpdateKeyContext: failed to inform FBE to update class E backup, ret=%{public}d",
                ret);
            StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateClassEBackUpFix", userId,
                ret, "EL5", "Inform FBE do update class E backup failed");
            return ret;
        }
        if (saveESecretStatus[userId]) {
            ret = UpdateCeEceSeceKeyContext(userId, EL5_KEY);
        }
    }
    if (ret != 0 && ((userId < START_APP_CLONE_USER_ID || userId > MAX_APP_CLONE_USER_ID))) {
        LOGE("[L3:KeyManager] UpdateKeyContext: failed to update EL5 key context");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL5", "");
        return ret;
    }
    LOGI("[L3:KeyManager] UpdateKeyContext: <<< EXIT SUCCESS <<< [retval=0]");
    return 0;
}

bool KeyManager::IsUeceSupport()
{
    int fd = open(UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L3:KeyManager] IsUeceSupport: UECE not supported");
        }
        LOGE("[L3:KeyManager] IsUeceSupport: failed to open UECE, errno=%{public}d", errno);
        return false;
    }
    close(fd);
    LOGI("[L3:KeyManager] IsUeceSupport: <<< EXIT SUCCESS <<< [uece supported=true]");
    return true;
}

int KeyManager::UpdateClassEBackUpFix(uint32_t userId)
{
    LOGI("[L3:KeyManager] UpdateClassEBackUp: >>> ENTER <<< [userId=%{public}u]", userId);
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    auto el5Key = GetUserElKey(userId, EL5_KEY);
    if (el5Key == nullptr) {
        LOGE("[L3:KeyManager] UpdateClassEBackUp: <<< EXIT FAILED <<< [el5 key not found for user %{public}u]", userId);
        return E_NON_EXIST;
    }
    auto ret = el5Key->UpdateClassEBackUp(userId);
    auto delay = StorageService::StorageRadar::ReportDuration("FBE:UpdateClassEBackUp",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userId);
    LOGI("[L3:KeyManager] UpdateClassEBackUp: <<< EXIT <<< [user=%{public}u, delay=%{public}s]", userId, delay.c_str());
    return ret;
}

int KeyManager::UpdateClassEBackUp(uint32_t userId)
{
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto el5Key = GetUserElKey(userId, EL5_KEY);
    if (el5Key == nullptr) {
        LOGE("Have not found user %{public}u el5 Key", userId);
        return E_NON_EXIST;
    }
    auto ret = el5Key->UpdateClassEBackUp(userId);
    auto delay = StorageService::StorageRadar::ReportDuration("FBE:UpdateClassEBackUp",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userId);
    LOGI("SD_DURATION: FBEX: UPDATE CLASS E BACKUP: user=%{public}u, delay=%{public}s", userId, delay.c_str());
    return ret;
}

int KeyManager::IsUeceSupportWithErrno()
{
    int fd = open(UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L3:KeyManager] IsUeceSupportWithErrno: UECE not supported");
            return ENOENT;
        }
        LOGE("[L3:KeyManager] IsUeceSupportWithErrno: failed to open UECE, errno=%{public}d", errno);
        return errno;
    }
    close(fd);
    LOGI("[L3:KeyManager] IsUeceSupportWithErrno: <<< EXIT SUCCESS <<< [retval=0, uece supported=true]");
    return E_OK;
}

int KeyManager::UpgradeKeys(const std::vector<FileList> &dirInfo)
{
    LOGI("[L3:KeyManager] UpgradeKeys: >>> ENTER <<< [count=%{public}zu]", dirInfo.size());
    for (const auto &it : dirInfo) {
        std::shared_ptr<BaseKey> elKey = GetBaseKey(it.path);
        if (elKey == nullptr) {
            LOGE("[L3:KeyManager] UpgradeKeys: failed to allocate base key memory");
            continue;
        }
        elKey->UpgradeKeys();
    }
    LOGI("[L3:KeyManager] UpgradeKeys: <<< EXIT SUCCESS <<<");
    return 0;
}

int KeyManager::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGI("[L3:KeyManager] GetFileEncryptStatus: >>> ENTER <<< [userId=%{public}u, needCheckDirMount=%{public}d]",
         userId, needCheckDirMount);
    isEncrypted = true;
    const char rootPath[] = "/data/app/el2/";
    const char basePath[] = "/base";
    size_t allPathSize = strlen(rootPath) + strlen(basePath) + 1 + USER_ID_SIZE_VALUE;
    char *path = reinterpret_cast<char *>(malloc(sizeof(char) * (allPathSize)));
    if (path == nullptr) {
        LOGE("[L3:KeyManager] GetFileEncryptStatus: <<< EXIT FAILED <<< [failed to allocate path memory]");
        return E_MEMORY_OPERATION_ERR;
    }
    int len = sprintf_s(path, allPathSize, "%s%u%s", rootPath, userId, basePath);
    if (len <= 0 || (size_t)len >= allPathSize) {
        free(path);
        LOGE("[L3:KeyManager] GetFileEncryptStatus: <<< EXIT FAILED <<< [failed to get base path]");
        return E_PARAMS_INVALID;
    }
    if (access(path, F_OK) != 0) {
        free(path);
        LOGE("[L3:KeyManager] GetFileEncryptStatus: cannot access el2 dir, user %{public}u el2 is encrypted", userId);
        return E_OK;
    }
    std::string el2Path(path);
    if (!SaveStringToFile(el2Path + EL2_ENCRYPT_TMP_FILE, " ")) {
        free(path);
        LOGE("[L3:KeyManager] GetFileEncryptStatus: cannot save el2 file, user %{public}u el2 is encrypted", userId);
        return E_OK;
    }
    free(path);
    int ret = remove((el2Path + EL2_ENCRYPT_TMP_FILE).c_str());
    LOGE("[L3:KeyManager] GetFileEncryptStatus: remove ret=%{public}d", ret);
    if (needCheckDirMount && !MountManager::GetInstance().CheckMountFileByUser(userId)) {
        LOGI("[L3:KeyManager] GetFileEncryptStatus: virtual directory does not exist");
        return E_OK;
    }
    isEncrypted = false;
    LOGI("[L3:KeyManager] GetFileEncryptStatus: <<< EXIT SUCCESS <<< [isEncrypted=false]");
    return E_OK;
}

bool KeyManager::IsUserCeDecrypt(uint32_t userId)
{
    bool isCeEncrypt = false;
    int ret = GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("[L3:KeyManager] IsUserCeDecrypt: user %{public}d el2 not decrypted", userId);
        return false;
    }
    LOGI("[L3:KeyManager] IsUserCeDecrypt: user %{public}d el2 decrypted", userId);
    return true;
}

int KeyManager::CheckUserPinProtect(unsigned int userId,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] CheckUserPinProtect: >>> ENTER <<< [userId=%{public}u, token_len=%{public}zu,"
        "secret_len=%{public}zu]", userId, token.size(), secret.size());
    std::error_code errCode;
    std::string restorePath = std::string(USER_EL2_DIR) + "/" + std::to_string(userId) + RESTORE_DIR;
    if (!std::filesystem::exists(restorePath, errCode)) {
        LOGI("[L3:KeyManager] CheckUserPinProtect: <<< EXIT SUCCESS <<< [no need to check pin code]");
        return E_OK;
    }
    // judge if device has PIN protect
    if ((token.empty() && secret.empty()) && IamClient::GetInstance().HasPinProtect(userId)) {
        LOGE("[L3:KeyManager] CheckUserPinProtect: <<< EXIT FAILED <<< [user %{public}d has pin code protect]", userId);
        return E_ERR;
    }
    LOGI("[L3:KeyManager] CheckUserPinProtect: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int KeyManager::TryToFixUserCeEceSeceKey(unsigned int userId,
                                         KeyType keyType,
                                         const std::vector<uint8_t> &token,
                                         const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] TryToFixUserCeEceSeceKey: >>> ENTER <<< [userId=%{public}u, keyType=%{public}u]",
        userId, keyType);
    if (!IamClient::GetInstance().HasPinProtect(userId)) {
        LOGE("[L3:KeyManager] TryToFixUserCeEceSeceKey: user %{public}d has no pin code protect", userId);
        return E_OK;
    }

    uint64_t secureUid = { 0 };
    if (!secret.empty() && !token.empty()) {
        IamClient::GetInstance().GetSecureUid(userId, secureUid);
        LOGE("[L3:KeyManager] TryToFixUserCeEceSeceKey: pin code exists, secure uid obtained");
    }
    UserAuth auth = { .token = token, .secret = secret, .secureUid = secureUid };
    UserTokenSecret userTokenSecret = { .token = token, .oldSecret = {}, .newSecret = secret, .secureUid = secureUid };

    int ret = E_OK;
#ifdef USER_CRYPTO_MIGRATE_KEY
    ret = UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType, false);
#else
    ret = UpdateCeEceSeceUserAuth(userId, userTokenSecret, keyType);
#endif
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] TryToFixUserCeEceSeceKey: <<< EXIT FAILED <<< [failed to fix elx key]");
        StorageRadar::ReportUpdateUserAuth("TryToFixUserCeEceSeceKey::UpdateCeEceSeceUserAuth",
            userId, ret, std::to_string(keyType), "try to fix elx key failed");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, keyType);
    if (ret != E_OK) {
        LOGE("[L3:KeyManager] TryToFixUserCeEceSeceKey: <<< EXIT FAILED <<< [failed to fix elx key context]");
        StorageRadar::ReportUpdateUserAuth("TryToFixUserCeEceSeceKey::UpdateCeEceSeceKeyContext",
            userId, ret, std::to_string(keyType), "");
        return ret;
    }
    LOGI("[L3:KeyManager] TryToFixUserCeEceSeceKey: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int KeyManager::TryToFixUeceKey(unsigned int userId,
                                const std::vector<uint8_t> &token,
                                const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManager] TryToFixUeceKey: >>> ENTER <<< [userId=%{public}u]", userId);
    if (!IamClient::GetInstance().HasPinProtect(userId)) {
        LOGE("User %{public}d has no pin code protect.", userId);
        return E_OK;
    }

    uint64_t secureUid = { 0 };
    if (!secret.empty() && !token.empty()) {
        IamClient::GetInstance().GetSecureUid(userId, secureUid);
        LOGE("Pin code is exist, get secure uid.");
    }
    UserAuth auth = { .token=token, .secret=secret, .secureUid = secureUid };
    UserTokenSecret tokenSecret = { .token = token, .oldSecret = { }, .newSecret = secret, .secureUid = secureUid};

    int ret = UpdateESecret(userId, tokenSecret);
    if (ret != E_OK) {
        LOGE("try to fix elx key failed !");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL5_KEY);
    if (ret != E_OK) {
        LOGE("try to fix elx key context failed !");
        StorageRadar::ReportUpdateUserAuth("TryToFixUeceKey::UpdateCeEceSeceKeyContext", userId, ret, "EL5", "");
        return ret;
    }
    return E_OK;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int KeyManager::RestoreUserKey(uint32_t userId, KeyType type)
{
    LOGI("[L3:KeyManager] RestoreUserKey(Migrate): >>> ENTER <<< [userId=%{public}u, type=%{public}d]", userId, type);
    std::string dir = GetKeyDirByUserAndType(userId, type);
    if (dir == "") {
        LOGE("[L3:KeyManager] RestoreUserKey(Migrate): <<< EXIT FAILED <<< [invalid type=%{public}u]", type);
        return E_PARAMS_INVALID;
    }

    if (!IsDir(dir)) {
        LOGE("[L3:KeyManager] RestoreUserKey(Migrate): <<< EXIT FAILED <<< [directory does not exist]");
        return -ENOENT;
    }
    int32_t ret = RestoreUserKey(userId, dir, NULL_KEY_AUTH, type);
    if (ret == 0 && type != EL1_KEY) {
        saveLockScreenStatus[userId] = true;
        LOGI("[L3:KeyManager] RestoreUserKey(Migrate): user=%{public}u, saveLockScreenStatus=%{public}d",
            userId, saveLockScreenStatus[userId]);
    }
    LOGI("[L3:KeyManager] RestoreUserKey(Migrate): <<< EXIT <<< [retval=%{public}d]", ret);
    return ret;
}
#endif

#ifdef EL5_FILEKEY_MANAGER
int KeyManager::RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    LOGI("[L3:KeyManager] RegisterUeceActivationCallback: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(ueceMutex_);
    if (ueceCallback == nullptr) {
        LOGE("[L3:KeyManager] RegisterUeceActivationCallback: <<< EXIT FAILED <<< [callback is null]");
        return E_PARAMS_INVALID;
    }
    if (ueceCallback_ != nullptr) {
        LOGI("[L3:KeyManager] RegisterUeceActivationCallback: El5FileMgr callback already registered, renewing");
        ueceCallback_ = ueceCallback;
        return E_OK;
    }
    ueceCallback_ = ueceCallback;
    LOGI("[L3:KeyManager] RegisterUeceActivationCallback: <<< EXIT SUCCESS <<< [callback registered]");
    return E_OK;
}

int KeyManager::UnregisterUeceActivationCallback()
{
    LOGI("[L3:KeyManager] UnregisterUeceActivationCallback: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(ueceMutex_);
    if (ueceCallback_ == nullptr) {
        LOGI("[L3:KeyManager] UnregisterUeceActivationCallback: callback already unregistered");
        return E_OK;
    }
    ueceCallback_ = nullptr;
    LOGI("[L3:KeyManager] UnregisterUeceActivationCallback: <<< EXIT SUCCESS <<< [callback unregistered]");
    return E_OK;
}
#endif

int KeyManager::NotifyUeceActivation(uint32_t userId, int32_t resultCode, bool needGetAllAppKey)
{
#ifdef EL5_FILEKEY_MANAGER
    {
        std::lock_guard<std::mutex> lock(ueceMutex_);
        if (ueceCallback_ == nullptr) {
            LOGE("el5 activation callback invalid");
            return E_OK;
        }
    }
    resultCode = (resultCode == E_ACTIVE_REPEATED ? E_OK : resultCode);
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    std::thread callbackThread ([this, userId, resultCode, needGetAllAppKey, p = std::move(promise)]() mutable {
        int32_t retValue = E_OK;
        LOGI("[L3:KeyManager] NotifyUeceActivation: ready for callback, El5 activation result=%{public}d,"
            "userId=%{public}u, needGetAllAppKey=%{public}d", resultCode, userId, needGetAllAppKey);
        std::lock_guard<std::mutex> lock(ueceMutex_);
        if (ueceCallback_ != nullptr) {
            ueceCallback_->OnEl5Activation(resultCode, userId, needGetAllAppKey, retValue);
            StorageRadar::ReportUpdateUserAuth("NotifyUeceActivation", userId, resultCode, "EL5",
                "ueceCallback_ is not nullptr");
        }
        p.set_value(retValue);
    });

    if (future.wait_for(std::chrono::milliseconds(WAIT_THREAD_TIMEOUT_MS)) == std::future_status::timeout) {
        LOGE("[L3:KeyManager] NotifyUeceActivation: el5 activation callback timed out");
        callbackThread.detach();
        std::ostringstream extraData;
        extraData << "Notify EL5 timeout, needGetAllAppKey: " << needGetAllAppKey << " resultCode: " << resultCode;
        StorageRadar::ReportUpdateUserAuth("NotifyUeceActivation", userId, E_TASK_TIME_OUT, "EL5", extraData.str());
        return E_OK;
    }

    auto delay = StorageService::StorageRadar::ReportDuration("UNLOCK USER APP KEYS",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L3:KeyManager] NotifyUeceActivation: SD_DURATION: UNLOCK USER APP KEYS CB: delay time=%{public}s,"
        "isAllAppKey=%{public}d",
        delay.c_str(), needGetAllAppKey);

    int32_t ret = future.get();
    LOGI("[L3:KeyManager] NotifyUeceActivation: unlock app keys ret=%{public}d", ret);
    callbackThread.join();
    if (resultCode != E_OK || ret == E_PARAMS_INVALID) {
        LOGI("[L3:KeyManager] NotifyUeceActivation: <<< EXIT <<< [retval=E_OK]");
        return E_OK;
    }
    LOGI("[L3:KeyManager] NotifyUeceActivation: <<< EXIT SUCCESS <<< [retval=%{public}d]", ret);
    return ret;
# else
    LOGD("[L3:KeyManager] NotifyUeceActivation: EL5_FILEKEY_MANAGER not supported");
    return E_OK;
#endif
}

bool KeyManager::IsDirRecursivelyEmpty(const char* dirPath)
{
    if (dirPath == nullptr || dirPath[0] == '\0') {
        LOGI("[L3:KeyManager] IsDirRecursivelyEmpty: IsDirEmptyAndLogIfNot: dirPath is null/empty");
        return true;
    }
    DIR* dir = opendir(dirPath);
    if (!dir) {
        LOGI("[L3:KeyManager] IsDirRecursivelyEmpty: IsDirEmptyAndLogIfNot:"
            "opendir failed: %s, errno=%d(%s)", dirPath, errno, strerror(errno));
        return true;
    }
    bool empty = true;
    struct dirent* ent = nullptr;
    while ((ent = readdir(dir)) != nullptr) {
        const char* name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }
        empty = false;
        std::string path = std::string(dirPath) + "/" + name;
        struct stat st;
        if (lstat(path.c_str(), &st) < 0) {
            if (errno == ENOENT) {
                continue;
            }
            LOGE("lstat failed: path=%s errno=%d", path.c_str(), errno);
            continue;
        }
        std::string extraData = "path is: " + path +
            " uid is: " + std::to_string(st.st_uid) +
            " gid is: " + std::to_string(st.st_gid);
        StorageService::StorageRadar::ReportCommonResult("dir is not empty", 0, E_DIR_NOT_EMPTY_ERROR, extraData);
    }
    closedir(dir);
    return empty;
}

bool KeyManager::GetSecureUid(uint32_t userId, uint64_t &secureUid)
{
    if ((userId < StorageService::START_APP_CLONE_USER_ID || userId >= StorageService::MAX_APP_CLONE_USER_ID) &&
        IamClient::GetInstance().HasPinProtect(userId)) {
        if (!IamClient::GetInstance().GetSecureUid(userId, secureUid)) {
            LOGE("Get secure uid form iam failed, use default value.");
            return false;
        }
        LOGI("PIN protect exist.");
        return true;
    }
    LOGE("userId: %{public}u check failed", userId);
    return false;
}
} // namespace StorageDaemon
} // namespace OHOS
