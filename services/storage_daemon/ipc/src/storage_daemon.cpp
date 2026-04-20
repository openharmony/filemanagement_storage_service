/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "utils/set_flag_utils.h"
#include "utils/storage_xcollie.h"
#include "utils/string_utils.h"
#include <dlfcn.h>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
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
#include "file_sharing/file_sharing.h"
#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "system_ability_definition.h"
#include "user/user_manager.h"
#include "utils/storage_utils.h"
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

StorageDaemon &StorageDaemon::GetInstance(void)
{
    static StorageDaemon instance;
    return instance;
}

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

int32_t StorageDaemon::GetCryptoFlag(KeyType type, uint32_t &flags)
{
    LOGI("[L1:StorageDaemon] GetCryptoFlag: >>> ENTER <<< type=%{public}u", type);
    switch (type) {
        case EL1_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1;
            LOGI("[L1:StorageDaemon] GetCryptoFlag: <<< EXIT SUCCESS <<< type=%{public}u, flags=%{public}u",
                type, flags);
            return E_OK;
        case EL2_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2;
            LOGI("[L1:StorageDaemon] GetCryptoFlag: <<< EXIT SUCCESS <<< type=%{public}u, flags=%{public}u",
                type, flags);
            return E_OK;
        case EL3_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL3;
            LOGI("[L1:StorageDaemon] GetCryptoFlag: <<< EXIT SUCCESS <<< type=%{public}u, flags=%{public}u",
                type, flags);
            return E_OK;
        case EL4_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL4;
            LOGI("[L1:StorageDaemon] GetCryptoFlag: <<< EXIT SUCCESS <<< type=%{public}u, flags=%{public}u",
                type, flags);
            return E_OK;
        case EL5_KEY:
            flags = IStorageDaemonEnum::CRYPTO_FLAG_EL5;
            LOGI("[L1:StorageDaemon] GetCryptoFlag: <<< EXIT SUCCESS <<< type=%{public}u, flags=%{public}u",
                type, flags);
            return E_OK;
        default:
            LOGE("[L1:StorageDaemon] GetCryptoFlag: <<< EXIT FAILED <<< type=%{public}u is invalid", type);
            return E_KEY_TYPE_INVALID;
    }
}

#ifdef USER_CRYPTO_MIGRATE_KEY
std::string StorageDaemon::GetNeedRestoreFilePath(int32_t userId, const std::string &user_dir)
{
    LOGI("[L1:StorageDaemon] GetNeedRestoreFilePath: >>> ENTER <<< userId=%{public}d, user_dir=%{public}s",
         userId, user_dir.c_str());
    std::string path = user_dir + "/" + std::to_string(userId) + "/latest/need_restore";
    LOGI("[L1:StorageDaemon] GetNeedRestoreFilePath: <<< EXIT SUCCESS <<< path=%{public}s", path.c_str());
    return path;
}

std::string StorageDaemon::GetNeedRestoreFilePathByType(int32_t userId, KeyType type)
{
    LOGI("[L1:StorageDaemon] GetNeedRestoreFilePathByType: >>> ENTER <<< userId=%{public}d, type=%{public}u",
        userId, type);
    switch (type) {
        case EL1_KEY:
            {
                std::string path = GetNeedRestoreFilePath(userId, USER_EL1_DIR);
                LOGI("[L1:StorageDaemon] GetNeedRestoreFilePathByType: <<< EXIT SUCCESS <<< path=%{public}s",
                    path.c_str());
                return path;
            }
        case EL2_KEY:
            {
                std::string path = GetNeedRestoreFilePath(userId, USER_EL2_DIR);
                LOGI("[L1:StorageDaemon] GetNeedRestoreFilePathByType: <<< EXIT SUCCESS <<< path=%{public}s",
                    path.c_str());
                return path;
            }
        case EL3_KEY:
            {
                std::string path = GetNeedRestoreFilePath(userId, USER_EL3_DIR);
                LOGI("[L1:StorageDaemon] GetNeedRestoreFilePathByType: <<< EXIT SUCCESS <<< path=%{public}s",
                    path.c_str());
                return path;
            }
        case EL4_KEY:
            {
                std::string path = GetNeedRestoreFilePath(userId, USER_EL4_DIR);
                LOGI("[L1:StorageDaemon] GetNeedRestoreFilePathByType: <<< EXIT SUCCESS <<< path=%{public}s",
                     path.c_str());
                return path;
            }
        case EL5_KEY:
            {
                std::string path = GetNeedRestoreFilePath(userId, USER_EL5_DIR);
                LOGI("[L1:StorageDaemon] GetNeedRestoreFilePathByType: <<< EXIT SUCCESS <<< path=%{public}s",
                     path.c_str());
                return path;
            }
        default:
            LOGE("[L1:StorageDaemon] GetNeedRestoreFilePathByType: <<< EXIT FAILED <<< type=%{public}u is invalid",
                type);
            return "";
    }
}

int32_t StorageDaemon::RestoreOneUserKey(int32_t userId, KeyType type)
{
    LOGI("[L1:StorageDaemon] RestoreOneUserKey: >>> ENTER <<< userId=%{public}d, type=%{public}u", userId, type);
    uint32_t flags = 0;
    int32_t ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT FAILED <<< GetCryptoFlag ret=%{public}d", ret);
        return ret;
    }

    std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
    if (elNeedRestorePath.empty()) {
        LOGE("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT FAILED <<< elNeedRestorePath is empty");
        return E_KEY_TYPE_INVALID;
    }

    std::error_code errCode;
    if (!std::filesystem::exists(elNeedRestorePath, errCode)) {
        LOGI("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT SUCCESS <<< elNeedRestorePath not exist,"
            "userId=%{public}d, type=%{public}u", userId, type);
        return E_OK;
    }
    std::string singleRestoreVersion;
    (void) OHOS::LoadStringFromFile(elNeedRestorePath, singleRestoreVersion);
    LOGI("[L1:StorageDaemon] start restore User %{public}u el%{public}u, "
        "restore version = %{public}s", userId, type, singleRestoreVersion.c_str());
    ret = KeyManager::GetInstance().RestoreUserKey(userId, type);
    if (ret != E_OK) {
        if (type != EL1_KEY) {
            LOGE("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT FAILED <<< userId=%{public}u, type=%{public}u,"
                "ret=%{public}d", userId, type, ret);
            return E_MIGRATE_ELX_FAILED; // maybe need user key, so return E_OK to continue
        }
        LOGE("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT FAILED <<< RestoreUserKey EL1_KEY failed,"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        return ret;
    }

    ret = UserManager::GetInstance().PrepareUserDirsForUpdate(userId, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT FAILED <<< PrepareUserDirs userId=%{public}u,"
            "flags=%{public}u, ret=%{public}d", userId, flags, ret);
        return ret;
    }
    if (type == EL2_KEY) {
        PrepareUeceDir(userId);
    }
    if ((ret = DoStoreAndUpdate(userId, {}, {}, type)) != E_OK) {
        LOGE("DoStoreAndUpdate failed, userId:%{public}u, ret:%{public}d, type:%{public}u", userId, ret, type);
        return ret;
    }
    if (userId < StorageService::START_APP_CLONE_USER_ID || userId > StorageService::MAX_APP_CLONE_USER_ID) {
        if (type != EL1_KEY) {
            (void)remove(elNeedRestorePath.c_str());
        }
    }

    // for double2single update el2-4 without secret
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    LOGI("[L1:StorageDaemon] RestoreOneUserKey: <<< EXIT SUCCESS <<< "
        "userId=%{public}d, type=%{public}u", userId, type);
    return E_OK;
}

int32_t StorageDaemon::RestoreUserKey(int32_t userId, uint32_t flags)
{
    LOGI("[L1:StorageDaemon] RestoreUserKey: >>> ENTER <<< userId=%{public}d, flags=%{public}u", userId, flags);
    if (!IsNeedRestorePathExist(userId, true)) {
        LOGE("[L1:StorageDaemon] RestoreUserKey: <<< EXIT FAILED <<< need_restore file is not existed");
        return -EEXIST;
    }

    std::vector<KeyType> keyTypes = {EL1_KEY, EL2_KEY, EL3_KEY, EL4_KEY, EL5_KEY};
    for (auto type : keyTypes) {
        auto ret = RestoreOneUserKey(userId, type);
        if (ret == E_MIGRATE_ELX_FAILED) {
            LOGE("[L1:StorageDaemon] RestoreUserKey: waiting for user pin, userId=%{public}d, type=%{public}d",
                userId, type);
            LOGE("Try restore user: %{public}d type: %{public}d migrate key, wait user pin !", userId, type);
            break;
        }
        if (ret != E_OK) {
            LOGE("[L1:StorageDaemon] RestoreUserKey: <<< EXIT FAILED <<< userId=%{public}d, type=%{public}d,"
                "ret=%{public}d", userId, type, ret);
            return ret;
        }
    }
    MountManager::GetInstance().PrepareAppdataDir(userId);
    LOGI("[L1:StorageDaemon] RestoreUserKey: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}
#endif

int32_t StorageDaemon::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("[L1:StorageDaemon] PrepareUserDirs: >>> ENTER <<< userId=%{public}d, flags=%{public}u", userId, flags);
    // CRYPTO_FLAG_EL3 create el3,  CRYPTO_FLAG_EL4.create el4
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
            LOGE("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT FAILED <<< RestoreUserKey userId=%{public}d,"
                "ret=%{public}d", userId, restoreRet);
            return restoreRet;
        }
        LOGI("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
        return restoreRet;
    }
#endif
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT FAILED <<< GenerateUserKeys userId=%{public}d,"
            "ret=%{public}d", userId, ret);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::GenerateUserKeys", userId, ret, extraData);
        return ret;
    }
