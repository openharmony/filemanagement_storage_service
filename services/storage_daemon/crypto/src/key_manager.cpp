/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

#include <filesystem>
#include <string>

#include "base_key.h"
#include "common_timer_errors.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "fscrypt_key_v1.h"
#include "fscrypt_key_v2.h"
#include "libfscrypt/fscrypt_control.h"
#include "libfscrypt/key_control.h"
#include "parameter.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
const UserAuth NULL_KEY_AUTH = {};
const std::string DEFAULT_NEED_RESTORE_VERSION = "1";

std::shared_ptr<BaseKey> KeyManager::GetBaseKey(const std::string& dir)
{
    uint8_t versionFromPolicy = GetFscryptVersionFromPolicy();
    uint8_t kernelSupportVersion = KeyCtrlGetFscryptVersion(MNT_DATA.c_str());
    if (kernelSupportVersion == FSCRYPT_INVALID) {
        LOGE("kernel not support fscrypt");
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
    LOGI("enter");
    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ == nullptr) {
        return -EOPNOTSUPP;
    }

    if (globalEl1Key_->InitKey() == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (globalEl1Key_->StoreKey(NULL_KEY_AUTH) == false) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key store failed");
        return -EFAULT;
    }

    if (globalEl1Key_->ActiveKey(FIRST_CREATE_KEY) == false) {
        globalEl1Key_->ClearKey();
        globalEl1Key_ = nullptr;
        LOGE("global security key active failed");
        return -EFAULT;
    }

    (void)globalEl1Key_->UpdateKey();
    hasGlobalDeviceKey_ = true;
    LOGI("key create success");
    return 0;
}

int KeyManager::RestoreDeviceKey(const std::string &dir)
{
    LOGI("enter");
    if (globalEl1Key_ != nullptr) {
        LOGD("device key has existed");
        return 0;
    }

    globalEl1Key_ = GetBaseKey(dir);
    if (globalEl1Key_ == nullptr) {
        return -EOPNOTSUPP;
    }

    if (globalEl1Key_->InitKey() == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key init failed");
        return -EFAULT;
    }

    if (globalEl1Key_->RestoreKey(NULL_KEY_AUTH) == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key restore failed");
        return -EFAULT;
    }

    if (globalEl1Key_->ActiveKey(RETRIEVE_KEY) == false) {
        globalEl1Key_ = nullptr;
        LOGE("global security key active failed");
        return -EFAULT;
    }
    hasGlobalDeviceKey_ = true;
    LOGI("key restore success");

    return 0;
}

int KeyManager::InitGlobalDeviceKey(void)
{
    LOGI("enter");
    int ret = InitFscryptPolicy();
    if (ret < 0) {
        LOGE("fscrypt init failed, fscrypt will not be enabled");
        return ret;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    if (hasGlobalDeviceKey_ || globalEl1Key_ != nullptr) {
        LOGD("glabal device el1 have existed");
        return 0;
    }

    ret = MkDir(STORAGE_DAEMON_DIR, S_IRWXU); // para.0700: root only
    if (ret && errno != EEXIST) {
        LOGE("create storage daemon dir error");
        return ret;
    }
    ret = MkDir(DEVICE_EL1_DIR, S_IRWXU);
    if (ret) {
        if (errno != EEXIST) {
            LOGE("make device el1 dir error");
            return ret;
        }
        UpgradeKeys({{0, DEVICE_EL1_DIR}});
        return RestoreDeviceKey(DEVICE_EL1_DIR);
    }

    return GenerateAndInstallDeviceKey(DEVICE_EL1_DIR);
}

int KeyManager::GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGI("enter");
    if (HasElkey(userId, type)) {
        LOGD("The user %{public}u el %{public}u have existed", userId, type);
        return 0;
    }

    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        return -EOPNOTSUPP;
    }

    if (elKey->InitKey() == false) {
        LOGE("user security key init failed");
        return -EFAULT;
    }

    if (elKey->StoreKey(auth) == false) {
        elKey->ClearKey();
        LOGE("user security key store failed");
        return -EFAULT;
    }

    if (elKey->ActiveKey(FIRST_CREATE_KEY) == false) {
        elKey->ClearKey();
        LOGE("user security key active failed");
        return -EFAULT;
    }

    (void)elKey->UpdateKey();
    if (type == EL1_KEY) {
        userEl1Key_[userId] = elKey;
    } else if (type == EL2_KEY) {
        userEl2Key_[userId] = elKey;
    } else if (type == EL3_KEY) {
        userEl3Key_[userId] = elKey;
    } else if (type == EL4_KEY) {
        userEl4Key_[userId] = elKey;
    }
    LOGI("key create success");

    return 0;
}

