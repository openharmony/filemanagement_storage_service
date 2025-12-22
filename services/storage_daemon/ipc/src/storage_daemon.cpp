/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc/storage_daemon.h"
#include "file_ex.h"
#include "utils/memory_reclaim_manager.h"
#include "utils/storage_radar.h"
#include "utils/storage_xcollie.h"
#include "utils/string_utils.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <thread>
#ifdef USER_CRYPTO_MANAGER
#include "crypto/app_clone_key_manager.h"
#include "crypto/iam_client.h"
#include "crypto/key_crypto_utils.h"
#include "crypto/key_manager.h"
#include "crypto/key_manager_ext.h"
#endif
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager.h"
#include "volume/volume_manager.h"
#endif
#include "file_share.h"
#include "file_sharing/file_sharing.h"
#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "system_ability_definition.h"
#include "user/user_manager.h"
#ifdef DFS_SERVICE
#include "cloud_daemon_manager.h"
#endif
#ifdef USER_CRYPTO_MIGRATE_KEY
#include "string_ex.h"
#include <filesystem>
#endif
#ifdef USE_LIBRESTORECON
#include "policycoreutils.h"
#endif

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
#ifdef DFS_SERVICE
using namespace OHOS::FileManagement::CloudFile;
#endif

#ifdef USE_LIBRESTORECON
constexpr const char *DATA_SERVICE_EL2 = "/data/service/el2/";
constexpr const char *DATA_SERVICE_EL1_PUBLIC_STORAGE_DAEMON_SD = "/data/service/el1/public/storage_daemon/sd";
constexpr const char *DATA_SERVICE_EL0_STORAGE_DAEMON_SD = "/data/service/el0/storage_daemon/sd";
#elif defined(USER_CRYPTO_MIGRATE_KEY)
constexpr const char *DATA_SERVICE_EL0_STORAGE_DAEMON_SD = "/data/service/el0/storage_daemon/sd";
#endif

#ifdef USER_CRYPTO_MIGRATE_KEY
constexpr const char *NEED_RESTORE_SUFFIX = "/latest/need_restore";
constexpr const char *NEW_DOUBLE_2_SINGLE = "2";
#endif

typedef int32_t (*CreateShareFileFunc)(const std::vector<std::string> &, uint32_t, uint32_t, std::vector<int32_t> &);
typedef int32_t (*DeleteShareFileFunc)(uint32_t, const std::vector<std::string> &);

int32_t StorageDaemon::GetCryptoFlag(KeyType type, uint32_t &flags)
{
    switch (type) {
        case EL1_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1;
            return E_OK;
        case EL2_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
            return E_OK;
        case EL3_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL3;
            return E_OK;
        case EL4_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL4;
            return E_OK;
        case EL5_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL5;
            return E_OK;
        default:
            LOGE("GetCryptoFlag error, type = %{public}u", type);
            return E_KEY_TYPE_INVALID;
    }
}

#ifdef USER_CRYPTO_MIGRATE_KEY
std::string StorageDaemon::GetNeedRestoreFilePath(int32_t userId, const std::string &user_dir)
{
    std::string path = user_dir + "/" + std::to_string(userId) + "/latest/need_restore";
    return path;
}

std::string StorageDaemon::GetNeedRestoreFilePathByType(int32_t userId, KeyType type)
{
    switch (type) {
        case EL1_KEY:
            return GetNeedRestoreFilePath(userId, USER_EL1_DIR);
        case EL2_KEY:
            return GetNeedRestoreFilePath(userId, USER_EL2_DIR);
        case EL3_KEY:
            return GetNeedRestoreFilePath(userId, USER_EL3_DIR);
        case EL4_KEY:
            return GetNeedRestoreFilePath(userId, USER_EL4_DIR);
        case EL5_KEY:
            return GetNeedRestoreFilePath(userId, USER_EL5_DIR);
        default:
            LOGE("GetNeedRestoreFilePathByType key type error, type = %{public}u", type);
            return "";
    }
}

int32_t StorageDaemon::RestoreOneUserKey(int32_t userId, KeyType type)
{
    uint32_t flags = 0;
    int32_t ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }

    std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
    if (elNeedRestorePath.empty()) {
        LOGI("elNeedRestorePath is empty, type = %{public}d", type);
        return E_KEY_TYPE_INVALID;
    }

    std::error_code errCode;
    if (!std::filesystem::exists(elNeedRestorePath, errCode)) {
        return E_OK;
    }
    std::string SINGLE_RESTORE_VERSION;
    (void) OHOS::LoadStringFromFile(elNeedRestorePath, SINGLE_RESTORE_VERSION);
    LOGI("start restore User %{public}u el%{public}u, restore version = %{public}s", userId, type,
        SINGLE_RESTORE_VERSION.c_str());
    ret = KeyManager::GetInstance().RestoreUserKey(userId, type);
    if (ret != E_OK) {
        if (type != EL1_KEY) {
            LOGE("userId %{public}u type %{public}u restore key failed, but return success, error = %{public}d",
                userId, type, ret);
            return E_MIGRETE_ELX_FAILED; // maybe need user key, so return E_OK to continue
        }
        LOGE("RestoreUserKey EL1_KEY failed, error = %{public}d, userId %{public}u", ret, userId);
        return ret;
    }

    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGE("PrepareUserDirs failed, userId %{public}u, flags %{public}u, error %{public}d", userId, flags, ret);
        return ret;
    }
    if (type == EL2_KEY) {
        PrepareUeceDir(userId);
    }
    if (userId < StorageService::START_APP_CLONE_USER_ID || userId > StorageService::MAX_APP_CLONE_USER_ID) {
        if (type != EL1_KEY) {
            (void)remove(elNeedRestorePath.c_str());
        }
    }

    // for double2single update el2-4 without secret
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    LOGW("restore User %{public}u el%{public}u success", userId, type);

    return E_OK;
}

