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

#include "ipc/storage_manager.h"
#include "system_ability_definition.h"
#include "utils/storage_manager_log.h"
#include "utils/storage_manager_errno.h"
#include "user/multi_user_manager_service.h"
#include <storage/storage_status_service.h>
#include <storage/storage_total_status_service.h>
#include <singleton.h>

namespace OHOS {
namespace StorageManager {
REGISTER_SYSTEM_ABILITY_BY_ID(StorageManager, STORAGE_MANAGER_MANAGER_ID, true);

void StorageManager::OnStart()
{
    LOGI("StorageManager::OnStart Begin");
    bool res = SystemAbility::Publish(this);
    LOGI("StorageManager::OnStart End, res = %{public}d", res);
}

void StorageManager::OnStop()
{
    LOGI("StorageManager::Onstop Done");
}

int32_t StorageManager::PrepareAddUser(int32_t userId)
{
    LOGI("StorageManager::PrepareAddUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->PrepareAddUser(userId);
    return err;
}

int32_t StorageManager::RemoveUser(int32_t userId)
{
    LOGI("StorageManger::RemoveUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->RemoveUser(userId);
    return err;
}

int32_t StorageManager::PrepareStartUser(int32_t userId)
{
    LOGI("StorageManger::PrepareStartUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->PrepareStartUser(userId);
    return err;
}

int32_t StorageManager::StopUser(int32_t userId)
{
    LOGI("StorageManger::StopUser start, userId: %{public}d", userId);
    std::shared_ptr<MultiUserManagerService> userManager = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t err = userManager->StopUser(userId);
    return err;
}

int64_t StorageManager::GetFreeSizeOfVolume(std::string volumeUuid)
{
    LOGI("StorageManger::getFreeSizeOfVolume start, volumeUuid: %{public}s", volumeUuid.c_str());
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetFreeSizeOfVolume(volumeUuid);
    return result;
}

int64_t StorageManager::GetTotalSizeOfVolume(std::string volumeUuid)
{
    LOGI("StorageManger::getTotalSizeOfVolume start, volumeUuid: %{public}s", volumeUuid.c_str());
    int64_t result = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSizeOfVolume(volumeUuid);
    return result;
}

std::vector<int64_t> StorageManager::GetBundleStats(std::string uuid, std::string pkgName)
{
    LOGI("StorageManger::getBundleStats start, pkgName: %{public}s", pkgName.c_str());
    std::vector<int64_t> result = DelayedSingleton<StorageStatusService>::GetInstance()->GetBundleStats(uuid, pkgName);
    return result;
}
}
}