int KeyManager::RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type)
{
    LOGI("enter");
    if (HasElkey(userId, type)) {
        LOGD("The user %{public}u el %{public}u have existed", userId, type);
        return 0;
    }

    auto elKey = GetBaseKey(dir);
    if (elKey == nullptr) {
        return -EOPNOTSUPP;
    }

    if (elKey->InitKey() == false) {
        LOGE("user security key init failed");
        return -EFAULT;
    }

    if (elKey->RestoreKey(auth) == false) {
        LOGE("user security key restore failed");
        return -EFAULT;
    }

    if (elKey->ActiveKey(RETRIEVE_KEY) == false) {
        LOGE("user security key active failed");
        return -EFAULT;
    }

    if (type == EL1_KEY) {
        userEl1Key_[userId] = elKey;
    } else if (type == EL2_KEY) {
        userEl2Key_[userId] = elKey;
    } else if (type == EL3_KEY) {
        userEl3Key_[userId] = elKey;
    } else if (type == EL4_KEY) {
        userEl4Key_[userId] = elKey;
    }
    LOGI("key restore success");

    return 0;
}

bool KeyManager::HasElkey(uint32_t userId, KeyType type)
{
    LOGI("enter");
    if (type == EL1_KEY) {
        if (userEl1Key_.find(userId) != userEl1Key_.end()) {
            LOGD("user el1 key has existed");
            return true;
        }
    } else if (type == EL2_KEY) {
        if (userEl2Key_.find(userId) != userEl2Key_.end()) {
            LOGD("user el2 key has existed");
            return true;
        }
    } else if (type == EL3_KEY) {
        if (userEl3Key_.find(userId) != userEl3Key_.end()) {
            LOGD("user el3 key has existed");
            return true;
        }
    } else if (type == EL4_KEY) {
        if (userEl4Key_.find(userId) != userEl4Key_.end()) {
            LOGD("user el4 key has existed");
            return true;
        }
    } else {
        LOGE("key type error");
    }

    return false;
}

bool KeyManager::IsNeedClearKeyFile(std::string file)
{
    LOGI("enter:");
    if (!std::filesystem::exists(file)) {
        LOGE("file not exist, file is %{private}s", file.c_str());
        return false;
    }

    std::string version;
    if (!OHOS::LoadStringFromFile(file, version)) {
        LOGE("LoadStringFromFile return fail, file is %{private}s", file.c_str());
        return false;
    }

    if (version != DEFAULT_NEED_RESTORE_VERSION) {
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
    std::vector<FileList> dirInfo;
    ReadDigitDir(USER_EL2_DIR, dirInfo);
    UpgradeKeys(dirInfo);
    dirInfo.clear();
    ReadDigitDir(USER_EL1_DIR, dirInfo);
    UpgradeKeys(dirInfo);
    for (auto item : dirInfo) {
        if (RestoreUserKey(item.userId, item.path, NULL_KEY_AUTH, EL1_KEY) != 0) {
            LOGE("user %{public}u el1 key restore error", item.userId);
        }
    }

    /* only for el3/el4 upgrade scene */
    dirInfo.clear();
    ReadDigitDir(USER_EL3_DIR, dirInfo);
    ProcUpgradeKey(dirInfo);
    dirInfo.clear();
    ReadDigitDir(USER_EL4_DIR, dirInfo);
    ProcUpgradeKey(dirInfo);
    return 0;
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

    return 0;
}

int KeyManager::InitGlobalUserKeys(void)
{
    LOGI("enter");
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = InitUserElkeyStorageDir();
    if (ret) {
        LOGE("Init user el storage dir failed");
        return ret;
    }

    std::string globalUserEl1Path = USER_EL1_DIR + "/" + std::to_string(GLOBAL_USER_ID);
    if (IsDir(globalUserEl1Path)) {
        ret = RestoreUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("Restore el1 failed");
            return ret;
        }
    } else {
        ret = GenerateAndInstallUserKey(GLOBAL_USER_ID, globalUserEl1Path, NULL_KEY_AUTH, EL1_KEY);
        if (ret != 0) {
            LOGE("Generate el1 failed");
            return ret;
        }
    }

    ret = LoadAllUsersEl1Key();
    if (ret) {
        LOGE("Load all users el1 failed");
        return ret;
    }
    LOGI("Init global user key success");

    return 0;
}