int32_t StorageDaemon::RestoreUserKey(int32_t userId, uint32_t flags)
{
    LOGI("prepare restore user dirs for %{public}d, flags %{public}u", userId, flags);
    if (!IsNeedRestorePathExist(userId, true)) {
        LOGE("need_restore file is not existed");
        return -EEXIST;
    }

    std::vector<KeyType> keyTypes = {EL1_KEY, EL2_KEY, EL3_KEY, EL4_KEY, EL5_KEY};
    for (auto type : keyTypes) {
        auto ret = RestoreOneUserKey(userId, type);
        if (ret == E_MIGRETE_ELX_FAILED) {
            LOGE("Try restore user: %{public}d type: %{public}d migrate key, wait user pin !", userId, type);
            break;
        }
        if (ret != E_OK) {
            return ret;
        }
    }
    MountManager::GetInstance().PrepareAppdataDir(userId);
    return E_OK;
}
#endif

int32_t StorageDaemon::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    // CRYPTO_FLAG_EL3 create el3,  CRYPTO_FLAG_EL4 create el4
    flags = flags | IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4 |
            IStorageDaemonEnum::CRYPTO_FLAG_EL5;
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().GenerateUserKeys(userId, flags);
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (ret == -EEXIST) {
        auto restoreRet = RestoreUserKey(userId, flags);
        if (restoreRet != E_OK) {
            std::string extraData = "flags=" + std::to_string(flags);
            StorageRadar::ReportUserManager("PrepareUserDirs::RestoreUserKey", userId, restoreRet, extraData);
        }
        return restoreRet;
    }
#endif
    if (ret != E_OK) {
        LOGE("Generate user %{public}d key error", userId);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::GenerateUserKeys", userId, ret, extraData);
        return ret;
    }
#endif
    (void)UserManager::GetInstance().DestroyUserDirs(userId, flags);
    int32_t prepareRet = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (prepareRet != E_OK) {
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::UserManager::PrepareUserDirs", userId, prepareRet, extraData);
    }
    MountManager::GetInstance().PrepareAppdataDir(userId);
#ifdef USER_CRYPTO_MANAGER
    if (prepareRet == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags);
        if (result != E_OK) {
            LOGE("KeyManagerExt GenerateUserKeys failed, error = %{public}d, userId %{public}u", result, userId);
        }
    }
#endif
    return prepareRet;
}

int32_t StorageDaemon::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    int32_t errCode = 0;
    // CRYPTO_FLAG_EL3 destroy el3,  CRYPTO_FLAG_EL4 destroy el4
    flags = flags | IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4 |
            IStorageDaemonEnum::CRYPTO_FLAG_EL5;
    int32_t destroyUserRet = UserManager::GetInstance().DestroyUserDirs(userId, flags);
    if (destroyUserRet != E_OK) {
        errCode = destroyUserRet;
        LOGW("Destroy user %{public}d dirs failed, please check", userId);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs", userId, errCode, extraData);
    }

#ifdef USER_CRYPTO_MANAGER
    destroyUserRet = KeyManager::GetInstance().DeleteUserKeys(userId);
    if (destroyUserRet != E_OK) {
        errCode = destroyUserRet;
        LOGW("DeleteUserKeys failed, please check");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs::DeleteUserKeys", userId, errCode, extraData);
    }
    if (destroyUserRet == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().DeleteUserKeys(userId);
        if (result != E_OK) {
            LOGE("KeyManagerExt DeleteUserKeys failed, error = %{public}d, userId %{public}u", result, userId);
        }
    }
    return errCode;
#else
    return errCode;
#endif
}

int32_t StorageDaemon::CompleteAddUser(int32_t userId)
{
    LOGI("CompleteAddUser enter.");
    if (userId >= StorageService::START_APP_CLONE_USER_ID && userId < StorageService::MAX_APP_CLONE_USER_ID) {
        LOGE("User %{public}d is app clone user, do not delete el1 need_restore.", userId);
        return E_OK;
    }
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, EL1_KEY);
    if (elNeedRestorePath.empty() || !std::filesystem::exists(elNeedRestorePath, errCode)) {
        return E_OK;
    }
    (void)remove(elNeedRestorePath.c_str());
    LOGI("CompleteAddUser remove el1 needRestore flag");
#endif
    return E_OK;
}

