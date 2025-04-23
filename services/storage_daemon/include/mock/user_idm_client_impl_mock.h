/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_USER_IDM_CLIENT_MOCK_H
#define STORAGE_DAEMON_USER_IDM_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "user_idm_client.h"
#include "user_idm_client_impl.h"

namespace OHOS {
namespace StorageDaemon {
using namespace OHOS::UserIam::UserAuth;
class IUserIdmClientMoc {
public:
    virtual ~IUserIdmClientMoc() = default;
public:
    virtual std::vector<uint8_t> OpenSession(int32_t userId) = 0;
    virtual void CloseSession(int32_t userId) = 0;
    virtual void AddCredential(int32_t userId, const CredentialParameters &para,
        const std::shared_ptr<UserIdmClientCallback> &callback) = 0;
    virtual void UpdateCredential(int32_t userId, const CredentialParameters &para,
        const std::shared_ptr<UserIdmClientCallback> &callback) = 0;
    virtual int32_t Cancel(int32_t userId) = 0;
    virtual void DeleteCredential(int32_t userId, uint64_t credentialId, const std::vector<uint8_t> &authToken,
        const std::shared_ptr<UserIdmClientCallback> &callback) = 0;
    virtual void DeleteUser(int32_t userId, const std::vector<uint8_t> &authToken,
        const std::shared_ptr<UserIdmClientCallback> &callback) = 0;
    virtual int32_t EraseUser(int32_t userId, const std::shared_ptr<UserIdmClientCallback> &callback) = 0;
    virtual int32_t GetCredentialInfo(int32_t userId, AuthType authType,
        const std::shared_ptr<GetCredentialInfoCallback> &callback) = 0;
    virtual int32_t GetSecUserInfo(int32_t userId, const std::shared_ptr<GetSecUserInfoCallback> &callback) = 0;
    virtual void ClearRedundancyCredential(const std::shared_ptr<UserIdmClientCallback> &callback) = 0;
    virtual int32_t GetCredentialInfoSync(int32_t userId, AuthType authType,
        std::vector<CredentialInfo> &credentialInfoList) = 0;
    virtual int32_t RegistCredChangeEventListener(const std::vector<AuthType> &authType,
        const std::shared_ptr<CredChangeEventListener> &listener) = 0;
    virtual int32_t UnRegistCredChangeEventListener(const std::shared_ptr<CredChangeEventListener> &listener) = 0;

public:
    static inline std::shared_ptr<IUserIdmClientMoc> userIdmClientMoc = nullptr;
};

class UserIdmClientMoc : public IUserIdmClientMoc {
public:
    MOCK_METHOD1(OpenSession, std::vector<uint8_t>(int32_t userId));
    MOCK_METHOD1(CloseSession, void(int32_t userId));
    MOCK_METHOD3(AddCredential, void(int32_t userId, const CredentialParameters &para,
        const std::shared_ptr<UserIdmClientCallback> &callback));
    MOCK_METHOD3(UpdateCredential, void(int32_t userId, const CredentialParameters &para,
        const std::shared_ptr<UserIdmClientCallback> &callback));
    MOCK_METHOD1(Cancel, int32_t(int32_t userId));
    MOCK_METHOD4(DeleteCredential, void(int32_t userId, uint64_t credentialId,
        const std::vector<uint8_t> &authToken, const std::shared_ptr<UserIdmClientCallback> &callback));
    MOCK_METHOD3(DeleteUser, void(int32_t userId, const std::vector<uint8_t> &authToken,
        const std::shared_ptr<UserIdmClientCallback> &callback));
    MOCK_METHOD2(EraseUser, int32_t(int32_t userId, const std::shared_ptr<UserIdmClientCallback> &callback));
    MOCK_METHOD3(GetCredentialInfo, int32_t(int32_t userId, AuthType authType,
        const std::shared_ptr<GetCredentialInfoCallback> &callback));
    MOCK_METHOD2(GetSecUserInfo, int32_t(int32_t userId, const std::shared_ptr<GetSecUserInfoCallback> &callback));
    MOCK_METHOD1(ClearRedundancyCredential, void(const std::shared_ptr<UserIdmClientCallback> &callback));
    MOCK_METHOD3(GetCredentialInfoSync, int32_t(int32_t userId, AuthType authType,
        std::vector<CredentialInfo> &credentialInfoList));
    MOCK_METHOD2(RegistCredChangeEventListener, int32_t(const std::vector<AuthType> &authType,
        const std::shared_ptr<CredChangeEventListener> &listener));
    MOCK_METHOD1(UnRegistCredChangeEventListener, int32_t(const std::shared_ptr<CredChangeEventListener> &listener));
};
}
}
#endif