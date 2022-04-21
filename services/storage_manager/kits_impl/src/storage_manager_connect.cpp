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

#include "storage/storage_status_service.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "ipc/storage_manager_proxy.h"
#include "ipc/storage_manager.h"

using namespace std;

namespace OHOS {
namespace StorageManager {
StorageManagerConnect::StorageManagerConnect() {}
StorageManagerConnect::~StorageManagerConnect() {}

int32_t StorageManagerConnect::Connect()
{
    LOGI("StorageManagerConnect::Connect start");
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("StorageManagerConnect::Connect samgr == nullptr");
        return E_IPC_ERROR;
    }
    auto object = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (object == nullptr) {
        LOGE("StorageManagerConnect::Connect object == nullptr");
        return E_IPC_ERROR;
    }
    storageManager_ = iface_cast<StorageManager::IStorageManager>(object);
    if (storageManager_ == nullptr) {
        LOGE("StorageManagerConnect::Connect service == nullptr");
        return E_IPC_ERROR;
    }
    LOGI("StorageManagerConnect::Connect end");
    return E_OK;
}

BundleStats StorageManagerConnect::GetBundleStats(string pkgName)
{
    BundleStats result;
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetBundleStats:Connect error");
        return result;
    }
    return storageManager_->GetBundleStats(pkgName);
}

int64_t StorageManagerConnect::GetFreeSizeOfVolume(string volumeUuid)
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetFreeSizeOfVolume:Connect error");
        return 0;
    }
    return storageManager_->GetFreeSizeOfVolume(volumeUuid);
}

int64_t StorageManagerConnect::GetTotalSizeOfVolume(string volumeUuid)
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetTotalSizeOfVolume:Connect error");
        return 0;
    }
    return storageManager_->GetTotalSizeOfVolume(volumeUuid);
}

bool StorageManagerConnect::Mount(std::string volumeId)
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::Mount:Connect error");
        return E_IPC_ERROR;
    }
    if (storageManager_->Mount(volumeId) == E_OK) {
        LOGE("StorageManagerConnect::Mount:success");
        return true;
    }
    return false;
}

bool StorageManagerConnect::Unmount(std::string volumeId)
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::Unmount:Connect error");
        return E_IPC_ERROR;
    }
    if (storageManager_->Unmount(volumeId) == E_OK) {
        LOGE("StorageManagerConnect::Unmount:success");
        return true;
    }
    return false;
}

std::vector<VolumeExternal> StorageManagerConnect::GetAllVolumes()
{
    vector<VolumeExternal> result = {};
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetAllVolumes:Connect error");
        return result;
    }
    return storageManager_->GetAllVolumes();
}

int64_t StorageManagerConnect::GetSystemSize()
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetSystemSize:Connect error");
        return 0;
    }
    int64_t result = storageManager_->GetSystemSize();
    return result;
}

int64_t StorageManagerConnect::GetTotalSize()
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetTotalSize:Connect error");
        return 0;
    }
    int64_t result = storageManager_->GetTotalSize();
    return result;
}

int64_t StorageManagerConnect::GetFreeSize()
{
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetFreeSize:Connect error");
        return 0;
    }
    int64_t result = storageManager_->GetFreeSize();
    return result;
}

StorageStats StorageManagerConnect::GetUserStorageStats()
{
    StorageStats result;
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetUserStorageStats:Connect error");
        return result;
    }
    result = storageManager_->GetUserStorageStats();
    return result;
}
    
StorageStats StorageManagerConnect::GetUserStorageStats(int32_t userId)
{
    StorageStats result;
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetUserStorageStats:Connect error");
        return result;
    }
    result = storageManager_->GetUserStorageStats(userId);
    return result;
}

BundleStats StorageManagerConnect::GetCurrentBundleStats()
{
    BundleStats result;
    if (Connect() != E_OK) {
        LOGE("StorageManagerConnect::GetCurrentBundleStats:Connect error");
        return result;
    }
    result = storageManager_->GetCurrentBundleStats();
    return result;
}
} // StorageManager
} // OHOS
