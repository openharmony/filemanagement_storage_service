/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
constexpr size_t MASK_KEY_ID_MIN_LENGTH = 6;
constexpr size_t MASK_KEY_ID_PREFIX_LENGTH = 2;
constexpr size_t MASK_KEY_ID_SUFFIX_LENGTH = 2;

std::string MaskLogKeyId(const std::string &keyId)
{
    if (keyId.empty()) {
        return "<empty>";
    }
    if (keyId.size() <= MASK_KEY_ID_MIN_LENGTH) {
        return "***";
    }
    return keyId.substr(0, MASK_KEY_ID_PREFIX_LENGTH) + "***" + keyId.substr(keyId.size() - MASK_KEY_ID_SUFFIX_LENGTH);
}
}

namespace OHOS {
namespace StorageDaemon {
sptr<IStorageDaemon> StorageDaemonClient::GetStorageDaemonProxy(void)
{
    LOGI("[L1:IPC] GetStorageDaemonProxy: >>> ENTER <<<");

    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("[L1:IPC] GetStorageDaemonProxy: <<< EXIT FAILED <<< samgr is nullptr");
        return nullptr;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::STORAGE_MANAGER_DAEMON_ID);
    if (object == nullptr) {
        LOGE("[L1:IPC] GetStorageDaemonProxy: <<< EXIT FAILED <<< storage daemon service ability is nullptr");
        return nullptr;
    }

    LOGI("[L1:IPC] GetStorageDaemonProxy: <<< EXIT SUCCESS <<<");
    return iface_cast<IStorageDaemon>(object);
}

int32_t StorageDaemonClient::CheckServiceStatus(uint32_t serviceFlags)
{
    LOGW("[L1:IPC] CheckServiceStatus: >>> ENTER <<< serviceFlags=%{public}u", serviceFlags);

    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGW("[L1:IPC] CheckServiceStatus: samgr is nullptr, retry");
        for (uint32_t i = 0; i < CHECK_SERVICE_TIMES; i++) {
            samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (samgr != nullptr) {
                break;
            }
            if (i % LOG_CHECK_INTERVAL == 0) {
                LOGW("[L1:IPC] CheckServiceStatus: check samgr %{public}u times", i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_PRE_CHECK));
        }
        if (samgr == nullptr) {
            LOGE("[L1:IPC] CheckServiceStatus: <<< EXIT FAILED <<< samgr retry failed");
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
                LOGW("[L1:IPC] CheckServiceStatus: check storage daemon status %{public}u times", i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_PRE_CHECK));
        }
        if (exist == false) {
            LOGE("[L1:IPC] CheckServiceStatus: <<< EXIT FAILED <<< storage daemon service system ability error");
            return E_SERVICE_IS_NULLPTR;
        }
    }
    LOGW("[L1:IPC] CheckServiceStatus: <<< EXIT SUCCESS <<<");

    return E_OK;
}

