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
#include "user/user_manager.h"
#include "disk/disk_manager.h"
#include "volume/volume_manager.h"
#include "storage_service_errno.h"
#include "crypto/key_manager.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
int32_t StorageDaemon::Shutdown()
{
    return E_OK;
}

int32_t StorageDaemon::Mount(std::string volId, uint32_t flags)
{
    LOGI("Handle Mount");
    return VolumeManager::Instance()->Mount(volId, flags);
}

int32_t StorageDaemon::UMount(std::string volId)
{
    LOGI("Handle UMount");
    return VolumeManager::Instance()->UMount(volId);
}

int32_t StorageDaemon::Check(std::string volId)
{
    LOGI("Handle Check");
    return VolumeManager::Instance()->Check(volId);
}

int32_t StorageDaemon::Format(std::string volId, std::string fsType)
{
    LOGI("Handle Format");
    return VolumeManager::Instance()->Format(volId, fsType);
}

int32_t StorageDaemon::Partition(std::string diskId, int32_t type)
{
    LOGI("Handle Partition");
    return DiskManager::Instance()->HandlePartition(diskId, type);
}

int32_t StorageDaemon::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    return UserManager::GetInstance()->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemon::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    return UserManager::GetInstance()->DestroyUserDirs(userId, flags);
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
    return KeyManager::GetInstance()->InitGlobalDeviceKey();
}

int32_t StorageDaemon::InitGlobalUserKeys(void)
{
    int ret = KeyManager::GetInstance()->InitGlobalUserKeys();
    if (ret) {
        LOGE("Init global users els failed");
        return ret;
    }
    return UserManager::GetInstance()->PrepareUserDirs(GLOBAL_USER_ID, CRYPTO_FLAG_EL1 | CRYPTO_FLAG_EL2);
}

int32_t StorageDaemon::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    return KeyManager::GetInstance()->GenerateUserKeys(userId, flags);
}

int32_t StorageDaemon::DeleteUserKeys(uint32_t userId)
{
    return KeyManager::GetInstance()->DeleteUserKeys(userId);
}

int32_t StorageDaemon::UpdateUserAuth(uint32_t userId, std::string auth, std::string compSecret)
{
    return KeyManager::GetInstance()->UpdateUserAuth(userId, auth,compSecret);
}

int32_t StorageDaemon::ActiveUserKey(uint32_t userId, std::string auth, std::string compSecret)
{
    return KeyManager::GetInstance()->ActiveUserKey(userId, auth, compSecret);
}

int32_t StorageDaemon::InactiveUserKey(uint32_t userId)
{
    return KeyManager::GetInstance()->InActiveUserKey(userId);
}

int32_t StorageDaemon::UpdateKeyContext(uint32_t userId)
{
    return KeyManager::GetInstance()->UpdateKeyContext(userId);
}
} // StorageDaemon
} // OHOS