#endif
    int32_t destroyRet = UserManager::GetInstance().DestroyUserDirs(userId, flags);
    if (destroyRet != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT FAILED <<< UserManager::DestroyUserDirs userId=%{public}d,"
            "ret=%{public}d", userId, destroyRet);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::UserManager::DestroyUserDirs", userId, destroyRet,
            extraData);
    }
    int32_t prepareRet = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (prepareRet != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT FAILED <<< UserManager::PrepareUserDirs userId=%{public}d,"
            "ret=%{public}d", userId, prepareRet);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::UserManager::PrepareUserDirs", userId, prepareRet, extraData);
    }
    MountManager::GetInstance().PrepareAppdataDir(userId);
#ifdef USER_CRYPTO_MANAGER
    if (prepareRet == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags);
        if (result != E_OK) {
            LOGE("[L1:StorageDaemon] PrepareUserDirs: KeyManagerExt GenerateUserKeys failed, userId=%{public}u,"
                "ret=%{public}d", userId, result);
            std::string extraData = "flags=" + std::to_string(flags);
            StorageRadar::ReportUserManager("PrepareUserDirs::KeyManagerExt::GenerateUserKeys", userId, result,
                extraData);
        }
    }
#endif
    if (prepareRet == E_OK) {
        LOGI("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT SUCCESS <<< userId=%{public}d, flags=%{public}u",
            userId, flags);
    } else {
        LOGE("[L1:StorageDaemon] PrepareUserDirs: <<< EXIT FAILED <<< userId=%{public}d, ret=%{public}d",
            userId, prepareRet);
    }
    return prepareRet;
}

int32_t StorageDaemon::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("[L1:StorageDaemon] DestroyUserDirs: >>> ENTER <<< userId=%{public}d, flags=%{public}u", userId, flags);
    int32_t errCode = 0;
    // CRYPTO_FLAG_EL3 destroy el3,  CRYPTO_FLAG_EL4 destroy el4
    flags = flags | IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4 |
            IStorageDaemonEnum::CRYPTO_FLAG_EL5;
    int32_t destroyUserRet = UserManager::GetInstance().DestroyUserDirs(userId, flags);
    if (destroyUserRet != E_OK) {
        errCode = destroyUserRet;
        LOGE("[L1:StorageDaemon] DestroyUserDirs: <<< EXIT FAILED <<< DestroyUserDirs userId=%{public}d,"
            "ret=%{public}d", userId, destroyUserRet);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs", userId, errCode, extraData);
    }

#ifdef USER_CRYPTO_MANAGER
    destroyUserRet = KeyManager::GetInstance().DeleteUserKeys(userId);
    if (destroyUserRet != E_OK) {
        errCode = destroyUserRet;
        LOGW("[L1:StorageDaemon] DestroyUserDirs: DeleteUserKeys failed, userId=%{public}d, ret=%{public}d",
            userId, destroyUserRet);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs::DeleteUserKeys", userId, errCode, extraData);
    }
    if (destroyUserRet == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().DeleteUserKeys(userId);
        if (result != E_OK) {
            LOGE("[L1:StorageDaemon] DestroyUserDirs: KeyManagerExt DeleteUserKeys failed, userId=%{public}u,"
                "ret=%{public}d", userId, result);
        }
    }
    if (errCode == E_OK) {
        LOGI("[L1:StorageDaemon] DestroyUserDirs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:StorageDaemon] DestroyUserDirs: <<< EXIT FAILED <<< userId=%{public}d, ret=%{public}d",
            userId, errCode);
    }
    return errCode;
#else
    if (errCode == E_OK) {
        LOGI("[L1:StorageDaemon] DestroyUserDirs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:StorageDaemon] DestroyUserDirs: <<< EXIT FAILED <<< userId=%{public}d, ret=%{public}d",
            userId, errCode);
    }
    return errCode;
#endif
}

int32_t StorageDaemon::CompleteAddUser(int32_t userId)
{
    LOGI("[L1:StorageDaemon] CompleteAddUser: >>> ENTER <<< userId=%{public}d", userId);
    if (userId >= StorageService::START_APP_CLONE_USER_ID && userId < StorageService::MAX_APP_CLONE_USER_ID) {
        LOGE("[L1:StorageDaemon] CompleteAddUser: <<< EXIT SUCCESS <<< app clone user, userId=%{public}d", userId);
        return E_OK;
    }
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, EL1_KEY);
    if (elNeedRestorePath.empty() || !std::filesystem::exists(elNeedRestorePath, errCode)) {
        LOGI("[L1:StorageDaemon] CompleteAddUser: <<< EXIT SUCCESS <<< userId=%{public}d, elNeedRestorePath not exist",
            userId);
        return E_OK;
    }
    (void)remove(elNeedRestorePath.c_str());
#endif
    LOGI("[L1:StorageDaemon] CompleteAddUser: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t StorageDaemon::InitGlobalKey(void)
{
    LOGI("[L1:StorageDaemon] InitGlobalKey: >>> ENTER <<<");
#ifdef USER_CRYPTO_MANAGER
    int ret = KeyManager::GetInstance().InitGlobalDeviceKey();
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] InitGlobalKey: <<< EXIT FAILED <<< ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("InitGlobalKey::InitGlobalDeviceKey", 0, ret, "EL1", "");
    }
#ifdef USE_LIBRESTORECON
    int32_t restoreRet = RestoreconRecurse(DATA_SERVICE_EL0_STORAGE_DAEMON_SD);
    if (restoreRet != E_OK) {
        LOGE("RestoreconRecurse failed, ret=%{public}d", restoreRet);
    }
#endif
    return ret;
#else
    LOGI("[L1:StorageDaemon] InitGlobalKey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::InitGlobalUserKeys(void)
{
    LOGI("[L1:StorageDaemon] InitGlobalUserKeys: >>> ENTER <<<");
#ifdef USER_FILE_SHARING
    // File sharing depends on the /data/service/el1/public be decrypted.
    // A hack way to prepare the sharing dir, move it to callbacks after the parameter ready.
    if (SetupFileSharingDir() == -1) {
        LOGE("[L1:StorageDaemon] InitGlobalUserKeys: SetupFileSharingDir failed");
    }
#endif

#ifdef USER_CRYPTO_MANAGER

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string el1NeedRestorePath = GetNeedRestoreFilePath(START_USER_ID, USER_EL1_DIR);
    if (std::filesystem::exists(el1NeedRestorePath, errCode)) {
        LOGE("[L1:StorageDaemon] InitGlobalUserKeys: el1NeedRestorePath is exist, update NEW_DOUBLE_2_SINGLE");
        std::string doubleVersion;
        std::string el0NeedRestorePath = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
        bool isRead = OHOS::LoadStringFromFile(el0NeedRestorePath, doubleVersion);
        int newSingleVersion = std::atoi(doubleVersion.c_str()) + 1;
        LOGW("[L1:StorageDaemon] Process NEW_DOUBLE(version:%{public}s}) ——> "
            "SINGLE Frame(version:%{public}d), ret: %{public}d",
            doubleVersion.c_str(), newSingleVersion, isRead);
        if (!SaveStringToFile(el0NeedRestorePath, std::to_string(newSingleVersion))) {
            LOGE("[L1:StorageDaemon] InitGlobalUserKeys: <<< EXIT FAILED <<< Save NEW_DOUBLE_2_SINGLE file failed");
            StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::SaveStringToFile", START_USER_ID,
                E_SAVE_STRING_TO_FILE_ERR, "EL1", "path=" + el0NeedRestorePath);
            return E_SAVE_KEY_TYPE_ERROR;
        }
    }
#endif

    int ret = KeyManager::GetInstance().InitGlobalUserKeys();
    if (ret) {
        LOGE("[L1:StorageDaemon] InitGlobalUserKeys: <<< EXIT FAILED <<< InitGlobalUserKeys ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys", GLOBAL_USER_ID, ret, "EL1", "");
        return ret;
    }
#endif
#ifdef USE_LIBRESTORECON
    RestoreconRecurse(DATA_SERVICE_EL1_PUBLIC_STORAGE_DAEMON_SD);
#endif
    auto result = UserManager::GetInstance().PrepareAllUserEl1Dirs();
    if (result != E_OK) {
        LOGE("[L1:StorageDaemon] InitGlobalUserKeys: <<< EXIT FAILED <<< PrepareAllUserEl1Dirs ret=%{public}d", result);
        StorageRadar::ReportUserKeyResult("PrepareAllUserEl1Dirs", GLOBAL_USER_ID, result, "EL1", "");
    }
    result = MountManager::GetInstance().PrepareAppdataDir(GLOBAL_USER_ID);
    if (result != E_OK) {
        LOGE("[L1:StorageDaemon] InitGlobalUserKeys: <<< EXIT FAILED <<< PrepareAppdataDir ret=%{public}d", result);
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::PrepareAppdataDir", GLOBAL_USER_ID, result, "EL1", "");
    }
    LOGI("[L1:StorageDaemon] InitGlobalUserKeys: <<< EXIT SUCCESS <<<");
    std::thread thread([this]() { SetDeleteFlag4KeyFiles(); });
    thread.detach();
    return result;
}
void StorageDaemon::SetDeleteFlag4KeyFiles()
{
    std::string el0DeviceKeyPath = "/data/service/el0/storage_daemon/sd";
    std::string el1UserKeyPath = "/data/service/el1/public/storage_daemon/sd";
    StorageService::SetFlagUtils::SetDelFlagsRecursive(el0DeviceKeyPath);
    StorageService::SetFlagUtils::SetDelFlagsRecursive(el1UserKeyPath);
}
int32_t StorageDaemon::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    LOGI("[L1:StorageDaemon] EraseAllUserEncryptedKeys: >>> ENTER <<< count=%{public}zu", localIdList.size());
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().EraseAllUserEncryptedKeys(localIdList);
    LOGI("[L1:StorageDaemon] EraseAllUserEncryptedKeys result, ret: %{public}d", ret);
    StorageRadar::ReportUserKeyResult("StorageDaemon::EraseAllUserEncryptedKeys", DEFAULT_USERID, ret, "ELx", "");
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] EraseAllUserEncryptedKeys: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:StorageDaemon] EraseAllUserEncryptedKeys: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
#else
    LOGI("[L1:StorageDaemon] EraseAllUserEncryptedKeys: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &oldSecret,
                                      const std::vector<uint8_t> &newSecret)
{
    LOGI("[L1:StorageDaemon] UpdateUserAuth: >>> ENTER <<< userId=%{public}u, secureUid=%{public}s",
        userId, GetAnonyString(std::to_string(secureUid)).c_str());
    UserTokenSecret userTokenSecret = {
        .token = token, .oldSecret = oldSecret, .newSecret = newSecret, .secureUid = secureUid};
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().UpdateUserAuth(userId, userTokenSecret);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] UpdateUserAuth: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
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
    LOGI("[L1:StorageDaemon] UpdateUserAuth: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
#else
    LOGI("[L1:StorageDaemon] UpdateUserAuth: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                    const std::vector<uint8_t> &newSecret,
                                                    uint64_t secureUid,
                                                    uint32_t userId,
                                                    const std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("[L1:StorageDaemon] UpdateUseAuthWithRecoveryKey: >>> ENTER <<< userId=%{public}u, secureUid=%{public}s",
        userId, GetAnonyString(std::to_string(secureUid)).c_str());
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().UpdateUseAuthWithRecoveryKey(authToken, newSecret,
        secureUid, userId, plainText);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] UpdateUseAuthWithRecoveryKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:StorageDaemon] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
        StorageRadar::ReportUpdateUserAuth("UpdateUseAuthWithRecoveryKey", userId, ret, "ELx", "recovery_key");
    }
    return ret;
