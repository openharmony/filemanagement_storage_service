/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "storage_daemon_client.h"

#include <chrono>
#include <ctime>
#include <thread>

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iservice_registry.h"
#include "libfscrypt/fscrypt_utils.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"

namespace {
constexpr uint32_t STORAGE_DAEMON_SFIFT = 1;
constexpr uint32_t CHECK_SERVICE_TIMES = 1000;
constexpr uint32_t SLEEP_TIME_PRE_CHECK = 10; // 10ms
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

bool StorageDaemonClient::CheckServiceStatus(uint32_t serviceFlags)
{
    LOGI("CheckServiceStatus start");

    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGW("samgr empty, retry");
        for (uint32_t i = 0; i < CHECK_SERVICE_TIMES; i++) {
            samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (samgr != nullptr) {
                break;
            }
            LOGI("check samgr %{public}u times", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_PRE_CHECK));
        }
        if (samgr == nullptr) {
            LOGE("samgr empty error");
            return false;
        }
    }

    bool exist = false;
    if (serviceFlags & STORAGE_SERVICE_FLAG) {
        for (uint32_t i = 0; i < CHECK_SERVICE_TIMES; i++) {
            auto object = samgr->CheckSystemAbility(STORAGE_DAEMON_SAID, exist);
            if (object != nullptr) {
                break;
            }
            LOGI("check storage daemon status %{public}u times", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_PRE_CHECK));
        }
        if (exist == false) {
            LOGE("storage daemon service system ability error");
            return false;
        }
    }
    LOGI("CheckServiceStatus end, success");

    return true;
}

int32_t StorageDaemonClient::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }
    return client->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonClient::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }
    return client->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonClient::StartUser(int32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->StartUser(userId);
}

int32_t StorageDaemonClient::StopUser(int32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->StopUser(userId);
}

int32_t StorageDaemonClient::PrepareUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonClient::DestroyUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonClient::InitGlobalKey(void)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->InitGlobalKey();
}

int32_t StorageDaemonClient::InitGlobalUserKeys(void)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->InitGlobalUserKeys();
}

int32_t StorageDaemonClient::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->GenerateUserKeys(userId, flags);
}

int32_t StorageDaemonClient::DeleteUserKeys(uint32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->DeleteUserKeys(userId);
}

int32_t StorageDaemonClient::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &oldSecret,
                                            const std::vector<uint8_t> &newSecret)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
}

int32_t StorageDaemonClient::ActiveUserKey(uint32_t userId,
                                           const std::vector<uint8_t> &token,
                                           const std::vector<uint8_t> &secret)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->ActiveUserKey(userId, token, secret);
}

int32_t StorageDaemonClient::InactiveUserKey(uint32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->InactiveUserKey(userId);
}

int32_t StorageDaemonClient::LockUserScreen(uint32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->LockUserScreen(userId);
}

int32_t StorageDaemonClient::UnlockUserScreen(uint32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->UnlockUserScreen(userId);
}

int32_t StorageDaemonClient::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->GetLockScreenStatus(userId, lockScreenStatus);
}

int32_t StorageDaemonClient::UpdateKeyContext(uint32_t userId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->UpdateKeyContext(userId);
}

int32_t StorageDaemonClient::GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->GenerateAppkey(userId, appUid, keyId);
}

int32_t StorageDaemonClient::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("service check failed");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->DeleteAppkey(userId, keyId);
}

int32_t StorageDaemonClient::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    if (!CheckServiceStatus(STORAGE_SERVICE_FLAG)) {
        LOGE("Storage service flag check failed!");
        return -EAGAIN;
    }

    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("Get StorageDaemon service failed!");
        return -EAGAIN;
    }

    return client->MountDfsDocs(userId, relativePath, networkId, deviceId);
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
} // namespace StorageDaemon
} // namespace OHOS
