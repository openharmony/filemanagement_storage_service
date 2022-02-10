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
#include "ipc/storage_manager_client.h"
#include <system_ability_definition.h>
#include <iservice_registry.h>
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "disk.h"

namespace OHOS {
namespace StorageDaemon {
int32_t StorageManagerClient::GetClient()
{
    if (storageManager_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("get system ability manager error");
            return E_IPC_ERROR;
        }

        auto object = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (object == nullptr) {
            LOGE("get storage manager object error");
            return E_IPC_ERROR;
        }

        storageManager_ = iface_cast<OHOS::StorageManager::IStorageManager>(object);
        if (storageManager_ == nullptr) {
            LOGE("iface_cast error");
            return E_IPC_ERROR;
        }
    }

    return E_OK;
}

int32_t StorageManagerClient::NotifyDiskCreated(std::shared_ptr<DiskInfo> &diskInfo)
{
    if (GetClient() != E_OK) {
        return E_IPC_ERROR;
    }

    StorageManager::Disk disk(diskInfo->GetId(), diskInfo->GetDevDSize(),
                              diskInfo->GetSysPath(), diskInfo->GetDevVendor(),
                              diskInfo->GetDevFlag());
    storageManager_->NotifyDiskCreated(disk);

    return E_OK;
}

int32_t StorageManagerClient::NotifyDiskDestroyed(std::string id)
{
    if (GetClient() != E_OK) {
        return E_IPC_ERROR;
    }

    storageManager_->NotifyDiskDestroyed(id);

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeCreated(VolumeInfo volumeInfo)
{
    if (GetClient() != E_OK) {
        return E_IPC_ERROR;
    }

    StorageManager::VolumeCore vc;
    storageManager_->NotifyVolumeCreated(vc);

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeMounted(VolumeInfo volumeInfo)
{
    if (GetClient() != E_OK) {
        return E_IPC_ERROR;
    }

    storageManager_->NotifyVolumeMounted("", 0, "", "", "");

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeDestroyed(VolumeInfo volumeInfo)

{
    if (GetClient() != E_OK) {
        return E_IPC_ERROR;
    }

    storageManager_->NotifyVolumeDestroyed("");

    return E_OK;
}
} // StorageDaemon
} // OHOS