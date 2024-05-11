/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>

#ifdef USER_CRYPTO_MANAGER
#include "crypto/anco_key_manager.h"
#include "crypto/key_manager.h"
#endif
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager.h"
#include "volume/volume_manager.h"
#endif
#include "file_sharing/file_sharing.h"
#include "quota/quota_manager.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "user/user_manager.h"
#include "user/mount_manager.h"
#include "utils/file_utils.h"
#include "system_ability_definition.h"
#include "file_share.h"
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

namespace OHOS {
namespace StorageDaemon {
#ifdef DFS_SERVICE
using namespace OHOS::FileManagement::CloudFile;
#endif

constexpr int32_t DEFAULT_VFS_CACHE_PRESSURE = 100;
constexpr int32_t MAX_VFS_CACHE_PRESSURE = 10000;
static const std::string DATA = "/data";
static const std::string VFS_CACHE_PRESSURE = "/proc/sys/vm/vfs_cache_pressure";
const std::string DATA_SERVICE_EL2 = "/data/service/el2/";
const std::string DATA_SERVICE_EL3 = "/data/service/el3/";
const std::string DATA_SERVICE_EL4 = "/data/service/el4/";
const std::string CONFIG_FILE_PATH = "/data/virt_service/rgm_manager/rgm_hmos/config/storage/direnc.json";
const std::string USER_PATH = "/data/app/el1/100";
const std::string ANCO_TYPE_SYS_EL1 = "encryption=Require_Sys_EL1";
const std::string ANCO_TYPE_USER_EL1 = "encryption=Require_User_EL1";
const std::string ANCO_TYPE_USER_EL2 = "encryption=Require_User_EL2";

typedef int32_t (*CreateShareFileFunc)(const std::vector<std::string> &, uint32_t, uint32_t, std::vector<int32_t> &);
typedef int32_t (*DeleteShareFileFunc)(uint32_t, const std::vector<std::string> &);

int32_t StorageDaemon::Shutdown()
{
    return E_OK;
}

int32_t StorageDaemon::Mount(std::string volId, uint32_t flags)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Mount");
    return VolumeManager::Instance()->Mount(volId, flags);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UMount(std::string volId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle UMount");
    return VolumeManager::Instance()->UMount(volId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::Check(std::string volId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Check");
    return VolumeManager::Instance()->Check(volId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::Format(std::string volId, std::string fsType)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Format");
    return VolumeManager::Instance()->Format(volId, fsType);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::Partition(std::string diskId, int32_t type)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Partition");
    return DiskManager::Instance()->HandlePartition(diskId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::SetVolumeDescription(std::string volId, std::string description)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle SetVolumeDescription");
    return VolumeManager::Instance()->SetVolumeDescription(volId, description);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GetCryptoFlag(KeyType type, uint32_t &flags)
{
    switch (type) {
        case EL1_KEY:
            flags = IStorageDaemon::CRYPTO_FLAG_EL1;
            return E_OK;
        case EL2_KEY:
            flags = IStorageDaemon::CRYPTO_FLAG_EL2;
            return E_OK;
        case EL3_KEY:
            flags = IStorageDaemon::CRYPTO_FLAG_EL3;
            return E_OK;
        case EL4_KEY:
            flags = IStorageDaemon::CRYPTO_FLAG_EL4;
            return E_OK;
        default:
            LOGE("GetCryptoFlag error, type = %{public}u", type);
            return E_KEY_TYPE_INVAL;
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
        default:
            LOGE("GetNeedRestoreFilePathByType key type error, type = %{public}u", type);
            return "";
    }
}

int32_t StorageDaemon::RestoreUserOneKey(int32_t userId, KeyType type)
{
    uint32_t flags = 0;
    int32_t ret;

    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }

    std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
    if (elNeedRestorePath == "") {
        return E_KEY_TYPE_INVAL;
    }

    if (std::filesystem::exists(elNeedRestorePath)) {
        LOGI("start restore User %{public}u el%{public}u", userId, type);
        ret = KeyManager::GetInstance()->RestoreUserKey(userId, type);
        if (ret != E_OK) {
            if (type != EL1_KEY) {
                LOGE("userId %{public}u type %{public}u restore key failed, but return success, error = %{public}d",
                     userId, type, ret);
                return E_OK; // maybe need user key, so return E_OK to continue
            }
            LOGE("RestoreUserKey EL1_KEY failed, error = %{public}d, userId %{public}u", ret, userId);
            return ret;
        }

        ret = UserManager::GetInstance()->PrepareUserDirs(userId, flags);
        if (ret != E_OK) {
            LOGE("PrepareUserDirs failed, userId %{public}u, flags %{public}u, error %{public}d", userId, flags, ret);
            return ret;
        }
        (void)remove(elNeedRestorePath.c_str());
        if (type == EL4_KEY) {
            UserManager::GetInstance()->CreateBundleDataDir(userId);
        }
        LOGI("restore User %{public}u el%{public}u success", userId, type);
    }
    return E_OK;
}

int32_t StorageDaemon::RestoreUserKey(int32_t userId, uint32_t flags)
{
    LOGI("prepare restore user dirs for %{public}d, flags %{public}u", userId, flags);
    int32_t ret = E_OK;

    if (!IsNeedRestorePathExist(userId, true)) {
        LOGE("need_restore file is not existed");
        return -EEXIST;
    }

    std::vector<KeyType> type = {EL1_KEY, EL2_KEY, EL3_KEY, EL4_KEY};
    for (unsigned long i = 0; i < type.size(); i++) {
        ret = RestoreUserOneKey(userId, type[i]);
        if (ret != E_OK) {
            return ret;
        }
    }

    return E_OK;
}
#endif

int32_t StorageDaemon::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    //CRYPTO_FLAG_EL3 create el3,  CRYPTO_FLAG_EL4 create el4
    flags = flags | IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4;
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (ret == -EEXIST) {
        return RestoreUserKey(userId, flags);
    }
#endif
    if (ret != E_OK) {
        LOGE("Generate user %{public}d key error", userId);
        return ret;
    }
#endif

    return UserManager::GetInstance()->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemon::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    //CRYPTO_FLAG_EL3 destroy el3,  CRYPTO_FLAG_EL4 destroy el4
    flags = flags | IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4;
    int32_t ret = UserManager::GetInstance()->DestroyUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGW("Destroy user %{public}d dirs failed, please check", userId);
    }

#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->DeleteUserKeys(userId);
#else
    return ret;
#endif
}

int32_t StorageDaemon::StartUser(int32_t userId)
{
    return UserManager::GetInstance()->StartUser(userId);
}

int32_t StorageDaemon::StopUser(int32_t userId)
{
    return UserManager::GetInstance()->StopUser(userId);
}

int32_t StorageDaemon::InitGlobalKey(void)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->InitGlobalDeviceKey();
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::InitGlobalUserKeys(void)
{
#ifdef USER_FILE_SHARING
    // File sharing depends on the /data/service/el1/public be decrypted.
    // A hack way to prepare the sharing dir, move it to callbacks after the parameter ready.
    if (SetupFileSharingDir() == -1) {
        LOGE("Failed to set up the directory for file sharing");
    }
#endif

#ifdef USER_CRYPTO_MANAGER
    int ret = KeyManager::GetInstance()->InitGlobalUserKeys();
    if (ret) {
        LOGE("Init global users els failed");
        return ret;
    }
#endif
    auto result = UserManager::GetInstance()->PrepareUserDirs(GLOBAL_USER_ID, CRYPTO_FLAG_EL1);
#ifdef USER_CRYPTO_MANAGER
    AncoInitCryptKey();
#endif
    return result;
}

int32_t StorageDaemon::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::DeleteUserKeys(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->DeleteUserKeys(userId);
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
    return KeyManager::GetInstance()->UpdateUserAuth(userId, userTokenSecret);
#else
    return E_OK;
#endif
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int32_t StorageDaemon::PrepareUserDirsAndUpdateUserAuth(uint32_t userId, KeyType type,
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGI("start userId %{public}u KeyType %{public}u", userId, type);
    int32_t ret = E_OK;
    uint32_t flags = 0;

    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }

    ret = KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, type, token, {'!'});
    if (ret != E_OK) {
        LOGE("Active user %{public}u key fail, type %{public}u, flags %{public}u", userId, type, flags);
        return ret;
    }

    UserTokenSecret userTokenSecret = {.token = token, .oldSecret = {'!'}, .newSecret = secret, .secureUid = 0};
    ret = KeyManager::GetInstance()->UpdateCeEceSeceUserAuth(userId, userTokenSecret, type, false);
    if (ret != E_OK) {
        return ret;
    }

    ret = KeyManager::GetInstance()->UpdateCeEceSeceKeyContext(userId, type);
    if (ret != E_OK) {
        return ret;
    }

    LOGI("try to destory dir first, user %{public}u, flags %{public}u", userId, flags);
    (void)UserManager::GetInstance()->DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance()->PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        return ret;
    }
    LOGI("userId %{public}u type %{public}u sucess", userId, type);
    return E_OK;
}

bool StorageDaemon::IsNeedRestorePathExist(uint32_t userId, bool needCheckEl1)
{
    std::string el2NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL2_DIR);
    std::string el3NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL3_DIR);
    std::string el4NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL4_DIR);
    bool isExist = std::filesystem::exists(el2NeedRestorePath) ||
                   std::filesystem::exists(el3NeedRestorePath) ||
                   std::filesystem::exists(el4NeedRestorePath);
    if (needCheckEl1) {
        std::string el1NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL1_DIR);
        isExist = isExist || std::filesystem::exists(el1NeedRestorePath);
    }
    return isExist;
}
#endif

int32_t StorageDaemon::GenerateKeyAndPrepareUserDirs(uint32_t userId, KeyType type,
                                                     const std::vector<uint8_t> &token,
                                                     const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    int ret;
    uint32_t flags = 0;

    LOGI("enter:");
    ret = KeyManager::GetInstance()->GenerateUserKeyByType(userId, type, token, secret);
    if (ret != E_OK) {
        LOGE("upgrade scene:generate user key fail, userId %{public}u, KeyType %{public}u, sec empty %{public}d",
             userId, type, secret.empty());
        return ret;
    }
    ret = GetCryptoFlag(type, flags);
    if (ret != E_OK) {
        return ret;
    }
    (void)UserManager::GetInstance()->DestroyUserDirs(userId, flags);
    ret = UserManager::GetInstance()->PrepareUserDirs(userId, flags);
    if (ret != E_OK) {
        LOGE("upgrade scene:prepare user dirs fail, userId %{public}u, flags %{public}u, sec empty %{public}d",
             userId, flags, secret.empty());
    }

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
    LOGI("ActiveUserKey with type %{public}u enter", type);
    int ret = KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, type, token, secret);
    if (ret != E_OK && ret != -ENOENT) {
#ifdef USER_CRYPTO_MIGRATE_KEY
        std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
        if (std::filesystem::exists(elNeedRestorePath) && (!token.empty() || !secret.empty())) {
            LOGI("start PrepareUserDirsAndUpdateUserAuth userId %{public}u, type %{public}u", userId, type);
            ret = PrepareUserDirsAndUpdateUserAuth(userId, type, token, secret);
        }
#endif
        if (ret != E_OK) {
            LOGE("active and restore fail, ret %{public}d, userId %{public}u, type %{public}u sec empty %{public}d",
                 ret, userId, type, secret.empty());
            return ret;
        }
    } else if (ret == -ENOENT) {
        LOGI("start GenerateKeyAndPrepareUserDirs userId %{public}u, type %{public}u sec empty %{public}d",
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

int32_t StorageDaemon::ActiveUserKey(uint32_t userId,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret)
{
    bool updateFlag = false;
#ifdef USER_CRYPTO_MANAGER
    LOGI("userId %{public}u, tok empty %{public}d sec empty %{public}d", userId, token.empty(), secret.empty());
    int ret = KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, EL2_KEY, token, secret);
    if (ret != E_OK) {
#ifdef USER_CRYPTO_MIGRATE_KEY
        LOGI("migrate, userId %{public}u, tok empty %{public}d sec empty %{public}d",
             userId, token.empty(), secret.empty());
        std::string el2NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL2_DIR);
        if (std::filesystem::exists(el2NeedRestorePath) && (!token.empty() || !secret.empty())) {
            updateFlag = true;
            ret = PrepareUserDirsAndUpdateUserAuth(userId, EL2_KEY, token, secret);
        }
#endif
        if (ret != E_OK) {
            LOGE("ActiveUserKey fail, userId %{public}u, type %{public}u, tok empty %{public}d sec empty %{public}d",
                 userId, EL2_KEY, token.empty(), secret.empty());
            return ret;
        }
    }

    ret = ActiveUserKeyAndPrepare(userId, EL3_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKey fail, userId %{public}u, type %{public}u", userId, EL3_KEY);
        return ret;
    }

    ret = ActiveUserKeyAndPrepare(userId, EL4_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKey fail, userId %{public}u, type %{public}u", userId, EL4_KEY);
        return ret;
    }
    RestoreconElX(userId);
    if (updateFlag) {
        UserManager::GetInstance()->CreateBundleDataDir(userId);
    }

    AncoActiveCryptKey(userId);
    return ret;
#else
    RestoreconElX(userId);
    if (updateFlag) {
        UserManager::GetInstance()->CreateBundleDataDir(userId);
    }
#ifdef USER_CRYPTO_MANAGER
    AncoActiveCryptKey(userId);
#endif
    return E_OK;
#endif
}

int32_t StorageDaemon::RestoreconElX(uint32_t userId)
{
#ifdef USE_LIBRESTORECON
    LOGI("Begin to restorecon path, userId = %{public}d", userId);
    RestoreconRecurse((DATA_SERVICE_EL2 + "public").c_str());
    const std::string &path = DATA_SERVICE_EL2 + std::to_string(userId);
    LOGI("RestoreconRecurse el2 public end, userId = %{public}d", userId);
    MountManager::GetInstance()->RestoreconSystemServiceDirs(userId);
    LOGI("RestoreconSystemServiceDirs el2 end, userId = %{public}d", userId);
    RestoreconRecurse((DATA_SERVICE_EL2 + std::to_string(userId) + "/share").c_str());
    LOGI("RestoreconRecurse el2 share end, userId = %{public}d", userId);
    const std::string &DATA_SERVICE_EL2_HMDFS = DATA_SERVICE_EL2 + std::to_string(userId) + "/hmdfs/";
    Restorecon(DATA_SERVICE_EL2_HMDFS.c_str());
    LOGI("Restorecon el2 DATA_SERVICE_EL2_HMDFS end, userId = %{public}d", userId);
    const std::string &ACCOUNT_FILES = "/hmdfs/account/files/";
    const std::string &EL2_HMDFS_ACCOUNT_FILES = DATA_SERVICE_EL2 + std::to_string(userId) + ACCOUNT_FILES;
    Restorecon(EL2_HMDFS_ACCOUNT_FILES.c_str());
    LOGI("Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES end, userId = %{public}d", userId);
    const std::string &FILES_RECENT = "/hmdfs/account/files/.Recent";
    const std::string &EL2_HMDFS_ACCOUNT_FILES_RECENT = DATA_SERVICE_EL2 + std::to_string(userId) + FILES_RECENT;
    Restorecon(EL2_HMDFS_ACCOUNT_FILES_RECENT.c_str());
    LOGI("Restorecon el2 DATA_SERVICE_EL2_HMDFS_ACCOUNT_FILES_RECENT end, userId = %{public}d", userId);
#endif
    return E_OK;
}

int32_t StorageDaemon::InactiveUserKey(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->InActiveUserKey(userId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::LockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->LockUserScreen(userId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UnlockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->UnlockUserScreen(userId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->GetLockScreenStatus(userId, lockScreenStatus);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->GenerateAppkey(userId, appUid, keyId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::DeleteAppkey(uint32_t userId, const std::string keyId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->DeleteAppkey(userId, keyId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateKeyContext(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->UpdateKeyContext(userId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::MountCryptoPathAgain(uint32_t userId)
{
    LOGI("begin to MountCryptoPathAgain");
#ifdef USER_CRYPTO_MANAGER
    return MountManager::GetInstance()->MountCryptoPathAgain(userId);
#else
    return E_OK;
#endif
}

std::vector<int32_t> StorageDaemon::CreateShareFile(const std::vector<std::string> &uriList,
                                                    uint32_t tokenId, uint32_t flag)
{
    LOGI("Create Share file list len is %{public}zu", uriList.size());
    std::vector<int32_t> retList;
    AppFileService::FileShare::CreateShareFile(uriList, tokenId, flag, retList);
    return retList;
}

int32_t StorageDaemon::DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &uriList)
{
    int32_t ret = AppFileService::FileShare::DeleteShareFile(tokenId, uriList);
    return ret;
}

int32_t StorageDaemon::SetBundleQuota(const std::string &bundleName, int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    return QuotaManager::GetInstance()->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageDaemon::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    return QuotaManager::GetInstance()->GetOccupiedSpace(idType, id, size);
}

int32_t StorageDaemon::GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
    const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes)
{
    return QuotaManager::GetInstance()->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes,
        pkgFileSizes);
}

int32_t StorageDaemon::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageDaemon::MountDfsDocs start.");
    return MountManager::GetInstance()->MountDfsDocs(userId, relativePath, networkId, deviceId);
}

static bool ReadFileToString(const std::string& pathInst, std::string& oldContent)
{
    std::fstream fd;
    fd.open(pathInst.c_str(), std::ios::in);
    if (!fd.is_open()) {
        LOGE("open fail!");
        return false;
    }
    // Get Old data
    std::getline(fd, oldContent);
    LOGE("StorageDaemon::ReadFileToString %{public}s", oldContent.c_str());
    fd.close();
    return true;
}

static bool SaveStringToFile(const std::string& pathInst, const std::string& content)
{
    std::fstream fd;
    fd.open(pathInst.c_str(), std::ios::out);
    if (!fd.is_open()) {
        LOGE("open fail!");
        return false;
    }
    LOGI("StorageDaemon::SaveStringToFile %{public}s", content.c_str());
    // Write New data
    fd << content;
    fd.close();
    return true;
}

int32_t StorageDaemon::UpdateMemoryPara(int32_t size, int32_t &oldSize)
{
    LOGI("StorageDaemon::UpdateMemoryPara");
    if (size > MAX_VFS_CACHE_PRESSURE || size < 0) {
        LOGE("size is invalid");
        return E_NOT_SUPPORT;
    }
    // Get old data
    std::string oldContent;
    if (!ReadFileToString(VFS_CACHE_PRESSURE, oldContent)) {
        LOGE("Failed to read");
    }
    if (!oldContent.empty()) {
        try {
            oldSize = std::stoi(oldContent);
        } catch (...) {
            LOGE("StorageDaemon:UpdateMemoryPara invalid old memory size");
            return E_SYS_CALL;
        }
    } else {
        oldSize = DEFAULT_VFS_CACHE_PRESSURE;
    }
    // Update new data
    if (!SaveStringToFile(VFS_CACHE_PRESSURE, std::to_string(size))) {
        LOGE("Failed to write");
        return E_SYS_CALL;
    }
    return E_OK;
}

void StorageDaemon::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
                                                                          const std::string &deviceId)
{
    LOGI("SystemAbilityId:%{public}d", systemAbilityId);
#ifdef EXTERNAL_STORAGE_MANAGER
    if (systemAbilityId == ACCESS_TOKEN_MANAGER_SERVICE_ID) {
        DiskManager::Instance()->ReplayUevent();
    }
#endif
    if (systemAbilityId == FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID) {
        MountManager::GetInstance()->SetCloudState(true);
    }
}

void StorageDaemon::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(int32_t systemAbilityId,
                                                                             const std::string &deviceId)
{
    LOGI("SystemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId == FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID) {
        MountManager::GetInstance()->SetCloudState(false);
    }
}

void StorageDaemon::AncoInitCryptKey()
{
#ifdef USER_CRYPTO_MANAGER
    std::error_code errorCode;
    if (std::filesystem::exists(CONFIG_FILE_PATH, errorCode)) {
        auto ret = AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(CONFIG_FILE_PATH, ANCO_TYPE_SYS_EL1,
                                                                           GLOBAL_USER_ID);
        if (ret != E_OK) {
            LOGE("SetAncoDirectoryElPolicy failed, ret = %{public}d", ret);
        }
        if (std::filesystem::exists(USER_PATH, errorCode)) {
            ret = AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(CONFIG_FILE_PATH, ANCO_TYPE_USER_EL1,
                                                                          ANCO_USER_ID);
            if (ret != E_OK) {
                LOGE("SetAncoDirectoryElPolicy failed, ret = %{public}d", ret);
            }
        }
    }
#endif
}

void StorageDaemon::AncoActiveCryptKey(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    std::error_code errorCode;
    if (std::filesystem::exists(CONFIG_FILE_PATH, errorCode)) {
        auto ret = AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(CONFIG_FILE_PATH, ANCO_TYPE_USER_EL2,
                                                                           userId);
        if (ret != E_OK) {
            LOGE("SetAncoDirectoryElPolicy failed, ret = %{public}d", ret);
        }
    }
#endif
}
} // namespace StorageDaemon
} // namespace OHOS
