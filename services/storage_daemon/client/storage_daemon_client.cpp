/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <chrono>
#include <ctime>
#include <thread>
#include "storage_daemon_client.h"
#include "iservice_registry.h"
#include "libfscrypt/fscrypt_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "userdata_dir_info.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace {
constexpr uint32_t STORAGE_DAEMON_SFIFT = 1;
constexpr uint32_t CHECK_SERVICE_TIMES = 1000;
constexpr uint32_t LOG_CHECK_INTERVAL = 50;
constexpr uint32_t SLEEP_TIME_PRE_CHECK = 20; // 20ms
constexpr uint32_t STORAGE_SERVICE_FLAG = (1 << STORAGE_DAEMON_SFIFT);
constexpr int32_t STORAGE_DAEMON_SAID = OHOS::STORAGE_MANAGER_DAEMON_ID;
}

namespace OHOS {
namespace StorageDaemon {
sptr<IStorageDaemon> StorageDaemonClient::GetStorageDaemonProxy(void)
{
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("samgr empty error");
        return nullptr;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::STORAGE_MANAGER_DAEMON_ID);
    if (object == nullptr) {
        LOGE("storage daemon client samgr ablity empty error");
        return nullptr;
    }

    return iface_cast<IStorageDaemon>(object);
}

int32_t StorageDaemonClient::CheckServiceStatus(uint32_t serviceFlags)
{
    LOGW("CheckServiceStatus start");

    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGW("samgr is nullptr, retry");
        for (uint32_t i = 0; i < CHECK_SERVICE_TIMES; i++) {
            samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (samgr != nullptr) {
                break;
            }
            if (i % LOG_CHECK_INTERVAL == 0) {
                LOGW("check samgr %{public}u times", i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_PRE_CHECK));
        }
        if (samgr == nullptr) {
            LOGE("samgr is nullptr, retry failed.");
            return E_SA_IS_NULLPTR;
        }
    }

    if (serviceFlags & STORAGE_SERVICE_FLAG) {
        bool exist = false;
        for (uint32_t i = 0; i < CHECK_SERVICE_TIMES; i++) {
            auto object = samgr->CheckSystemAbility(STORAGE_DAEMON_SAID, exist);
            if (object != nullptr) {
                break;
            }
            if (i % LOG_CHECK_INTERVAL == 0) {
                LOGW("check storage daemon status %{public}u times", i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_PRE_CHECK));
        }
        if (exist == false) {
            LOGE("storage daemon service system ability error");
            return E_SERVICE_IS_NULLPTR;
        }
    }
    LOGW("CheckServiceStatus end, success");

    return E_OK;
}

int32_t StorageDaemonClient::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    LOGI("Enter SetDirEncryptionPolicy!");
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }
    return client->SetDirEncryptionPolicy(userId, dirPath, level);
}

int32_t StorageDaemonClient::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("StorageDaemonClient::PrepareUserDirs, userId:%{public}d, flags:%{public}u", userId, flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::CheckServiceStatus", userId, status, extraData);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, extraData);
        return E_SA_IS_NULLPTR;
    }
    return client->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonClient::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("StorageDaemonClient::DestroyUserDirs, userId:%{public}d, flags:%{public}u", userId, flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs::CheckServiceStatus", userId, status, extraData);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, extraData);
        return E_SA_IS_NULLPTR;
    }
    return client->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonClient::StartUser(int32_t userId)
{
    LOGI("StorageDaemonClient::StartUser, userId:%{public}d", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        StorageRadar::ReportUserManager("StartUser::CheckServiceStatus", userId, status, "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        StorageRadar::ReportUserManager("StartUser::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, "");
        return E_SA_IS_NULLPTR;
    }

    return client->StartUser(userId);
}

int32_t StorageDaemonClient::StopUser(int32_t userId)
{
    LOGI("StorageDaemonClient::StopUser, userId:%{public}d", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        StorageRadar::ReportUserManager("StartUser::CheckServiceStatus", userId, status, "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        StorageRadar::ReportUserManager("StartUser::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, "");
        return E_SA_IS_NULLPTR;
    }

    return client->StopUser(userId);
}

int32_t StorageDaemonClient::PrepareUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags)
{
    LOGI("StorageDaemonClient::PrepareUserSpace, userId:%{public}u, volumId:%{public}s, flags:%{publc}u",
        userId, volumId.c_str(), flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonClient::DestroyUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags)
{
    LOGI("StorageDaemonClient::DestroyUserSpace, userId:%{public}u, volumId:%{public}s, flags:%{publc}u",
        userId, volumId.c_str(), flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonClient::InitGlobalKey(void)
{
    int32_t status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        StorageRadar::ReportUserKeyResult("InitGlobalKey::CheckServiceStatus", 0, status, "EL1", "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        StorageRadar::ReportUserKeyResult("InitGlobalKey::GetStorageDaemonProxy", 0, E_SA_IS_NULLPTR, "EL1", "");
        return E_SA_IS_NULLPTR;
    }

    return client->InitGlobalKey();
}

int32_t StorageDaemonClient::InitGlobalUserKeys(void)
{
    int32_t status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::CheckServiceStatus", 0, status, "EL1", "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::GetStorageDaemonProxy", 0, E_SA_IS_NULLPTR, "EL1", "");
        return E_SA_IS_NULLPTR;
    }

    return client->InitGlobalUserKeys();
}

int32_t StorageDaemonClient::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    LOGI("StorageDaemonClient::EraseAllUserEncryptedKeys, localIdList:%{public}d", localIdList.empty());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        StorageRadar::ReportUserKeyResult("EraseAllUserEncryptedKeys::CheckServiceStatus", 0, status, "EL1", "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        StorageRadar::ReportUserKeyResult("EraseAllUserEncryptedKeys::GetStorageDaemonProxy", 0, E_SA_IS_NULLPTR,
            "EL1", "");
        return E_SA_IS_NULLPTR;
    }

    return client->EraseAllUserEncryptedKeys(localIdList);
}

