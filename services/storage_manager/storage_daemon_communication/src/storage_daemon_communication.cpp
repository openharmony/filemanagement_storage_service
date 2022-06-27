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

#include "storage_daemon_communication/storage_daemon_communication.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon.h"
#include "ipc/storage_daemon_proxy.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
StorageDaemonCommunication::StorageDaemonCommunication()
{
    LOGI("DEBUG StorageDaemonCommunication constructer");
    storageDaemon_ = nullptr;
}

StorageDaemonCommunication::~StorageDaemonCommunication()
{
    LOGI("DEBUG ~StorageDaemonCommunication destructer ~");
}

int32_t StorageDaemonCommunication::Connect() 
{
    int32_t err = 0;
    LOGI("StorageDaemonCommunication::Connect start");
    if (storageDaemon_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("StorageDaemonCommunication::Connect samgr nullptr");
            return E_IPC_ERROR;
        }
        auto object = sam->GetSystemAbility(STORAGE_MANAGER_DAEMON_ID);
        if (object == nullptr) {
            LOGE("StorageDaemonCommunication::Connect object nullptr");
            return E_IPC_ERROR;
        }
        storageDaemon_ = iface_cast<OHOS::StorageDaemon::IStorageDaemon>(object);
        if (storageDaemon_ == nullptr) {
            LOGE("StorageDaemonCommunication::Connect service nullptr");
            return E_IPC_ERROR;
        }
    }
    LOGI("StorageDaemonCommunication::Connect end");
    return err;
}

int32_t StorageDaemonCommunication::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageDaemonCommunication::PrepareAddUser start");
    
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::PrepareAddUser connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonCommunication::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageDaemonCommunication::RemoveUser start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::RemoveUser connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonCommunication::PrepareStartUser(int32_t userId)
{
    LOGI("StorageDaemonCommunication::PrepareStartUser start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::PrepareStartUser connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->StartUser(userId);
}

int32_t StorageDaemonCommunication::StopUser(int32_t userId)
{
    LOGI("StorageDaemonCommunication::StopUser start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::StopUser connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->StopUser(userId);
}

int32_t StorageDaemonCommunication::Mount(std::string volumeId, int32_t flag)
{
    LOGI("StorageDaemonCommunication::mount start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::mount connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->Mount(volumeId, flag);
}

int32_t StorageDaemonCommunication::Unmount(std::string volumeId)
{
    LOGI("StorageDaemonCommunication::unmount start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::unmount connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->UMount(volumeId);
}

int32_t StorageDaemonCommunication::Check(std::string volumeId)
{
    LOGI("StorageDaemonCommunication::check start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::check connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->Check(volumeId);
}

int32_t StorageDaemonCommunication::Partition(std::string diskId, int32_t type)
{
    LOGI("StorageDaemonCommunication::Partition start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::Partition connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->Partition(diskId, type);
}

int32_t StorageDaemonCommunication::Format(std::string volumeId, std::string type)
{
    LOGI("StorageDaemonCommunication::Format start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::Format connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->Format(volumeId, type);
}

int32_t StorageDaemonCommunication::SetVolumeDescription(std::string volumeId, std::string description)
{
    LOGI("StorageDaemonCommunication::SetVolumeDescription start");
    if (Connect() != E_OK) {
        LOGE("StorageDaemonCommunication::SetVolumeDescription connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->SetVolumeDescription(volumeId, description);
}

int32_t StorageDaemonCommunication::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("enter");
    if (Connect() != E_OK) {
        LOGE("Connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->GenerateUserKeys(userId, flags);
}

int32_t StorageDaemonCommunication::DeleteUserKeys(uint32_t userId)
{
    LOGI("enter");
    if (Connect() != E_OK) {
        LOGE("Connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->DeleteUserKeys(userId);
}

int32_t StorageDaemonCommunication::UpdateUserAuth(uint32_t userId, std::string auth, std::string compSecret)
{
    LOGI("enter");
    if (Connect() != E_OK) {
        LOGE("Connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->UpdateUserAuth(userId, auth, compSecret);
}

int32_t StorageDaemonCommunication::ActiveUserKey(uint32_t userId, std::string auth, std::string compSecret)
{
    LOGI("enter");
    if (Connect() != E_OK) {
        LOGE("Connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->ActiveUserKey(userId, auth, compSecret);
}

int32_t StorageDaemonCommunication::InactiveUserKey(uint32_t userId)
{
    LOGI("enter");
    if (Connect() != E_OK) {
        LOGE("Connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->InactiveUserKey(userId);
}

int32_t StorageDaemonCommunication::UpdateKeyContext(uint32_t userId)
{
    LOGI("enter");
    if (Connect() != E_OK) {
        LOGE("Connect failed");
        return E_IPC_ERROR;
    }
    return storageDaemon_->UpdateKeyContext(userId);
}
} // StorageManager
} // OHOS
