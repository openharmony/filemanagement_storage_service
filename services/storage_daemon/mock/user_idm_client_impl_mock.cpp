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

#include "user_idm_client_impl_mock.h"

using namespace std;
using namespace OHOS::StorageDaemon;
using namespace OHOS::UserIam::UserAuth;

UserIdmClientImpl &UserIdmClientImpl::Instance()
{
    static UserIdmClientImpl impl;
    return impl;
}

UserIdmClient &UserIdmClient::GetInstance()
{
    return UserIdmClientImpl::Instance();
}

int32_t UserIdmClientImpl::GetSecUserInfo(int32_t userId, const std::shared_ptr<GetSecUserInfoCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->GetSecUserInfo(userId, callback);
}

std::vector<uint8_t> UserIdmClientImpl::OpenSession(int32_t userId)
{
    return IUserIdmClientMoc::userIdmClientMoc->OpenSession(userId);
}

void UserIdmClientImpl::CloseSession(int32_t userId)
{
    return IUserIdmClientMoc::userIdmClientMoc->CloseSession(userId);
}

void UserIdmClientImpl::AddCredential(int32_t userId, const CredentialParameters &para,
    const std::shared_ptr<UserIdmClientCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->AddCredential(userId, para, callback);
}

void UserIdmClientImpl::UpdateCredential(int32_t userId, const CredentialParameters &para,
    const std::shared_ptr<UserIdmClientCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->UpdateCredential(userId, para, callback);
}

int32_t UserIdmClientImpl::Cancel(int32_t userId)
{
    return IUserIdmClientMoc::userIdmClientMoc->Cancel(userId);
}

void UserIdmClientImpl::DeleteCredential(int32_t userId, uint64_t credentialId,
    const std::vector<uint8_t> &authToken, const std::shared_ptr<UserIdmClientCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->DeleteCredential(userId, credentialId, authToken, callback);
}

void UserIdmClientImpl::DeleteUser(int32_t userId, const std::vector<uint8_t> &authToken,
    const std::shared_ptr<UserIdmClientCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->DeleteUser(userId, authToken, callback);
}

int32_t UserIdmClientImpl::EraseUser(int32_t userId, const std::shared_ptr<UserIdmClientCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->EraseUser(userId, callback);
}

int32_t UserIdmClientImpl::GetCredentialInfo(int32_t userId, AuthType authType,
    const std::shared_ptr<GetCredentialInfoCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->GetCredentialInfo(userId, authType, callback);
}

void UserIdmClientImpl::ClearRedundancyCredential(const std::shared_ptr<UserIdmClientCallback> &callback)
{
    return IUserIdmClientMoc::userIdmClientMoc->ClearRedundancyCredential(callback);
}

int32_t UserIdmClientImpl::GetCredentialInfoSync(int32_t userId, AuthType authType,
    std::vector<CredentialInfo> &credentialInfoList)
{
    return IUserIdmClientMoc::userIdmClientMoc->GetCredentialInfoSync(userId, authType, credentialInfoList);
}

int32_t UserIdmClientImpl::RegistCredChangeEventListener(const std::vector<AuthType> &authType,
    const std::shared_ptr<CredChangeEventListener> &listener)
{
    return IUserIdmClientMoc::userIdmClientMoc->RegistCredChangeEventListener(authType, listener);
}

int32_t UserIdmClientImpl::UnRegistCredChangeEventListener(const std::shared_ptr<CredChangeEventListener> &listener)
{
    return IUserIdmClientMoc::userIdmClientMoc->UnRegistCredChangeEventListener(listener);
}