int32_t StorageDaemonClient::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &oldSecret,
                                            const std::vector<uint8_t> &newSecret)
{
    LOGI("StorageDaemonClient::UpdateUserAuth, userId: %{public}u, token:%{public}d,"
        "oldSecret:%{public}d, newSecret:%{public}d", userId, token.empty(), oldSecret.empty(), newSecret.empty());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
}

int32_t StorageDaemonClient::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                          const std::vector<uint8_t> &newSecret,
                                                          uint64_t secureUid,
                                                          uint32_t userId,
                                                          std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("StorageDaemonClient::UpdateUseAuthWithRecoveryKey, authToken: %{public}d, newSecret:%{public}d,"
        "userId:%{public}d", authToken.empty(), newSecret.empty(), userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
}

int32_t StorageDaemonClient::ActiveUserKey(uint32_t userId,
                                           const std::vector<uint8_t> &token,
                                           const std::vector<uint8_t> &secret)
{
    LOGI("StorageDaemonClient::ActiveUserKey, userId: %{public}u, token:%{public}d, secret:%{public}d",
        userId, token.empty(), secret.empty());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->ActiveUserKey(userId, token, secret);
}

int32_t StorageDaemonClient::InactiveUserKey(uint32_t userId)
{
    LOGI("StorageDaemonClient::InactiveUserKey, userId: %{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->InactiveUserKey(userId);
}

int32_t StorageDaemonClient::LockUserScreen(uint32_t userId)
{
    LOGI("StorageDaemonClient::LockUserScreen, userId: %{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->LockUserScreen(userId);
}

int32_t StorageDaemonClient::UnlockUserScreen(uint32_t userId, const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &secret)
{
    LOGI("StorageDaemonClient::UnlockUserScreen, userId: %{public}u, token:%{public}d, secret:%{public}d",
        userId, token.empty(), secret.empty());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UnlockUserScreen(userId, token, secret);
}

int32_t StorageDaemonClient::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    LOGI("StorageDaemonClient::GetLockScreenStatus, userId: %{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->GetLockScreenStatus(userId, lockScreenStatus);
}

int32_t StorageDaemonClient::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("StorageDaemonClient::UpdateKeyContext, userId: %{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UpdateKeyContext(userId, needRemoveTmpKey);
}

int32_t StorageDaemonClient::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId)
{
    LOGI("StorageDaemonClient::GenerateAppkey, userId: %{public}u, hashId:%{public}u", userId, hashId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->GenerateAppkey(userId, hashId, keyId, false);
}

int32_t StorageDaemonClient::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    LOGI("StorageDaemonClient::DeleteAppkey, userId: %{public}u, keyId:%{public}s", userId, keyId.c_str());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->DeleteAppkey(userId, keyId);
}

int32_t StorageDaemonClient::CreateRecoverKey(uint32_t userId,
                                              uint32_t userType,
                                              const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &secret)
{
    LOGI("StorageDaemonClient::CreateRecoverKey, userId: %{public}u, userType:%{public}u,token:%{public}d,"
        "secret:%{public}d", userId, userType, token.empty(), secret.empty());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->CreateRecoverKey(userId, userType, token, secret);
}

int32_t StorageDaemonClient::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("StorageDaemonClient::SetRecoverKey, key:%{public}d", key.empty());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->SetRecoverKey(key);
}

int32_t StorageDaemonClient::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }

    return client->MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonClient::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }

    return client->UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonClient::FscryptEnable(const std::string &fscryptOptions)
{
#ifdef USER_CRYPTO_MANAGER
    int ret = SetFscryptSysparam(fscryptOptions.c_str());
    if (ret) {
        LOGE("Init fscrypt policy failed ret %{public}d", ret);
        return ret;
    }
#endif

    return 0;
}

int32_t StorageDaemonClient::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGI("StorageDaemonClient::GetFileEncryptStatus, userId:%{public}d", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }

    return client->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
}

int32_t StorageDaemonClient::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    LOGI("StorageDaemonClient::GetUserNeedActiveStatus, userId:%{public}d", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }

    return client->GetUserNeedActiveStatus(userId, needActive);
}

int32_t StorageDaemonClient::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }
    return client->MountFileMgrFuse(userId, path, fuseFd);
}

int32_t StorageDaemonClient::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }
    return client->UMountFileMgrFuse(userId, path);
}

int32_t StorageDaemonClient::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    LOGI("StorageDaemonClient::IsFileOccupied");
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }
    return client->IsFileOccupied(path, inputList, outputList, isOccupy);
}
 
int32_t StorageDaemonClient::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    LOGI("StorageDaemonClient::ListUserdataDirInfo");
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("service check failed");
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return E_SA_IS_NULLPTR;
    }
    return client->ListUserdataDirInfo(scanDirs);
}
} // namespace StorageDaemon
} // namespace OHOS
