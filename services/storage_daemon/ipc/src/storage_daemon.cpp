/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <thread>
#include "file_ex.h"
#include "hi_audit.h"
#include "hisysevent.h"
#include "utils/set_flag_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"

#ifdef USER_CRYPTO_MANAGER
#include "crypto/app_clone_key_manager.h"
#include "crypto/iam_client.h"
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

using namespace OHOS::StorageService;
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
const std::string DATA_SERVICE_EL1_PUBLIC_STORAGE_DAEMON_SD = "/data/service/el1/public/storage_daemon/sd";
const std::string DATA_SERVICE_EL0_STORAGE_DAEMON_SD = "/data/service/el0/storage_daemon/sd";
const std::string NEED_RESTORE_SUFFIX = "/latest/need_restore";
const std::string NEW_DOUBLE_2_SINGELE = "2";

typedef int32_t (*CreateShareFileFunc)(const std::vector<std::string> &, uint32_t, uint32_t, std::vector<int32_t> &);
typedef int32_t (*DeleteShareFileFunc)(uint32_t, const std::vector<std::string> &);

int32_t StorageDaemon::Shutdown()
{
    return E_OK;
}

int32_t StorageDaemon::Mount(const std::string &volId, uint32_t flags)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Mount");
    int32_t ret = VolumeManager::Instance()->Mount(volId, flags);
    if (ret != E_OK) {
        LOGW("Mount failed, please check");
        StorageRadar::ReportVolumeOperation("VolumeManager::Mount", ret);
        AuditLog storageAuditLog = { false, "FAILED TO Mount", "ADD", "Mount", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { false, "SUCCESS TO Mount", "ADD", "Mount", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UMount(const std::string &volId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle UMount");
    int32_t ret = VolumeManager::Instance()->UMount(volId);
    if (ret != E_OK) {
        LOGW("UMount failed, please check");
        StorageRadar::ReportVolumeOperation("VolumeManager::UMount", ret);
        AuditLog storageAuditLog = { false, "FAILED TO UMount", "DEL", "UMount", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { false, "SUCCESS TO UMount", "DEL", "UMount", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::Check(const std::string &volId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Check");
    return VolumeManager::Instance()->Check(volId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::Format(const std::string &volId, const std::string &fsType)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Format");
    int32_t ret = VolumeManager::Instance()->Format(volId, fsType);
    if (ret != E_OK) {
        LOGW("Format failed, please check");
        StorageRadar::ReportVolumeOperation("VolumeManager::Format", ret);
        AuditLog storageAuditLog = { true, "FAILED TO Format", "UPDATE", "Format", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { true, "SUCCESS TO Format", "UPDATE", "Format", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::Partition(const std::string &diskId, int32_t type)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Partition");
    int32_t ret = DiskManager::Instance()->HandlePartition(diskId);
    if (ret != E_OK) {
        LOGW("HandlePartition failed, please check");
        StorageRadar::ReportVolumeOperation("VolumeManager::Partition", ret);
        AuditLog storageAuditLog = { true, "FAILED TO Partition", "UPDATE", "Partition", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { true, "SUCCESS TO Partition", "UPDATE", "Partition", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::SetVolumeDescription(const std::string &volId, const std::string &description)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle SetVolumeDescription");
    int32_t ret = VolumeManager::Instance()->SetVolumeDescription(volId, description);
    if (ret != E_OK) {
        LOGW("SetVolumeDescription failed, please check");
        StorageRadar::ReportVolumeOperation("VolumeManager::SetVolumeDescription", ret);
        AuditLog storageAuditLog = { true,  "FAILED TO SetVolumeDescription", "UPDATE", "SetVolumeDescription", 1,
            "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { true, "SUCCESS TO SetVolumeDescription", "UPDATE", "SetVolumeDescription", 1,
            "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
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
        case EL5_KEY:
            flags = IStorageDaemon::CRYPTO_FLAG_EL5;
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
        return E_KEY_TYPE_INVAL;
    }

    std::error_code errCode;
    if (!std::filesystem::exists(elNeedRestorePath, errCode)) {
        return E_OK;
    }
    LOGI("start restore User %{public}u el%{public}u", userId, type);
    ret = KeyManager::GetInstance()->RestoreUserKey(userId, type);
    if (ret != E_OK) {
        if (type != EL1_KEY) {
            LOGE("userId %{public}u type %{public}u restore key failed, but return success, error = %{public}d",
                userId, type, ret);
            return E_MIGRETE_ELX_FAILED; // maybe need user key, so return E_OK to continue
        }
        LOGE("RestoreUserKey EL1_KEY failed, error = %{public}d, userId %{public}u", ret, userId);
        return ret;
    }

    ret = UserManager::GetInstance()->PrepareUserDirs(userId, flags);
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
    if (type == EL4_KEY) {
        UserManager::GetInstance()->CreateBundleDataDir(userId);
    }
    LOGI("restore User %{public}u el%{public}u success", userId, type);

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
    for (KeyType type : keyTypes) {
        auto ret = RestoreOneUserKey(userId, type);
        if (ret == E_MIGRETE_ELX_FAILED) {
            LOGE("Try restore user: %{public}d type: %{public}d migrate key, wait user pin !", userId, type);
            break;
        }
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
    flags = flags | IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4 | IStorageDaemon::CRYPTO_FLAG_EL5;
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (ret == -EEXIST) {
        AuditLog storageAuditLog = { false, "FAILED TO GenerateUserKeys", "ADD", "GenerateUserKeys", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
        return RestoreUserKey(userId, flags);
    }
#endif
    if (ret != E_OK) {
        LOGE("Generate user %{public}d key error", userId);
        StorageRadar::ReportUserManager("PrepareUserDirs::UserManager::GenerateUserKeys", userId, ret,
            BizStage::BIZ_STAGE_PREPARE_ADD_USER);
        AuditLog storageAuditLog = { false, "FAILED TO GenerateUserKeys", "ADD", "GenerateUserKeys", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
        return ret;
    }
#endif
    int32_t prepareRet = UserManager::GetInstance()->PrepareUserDirs(userId, flags);
    MountManager::GetInstance()->PrepareAppdataDir(userId);
    return prepareRet;
}

int32_t StorageDaemon::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    int32_t errCode = 0;
    //CRYPTO_FLAG_EL3 destroy el3,  CRYPTO_FLAG_EL4 destroy el4
    flags = flags | IStorageDaemon::CRYPTO_FLAG_EL3 | IStorageDaemon::CRYPTO_FLAG_EL4 | IStorageDaemon::CRYPTO_FLAG_EL5;
    int32_t destroyUserRet = UserManager::GetInstance()->DestroyUserDirs(userId, flags);
    if (destroyUserRet != E_OK) {
        errCode = destroyUserRet;
        LOGW("Destroy user %{public}d dirs failed, please check", userId);
        StorageRadar::ReportUserManager("DestroyUserDirs", userId, errCode, BizStage::BIZ_STAGE_REMOVE_USER);
        AuditLog storageAuditLog = { false, "FAILED TO DestroyUserDirs", "DEL", "DestroyUserDirs", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }

#ifdef USER_CRYPTO_MANAGER
    destroyUserRet = KeyManager::GetInstance()->DeleteUserKeys(userId);
    if (destroyUserRet != E_OK) {
        errCode = destroyUserRet;
        LOGW("DeleteUserKeys failed, please check");
        StorageRadar::ReportUserManager("DestroyUserDirs::DeleteUserKeys", userId, errCode,
            BizStage::BIZ_STAGE_REMOVE_USER);
        AuditLog storageAuditLog = { false, "FAILED TO DeleteUserKeys", "DEL", "DeleteUserKeys", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return errCode;
#else
    return errCode;
#endif
}

int32_t StorageDaemon::StartUser(int32_t userId)
{
    int32_t ret = UserManager::GetInstance()->StartUser(userId);
    if (ret != E_OK && ret != E_KEY_NOT_ACTIVED) {
        LOGE("StartUser failed, please check");
        StorageRadar::ReportUserManager("StartUser", userId, ret, BizStage::BIZ_STAGE_START_USER);
        AuditLog storageAuditLog = { false, "FAILED TO StartUser", "ADD", "StartUser", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { false, "SUCCESS TO StartUser", "ADD", "StartUser", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
}

int32_t StorageDaemon::StopUser(int32_t userId)
{
    int32_t ret = UserManager::GetInstance()->StopUser(userId);
    if (ret != E_OK) {
        LOGE("StopUser failed, please check");
        StorageRadar::ReportUserManager("StopUser", userId, ret, BizStage::BIZ_STAGE_STOP_USER);
        AuditLog storageAuditLog = { false, "FAILED TO StopUser", "DEL", "StopUser", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { false, "SUCCESS TO StopUser", "DEL", "StopUser", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
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
    int ret = KeyManager::GetInstance()->InitGlobalDeviceKey();
    if (ret != E_OK) {
        LOGE("InitGlobalDeviceKey failed, please check");
        StorageRadar::ReportInitGlobalKey("InitGlobalKey", 0, ret, "EL1");
        AuditLog storageAuditLog = { false, "FAILED TO InitGlobalDeviceKey", "ADD", "InitGlobalDeviceKey", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
#ifdef USE_LIBRESTORECON
    RestoreconRecurse(DATA_SERVICE_EL0_STORAGE_DAEMON_SD.c_str());
#endif
    return ret;
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

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string el2NeedRestorePath = GetNeedRestoreFilePath(START_USER_ID, USER_EL2_DIR);
    if (std::filesystem::exists(el2NeedRestorePath, errCode)) {
        LOGE("USER_EL2_DIR is exist, update NEW_DOUBLE_2_SINGLE");
        std::string EL0_NEED_RESTORE = DATA_SERVICE_EL0_STORAGE_DAEMON_SD + NEED_RESTORE_SUFFIX;
        if (!SaveStringToFile(EL0_NEED_RESTORE, NEW_DOUBLE_2_SINGELE)) {
            LOGE("Save NEW_DOUBLE_2_SINGELE file failed");
            return false;
        }
    }
#endif

    int ret = KeyManager::GetInstance()->InitGlobalUserKeys();
    if (ret) {
        LOGE("Init global users els failed");
        StorageRadar::ReportInitGlobalKey("InitGlobalUserKeys", GLOBAL_USER_ID, ret, "EL1");
        AuditLog storageAuditLog = { false, "FAILED TO InitGlobalUserKeys", "ADD", "InitGlobalUserKeys", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
        return ret;
    }
#endif
#ifdef USE_LIBRESTORECON
    RestoreconRecurse(DATA_SERVICE_EL1_PUBLIC_STORAGE_DAEMON_SD.c_str());
#endif
    auto result = UserManager::GetInstance()->PrepareUserDirs(GLOBAL_USER_ID, CRYPTO_FLAG_EL1);
    if (result != E_OK) {
        LOGE("PrepareUserDirs failed, please check");
        StorageRadar::ReportInitGlobalKey("InitGlobalUserKeys::PrepareUserDirs", GLOBAL_USER_ID, result, "EL1");
        AuditLog storageAuditLog = { false, "FAILED TO PrepareUserDirs", "ADD", "PrepareUserDirs", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    MountManager::GetInstance()->PrepareAppdataDir(GLOBAL_USER_ID);
    std::thread thread([this]() { SetDeleteFlag4KeyFiles(); });
    thread.detach();
    return result;
}

void StorageDaemon::SetDeleteFlag4KeyFiles()
{
    StorageService::SetFlagUtils::ParseDirAllPath();
}

int32_t StorageDaemon::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
    if (ret != E_OK) {
        LOGE("GenerateUserKeys failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "GenerateUserKeys",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_GENERATE_USER_KEYS,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { false, "FAILED TO GenerateUserKeys", "ADD", "GenerateUserKeys", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::DeleteUserKeys(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->DeleteUserKeys(userId);
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
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { false, "FAILED TO DeleteUserKeys", "DEL", "DeleteUserKeys", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
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
    int32_t ret = KeyManager::GetInstance()->UpdateUserAuth(userId, userTokenSecret);
    if (ret != E_OK) {
        LOGE("UpdateUserAuth failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "UpdateUserAuth",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_UPDATE_USER_AUTH,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { false, "FAILED TO UpdateUserAuth", "CP", "UpdateUserAuth", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
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
                                                    std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("begin to UpdateUseAuthWithRecoveryKey");
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
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

    uint64_t secureUid = { 0 };
    if (!IamClient::GetInstance().GetSecureUid(userId, secureUid)) {
        LOGE("Get secure uid form iam failed, use default value.");
    }
    UserTokenSecret userTokenSecret = { .token = token, .oldSecret = {'!'}, .newSecret = secret,
                                        .secureUid = secureUid };
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
    if (flags == IStorageDaemon::CRYPTO_FLAG_EL2) {
        PrepareUeceDir(userId);
    }
    LOGI("userId %{public}u type %{public}u sucess", userId, type);
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
    int32_t ret = UserManager::GetInstance()->DestroyUserDirs(userId, IStorageDaemon::CRYPTO_FLAG_EL5);
    LOGI("delete user %{public}u uece %{public}u, ret %{public}d", userId, IStorageDaemon::CRYPTO_FLAG_EL5, ret);
    ret = UserManager::GetInstance()->PrepareUserDirs(userId, IStorageDaemon::CRYPTO_FLAG_EL5);
    LOGI("prepare user %{public}u uece %{public}u, ret %{public}d", userId, IStorageDaemon::CRYPTO_FLAG_EL5, ret);
    return ret;
}
#endif

int32_t StorageDaemon::GenerateKeyAndPrepareUserDirs(uint32_t userId, KeyType type,
                                                     const std::vector<uint8_t> &token,
                                                     const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret;
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
    std::string keyUeceDir = UECE_DIR + "/" + std::to_string(userId);
    if ((flags & IStorageDaemon::CRYPTO_FLAG_EL5) && IsDir(keyUeceDir) && !std::filesystem::is_empty(keyUeceDir)) {
        LOGE("uece has already create, do not need create !");
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
        std::error_code errCode;
        std::string elNeedRestorePath = GetNeedRestoreFilePathByType(userId, type);
        if ((!token.empty() || !secret.empty()) && std::filesystem::exists(elNeedRestorePath, errCode)) {
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

int32_t StorageDaemon::ActiveUserKeyAndPrepareElX(uint32_t userId,
                                                  const std::vector<uint8_t> &token,
                                                  const std::vector<uint8_t> &secret)
{
#ifdef USER_CRYPTO_MANAGER
    int ret = ActiveUserKeyAndPrepare(userId, EL3_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u, type %{public}u", userId, EL3_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL3");
        AuditLog storageAuditLog = { false, "FAILED TO ActiveUserKeyAndPrepare", "ADD", "ActiveUserKeyAndPrepare", 1,
            "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
        return ret;
    }
    ret = ActiveUserKeyAndPrepare(userId, EL4_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u, type %{public}u", userId, EL4_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL4");
        AuditLog storageAuditLog = { false, "FAILED TO ActiveUserKeyAndPrepare", "ADD", "ActiveUserKeyAndPrepare", 1,
            "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
        return ret;
    }
    ret = ActiveUserKeyAndPrepare(userId, EL5_KEY, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u, type %{public}u", userId, EL5_KEY);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKeyAndPrepare", userId, ret, "EL5");
        AuditLog storageAuditLog = { false, "FAILED TO ActiveUserKeyAndPrepare", "ADD", "ActiveUserKeyAndPrepare", 1,
            "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
        return ret;
    }
#endif
    return E_OK;
}

int32_t StorageDaemon::ActiveUserKey(uint32_t userId,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret)
{
    int ret = E_OK;
    bool updateFlag = false;
#ifdef USER_CRYPTO_MANAGER
    LOGI("userId %{public}u, tok empty %{public}d sec empty %{public}d", userId, token.empty(), secret.empty());
    ret = KeyManager::GetInstance()->ActiveCeSceSeceUserKey(userId, EL2_KEY, token, secret);
    if (ret != E_OK) {
#ifdef USER_CRYPTO_MIGRATE_KEY
        LOGI("Migrate usrId %{public}u, Emp_tok %{public}d Emp_sec %{public}d", userId, token.empty(), secret.empty());
        std::error_code errCode;
        std::string el2NeedRestorePath = GetNeedRestoreFilePath(userId, USER_EL2_DIR);
        if (std::filesystem::exists(el2NeedRestorePath, errCode) && (!token.empty() || !secret.empty())) {
            updateFlag = true;
            ret = PrepareUserDirsAndUpdateUserAuth(userId, EL2_KEY, token, secret);
            std::string EL0_NEED_RESTORE = DATA_SERVICE_EL0_STORAGE_DAEMON_SD + NEED_RESTORE_SUFFIX;
            if (!SaveStringToFile(EL0_NEED_RESTORE, NEW_DOUBLE_2_SINGELE)) {
                LOGE("Save key type file failed");
                return false;
            }
        }
#endif
        if (ret != E_OK) {
            LOGE("ActiveUserKey fail, userId %{public}u, type %{public}u, tok empty %{public}d sec empty %{public}d",
                 userId, EL2_KEY, token.empty(), secret.empty());
            if (!token.empty() && !secret.empty()) {
                StorageRadar::ReportActiveUserKey("ActiveUserKey::ActiveUserKey", userId, ret, "EL2");
            }
            return E_ACTIVE_EL2_FAILED;
        }
    }
    ret = ActiveUserKeyAndPrepareElX(userId, token, secret);
    if (ret != E_OK) {
        LOGE("ActiveUserKeyAndPrepare failed, userId %{public}u.", userId);
        return ret;
    }
    ret = KeyManager::GetInstance()->UnlockUserAppKeys(userId, true);
    if (ret != E_OK) {
        LOGE("UnlockUserAppKeys failed, userId %{public}u.", userId);
        StorageRadar::ReportActiveUserKey("ActiveUserKey::UnlockUserAppKeys", userId, ret, "EL2");
        return E_UNLOCK_APP_KEY2_FAILED;
    }
#endif
    if (updateFlag) {
        UserManager::GetInstance()->CreateBundleDataDir(userId);
    }
    std::thread([this, userId]() { RestoreconElX(userId); }).detach();
    std::thread([this]() { ActiveAppCloneUserKey(); }).detach();
    return ret;
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
    int32_t ret = KeyManager::GetInstance()->InActiveUserKey(userId);
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
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { false, "FAILED TO InActiveUserKey", "DEL", "InActiveUserKey", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::LockUserScreen(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->LockUserScreen(userId);
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
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { true, "FAILED TO LockUserScreen", "UPDATE", "LockUserScreen", 1, "FAIL" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
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
    int32_t ret = KeyManager::GetInstance()->UnlockUserScreen(userId, token, secret);
    if (ret != E_OK) {
        LOGE("UnlockUserScreen failed, userId=%{public}u, ret=%{public}d", userId, ret);
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "UnlockUserScreen",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_UNLOCK_USER_SCREEN,
            .keyElxLevel = (ret == E_UNLOCK_APP_KEY2_FAILED)? "EL5" : "EL3/EL4",
            .errorCode = ret
        };
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { true, "FAILED TO UnlockUserScreen", "UPDATE", "UnlockUserScreen", 1, "FAILED" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
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

int32_t StorageDaemon::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->GenerateAppkey(userId, hashId, keyId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->DeleteAppkey(userId, keyId);
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
    return KeyManager::GetInstance()->CreateRecoverKey(userId, userType, token, secret);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("begin to SetRecoverKey");
#ifdef USER_CRYPTO_MANAGER
    return KeyManager::GetInstance()->SetRecoverKey(key);
#else
    return E_OK;
#endif
}

int32_t StorageDaemon::UpdateKeyContext(uint32_t userId)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->UpdateKeyContext(userId);
    if (ret != E_OK) {
        LOGE("UpdateKeyContext failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "UpdateKeyContext",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_UPDATE_KEY_CONTEXT,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { true, "FAILED TO UpdateKeyContext", "UPDATE", "UpdateKeyContext", 1, "FAILED" };
        HiAudit::GetInstance().Write(storageAuditLog);
    } else {
        AuditLog storageAuditLog = { true, "SUCCESS TO UpdateKeyContext", "UPDATE", "UpdateKeyContext", 1, "SUCCESS" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
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
    const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes,
    std::vector<int64_t> &incPkgFileSizes)
{
    return QuotaManager::GetInstance()->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes,
        pkgFileSizes, incPkgFileSizes);
}

int32_t StorageDaemon::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageDaemon::MountDfsDocs start.");
    return MountManager::GetInstance()->MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemon::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageDaemon::UMountDfsDocs start.");
    return MountManager::GetInstance()->UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemon::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = KeyManager::GetInstance()->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    if (ret != E_OK) {
        LOGE("GetFileEncryptStatus failed, please check");
        RadarParameter parameterRes = {
            .orgPkg = DEFAULT_ORGPKGNAME,
            .userId = userId,
            .funcName = "GetFileEncryptStatus",
            .bizScene = BizScene::USER_KEY_ENCRYPTION,
            .bizStage = BizStage::BIZ_STAGE_GET_FILE_ENCRYPT_STATUS,
            .keyElxLevel = "EL1",
            .errorCode = ret
        };
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        AuditLog storageAuditLog = { false, "FAILED TO GetFileEncryptStatus", "SELECT", "GetFileEncryptStatus", 1,
            "FAILED" };
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    return ret;
#else
    return E_OK;
#endif
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
        oldSize = std::stoi(oldContent);
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

void StorageDaemon::ActiveAppCloneUserKey()
{
#ifdef USER_CRYPTO_MANAGER
    auto ret = AppCloneKeyManager::GetInstance()->ActiveAppCloneUserKey();
    if (ret != E_OK && (ret != E_NOT_SUPPORT)) {
        LOGE("ActiveAppCloneUserKey failed, errNo %{public}d", ret);
    }
#endif
}
} // namespace StorageDaemon
} // namespace OHOS
