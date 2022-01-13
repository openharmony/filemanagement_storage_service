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
#include "storage_daemon_client.h"

#include "utils/log.h"
#include "iremote_proxy.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

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

int32_t StorageDaemonClient::StartUser(int32_t userId)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->StartUser(userId);
}

int32_t StorageDaemonClient::StopUser(int32_t userId)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->StopUser(userId);
}

int32_t StorageDaemonClient::PrepareUserDirs(int32_t userId, u_int32_t flags)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonClient::DestroyUserDirs(int32_t userId, u_int32_t flags)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonClient::InitGlobalKey(void)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->InitGlobalKey();
}

int32_t StorageDaemonClient::InitGlobalUserKeys(void)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->InitGlobalUserKeys();
}

int32_t StorageDaemonClient::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->GenerateUserKeys(userId, flags);
}

int32_t StorageDaemonClient::DeleteUserKeys(uint32_t userId)
{
    sptr<IStorageDaemon> client = GetStorageDaemonProxy();
    if (client == nullptr) {
        LOGE("get storage daemon service failed");
        return -EAGAIN;
    }

    return client->DeleteUserKeys(userId);
}
}
}