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
#include "os_account_constants.h"
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

int32_t MultiUserManagerService::CheckUserIdRange(int32_t userId)
{
    if (userId < AccountSA::Constants::START_USER_ID || userId > AccountSA::Constants::MAX_USER_ID) {
        LOGE("MultiUserManagerService: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

int32_t MultiUserManagerService::CheckUserState(int32_t userId, UserState state)
{   
    auto iterator = users_.find(userId);
    if (iterator == users_.end()) {
        LOGE("MultiUserManagerService: the user %{public}d doesn't exist", userId);
        return E_NON_EXIST;
    }

    UserInfo& user = iterator->second;
    if (user.GetState() != state) {
        LOGE("MultiUserManagerService: the user's state %{public}d is invalid", user.GetState());
        return E_USER_STATE;
    }

    return E_OK;
}

inline void MultiUserManagerService::SetUserState(int32_t userId, UserState state)
{
    users_.at(userId).SetState(state);
}

int32_t MultiUserManagerService::PrepareAddUser(int32_t userId)
{
    LOGI("MultiUserManagerService::PrepareAddUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("MultiUserManagerService::PrepareAddUser userId %{public}d out of range", userId);
        return err;
    }
    auto iterator = users_.find(userId);
    if (iterator != users_.end()) {
        LOGE("MultiUserManagerService: the user %{public}d already exists", userId);
        return E_EXIST;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->PrepareAddUser(userId);
    users_.insert({ userId, UserInfo(userId, USER_CREATE)});
    return err;
}

int32_t MultiUserManagerService::RemoveUser(int32_t userId)
{
    LOGI("MultiUserManagerService::RemoveUser, userId:%{public}d", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("MultiUserManagerService::RemoveUser userId %{public}d out of range", userId);
        return err;
    }
    err = CheckUserState(userId, USER_CREATE);
    if (err != E_OK) {
        return err;
    }    
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->RemoveUser(userId);
    users_.erase(userId);
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
    err = CheckUserState(userId, USER_CREATE);
    if (err != E_OK) {
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->PrepareStartUser(userId);
    SetUserState(userId, USER_START);
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
    err = CheckUserState(userId, USER_START);
    if (err != E_OK) {
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->StopUser(userId);
    SetUserState(userId, USER_CREATE);
    return err;
}
} // StorageManager
} // OHOS