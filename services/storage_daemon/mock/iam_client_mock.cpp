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

#include "iam_client_mock.h"

using namespace std;

using namespace OHOS::StorageDaemon;

IamClient::IamClient()
{
}

IamClient::~IamClient()
{
}

bool IamClient::GetSecureUid(uint32_t userId, uint64_t &secureUid)
{
    if (IIamClientMoc::iamClientMoc == nullptr) {
        return true;
    }
    return IIamClientMoc::iamClientMoc->GetSecureUid(userId, secureUid);
}

bool IamClient::GetSecUserInfo(uint32_t userId, UserIam::UserAuth::SecUserInfo &info)
{
    if (IIamClientMoc::iamClientMoc == nullptr) {
        return true;
    }
    return IIamClientMoc::iamClientMoc->GetSecUserInfo(userId, info);
}

int IamClient::HasFaceFinger(uint32_t userId, bool &isExist)
{
    if (IIamClientMoc::iamClientMoc == nullptr) {
        return 0;
    }
    return IIamClientMoc::iamClientMoc->HasFaceFinger(userId, isExist);
}

bool IamClient::HasPinProtect(uint32_t userId)
{
    if (IIamClientMoc::iamClientMoc == nullptr) {
        return true;
    }
    return IIamClientMoc::iamClientMoc->HasPinProtect(userId);
}

int32_t IamClient::NotifyGetSecUserInfo()
{
    if (IIamClientMoc::iamClientMoc == nullptr) {
        return 0;
    }
    return IIamClientMoc::iamClientMoc->NotifyGetSecUserInfo();
}

int32_t IamClient::NotifyGetSecureUid()
{
    if (IIamClientMoc::iamClientMoc == nullptr) {
        return 0;
    }
    return IIamClientMoc::iamClientMoc->NotifyGetSecureUid();
}
