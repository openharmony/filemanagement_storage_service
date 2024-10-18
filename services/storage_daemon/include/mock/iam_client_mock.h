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
#ifndef STORAGE_DAEMON_IAM_CLIENT_MOCK_H
#define STORAGE_DAEMON_IAM_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "iam_client.h"

namespace OHOS {
namespace StorageDaemon {
class IIamClientMoc {
public:
    virtual ~IIamClientMoc() = default;
public:
    virtual int HasFaceFinger(uint32_t userId, bool &isExist) = 0;
    virtual bool GetSecureUid(uint32_t userId, uint64_t &secureUid) = 0;
    virtual bool GetSecUserInfo(uint32_t userId, UserIam::UserAuth::SecUserInfo &info) = 0;
    virtual bool HasPinProtect(uint32_t userId) = 0;
    virtual int32_t NotifyGetSecureUid() = 0;
    virtual int32_t NotifyGetSecUserInfo() = 0;
public:
    static inline std::shared_ptr<IIamClientMoc> iamClientMoc = nullptr;
};

class IamClientMoc : public IIamClientMoc {
public:
    MOCK_METHOD2(HasFaceFinger, int(uint32_t userId, bool &isExist));
    MOCK_METHOD2(GetSecureUid, bool(uint32_t userId, uint64_t &secureUid));
    MOCK_METHOD2(GetSecUserInfo, bool(uint32_t userId, UserIam::UserAuth::SecUserInfo &info));
    MOCK_METHOD1(HasPinProtect, bool(uint32_t userId));
    MOCK_METHOD0(NotifyGetSecureUid, int32_t());
    MOCK_METHOD0(NotifyGetSecUserInfo, int32_t());
};
}
}
#endif