#else
    LOGI("[L1:StorageDaemon] UpdateUseAuthWithRecoveryKey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

#ifdef USER_CRYPTO_MIGRATE_KEY
std::string StorageDaemon::GetNeedRestoreVersion(uint32_t userId, KeyType type)
{
    LOGI("[L1:StorageDaemon] GetNeedRestoreVersion: >>> ENTER <<< userId=%{public}u, type=%{public}u", userId, type);
    std::string need_restore_path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, type) + RESTORE_DIR;
    std::string need_restore_version;
    OHOS::LoadStringFromFile(need_restore_path, need_restore_version);
    LOGI("[L1:StorageDaemon] GetNeedRestoreVersion: <<< EXIT SUCCESS <<< version=%{public}s",
        need_restore_version.c_str());
    return need_restore_version;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuth(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuth: >>> ENTER <<< userId=%{public}u, type=%{public}u",
        userId, type);
    int32_t ret = E_OK;
    std::string need_restore_version = GetNeedRestoreVersion(userId, type);
    int32_t OLD_UPDATE_VERSION_MAXLIMIT = std::atoi(NEW_DOUBLE_2_SINGLE);
    if (std::atoi(need_restore_version.c_str()) <= OLD_UPDATE_VERSION_MAXLIMIT) {
        LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuth: Old DOUBLE_2_SINGLE");
        ret = PrepareUserDirsAndUpdateUserAuthOld(userId, type, token, secret);
    } else {
        LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuth: New DOUBLE_2_SINGLE");
        ret = PrepareUserDirsAndUpdateUserAuthVx(userId, type, token, secret, need_restore_version);
    }
    // for double2single update el2-4 with secret
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuth: <<< EXIT SUCCESS <<< userId=%{public}u,"
            "type=%{public}u", userId, type);
    } else {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuth: <<< EXIT FAILED <<< userId=%{public}u,"
            "type=%{public}u, ret=%{public}d", userId, type, ret);
    }
    return ret;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuthOld(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: >>> ENTER <<< userId=%{public}u, type=%{public}u",
        userId, type);
    int32_t ret = E_OK;
    uint32_t flags = 0;

    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: <<< EXIT FAILED <<< GetCryptoFlag"
            "ret=%{public}d", ret);
        return ret;
    }

    ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, type, token, {'!'});
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: <<< EXIT FAILED <<< ActiveCeSceSeceUserKey"
            "userId=%{public}u, type=%{public}u, ret=%{public}d", userId, type, ret);
        return ret;
    }

    uint64_t secureUid = { 0 };
    if (!IamClient::GetInstance().GetSecureUid(userId, secureUid)) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: Get secure uid form iam failed");
    }
    UserTokenSecret userTokenSecret = { .token = token, .oldSecret = {'!'}, .newSecret = secret,
                                        .secureUid = secureUid };
    ret = KeyManager::GetInstance().UpdateCeEceSeceUserAuth(userId, userTokenSecret, type, false);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: <<< EXIT FAILED <<< UpdateCeEceSeceUserAuth"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        std::string isOldEmy = userTokenSecret.oldSecret.empty() ? "true" : "false";
        std::string isNewEmy = userTokenSecret.newSecret.empty() ? "true" : "false";
        std::string secretInfo = "oldSecret isEmpty = " + isOldEmy + ", newSecret isEmpty = " + isNewEmy;
        StorageRadar::ReportUpdateUserAuth("PrepareUserDirsAndUpdateUserAuth::UpdateCeEceSeceUserAuth",
            userId, ret, std::to_string(type), secretInfo);
        return ret;
    }

    ret = KeyManager::GetInstance().UpdateCeEceSeceKeyContext(userId, type);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: <<< EXIT FAILED <<< UpdateCeEceSeceKeyContext"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportUpdateUserAuth("PrepareUserDirsAndUpdateUserAuth::UpdateCeEceSeceKeyContext",
            userId, ret, std::to_string(type), "");
        return ret;
    }

    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: try to destory dir first, userId=%{public}u,"
        "flags=%{public}u", userId, flags);
    (void)UserManager::GetInstance().DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: <<< EXIT FAILED <<< PrepareUserDirs"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        return ret;
    }
    if (flags == IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthOld: <<< EXIT SUCCESS <<< userId=%{public}u,"
        "type=%{public}u", userId, type);
    return E_OK;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuthVx(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret,
    const std::string needRestoreVersion)
{
    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: >>> ENTER <<< userId=%{public}u, type=%{public}u",
        userId, type);
    int32_t ret = E_OK;
    uint32_t flags = 0;

    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: <<< EXIT FAILED <<< GetCryptoFlag ret=%{public}d",
            ret);
        return ret;
    }
    std::string needRestorePath = KeyManager::GetInstance().GetKeyDirByUserAndType(userId, type) + RESTORE_DIR;
    uint32_t new_need_restore = static_cast<uint32_t>(std::atoi(needRestoreVersion.c_str()) + 1);
    std::string errMsg = "";
    if (new_need_restore == UpdateVersion::UPDATE_V4 &&
        !SaveStringToFileSync(needRestorePath, std::to_string(new_need_restore), errMsg)) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: <<< EXIT FAILED <<< SaveStringToFileSync"
            "userId=%{public}d, el%{public}d", userId, type);
        StorageRadar::ReportUpdateUserAuth("PrepareUserDirsAndUpdateUserAuthVx::SaveStringToFileSync",
            userId, E_SAVE_STRING_TO_FILE_ERR, std::to_string(type), errMsg);
    }
    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: New DOUBLE_2_SINGLE::ActiveCeSceSeceUserKey");
    ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, type, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: <<< EXIT FAILED <<< ActiveCeSceSeceUserKey"
            "userId=%{public}u, type=%{public}u, ret=%{public}d", userId, type, ret);
        return ret;
    }

    std::error_code errCode;
    std::string newVersion = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, type) + FSCRYPT_VERSION_DIR;
    if (std::filesystem::exists(newVersion, errCode)) {
        ClearNatoRestoreKey(userId, type, true);
        LOGI("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: <<< EXIT SUCCESS <<< userId=%{public}u,"
            "type=%{public}u, nato exists", userId, type);
        return E_OK;
    }

    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: try to destory dir first, userId=%{public}u,"
        "flags=%{public}u", userId, flags);
    ret = UserManager::GetInstance().PrepareUserDirsForUpdate(userId, flags);
    if (ret != E_OK) {
        LOGE("prepare user dirs for update failed, user:%{public}u, flags:%{public}u", userId, flags);
        return ret;
    }
    ret = DoStoreAndUpdate(userId, token, secret, type);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: <<< EXIT FAILED <<< PrepareUserDirs"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        return ret;
    }
    if (flags == IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    LOGI("[L1:StorageDaemon] PrepareUserDirsAndUpdateUserAuthVx: <<< EXIT SUCCESS <<< userId=%{public}u,"
        "type=%{public}u", userId, type);
    return E_OK;
}

