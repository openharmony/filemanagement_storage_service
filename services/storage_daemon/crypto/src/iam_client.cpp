/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>

#include "iam_client.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr uint8_t IAM_MAX_RETRY_TIME = 3;
constexpr uint16_t IAM_RETRY_INTERVAL_MS = 50 * 1000;
constexpr int8_t GET_SEC_TIMEOUT = 10;
IamClient::IamClient()
{
    LOGI("enter");
}

IamClient::~IamClient()
{
    LOGI("enter");
}

#ifdef USER_AUTH_FRAMEWORK
void UserSecCallback::OnSecUserInfo(int32_t result, const UserIam::UserAuth::SecUserInfo &info)
{
    static_cast<void>(result);
    LOGI("enter");
    secureUid_ = info.secureUid;
    IamClient::GetInstance().NotifyGetSecureUid();
}

uint64_t UserSecCallback::GetSecureUid()
{
    LOGI("enter");
    return secureUid_;
}

void UserEnrollCallback::OnSecUserInfo(int32_t result, const UserIam::UserAuth::SecUserInfo &info)
{
    static_cast<void>(result);
    LOGI("enter");
    info_ = info;
    IamClient::GetInstance().NotifyGetSecUserInfo();
}

UserIam::UserAuth::SecUserInfo UserEnrollCallback::GetSecUserInfo()
{
    LOGI("enter");
    return info_;
}
#endif

bool IamClient::GetSecureUid(uint32_t userId, uint64_t &secureUid)
{
    LOGI("enter");
#ifdef USER_AUTH_FRAMEWORK
    LOGI("get secure uid real !");
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    secureUidStatus_ = FAILED;
    std::shared_ptr<UserSecCallback> secCallback = std::make_shared<UserSecCallback>();
    int32_t retCode = UserIam::UserAuth::UserIdmClient::GetInstance().GetSecUserInfo(userId, secCallback);
    if (retCode != UserIam::UserAuth::ResultCode::SUCCESS && retCode != UserIam::UserAuth::ResultCode::NOT_ENROLLED &&
        retCode != UserIam::UserAuth::ResultCode::GENERAL_ERROR) {
        for (int i = 0; i < IAM_MAX_RETRY_TIME; ++i) {
            usleep(IAM_RETRY_INTERVAL_MS);
            retCode = UserIam::UserAuth::UserIdmClient::GetInstance().GetSecUserInfo(userId, secCallback);
            LOGE("GetSecureUid has retry %{public}d times, retryRet %{public}d", i, retCode);
            if (retCode == UserIam::UserAuth::ResultCode::SUCCESS ||
                retCode == UserIam::UserAuth::ResultCode::NOT_ENROLLED ||
                retCode == UserIam::UserAuth::ResultCode::GENERAL_ERROR) {
                break;
            }
        }
    }
    if (retCode != UserIam::UserAuth::ResultCode::SUCCESS && retCode != UserIam::UserAuth::ResultCode::NOT_ENROLLED &&
        retCode != UserIam::UserAuth::ResultCode::GENERAL_ERROR) {
        LOGE("Get secure uid failed !");
        StorageRadar::ReportIamResult("GetSecureUid", userId, retCode);
        return false;
    }
    std::unique_lock<std::mutex> lock(iamMutex_);
    iamCon_.wait_for(lock, std::chrono::seconds(GET_SEC_TIMEOUT), [this] {
        return (this->secureUidStatus_ == SUCCESS);
    });
    if (secureUidStatus_ == FAILED) {
        LOGE("Get secure uid failed, use default !");
    }
    secureUid = secCallback->GetSecureUid();
    auto delay = StorageService::StorageRadar::ReportDuration("IAM: GET SECURE UID",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userId);
    LOGI("SD_DURATION: IAM: GET SECUR UID, delay time = %{public}s", delay.c_str());
#else
    LOGI("iam not support, use default !");
    secureUid = { 0 };
#endif
    LOGI("finish");
    return true;
}

