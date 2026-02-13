/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "volume/external_volume_info.h"

namespace OHOS {
namespace StorageDaemon {
constexpr int32_t GET_CLIENT_RETRY_TIMES = 5;
constexpr int32_t SLEEP_TIME = 1;
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
    if (info == nullptr) {
        return E_PARAMS_INVALID;
    }
    StorageManager::VolumeCore vc(info->GetVolumeId(), info->GetVolumeType(),
                                  info->GetDiskId(), info->GetState(), info->GetFsTypeBase());
    storageManager_->NotifyVolumeCreated(vc);

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeMounted(std::shared_ptr<VolumeInfo> volumeInfo)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (volumeInfo == nullptr) {
        return E_PARAMS_INVALID;
    }

    std::shared_ptr<ExternalVolumeInfo> info = std::static_pointer_cast<ExternalVolumeInfo>(volumeInfo);

    if (info == nullptr) {
        LOGE("volumeInfo is not ExternalVolumeInfo");
        return E_PARAMS_INVALID;
    }
    
    if (storageManager_ != nullptr) {
        storageManager_->NotifyVolumeMounted(
            StorageManager::VolumeInfoStr{
                info->GetVolumeId(), info->GetFsType(), info->GetFsUuid(),
                info->GetMountPath(), info->GetFsLabel(), info->GetDamagedFlag()
            }
        );
    }
    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeStateChanged(std::string volId, StorageManager::VolumeState state)

{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }

    storageManager_->NotifyVolumeStateChanged(volId, state);

    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeDamaged(std::shared_ptr<VolumeInfo> volumeInfo)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (volumeInfo == nullptr) {
        return E_PARAMS_INVALID;
    }

    std::shared_ptr<ExternalVolumeInfo> info = std::static_pointer_cast<ExternalVolumeInfo>(volumeInfo);
    
    if (info == nullptr) {
        LOGE("volumeInfo is not ExternalVolumeInfo");
        return E_PARAMS_INVALID;
    }
    
    if (storageManager_ != nullptr) {
        storageManager_->NotifyVolumeDamaged(
            StorageManager::VolumeInfoStr{
                info->GetVolumeId(), info->GetFsType(), info->GetFsUuid(),
                info->GetMountPath(), info->GetFsLabel(), info->GetDamagedFlag()
            }
        );
    }
    return E_OK;
}

int32_t StorageManagerClient::NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                               const std::string &uuid)
{
    LOGI("NotifyMtpMounted: id = %{public}s, path = %{public}s, desc = %{public}s, uuid = %{public}s", id.c_str(),
        path.c_str(), desc.c_str(), GetAnonyString(uuid).c_str());
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ != nullptr) {
        storageManager_->NotifyMtpMounted(id, path, desc, uuid);
    }
    return E_OK;
}

int32_t StorageManagerClient::NotifyMtpUnmounted(const std::string &id, const bool isBadRemove)
{
    LOGI("NotifyMtpUnmounted: id = %{public}s", id.c_str());
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ != nullptr) {
        storageManager_->NotifyMtpUnmounted(id, isBadRemove);
    }
    return E_OK;
}

int32_t StorageManagerClient::IsUsbFuseByType(const std::string &fsType, bool &enabled)
{
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ != nullptr) {
        return storageManager_->IsUsbFuseByType(fsType, enabled);
    }
    return E_SERVICE_IS_NULLPTR;
}

int32_t StorageManagerClient::NotifyCreateBundleDataDirWithEl(uint32_t userId, uint8_t elx)
{
    LOGI("start");
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ == nullptr) {
        return E_SERVICE_IS_NULLPTR;
    }
    return storageManager_->NotifyCreateBundleDataDirWithEl(userId, elx);
}

int32_t StorageManagerClient::QueryActiveOsAccountIds(std::vector<int32_t> &ids)
{
    LOGI("start");
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ == nullptr) {
        return E_SERVICE_IS_NULLPTR;
    }
    return storageManager_->QueryActiveOsAccountIds(ids);
}

int32_t StorageManagerClient::IsOsAccountExists(unsigned int userId, bool &isOsAccountExists)
{
    LOGI("start");
    if (GetClient() != E_OK) {
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ == nullptr) {
        return E_SERVICE_IS_NULLPTR;
    }
    return storageManager_->IsOsAccountExists(userId, isOsAccountExists);
}
} // StorageDaemon
} // OHOS