int32_t StorageDaemon::PrepareUserDirsAndUpdateAuth4Nato(uint32_t userId,
    KeyType type, const std::vector<uint8_t> &token)
{
    LOGW("[L1:StorageDaemon] PrepareUserDirsAndUpdateAuth4Nato: >>> ENTER <<< userId=%{public}u, type=%{public}u",
        userId, type);
    std::error_code errCode;
    std::string natoRestore = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, type) + RESTORE_DIR;
    if (!std::filesystem::exists(natoRestore, errCode)) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateAuth4Nato: <<< EXIT FAILED <<< natoRestore not exist,"
            "userId=%{public}d, type=%{public}u", userId, type);
        return E_KEY_TYPE_INVALID;
    }

    int32_t ret = KeyManager::GetInstance().ActiveElxUserKey4Nato(userId, type, token);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateAuth4Nato: <<< EXIT FAILED <<< ActiveElxUserKey4Nato"
            "userId=%{public}d, type=%{public}u, ret=%{public}d", userId, type, ret);
        return ret;
    }

    uint32_t flags = 0;
    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateAuth4Nato: <<< EXIT FAILED <<< GetCryptoFlag ret=%{public}d",
            ret);
        return E_KEY_TYPE_INVALID;
    }
    ret = UserManager::GetInstance().DestroyUserDirs(userId, flags);
    if (ret != E_OK) {
        StorageRadar::ReportUserKeyResult("PrepareUserDirsAndUpdateAuth4Nato::DestroyUserDirs", userId, ret,
            std::to_string(type), "DestroyUserDirs failed, flags =" + std::to_string(flags));
    }
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] PrepareUserDirsAndUpdateAuth4Nato: <<< EXIT FAILED <<< PrepareUserDirs"
            "userId=%{public}d, ret=%{public}d", userId, ret);
        return E_NATO_PREPARE_USER_DIR_ERROR;
    }
    if (flags == IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    LOGI("[L1:StorageDaemon] PrepareUserDirsAndUpdateAuth4Nato: <<< EXIT SUCCESS <<< userId=%{public}u,"
        "type=%{public}u", userId, type);
    return E_OK;
}

bool StorageDaemon::IsNeedRestorePathExist(uint32_t userId, bool needCheckEl1)
{
    LOGI("[L1:StorageDaemon] IsNeedRestorePathExist: >>> ENTER <<< userId=%{public}u, needCheckEl1=%{public}d",
        userId, needCheckEl1);
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
    LOGI("[L1:StorageDaemon] IsNeedRestorePathExist: <<< EXIT SUCCESS <<< userId=%{public}u, isExist=%{public}d",
        userId, isExist);
    return isExist;
}

int32_t StorageDaemon::PrepareUeceDir(uint32_t userId)
{
    LOGI("[L1:StorageDaemon] PrepareUeceDir: >>> ENTER <<< userId=%{public}u", userId);
    int32_t ret = UserManager::GetInstance().DestroyUserDirs(userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5);
    LOGI("[L1:StorageDaemon] delete user %{public}u uece %{public}u, ret %{public}d",
        userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5, ret);
    ret = UserManager::GetInstance().PrepareUserDirs(userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5);
    LOGI("[L1:StorageDaemon] prepare user %{public}u uece %{public}u, ret %{public}d",
        userId, IStorageDaemonEnum::CRYPTO_FLAG_EL5, ret);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] PrepareUeceDir: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:StorageDaemon] PrepareUeceDir: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
    }
    return ret;
}
#endif

int32_t StorageDaemon::GenerateKeyAndPrepareUserDirs(uint32_t userId,
                                                     KeyType type,
                                                     const std::vector<uint8_t> &token,
                                                     const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: >>> ENTER <<< userId=%{public}u, type=%{public}u,"
        "secretEmpty=%{public}d", userId, type, secret.empty());
#ifdef USER_CRYPTO_MANAGER
    int32_t ret;
    uint32_t flags = 0;

    LOGI("[L1:StorageDaemon] enter:");
    ret = KeyManager::GetInstance().GenerateUserKeyByType(userId, type, token, secret);
    if (ret != E_OK) {
        std::string extraData = std::string("userId=") + std::to_string(userId) + ", type=" + std::to_string(type);
        StorageRadar::ReportUserKeyResult("GenerateKeyAndPrepareUserDirs", userId, ret,
            "", extraData);
        LOGE("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: <<< EXIT FAILED <<< GenerateUserKeyByType"
            "userId=%{public}u, type=%{public}u, ret=%{public}d", userId, type, ret);
        return ret;
    }
    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: <<< EXIT FAILED <<< GetCryptoFlag ret=%{public}d", ret);
        return ret;
    }
    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(userId);
    if ((flags & IStorageDaemonEnum::CRYPTO_FLAG_EL5) && IsDir(keyUeceDir) && !std::filesystem::is_empty(keyUeceDir)) {
        LOGE("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: <<< EXIT SUCCESS <<< uece already exist,"
            "userId=%{public}u", userId);
#ifdef USER_CRYPTO_MIGRATE_KEY
        std::error_code errCode;
        std::string el0NeedRestore = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
        if (std::filesystem::exists(el0NeedRestore, errCode)) {
            UserManager::GetInstance().CreateElxBundleDataDir(userId, type);  // for double2single update el5
        }
#endif
        return ret;
    }
    ret = UserManager::GetInstance().DestroyUserDirs(userId, flags);
    if (ret != E_OK) {
        StorageRadar::ReportUserKeyResult("GenerateKeyAndPrepareUserDirs::DestroyUserDirs", userId, ret,
            std::to_string(type), "DestroyUserDirs failed, flags =" + std::to_string(flags));
    }
    ret = UserManager::GetInstance().PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: <<< EXIT FAILED <<< PrepareUserDirs"
            "userId=%{public}u, flags=%{public}u, ret=%{public}d", userId, flags, ret);
    }
    UserManager::GetInstance().CreateElxBundleDataDir(userId, type);
    LOGI("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: <<< EXIT SUCCESS <<< userId=%{public}u, type=%{public}u",
        userId, type);
    return ret;
#else
    LOGI("[L1:StorageDaemon] GenerateKeyAndPrepareUserDirs: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::ActiveUserKeyAndPrepare(uint32_t userId, KeyType type,
                                               const std::vector<uint8_t> &token,
                                               const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepare: >>> ENTER <<< userId=%{public}u, type=%{public}u,"
        "secretEmpty=%{public}d", userId, type, secret.empty());
#ifdef USER_CRYPTO_MANAGER
    int ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, type, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED && ret != -ENOENT) {
#ifdef USER_CRYPTO_MIGRATE_KEY
        std::error_code errCode;
        std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
        if ((!token.empty() || !secret.empty()) && std::filesystem::exists(elNeedRestorePath, errCode)) {
            LOGW("[L1:StorageDaemon] ActiveUserKeyAndPrepare: Migrate PrepareUserDirsAndUpdateUserAuth"
                "userId=%{public}u, type=%{public}u", userId, type);
            ret = PrepareUserDirsAndUpdateUserAuth(userId, type, token, secret);
        }
#endif
        if (ret != E_OK) {
            LOGE("[L1:StorageDaemon] ActiveUserKeyAndPrepare: <<< EXIT FAILED <<< userId=%{public}u, type=%{public}u,"
                "ret=%{public}d", userId, type, ret);
            return ret;
        }
    } else if (ret == -ENOENT) {
        LOGW("[L1:StorageDaemon] ActiveUserKeyAndPrepare: start GenerateKeyAndPrepareUserDirs userId=%{public}u,"
            "type=%{public}u", userId, type);
        ret = GenerateKeyAndPrepareUserDirs(userId, type, token, secret);
        if (ret != E_OK) {
            LOGE("[L1:StorageDaemon] ActiveUserKeyAndPrepare: <<< EXIT FAILED <<< GenerateKeyAndPrepareUserDirs"
                "userId=%{public}u, type=%{public}u, ret=%{public}d", userId, type, ret);
            return ret;
        }
    }

    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepare: <<< EXIT SUCCESS <<< userId=%{public}u, type=%{public}u",
        userId, type);
    return ret;
#else
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepare: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::ActiveUserKeyAndPrepareElX(uint32_t userId,
                                                  const std::vector<uint8_t> &token,
                                                  const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: >>> ENTER <<< userId=%{public}u, secretEmpty=%{public}d",
        userId, secret.empty());