int32_t StorageDaemon::InitGlobalKey(void)
{
#ifdef USER_CRYPTO_MANAGER
    int ret = KeyManager::GetInstance().InitGlobalDeviceKey();
    if (ret != E_OK) {
        LOGE("InitGlobalDeviceKey failed, please check");
        StorageRadar::ReportUserKeyResult("InitGlobalKey::InitGlobalDeviceKey", 0, ret, "EL1", "");
    }
#ifdef USE_LIBRESTORECON
    RestoreconRecurse(DATA_SERVICE_EL0_STORAGE_DAEMON_SD);
#endif
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::InitGlobalUserKeys(void)
{
    LOGI("Daemon_InitGlobalUserKeys start.");
#ifdef USER_FILE_SHARING
    // File sharing depends on the /data/service/el1/public be decrypted.
    // A hack way to prepare the sharing dir, move it to callbacks after the parameter ready.
    if (SetupFileSharingDir() == -1) {
        LOGE("Failed to set up the directory for file sharing");
    }
#endif

#ifdef USER_CRYPTO_MANAGER

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string el1NeedRestorePath = GetNeedRestoreFilePath(START_USER_ID, USER_EL1_DIR);
    if (std::filesystem::exists(el1NeedRestorePath, errCode)) {
        LOGE("el1NeedRestorePath is exist, update NEW_DOUBLE_2_SINGLE");
        std::string doubleVersion;
        std::string el0NeedRestorePath = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
        bool isRead = OHOS::LoadStringFromFile(el0NeedRestorePath, doubleVersion);
        int newSingleVersion = std::atoi(doubleVersion.c_str()) + 1;
        LOGW("Process NEW_DOUBLE(version:%{public}s}) ——> SINGLE Frame(version:%{public}d), ret: %{public}d",
            doubleVersion.c_str(), newSingleVersion, isRead);
        if (!SaveStringToFile(el0NeedRestorePath, std::to_string(newSingleVersion))) {
            LOGE("Save NEW_DOUBLE_2_SINGLE file failed");
            return false;
        }
    }
#endif

    int ret = KeyManager::GetInstance().InitGlobalUserKeys();
    if (ret) {
        LOGE("Init global users els failed");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys", GLOBAL_USER_ID, ret, "EL1", "");
        return ret;
    }
#endif
#ifdef USE_LIBRESTORECON
    RestoreconRecurse(DATA_SERVICE_EL1_PUBLIC_STORAGE_DAEMON_SD);
#endif
    auto result = UserManager::GetInstance().PrepareAllUserEl1Dirs();
    if (result != E_OK) {
        LOGE("PrepareAllUserEl1Dirs failed, please check");
        StorageRadar::ReportUserKeyResult("PrepareAllUserEl1Dirs", GLOBAL_USER_ID, result, "EL1", "");
    }
    result = MountManager::GetInstance().PrepareAppdataDir(GLOBAL_USER_ID);
    if (result != E_OK) {
        LOGE("PrepareAppdataDir failed, please check");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::PrepareAppdataDir", GLOBAL_USER_ID, result, "EL1", "");
    }
    return result;
}

int32_t StorageDaemon::DeleteUserKeys(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().DeleteUserKeys(userId);
    if (ret != E_OK) {
        LOGE("DeleteUserKeys failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "DeleteUserKeys",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_DELETE_USER_KEYS,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    if (ret == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().DeleteUserKeys(userId);
        if (result != E_OK) {
            LOGE("KeyManagerExt DeleteUserKeys failed, error = %{public}d, userId %{public}u", result, userId);
        }
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::EraseAllUserEncryptedKeys()
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().EraseAllUserEncryptedKeys();
    LOGI("EraseAllUserEncryptedKeys result, ret: %{public}d", ret);
    StorageRadar::ReportUserKeyResult("StorageDaemon::EraseAllUserEncryptedKeys", DEFAULT_USERID, ret, "ELx", "");
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &oldSecret,
                                      const std::vector<uint8_t> &newSecret)
{
    UserTokenSecret userTokenSecret = {
        .token = token, .oldSecret = oldSecret, .newSecret = newSecret, .secureUid = secureUid};
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().UpdateUserAuth(userId, userTokenSecret);
    if (ret != E_OK) {
        LOGE("UpdateUserAuth failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = "account_mgr",
            .userId = userId,
            .funcName = "UpdateUserAuth",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_UPDATE_USER_AUTH,
            .keyElxLevel = "ELx",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                    const std::vector<uint8_t> &newSecret,
                                                    uint64_t secureUid,
                                                    uint32_t userId,
                                                    const std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("begin to UpdateUseAuthWithRecoveryKey");
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
#else
    return E_OK;
#endif
}

#ifdef USER_CRYPTO_MIGRATE_KEY
std::string StorageDaemon::GetNeedRestoreVersion(uint32_t userId, KeyType type)
{
    std::string need_restore_path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, type) + RESTORE_DIR;
    std::string need_restore_version;
    OHOS::LoadStringFromFile(need_restore_path, need_restore_version);
    return need_restore_version;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuth(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    int32_t ret = E_OK;
    std::string need_restore_version = GetNeedRestoreVersion(userId, type);
    int32_t OLD_UPDATE_VERSION_MAXLIMIT = std::atoi(NEW_DOUBLE_2_SINGLE);
    if (std::atoi(need_restore_version.c_str()) <= OLD_UPDATE_VERSION_MAXLIMIT) {
        LOGW("Old DOUBLE_2_SINGLE::PrepareUserDirsAndUpdateUserAuth.");
        ret = PrepareUserDirsAndUpdateUserAuthOld(userId, type, token, secret);
    } else {
        LOGW("New DOUBLE_2_SINGLE::PrepareUserDirsAndUpdateUserAuth.");
        ret = PrepareUserDirsAndUpdateUserAuthVx(userId, type, token, secret, need_restore_version);
    }
    // for double2single update el2-4 with secret
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    return ret;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuthOld(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGW("start userId %{public}u KeyType %{public}u", userId, type);
    int32_t ret = E_OK;
    uint32_t flags = 0;

    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }

    ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, type, token, {'!'});
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("Active user %{public}u key fail, type %{public}u, flags %{public}u", userId, type, flags);
        return ret;
    }

    uint64_t secureUid = { 0 };
    if (!IamClient::GetInstance().GetSecureUid(userId, secureUid)) {
        LOGE("Get secure uid form iam failed, use default value.");
    }
    UserTokenSecret userTokenSecret = { .token = token, .oldSecret = {'!'}, .newSecret = secret,
                                        .secureUid = secureUid };
    ret = KeyManager::GetInstance().UpdateCeEceSeceUserAuth(userId, userTokenSecret, type, false);
    if (ret != E_OK) {
        std::string isOldEmy = userTokenSecret.oldSecret.empty() ? "true" : "false";
        std::string isNewEmy = userTokenSecret.newSecret.empty() ? "true" : "false";
        std::string secretInfo = "oldSecret isEmpty = " + isOldEmy + ", newSecret isEmpty = " + isNewEmy;
        StorageRadar::ReportUpdateUserAuth("PrepareUserDirsAndUpdateUserAuth::UpdateCeEceSeceUserAuth",
            userId, ret, std::to_string(type), secretInfo);
        return ret;
    }

    ret = KeyManager::GetInstance().UpdateCeEceSeceKeyContext(userId, type);
    if (ret != E_OK) {
        StorageRadar::ReportUpdateUserAuth("PrepareUserDirsAndUpdateUserAuth::UpdateCeEceSeceKeyContext",
            userId, ret, std::to_string(type), "");
        return ret;
    }

    LOGW("try to destory dir first, user %{public}u, flags %{public}u", userId, flags);
    (void)UserManager::GetInstance().DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        return ret;
    }
    if (flags == IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    LOGW("userId %{public}u type %{public}u sucess", userId, type);
    return E_OK;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuthVx(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret,
    const std::string needRestoreVersion)
{
    LOGW("start userId %{public}u KeyType %{public}u", userId, type);
    int32_t ret = E_OK;
    uint32_t flags = 0;

    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }
    std::string need_restore_path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, type) + RESTORE_DIR;
    uint32_t new_need_restore = static_cast<uint32_t>(std::atoi(needRestoreVersion.c_str()) + 1);
    std::string errMsg = "";
    if (new_need_restore == UpdateVersion::UPDATE_V4 &&
        !SaveStringToFileSync(need_restore_path, std::to_string(new_need_restore), errMsg)) {
        LOGE("Write userId: %{public}d, El%{public}d need_restore failed.", userId, type);
        StorageRadar::ReportUpdateUserAuth("PrepareUserDirsAndUpdateUserAuthVx::SaveStringToFileSync",
            userId, E_SAVE_STRING_TO_FILE_ERR, std::to_string(type), errMsg);
    }
    LOGW("New DOUBLE_2_SINGLE::ActiveCeSceSeceUserKey.");
    ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, type, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("Active user %{public}u key fail, type %{public}u, flags %{public}u", userId, type, flags);
        return ret;
    }

    std::error_code errCode;
    std::string newVersion = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, type) + FSCRYPT_VERSION_DIR;
    if (std::filesystem::exists(newVersion, errCode)) {
        ClearNatoRestoreKey(userId, type, true);
        return E_OK;
    }

    LOGW("try to destory dir first, user %{public}u, flags %{public}u", userId, flags);
    (void)UserManager::GetInstance().DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        return ret;
    }
    if (flags == IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    LOGW("userId %{public}u type %{public}u sucess", userId, type);
    return E_OK;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateAuth4Nato(uint32_t userId,
    KeyType type, const std::vector<uint8_t> &token)
{
    LOGW("Prepare dirs and update auth for nato secen for userId=%{public}d, keyType=%{public}u", userId, type);
    std::error_code errCode;
    std::string natoRestore = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, type) + RESTORE_DIR;
    if (!std::filesystem::exists(natoRestore, errCode)) {
        LOGE("Nato restore file=%{public}s is not exist.", natoRestore.c_str());
        return E_KEY_TYPE_INVALID;
    }

    int32_t ret = KeyManager::GetInstance().ActiveElxUserKey4Nato(userId, type, token);
    if (ret != E_OK) {
        LOGE("Active user=%{public}d el=%{public}u key fot nato secen failed, ret=%{public}d.", userId, type, ret);
        return ret;
    }

    uint32_t flags = 0;
    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return E_KEY_TYPE_INVALID;
    }
    (void)UserManager::GetInstance().DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        return E_NATO_PREPARE_USER_DIR_ERROR;
    }
    if (flags == IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    LOGW("Prepare dirs and update auth for nato secen for userId=%{public}u keyType=%{public}u sucess.", userId, type);
    return E_OK;
}

