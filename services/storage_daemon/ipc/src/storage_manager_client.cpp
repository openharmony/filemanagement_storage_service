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
    LOGI("[L1:StorageManagerClient] GetClient: >>> ENTER <<<");
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    int32_t count = 0;

    while (storageManager_ == nullptr && count++ < GET_CLIENT_RETRY_TIMES) {
        if (sam == nullptr) {
            LOGE("[L1:StorageManagerClient] GetClient: <<< EXIT FAILED <<< get system ability manager error");
            sleep(SLEEP_TIME);
            continue;
        }

        auto object = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (object == nullptr) {
            LOGE("[L1:StorageManagerClient] GetClient: get storage manager object error, retry count=%{public}d",
                 count);
            sleep(SLEEP_TIME);
            continue;
        }

        storageManager_ = iface_cast<OHOS::StorageManager::IStorageManager>(object);
        if (storageManager_ == nullptr) {
            LOGE("[L1:StorageManagerClient] GetClient: iface_cast error, retry count=%{public}d", count);
            sleep(SLEEP_TIME);
            continue;
        }
    }

    int32_t ret = storageManager_ == nullptr ? E_SERVICE_IS_NULLPTR : E_OK;
    if (ret == E_OK) {
        LOGI("[L1:StorageManagerClient] GetClient: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:StorageManagerClient] GetClient: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

int32_t StorageManagerClient::NotifyDiskCreated(DiskInfo &diskInfo)
{
    LOGI("[L1:StorageManagerClient] NotifyDiskCreated: >>> ENTER <<< diskId=%{public}s",
         diskInfo.GetId().c_str());
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyDiskCreated: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }

    StorageManager::Disk disk(diskInfo.GetId(), diskInfo.GetDevDSize(),
                              diskInfo.GetSysPath(), diskInfo.GetDevVendor(),
                              diskInfo.GetDevFlag());
    storageManager_->NotifyDiskCreated(disk);

    LOGI("[L1:StorageManagerClient] NotifyDiskCreated: <<< EXIT SUCCESS <<< diskId=%{public}s",
         diskInfo.GetId().c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyDiskDestroyed(std::string id)
{
    LOGI("[L1:StorageManagerClient] NotifyDiskDestroyed: >>> ENTER <<< id=%{public}s", id.c_str());
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyDiskDestroyed: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }

    storageManager_->NotifyDiskDestroyed(id);

    LOGI("[L1:StorageManagerClient] NotifyDiskDestroyed: <<< EXIT SUCCESS <<< id=%{public}s", id.c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeCreated(std::shared_ptr<VolumeInfo> info)
{
    LOGI("[L1:StorageManagerClient] NotifyVolumeCreated: >>> ENTER <<<");
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeCreated: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (info == nullptr) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeCreated: <<< EXIT FAILED <<< info is nullptr");
        return E_PARAMS_INVALID;
    }
    StorageManager::VolumeCore vc(info->GetVolumeId(), info->GetVolumeType(),
                                  info->GetDiskId(), info->GetState(), info->GetFsTypeBase());
    storageManager_->NotifyVolumeCreated(vc);

    LOGI("[L1:StorageManagerClient] NotifyVolumeCreated: <<< EXIT SUCCESS <<< volumeId=%{public}s",
         info->GetVolumeId().c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeMounted(std::shared_ptr<VolumeInfo> volumeInfo)
{
    LOGI("[L1:StorageManagerClient] NotifyVolumeMounted: >>> ENTER <<<");
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeMounted: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (volumeInfo == nullptr) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeMounted: <<< EXIT FAILED <<< volumeInfo is nullptr");
        return E_PARAMS_INVALID;
    }

    std::shared_ptr<ExternalVolumeInfo> info = std::static_pointer_cast<ExternalVolumeInfo>(volumeInfo);

    if (info == nullptr) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeMounted: <<< EXIT FAILED <<< volumeInfo is not ExternalVolumeInfo");
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
    LOGI("[L1:StorageManagerClient] NotifyVolumeMounted: <<< EXIT SUCCESS <<< volumeId=%{public}s",
         info->GetVolumeId().c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeStateChanged(std::string volId, StorageManager::VolumeState state)

{
    LOGI("[L1:StorageManagerClient] NotifyVolumeStateChanged: >>> ENTER <<< volId=%{public}s, state=%{public}d",
         volId.c_str(), static_cast<int>(state));
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeStateChanged: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }

    storageManager_->NotifyVolumeStateChanged(volId, state);

    LOGI("[L1:StorageManagerClient] NotifyVolumeStateChanged: <<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyVolumeDamaged(std::shared_ptr<VolumeInfo> volumeInfo)
{
    LOGI("[L1:StorageManagerClient] NotifyVolumeDamaged: >>> ENTER <<<");
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeDamaged: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (volumeInfo == nullptr) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeDamaged: <<< EXIT FAILED <<< volumeInfo is nullptr");
        return E_PARAMS_INVALID;
    }

    std::shared_ptr<ExternalVolumeInfo> info = std::static_pointer_cast<ExternalVolumeInfo>(volumeInfo);

    if (info == nullptr) {
        LOGE("[L1:StorageManagerClient] NotifyVolumeDamaged: <<< EXIT FAILED <<< volumeInfo is not ExternalVolumeInfo");
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
    LOGI("[L1:StorageManagerClient] NotifyVolumeDamaged: <<< EXIT SUCCESS <<< volumeId=%{public}s",
         info->GetVolumeId().c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                               const std::string &uuid, const std::string &fsType)
{
    LOGI("[L1:StorageManagerClient] NotifyMtpMounted: >>> ENTER <<< id=%{public}s, path=%{public}s, desc=%{public}s,"
         "uuid=%{private}s",
        id.c_str(), path.c_str(), desc.c_str(), GetAnonyString(uuid).c_str());
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyMtpMounted: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ != nullptr) {
        storageManager_->NotifyMtpMounted(id, path, desc, uuid, fsType);
    }
    LOGI("[L1:StorageManagerClient] NotifyMtpMounted: <<< EXIT SUCCESS <<< id=%{public}s", id.c_str());
    return E_OK;
}

int32_t StorageManagerClient::NotifyMtpUnmounted(const std::string &id, const bool isBadRemove)
{
    LOGI("[L1:StorageManagerClient] NotifyMtpUnmounted: >>> ENTER <<< id=%{public}s, isBadRemove=%{public}d",
         id.c_str(), isBadRemove);
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyMtpUnmounted: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ != nullptr) {
        storageManager_->NotifyMtpUnmounted(id, isBadRemove);
    }
    LOGI("[L1:StorageManagerClient] NotifyMtpUnmounted: <<< EXIT SUCCESS <<< id=%{public}s", id.c_str());
    return E_OK;
}

int32_t StorageManagerClient::IsUsbFuseByType(const std::string &fsType, bool &enabled)
{
    LOGI("[L1:StorageManagerClient] IsUsbFuseByType: >>> ENTER <<< fsType=%{public}s", fsType.c_str());
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] IsUsbFuseByType: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ != nullptr) {
        int32_t ret = storageManager_->IsUsbFuseByType(fsType, enabled);
        if (ret == E_OK) {
            LOGI("[L1:StorageManagerClient] IsUsbFuseByType: <<< EXIT SUCCESS <<< fsType=%{public}s,"
                 "enabled=%{public}d",
                 fsType.c_str(), enabled);
        } else {
            LOGE("[L1:StorageManagerClient] IsUsbFuseByType: <<< EXIT FAILED <<< ret=%{public}d", ret);
        }
        return ret;
    }
    LOGE("[L1:StorageManagerClient] IsUsbFuseByType: <<< EXIT FAILED <<< storageManager_ is nullptr");
    return E_SERVICE_IS_NULLPTR;
}

int32_t StorageManagerClient::NotifyCreateBundleDataDirWithEl(uint32_t userId, uint8_t elx)
{
    LOGI("[L1:StorageManagerClient] NotifyCreateBundleDataDirWithEl: >>> ENTER <<< userId=%{public}u, elx=%{public}u",
         userId, elx);
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] NotifyCreateBundleDataDirWithEl: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ == nullptr) {
        LOGE("[L1:StorageManagerClient] NotifyCreateBundleDataDirWithEl: <<< EXIT FAILED <<< storageManager_ is"
             "nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    int32_t ret = storageManager_->NotifyCreateBundleDataDirWithEl(userId, elx);
    if (ret == E_OK) {
        LOGI("[L1:StorageManagerClient] NotifyCreateBundleDataDirWithEl: <<< EXIT SUCCESS <<< userId=%{public}u,"
             "elx=%{public}u",
             userId, elx);
    } else {
        LOGE("[L1:StorageManagerClient] NotifyCreateBundleDataDirWithEl: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

int32_t StorageManagerClient::QueryActiveOsAccountIds(std::vector<int32_t> &ids)
{
    LOGI("[L1:StorageManagerClient] QueryActiveOsAccountIds: >>> ENTER <<<");
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] QueryActiveOsAccountIds: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ == nullptr) {
        LOGE("[L1:StorageManagerClient] QueryActiveOsAccountIds: <<< EXIT FAILED <<< storageManager_ is nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    int32_t ret = storageManager_->QueryActiveOsAccountIds(ids);
    if (ret == E_OK) {
        LOGI("[L1:StorageManagerClient] QueryActiveOsAccountIds: <<< EXIT SUCCESS <<< count=%{public}zu", ids.size());
    } else {
        LOGE("[L1:StorageManagerClient] QueryActiveOsAccountIds: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

int32_t StorageManagerClient::IsOsAccountExists(unsigned int userId, bool &isOsAccountExists)
{
    LOGI("[L1:StorageManagerClient] IsOsAccountExists: >>> ENTER <<< userId=%{public}u", userId);
    if (GetClient() != E_OK) {
        LOGE("[L1:StorageManagerClient] IsOsAccountExists: <<< EXIT FAILED <<< GetClient failed");
        return E_SERVICE_IS_NULLPTR;
    }
    if (storageManager_ == nullptr) {
        LOGE("[L1:StorageManagerClient] IsOsAccountExists: <<< EXIT FAILED <<< storageManager_ is nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    int32_t ret = storageManager_->IsOsAccountExists(userId, isOsAccountExists);
    if (ret == E_OK) {
        LOGI("[L1:StorageManagerClient] IsOsAccountExists: <<< EXIT SUCCESS <<< userId=%{public}u, exists=%{public}d",
             userId, isOsAccountExists);
    } else {
        LOGE("[L1:StorageManagerClient] IsOsAccountExists: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}
} // StorageDaemon
} // OHOS