int KeyManager::GenerateUserKeys(unsigned int user, uint32_t flags)
{
    LOGI("start, user:%{public}u", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    if ((!IsDir(USER_EL1_DIR)) || (!IsDir(USER_EL2_DIR)) || (!IsDir(USER_EL3_DIR)) || (!IsDir(USER_EL4_DIR))) {
        LOGD("El storage dir is not existed");
        return -ENOENT;
    }

    std::string el1Path = USER_EL1_DIR + "/" + std::to_string(user);
    std::string el2Path = USER_EL2_DIR + "/" + std::to_string(user);
    std::string el3Path = USER_EL3_DIR + "/" + std::to_string(user);
    std::string el4Path = USER_EL4_DIR + "/" + std::to_string(user);
    if (IsDir(el1Path) || IsDir(el2Path) || IsDir(el3Path) || IsDir(el4Path)) {
        LOGE("user %{public}d el key have existed, create error", user);
        return -EEXIST;
    }
    int ret = GenerateAndInstallUserKey(user, el1Path, NULL_KEY_AUTH, EL1_KEY);
    if (ret) {
        LOGE("user el1 create error");
        return ret;
    }

    ret = GenerateAndInstallUserKey(user, el2Path, NULL_KEY_AUTH, EL2_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el2 create error");
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el3Path, NULL_KEY_AUTH, EL3_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el3 create error");
        return ret;
    }
    ret = GenerateAndInstallUserKey(user, el4Path, NULL_KEY_AUTH, EL4_KEY);
    if (ret) {
        DoDeleteUserKeys(user);
        LOGE("user el4 create error");
        return ret;
    }
    LOGI("Create user el success");

    return 0;
}

int KeyManager::GenerateUserKeyByType(unsigned int user, KeyType type,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &secret)
{
    LOGI("start, user:%{public}u, type %{public}u", user, type);
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
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
    UserAuth auth = {.token = token, .secret = secret, .secureUid = 0};
    int ret = GenerateAndInstallUserKey(user, elUserKeyPath, auth, type);
    if (ret) {
        LOGE("user el create error, user %{public}u, type %{public}u", user, type);
        return ret;
    }
    LOGI("Create user el success, user %{public}u, type %{public}u", user, type);

    return 0;
}

int KeyManager::DoDeleteUserCeEceSeceKeys(unsigned int user,
                                          const std::string USER_DIR,
                                          std::map<unsigned int, std::shared_ptr<BaseKey>> &userElKey_)
{
    int ret = 0;
    std::string elPath;
    auto it = userElKey_.find(user);
    if (it != userElKey_.end()) {
        auto elKey = it->second;
        elKey->ClearKey();
        userElKey_.erase(user);
        saveLockScreenStatus.erase(user);
    } else {
        elPath = USER_DIR + "/" + std::to_string(user);
        std::shared_ptr<BaseKey> elKey = GetBaseKey(elPath);
        if (elKey == nullptr) {
            LOGE("Malloc el1 Basekey memory failed");
            return -ENOMEM;
        }
        if (!elKey->ClearKey()) {
            LOGE("Delete el1 key failed");
            ret = -EFAULT;
        }
    }
    return ret;
}

int KeyManager::DoDeleteUserKeys(unsigned int user)
{
    int ret = 0;
    ret = DoDeleteUserCeEceSeceKeys(user, USER_EL1_DIR, userEl1Key_);
    if (ret != 0) {
        LOGE("Delete el1 key failed");
        ret = -EFAULT;
    }
    ret = DoDeleteUserCeEceSeceKeys(user, USER_EL2_DIR, userEl2Key_);
    if (ret != 0) {
        LOGE("Delete el2 key failed");
        ret = -EFAULT;
    }
    ret = DoDeleteUserCeEceSeceKeys(user, USER_EL3_DIR, userEl3Key_);
    if (ret != 0) {
        LOGE("Delete el3 key failed");
        ret = -EFAULT;
    }
    ret = DoDeleteUserCeEceSeceKeys(user, USER_EL4_DIR, userEl4Key_);
    if (ret != 0) {
        LOGE("Delete el4 key failed");
        ret = -EFAULT;
    }
    return ret;
}

int KeyManager::DeleteUserKeys(unsigned int user)
{
    LOGI("start, user:%{public}d", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoDeleteUserKeys(user);
    LOGI("delete user key end");

    return ret;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
                               bool needGenerateShield)
#else
int KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret)
#endif
{
#ifdef USER_CRYPTO_MIGRATE_KEY
    int ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL2_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el2 key fail", user);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL3_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el3 key fail", user);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL4_KEY, needGenerateShield);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el4 key fail", user);
        return ret;
    }
