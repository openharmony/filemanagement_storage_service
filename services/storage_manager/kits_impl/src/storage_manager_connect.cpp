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

#include "storage_manager_connect.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "ipc/storage_manager.h"
#include "ipc/storage_manager_proxy.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

using namespace std;

namespace OHOS {
namespace StorageManager {
StorageManagerConnect::StorageManagerConnect() {}
StorageManagerConnect::~StorageManagerConnect() {}

int32_t StorageManagerConnect::Connect()
{
    LOGI("StorageManagerConnect::Connect start");
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageManager_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("StorageManagerConnect::Connect samgr == nullptr");
            return E_SA_IS_NULLPTR;
        }
        auto object = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (object == nullptr) {
            LOGE("StorageManagerConnect::Connect object == nullptr");
            return E_REMOTE_IS_NULLPTR;
        }
        storageManager_ = iface_cast<StorageManager::IStorageManager>(object);
        if (storageManager_ == nullptr) {
            LOGE("StorageManagerConnect::Connect service == nullptr");
            return E_SERVICE_IS_NULLPTR;
        }
        deathRecipient_ = new (std::nothrow) SmDeathRecipient();
        if (!deathRecipient_) {
            LOGE("StorageManagerConnect::Connect failed to create death recipient");
            return E_DEATH_RECIPIENT_IS_NULLPTR;
        }

        storageManager_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    LOGI("StorageManagerConnect::Connect end");
    return E_OK;
}

int32_t StorageManagerConnect::GetBundleStats(string pkgName, BundleStats &BundleStats)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetBundleStats:Connect error");
        return err;
    }
    return storageManager_->GetBundleStats(pkgName, BundleStats);
}

int32_t StorageManagerConnect::GetFreeSizeOfVolume(string volumeUuid, int64_t &freeSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetFreeSizeOfVolume:Connect error");
        return err;
    }
    return storageManager_->GetFreeSizeOfVolume(volumeUuid, freeSize);
}

int32_t StorageManagerConnect::GetTotalSizeOfVolume(string volumeUuid, int64_t &totalSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetTotalSizeOfVolume:Connect error");
        return err;
    }
    return storageManager_->GetTotalSizeOfVolume(volumeUuid, totalSize);
}

int32_t StorageManagerConnect::Mount(std::string volumeId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::Mount:Connect error");
        return err;
    }
    return storageManager_->Mount(volumeId);
}

int32_t StorageManagerConnect::Unmount(std::string volumeId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::Unmount:Connect error");
        return err;
    }
    return storageManager_->Unmount(volumeId);
}

int32_t StorageManagerConnect::GetAllVolumes(std::vector<VolumeExternal> &vecOfVol)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetAllVolumes:Connect error");
        return err;
    }
    return storageManager_->GetAllVolumes(vecOfVol);
}

int32_t StorageManagerConnect::GetSystemSize(int64_t &systemSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetSystemSize:Connect error");
        return err;
    }
    return storageManager_->GetSystemSize(systemSize);
}

int32_t StorageManagerConnect::GetTotalSize(int64_t &totalSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetTotalSize:Connect error");
        return err;
    }
    return storageManager_->GetTotalSize(totalSize);
}

int32_t StorageManagerConnect::GetFreeSize(int64_t &freeSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetFreeSize:Connect error");
        return err;
    }
    return storageManager_->GetFreeSize(freeSize);
}

int32_t StorageManagerConnect::GetUserStorageStats(StorageStats &storageStats)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetUserStorageStats:Connect error");
        return err;
    }
    return storageManager_->GetUserStorageStats(storageStats);
}

int32_t StorageManagerConnect::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetUserStorageStats:Connect error");
        return err;
    }
    return storageManager_->GetUserStorageStats(userId, storageStats);
}

int32_t StorageManagerConnect::GetCurrentBundleStats(BundleStats &bundleStats)
{
    BundleStats result;
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetCurrentBundleStats:Connect error");
        return err;
    }
    return storageManager_->GetCurrentBundleStats(bundleStats);
}

int32_t StorageManagerConnect::GetVolumeByUuid(std::string uuid, VolumeExternal &vol)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetVolumeByUuid:Connect error");
        return err;
    }
    return storageManager_->GetVolumeByUuid(uuid, vol);
}

int32_t StorageManagerConnect::GetVolumeById(std::string volumeId, VolumeExternal &vol)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::GetVolumeById:Connect error");
        return err;
    }
    return storageManager_->GetVolumeById(volumeId, vol);
}

int32_t StorageManagerConnect::SetVolumeDescription(std::string uuid, std::string description)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::SetVolumeDescription:Connect error");
        return err;
    }
    return storageManager_->SetVolumeDescription(uuid, description);
}

int32_t StorageManagerConnect::Format(std::string volumeId, std::string fsType)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::Format:Connect error");
        return err;
    }
    return storageManager_->Format(volumeId, fsType);
}

int32_t StorageManagerConnect::Partition(std::string diskId, int32_t type)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageManagerConnect::Partition:Connect error");
        return err;
    }
    return storageManager_->Partition(diskId, type);
}

int32_t StorageManagerConnect::ResetProxy()
{
    LOGD("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((storageManager_ != nullptr) && (storageManager_->AsObject() != nullptr)) {
        storageManager_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    storageManager_ = nullptr;

    return E_OK;
}

void SmDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DelayedSingleton<StorageManagerConnect>::GetInstance()->ResetProxy();
}

int32_t Convert2JsErrNum(int32_t errNum)
{
    if (errCodeTable.find(errNum) != errCodeTable.end()) {
        return errCodeTable.at(errNum);
    } else {
        return errNum;
    }
}
} // StorageManager
} // OHOS