bool IamClient::GetSecUserInfo(uint32_t userId, UserIam::UserAuth::SecUserInfo &info)
{
    LOGI("enter");
#ifdef USER_AUTH_FRAMEWORK
    LOGI("Get SecUserInfo real !");
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    secUserInfoState_ = SEC_USER_INFO_FAILED;
    std::shared_ptr<UserEnrollCallback> userEnrollCallback = std::make_shared<UserEnrollCallback>();
    int32_t retCode = UserIam::UserAuth::UserIdmClient::GetInstance().GetSecUserInfo(userId, userEnrollCallback);
    if (retCode != UserIam::UserAuth::ResultCode::SUCCESS && retCode != UserIam::UserAuth::ResultCode::NOT_ENROLLED &&
        retCode != UserIam::UserAuth::ResultCode::GENERAL_ERROR) {
        for (int i = 0; i < IAM_MAX_RETRY_TIME; ++i) {
            usleep(IAM_RETRY_INTERVAL_MS);
            retCode = UserIam::UserAuth::UserIdmClient::GetInstance().GetSecUserInfo(userId, userEnrollCallback);
            LOGE("GetSecureUid has retry %{public}d times, retryRet %{public}d", i, retCode);
            if (retCode == UserIam::UserAuth::ResultCode::SUCCESS ||
                retCode == UserIam::UserAuth::ResultCode::NOT_ENROLLED ||
                retCode == UserIam::UserAuth::ResultCode::GENERAL_ERROR) {
                break;
            }
        }
    }
    if (retCode != UserIam::UserAuth::ResultCode::SUCCESS && retCode != UserIam::UserAuth::ResultCode::NOT_ENROLLED &&
        retCode != UserIam::UserAuth::ResultCode::GENERAL_ERROR) {
        LOGE("Get SecUserInfo failed !");
        return false;
    }
    std::unique_lock<std::mutex> lock(iamMutex_);
    iamCon_.wait_for(lock, std::chrono::seconds(GET_SEC_TIMEOUT),
                     [this] { return (this->secUserInfoState_ == SEC_USER_INFO_SUCCESS); });
    if (secUserInfoState_ == SEC_USER_INFO_FAILED) {
        LOGE("Get SecUserInfo failed, use default !");
    }
    info = userEnrollCallback->GetSecUserInfo();
    auto delay = StorageService::StorageRadar::ReportDuration("IAM: GET SECURE USER INFO",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userId);
    LOGI("SD_DURATION: IAM: GET SECURE USER INFO, delay time = %{public}s", delay.c_str());
#else
    LOGI("iam not support, use default !");
    info = {};
#endif
    LOGI("finish");
    return true;
}

int IamClient::HasFaceFinger(uint32_t userId, bool &isExist)
{
    isExist = false;
    UserIam::UserAuth::SecUserInfo info;
    if (!GetSecUserInfo(userId, info)) {
        LOGE("Get SecUserInfo failed!");
        return -ENOENT;
    }
    std::vector<UserIam::UserAuth::EnrolledInfo> enrollInfo = info.enrolledInfo;
    for (auto &item : enrollInfo) {
        LOGI("EnrollInfo authType is : %{public}d", item.authType);
        if (item.authType == UserIam::UserAuth::FACE || item.authType == UserIam::UserAuth::FINGERPRINT) {
            LOGI("The user: %{public}d have authType: %{public}d", userId, item.authType);
            isExist = true;
            return 0;
        }
    }
    LOGI("The Face And FingerPrint are not exist !");
    return 0;
}

bool IamClient::HasPinProtect(uint32_t userId)
{
    UserIam::UserAuth::SecUserInfo info;
    if (!GetSecUserInfo(userId, info)) {
        LOGE("Get SecUserInfo failed!");
        return false;
    }
    std::vector<UserIam::UserAuth::EnrolledInfo> enrollInfo = info.enrolledInfo;
    for (auto &item : enrollInfo) {
        if (item.authType == UserIam::UserAuth::PIN) {
            LOGI("The device have pin protect for userId = %{public}d.", userId);
            return true;
        }
    }
    LOGI("The device has no pin protect for userId = %{public}d.", userId);
    return false;
}

int32_t IamClient::NotifyGetSecUserInfo()
{
    std::lock_guard<std::mutex> lock(iamMutex_);
    secUserInfoState_ = SEC_USER_INFO_SUCCESS;
    iamCon_.notify_one();
    return 0;
}

int32_t IamClient::NotifyGetSecureUid()
{
    std::lock_guard<std::mutex> lock(iamMutex_);
    secureUidStatus_ = SUCCESS;
    iamCon_.notify_one();
    return 0;
}
} // namespace StorageDaemon
} // namespace HOHS
