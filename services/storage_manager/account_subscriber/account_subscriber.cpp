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

#include "account_subscriber/account_subscriber.h"

#include <condition_variable>
#include <cstring>
#include <sstream>

#include "iservice_registry.h"
#include "int_wrapper.h"
#include "appspawn.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "system_ability_definition.h"
#include "utils/storage_radar.h"
#include "parameter.h"

using namespace OHOS::AAFwk;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
static constexpr int CONNECT_TIME = 10;
static std::mutex mediaMutex_;
static std::mutex userRecordMutex_;
static const int32_t SLEEP_TIME_INTERVAL_1MS = 1000;
static constexpr bool DECRYPTED = false;
static constexpr const char *SYS_PARAM_APPSPAWN_UNLOCK_MOUNT = "startup.appspawn.unlock_mount.";
static const int32_t MOUNT_MAX_WAIT_TIME = 5;

AccountSubscriber &AccountSubscriber::GetInstance()
{
    static AccountSubscriber instance;
    return instance;
}

void AccountSubscriber::SendSecondMountedEvent(int32_t userId)
{
    AAFwk::Want want;
    AAFwk::WantParams wantParams;
    wantParams.SetParam("userId", AAFwk::Integer::Box(userId));
    want.SetAction("usual.event.SECOND_MOUNTED");
    want.SetParams(wantParams);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
    LOGI("Send usual.event.SECOND_MOUNTED event success, userId=%{public}d", userId);
}

int32_t AccountSubscriber::SendUserLockStatusToAppSpawn(int32_t userId, bool lockStatus)
{
    std::lock_guard<std::mutex> lock(appSpawnMutex_);
    std::string lockStatusStr = lockStatus ? "ENCRYPTED" : "DECRYPTED";
    LOGI("SendUserLockStatusToAppSpawn begin, userId=%{public}d, lockStatus=%{public}s",
        userId, lockStatusStr.c_str());

    int32_t ret = AppSpawnClientSendUserLockStatus(userId, lockStatus);
    if (ret != E_OK) {
        LOGE("SendUserLockStatusToAppSpawn failed, userId=%{public}d, lockStatus=%{public}s, ret=%{public}d",
            userId, lockStatusStr.c_str(), ret);
    } else {
        LOGI("SendUserLockStatusToAppSpawn success, userId=%{public}d, lockStatus=%{public}s",
            userId, lockStatusStr.c_str());
    }
    return ret;
}

void AccountSubscriber::MountCryptoPathAgain(int32_t userId)
{
    LOGI("MountCryptoPathAgain begin, userId=%{public}d", userId);
    StorageService::StorageRadar::ReportFucBehavior("MountCryptoPathAgain", userId,
        "MountCryptoPathAgain Begin", E_OK);
    SetParam(userId, PARAM_STATUS::PARAM_UNKNOWN);
    std::string paramKey = SYS_PARAM_APPSPAWN_UNLOCK_MOUNT + std::to_string(userId);
    int ret = WatchParameter(paramKey.c_str(), OnUnlockParamChanged, this);
    if (ret != 0) {
        LOGE("MountCryptoPathAgain::WatchParameter failed, userId=%{public}d, watchRet=%{public}d", userId, ret);
    }
    int32_t err = SendUserLockStatusToAppSpawn(userId, DECRYPTED);
    if (err != E_OK) {
        ret = RemoveParameterWatcher(paramKey.c_str(), OnUnlockParamChanged, this);
        LOGE("MountCryptoPathAgain failed, userId=%{public}d, err=%{public}d, watchRet=%{public}d",
            userId, err, ret);
        StorageService::StorageRadar::ReportUserManager("MountCryptoPathAgain::SendUserLockStatusToAppSpawn",
            userId, err, "");
        RmParam(userId);
        return;
    }
    bool success = false;
    {
        std::unique_lock<std::mutex> lock(waitMutex_);
        success = waitCv_.wait_for(lock, std::chrono::seconds(MOUNT_MAX_WAIT_TIME),
            [this, userId] { return paramChangedMap_[userId] != PARAM_STATUS::PARAM_UNKNOWN; });
    }
    ret = RemoveParameterWatcher(paramKey.c_str(), OnUnlockParamChanged, this);
    if (!success) {
        LOGE("MountCryptoPathAgain wait param timeout, userId=%{public}d, rmWatcherRet=%{public}d",
            userId, ret);
        StorageService::StorageRadar::ReportUserManager("MountCryptoPathAgain::WaitParamTimeout",
            userId, E_TIMEOUT_MOUNT, "");
        RmParam(userId);
        return;
    }
    if (paramChangedMap_[userId] != PARAM_STATUS::PARAM_SUCCESS) {
        LOGE("MountCryptoPathAgain param result failed, userId=%{public}d, rmWatcherRet=%{public}d", userId, ret);
        StorageService::StorageRadar::ReportUserManager("MountCryptoPathAgain::ParamResultFailed",
            userId, E_MOUNT_AGAIN_FAILED, "");
        RmParam(userId);
        return;
    }
    SendSecondMountedEvent(userId);
    LOGI("MountCryptoPathAgain success, userId=%{public}d, rmWatcherRet=%{public}d", userId, ret);
    StorageService::StorageRadar::ReportFucBehavior("MountCryptoPathAgain", userId,
        "MountCryptoPathAgain End", E_OK);
    RmParam(userId);
}

void AccountSubscriber::OnUnlockParamChanged(const char *key, const char *value, void *context)
{
    LOGI("OnUnlockParamChanged key=%{public}s, value=%{public}s",
        key ? key : "null", value ? value : "null");
    if (key == nullptr || value == nullptr) {
        LOGE("OnUnlockParamChanged invalid parameters");
        return;
    }
    std::string paramKey = key;
    int32_t userId = -1;
    if (paramKey.find(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT) != 0) {
        LOGE("OnUnlockParamChanged invalid key: %{public}s", key);
        return;
    }
    std::string userIdStr = paramKey.substr(strlen(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT));
    if (userIdStr.empty()) {
        LOGE("OnUnlockParamChanged key:%{public}s without userId", key);
        return;
    }
    auto it = std::find_if(userIdStr.begin(), userIdStr.end(), [](char c) { return !std::isdigit(c); });
    if (it != userIdStr.end()) {
        LOGE("OnUnlockParamChanged key: %{public}s check failed", key);
        return;
    }
    std::istringstream ss(userIdStr);
    if (!(ss >> userId)) {
        LOGE("OnUnlockParamChanged invalid userId: %{public}s", userIdStr.c_str());
        return;
    }
    bool isParamExist = AccountSubscriber::GetInstance().IsParamExist(userId);
    PARAM_STATUS paramStatus = (strcmp(value, "0") == 0) ? PARAM_SUCCESS : PARAM_FAIL;
    if (isParamExist) {
        AccountSubscriber::GetInstance().SetParam(userId, paramStatus);
        AccountSubscriber::GetInstance().NotifyAll();
    }
    LOGI("OnUnlockParamChanged userId=%{public}d, paramStatus=%{public}d",
        userId, static_cast<int>(paramStatus));
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
        LOGE("try to connect media again, retry count: %{public}d/%{public}d", i + 1, CONNECT_TIME);
        usleep(SLEEP_TIME_INTERVAL_1MS);
    }
}

void AccountSubscriber::NotifyAll()
{
    std::lock_guard<std::mutex> lock(waitMutex_);
    waitCv_.notify_all();
}

void AccountSubscriber::SetParam(int32_t userId, PARAM_STATUS paramChangedStatus)
{
    std::lock_guard<std::mutex> lock(waitMutex_);
    paramChangedMap_[userId] = paramChangedStatus;
}

bool AccountSubscriber::IsParamExist(int32_t userId)
{
    std::lock_guard<std::mutex> lock(waitMutex_);
    return paramChangedMap_.count(userId) > 0;
}

void AccountSubscriber::RmParam(int32_t userId)
{
    std::lock_guard<std::mutex> lock(waitMutex_);
    paramChangedMap_.erase(userId);
}
}  // namespace StorageManager
}  // namespace OHOS
