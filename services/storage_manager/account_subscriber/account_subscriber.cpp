/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "account_subscriber/account_subscriber.h"

#include "iservice_registry.h"
#include "int_wrapper.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "utils/storage_radar.h"

using namespace OHOS::AAFwk;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
static constexpr int CONNECT_TIME = 10;
static std::mutex mediaMutex_;
static std::mutex userRecordMutex_;
static const int32_t SLEEP_TIME_INTERVAL_1MS = 1000;

AccountSubscriber &AccountSubscriber::GetInstance()
{
    static AccountSubscriber instance;
    return instance;
}


static void SendSecondMountedEvent(int32_t userId)
{
    AAFwk::Want want;
    AAFwk::WantParams wantParams;
    wantParams.SetParam("userId", AAFwk::Integer::Box(userId));
    want.SetAction("usual.event.SECOND_MOUNTED");
    want.SetParams(wantParams);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
    LOGI("Send usual.event.SECOND_MOUNTED event success.");
}

static void MountCryptoPathAgain(int32_t userId)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->MountCryptoPathAgain(userId);
    if (err != 0) {
        LOGE("mount crypto path failed err is %{public}d", err);
        return;
    }
    SendSecondMountedEvent(userId);
    LOGI("MountCryptoPathAgain success");
}

void AccountSubscriber::ResetUserEventRecord(int32_t userId)
{
    if (userId < StorageService::START_USER_ID || userId > StorageService::MAX_USER_ID) {
        return;
    }
    LOGI("ResetUserEventRecord start, userId is %{public}d", userId);

    std::unique_lock<std::mutex> userLock(userRecordMutex_);
    if (userRecord_.find(userId) != userRecord_.end()) {
        userRecord_.erase(userId);
    }

    std::unique_lock<std::mutex> mediaLock(mediaMutex_);
    if (mediaShareMap_.find(userId) != mediaShareMap_.end()) {
        mediaShareMap_.erase(userId);
    }
}

void AccountSubscriber::NotifyUserChangedEvent(uint32_t userId, StorageService::UserChangedEventType eventType)
{
    std::unique_lock<std::mutex> lock(userRecordMutex_);
    /* get user status */
    uint32_t status = GetUserStatus(userId);
    /* update status */
    if (eventType == StorageService::UserChangedEventType::EVENT_USER_UNLOCKED) {
        status = HandleUserUnlockEvent(status);
    } else if (eventType == StorageService::UserChangedEventType::EVENT_USER_SWITCHED) {
        status = HandleUserSwitchedEvent(status);
    }
    userId_ = userId;
    userRecord_[userId] = status;
    LOGI("eventType:%{public}u, userId:%{public}d status:%{public}d", eventType, userId, status);
    if (status != (1 << USER_UNLOCK_BIT | 1 << USER_SWITCH_BIT)) {
        return;
    }

    if ((status & USER_UNLOCK_BIT) == USER_UNLOCK_BIT) {
        MountCryptoPathAgain(userId);
        userRecord_.erase(userId);
    }
    lock.unlock();
    std::thread mediaThread(&AccountSubscriber::GetSystemAbility, this);
    mediaThread.detach();
}

uint32_t AccountSubscriber::GetUserStatus(int32_t userId)
{
    uint32_t userStatus = 0;
    auto entry = userRecord_.find(userId);
    if (entry != userRecord_.end()) {
        userStatus = entry->second;
    }
    return userStatus;
}

uint32_t AccountSubscriber::HandleUserUnlockEvent(uint32_t userStatus)
{
    if (userStatus == (1 << USER_UNLOCK_BIT | 1 << USER_SWITCH_BIT)) {
        userStatus = 0;
    }
    userStatus |= 1 << USER_UNLOCK_BIT;
    return userStatus;
}

uint32_t AccountSubscriber::HandleUserSwitchedEvent(uint32_t userStatus)
{
    userStatus |= 1 << USER_SWITCH_BIT;
    /* clear previous user status */
    auto oldEntry = userRecord_.find(userId_);
    if (oldEntry != userRecord_.end()) {
        userRecord_[userId_] = oldEntry->second & (~USER_SWITCH_BIT);
    }
    return userStatus;
}

void AccountSubscriber::GetSystemAbility()
{
    std::lock_guard<std::mutex> lockMedia(mediaMutex_);
    LOGI("connect %{public}d media library", userId_);
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("GetSystemAbilityManager sam == nullptr");
        return;
    }
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOGE("GetSystemAbility remoteObj == nullptr");
        return;
    }
    for (int i = 0; i < CONNECT_TIME; i++) {
        std::shared_ptr<DataShare::DataShareHelper> mediaShare =
            DataShare::DataShareHelper::Creator(remoteObj, "datashare:///media");
        if (mediaShare != nullptr) {
            LOGI("connect media success.");
            mediaShareMap_[userId_] = mediaShare;
            break;
        }
        LOGE("try to connect media again.");
        usleep(SLEEP_TIME_INTERVAL_1MS);
    }
}
}  // namespace StorageManager
}  // namespace OHOS
