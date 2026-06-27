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

#include "ipc/storage_manager_client.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"

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
    LOGI("[L1:StorageManagerClient] QueryActiveOsAccountIds: >>> ENTER <<< ids length=%{public}zu.", ids.size());
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