int32_t StorageDaemonClient::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    LOGI("[L1:IPC] SetDirEncryptionPolicy: >>> ENTER <<< userId=%{public}u, dirPath=%{public}s, level=%{public}u",
         userId, dirPath.c_str(), level);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] SetDirEncryptionPolicy: <<< EXIT FAILED <<< userId=%{public}u, service check failed,"
             "err=%{public}d",
             userId, status);
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] SetDirEncryptionPolicy: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed",
             userId);
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->SetDirEncryptionPolicy(userId, dirPath, level);
    if (ret == E_OK) {
        LOGI("[L1:IPC] SetDirEncryptionPolicy: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] SetDirEncryptionPolicy: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("[L1:IPC] PrepareUserDirs: >>> ENTER <<< userId=%{public}d, flags=%{public}u", userId, flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] PrepareUserDirs: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::CheckServiceStatus", userId, status, extraData);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] PrepareUserDirs: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareUserDirs::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, extraData);
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->PrepareUserDirs(userId, flags);
    if (ret == E_OK) {
        LOGI("[L1:IPC] PrepareUserDirs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] PrepareUserDirs: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("[L1:IPC] DestroyUserDirs: >>> ENTER <<< userId=%{public}d, flags=%{public}u", userId, flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] DestroyUserDirs: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs::CheckServiceStatus", userId, status, extraData);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] DestroyUserDirs: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("DestroyUserDirs::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, extraData);
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->DestroyUserDirs(userId, flags);
    if (ret == E_OK) {
        LOGI("[L1:IPC] DestroyUserDirs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] DestroyUserDirs: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::StartUser(int32_t userId)
{
    LOGI("[L1:IPC] StartUser: >>> ENTER <<< userId=%{public}d", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] StartUser: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        StorageRadar::ReportUserManager("StartUser::CheckServiceStatus", userId, status, "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] StartUser: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        StorageRadar::ReportUserManager("StartUser::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, "");
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->StartUser(userId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] StartUser: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] StartUser: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::StopUser(int32_t userId)
{
    LOGI("[L1:IPC] StopUser: >>> ENTER <<< userId=%{public}d", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] StopUser: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        StorageRadar::ReportUserManager("StopUser::CheckServiceStatus", userId, status, "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] StopUser: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        StorageRadar::ReportUserManager("StopUser::GetStorageDaemonProxy", userId, E_SA_IS_NULLPTR, "");
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->StopUser(userId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] StopUser: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] StopUser: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::PrepareUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags)
{
    LOGI("[L1:IPC] PrepareUserSpace: >>> ENTER <<< userId=%{public}u, volumId=%{public}s, flags=%{public}u",
        userId, volumId.c_str(), flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] PrepareUserSpace: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] PrepareUserSpace: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->PrepareUserDirs(userId, flags);
    if (ret == E_OK) {
        LOGI("[L1:IPC] PrepareUserSpace: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] PrepareUserSpace: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::DestroyUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags)
{
    LOGI("[L1:IPC] DestroyUserSpace: >>> ENTER <<< userId=%{public}u, volumId=%{public}s, flags=%{public}u",
        userId, volumId.c_str(), flags);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] DestroyUserSpace: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] DestroyUserSpace: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->DestroyUserDirs(userId, flags);
    if (ret == E_OK) {
        LOGI("[L1:IPC] DestroyUserSpace: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] DestroyUserSpace: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::InitGlobalKey(void)
{
    LOGI("[L1:IPC] InitGlobalKey: >>> ENTER <<<");
    int32_t status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] InitGlobalKey: <<< EXIT FAILED <<< service check failed, err=%{public}d", status);
        StorageRadar::ReportUserKeyResult("InitGlobalKey::CheckServiceStatus", 0, status, "EL1", "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] InitGlobalKey: <<< EXIT FAILED <<< get proxy failed");
        StorageRadar::ReportUserKeyResult("InitGlobalKey::GetStorageDaemonProxy", 0, E_SA_IS_NULLPTR, "EL1", "");
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->InitGlobalKey();
    if (ret == E_OK) {
        LOGI("[L1:IPC] InitGlobalKey: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:IPC] InitGlobalKey: <<< EXIT FAILED <<< err=%{public}d", ret);
    }
    return ret;
}

int32_t StorageDaemonClient::InitGlobalUserKeys(void)
{
    LOGI("[L1:IPC] InitGlobalUserKeys: >>> ENTER <<<");
    int32_t status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] InitGlobalUserKeys: <<< EXIT FAILED <<< service check failed, err=%{public}d", status);
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::CheckServiceStatus", 0, status, "EL1", "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] InitGlobalUserKeys: <<< EXIT FAILED <<< get proxy failed");
        StorageRadar::ReportUserKeyResult("InitGlobalUserKeys::GetStorageDaemonProxy", 0, E_SA_IS_NULLPTR, "EL1", "");
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->InitGlobalUserKeys();
    if (ret == E_OK) {
        LOGI("[L1:IPC] InitGlobalUserKeys: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:IPC] InitGlobalUserKeys: <<< EXIT FAILED <<< err=%{public}d", ret);
    }
    return ret;
}

int32_t StorageDaemonClient::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    LOGI("[L1:IPC] EraseAllUserEncryptedKeys: >>> ENTER <<< localIdCount=%{public}zu", localIdList.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] EraseAllUserEncryptedKeys: <<< EXIT FAILED <<< service check failed, err=%{public}d", status);
        StorageRadar::ReportUserKeyResult("EraseAllUserEncryptedKeys::CheckServiceStatus", 0, status, "EL1", "");
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] EraseAllUserEncryptedKeys: <<< EXIT FAILED <<< get proxy failed");
        StorageRadar::ReportUserKeyResult("EraseAllUserEncryptedKeys::GetStorageDaemonProxy", 0, E_SA_IS_NULLPTR,
            "EL1", "");
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->EraseAllUserEncryptedKeys(localIdList);
    if (ret == E_OK) {
        LOGI("[L1:IPC] EraseAllUserEncryptedKeys: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:IPC] EraseAllUserEncryptedKeys: <<< EXIT FAILED <<< err=%{public}d", ret);
    }
    return ret;
}

int32_t StorageDaemonClient::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &oldSecret,
                                            const std::vector<uint8_t> &newSecret)
{
    LOGI("[L1:IPC] UpdateUserAuth: >>> ENTER <<< userId=%{public}u, secureUid=%{public}llu, tokenLen=%{public}zu, "
         "oldSecretLen=%{public}zu, newSecretLen=%{public}zu",
         userId, (unsigned long long)secureUid, token.size(), oldSecret.size(), newSecret.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] UpdateUserAuth: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] UpdateUserAuth: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    if (ret == E_OK) {
        LOGI("[L1:IPC] UpdateUserAuth: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] UpdateUserAuth: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                          const std::vector<uint8_t> &newSecret,
                                                          uint64_t secureUid,
                                                          uint32_t userId,
                                                          std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("[L1:IPC] UpdateUseAuthWithRecoveryKey: >>> ENTER <<< userId=%{public}u, secureUid=%{public}llu, "
         "authTokenLen=%{public}zu, newSecretLen=%{public}zu",
         userId, (unsigned long long)secureUid, authToken.size(), newSecret.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< userId=%{public}u, service check failed,"
             "err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    if (ret == E_OK) {
        LOGI("[L1:IPC] UpdateUseAuthWithRecoveryKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] UpdateUseAuthWithRecoveryKey: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d",
             userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::ActiveUserKey(uint32_t userId,
                                           const std::vector<uint8_t> &token,
                                           const std::vector<uint8_t> &secret)
{
    LOGI("[L1:IPC] ActiveUserKey: >>> ENTER <<< userId=%{public}u, tokenLen=%{public}zu, secretLen=%{public}zu",
         userId, token.size(), secret.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] ActiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] ActiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->ActiveUserKey(userId, token, secret);
    if (ret == E_OK) {
        LOGI("[L1:IPC] ActiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] ActiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::InactiveUserKey(uint32_t userId)
{
    LOGI("[L1:IPC] InactiveUserKey: >>> ENTER <<< userId=%{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] InactiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] InactiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->InactiveUserKey(userId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] InactiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] InactiveUserKey: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::LockUserScreen(uint32_t userId)
{
    LOGI("[L1:IPC] LockUserScreen: >>> ENTER <<< userId=%{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] LockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] LockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->LockUserScreen(userId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] LockUserScreen: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] LockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::UnlockUserScreen(uint32_t userId, const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &secret)
{
    LOGI("[L1:IPC] UnlockUserScreen: >>> ENTER <<< userId=%{public}u, tokenLen=%{public}zu, secretLen=%{public}zu",
         userId, token.size(), secret.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] UnlockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] UnlockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->UnlockUserScreen(userId, token, secret);
    if (ret == E_OK) {
        LOGI("[L1:IPC] UnlockUserScreen: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] UnlockUserScreen: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    LOGI("[L1:IPC] GetLockScreenStatus: >>> ENTER <<< userId=%{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] GetLockScreenStatus: <<< EXIT FAILED <<< userId=%{public}u, service check failed,"
             "err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] GetLockScreenStatus: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->GetLockScreenStatus(userId, lockScreenStatus);
    if (ret == E_OK) {
        LOGI("[L1:IPC] GetLockScreenStatus: <<< EXIT SUCCESS <<< userId=%{public}u, status=%{public}d",
             userId, lockScreenStatus);
    } else {
        LOGE("[L1:IPC] GetLockScreenStatus: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("[L1:IPC] UpdateKeyContext: >>> ENTER <<< userId=%{public}u, needRemoveTmpKey=%{public}d",
         userId, needRemoveTmpKey);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] UpdateKeyContext: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] UpdateKeyContext: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->UpdateKeyContext(userId, needRemoveTmpKey);
    if (ret == E_OK) {
        LOGI("[L1:IPC] UpdateKeyContext: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] UpdateKeyContext: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId)
{
    LOGI("[L1:IPC] GenerateAppkey: >>> ENTER <<< userId=%{public}u, hashId=%{public}u", userId, hashId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->GenerateAppkey(userId, hashId, keyId, false);
    if (ret == E_OK) {
        const std::string maskedKeyId = MaskLogKeyId(keyId);
        LOGI("[L1:IPC] GenerateAppkey: <<< EXIT SUCCESS <<< userId=%{public}u, keyId=%{public}s",
             userId, maskedKeyId.c_str());
    } else {
        LOGE("[L1:IPC] GenerateAppkey: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
    const std::string maskedKeyId = MaskLogKeyId(keyId);
    LOGI("[L1:IPC] DeleteAppkey: >>> ENTER <<< userId=%{public}u, keyIdMasked=%{public}s", userId, maskedKeyId.c_str());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] DeleteAppkey: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] DeleteAppkey: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->DeleteAppkey(userId, keyId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] DeleteAppkey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] DeleteAppkey: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::CreateRecoverKey(uint32_t userId,
                                              uint32_t userType,
                                              const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &secret)
{
    LOGI("[L1:IPC] CreateRecoverKey: >>> ENTER <<< userId=%{public}u, userType=%{public}u, tokenLen=%{public}zu, "
         "secretLen=%{public}zu", userId, userType, token.size(), secret.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] CreateRecoverKey: <<< EXIT FAILED <<< userId=%{public}u, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] CreateRecoverKey: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->CreateRecoverKey(userId, userType, token, secret);
    if (ret == E_OK) {
        LOGI("[L1:IPC] CreateRecoverKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    } else {
        LOGE("[L1:IPC] CreateRecoverKey: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("[L1:IPC] SetRecoverKey: >>> ENTER <<< keyLen=%{public}zu", key.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] SetRecoverKey: <<< EXIT FAILED <<< service check failed, err=%{public}d", status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] SetRecoverKey: <<< EXIT FAILED <<< get proxy failed");
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->SetRecoverKey(key);
    if (ret == E_OK) {
        LOGI("[L1:IPC] SetRecoverKey: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:IPC] SetRecoverKey: <<< EXIT FAILED <<< err=%{public}d", ret);
    }
    return ret;
}

int32_t StorageDaemonClient::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("[L1:IPC] MountDfsDocs: >>> ENTER <<< userId=%{public}d, relativePath=%{public}s, networkId=%{public}s, "
         "deviceId=%{public}s", userId, relativePath.c_str(), networkId.c_str(), deviceId.c_str());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] MountDfsDocs: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] MountDfsDocs: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->MountDfsDocs(userId, relativePath, networkId, deviceId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] MountDfsDocs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] MountDfsDocs: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("[L1:IPC] UMountDfsDocs: >>> ENTER <<< userId=%{public}d, relativePath=%{public}s, networkId=%{public}s, "
         "deviceId=%{public}s", userId, relativePath.c_str(), networkId.c_str(), deviceId.c_str());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] UMountDfsDocs: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] UMountDfsDocs: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    if (ret == E_OK) {
        LOGI("[L1:IPC] UMountDfsDocs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] UMountDfsDocs: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::FscryptEnable(const std::string &fscryptOptions)
{
    LOGI("[L1:IPC] FscryptEnable: >>> ENTER <<< optionsLen=%{public}zu", fscryptOptions.size());
#ifdef USER_CRYPTO_MANAGER
    int ret = SetFscryptSysparam(fscryptOptions.c_str());
    if (ret) {
        LOGE("[L1:IPC] FscryptEnable: <<< EXIT FAILED <<< init policy failed, err=%{public}d", ret);
        return ret;
    }
#endif

    LOGI("[L1:IPC] FscryptEnable: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t StorageDaemonClient::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGI("[L1:IPC] GetFileEncryptStatus: >>> ENTER <<< userId=%{public}u, needCheckDirMount=%{public}d",
         userId, needCheckDirMount);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] GetFileEncryptStatus: <<< EXIT FAILED <<< userId=%{public}u, service check failed,"
             "err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] GetFileEncryptStatus: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    if (ret == E_OK) {
        LOGI("[L1:IPC] GetFileEncryptStatus: <<< EXIT SUCCESS <<< userId=%{public}u, isEncrypted=%{public}d",
             userId, isEncrypted);
    } else {
        LOGE("[L1:IPC] GetFileEncryptStatus: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    LOGI("[L1:IPC] GetUserNeedActiveStatus: >>> ENTER <<< userId=%{public}u", userId);
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] GetUserNeedActiveStatus: <<< EXIT FAILED <<< userId=%{public}u, service check failed,"
             "err=%{public}d",
             userId, status);
        return status;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] GetUserNeedActiveStatus: <<< EXIT FAILED <<< userId=%{public}u, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }

    int32_t ret = client->GetUserNeedActiveStatus(userId, needActive);
    if (ret == E_OK) {
        LOGI("[L1:IPC] GetUserNeedActiveStatus: <<< EXIT SUCCESS <<< userId=%{public}u, needActive=%{public}d",
             userId, needActive);
    } else {
        LOGE("[L1:IPC] GetUserNeedActiveStatus: <<< EXIT FAILED <<< userId=%{public}u, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    LOGI("[L1:IPC] MountFileMgrFuse: >>> ENTER <<< userId=%{public}d, path=%{public}s", userId, path.c_str());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] MountFileMgrFuse: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] MountFileMgrFuse: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->MountFileMgrFuse(userId, path, fuseFd);
    if (ret == E_OK) {
        LOGI("[L1:IPC] MountFileMgrFuse: <<< EXIT SUCCESS <<< userId=%{public}d, fuseFd=%{public}d", userId, fuseFd);
    } else {
        LOGE("[L1:IPC] MountFileMgrFuse: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    LOGI("[L1:IPC] UMountFileMgrFuse: >>> ENTER <<< userId=%{public}d, path=%{public}s", userId, path.c_str());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] UMountFileMgrFuse: <<< EXIT FAILED <<< userId=%{public}d, service check failed, err=%{public}d",
             userId, status);
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] UMountFileMgrFuse: <<< EXIT FAILED <<< userId=%{public}d, get proxy failed", userId);
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->UMountFileMgrFuse(userId, path);
    if (ret == E_OK) {
        LOGI("[L1:IPC] UMountFileMgrFuse: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    } else {
        LOGE("[L1:IPC] UMountFileMgrFuse: <<< EXIT FAILED <<< userId=%{public}d, err=%{public}d", userId, ret);
    }
    return ret;
}

int32_t StorageDaemonClient::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    LOGI("[L1:IPC] IsFileOccupied: >>> ENTER <<< path=%{public}s, inputCount=%{public}zu",
         path.c_str(), inputList.size());
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] IsFileOccupied: <<< EXIT FAILED <<< service check failed, err=%{public}d", status);
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] IsFileOccupied: <<< EXIT FAILED <<< get proxy failed");
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->IsFileOccupied(path, inputList, outputList, isOccupy);
    if (ret == E_OK) {
        LOGI("[L1:IPC] IsFileOccupied: <<< EXIT SUCCESS <<< isOccupy=%{public}d, outputCount=%{public}zu",
             isOccupy, outputList.size());
    } else {
        LOGE("[L1:IPC] IsFileOccupied: <<< EXIT FAILED <<< err=%{public}d", ret);
    }
    return ret;
}

int32_t StorageDaemonClient::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    LOGI("[L1:IPC] ListUserdataDirInfo: >>> ENTER <<<");
    auto status = CheckServiceStatus(STORAGE_SERVICE_FLAG);
    if (status != E_OK) {
        LOGE("[L1:IPC] ListUserdataDirInfo: <<< EXIT FAILED <<< service check failed, err=%{public}d", status);
        return status;
    }
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("[L1:IPC] ListUserdataDirInfo: <<< EXIT FAILED <<< get proxy failed");
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = client->ListUserdataDirInfo(scanDirs);
    if (ret == E_OK) {
        LOGI("[L1:IPC] ListUserdataDirInfo: <<< EXIT SUCCESS <<< dirCount=%{public}zu", scanDirs.size());
    } else {
        LOGE("[L1:IPC] ListUserdataDirInfo: <<< EXIT FAILED <<< err=%{public}d", ret);
    }
    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS
