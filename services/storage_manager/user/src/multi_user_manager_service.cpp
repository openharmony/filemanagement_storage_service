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

#include "user/multi_user_manager_service.h"

#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
MultiUserManagerService::MultiUserManagerService()
{
    LOGI("DEBUG MultiUserManagerService constructer");
}

MultiUserManagerService::~MultiUserManagerService()
{
    LOGI("DEBUG ~MultiUserManagerService destructer ~");
}

int32_t MultiUserManagerService::CheckUserIdRange(int32_t userId)
{
    if (userId < StorageService::START_USER_ID || userId > StorageService::MAX_USER_ID) {
        LOGE("MultiUserManagerService: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

int32_t MultiUserManagerService::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("MultiUserManagerService::PrepareAddUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("MultiUserManagerService::PrepareAddUser userId %{public}d out of range", userId);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->PrepareAddUser(userId, flags);
    return err;
}

int32_t MultiUserManagerService::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("MultiUserManagerService::RemoveUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("MultiUserManagerService::RemoveUser userId %{public}d out of range", userId);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->RemoveUser(userId, flags);
    return err;
}

int32_t MultiUserManagerService::PrepareStartUser(int32_t userId)
{
    LOGI("MultiUserManagerService::PrepareStartUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("MultiUserManagerService::PrepareStartUser userId %{public}d out of range", userId);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->PrepareStartUser(userId);
    return err;
}

int32_t MultiUserManagerService::StopUser(int32_t userId)
{
    LOGI("MultiUserManagerService::StopUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("MultiUserManagerService::StopUser userId %{public}d out of range", userId);
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->StopUser(userId);
    return err;
}
} // StorageManager
} // OHOS
