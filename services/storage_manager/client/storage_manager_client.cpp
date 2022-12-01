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
#include "client/storage_manager_client.h"

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iservice_registry.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace StorageManager {
sptr<IStorageManager> StorageManagerClient::GetStorageManagerProxy(void)
{
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("samgr empty error");
        return nullptr;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::STORAGE_MANAGER_MANAGER_ID);
    if (object == nullptr) {
        LOGE("storage manager client samgr ablity empty error");
        return nullptr;
    }

    return iface_cast<IStorageManager>(object);
}

int32_t StorageManagerClient::PrepareAddUser(uint32_t userId, uint32_t flags)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->PrepareAddUser(userId, flags);
}

int32_t StorageManagerClient::RemoveUser(uint32_t userId, uint32_t flags)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->RemoveUser(userId, flags);
}

int32_t StorageManagerClient::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->GenerateUserKeys(userId, flags);
}

int32_t StorageManagerClient::DeleteUserKeys(uint32_t userId)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->DeleteUserKeys(userId);
}

int32_t StorageManagerClient::UpdateUserAuth(uint32_t userId,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &oldSecret,
                                             const std::vector<uint8_t> &newSecret)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->UpdateUserAuth(userId, token, oldSecret, newSecret);
}

int32_t StorageManagerClient::ActiveUserKey(uint32_t userId,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &secret)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->ActiveUserKey(userId, token, secret);
}

int32_t StorageManagerClient::InactiveUserKey(uint32_t userId)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->InactiveUserKey(userId);
}

int32_t StorageManagerClient::UpdateKeyContext(uint32_t userId)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return -EFAULT;
    }

    return client->UpdateKeyContext(userId);
}
}
}