#ifdef USER_CRYPTO_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = ActiveUserKeyAndPrepare(userId, EL3_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: <<< EXIT FAILED <<< ActiveUserKeyAndPrepare"
            "userId=%{public}u, type=EL3, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL3");
        return ret;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL3",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: EL3 delay=%{public}s, userId=%{public}u",
        delay.c_str(), userId);
    LOGI("SD_DURATION: ACTIVE EL3: delay time = %{public}s", delay.c_str());

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = ActiveUserKeyAndPrepare(userId, EL4_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: <<< EXIT FAILED <<< ActiveUserKeyAndPrepare"
            "userId=%{public}u, type=EL4, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL4");
        return ret;
    }
    delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL4",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: EL4 delay=%{public}s, userId=%{public}u",
        delay.c_str(), userId);
    LOGI("SD_DURATION: ACTIVE EL4: delay time = %{public}s", delay.c_str());

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = ActiveUserKeyAndPrepare(userId, EL5_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: <<< EXIT FAILED <<< ActiveUserKeyAndPrepare"
            "userId=%{public}u, type=EL5, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL5");
        return ret;
    }
    delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL5",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: EL5 delay=%{public}s, userId=%{public}u",
         delay.c_str(), userId);
    LOGI("SD_DURATION: ACTIVE EL5: delay time = %{public}s", delay.c_str());
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
#endif
    LOGI("[L1:StorageDaemon] ActiveUserKeyAndPrepareElX: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
}

int32_t StorageDaemon::ActiveUserKey(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] ActiveUserKey: >>> ENTER <<< userId=%{public}u, tokenEmpty=%{public}d,"
        "secretEmpty=%{public}d", userId, token.empty(), secret.empty());
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
        LOGW("[L1:StorageDaemon] ActiveUserKey: el2 need_restore exists, invoke ActiveUserKey4Update");
        ret = ActiveUserKey4Update(userId, token, secret);
    } else {
        LOGW("[L1:StorageDaemon] ActiveUserKey: need_restore not exist, invoke ActiveUserKey4Single");
        ret = ActiveUserKey4Single(userId, token, secret);
    }
#endif
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey: <<< EXIT FAILED <<< userId=%{public}d, ret=%{public}d", userId, ret);
        StorageRadar::ReportUserKeyResult("ActiveUserKey", userId, ret, "",
            "ActiveUserKey4Update/4Single failed, userId=" + std::to_string(userId) + ", ret=" + std::to_string(ret));
        return ret;
    }
    std::thread([this, userId]() { RestoreconElX(userId); }).detach();
    std::thread([this]() { ActiveAppCloneUserKey(); }).detach();
    std::thread([this, userId]() { UserManager::GetInstance().CheckDirsFromVec(userId); }).detach();

#ifdef USER_CRYPTO_MANAGER
    int32_t result = KeyManagerExt::GetInstance().ActiveUserKey(userId, token, secret);
    if (result != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey: KeyManagerExt ActiveUserKey failed, "
            "userId=%{public}u, ret=%{public}d", userId, result);
        StorageRadar::ReportUserKeyResult("ActiveUserKey::KeyManagerExt", userId, result, "",
            "KeyManagerExt ActiveUserKey failed, userId=" + std::to_string(userId) +
            ", ret=" + std::to_string(result));
    }
#endif
    LOGI("[L1:StorageDaemon] ActiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    MemoryReclaimManager::ScheduleReclaimCurrentProcess(ACTIVE_USER_KEY_DELAY_SECOND);
    return ret;
}

int32_t StorageDaemon::ActiveUserKey4Single(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] ActiveUserKey4Single: >>> ENTER <<< userId=%{public}u, tokenEmpty=%{public}d,"
        "secretEmpty=%{public}d", userId, token.empty(), secret.empty());
    int ret = E_OK;
#ifdef USER_CRYPTO_MANAGER
    LOGW("[L1:StorageDaemon] userId %{public}u, tok empty %{public}d sec empty %{public}d",
        userId, token.empty(), secret.empty());
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = KeyManager::GetInstance().ActiveCeSceSeceUserKey(userId, EL2_KEY, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Single: <<< EXIT FAILED <<< ActiveCeSceSeceUserKey userId=%{public}u,"
            "ret=%{public}d", userId, ret);
        if (!token.empty() && !secret.empty()) {
            StorageRadar::ReportActiveUserKey("ActiveUserKey4Single::ActiveCeSceSeceUserKey", userId, ret, "EL2");
        }
        return E_ACTIVE_EL2_FAILED;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("ACTIVE EL2",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L1:StorageDaemon] ActiveUserKey4Single: EL2 delay=%{public}s, userId=%{public}d", delay.c_str(), userId);

    ret = ActiveUserKeyAndPrepareElX(userId, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Single: <<< EXIT FAILED <<< ActiveUserKeyAndPrepareElX"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        KeyManager::GetInstance().NotifyUeceActivation(userId, ret, true);
        return ret;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Single: <<< EXIT SUCCESS <<< userId=%{public}d", userId);

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    auto ueceRet = KeyManager::GetInstance().NotifyUeceActivation(userId, ret, ret == E_ACTIVE_REPEATED ? false : true);
    if (ueceRet != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Single: <<< EXIT FAILED <<< NotifyUeceActivation userId=%{public}u,"
            "ret=%{public}d", userId, ueceRet);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Single::NotifyUeceActivation", userId, ueceRet, "EL5");
        return E_UNLOCK_APP_KEY2_FAILED;
    }
    delay = StorageService::StorageRadar::ReportDuration("UNLOCK USER APP KEY",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGW("[L1:StorageDaemon] Active user key for single secen for userId=%{public}d success, "
        "unlockAppKeyDelay=%{public}s", userId, delay.c_str());
#endif
    if (ret == E_OK || ret == E_ACTIVE_REPEATED) {
        LOGI("[L1:StorageDaemon] ActiveUserKey4Single: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Single: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
    }
    return ret == E_ACTIVE_REPEATED ? E_OK : ret;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int32_t StorageDaemon::ActiveUserKey4Nato(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] ActiveUserKey4Nato: >>> ENTER <<< userId=%{public}u", userId);
    if (!token.empty() || !secret.empty()) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Nato: <<< EXIT FAILED <<< token or secret is not empty");
        ClearAllNatoRestoreKey(userId, true);
        return E_ACTIVE_EL2_FAILED;
    }
    int32_t ret = PrepareUserDirsAndUpdateAuth4Nato(userId, EL2_KEY, token);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Nato: <<< EXIT FAILED <<< el2 PrepareUserDirsAndUpdateAuth4Nato"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Nato::PrepareUserDirsAndUpdateAuth4Nato", userId, ret, "EL2");
        ClearAllNatoRestoreKey(userId, true);
        return E_ACTIVE_EL2_FAILED;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Nato: el2 success, userId=%{public}d", userId);

    ret = PrepareUserDirsAndUpdateAuth4Nato(userId, EL3_KEY, token);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Nato: <<< EXIT FAILED <<< el3 PrepareUserDirsAndUpdateAuth4Nato"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Nato::PrepareUserDirsAndUpdateAuth4Nato", userId, ret, "EL3");
        ClearAllNatoRestoreKey(userId, true);
        return ret;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Nato: el3 success, userId=%{public}d", userId);

    ret = PrepareUserDirsAndUpdateAuth4Nato(userId, EL4_KEY, token);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Nato: <<< EXIT FAILED <<< el4 PrepareUserDirsAndUpdateAuth4Nato"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Nato::PrepareUserDirsAndUpdateAuth4Nato", userId, ret, "EL4");
        ClearAllNatoRestoreKey(userId, true);
        return ret;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Nato: el4 success, userId=%{public}d", userId);
    ClearAllNatoRestoreKey(userId, false);

    std::thread([this]() { ActiveAppCloneUserKey(); }).detach();
    LOGI("[L1:StorageDaemon] ActiveUserKey4Nato: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t StorageDaemon::ActiveUserKey4Update(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] ActiveUserKey4Update: >>> ENTER <<< userId=%{public}u, tokenEmpty=%{public}d,"
         "secretEmpty=%{public}d",
         userId, token.empty(), secret.empty());
    if (token.empty() && secret.empty()) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Update: <<< EXIT FAILED <<< token and secret are empty");
        return E_ACTIVE_EL2_FAILED;
    }
    int32_t ret = PrepareUserDirsAndUpdateUserAuth(userId, EL2_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Update: <<< EXIT FAILED <<< PrepareUserDirsAndUpdateUserAuth"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Update::PrepareUserDirsAndUpdateUserAuth", userId, ret, "EL2");
        return E_ACTIVE_EL2_FAILED;
    }
    std::string el0NeedRestorePath = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
    if (!SaveStringToFile(el0NeedRestorePath, NEW_DOUBLE_2_SINGLE)) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Update: <<< EXIT FAILED <<< Save key type file failed");
        return E_SYS_KERNEL_ERR;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Update: el2 success, userId=%{public}d", userId);

    ret = ActiveUserKeyAndPrepareElX(userId, token, secret);
    if (ret != E_OK && ret != E_ACTIVE_REPEATED) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Update: <<< EXIT FAILED <<< ActiveUserKeyAndPrepareElX"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        KeyManager::GetInstance().NotifyUeceActivation(userId, ret, true);
        return ret;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Update: el3-5 success, userId=%{public}d", userId);
    auto ueceRet = KeyManager::GetInstance().NotifyUeceActivation(userId, ret, true);
    if (ueceRet != E_OK) {
        LOGE("[L1:StorageDaemon] ActiveUserKey4Update: <<< EXIT FAILED <<< NotifyUeceActivation userId=%{public}u,"
            "ret=%{public}d", userId, ueceRet);
        StorageRadar::ReportActiveUserKey("ActiveUserKey4Update::NotifyUeceActivation", userId, ueceRet, "EL5");
        return E_UNLOCK_APP_KEY2_FAILED;
    }
    LOGI("[L1:StorageDaemon] ActiveUserKey4Update: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

static void DeleteKeyFiles(const std::string &basePath, const std::vector<std::string> &fileList)
{
    for (const auto &file : fileList) {
        unlink((basePath + file).c_str());
    }
}

void StorageDaemon::ClearKeyDirInfo(const std::string &path)
{
    std::vector<std::string> filesToDelete = {
        "/fscrypt_version",
        "/key_desc",
        "/latest/encrypted",
        "/latest/need_update",
        "/latest/sec_discard",
        "/latest/shield"
    };
    DeleteKeyFiles(path, filesToDelete);

    std::string dir = path + PATH_LATEST;
    if (!KeyManager::GetInstance().IsDirRecursivelyEmpty(dir.c_str())) {
        return;
    }
    rmdir(dir.c_str());
    if (KeyManager::GetInstance().IsDirRecursivelyEmpty(path.c_str())) {
            rmdir(path.c_str());
    }
}

void StorageDaemon::ClearKeyDir(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        LOGE("Failed to open directory %{public}s", path.c_str());
        return;
    }

    LOGI("First-level directories under %{public}s:", path.c_str());
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        if (ent->d_type == DT_DIR) {
            std::string fullPath = path + "/" + ent->d_name;
            ClearKeyDirInfo(fullPath);
        }
    }
    closedir(dir);
    if (KeyManager::GetInstance().IsDirRecursivelyEmpty(path.c_str())) {
        rmdir(path.c_str());
    }
}

void StorageDaemon::ClearNatoRestoreKey(uint32_t userId, KeyType type, bool isClearAll)
{
    LOGI("[L1:StorageDaemon] ClearNatoRestoreKey: >>> ENTER <<< userId=%{public}u, type=%{public}u,"
        "isClearAll=%{public}d", userId, type, isClearAll);
    std::string natoKey = KeyManager::GetInstance().GetNatoNeedRestorePath(userId, type);

    if (!isClearAll) {
        LOGI("[L1:StorageDaemon] ClearNatoRestoreKey: <<< EXIT SUCCESS <<< cleared latest only, userId=%{public}u,"
            "type=%{public}u", userId, type);
        // Only delete 4 files in latest directory
        std::vector<std::string> filesToDelete = {
            "/latest/encrypted",
            "/latest/need_update",
            "/latest/sec_discard",
            "/latest/shield"
        };
        DeleteKeyFiles(natoKey, filesToDelete);
        std::string dir = natoKey + PATH_LATEST;
        if (KeyManager::GetInstance().IsDirRecursivelyEmpty(dir.c_str())) {
            rmdir(dir.c_str());
        }
        return;
    }

    // Delete all 6 key files
    ClearKeyDirInfo(natoKey);

    if ((type == EL2_KEY) && std::filesystem::is_empty(NATO_EL2_DIR)) {
        ClearKeyDir(std::string(NATO_EL2_DIR));
        ClearKeyDir(std::string(NATO_EL2_DIR) + "_bak");
    }
    if ((type == EL3_KEY) && std::filesystem::is_empty(NATO_EL3_DIR)) {
        ClearKeyDir(std::string(NATO_EL3_DIR));
        ClearKeyDir(std::string(NATO_EL3_DIR) + "_bak");
    }
    if ((type == EL4_KEY) && std::filesystem::is_empty(NATO_EL4_DIR)) {
        ClearKeyDir(std::string(NATO_EL4_DIR));
        ClearKeyDir(std::string(NATO_EL4_DIR) + "_bak");
    }
    LOGI("[L1:StorageDaemon] ClearNatoRestoreKey: <<< EXIT SUCCESS <<< userId=%{public}u, type=%{public}u",
        userId, type);
}

void StorageDaemon::ClearAllNatoRestoreKey(uint32_t userId, bool isClearAll)
{
    LOGI("[L1:StorageDaemon] ClearAllNatoRestoreKey: >>> ENTER <<< userId=%{public}u, isClearAll=%{public}d",
         userId, isClearAll);
    ClearNatoRestoreKey(userId, EL2_KEY, isClearAll);
    ClearNatoRestoreKey(userId, EL3_KEY, isClearAll);
    ClearNatoRestoreKey(userId, EL4_KEY, isClearAll);
    LOGI("[L1:StorageDaemon] ClearAllNatoRestoreKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
}
#endif

int32_t StorageDaemon::RestoreconElX(uint32_t userId)
{
    LOGI("[L1:StorageDaemon] RestoreconElX: >>> ENTER <<< userId=%{public}d", userId);
#ifdef USE_LIBRESTORECON
    int32_t ret = RestoreconRecurse((std::string(DATA_SERVICE_EL2) + "public").c_str());
    if (ret != E_OK) {
        LOGE("RestoreconRecurse el2 public failed, ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("RestoreconElX::RestoreconRecurse", userId, ret, "EL2",
            "path=" + std::string(DATA_SERVICE_EL2) + "public");
    }
    const std::string &path = std::string(DATA_SERVICE_EL2) + std::to_string(userId);
    LOGI("[L1:StorageDaemon] RestoreconRecurse el2 public end, userId = %{public}d", userId);
    UserManager::GetInstance().RestoreconSystemServiceDirs(userId);
    LOGI("[L1:StorageDaemon] RestoreconSystemServiceDirs el2 end, userId = %{public}d", userId);
    ret = RestoreconRecurse((std::string(DATA_SERVICE_EL2) + std::to_string(userId) + "/share").c_str());
    if (ret != E_OK) {
        LOGE("RestoreconRecurse el2 share failed, ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("RestoreconElX::RestoreconRecurse", userId, ret, "EL2",
            "path=" + std::string(DATA_SERVICE_EL2) + std::to_string(userId) + "/share");
    }
    LOGI("[L1:StorageDaemon] RestoreconRecurse el2 share end, userId = %{public}d", userId);
    const std::string &DATA_SERVICE_EL2_HMDFS = std::string(DATA_SERVICE_EL2) + std::to_string(userId) + "/hmdfs/";
    ret = Restorecon(DATA_SERVICE_EL2_HMDFS.c_str());
    if (ret != E_OK) {
        LOGE("Restorecon el2 DATA_SERVICE_EL2_HMDFS failed, ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("RestoreconElX::Restorecon", userId, ret, "EL2",
            "path=" + DATA_SERVICE_EL2_HMDFS);
    }
    LOGI("[L1:StorageDaemon] Restorecon el2 DATA_SERVICE_EL2_HMDFS end, userId = %{public}d", userId);
    const std::string &ACCOUNT_FILES = "/hmdfs/account/files/";
    const std::string &EL2_HMDFS_ACCOUNT_FILES = std::string(DATA_SERVICE_EL2) + std::to_string(userId) +
        ACCOUNT_FILES;
    ret = Restorecon(EL2_HMDFS_ACCOUNT_FILES.c_str());
    if (ret != E_OK) {
        LOGE("Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES failed, ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("RestoreconElX::Restorecon", userId, ret, "EL2",
            "path=" + EL2_HMDFS_ACCOUNT_FILES);
    }
    LOGI("[L1:StorageDaemon] Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES end, userId = %{public}d", userId);
    const std::string &FILES_RECENT = "/hmdfs/account/files/.Recent";
    const std::string &EL2_HMDFS_ACCOUNT_FILES_RECENT = std::string(DATA_SERVICE_EL2) + std::to_string(userId) +
        FILES_RECENT;
    ret = Restorecon(EL2_HMDFS_ACCOUNT_FILES_RECENT.c_str());
    if (ret != E_OK) {
        LOGE("Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES_RECENT failed, ret=%{public}d", ret);
        StorageRadar::ReportUserKeyResult("RestoreconElX::Restorecon", userId, ret, "EL2",
            "path=" + EL2_HMDFS_ACCOUNT_FILES_RECENT);
    }
    LOGI("[L1:StorageDaemon] Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES_RECENT end, "
        "userId = %{public}d", userId);
#endif
    LOGI("[L1:StorageDaemon] RestoreconElX: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t StorageDaemon::InactiveUserKey(uint32_t userId)
{
    LOGI("[L1:StorageDaemon] InactiveUserKey: >>> ENTER <<< userId=%{public}u", userId);
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority(); // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().InActiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] InactiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
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
            LOGE("[L1:StorageDaemon] InactiveUserKey: KeyManagerExt InActiveUserKey failed, userId=%{public}u,"
                "ret=%{public}d", userId, result);
        }
    }
    LOGI("[L1:StorageDaemon] InactiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
#else
    LOGI("[L1:StorageDaemon] InactiveUserKey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::LockUserScreen(uint32_t userId)
{
    LOGI("[L1:StorageDaemon] LockUserScreen: >>> ENTER <<< userId=%{public}u", userId);
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().LockUserScreen(userId);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] LockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
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
            LOGE("[L1:StorageDaemon] LockUserScreen: KeyManagerExt InActiveUserKey failed, userId=%{public}u,"
                "ret=%{public}d", userId, result);
        }
    }
    MemoryReclaimManager::ScheduleReclaimCurrentProcess(LOCK_USER_SCREEN_DELAY_SECOND);
    LOGI("[L1:StorageDaemon] LockUserScreen: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
#else
    LOGI("[L1:StorageDaemon] LockUserScreen: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::UnlockUserScreen(uint32_t userId,
                                        const std::vector<uint8_t> &token,
                                        const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] UnlockUserScreen: >>> ENTER <<< userId=%{public}u, tokenEmpty=%{public}d,"
        "secretEmpty=%{public}d", userId, token.empty(), secret.empty());
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority(); // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().UnlockUserScreen(userId, token, secret);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] UnlockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
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
        return ret;
    }
    if (ret == E_OK) {
        int32_t result = KeyManagerExt::GetInstance().ActiveUserKey(userId, token, secret);
        if (result != E_OK) {
            LOGE("[L1:StorageDaemon] UnlockUserScreen: KeyManagerExt ActiveUserKey failed, userId=%{public}u,"
                "ret=%{public}d", userId, result);
        }
    }
    LOGI("[L1:StorageDaemon] UnlockUserScreen: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
#else
    LOGI("[L1:StorageDaemon] UnlockUserScreen: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    LOGI("[L1:StorageDaemon] GetLockScreenStatus: >>> ENTER <<< userId=%{public}u", userId);
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance().GetLockScreenStatus(userId, lockScreenStatus);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] GetLockScreenStatus: <<< EXIT SUCCESS <<< userId=%{public}u, status=%{public}d",
            userId, lockScreenStatus);
    } else {
        LOGE("[L1:StorageDaemon] GetLockScreenStatus: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
    }
    return ret;
#else
    LOGI("[L1:StorageDaemon] GetLockScreenStatus: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    LOGI("[L1:StorageDaemon] GenerateAppkey: >>> ENTER <<< userId=%{public}u, hashId=%{public}u, needReSet=%{public}d",
        userId, hashId, needReSet);
#ifdef USER_CRYPTO_MANAGER
    int ret = KeyManager::GetInstance().GenerateAppkey(userId, hashId, keyId, needReSet);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportUserKeyResult("GenerateAppKey", userId, ret, EL5,
            "not support uece / EL5key is nullptr or ENONET");
    }
    LOGI("[L1:StorageDaemon] GenerateAppkey: <<< EXIT SUCCESS <<< userId=%{public}u, keyId=%{public}s",
        userId, GetAnonyString(keyId).c_str());
    return ret;
#else
    LOGI("[L1:StorageDaemon] GenerateAppkey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
    LOGI("[L1:StorageDaemon] DeleteAppkey: >>> ENTER <<< userId=%{public}u, keyIdLen=%{public}zu",
         userId, keyId.size());
#ifdef USER_CRYPTO_MANAGER
    int ret = ret = KeyManager::GetInstance().DeleteAppkey(userId, keyId);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] DeleteAppkey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
        StorageRadar::ReportUserKeyResult("DeleteAppKey", userId, ret, EL5,
            "EL5key is nullptr / Failed to delete AppKey2");
        return ret;
    }
    LOGI("[L1:StorageDaemon] DeleteAppkey: <<< EXIT SUCCESS <<< userId=%{public}u, keyId=%{public}s",
        userId, GetAnonyString(keyId).c_str());
    return ret;
#else
    LOGI("[L1:StorageDaemon] DeleteAppkey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::CreateRecoverKey(uint32_t userId,
                                        uint32_t userType,
                                        const std::vector<uint8_t> &token,
                                        const std::vector<uint8_t> &secret)
{
    LOGI("[L1:StorageDaemon] CreateRecoverKey: >>> ENTER <<< userId=%{public}u, userType=%{public}u",
        userId, userType);
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    int32_t ret = KeyManager::GetInstance().CreateRecoverKey(userId, userType, token, secret);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] CreateRecoverKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:StorageDaemon] CreateRecoverKey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
    }
    return ret;
#else
    LOGI("[L1:StorageDaemon] CreateRecoverKey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("[L1:StorageDaemon] SetRecoverKey: >>> ENTER <<< keySize=%{public}zu", key.size());
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    int32_t ret = KeyManager::GetInstance().SetRecoverKey(key);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] SetRecoverKey: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:StorageDaemon] SetRecoverKey: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
#else
    LOGI("[L1:StorageDaemon] SetRecoverKey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
    LOGI("[L1:StorageDaemon] ResetSecretWithRecoveryKey: >>> ENTER <<< userId=%{public}u, rkType=%{public}u",
        userId, rkType);
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    int32_t ret = KeyManager::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] ResetSecretWithRecoveryKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:StorageDaemon] ResetSecretWithRecoveryKey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
    }
    return ret;
#else
    LOGI("[L1:StorageDaemon] ResetSecretWithRecoveryKey: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("[L1:StorageDaemon] UpdateKeyContext: >>> ENTER <<< userId=%{public}u, needRemoveTmpKey=%{public}d",
        userId, needRemoveTmpKey);
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority(); // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().UpdateKeyContext(userId, needRemoveTmpKey);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] UpdateKeyContext: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d", userId, ret);
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
        return ret;
    }
    LOGI("[L1:StorageDaemon] UpdateKeyContext: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
#else
    LOGI("[L1:StorageDaemon] UpdateKeyContext: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGI("[L1:StorageDaemon] GetFileEncryptStatus: >>> ENTER <<< userId=%{public}u, needCheckDirMount=%{public}d",
        userId, needCheckDirMount);
#ifdef USER_CRYPTO_MANAGER
    (void)SetPriority();  // set tid priority to 40
    int32_t ret = KeyManager::GetInstance().GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] GetFileEncryptStatus: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
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
        return ret;
    }
    LOGI("[L1:StorageDaemon] GetFileEncryptStatus: <<< EXIT SUCCESS <<< userId=%{public}u, isEncrypted=%{public}d",
        userId, isEncrypted);
    return ret;
#else
    LOGI("[L1:StorageDaemon] GetFileEncryptStatus: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    LOGI("[L1:StorageDaemon] GetUserNeedActiveStatus: >>> ENTER <<< userId=%{public}u", userId);
#ifdef USER_CRYPTO_MIGRATE_KEY
    needActive = IsNeedRestorePathExist(userId, false);
#endif
    LOGI("[L1:StorageDaemon] GetUserNeedActiveStatus: <<< EXIT SUCCESS <<< userId=%{public}u, needActive=%{public}d",
        userId, needActive);
    return E_OK;
}

void StorageDaemon::ActiveAppCloneUserKey()
{
    LOGI("[L1:StorageDaemon] ActiveAppCloneUserKey: >>> ENTER <<<");
#ifdef USER_CRYPTO_MANAGER
    unsigned int failedUserId = 0;
    auto ret = AppCloneKeyManager::GetInstance().ActiveAppCloneUserKey(failedUserId);
    if ((ret != E_OK) && (ret != E_NOT_SUPPORT)) {
        LOGE("[L1:StorageDaemon] ActiveAppCloneUserKey: <<< EXIT FAILED <<< ret=%{public}d", ret);
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
            LOGW("[L1:StorageDaemon] Destroy user %{public}d dirs, ret is: %{public}d", failedUserId, ret);
            ret = KeyManager::GetInstance().DeleteUserKeys(failedUserId);
            LOGW("[L1:StorageDaemon] Delete user %{public}d keys, ret is: %{public}d", failedUserId, ret);
        }
#endif
    }
#endif
    LOGI("[L1:StorageDaemon] ActiveAppCloneUserKey: <<< EXIT SUCCESS <<<");
}

int32_t StorageDaemon::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    LOGI("[L1:StorageDaemon] IsFileOccupied: >>> ENTER <<< path=%{public}s, inputList.size=%{public}zu",
        path.c_str(), inputList.size());
    int32_t ret = MountManager::GetInstance().IsFileOccupied(path, inputList, outputList, isOccupy);
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] IsFileOccupied: <<< EXIT SUCCESS <<< path=%{public}s, isOccupy=%{public}d",
            path.c_str(), isOccupy);
    } else {
        LOGE("[L1:StorageDaemon] IsFileOccupied: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

void StorageDaemon::SetPriority()
{
    LOGI("[L1:StorageDaemon] SetPriority: >>> ENTER <<<");
    int tid = syscall(SYS_gettid);
    if (setpriority(PRIO_PROCESS, tid, PRIORITY_LEVEL) != 0) {
        LOGE("[L1:StorageDaemon] SetPriority: <<< EXIT FAILED <<< failed to set priority");
    } else {
        LOGW("[L1:StorageDaemon] SetPriority: <<< EXIT SUCCESS <<< tid=%{public}d", tid);
    }
}

int32_t StorageDaemon::InactiveUserPublicDirKey(uint32_t userId)
{
    LOGI("[L1:StorageDaemon] InactiveUserPublicDirKey: >>> ENTER <<< userId=%{public}u", userId);
    int32_t ret = E_OK;
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    ret = KeyManagerExt::GetInstance().InActiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] InactiveUserPublicDirKey: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
    }
#endif
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] InactiveUserPublicDirKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    }
    return ret;
}