bool StorageDaemon::IsNeedRestorePathExist(uint32_t userId, bool needCheckEl1)
{
    std::error_code errCode;
    std::string el2NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL2_DIR);
    std::string el3NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL3_DIR);
    std::string el4NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL4_DIR);
    bool isExist = std::filesystem::exists(el2NeedRestorePath, errCode) ||
                   std::filesystem::exists(el3NeedRestorePath, errCode) ||
                   std::filesystem::exists(el4NeedRestorePath, errCode);
    if (needCheckEl1) {
        std::string el1NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL1_DIR);
        isExist = isExist || std::filesystem::exists(el1NeedRestorePath, errCode);
    }
    return isExist;
}

int32_t StorageDaemon::PrepareUeceDir(uint32_t userId)
{
    int32_t ret = UserManager::GetInstance().DestroyUserDirs(userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5);
    LOGI("delete user %{public}u uece %{public}u, ret %{public}d", userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5, ret);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5);
    LOGI("prepare user %{public}u uece %{public}u, ret %{public}d", userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5, ret);
    return ret;
}
#endif

int32_t StorageDaemon::GenerateKeyAndPrepareUserDirs(uint32_t userId,
                                                     KeyType type,
                                                     const std::vector<uint8_t> &token,
                                                     const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret;
    uint32_t flags = 0;

    LOGI("enter:");
    ret = KeyManager::GetInstance().GenerateUserKeyByType(userId, type, token, secret);
    if (ret != E_OK) {
        LOGE("upgrade scene:generate user key fail, userId %{public}u, KeyType %{public}u, sec empty %{public}d",
             userId, type, secret.empty());
        return ret;
    }
    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }
    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(userId);
    if ((flags & IStorageDaemonEnum::CRYPTO_FLAG_EL5) && IsDir(keyUeceDir) && !std::filesystem::is_empty(keyUeceDir)) {
        LOGE("uece has already create, do not need create !");
#ifdef USER_CRYPTO_MIGRATE_KEY
        std::error_code errCode;
        std::string el0NeedRestore = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
        if (std::filesystem::exists(el0NeedRestore, errCode)) {
            UserManager::GetInstance().CreateElxBundleDataDir(userId, type);  // for double2single update el5
        }
#endif
        return ret;
    }
    (void)UserManager::GetInstance().DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGE("upgrade scene:prepare user dirs fail, userId %{public}u, flags %{public}u, sec empty %{public}d",
             userId, flags, secret.empty());
    }
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::ActiveUserKeyAndPrepare(uint32_t userId, KeyType type,
                                               const std::vector<uint8_t> &token,
                                               const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    LOGW("ActiveUserKey with type %{public}u enter", type);
    int ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, type, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED && ret != -ENOENT) {
#ifdef USER_CRYPTO_MIGRATE_KEY
        std::error_code errCode;
        std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
        if ((!token.empty() || !secret.empty()) && std::filesystem::exists(elNeedRestorePath, errCode)) {
            LOGW("Migrate PrepareUserDirsAndUpdateUserAuth userId %{public}u, type %{public}u", userId, type);
            ret = PrepareUserDirsAndUpdateUserAuth(userId, type, token, secret);
        }
#endif
        if (ret != E_OK) {
            LOGE("active and restore fail, ret %{public}d, userId %{public}u, type %{public}u sec empty %{public}d",
                 ret, userId, type, secret.empty());
            return ret;
        }
    } else if (ret == -ENOENT) {
        LOGW("start GenerateKeyAndPrepareUserDirs userId %{public}u, type %{public}u sec empty %{public}d",
             userId, type, secret.empty());
        ret = GenerateKeyAndPrepareUserDirs(userId, type, token, secret);
        if (ret != E_OK) {
            LOGE("active and generate fail ret %{public}d, userId %{public}u, type %{public}u, sec empty %{public}d",
                 ret, userId, type, secret.empty());
            return ret;
        }
    }

    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::ActiveUserKeyAndPrepareElX(uint32_t userId,
                                                  const std::vector<uint8_t> &token,
                                                  const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = ActiveUserKeyAndPrepare(userId, EL3_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u, type %{public}u", userId, EL3_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL3");
        return ret;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL3",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: ACTIVE EL3: delay time = %{public}s", delay.c_str());

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = ActiveUserKeyAndPrepare(userId, EL4_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u, type %{public}u", userId, EL4_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL4");
        return ret;
    }
    delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL4",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: ACTIVE EL4: delay time = %{public}s", delay.c_str());

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = ActiveUserKeyAndPrepare(userId, EL5_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u, type %{public}u", userId, EL5_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL5");
        return ret;
    }
    delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL5",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: ACTIVE EL5: delay time = %{public}s", delay.c_str());
    return ret;
