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
#include "utils/storage_manager_log.h"
#include "utils/storage_manager_errno.h"

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

int32_t MultiUserManagerService::PrepareAddUser(int32_t userId) const
{
    LOGI("MultiUserManagerService::PrepareAddUser, userId:%{public}d", userId);
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->PrepareAddUser(userId);
    return err;
}

int32_t MultiUserManagerService::RemoveUser(int32_t userId) const
{
    LOGI("MultiUserManagerService::RemoveUser, userId:%{public}d", userId);
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->RemoveUser(userId);
    return err;
}

int32_t MultiUserManagerService::PrepareStartUser(int32_t userId) const
{
    LOGI("MultiUserManagerService::PrepareStartUser, userId:%{public}d", userId);
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->PrepareStartUser(userId);
    return err;
}

int32_t MultiUserManagerService::StopUser(int32_t userId) const
{
    LOGI("MultiUserManagerService::StopUser, userId:%{public}d", userId);
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->StopUser(userId);
    return err;
}
} // StorageManager
} // OHOS