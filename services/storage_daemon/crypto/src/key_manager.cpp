/*
 * Copyright (C) 2022-2025 Huawei Device Co., Ltd.
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
        LOGE("kernel not support fscrypt, ret is %{public}d, errno is %{public}d.", kernelSupportVersion, errno);
        return nullptr;
    }
    if ((versionFromPolicy == kernelSupportVersion) && (kernelSupportVersion == FSCRYPT_V2)) {
        return std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>(dir));
    }
    if (versionFromPolicy != kernelSupportVersion) {
        LOGE("version from policy %{public}u not same as version from kernel %{public}u", versionFromPolicy,
             kernelSupportVersion);
    }
    return std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV1>(dir));
}

int KeyManager::GenerateAndInstallDeviceKey(const std::string &dir)
{
    LOGW("enter");
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return E_OK;
    }
    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ == nullptr) {
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_NULLPTR, "EL1", "");
        return E_GLOBAL_KEY_NULLPTR;
    }

    if (globalEl1Key_->InitKey(true) == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key init failed");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_INIT_ERROR, "EL1", "");
        return E_GLOBAL_KEY_INIT_ERROR;
    }
    auto ret = globalEl1Key_->StoreKey(NULL_KEY_AUTH);
    if (ret != E_OK) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key store failed");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_STORE_ERROR, "EL1", "");
        return E_GLOBAL_KEY_STORE_ERROR;
    }

    if (globalEl1Key_->ActiveKey({}, FIRST_CREATE_KEY) != E_OK) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key active failed");
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_ACTIVE_ERROR, "EL1", "");
        return E_GLOBAL_KEY_ACTIVE_ERROR;
    }

    if (globalEl1Key_->UpdateKey() != E_OK) {
        StorageRadar::ReportUserKeyResult("GenerateAndInstallDeviceKey", 0, E_GLOBAL_KEY_UPDATE_ERROR, "EL1", "");
    }
    hasGlobalDeviceKey_ = true;
    LOGW("key create success");
    return 0;
}

int KeyManager::RestoreDeviceKey(const std::string &dir)
{
    LOGI("enter");
    if (globalEl1Key_ != nullptr) {
        LOGI("device key has existed");
        return 0;
    }

    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ == nullptr) {
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_NULLPTR, "EL1", "");
        return E_GLOBAL_KEY_NULLPTR;
    }

    if (globalEl1Key_->InitKey(false) == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key init failed");
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_INIT_ERROR, "EL1", "");
        return E_GLOBAL_KEY_INIT_ERROR;
    }

    auto ret = globalEl1Key_->RestoreKey(NULL_KEY_AUTH);
    if (ret != E_OK) {
        globalEl1Key_ = nullptr;
        LOGE("global security key restore failed");
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_STORE_ERROR, "EL1", "");
        return E_GLOBAL_KEY_STORE_ERROR;
    }

    if (globalEl1Key_->ActiveKey({}, RETRIEVE_KEY) != E_OK) {
        globalEl1Key_ = nullptr;
        LOGE("global security key active failed");
        StorageRadar::ReportUserKeyResult("RestoreDeviceKey", 0, E_GLOBAL_KEY_ACTIVE_ERROR, "EL1", "");
        return E_GLOBAL_KEY_ACTIVE_ERROR;
    }
    hasGlobalDeviceKey_ = true;
    LOGI("key restore success");

    return 0;
}

int KeyManager::InitGlobalDeviceKey(void)
{
    LOGW("enter");
    int ret = InitFscryptPolicy();
    if (ret < 0) {
        LOGE("fscrypt init failed, fscrypt will not be enabled");
        StorageRadar::ReportUserKeyResult("InitGlobalDeviceKey::InitFscryptPolicy", 0, ret, "EL1", "");
        return ret;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    if (hasGlobalDeviceKey_ || globalEl1Key_ != nullptr) {
        LOGI("glabal device el1 have existed");
        return 0;
    }

    ret = MkDir(STORAGE_DAEMON_DIR, S_IRWXU); // para.0700: root only
    if (ret && errno != EEXIST) {
        LOGE("create storage daemon dir error");
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
        LOGE("create device el1 key dir = (/data/service/el0/storage_daemon/sd) error");
        StorageRadar::ReportUserKeyResult("InitGlobalDeviceKey::MkDir", 0, ret, "EL1",
            std::string("errno = ") + std::to_string(errno) + ", path = " + DEVICE_EL1_DIR);
        return ret;
    }

    return GenerateAndInstallDeviceKey(DEVICE_EL1_DIR);
}

int KeyManager::GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGW("enter");
    if (HasElkey(userId, type)) {
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
        LOGE("user security key init failed");
        return E_ELX_KEY_INIT_ERROR;
    }
    auto ret = elKey->StoreKey(auth);
    if (ret != E_OK) {
        elKey->ClearKey();
        LOGE("user security key store failed");
        return E_ELX_KEY_STORE_ERROR;
    }
    // Generate hashkey for encrypt public directory
    elKey->GenerateHashKey();
    if (elKey->ActiveKey(auth.token, FIRST_CREATE_KEY) != E_OK) {
        elKey->ClearKey();
        LOGE("user security key active failed");
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    (void)elKey->UpdateKey();
    if (type >= EL1_KEY && type < EL5_KEY) {
        SaveUserElKey(userId, type, elKey);
    }
    LOGI("key create success");
    return 0;
}

int KeyManager::GenerateAndInstallEl5Key(uint32_t userId, const std::string &dir, const UserAuth &auth)
{
    LOGI("enter");
    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        return E_GLOBAL_KEY_NULLPTR;
    }
    bool isNeedEncryptClassE = true;
    saveESecretStatus[userId] = true;
    auto ret = elKey->AddClassE(isNeedEncryptClassE, saveESecretStatus[userId], FIRST_CREATE_KEY);
    if (ret != E_OK) {
        elKey->ClearKey();
        LOGE("user %{public}u el5 create error, error=%{public}d", userId, ret);
        return E_EL5_ADD_CLASS_ERROR;
    }
    std::string keyDir = GetKeyDirByUserAndType(userId, EL5_KEY);
    if (keyDir == "") {
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
                LOGE("user %{public}u el5 create error", userId);
                return E_EL5_ENCRYPT_CLASS_ERROR;
            }
        }
    } else {
        bool eBufferStatue = false;
        if (elKey->DecryptClassE(auth, saveESecretStatus[userId], eBufferStatue, userId, false) != E_OK) {
            LOGE("user %{public}u decrypt error", userId);
        }
    }
    SaveUserElKey(userId, EL5_KEY, elKey);
    return 0;
}

int KeyManager::RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGI("enter");
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (HasElkey(userId, type)) {
        return E_OK;
    }

    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        return E_GLOBAL_KEY_NULLPTR;
    }

    if (elKey->InitKey(false) == false) {
        LOGE("user security key init failed");
        return E_ELX_KEY_INIT_ERROR;
    }

    auto ret = elKey->RestoreKey(auth);
    if (ret != E_OK) {
        LOGE("user security key restore failed");
        return E_ELX_KEY_STORE_ERROR;
    }

    if (elKey->ActiveKey(auth.token, RETRIEVE_KEY) != E_OK) {
        LOGE("user security key active failed");
        return E_ELX_KEY_ACTIVE_ERROR;
    }

    SaveUserElKey(userId, type, elKey);
    LOGI("key restore success");

    return E_OK;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int32_t KeyManager::ClearAppCloneUserNeedRestore(unsigned int userId, std::string elNeedRestorePath)
{
    LOGI("enter");
    if (userId < StorageService::START_APP_CLONE_USER_ID || userId >= StorageService::MAX_APP_CLONE_USER_ID) {
        LOGI("Clear userId %{public}d out of range", userId);
        return E_USERID_RANGE;
    }

    LOGE("User %{public}d is app clone user, do delete elx need_restore.", userId);
    std::error_code errCode;
    if (!std::filesystem::exists(elNeedRestorePath, errCode)) {
        LOGI("need_restore don't exist, not need to delete.");
        return E_OK;
    }
    (void)remove(elNeedRestorePath.c_str());
    LOGI("Complete delete need_restore.");
    return E_OK;
}
#endif

bool KeyManager::HasElkey(uint32_t userId, KeyType type)
{
    if (userElKeys_.find(userId) != userElKeys_.end()) {
        if (userElKeys_[userId].find(type) != userElKeys_[userId].end()) {
            LOGI("The user %{public}u el %{public}u have existed", userId, type);
            return true;
        }
    }
    LOGE("Have not found user %{public}u key, type %{public}u", userId, type);
    return false;
}

bool KeyManager::IsNeedClearKeyFile(std::string file)
{
    LOGI("enter:");
    std::error_code errCode;
    if (!std::filesystem::exists(file, errCode)) {
        LOGE("file not exist, file is %{private}s", file.c_str());
        return false;
    }

    std::string version;
    if (!OHOS::LoadStringFromFile(file, version)) {
        LOGE("LoadStringFromFile return fail, file is %{private}s", file.c_str());
        return false;
    }

    if (version != DEFAULT_NEED_RESTORE_VERSION && version != DEFAULT_NEED_RESTORE_UPDATE_VERSION) {
        LOGE("need to clear, file is %{private}s, version is %{public}s.", file.c_str(), version.c_str());
        return true;
    }
    LOGE("no need to clear, file is %{private}s, version is %{public}s", file.c_str(), version.c_str());
    return false;
}

void KeyManager::ProcUpgradeKey(const std::vector<FileList> &dirInfo)
{
    LOGI("enter:");
    for (const auto &it : dirInfo) {
        std::string needRestorePath = it.path + "/latest/need_restore";
        if (IsNeedClearKeyFile(needRestorePath)) {
            bool ret = RmDirRecurse(it.path);
            if (!ret) {
                LOGE("remove key dir fail, result is %{public}d, dir %{private}s", ret, it.path.c_str());
            }
        }
    }
}

int KeyManager::LoadAllUsersEl1Key(void)
{
    LOGI("enter");
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
            LOGE("user %{public}u el1 key restore error", item.userId);
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
    return ret;
}

int KeyManager::InitUserElkeyStorageDir(void)
{
    int ret = MkDir(SERVICE_STORAGE_DAEMON_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make service storage daemon dir error");
        return ret;
    }

    ret = MkDir(FSCRYPT_EL_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make service storage daemon dir error");
        return ret;
    }

    ret = MkDir(USER_EL1_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make el1 storage dir error");
        return ret;
    }
    ret = MkDir(USER_EL2_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make el2 storage dir error");
        return ret;
    }
    // 0700 means create el3 permissions
    ret = MkDir(USER_EL3_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make el3 storage dir error");
        return ret;
    }
    // 0700 means create el4 permissions
    ret = MkDir(USER_EL4_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make el4 storage dir error");
        return ret;
    }
    // 0700 means create el5 permissions
    ret = MkDir(USER_EL5_DIR, S_IRWXU);
    if (ret && errno != EEXIST) {
        LOGE("make el5 storage dir error");
        return ret;
    }
    return 0;
}

int KeyManager::InitGlobalUserKeys(void)
{
    LOGW("enter");
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return 0;
    }
    int ret = InitUserElkeyStorageDir();
    if (ret) {
        LOGE("Init user el storage dir failed");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::InitUserElkeyStorageDir", GLOBAL_USER_ID,
            ret, "EL1", "");
        return ret;
    }

    std::string globalUserEl1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
    if (IsDir(globalUserEl1Path)) {
        ret = RestoreUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("Restore el1 failed");
            StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::RestoreUserKey", GLOBAL_USER_ID,
                ret, "EL1", "global user el1 path = " + globalUserEl1Path);
            return ret;
        }
    } else {
        std::lock_guard<std::mutex> lock(keyMutex_);
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("Generate el1 failed");
            StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::GenerateAndInstallUserKey", GLOBAL_USER_ID,
                ret, "EL1", "global user el1 path = " + globalUserEl1Path);
            return ret;
        }
    }

    ret = LoadAllUsersEl1Key();
    if (ret) {
        LOGE("Load all users el1 failed");
        return ret;
    }
    LOGW("Init global user key success");
    return 0;
}

int KeyManager::GenerateUserKeys(unsigned int user, uint32_t flags)
{
    LOGW("start, user:%{public}u", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return 0;
    }
    if ((!IsDir(USER_EL1_DIR)) || (!IsDir(USER_EL2_DIR)) || (!IsDir(USER_EL3_DIR)) ||
        (!IsDir(USER_EL4_DIR)) || (!IsDir(USER_EL5_DIR))) {
        LOGI("El storage dir is not existed");
        return -ENOENT;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = GenerateElxAndInstallUserKey(user);
    if (ret != E_OK) {
        LOGE("Generate ELX failed!");
        return ret;
    }
    LOGW("Create user el success");
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
        LOGE("user el1 create error");
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL1", "el1Path = " + el1Path);
        return ret;
    }

    ret = GenerateAndInstallUserKey(user, el2Path, NULL_KEY_AUTH, EL2_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el2 create error");
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL2", "el2Path = " + el2Path);
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el3Path, NULL_KEY_AUTH, EL3_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el3 create error");
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL3", "el3Path = " + el3Path);
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el4Path, NULL_KEY_AUTH, EL4_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el4 create error");
        StorageRadar::ReportUserKeyResult("GenerateElxAndInstallUserKey", user, ret, "EL4", "el4Path = " + el4Path);
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el5Path, NULL_KEY_AUTH, EL5_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el5 create error");
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
        LOGE("el1 need_restore file is existed, upgrade scene not support.");
        return -EEXIST;
    }
    int ret = GenerateIntegrityDirs(user, EL1_KEY);
    if (ret != -EEXIST) {
        LOGE("GenerateIntegrityDirs el1 failed.");
        StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", user, ret, "EL1", "");
    }
    ret = GenerateIntegrityDirs(user, EL2_KEY);
    if (ret != -EEXIST) {
        LOGE("GenerateIntegrityDirs el2 failed.");
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
        LOGE("user %{public}d el %{public}d is not integrity. create error", userId, type);
        std::string extraData =
            "dir is not Integrity , userId =" + std::to_string(userId) + ", type = " + std::to_string(type);
        StorageRadar::ReportUserKeyResult("GenerateIntegrityDirs", userId, E_DIR_INTEGRITY_ERR, dirType,
                                          extraData);
        int ret = DoDeleteUserCeEceSeceKeys(userId, userDir, type);
        if (ret != E_OK) {
            LOGE("Delete userId=%{public}d el %{public}d key failed", userId, type);
        }

        ret = GenerateUserKeyByType(userId, type, {}, {});
        if (ret != E_OK) {
            LOGE("upgrade scene:generate user key fail, userId %{public}d, KeyType %{public}d", userId, type);
            return ret;
        }

        LOGI("try to destory dir first, user %{public}d, Type %{public}d", userId, type);
        (void)UserManager::GetInstance().DestroyUserDirs(userId, flag_type);
        ret = UserManager::GetInstance().PrepareUserDirs(userId, flag_type);
        if (ret != E_OK) {
            LOGE("upgrade scene:prepare user dirs fail, userId %{public}d, type %{public}d", userId, type);
            return ret;
        }
    }
    LOGI("userId=%{public}d el %{public}d directory is existed, no need fix.", userId, type);
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
    LOGI("start, user:%{public}u, type %{public}u", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return 0;
    }

    std::string elPath = GetKeyDirByType(type);
    if (!IsDir(elPath)) {
        LOGI("El storage dir is not existed");
        return -ENOENT;
    }

    std::string elUserKeyPath = elPath + + "/" + std::to_string(user);
    if (IsDir(elUserKeyPath)) {
        LOGE("user %{public}d el key have existed, create error", user);
        return -EEXIST;
    }
    uint64_t secureUid = { 0 };
    if (!secret.empty() && !token.empty()) {
        IamClient::GetInstance().GetSecureUid(user, secureUid);
        LOGE("token is exist, get secure uid");
    }
    UserAuth auth = { .token = token, .secret = secret, .secureUid = secureUid };
    int ret = GenerateAndInstallUserKey(user, elUserKeyPath, auth, type);
    if (ret) {
        LOGE("user el create error, user %{public}u, type %{public}u", user, type);
        StorageRadar::ReportUserKeyResult("GenerateUserKeyByType::GenerateAndInstallUserKey",
            user, ret, std::to_string(type), "user key path = " + elUserKeyPath);
        return ret;
    }
    LOGI("Create user el success, user %{public}u, type %{public}u", user, type);

    return 0;
}

void KeyManager::DeleteElKey(unsigned int user, KeyType type)
{
    if (userElKeys_.find(user) == userElKeys_.end()) {
        LOGE("The user %{public}u not existed", user);
        return;
    }
    if (userElKeys_[user].find(type) == userElKeys_[user].end()) {
        LOGE("The el%{public}u not existed", type);
        return;
    }
    userElKeys_[user].erase(type);
    if (userElKeys_[user].empty()) {
        userElKeys_.erase(user);
    }
}

int KeyManager::DoDeleteUserCeEceSeceKeys(unsigned int user, const std::string userDir, KeyType type)
{
    LOGI("enter, userDir is %{public}s", userDir.c_str());
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
            LOGE("clear key failed");
            ret = E_CLEAR_KEY_FAILED;
        }
        DeleteElKey(user, type);
        saveLockScreenStatus.erase(user);
    } else {
        std::string elPath = userDir + "/" + std::to_string(user);
        std::shared_ptr<BaseKey> elKey = GetBaseKey(elPath);
        if (elKey == nullptr) {
            LOGE("Malloc el1 Basekey memory failed");
            return E_PARAMS_NULLPTR_ERR;
        }
        if (!elKey->ClearKey()) {
            LOGE("clear key failed");
            ret = E_CLEAR_KEY_FAILED;
        }
    }
    LOGI("end, ret is %{public}d", ret);
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
            LOGE("Delete el%{public}d key failed", i + 1);
            errCode = deleteRet;
            StorageRadar::ReportUserKeyResult("DoDeleteUserKeys", user, errCode,
                "El" + std::to_string(i + 1), elxPath + USER_DIRS[i]);
        }
    }
    return errCode;
}

int KeyManager::DeleteUserKeys(unsigned int user)
{
    LOGI("start, user:%{public}d", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return 0;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoDeleteUserKeys(user);
    LOGI("delete user key end, ret is %{public}d", ret);

    auto userTask = userLockScreenTask_.find(user);
    if (userTask != userLockScreenTask_.end()) {
        userLockScreenTask_.erase(userTask);
        LOGI("Delete user %{public}u, erase user task", user);
    }
    return ret;
}

int KeyManager::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    LOGI("Start EraseAllUserEncryptedKeys");
    bool isDeleteFailed = false;
    if (localIdList.empty()) {
        LOGI("localIdList is empty");
    }
    for (const auto &localId : localIdList) {
        LOGI("Deleting keys for user: %{public}d", localId);
        int32_t ret = DeleteUserKeys(localId);
        if (ret != E_OK) {
            LOGE("EraseAllUserEncryptedKeys failed, please check, user: %{public}d", localId);
            StorageRadar::ReportUserKeyResult("KeyManager::EraseAllUserEncryptedKeys", localId, ret,
                "ELx", "");
            isDeleteFailed = true;
        }
    }
    DeleteUserKeys(ZERO_USER);
    DeleteGlobalDeviceKey(DEVICE_EL1_DIR);
    LOGI("EraseAllUserEncryptedKeys completed, last error: %{public}d", isDeleteFailed);
    return isDeleteFailed ? E_ERASE_USER_KEY_ERROR : E_OK;
}

int KeyManager::DeleteGlobalDeviceKey(const std::string &dir)
{
    LOGI("Start DeleteGlobalDeviceKey");
    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ != nullptr) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        hasGlobalDeviceKey_ = false;
        LOGE("global security key is clear");
    }
    std::string backupDir = dir + BACKUP_NAME;
    globalEl1Key_ = GetBaseKey(backupDir);
    if (globalEl1Key_ != nullptr) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        hasGlobalDeviceKey_ = false;
        LOGE("global security bakup key is clear");
    }
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

    LOGW("enter, param status: user=%{public}d, token=%{public}d, oldSec=%{public}d, newSec=%{public}d", user,
        userTokenSecret.token.empty(), userTokenSecret.oldSecret.empty(), userTokenSecret.newSecret.empty());
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = UpdateESecret(user, userTokenSecret);
    if (ret != 0) {
        LOGE("user %{public}u UpdateESecret fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateESecret", user, ret, "EL5", secretInfo + queryTime);
        return ret;
    }
#ifdef USER_CRYPTO_MIGRATE_KEY
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL2_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el2 key fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_Migrate", user, ret, "EL2", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL3_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el3 key fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_Migrate", user, ret, "EL3", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL4_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el4 key fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth_Migrate", user, ret, "EL4", secretInfo + queryTime);
        return ret;
    }
#else
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL2_KEY);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el2 key fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth", user, ret, "EL2", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL3_KEY);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el3 key fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth", user, ret, "EL3", secretInfo + queryTime);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL4_KEY);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el4 key fail", user);
        queryTime += " UpdateUserAuth: " + BuildTimeInfo(startTime, StorageService::StorageRadar::RecordCurrentTime());
        StorageRadar::ReportUpdateUserAuth("UpdateCeEceSeceUserAuth", user, ret, "EL4", secretInfo + queryTime);
        return ret;
    }
#endif
    return ret;
}

int32_t KeyManager::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
    const std::vector<uint8_t> &newSecret, uint64_t secureUid, uint32_t userId,
    const std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("enter UpdateUseAuthWithRecoveryKey start, user:%{public}d", userId);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);
    std::vector<std::string> elKeyDirs = {el2Path, el3Path, el4Path};

    uint32_t i = 0;
    for (const auto &elxKeyDir : elKeyDirs) {
        if (!IsDir(elxKeyDir)) {
            LOGE("Have not found type %{public}s", elxKeyDir.c_str());
            return E_KEY_TYPE_INVALID;
        }
        std::shared_ptr<BaseKey> elxKey = GetBaseKey(elxKeyDir);
        if (elxKey == nullptr) {
            LOGE("load elx key failed , key path is %{public}s", elxKeyDir.c_str());
            return E_PARAMS_NULLPTR_ERR;
        }

        if (plainText.size() < elKeyDirs.size()) {
            LOGE("plain text size error");
            return E_PARAMS_INVALID;
        }
        KeyBlob originKey(plainText[i]);
        elxKey->SetOriginKey(originKey);
        i++;
        auto ret = elxKey->StoreKey({authToken, newSecret, secureUid});
        if (ret != E_OK) {
            LOGE("Store key error");
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    if (IsUeceSupport()) {
        std::shared_ptr<BaseKey> el5Key = GetBaseKey(std::string(USER_EL5_DIR) + "/" + std::to_string(userId));
        if (!el5Key) {
            return E_PARAMS_NULLPTR_ERR;
        }
        bool tempUeceSupport = true;
        UserAuth userAuth = {.token = authToken, .secret = newSecret, .secureUid = secureUid};
        auto ret = el5Key->EncryptClassE(userAuth, tempUeceSupport, userId, USER_ADD_AUTH);
        if (ret != E_OK) {
            el5Key->ClearKey();
            LOGE("user %{public}u Encrypt E fail", userId);
            return E_EL5_ENCRYPT_CLASS_ERROR;
        }
        ret = el5Key->LockUece(tempUeceSupport);
        if (ret != E_OK) {
            LOGE("lock user %{public}u key failed !", userId);
        }
    }
    return E_OK;
}

int KeyManager::UpdateESecret(unsigned int user, struct UserTokenSecret &tokenSecret)
{
    LOGW("UpdateESecret enter");
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return E_OK;
    }
    std::shared_ptr<BaseKey> el5Key = GetUserElKey(user, EL5_KEY);
    std::string el5Path = std::string(USER_EL5_DIR) + "/" + std::to_string(user);
    if (IsUeceSupport() && el5Key == nullptr) {
        if (!MkDirRecurse(el5Path, S_IRWXU)) {
            LOGE("MkDirRecurse %{public}u failed!", user);
            return E_CREATE_DIR_RECURSIVE_FAILED;
        }
        LOGI("MkDirRecurse %{public}u success!", user);
        el5Key = GetUserElKey(user, EL5_KEY);
    }
    if (el5Key == nullptr) {
        LOGE("Have not found user %{public}u el key", user);
        return E_PARAMS_NULLPTR_ERR;
    }
    if (tokenSecret.newSecret.empty()) {
        return DoChangerPinCodeClassE(user, el5Key);
    }
    if (!tokenSecret.newSecret.empty() && !tokenSecret.oldSecret.empty()) {
        saveESecretStatus[user] = true;
        auto ret = el5Key->ChangePinCodeClassE(saveESecretStatus[user], user);
        if (ret != E_OK) {
            LOGE("user %{public}u ChangePinCodeClassE fail, error=%{public}d", user, ret);
            return E_EL5_UPDATE_CLASS_ERROR;
        }
        return 0;
    }
    uint32_t status = tokenSecret.oldSecret.empty() ? USER_ADD_AUTH : USER_CHANGE_AUTH;
    LOGI("UpdateESecret status is %{public}u", status);
    UserAuth auth = { .token = tokenSecret.token, .secret = tokenSecret.newSecret, .secureUid = tokenSecret.secureUid };
    saveESecretStatus[user] = true;
    auto ret = el5Key->EncryptClassE(auth, saveESecretStatus[user], user, status);
    if (static_cast<uint32_t>(ret) == FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG) {
        LOGE("user= %{public}d, error=FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG, no need to add again", user);
    } else if (ret != E_OK) {
        LOGE("user %{public}u EncryptClassE fail", user);
        return E_EL5_ENCRYPT_CLASS_ERROR;
    }
    el5Key->ClearKeyInfo();
    LOGW("saveESecretStatus is %{public}u", saveESecretStatus[user]);
    return 0;
}

int KeyManager::DoChangerPinCodeClassE(unsigned int user, std::shared_ptr<BaseKey> &el5Key)
{
    auto ret = el5Key->DeleteClassEPinCode(user);
    if (ret != E_OK) {
        LOGE("user %{public}u DeleteClassE fail, error=%{public}d", user, ret);
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
    LOGW("start, user:%{public}d", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return E_OK;
    }
    std::shared_ptr<BaseKey> item = GetUserElKey(user, type);
    if (item == nullptr) {
        LOGE("Have not found user %{public}u el key", user);
        return E_PARAMS_NULLPTR_ERR;
    }

    UserAuth auth = { {}, userTokenSecret.oldSecret, userTokenSecret.secureUid };
    UserAuth auth_newSec = { userTokenSecret.token, userTokenSecret.newSecret, userTokenSecret.secureUid };
    LOGW("param status token:%{public}d, oldSec:%{public}d, newSec:%{public}d", userTokenSecret.token.empty(),
        userTokenSecret.oldSecret.empty(), userTokenSecret.newSecret.empty());
    if (!userTokenSecret.oldSecret.empty()) {
        KeyBlob token(userTokenSecret.token);
        auth.token = std::move(token);
    }
    if ((item->RestoreKey(auth) != E_OK) && (item->RestoreKey(NULL_KEY_AUTH) != E_OK) &&
        (item->RestoreKey(auth_newSec) != E_OK)) {
        LOGE("Restore key error");
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
        LOGE("Store key error");
        return E_ELX_KEY_STORE_ERROR;
    }

    // Generate hashkey for encrypt public directory
    item->GenerateHashKey();
    item->ClearKeyInfo();
    userPinProtect[user] = !userTokenSecret.newSecret.empty();
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
        LOGI("NEED_RESTORE path exist: %{public}s, errcode: %{public}d", need_restore_path.c_str(), errCode.value());
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
            LOGE("GetKeyDirByUserAndType type %{public}u is invalid", type);
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
            LOGE("GetNatoNeedRestorePath type %{public}u is invalid", type);
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
            LOGE("GetKeyDirByType type %{public}u is invalid", type);
            break;
    }
    return keyDir;
}

void KeyManager::SaveUserElKey(unsigned int user, KeyType type, std::shared_ptr<BaseKey> elKey)
{
    if (type >= EL1_KEY && type <= EL5_KEY) {
        userElKeys_[user][type] = elKey;
    } else {
        LOGE("Save el key failed,type:%{public}d", type);
    }
}

std::shared_ptr<BaseKey> KeyManager::GetUserElKey(unsigned int user, KeyType type, bool isSave)
{
    bool isNeedGenerateBaseKey = false;
    std::shared_ptr<BaseKey> elKey = nullptr;
    if (!HasElkey(user, type)) {
        std::string keyDir = GetKeyDirByUserAndType(user, type);
        if (!IsDir(keyDir)) {
            LOGE("Have not found user %{public}u el, %{public}u type", user, type);
            return nullptr;
        }
        elKey = GetBaseKey(keyDir);
        if (elKey == nullptr) {
            LOGE("BaseKey memory failed");
            return nullptr;
        }
        isNeedGenerateBaseKey = true;
        LOGI("Generate new baseKey type: %{public}u", type);
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
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return E_OK;
    }
    int ret = CheckNeedRestoreVersion(user, type);
    if (ret == -EFAULT || ret == -ENOENT) {
        return ret;
    }
    if (CheckUserPinProtect(user, token, secret) != E_OK) {
        LOGE("IAM & Storage mismatch, wait user input pin.");
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
        LOGE("elKey failed");
        return -EOPNOTSUPP;
    }
    if (type == EL5_KEY) {
        return ActiveUece(user, elKey, token, secret);
    }
    if (ActiveElXUserKey(user, token, type, secret, elKey) != 0) {
        LOGE("ActiveElXUserKey failed");
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    SaveUserElKey(user, type, elKey);
    userPinProtect[user] = !secret.empty();
    saveLockScreenStatus[user] = true;
    LOGI("Active user %{public}u el success, saveLockScreenStatus is %{public}d", user, saveLockScreenStatus[user]);
    return E_OK;
}

int KeyManager::ActiveElxUserKey4Nato(unsigned int user, KeyType type, const KeyBlob &authToken)
{
    LOGW("Active Elx user key for nato for userId=%{public}d, keyType=%{public}u", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::string keyDir = GetNatoNeedRestorePath(user, type);
    if (keyDir == "") {
        return E_KEY_TYPE_INVALID;
    }
    if (!CheckDir(type, keyDir, user)) {
        return E_NATO_CHECK_KEY_DIR_ERROR;
    }
    std::shared_ptr<BaseKey> elKey = GetBaseKey(keyDir);
    if (elKey == nullptr) {
        LOGE("GetBaseKey nato for userId=%{public}d el%{public}u failed.", user, type);
        return E_PARAMS_NULLPTR_ERR;
    }
    if (!elKey->InitKey(false)) {
        LOGE("InitKey nato for userId=%{public}d el%{public}u failed.", user, type);
        return E_NATO_INIT_USER_KEY_ERROR;
    }
    if (elKey->RestoreKey4Nato(keyDir, type) != E_OK) {
        LOGE("RestoreKey nato for userId=%{public}d el%{public}u failed.", user, type);
        return E_NATO_RESTORE_USER_KEY_ERROR;
    }
    if (elKey->ActiveKey(authToken, RETRIEVE_KEY) != E_OK) {
        LOGE("ActiveKey nato for userId=%{public}d el%{public}u failed.", user, type);
        return E_NATO_ACTIVE_EL4_KEY_ERROR;
    }
    LOGW("Active Elx user key for nato for userId=%{public}d, keyType=%{public}u success.", user, type);
    return E_OK;
}

int KeyManager::ActiveUece(unsigned int user,
                           std::shared_ptr<BaseKey> elKey,
                           const std::vector<uint8_t> &token,
                           const std::vector<uint8_t> &secret)
{
    if (ActiveUeceUserKey(user, token, secret, elKey) != 0) {
        LOGE("ActiveUeceUserKey failed");
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    return 0;
}

bool KeyManager::CheckDir(KeyType type, std::string keyDir, unsigned int user)
{
    if ((type != EL5_KEY) && !IsDir(keyDir)) {
        LOGE("Have not found user %{public}u el", user);
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
            LOGI("The ElKey is nullptr: %{public}d", elKey == nullptr);
            return false;
        }

        if (!elKey->KeyDescIsEmpty()) {
            LOGI("user el%{public}u key desc has existed", type);
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
        LOGE("Have not found dir %{public}u el5", user);
        return -ENOENT;
    }

    if (IsDir(keyDir) && std::filesystem::is_empty(keyDir)) {
        OHOS::ForceRemoveDirectory(keyDir);
        LOGE("Have removed key dir %{public}u el5", user);
        return -ENOENT;
    }
    return 0;
}

bool KeyManager::GetUserDelayHandler(uint32_t userId, std::shared_ptr<DelayHandler> &delayHandler)
{
    LOGI("enter");
    auto iterTask = userLockScreenTask_.find(userId);
    if (iterTask == userLockScreenTask_.end()) {
        userLockScreenTask_[userId] = std::make_shared<DelayHandler>(userId);
    }
    delayHandler = userLockScreenTask_[userId];
    if (delayHandler == nullptr) {
        LOGE("user %{public}d delayHandler is nullptr !", userId);
        return false;
    }
    return true;
}

int KeyManager::ActiveUeceUserKey(unsigned int user,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey)
{
    saveESecretStatus[user] = !secret.empty();
    LOGW("userId %{public}u, token empty %{public}d sec empty %{public}d", user, token.empty(), secret.empty());
    SaveUserElKey(user, EL5_KEY, elKey);
    UserAuth auth = { .token = token, .secret = secret };
    bool eBufferStatue = false;
    auto ret = elKey->DecryptClassE(auth, saveESecretStatus[user], eBufferStatue, user, true);
    if (ret != E_OK) {
        LOGE("Unlock user %{public}u E_Class failed", user);
        return E_EL5_DELETE_CLASS_ERROR;
    }

    if (!token.empty() && !secret.empty() && eBufferStatue) {
        if (TryToFixUeceKey(user, token, secret) != E_OK) {
            LOGE("TryToFixUeceKey el5 failed !");
            return E_TRY_TO_FIX_USER_KEY_ERR;
        }
    }
    LOGW("ActiveCeSceSeceUserKey user %{public}u, saveESecretStatus %{public}d", user, saveESecretStatus[user]);
    return 0;
}

int KeyManager::ActiveElXUserKey(unsigned int user,
                                 const std::vector<uint8_t> &token, KeyType keyType,
                                 const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey)
{
    if (elKey->InitKey(false) == false) {
        LOGE("Init el failed");
        return E_ELX_KEY_INIT_ERROR;
    }
    UserAuth auth = { token, secret };
    auto keyResult = elKey->RestoreKey(auth);
    bool noKeyResult = (keyResult != E_OK) && (elKey->RestoreKey(NULL_KEY_AUTH) == E_OK);
    // key and no-key situation all failed, include upgrade situation, return err
    if (keyResult != E_OK && !noKeyResult) {
        LOGE("Restore el failed, type: %{public}u", keyType);
        return E_RESTORE_KEY_FAILED;
    }
    // if device has pwd and decrypt success, continue.otherwise try no pwd and fix situation.
    if (keyResult != E_OK && noKeyResult) {
        if (TryToFixUserCeEceSeceKey(user, keyType, token, secret) != E_OK) {
            LOGE("TryToFixUserCeEceSeceKey elx failed, type %{public}u", keyType);
            return E_TRY_TO_FIX_USER_KEY_ERR;
        }
    }
    std::string NEED_UPDATE_PATH = GetKeyDirByUserAndType(user, keyType) + PATH_LATEST + SUFFIX_NEED_UPDATE;
    std::string NEED_RESTORE_PATH = GetKeyDirByUserAndType(user, keyType) + PATH_LATEST + SUFFIX_NEED_RESTORE;
    if (!FileExists(NEED_RESTORE_PATH) && !FileExists(NEED_UPDATE_PATH)) {
        auto ret = elKey->StoreKey(auth);
        if (ret != E_OK) {
            LOGE("Store el failed");
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    // Generate hashkey for encrypt public directory
    elKey->GenerateHashKey();
    if (elKey->ActiveKey(auth.token, RETRIEVE_KEY) != E_OK) {
        LOGE("Active user %{public}u key failed", user);
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    return 0;
}

int KeyManager::UnlockUserScreen(uint32_t user, const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGI("start");
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
        LOGE("user ce does not decrypt, skip");
        return 0;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        saveLockScreenStatus[user] = true;
        LOGI("saveLockScreenStatus is %{public}d", saveLockScreenStatus[user]);
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
    LOGW("UnlockUserScreen user %{public}u el3 and el4 success and saveLockScreenStatus is %{public}d", user,
         saveLockScreenStatus[user]);
    return 0;
}

int32_t KeyManager::UnlockEceSece(uint32_t user,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret)
{
    auto el4Key = GetUserElKey(user, EL4_KEY);
    if (el4Key == nullptr) {
        saveLockScreenStatus[user] = true;
        LOGE("The user %{public}u not been actived and saveLockScreenStatus is %{public}d", user,
             saveLockScreenStatus[user]);
        return E_NON_EXIST;
    }
    if (el4Key->RestoreKey({ token, secret }, false) != E_OK && el4Key->RestoreKey(NULL_KEY_AUTH, false) != E_OK) {
        LOGE("Restore user %{public}u el4 key failed", user);
        return E_RESTORE_KEY_FAILED;
    }
    int32_t ret = el4Key->UnlockUserScreen(token, user, FSCRYPT_SDP_ECE_CLASS);
    if (ret != E_OK) {
        LOGE("UnlockUserScreen user %{public}u el4 key failed", user);
        return E_UNLOCK_SCREEN_FAILED;
    }
    LOGI("DecryptClassE user %{public}u saveESecretStatus %{public}d", user, saveESecretStatus[user]);
    return E_OK;
}

int32_t KeyManager::UnlockUece(uint32_t user,
                               const std::vector<uint8_t> &token,
                               const std::vector<uint8_t> &secret)
{
    UserAuth auth = {.token = token, .secret = secret};
    saveESecretStatus[user] = !auth.token.IsEmpty();
    auto el5Key = GetUserElKey(user, EL5_KEY);
    bool eBufferStatue = false;
    if (el5Key != nullptr) {
        auto ret = el5Key->DecryptClassE(auth, saveESecretStatus[user], eBufferStatue, user, false);
        if (ret != E_OK) {
            LOGE("Unlock user %{public}u uece failed", user);
            return ret;
        }
    }
    return E_OK;
}

int KeyManager::GetLockScreenStatus(uint32_t user, bool &lockScreenStatus)
{
    getLockStatusTime_[LOCK_STATUS_START] = StorageService::StorageRadar::RecordCurrentTime();
    LOGI("start");
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto iter = saveLockScreenStatus.find(user);
    lockScreenStatus = (iter == saveLockScreenStatus.end()) ? false: iter->second;
    LOGW("lockScreenStatus is %{public}d", lockScreenStatus);
    getLockStatusTime_[LOCK_STATUS_END] = StorageService::StorageRadar::RecordCurrentTime();
    return 0;
}

int KeyManager::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    if (!IsUeceSupport()) {
        LOGI("Not support uece or encryption not enabled!");
        return -ENOTSUP;
    }
    LOGI("enter");
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (needReSet) {
        return GenerateAppkeyWithRecover(userId, hashId, keyId);
    }

    if (userId == KEY_RECOVERY_USER_ID) {
        LOGI("GenerateAppKey when RecoverKey");
        auto el5Key = GetBaseKey(GetKeyDirByUserAndType(userId, EL5_KEY));
        if (el5Key == nullptr) {
            LOGE("el5Key is nullptr");
            return E_PARAMS_NULLPTR_ERR;
        }
        auto ret = el5Key->GenerateAppkey(userId, hashId, keyId);
        if (ret != E_OK) {
            LOGE("Failed to generate Appkey2, error=%{public}d", ret);
            return E_EL5_GENERATE_APP_KEY_ERR;
        }
        return 0;
    }
    auto el5Key = GetBaseKey(GetKeyDirByUserAndType(userId, EL5_KEY));
    if (el5Key == nullptr) {
        LOGE("el5Key is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    auto ret = el5Key->GenerateAppkey(userId, hashId, keyId);
    if (ret != E_OK) {
        LOGE("Failed to generate Appkey2 error=%{public}d", ret);
        return E_EL5_GENERATE_APP_KEY_ERR;
    }

    return 0;
}

int KeyManager::GenerateAppkeyWithRecover(uint32_t userId, uint32_t hashId, std::string &keyId)
{
    LOGI("GenerateAppkey needReSet");
    std::string el5Path = std::string(MAINTAIN_USER_EL5_DIR) + "/" + std::to_string(userId);
    auto el5Key = GetBaseKey(el5Path);
    if (el5Key == nullptr) {
        LOGE("el5Key is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    auto ret = el5Key->GenerateAppkey(userId, hashId, keyId);
    if (ret != E_OK) {
        LOGE("Failed to generate Appkey2 error=%{public}d", ret);
        return E_EL5_GENERATE_APP_KEY_WITH_RECOVERY_ERR;
    }
    ret = el5Key->DeleteClassEPinCode(userId);
    if (ret != E_OK) {
        LOGE("GenerateAppkey DeleteClassEPinCode failed");
        return E_EL5_DELETE_CLASS_WITH_RECOVERY_ERR;
    }
    saveESecretStatus[userId] = false;
    return 0;
}

int KeyManager::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    if (!IsUeceSupport()) {
        LOGI("Not support uece or encryption not enabled!");
        return -ENOTSUP;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto el5Key = GetBaseKey(GetKeyDirByUserAndType(userId, EL5_KEY));
    if (el5Key == nullptr) {
        LOGE("el5Key is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    if (el5Key->DeleteAppkey(keyId) != E_OK) {
        LOGE("Failed to delete Appkey2");
        return E_EL5_DELETE_APP_KEY_ERR;
    }
    return 0;
}

int KeyManager::CreateRecoverKey(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
                                 const std::vector<uint8_t> &secret)
{
    LOGI("enter");
    std::string globalUserEl1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
    std::string el1Path = std::string(USER_EL1_DIR) + "/" + std::to_string(userId);
    std::string el2Path = std::string(USER_EL2_DIR) + "/" + std::to_string(userId);
    std::string el3Path = std::string(USER_EL3_DIR) + "/" + std::to_string(userId);
    std::string el4Path = std::string(USER_EL4_DIR) + "/" + std::to_string(userId);
    std::vector<std::string> elKeyDirs = { DEVICE_EL1_DIR, globalUserEl1Path, el1Path, el2Path, el3Path, el4Path };
    std::vector<KeyBlob> originKeys;
    for (const auto &elxKeyDir : elKeyDirs) {
        if (!IsDir(elxKeyDir)) {
            LOGE("Have not found type %{public}s el", elxKeyDir.c_str());
            return E_KEY_TYPE_INVALID;
        }
        std::shared_ptr<BaseKey> elxKey = GetBaseKey(elxKeyDir);
        if (elxKey == nullptr) {
            LOGE("load elx key failed , key path is %{public}s", elxKeyDir.c_str());
            return E_PARAMS_NULLPTR_ERR;
        }
        UserAuth auth = { token, secret };
        if (secret.empty() && token.size() == RECOVERY_TOKEN_CHALLENGE_LENG) {
            LOGW("secret is empty, use none token.");
            auth = { {}, {}};
        }
        if ((elxKey->RestoreKey(auth, false) != E_OK) && (elxKey->RestoreKey(NULL_KEY_AUTH, false) != E_OK)) {
            LOGE("Restore el failed");
            return E_RESTORE_KEY_FAILED;
        }
        KeyBlob originKey;
        if (!elxKey->GetOriginKey(originKey)) {
            LOGE("get origin key failed !");
            return -ENOENT;
        }
        originKeys.push_back(std::move(originKey));
    }
    int ret = RecoveryManager::GetInstance().CreateRecoverKey(userId, userType, token, secret, originKeys);
    if (ret != E_OK) {
        LOGE("Create recovery key failed !");
        return ret;
    }
    originKeys.clear();
    return E_OK;
}

int KeyManager::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("enter");
    std::vector<KeyBlob> originIvs;
    if (RecoveryManager::GetInstance().SetRecoverKey(key) != E_OK) {
        LOGE("Set recovery key filed !");
        return E_SET_RECOVERY_KEY_ERR;
    }
    return E_OK;
}

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
int32_t KeyManager::FileBasedEncryptfsMount()
{
    std::string srcPath = FILE_BASED_ENCRYPT_SRC_PATH;
    std::string dstPath = FILE_BASED_ENCRYPT_DST_PATH;
    errno = 0;
    int32_t ret = UMount(dstPath);
    if (ret != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("failed to unmount file based encrypt fs, err %{public}d", errno);
        std::string extraData = "srcPath=" + srcPath + ",dstPath=" + dstPath + ",kernelCode=" + std::to_string(errno);
        StorageRadar::ReportUserManager("FileBasedEncryptfsMount", DEFAULT_REPAIR_USERID, E_UMOUNT_FBE, extraData);
        return E_UMOUNT_FBE;
    }
    errno = 0;
    ret = Mount(srcPath, dstPath, nullptr, MS_BIND, nullptr);
    if (ret != E_OK && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to bind mount file based encrypt fs, err %{public}d", errno);
        std::string extraData = "srcPath=" + srcPath + ",dstPath=" + dstPath + ",kernelCode=" + std::to_string(errno);
        StorageRadar::ReportUserManager("FileBasedEncryptfsMount", DEFAULT_REPAIR_USERID, E_MOUNT_FBE, extraData);
        return E_MOUNT_FBE;
    }
    LOGI("bind mount file based encrypt fs success, err %{public}d", errno);
    return E_OK;
}

int32_t KeyManager::InstallEmptyUserKeyForRecovery(uint32_t userId)
{
    LOGI("enter userId=%{public}d", userId);
    int32_t ret = E_OK;
    if (userId == KEY_RECOVERY_USER_ID) {
        ret = GenerateElxAndInstallUserKey(userId);
        LOGI("InstallEmptyUserKeyForRecovery for KEY_RECOVERY_USER, ret=%{public}d", ret);
        ret = FileBasedEncryptfsMount();
        if (ret != E_OK) {
            LOGE("mount file based encrypt fs failed for recovery, ret:%{public}d!", ret);
            return ret;
        }
        // Since the EL1_KEY corresponding to GLOBAL_USER_ID has already been cached during boot - pointing to the key
        // path under `/data` - attempting to regenerate the key to point to `/mnt/data_old` will be intercepted.
        // Therefore, the cache must be cleared first.
        // In recovery mode, the key pointing to `/data` is actually located in the ramdisk. This has no impact on
        // normal system operation after recovery.
        ret = GenerateAndInstallUserKey(userId, MAINTAIN_DEVICE_EL1_DIR, NULL_KEY_AUTH, EL0_KEY);
        LOGI("InstallEmptyUserKeyForRecovery elxDir=%{public}s, ret=%{public}d", MAINTAIN_DEVICE_EL1_DIR, ret);
        DeleteElKey(GLOBAL_USER_ID, EL1_KEY);
        std::string globalUserEl1Path = std::string(MAINTAIN_USER_EL1_DIR) + "/" + std::to_string(GLOBAL_USER_ID);
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        LOGI("InstallEmptyUserKeyForRecovery elxDir=%{public}s, ret=%{public}d", globalUserEl1Path.c_str(), ret);
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
        LOGI("InstallEmptyUserKeyForRecovery elxDir=%{public}s, ret=%{public}d", elxDir.c_str(), ret);
    }
    return ret;
}
#endif

int32_t KeyManager::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
    LOGI("enter ResetSecretWithRecoveryKey start, user:%{public}d", userId);
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    if (!IsEncryption()) {
        return InstallEmptyUserKeyForRecovery(userId);
    }
    std::vector<KeyBlob> originIvs;
    auto ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key, originIvs);
    if (ret != E_OK) {
        LOGE("ResetSecretWithRecoveryKey filed !");
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
        LOGE("plain text size error");
        return E_PARAMS_INVALID;
    }

    uint32_t i = 0;
    for (const auto &elxKeyDir : elKeyDirs) {
        if (!IsDir(elxKeyDir)) {
            LOGE("Have not found type %{public}s", elxKeyDir.c_str());
            return E_KEY_TYPE_INVALID;
        }
        std::shared_ptr<BaseKey> elxKey = GetBaseKey(elxKeyDir);
        if (elxKey == nullptr) {
            LOGE("load elx key failed , key path is %{public}s", elxKeyDir.c_str());
            return E_PARAMS_NULLPTR_ERR;
        }

        elxKey->SetOriginKey(originIvs[i]);
        i++;
        auto ret = elxKey->StoreKey(NULL_KEY_AUTH);
        if (ret != E_OK) {
            LOGE("Store key error");
            return E_ELX_KEY_STORE_ERROR;
        }
    }
    ret = FileBasedEncryptfsMount();
    if (ret != E_OK) {
        LOGE("mount file based encrypt fs failed!");
        return ret;
    }
#endif
    return E_OK;
}

int KeyManager::InActiveUserKey(unsigned int user)
{
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    KeyType types[] = { EL2_KEY, EL3_KEY, EL4_KEY, EL5_KEY };
    int ret = 0;
    for (KeyType type : types) {
        ret = InactiveUserElKey(user, type);
        if (ret != E_OK) {
            LOGE("Inactive user El%{public}d key failed", type);
            StorageRadar::ReportUserKeyResult("InactiveUserElKey", user, ret, "EL" + std::to_string(type), "");
            return ret;
        }
    }
    auto userTask = userLockScreenTask_.find(user);
    if (userTask != userLockScreenTask_.end()) {
        userLockScreenTask_.erase(userTask);
        LOGI("InActive user %{public}u, erase user task", user);
    }
    return 0;
}

int KeyManager::InactiveUserElKey(unsigned int user, KeyType type)
{
    std::shared_ptr<BaseKey> elKey;
    if (!HasElkey(user, type)) {
        LOGE("Have not found user %{public}u type %{public}u", user, type);
        std::string keyDir = GetKeyDirByUserAndType(user, type);
        if (type != EL5_KEY && !IsDir(keyDir)) {
            LOGE("have not found user %{public}u, type %{public}u", user, type);
            return E_PARAMS_INVALID;
        }
        elKey = GetBaseKey(keyDir);
    } else {
        elKey = userElKeys_[user][type];
    }
    if (elKey == nullptr) {
        LOGE("BaseKey memory failed");
        return E_PARAMS_INVALID;
    }
    if (elKey->InactiveKey(USER_LOGOUT) != E_OK) {
        LOGE("Clear user %{public}u key failed", user);
        return E_ELX_KEY_INACTIVE_ERROR;
    }
    LOGI("remove elx desc");
    auto elx = elKey->GetKeyDir();
    if (!elx.empty() && elx != "el1") {
        std::string descElx = std::string(FSCRYPT_EL_DIR) + "/" + elx + "/" + std::to_string(user) + DESC_DIR;
        (void)remove(descElx.c_str());
        LOGI("remove desc success.");
    }
    DeleteElKey(user, type);
    LOGI("Inactive user %{public}u elX success", user);
    return 0;
}

int KeyManager::LockUserScreen(uint32_t user)
{
    LOGI("start");
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::error_code errCode;
    if (!IsUserCeDecrypt(user) || std::filesystem::exists(GetNatoNeedRestorePath(user, EL4_KEY), errCode)) {
        LOGE("user ce does not decrypt, skip");
        return 0;
    }

    auto iter = userPinProtect.find(user);
    if (iter == userPinProtect.end() || iter->second == false) {
        if (!IamClient::GetInstance().HasPinProtect(user)) {
            LOGI("Has no pin protect, saveLockScreenStatus is %{public}d", saveLockScreenStatus[user]);
            return 0;
        }
        userPinProtect.erase(user);
        userPinProtect.insert(std::make_pair(user, true));
        LOGI("User is %{public}u ,Lock screen, SaveLockScreenStatus is %{public}d", user, saveLockScreenStatus[user]);
    }
    iter = saveLockScreenStatus.find(user);
    if (iter == saveLockScreenStatus.end()) {
        saveLockScreenStatus.insert(std::make_pair(user, false));
        LOGI("User is %{public}u ,Insert LockScreenStatus, SaveLockScreenStatus is %{public}d", user,
             saveLockScreenStatus[user]);
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        saveLockScreenStatus[user] = false;
        LOGI("KeyCtrlHasFscryptSyspara is false, saveLockScreenStatus is %{public}d",
            saveLockScreenStatus[user]);
        return 0;
    }
    auto el5Key = GetUserElKey(user, EL5_KEY, false);
    saveESecretStatus[user] = true;
    if (el5Key != nullptr && el5Key->LockUece(saveESecretStatus[user]) != E_OK) {
        LOGE("lock user %{public}u el5 key failed !", user);
    }
    auto el4Key = GetUserElKey(user, EL4_KEY, false);
    if (el4Key == nullptr) {
        LOGE("Have not found user %{public}u el3 or el4", user);
        StorageRadar::ReportUpdateUserAuth("LockUserScreen::GetUserElKey", user, E_NON_EXIST, "EL4", "not found key");
        return E_NON_EXIST;
    }
    std::shared_ptr<DelayHandler> userDelayHandler;
    if (GetUserDelayHandler(user, userDelayHandler)) {
        userDelayHandler->StartDelayTask(el4Key);
    }

    saveLockScreenStatus[user] = false;
    LOGI("LockUserScreen user %{public}u el3 and el4 success, saveLockScreenStatus is %{public}d",
        user, saveLockScreenStatus[user]);
    return 0;
}

int KeyManager::SetDirectoryElPolicy(unsigned int user, KeyType type, const std::vector<FileList> &vec)
{
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara() || !IsEncryption()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
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
        LOGE("Not specify el flags, no need to crypt");
        return 0;
    }
    if (getElxKeyPath(user, type, eceSeceKeyPath) != 0) {
        LOGE("method getEceSeceKeyPath fail");
        return -ENOENT;
    }
    for (auto item : vec) {
        int ret = LoadAndSetPolicy(keyPath.c_str(), item.path.c_str());
        if (ret != 0) {
            LOGE("Set directory el policy error, ret: %{public}d, path:%{public}s", ret, item.path.c_str());
            return E_LOAD_AND_SET_POLICY_ERR;
        }
    }
    if (type == EL3_KEY || type == EL4_KEY) {
        for (auto item : vec) {
            if (LoadAndSetEceAndSecePolicy(eceSeceKeyPath.c_str(), item.path.c_str(), static_cast<int>(type)) != 0) {
                LOGE("Set directory el policy error!");
                return E_LOAD_AND_SET_ECE_POLICY_ERR;
            }
        }
    }
    LOGW("Set user %{public}u el policy success", user);
    return 0;
}

int32_t KeyManager::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath,
    StorageService::EncryptionLevel level)
{
    LOGI("KeyManager::SetDirEncryptionPolicy begin!");
    if (!KeyCtrlHasFscryptSyspara() || !IsEncryption()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return E_NOT_SUPPORT;
    }

    bool isCeEncrypt = false;
    auto ret = GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("User el2 has not decrypt, userId is %{public}u", userId);
        return E_KEY_NOT_ACTIVED;
    }

    std::string keyPath;
    ret = ((level == EL1_SYS_KEY) || (level == EL1_USER_KEY)) ? getElxKeyPath(userId, EL1_KEY, keyPath)
        : getElxKeyPath(userId, EL2_KEY, keyPath);
    if (ret != E_OK) {
        LOGE("GetkeyPath fail, userId is %{public}u, level is %{public}u", userId, level);
        return ret;
    }

    ret = LoadAndSetPolicy(keyPath.c_str(), dirPath.c_str());
    if (ret != E_OK) {
        LOGE("SetDirEncryptionPolicy failed, userId is %{public}u, level is %{public}u", userId, level);
        return ret;
    }

    if (level == EL3_USER_KEY || level == EL4_USER_KEY) {
        std::string eceSeceKeyPath;
        KeyType tempType = level == EL3_USER_KEY ? EL3_KEY : EL4_KEY;
        ret = getElxKeyPath(userId, tempType, eceSeceKeyPath);
        if (ret != E_OK) {
            LOGE("getkeyPath fail, userId is %{public}u, level is %{public}u", userId, level);
            return ret;
        }
        ret = LoadAndSetEceAndSecePolicy(eceSeceKeyPath.c_str(), dirPath.c_str(), static_cast<int>(level));
        if (ret != E_OK) {
            LOGE("Set directory EceAndSece policy error!, userId is %{public}u, level is %{public}u", userId, level);
            return ret;
        }
    }
    LOGI("KeyManager::SetDirEncryptionPolicy success!");
    return E_OK;
}

int KeyManager::getElxKeyPath(unsigned int user, KeyType type, std::string &elxKeyPath)
{
    std::string natoPath = GetNatoNeedRestorePath(user, type);
    std::error_code errCode;
    if (std::filesystem::exists(natoPath, errCode)) {
        LOGW("type=%{public}d NATO path is exist.", type);
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
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGW("FscryptSyspara has not or encryption not enabled");
        return 0;
    }
    if (HasElkey(userId, type) == false) {
        return E_PARAMS_INVALID;
    }
    std::shared_ptr<BaseKey> elKey = GetUserElKey(userId, type);
    if (elKey == nullptr) {
        LOGE("Have not found user %{public}u, type el%{public}u", userId, type);
        return -ENOENT;
    }
    auto ret = elKey->UpdateKey();
    if (ret != E_OK) {
        LOGE("Basekey update newest context failed");
        return E_ELX_KEY_UPDATE_ERROR;
    }
    return 0;
}

int KeyManager::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("UpdateKeyContext enter");
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = UpdateCeEceSeceKeyContext(userId, EL2_KEY);
    if (ret != 0) {
        LOGE("Basekey update EL2 newest context failed");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL2", "");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL3_KEY);
    if (ret != 0) {
        LOGE("Basekey update EL3 newest context failed");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL3", "");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL4_KEY);
    if (ret != 0) {
        LOGE("Basekey update EL4 newest context failed");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL4", "");
        return ret;
    }
    if (IsUeceSupport()) {
        ret = UpdateClassEBackUpFix(userId);
        if (ret != 0) {
            LOGE("Inform FBE do update class E backup failed, ret=%{public}d", ret);
            return ret;
        }
        if (saveESecretStatus[userId]) {
            ret = UpdateCeEceSeceKeyContext(userId, EL5_KEY);
        }
    }
    if (ret != 0 && ((userId < START_APP_CLONE_USER_ID || userId > MAX_APP_CLONE_USER_ID))) {
        LOGE("Basekey update EL5 newest context failed");
        StorageRadar::ReportUpdateUserAuth("UpdateKeyContext::UpdateCeEceSeceKeyContext", userId, ret, "EL5", "");
        return ret;
    }
    LOGI("Basekey update key context success");
    return 0;
}

bool KeyManager::IsUeceSupport()
{
    int fd = open(UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("uece does not support !");
        }
        LOGE("open uece failed, errno : %{public}d", errno);
        return false;
    }
    close(fd);
    LOGI("uece is support.");
    return true;
}

int KeyManager::UpdateClassEBackUpFix(uint32_t userId)
{
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
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
            LOGE("uece does not support !");
            return ENOENT;
        }
        LOGE("open uece failed, errno : %{public}d", errno);
        return errno;
    }
    close(fd);
    LOGI("uece is support.");
    return E_OK;
}

int KeyManager::UpgradeKeys(const std::vector<FileList> &dirInfo)
{
    for (const auto &it : dirInfo) {
        std::shared_ptr<BaseKey> elKey = GetBaseKey(it.path);
        if (elKey == nullptr) {
            LOGE("Basekey memory failed");
            continue;
        }
        elKey->UpgradeKeys();
    }
    return 0;
}

int KeyManager::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGI("Begin check encrypted status, userId is %{public}d, needCheckDirMount is %{public}d",
         userId, needCheckDirMount);
    isEncrypted = true;
    const char rootPath[] = "/data/app/el2/";
    const char basePath[] = "/base";
    size_t allPathSize = strlen(rootPath) + strlen(basePath) + 1 + USER_ID_SIZE_VALUE;
    char *path = reinterpret_cast<char *>(malloc(sizeof(char) * (allPathSize)));
    if (path == nullptr) {
        LOGE("Failed to malloce path.");
        return E_MEMORY_OPERATION_ERR;
    }
    int len = sprintf_s(path, allPathSize, "%s%u%s", rootPath, userId, basePath);
    if (len <= 0 || (size_t)len >= allPathSize) {
        free(path);
        LOGE("Failed to get base path");
        return E_PARAMS_INVALID;
    }
    if (access(path, F_OK) != 0) {
        free(path);
        LOGE("Can not access el2 dir, user %{public}d el2 is encrypted.", userId);
        return E_OK;
    }
    std::string el2Path(path);
    if (!SaveStringToFile(el2Path + EL2_ENCRYPT_TMP_FILE, " ")) {
        free(path);
        LOGE("Can not save el2 file, user %{public}d el2 is encrypted.", userId);
        return E_OK;
    }
    free(path);
    int ret = remove((el2Path + EL2_ENCRYPT_TMP_FILE).c_str());
    LOGE("remove ret = %{public}d", ret);
    if (needCheckDirMount && !MountManager::GetInstance().CheckMountFileByUser(userId)) {
        LOGI("The virturalDir is not exists.");
        return E_OK;
    }
    isEncrypted = false;
    LOGI("This is decrypted status");
    return E_OK;
}

bool KeyManager::IsUserCeDecrypt(uint32_t userId)
{
    bool isCeEncrypt = false;
    int ret = GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("User %{public}d el2 has not decrypt.", userId);
        return false;
    }
    LOGI("User %{public}d el2 decrypted.", userId);
    return true;
}

int KeyManager::CheckUserPinProtect(unsigned int userId,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret)
{
    LOGI("enter CheckUserPinProtect");
    std::error_code errCode;
    std::string restorePath = std::string(USER_EL2_DIR) + "/" + std::to_string(userId) + RESTORE_DIR;
    if (!std::filesystem::exists(restorePath, errCode)) {
        LOGI("Do not check pin code.");
        return E_OK;
    }
    // judge if device has PIN protect
    if ((token.empty() && secret.empty()) && IamClient::GetInstance().HasPinProtect(userId)) {
        LOGE("User %{public}d has pin code protect.", userId);
        return E_ERR;
    }
    return E_OK;
}

int KeyManager::TryToFixUserCeEceSeceKey(unsigned int userId,
                                         KeyType keyType,
                                         const std::vector<uint8_t> &token,
                                         const std::vector<uint8_t> &secret)
{
    LOGI("enter TryToFixUserCeEceSeceKey");
    if (!IamClient::GetInstance().HasPinProtect(userId)) {
        LOGE("User %{public}d has no pin code protect.", userId);
        return E_OK;
    }

    uint64_t secureUid = { 0 };
    if (!secret.empty() && !token.empty()) {
        IamClient::GetInstance().GetSecureUid(userId, secureUid);
        LOGE("Pin code is exist, get secure uid.");
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
        LOGE("try to fix elx key failed !");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, keyType);
    if (ret != E_OK) {
        LOGE("try to fix elx key context failed !");
        StorageRadar::ReportUpdateUserAuth("TryToFixUserCeEceSeceKey::UpdateCeEceSeceKeyContext",
            userId, ret, std::to_string(keyType), "");
        return ret;
    }
    return E_OK;
}

int KeyManager::TryToFixUeceKey(unsigned int userId,
                                const std::vector<uint8_t> &token,
                                const std::vector<uint8_t> &secret)
{
    LOGI("enter TryToFixUeceKey");
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
    LOGI("start, user is %{public}u , type is %{public}d", userId, type);
    std::string dir = GetKeyDirByUserAndType(userId, type);
    if (dir == "") {
        LOGE("type is invalid, %{public}u", type);
        return E_PARAMS_INVALID;
    }

    if (!IsDir(dir)) {
        LOGE("dir not exist");
        return -ENOENT;
    }
    int32_t ret = RestoreUserKey(userId, dir, NULL_KEY_AUTH, type);
    if (ret == 0 && type != EL1_KEY) {
        saveLockScreenStatus[userId] = true;
        LOGI("User is %{public}u , saveLockScreenStatus is %{public}d", userId, saveLockScreenStatus[userId]);
    }
    return ret;
}
#endif

#ifdef EL5_FILEKEY_MANAGER
int KeyManager::RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    std::lock_guard<std::mutex> lock(ueceMutex_);
    if (ueceCallback == nullptr) {
        LOGE("callback is nullptr");
        return E_PARAMS_INVALID;
    }
    if (ueceCallback_ != nullptr) {
        LOGI("El5FileMgr already registered callback, renew");
        ueceCallback_ = ueceCallback;
        return E_OK;
    }
    ueceCallback_ = ueceCallback;
    LOGI("El5FileMgr register callback");
    return E_OK;
}

int KeyManager::UnregisterUeceActivationCallback()
{
    std::lock_guard<std::mutex> lock(ueceMutex_);
    if (ueceCallback_ == nullptr) {
        LOGI("El5FileMgr already unregistered callback");
        return E_OK;
    }
    ueceCallback_ = nullptr;
    LOGI("Unregister callback");
    return E_OK;
}
#endif

int KeyManager::NotifyUeceActivation(uint32_t userId, int32_t resultCode, bool needGetAllAppKey)
{
#ifdef EL5_FILEKEY_MANAGER
    if (ueceCallback_ == nullptr) {
        LOGE("el5 activation callback invalid");
        return E_OK;
    }
    resultCode = (resultCode == E_ACTIVE_REPEATED ? E_OK : resultCode);
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    std::thread callbackThread ([this, userId, resultCode, needGetAllAppKey, p = std::move(promise)]() mutable {
        int32_t retValue = E_OK;
        LOGI("ready for callback, El5 activation result = %{public}d, userId=%{public}u, needGetAllAppKey=%{public}d",
            resultCode, userId, needGetAllAppKey);
        ueceCallback_->OnEl5Activation(resultCode, userId, needGetAllAppKey, retValue);
        p.set_value(retValue);
    });

    if (future.wait_for(std::chrono::milliseconds(WAIT_THREAD_TIMEOUT_MS)) == std::future_status::timeout) {
        LOGE("el5 activation callback timed out");
        callbackThread.detach();
        std::ostringstream extraData;
        extraData << "Notify EL5 timeout, needGetAllAppKey: " << needGetAllAppKey << " resultCode: " << resultCode;
        StorageRadar::ReportUpdateUserAuth("NotifyUeceActivation", userId, E_TASK_TIME_OUT, "EL5", extraData.str());
        return E_OK;
    }

    auto delay = StorageService::StorageRadar::ReportDuration("UNLOCK USER APP KEYS",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: UNLOCK USER APP KEYS CB: delay time = %{public}s, isAllAppKey=%{public}d",
        delay.c_str(), needGetAllAppKey);

    int32_t ret = future.get();
    LOGI("Unlock App Keys ret= %{public}d", ret);
    callbackThread.join();
    if (resultCode != E_OK || ret == E_PARAMS_INVALID) {
        return E_OK;
    }
    return ret;
# else
    LOGD("EL5_FILEKEY_MANAGER is not supported");
    return E_OK;
#endif
}
} // namespace StorageDaemon
} // namespace OHOS