#endif
    return E_OK;
}

int32_t StorageDaemon::ActiveUserKey(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGW("userId=%{public}u, tok empty=%{public}d sec empty=%{public}d", userId, token.empty(), secret.empty());
    int32_t ret = E_OK;
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string natoRestore = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, EL2_KEY) + RESTORE_DIR;
    if (std::filesystem::exists(natoRestore, errCode)) {
        LOGW("Nato need_restore file=%{public}s is exist, invoke ActiveUserKey4Nato.", natoRestore.c_str());
        return ActiveUserKey4Nato(userId, token, secret);
    }

    std::string el2RestorePath = GetNeedRestoreFilePath(userId, USER_EL2_DIR);
    if (std::filesystem::exists(el2RestorePath, errCode)) {
        LOGW("El2 need_restore file=%{public}s is exist, invoke ActiveUserKey4Update.", el2RestorePath.c_str());
        ret = ActiveUserKey4Update(userId, token, secret);
    } else {
        LOGW("need_restore file not exist, invoke ActiveUserKey4Single.");
        ret = ActiveUserKey4Single(userId, token, secret);
    }
#endif
    if (ret != E_OK) {
        LOGE("Active user key for userId=%{public}d failed, ret=%{public}d.", userId, ret);
        return ret;
    }
    std::thread([this, userId]() { RestoreconElX(userId); }).detach();
    std::thread([this]() { ActiveAppCloneUserKey(); }).detach();
    std::thread([this, userId]() { UserManager::GetInstance().CheckDirsFromVec(userId); }).detach();

#ifdef USER_CRYPTO_MANAGER
    int32_t result = KeyManagerExt::GetInstance().ActiveUserKey(userId, token, secret);
    if (result != E_OK) {
        LOGE("KeyManagerExt ActiveUserKey failed, error = %{public}d, userId %{public}u", result, userId);
    }
#endif
    LOGW("Active user key for userId=%{public}d success.", userId);
    MemoryReclaimManager::ScheduleReclaimCurrentProcess(ACTIVE_USER_KEY_DELAY_SECOND);
    return ret;
}

