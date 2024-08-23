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

#ifndef FSS_SECURITY_MERGE_IAMCLIENT_H
#define FSS_SECURITY_MERGE_IAMCLIENT_H

#include <condition_variable>
#include <mutex>

#ifdef USER_AUTH_FRAMEWORK
#include "iam_common_defines.h"
#include "user_idm_client.h"
#include "user_idm_client_callback.h"
#include "user_idm_client_defines.h"
#endif

namespace OHOS {
namespace StorageDaemon {

const int8_t GET_SEC_TIMEOUT = 10;

enum UserSecStatus {
    SUCCESS,
    FAILED
};

enum SecUserInfoState {
    SEC_USER_INFO_SUCCESS,
    SEC_USER_INFO_FAILED
};

#ifdef USER_AUTH_FRAMEWORK
class UserSecCallback : public UserIam::UserAuth::GetSecUserInfoCallback {
public:
    UserSecCallback()
    {
        secureUid_ = { 0 };
    }
    virtual ~UserSecCallback()
    {
        secureUid_ = { 0 };
    }
    void OnSecUserInfo(const UserIam::UserAuth::SecUserInfo &info) override;
    uint64_t GetSecureUid();

private:
    uint64_t secureUid_;
};

class UserEnrollCallback : public UserIam::UserAuth::GetSecUserInfoCallback {
public:
    UserEnrollCallback()
    {
        info_ = {};
    }
    virtual ~UserEnrollCallback()
    {
        info_ = {};
    }
    void OnSecUserInfo(const UserIam::UserAuth::SecUserInfo &info) override;
    UserIam::UserAuth::SecUserInfo GetSecUserInfo();

private:
    UserIam::UserAuth::SecUserInfo info_;
};

#endif

class IamClient {
public:
    static IamClient &GetInstance()
    {
        static IamClient instance;
        return instance;
    }

    bool GetSecureUid(uint32_t userId, uint64_t &secureUid);
    bool GetSecUserInfo(uint32_t userId, UserIam::UserAuth::SecUserInfo &info);
    int HasFaceFinger(uint32_t userId, bool &isExist);

    int32_t NotifyGetSecureUid();
    int32_t NotifyGetSecUserInfo();

private:
    IamClient();
    ~IamClient();
    IamClient(const IamClient &) = delete;
    IamClient &operator=(const IamClient &) = delete;

    SecUserInfoState secUserInfoState_;
    UserSecStatus secureUidStatus_;
    std::condition_variable iamCon_;
    std::mutex iamMutex_;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // FSS_SECURITY_MERGE_IAMCLIENT_H
