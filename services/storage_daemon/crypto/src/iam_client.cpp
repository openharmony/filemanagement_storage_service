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

#include "iam_client.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
IamClient::IamClient()
{
    LOGD("enter");
    secureUidStatus_ = FAILED;
}

IamClient::~IamClient()
{
    LOGD("enter");
}

void UserSecCallback::OnSecUserInfo(const UserIam::UserAuth::SecUserInfo &info)
{
    LOGI("enter");
    secureUid_ = info.secureUid;
    IamClient::GetInstance().NotifyGetSecureUid();
}

uint64_t UserSecCallback::GetSecureUid()
{
    LOGI("enter");
    return secureUid_;
}

bool IamClient::GetSecureUid(uint32_t userId, uint64_t &secureUid)
{
    LOGI("enter");
    secureUidStatus_ = FAILED;
    std::shared_ptr<UserSecCallback> secCallback = std::make_shared<UserSecCallback>();
    if (UserIam::UserAuth::UserIdmClient::GetInstance().GetSecUserInfo(userId, secCallback) !=
        UserIam::UserAuth::ResultCode::SUCCESS) {
        LOGE("Get secure uid failed !");
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
    LOGI("finish");
    return true;
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