#else
    int ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL2_KEY);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el2 key fail", user);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL3_KEY);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el3 key fail", user);
        return ret;
    }
    ret = UpdateCeEceSeceUserAuth(user, userTokenSecret, EL4_KEY);
    if (ret != 0) {
        LOGE("user %{public}u UpdateUserAuth el4 key fail", user);
        return ret;
    }
#endif

    return ret;
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
    LOGI("start, user:%{public}d", user);
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::shared_ptr<BaseKey> item = GetUserElKey(user, type);
    if (item == nullptr) {
        LOGE("Have not found user %{public}u el key", user);
        return -ENOENT;
    }

    UserAuth auth = {userTokenSecret.token, userTokenSecret.oldSecret, userTokenSecret.secureUid};
    if ((item->RestoreKey(auth) == false) && (item->RestoreKey(NULL_KEY_AUTH) == false)) {
        LOGE("Restore key error");
        return -EFAULT;
    }

    auth.secret = userTokenSecret.newSecret;
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (item->StoreKey(auth, needGenerateShield) == false) {
#else
    if (item->StoreKey(auth) == false) {
#endif
        LOGE("Store key error");
        return -EFAULT;
    }

    userPinProtect[user] = !userTokenSecret.newSecret.empty();
    return 0;
}

int KeyManager::ActiveUserKey(unsigned int user, const std::vector<uint8_t> &token,
                              const std::vector<uint8_t> &secret)
{
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }

    if (ActiveCeSceSeceUserKey(user, EL2_KEY, token, secret) != 0) {
        LOGI("Active user %{public}u el2 fail", user);
        return -EFAULT;
    }
    if (ActiveCeSceSeceUserKey(user, EL3_KEY, token, secret) != 0) {
        LOGI("Active user %{public}u el3 fail", user);
        return -EFAULT;
    }
    if (ActiveCeSceSeceUserKey(user, EL4_KEY, token, secret) != 0) {
        LOGI("Active user %{public}u el4 fail", user);
        return -EFAULT;
    }
    return 0;
}

std::string KeyManager::GetKeyDirByUserAndType(unsigned int user, KeyType type)
{
    std::string keyDir = "";
    switch (type) {
        case EL1_KEY:
            keyDir = USER_EL1_DIR + "/" + std::to_string(user);
            break;
        case EL2_KEY:
            keyDir = USER_EL2_DIR + "/" + std::to_string(user);
            break;
        case EL3_KEY:
            keyDir = USER_EL3_DIR + "/" + std::to_string(user);
            break;
        case EL4_KEY:
            keyDir = USER_EL4_DIR + "/" + std::to_string(user);
            break;
        default:
            LOGE("GetKeyDirByUserAndType type %{public}u is invalid", type);
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
        default:
            LOGE("GetKeyDirByType type %{public}u is invalid", type);
            break;
    }
    return keyDir;
}

