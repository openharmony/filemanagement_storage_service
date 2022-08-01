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

#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <unistd.h>

#include "disk.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "volume/external_volume_info.h"

namespace OHOS {
namespace StorageDaemon {
static constexpr int32_t GET_CLIENT_RETRY_TIMES = 5;
static constexpr int32_t SLEEP_TIME = 1;
int32_t StorageManagerClient::GetClient()
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t count = 0;

    while (storageManager_ == nullptr && count++ < GET_CLIENT_RETRY_TIMES) {
        if (sam == nullptr) {
            LOGE("get system ability manager error");
            sleep(SLEEP_TIME);
            continue;
        }

        auto object = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (object == nullptr) {
            LOGE("get storage manager object error");
            sleep(SLEEP_TIME);
            continue;
        }

        storageManager_ = iface_cast<OHOS::StorageManager::IStorageManager>(object);
        if (storageManager_ == nullptr) {
            LOGE("iface_cast error");
            sleep(SLEEP_TIME);
            continue;
        }
    }

    return storageManager_ == nullptr ? E_SERVICE_IS_NULLPTR : E_OK;
}

int32_t StorageManagerClient::NotifyDiskCreated(DiskInfo &diskInfo)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }

    StorageManager::Disk disk(diskInfo.GetId(), diskInfo.GetDevDSize(),
                              diskInfo.GetSysPath(), diskInfo.GetDevVendor(),
                              diskInfo.GetDevFlag());
    storageManager_->NotifyDiskCreated(disk);

    return E_OK;
}

int32_t StorageManagerClient::NotifyDiskDestroyed(std::string id)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }

    storageManager_->NotifyDiskDestroyed(id);

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeCreated(std::shared_ptr<VolumeInfo> info)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }

    StorageManager::VolumeCore vc(info->GetVolumeId(), info->GetVolumeType(),
                                  info->GetDiskId(), info->GetState());
    storageManager_->NotifyVolumeCreated(vc);

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeMounted(std::shared_ptr<VolumeInfo> volumeInfo)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }

    std::shared_ptr<ExternalVolumeInfo> info = std::static_pointer_cast<ExternalVolumeInfo>(volumeInfo);
    storageManager_->NotifyVolumeMounted(info->GetVolumeId(), info->GetFsType(), info->GetFsUuid(),
                                         info->GetMountPath(), info->GetFsLabel());

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeDestroyed(std::string volId)

{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }

    storageManager_->NotifyVolumeDestroyed(volId);

    return E_OK;
}
} // StorageDaemon
} // OHOS