int32_t StorageDaemon::ActiveUserKey4Single(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    int ret = E_OK;
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority();  // set tid priority to 40
    LOGW("userId %{public}u, tok empty %{public}d sec empty %{public}d", userId, token.empty(), secret.empty());
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, EL2_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("ActiveUserKey failed, userId=%{public}u, type=%{public}u, tok empty %{public}d sec empty %{public}d",
            userId, EL2_KEY, token.empty(), secret.empty());
        if (!token.empty() && !secret.empty()) {
            StorageRadar::ReportActiveUserKey("ActiveUserKey4Single::ActiveCeSceSeceUserKey", userId, ret, "EL2");
        }
        return E_ACTIVE_EL2_FAILED;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL2",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: Active ce sce sece user key for userId=%{public}d el2 success. delay time = %{public}s",
        userId, delay.c_str());

    ret = ActiveUserKeyAndPrepareElX(userId, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("ActiveUserKeyAndPrepareElX failed, userId %{public}u.", userId);
        KeyManager::GetInstance().NotifyUeceActivation(userId, ret, true);
        return ret;
    }
    LOGI("Active user key and prepare el3~el5 for single secen for userId=%{public}d success.", userId);

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    auto ueceRet = KeyManager::GetInstance().NotifyUeceActivation(userId, ret, ret == E_ACTIVE_REPEATED ? false : true);
    if (ueceRet != E_OK) {
        LOGE("NotifyUeceActivation failed, ret=%{public}d, userId=%{public}u.", ueceRet, userId);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Single::NotifyUeceActivation", userId, ueceRet, "EL5");
        return E_UNLOCK_APP_KEY2_FAILED;
    }
    delay = StorageService::StorageRadar::ReportDuration("UNLOCK USER APP KEY",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: UNLOCK USER APP KEY: delay time = %{public}s.", delay.c_str());
    LOGW("Active user key for single secen for userId=%{public}d success.", userId);
#endif
    return ret == E_ACTIVE_REPEATED ? E_OK : ret;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int32_t StorageDaemon::ActiveUserKey4Nato(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGW("Active user key for nato secen for userId=%{public}d.", userId);
    if (!token.empty() || !secret.empty()) {
        LOGE("ActiveUserKey4Nato failed, input token or secret is not empty.");
        ClearAllNatoRestoreKey(userId, true);
        return E_ACTIVE_EL2_FAILED;
    }
    int32_t ret = PrepareUserDirsAndUpdateAuth4Nato(userId, EL2_KEY, token);
    if (ret != E_OK) {
        LOGE("ActiveUserKey4Nato el2 failed, userId=%{public}u, keyType=%{public}u", userId, EL2_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Nato::PrepareUserDirsAndUpdateAuth4Nato", userId, ret, "EL2");
        ClearAllNatoRestoreKey(userId, true);
        return E_ACTIVE_EL2_FAILED;
    }
    LOGI("Prepare dirs and update auth for nato secen for userId=%{public}d el2 success.", userId);

    ret = PrepareUserDirsAndUpdateAuth4Nato(userId, EL3_KEY, token);
    if (ret != E_OK) {
        LOGE("ActiveUserKey4Nato el3 failed, userId %{public}u, type %{public}u", userId, EL3_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Nato::PrepareUserDirsAndUpdateAuth4Nato", userId, ret, "EL3");
        ClearAllNatoRestoreKey(userId, true);
        return ret;
    }
    LOGI("Prepare dirs and update auth for nato secen for userId=%{public}d el3 success.", userId);

    ret = PrepareUserDirsAndUpdateAuth4Nato(userId, EL4_KEY, token);
    if (ret != E_OK) {
        LOGE("ActiveUserKey4Nato el4 failed, userId %{public}u, type %{public}u", userId, EL4_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Nato::PrepareUserDirsAndUpdateAuth4Nato", userId, ret, "EL4");
        ClearAllNatoRestoreKey(userId, true);
        return ret;
    }
    LOGI("Prepare dirs and update auth for nato secen for userId=%{public}d el4 success.", userId);
    ClearAllNatoRestoreKey(userId, false);

    std::thread([this]() { ActiveAppCloneUserKey(); }).detach();
    LOGW("Active user key for nato secen for userId=%{public}d success.", userId);
    return E_OK;
}

int32_t StorageDaemon::ActiveUserKey4Update(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGW("Active user key for update secen for userId=%{public}d.", userId);
    if (token.empty() && secret.empty()) {
        return E_ACTIVE_EL2_FAILED;
    }
    int32_t ret = PrepareUserDirsAndUpdateUserAuth(userId, EL2_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKey fail, userId %{public}u, type %{public}u, tok empty %{public}d sec empty %{public}d",
            userId, EL2_KEY, token.empty(), secret.empty());
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Update::PrepareUserDirsAndUpdateUserAuth", userId, ret, "EL2");
        return E_ACTIVE_EL2_FAILED;
    }
    std::string EL0_NEED_RESTORE = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
    if (!SaveStringToFile(EL0_NEED_RESTORE, NEW_DOUBLE_2_SINGLE)) {
        LOGE("Save key type file failed");
        return E_SYS_KERNEL_ERR;
    }
    LOGI("Prepare dirs and update auth for update secen for userId=%{public}d el2 success.", userId);

    ret = ActiveUserKeyAndPrepareElX(userId, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("Active user key and prepare el3~el5 for update secen failed, userId %{public}u.", userId);
        KeyManager::GetInstance().NotifyUeceActivation(userId, ret, true);
        return ret;
    }
    LOGI("Active user key and prepare el3~el5 for update secen for userId=%{public}d success.", userId);

    auto ueceRet = KeyManager::GetInstance().NotifyUeceActivation(userId, ret, true);
    if (ueceRet != E_OK) {
        LOGE("NotifyUeceActivation failed, ret=%{public}d, userId=%{public}u.", ueceRet, userId);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Update::NotifyUeceActivation", userId, ueceRet, "EL5");
        return E_UNLOCK_APP_KEY2_FAILED;
    }
    LOGW("Active user key for update secen for userId=%{public}d success.", userId);
    return E_OK;
}

void StorageDaemon::ClearNatoRestoreKey(uint32_t userId, KeyType type, bool isClearAll)
{
    std::string natoKey = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, type);
    if (!isClearAll) {
        RmDirRecurse(natoKey + PATH_LATEST);
        return;
    }
    RmDirRecurse(natoKey);
    if ((type == EL2_KEY) && std::filesystem::is_empty(NATO_EL2_DIR)) {
        RmDirRecurse(NATO_EL2_DIR);
        RmDirRecurse(std::string(NATO_EL2_DIR) + "_bak");
    }
    if ((type == EL3_KEY) && std::filesystem::is_empty(NATO_EL3_DIR)) {
        RmDirRecurse(NATO_EL3_DIR);
        RmDirRecurse(std::string(NATO_EL3_DIR) + "_bak");
    }
    if ((type == EL4_KEY) && std::filesystem::is_empty(NATO_EL4_DIR)) {
        RmDirRecurse(NATO_EL4_DIR);
        RmDirRecurse(std::string(NATO_EL4_DIR) + "_bak");
    }
}

void StorageDaemon::ClearAllNatoRestoreKey(uint32_t userId, bool isClearAll)
{
    ClearNatoRestoreKey(userId, EL2_KEY, isClearAll);
    ClearNatoRestoreKey(userId, EL3_KEY, isClearAll);
    ClearNatoRestoreKey(userId, EL4_KEY, isClearAll);
}
#endif

int32_t StorageDaemon::RestoreconElX(uint32_t userId)
{
#ifdef USE_LIBRESTORECON
    LOGI("Begin to restorecon path, userId = %{public}d", userId);

    RestoreconRecurse((std::string(DATA_SERVICE_EL2) + "public").c_str());
    const std::string &path = std::string(DATA_SERVICE_EL2) + std::to_string(userId);
    LOGI("RestoreconRecurse el2 public end, userId = %{public}d", userId);
    UserManager::GetInstance().RestoreconSystemServiceDirs(userId);
    LOGI("RestoreconSystemServiceDirs el2 end, userId = %{public}d", userId);
    RestoreconRecurse((std::string(DATA_SERVICE_EL2) + std::to_string(userId) + "/share").c_str());
    LOGI("RestoreconRecurse el2 share end, userId = %{public}d", userId);
    const std::string &DATA_SERVICE_EL2_HMDFS = std::string(DATA_SERVICE_EL2) + std::to_string(userId) + "/hmdfs/";
    Restorecon(DATA_SERVICE_EL2_HMDFS.c_str());
    LOGI("Restorecon el2 DATA_SERVICE_EL2_HMDFS end, userId = %{public}d", userId);
    const std::string &ACCOUNT_FILES = "/hmdfs/account/files/";
    const std::string &EL2_HMDFS_ACCOUNT_FILES = std::string(DATA_SERVICE_EL2) + std::to_string(userId) +
        ACCOUNT_FILES;
    Restorecon(EL2_HMDFS_ACCOUNT_FILES.c_str());
    LOGI("Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES end, userId = %{public}d", userId);
    const std::string &FILES_RECENT = "/hmdfs/account/files/.Recent";
    const std::string &EL2_HMDFS_ACCOUNT_FILES_RECENT = std::string(DATA_SERVICE_EL2) + std::to_string(userId) +
        FILES_RECENT;
    Restorecon(EL2_HMDFS_ACCOUNT_FILES_RECENT.c_str());
    LOGI("Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES_RECENT end, userId = %{public}d", userId);
#endif
    return E_OK;
}

int32_t StorageDaemon::InactiveUserKey(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority();  // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().InActiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("InActiveUserKey failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "InActiveUserKey",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_INACTIVE_USER_KEY,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    if (ret == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().InActiveUserKey(userId);
        if (result != E_OK) {
            LOGE("KeyManagerExt InActiveUserKey failed, error = %{public}d, userId %{public}u", result, userId);
        }
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::LockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().LockUserScreen(userId);
    if (ret != E_OK) {
        LOGE("LockUserScreen failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "LockUserScreen",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_LOCK_USER_SCREEN,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    if (ret == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().InActiveUserKey(userId);
        if (result != E_OK) {
            LOGE("KeyManagerExt InActiveUserKey failed, error = %{public}d, userId %{public}u", result, userId);
        }
    }
    MemoryReclaimManager::ScheduleReclaimCurrentProcess(LOCK_USER_SCREEN_DELAY_SECOND);
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UnlockUserScreen(uint32_t userId,
                                        const std::vector<uint8_t> &token,
                                        const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority();  // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().UnlockUserScreen(userId, token, secret);
    if (ret != E_OK) {
        LOGE("UnlockUserScreen failed, userId=%{public}u, ret=%{public}d.", userId, ret);
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "UnlockUserScreen",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_UNLOCK_USER_SCREEN,
            .keyElxLevel = (ret == E_UNLOCK_APP_KEY2_FAILED) ? "EL5" : "EL3/EL4",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    if (ret == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().ActiveUserKey(userId, token, secret);
        if (result != E_OK) {
            LOGE("KeyManagerExt ActiveUserKey failed, error = %{public}d, userId %{public}u", result, userId);
        }
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance().GetLockScreenStatus(userId, lockScreenStatus);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
#ifdef USER_CRYPTO_MANAGER
    int ret = KeyManager::GetInstance().GenerateAppkey(userId, hashId, keyId, needReSet);
    if (ret != E_OK) {
        StorageRadar::ReportUserKeyResult("GenerateAppKey", userId, ret, EL5,
            "not support uece / EL5key is nullptr or ENONET");
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
#ifdef USER_CRYPTO_MANAGER
    int ret = ret = KeyManager::GetInstance().DeleteAppkey(userId, keyId);
    if (ret != E_OK) {
        StorageRadar::ReportUserKeyResult("DeleteAppKey", userId, ret, EL5,
            "EL5key is nullptr / Failed to delete AppKey2");
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::CreateRecoverKey(uint32_t userId,
                                        uint32_t userType,
                                        const std::vector<uint8_t> &token,
                                        const std::vector<uint8_t> &secret)
{
    LOGI("begin to CreateRecoverKey");
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance().CreateRecoverKey(userId, userType, token, secret);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("begin to SetRecoverKey");
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance().SetRecoverKey(key);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
    LOGI("begin to ResetSecretWithRecoveryKey");
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority();  // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().UpdateKeyContext(userId, needRemoveTmpKey);
    if (ret != E_OK) {
        LOGE("UpdateKeyContext failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = "account_mgr",
            .userId = userId,
            .funcName = "UpdateKeyContext",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_UPDATE_KEY_CONTEXT,
            .keyElxLevel = "ELx",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority();  // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    if (ret != E_OK) {
        LOGE("GetFileEncryptStatus failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "GetFileEncryptStatus",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_GET_FILE_ENCRYPT_STATUS,
            .keyElxLevel = "ELx",
            .errorCode = ret
        };
        StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
#ifdef USER_CRYPTO_MIGRATE_KEY
    needActive = IsNeedRestorePathExist(userId, false);
#endif
    return E_OK;
}

void StorageDaemon::ActiveAppCloneUserKey()
{
#ifdef USER_CRYPTO_MANAGER
    unsigned int failedUserId = 0;
    auto ret = AppCloneKeyManager::GetInstance().ActiveAppCloneUserKey(failedUserId);
    if ((ret != E_OK) && (ret != E_NOT_SUPPORT)) {
        LOGE("ActiveAppCloneUserKey failed, errNo %{public}d", ret);
#ifdef USER_CRYPTO_MIGRATE_KEY
        uint32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2 |
                         IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4;
        bool isOsAccountExists = true;
        StorageService::KeyCryptoUtils::CheckAccountExists(failedUserId, isOsAccountExists);
        std::error_code errCode;
        std::string el4NeedRestorePath = GetNeedRestoreFilePathByType(failedUserId, EL4_KEY);
        if ((failedUserId >= START_APP_CLONE_USER_ID && failedUserId <= MAX_APP_CLONE_USER_ID) &&
            !isOsAccountExists && !std::filesystem::exists(el4NeedRestorePath, errCode)) {
            ret = UserManager::GetInstance().DestroyUserDirs(failedUserId, flags);
            LOGW("Destroy user %{public}d dirs, ret is: %{public}d", failedUserId, ret);
            ret = KeyManager::GetInstance().DeleteUserKeys(failedUserId);
            LOGW("Delete user %{public}d keys, ret is: %{public}d", failedUserId, ret);
        }
#endif
    }
#endif
}

int32_t StorageDaemon::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    return MountManager::GetInstance().IsFileOccupied(path, inputList, outputList, isOccupy);
}

void StorageDaemon::SetPriority()
{
    int tid = syscall(SYS_gettid);
    if (setpriority(PRIO_PROCESS, tid, PRIORITY_LEVEL) != 0) {
        LOGE("failed to set priority");
    }
    LOGW("set storage_daemon priority: %{public}d", tid);
}

int32_t StorageDaemon::InactiveUserPublicDirKey(uint32_t userId)
{
    int32_t ret = E_OK;
#ifdef USER_CRYPTO_MANAGER
    ret = KeyManagerExt::GetInstance().InActiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("InactiveUserPublicDirKey failed, error = %{public}d, userId %{public}u", ret, userId);
    }
#endif
    return ret;
}

int32_t StorageDaemon::UpdateUserPublicDirPolicy(uint32_t userId)
{
    int32_t ret = E_OK;
#ifdef USER_CRYPTO_MANAGER
    ret = KeyManagerExt::GetInstance().UpdateUserPublicDirPolicy(userId);
    if (ret != E_OK) {
        LOGE("UpdateUserPublicDirPolicy failed, error = %{public}d, userId %{public}u", ret, userId);
    }
#endif
    return ret;
}

int StorageDaemon::RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
#ifdef EL5_FILEKEY_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = KeyManager::GetInstance().RegisterUeceActivationCallback(ueceCallback);
    auto delay = StorageService::StorageRadar::ReportDuration("RegisterUeceActivationCallback",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, StorageService::DEFAULT_USERID);
    LOGE("SD_DURATION: RegisterUeceActivation Callback: ret = %{public}d, delay time =%{public}s",
        ret, delay.c_str());
    return ret;
#else
    LOGD("EL5_FILEKEY_MANAGER is not supported");
    return E_OK;
#endif
}

int StorageDaemon::UnregisterUeceActivationCallback()
{
#ifdef EL5_FILEKEY_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = KeyManager::GetInstance().UnregisterUeceActivationCallback();
    auto delay = StorageService::StorageRadar::ReportDuration("UnregisterUeceActivationCallback",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, StorageService::DEFAULT_USERID);
    LOGE("SD_DURATION:UnregisterUeceActivation Callback: ret = %{public}d, delay time =%{public}s",
        ret, delay.c_str());
    return ret;
#else
    LOGD("EL5_FILEKEY_MANAGER is not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
#ifdef USER_CRYPTO_MIGRATE_KEY
    LOGI("Enter StorageDaemon::SetDirEncryptionPolicy!");
    std::string extraData = "userId is:" + std::to_string(userId) + ", level is:" + std::to_string(level)
        + ", path is:" + dirPath;
    if (level < EL1_SYS_KEY || level > EL4_USER_KEY) {
        StorageRadar::ReportCommonResult("SetDirEncryptionPolicy level is failed", userId, E_PARAMS_INVALID, extraData);
        LOGE("level is wrong, level is %{public}u", level);
        return E_PARAMS_INVALID;
    }

    if (level == EL1_SYS_KEY && userId != GLOBAL_USER_ID) {
        StorageRadar::ReportCommonResult("SetDirEncryptionPolicy setSysPolicy failed", userId, E_PARAMS_INVALID,
            extraData);
        LOGE("SysPolice must be root uid, level is %{public}u", level);
        return E_PARAMS_INVALID;
    }
    StorageService::EncryptionLevel keyLevel = UintToKeyType(level);
    LOGI("user id is : %{public}u, dir path is %{public}s, level is %{public}u", userId, dirPath.c_str(), level);
    auto ret = IsDirPathSupport(dirPath);
    if (ret != E_OK) {
        StorageRadar::ReportCommonResult("SetDirEncryptionPolicy IsDirPathSupport file failed", userId, ret, extraData);
        LOGE("IsDirPathSupport file failed, errNo %{public}d", ret);
        return ret;
    }

    ret = KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, keyLevel);
    if (ret != E_OK) {
        StorageRadar::ReportCommonResult("StorageDaemon::SetDirEncryptionPolicy", userId, ret, extraData);
        LOGE("VerifyAncoUserDirs file failed, errNo %{public}d", ret);
        return ret;
    }

    LOGI("StorageDaemon::SetDirEncryptionPolicy success.");
    return ret;
#else
    return E_NOT_SUPPORT;
#endif
}

StorageService::EncryptionLevel StorageDaemon::UintToKeyType(uint32_t type)
{
    switch (type) {
        case StorageService::EncryptionLevel::EL1_SYS_KEY:
            return StorageService::EncryptionLevel::EL1_SYS_KEY;
        case StorageService::EncryptionLevel::EL1_USER_KEY:
            return StorageService::EncryptionLevel::EL1_USER_KEY;
        case StorageService::EncryptionLevel::EL2_USER_KEY:
            return StorageService::EncryptionLevel::EL2_USER_KEY;
        case StorageService::EncryptionLevel::EL3_USER_KEY:
            return StorageService::EncryptionLevel::EL3_USER_KEY;
        case StorageService::EncryptionLevel::EL4_USER_KEY:
            return StorageService::EncryptionLevel::EL4_USER_KEY;
        default:
            return StorageService::EncryptionLevel::EL1_SYS_KEY;
    }
}

int32_t StorageDaemon::IsDirPathSupport(const std::string &dirPath)
{
    LOGI("check path is %{public}s", dirPath.c_str());
    if (access(dirPath.c_str(), F_OK) != 0) {
        LOGE("dirPath is not exist, please create dir");
        return E_NON_ACCESS;
    }
    if (!IsDir(dirPath)) {
        LOGE("dirPath is not dir, please create dir");
        return E_NOT_DIR_PATH;
    }
    if (strncmp(dirPath.c_str(), ANCO_DIR, strlen(ANCO_DIR)) != 0) {
        LOGE("dir is not permission.");
        return E_PARAMS_INVALID;
    }
    
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