void KeyManager::SaveUserElKey(unsigned int user, KeyType type, std::shared_ptr<BaseKey> elKey)
{
    switch (type) {
        case EL1_KEY:
            userEl1Key_[user] = elKey;
            break;
        case EL2_KEY:
            userEl2Key_[user] = elKey;
            break;
        case EL3_KEY:
            userEl3Key_[user] = elKey;
            break;
        case EL4_KEY:
            userEl4Key_[user] = elKey;
            break;
        default:
            LOGE("SaveUserElKey type %{public}u is invalid", type);
    }
}

std::shared_ptr<BaseKey> KeyManager::GetUserElKey(unsigned int user, KeyType type)
{
    if (HasElkey(user, type) != true) {
        LOGE("Have not found user %{public}u key, type %{public}u", user, type);
        return nullptr;
    }

    switch (type) {
        case EL1_KEY:
            return userEl1Key_[user];
        case EL2_KEY:
            return userEl2Key_[user];
        case EL3_KEY:
            return userEl3Key_[user];
        case EL4_KEY:
            return userEl4Key_[user];
        default:
            LOGE("GetUserElKey type %{public}u is invalid", type);
            return nullptr;
    }
}

int KeyManager::ActiveCeSceSeceUserKey(unsigned int user, KeyType type,
                                       const std::vector<uint8_t> &token,
                                       const std::vector<uint8_t> &secret)
{
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (HasElkey(user, type)) {
        LOGE("The user %{public}u el have been actived, key type is %{public}u", user, type);
        return 0;
    }
    std::string keyDir = GetKeyDirByUserAndType(user, type);
    if (keyDir == "") {
        return E_KEY_TYPE_INVAL;
    }
    if (!IsDir(keyDir)) {
        LOGE("Have not found user %{public}u el", user);
        return -ENOENT;
    }

    std::shared_ptr<BaseKey> elKey = GetBaseKey(keyDir);
    if (elKey == nullptr) {
        LOGE("elKey failed");
        return -EOPNOTSUPP;
    }
    if (elKey->InitKey() == false) {
        LOGE("Init el failed");
        return -EFAULT;
    }
    UserAuth auth = {token, secret};
    if ((elKey->RestoreKey(auth) == false) && (elKey->RestoreKey(NULL_KEY_AUTH) == false)) {
        LOGE("Restore el failed");
        return -EFAULT;
    }
    std::string NEED_UPDATE_PATH = keyDir + PATH_LATEST + SUFFIX_NEED_UPDATE;
    if (!FileExists(NEED_UPDATE_PATH) && (elKey->StoreKey(auth) == false)) {
        LOGE("Store el failed");
        return -EFAULT;
    }
    if (elKey->ActiveKey(RETRIEVE_KEY) == false) {
        LOGE("Active user %{public}u key failed", user);
        return -EFAULT;
    }

    SaveUserElKey(user, type, elKey);
    if (secret.empty()) {
        userPinProtect.insert(std::make_pair(user, false));
    } else {
        userPinProtect.insert(std::make_pair(user, true));
    }
    LOGI("Active user %{public}u el success", user);
    saveLockScreenStatus.insert(std::make_pair(user, true));
    return 0;
}

int KeyManager::UnlockUserScreen(uint32_t user)
{
    LOGI("start");
    auto iter = saveLockScreenStatus.find(user);
    if (iter == saveLockScreenStatus.end()) {
        saveLockScreenStatus.insert(std::make_pair(user, false));
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        saveLockScreenStatus[user] = true;
        LOGI("saveLockScreenStatus is %{public}d", saveLockScreenStatus[user]);
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (userEl4Key_.find(user) == userEl4Key_.end()) {
        saveLockScreenStatus[user] = true;
        LOGE("The user %{public}u not been actived and saveLockScreenStatus is %{public}d", user,
             saveLockScreenStatus[user]);
        return 0;
    }
    auto el4Key = userEl4Key_[user];
    if (!el4Key->UnlockUserScreen(user, FSCRYPT_SDP_ECE_CLASS)) {
        LOGE("UnlockUserScreen user %{public}u el4 key failed", user);
        return -EFAULT;
    }
    saveLockScreenStatus[user] = true;
    LOGI("UnlockUserScreen user %{public}u el3 and el4 success and saveLockScreenStatus is %{public}d", user,
         saveLockScreenStatus[user]);
    return 0;
}

int KeyManager::GetLockScreenStatus(uint32_t user, bool &lockScreenStatus)
{
    LOGI("start");
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto iter = saveLockScreenStatus.find(user);
    lockScreenStatus = (iter == saveLockScreenStatus.end()) ? false: iter->second;
    LOGI("lockScreenStatus is %{public}d", lockScreenStatus);
    return 0;
}

int KeyManager::GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId)
{
    std::lock_guard<std::mutex> lock(keyMutex_);
    LOGE("Generate appkey2 success");
    return 0;
}