int32_t StorageDaemon::UpdateUserPublicDirPolicy(uint32_t userId)
{
    LOGI("[L1:StorageDaemon] UpdateUserPublicDirPolicy: >>> ENTER <<< userId=%{public}u", userId);
    int32_t ret = E_OK;
#if defined(USER_CRYPTO_MANAGER) && defined(PC_USER_MANAGER)
    ret = KeyManagerExt::GetInstance().UpdateUserPublicDirPolicy(userId);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] UpdateUserPublicDirPolicy: <<< EXIT FAILED <<< userId=%{public}u, ret=%{public}d",
            userId, ret);
    }
#endif
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] UpdateUserPublicDirPolicy: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    }
    return ret;
}

int StorageDaemon::RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    LOGI("[L1:StorageDaemon] RegisterUeceActivationCallback: >>> ENTER <<<");
#ifdef EL5_FILEKEY_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = KeyManager::GetInstance().RegisterUeceActivationCallback(ueceCallback);
    auto delay = StorageService::StorageRadar::ReportDuration("RegisterUeceActivationCallback",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, StorageService::DEFAULT_USERID);
    LOGE("[L1:StorageDaemon] SD_DURATION: RegisterUeceActivation Callback: ret = %{public}d, delay time =%{public}s",
        ret, delay.c_str());
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] RegisterUeceActivationCallback: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:StorageDaemon] RegisterUeceActivationCallback: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
#else
    LOGD("[L1:StorageDaemon] RegisterUeceActivationCallback: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int StorageDaemon::UnregisterUeceActivationCallback()
{
    LOGI("[L1:StorageDaemon] UnregisterUeceActivationCallback: >>> ENTER <<<");
#ifdef EL5_FILEKEY_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = KeyManager::GetInstance().UnregisterUeceActivationCallback();
    auto delay = StorageService::StorageRadar::ReportDuration("UnregisterUeceActivationCallback",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, StorageService::DEFAULT_USERID);
    LOGE("[L1:StorageDaemon] SD_DURATION:UnregisterUeceActivation Callback: ret = %{public}d, delay time =%{public}s",
        ret, delay.c_str());
    if (ret == E_OK) {
        LOGI("[L1:StorageDaemon] UnregisterUeceActivationCallback: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:StorageDaemon] UnregisterUeceActivationCallback: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
#else
    LOGD("[L1:StorageDaemon] UnregisterUeceActivationCallback: <<< EXIT SUCCESS <<< not supported");
    return E_OK;
#endif
}

int32_t StorageDaemon::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    LOGI("[L1:StorageDaemon] SetDirEncryptionPolicy: >>> ENTER <<< userId=%{public}u, dirPath=%{public}s,"
        "level=%{public}u", userId, dirPath.c_str(), level);
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string extraData = "userId is:" + std::to_string(userId) + ", level is:" + std::to_string(level)
        + ", path is:" + dirPath;
    if (level < EL1_SYS_KEY || level > EL4_USER_KEY) {
        LOGE("[L1:StorageDaemon] SetDirEncryptionPolicy: <<< EXIT FAILED <<< level is wrong=%{public}u", level);
        StorageRadar::ReportCommonResult("SetDirEncryptionPolicy level is failed", userId, E_PARAMS_INVALID, extraData);
        LOGE("[L1:StorageDaemon] level is wrong, level is %{public}u", level);
        return E_PARAMS_INVALID;
    }

    if (level == EL1_SYS_KEY && userId != GLOBAL_USER_ID) {
        LOGE("[L1:StorageDaemon] SetDirEncryptionPolicy: <<< EXIT FAILED <<< SysPolice must be root uid");
        StorageRadar::ReportCommonResult("SetDirEncryptionPolicy setSysPolicy failed", userId, E_PARAMS_INVALID,
            extraData);
        LOGE("[L1:StorageDaemon] SysPolice must be root uid, level is %{public}u", level);
        return E_PARAMS_INVALID;
    }
    StorageService::EncryptionLevel keyLevel = UintToKeyType(level);
    LOGI("[L1:StorageDaemon] user id is : %{public}u, dir path is %{public}s, level is %{public}u",
        userId, dirPath.c_str(), level);
    auto ret = IsDirPathSupport(dirPath);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] SetDirEncryptionPolicy: <<< EXIT FAILED <<< IsDirPathSupport ret=%{public}d", ret);
        StorageRadar::ReportCommonResult("SetDirEncryptionPolicy IsDirPathSupport file failed", userId, ret, extraData);
        LOGE("[L1:StorageDaemon] IsDirPathSupport file failed, errNo %{public}d", ret);
        return ret;
    }

    ret = KeyManager::GetInstance().SetDirEncryptionPolicy(userId, dirPath, keyLevel);
    if (ret != E_OK) {
        LOGE("[L1:StorageDaemon] SetDirEncryptionPolicy: <<< EXIT FAILED <<< SetDirEncryptionPolicy ret=%{public}d",
            ret);
        StorageRadar::ReportCommonResult("StorageDaemon::SetDirEncryptionPolicy", userId, ret, extraData);
        LOGE("[L1:StorageDaemon] VerifyAncoUserDirs file failed, errNo %{public}d", ret);
        return ret;
    }

    LOGI("[L1:StorageDaemon] SetDirEncryptionPolicy: <<< EXIT SUCCESS <<< userId=%{public}u, dirPath=%{public}s",
        userId, dirPath.c_str());
    return ret;
#else
    LOGE("[L1:StorageDaemon] SetDirEncryptionPolicy: <<< EXIT FAILED <<< not supported");
    return E_NOT_SUPPORT;
#endif
}

StorageService::EncryptionLevel StorageDaemon::UintToKeyType(uint32_t type)
{
    LOGI("[L1:StorageDaemon] UintToKeyType: >>> ENTER <<< type=%{public}u", type);
    switch (type) {
        case StorageService::EncryptionLevel::EL1_SYS_KEY:
            return StorageService::EncryptionLevel::EL1_SYS_KEY;
        case StorageService::EncryptionLevel::EL1_USER_KEY:
            LOGI("[L1:StorageDaemon] UintToKeyType: <<< EXIT SUCCESS <<< type=%{public}u -> EL1_USER_KEY", type);
            return StorageService::EncryptionLevel::EL1_USER_KEY;
        case StorageService::EncryptionLevel::EL2_USER_KEY:
            LOGI("[L1:StorageDaemon] UintToKeyType: <<< EXIT SUCCESS <<< type=%{public}u -> EL2_USER_KEY", type);
            return StorageService::EncryptionLevel::EL2_USER_KEY;
        case StorageService::EncryptionLevel::EL3_USER_KEY:
            LOGI("[L1:StorageDaemon] UintToKeyType: <<< EXIT SUCCESS <<< type=%{public}u -> EL3_USER_KEY", type);
            return StorageService::EncryptionLevel::EL3_USER_KEY;
        case StorageService::EncryptionLevel::EL4_USER_KEY:
            LOGI("[L1:StorageDaemon] UintToKeyType: <<< EXIT SUCCESS <<< type=%{public}u -> EL4_USER_KEY", type);
            return StorageService::EncryptionLevel::EL4_USER_KEY;
        default:
            LOGI("[L1:StorageDaemon] UintToKeyType: <<< EXIT SUCCESS <<< type=%{public}u -> EL1_SYS_KEY (default)",
                type);
            return StorageService::EncryptionLevel::EL1_SYS_KEY;
    }
}

int32_t StorageDaemon::IsDirPathSupport(const std::string &dirPath)
{
    LOGI("[L1:StorageDaemon] IsDirPathSupport: >>> ENTER <<< dirPath=%{public}s", dirPath.c_str());
    if (access(dirPath.c_str(), F_OK) != 0) {
        LOGE("[L1:StorageDaemon] IsDirPathSupport: <<< EXIT FAILED <<< dirPath is not exist");
        return E_NON_ACCESS;
    }
    if (!IsDir(dirPath)) {
        LOGE("[L1:StorageDaemon] IsDirPathSupport: <<< EXIT FAILED <<< dirPath is not dir");
        return E_NOT_DIR_PATH;
    }
    if (strncmp(dirPath.c_str(), ANCO_DIR, strlen(ANCO_DIR)) != 0) {
        LOGE("[L1:StorageDaemon] IsDirPathSupport: <<< EXIT FAILED <<< dir is not permission");
        return E_PARAMS_INVALID;
    }
    return E_OK;
}

int32_t StorageDaemon::DoStoreAndUpdate(uint32_t userId,
                                        const std::vector<uint8_t> &token,
                                        const std::vector<uint8_t> &secret,
                                        KeyType keyType)
{
#ifdef USER_CRYPTO_MIGRATE_KEY
    uint64_t secureUid = { 0 };
    if (!KeyManager::GetInstance().GetSecureUid(userId, secureUid)) {
        LOGE("GetSecureUid failed, userId:%{public}u", userId);
    }
    UserTokenSecret userTokenSecret = { token, secret, secret, secureUid };
    auto ret = KeyManager::GetInstance().UpdateUserAuthByKeyType(userId, userTokenSecret, keyType);
    if (ret != E_OK) {
        LOGE("UpdateUserAuth failed, ret:%{public}d, userId:%{public}u, keyType:%{public}u", ret, userId, keyType);
        return ret;
    }
    ret = KeyManager::GetInstance().UpdateKeyContextByKeyType(userId, keyType);
    if (ret != E_OK) {
        LOGE("UpdateKeyContext failed, ret:%{public}d, userId:%{public}u, keyType:%{public}u", ret, userId, keyType);
        return ret;
    }
#endif
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