int KeyManager::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    std::lock_guard<std::mutex> lock(keyMutex_);
    LOGE("Delete appkey2 success");
    return 0;
}

int KeyManager::InActiveUserKey(unsigned int user)
{
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (userEl2Key_.find(user) == userEl2Key_.end()) {
        LOGE("Have not found user %{public}u el2", user);
        return -ENOENT;
    }
    auto elKey = userEl2Key_[user];
    if (elKey->InactiveKey(USER_LOGOUT) == false) {
        LOGE("Clear user %{public}u key failed", user);
        return -EFAULT;
    }
    userEl2Key_.erase(user);
    LOGI("Inactive user %{public}u el2 success", user);

    if (userEl3Key_.find(user) == userEl3Key_.end()) {
        LOGE("Have not found user %{public}u el3", user);
        return -ENOENT;
    }
    elKey = userEl3Key_[user];
    if (elKey->InactiveKey(USER_LOGOUT) == false) {
        LOGE("Clear user %{public}u key failed", user);
        return -EFAULT;
    }
    userEl3Key_.erase(user);
    LOGI("Inactive user %{public}u el3 success", user);

    if (userEl4Key_.find(user) == userEl4Key_.end()) {
        LOGE("Have not found user %{public}u el4", user);
        return -ENOENT;
    }
    elKey = userEl4Key_[user];
    if (elKey->InactiveKey(USER_LOGOUT) == false) {
        LOGE("Clear user %{public}u key failed", user);
        return -EFAULT;
    }
    userEl4Key_.erase(user);
    LOGI("Inactive user %{public}u elX success", user);

    return 0;
}

int KeyManager::LockUserScreen(uint32_t user)
{
    LOGD("start");
    std::lock_guard<std::mutex> lock(keyMutex_);
    auto iter = userPinProtect.find(user);
    if (iter == userPinProtect.end() || iter->second == false) {
        LOGI("saveLockScreenStatus is %{public}d", saveLockScreenStatus[user]);
        return 0;
    }
    iter = saveLockScreenStatus.find(user);
    if (iter == saveLockScreenStatus.end()) {
        saveLockScreenStatus.insert(std::make_pair(user, false));
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        saveLockScreenStatus[user] = false;
        LOGI("KeyCtrlHasFscryptSyspara is false, saveLockScreenStatus is %{public}d",
            saveLockScreenStatus[user]);
        return 0;
    }
    if (userEl4Key_.find(user) == userEl4Key_.end()) {
        LOGE("Have not found user %{public}u el3 or el4", user);
        return -ENOENT;
    }
    auto elKey = userEl4Key_[user];
    if (!elKey->LockUserScreen(user, FSCRYPT_SDP_ECE_CLASS)) {
        LOGE("Clear user %{public}u key failed", user);
        return -EFAULT;
    }

    saveLockScreenStatus[user] = false;
    LOGI("LockUserScreen user %{public}u el3 and el4 success, saveLockScreenStatus is %{public}d",
        user, saveLockScreenStatus[user]);
    return 0;
}

int KeyManager::SetDirectoryElPolicy(unsigned int user, KeyType type, const std::vector<FileList> &vec)
{
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }
    std::string keyPath;
    std::string eceSeceKeyPath;
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (type == EL1_KEY) {
        if (userEl1Key_.find(user) == userEl1Key_.end()) {
            LOGE("Have not found user %{public}u el1 key, not enable el1", user);
            return -ENOENT;
        }
        keyPath = userEl1Key_[user]->GetDir();
    } else if (type == EL2_KEY || type == EL3_KEY || type == EL4_KEY) {
        if (userEl2Key_.find(user) == userEl2Key_.end()) {
            LOGE("Have not found user %{public}u el2 key, not enable el2", user);
            return -ENOENT;
        }
        keyPath = userEl2Key_[user]->GetDir();
    } else {
        LOGE("Not specify el flags, no need to crypt");
        return 0;
    }
    if (getEceSeceKeyPath(user, type, eceSeceKeyPath) != 0) {
        LOGE("method getEceSeceKeyPath fail");
        return -ENOENT;
    }
    for (auto item : vec) {
        int ret = LoadAndSetPolicy(keyPath.c_str(), item.path.c_str());
        if (ret != 0) {
            LOGE("Set directory el policy error, ret: %{public}d", ret);
            return -EFAULT;
        }
    }
    if (type == EL3_KEY || type == EL4_KEY) {
        for (auto item : vec) {
            if (LoadAndSetEceAndSecePolicy(eceSeceKeyPath.c_str(), item.path.c_str(), static_cast<int>(type)) != 0) {
                LOGE("Set directory el policy error!");
                return -EFAULT;
            }
        }
    }
    LOGI("Set user %{public}u el policy success", user);
    return 0;
}

int KeyManager::getEceSeceKeyPath(unsigned int user, KeyType type, std::string &eceSeceKeyPath)
{
    if (type == EL3_KEY) {
        if (userEl3Key_.find(user) == userEl3Key_.end()) {
            LOGD("Have not found user %{public}u el3 key, not enable el3", user);
            return -ENOENT;
        }
        eceSeceKeyPath = userEl3Key_[user]->GetDir();
    }
    if (type == EL4_KEY) {
        if (userEl4Key_.find(user) == userEl4Key_.end()) {
            LOGD("Have not found user %{public}u el4 key, not enable el4", user);
            return -ENOENT;
        }
        eceSeceKeyPath = userEl4Key_[user]->GetDir();
    }
    return 0;
}

int KeyManager::UpdateCeEceSeceKeyContext(uint32_t userId, KeyType type)
{
    LOGI("start");
    if (!KeyCtrlHasFscryptSyspara()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    if (HasElkey(userId, type) == false) {
        LOGE("Have not found user %{public}u el%{public}u", userId, type);
        return -ENOENT;
    }
    std::shared_ptr<BaseKey> elKey = GetUserElKey(userId, type);
    if (elKey == nullptr) {
        LOGE("Have not found user %{public}u, type el%{public}u", userId, type);
        return -ENOENT;
    }
    if (!elKey->UpdateKey()) {
        LOGE("Basekey update newest context failed");
        return -EFAULT;
    }
    return 0;
}

int KeyManager::UpdateKeyContext(uint32_t userId)
{
    LOGI("UpdateKeyContext enter");
    int ret = UpdateCeEceSeceKeyContext(userId, EL2_KEY);
    if (ret != 0) {
        LOGE("Basekey update EL2 newest context failed");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL3_KEY);
    if (ret != 0) {
        LOGE("Basekey update EL3 newest context failed");
        return ret;
    }
    ret = UpdateCeEceSeceKeyContext(userId, EL4_KEY);
    if (ret != 0) {
        LOGE("Basekey update EL4 newest context failed");
        return ret;
    }
    LOGI("Basekey update key context success");
    return 0;
}

int KeyManager::UpgradeKeys(const std::vector<FileList> &dirInfo)
{
    for (const auto &it : dirInfo) {
        std::shared_ptr<BaseKey> elKey = GetBaseKey(it.path);
        if (elKey == nullptr) {
            LOGE("elKel feiled");
            continue;
        }
        elKey->UpgradeKeys();
    }
    return 0;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int KeyManager::RestoreUserKey(uint32_t userId, KeyType type)
{
    LOGI("start, user is %{public}u , type is %{public}d", userId, type);
    std::string dir = GetKeyDirByUserAndType(userId, type);
    if (dir == "") {
        LOGE("type is invalid, %{public}u", type);
        return -EFAULT;
    }

    if (!IsDir(dir)) {
        LOGE("dir not exist");
        return -ENOENT;
    }
    return RestoreUserKey(userId, dir, NULL_KEY_AUTH, type);
}
#endif
} // namespace StorageDaemon
} // namespace OHOS
