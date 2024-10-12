/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "iam_client.h"
#include "storage_service_errno.h"
#include "user_idm_client_impl_mock.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS::StorageDaemon {
class IamClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UserIdmClientMoc> userIdmClientImplMock_ = nullptr;
};

void IamClientTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    userIdmClientImplMock_ = make_shared<UserIdmClientMoc>();
    UserIdmClientMoc::userIdmClientMoc = userIdmClientImplMock_;
}

void IamClientTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    UserIdmClientMoc::userIdmClientMoc = nullptr;
    userIdmClientImplMock_ = nullptr;
}

void IamClientTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void IamClientTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: iam_client_GetSecureUid
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid start";
    uint32_t userId = 100;
    uint64_t secureUid = 1;
    #ifdef USER_AUTH_FRAMEWORK
    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));
    #endif
    IamClient &client = IamClient::GetInstance();
    EXPECT_TRUE(client.GetSecureUid(userId, secureUid));
    EXPECT_EQ(secureUid, 0);
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo start";
    uint32_t userId = 100;
    UserIam::UserAuth::SecUserInfo info;
    #ifdef USER_AUTH_FRAMEWORK
    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));
    #endif
    IamClient &client = IamClient::GetInstance();
    EXPECT_TRUE(client.GetSecUserInfo(userId, info));
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo end";
}

#ifdef USER_AUTH_FRAMEWORK
/**
 * @tc.name: iam_client_GetSecureUid
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid_failed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_failed start";
    uint32_t userId = 100;
    uint64_t secureUid = 1;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL));
    IamClient &client = IamClient::GetInstance();
    EXPECT_FALSE(client.GetSecureUid(userId, secureUid));
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_failed end";
}

/**
 * @tc.name: iam_client_GetSecureUid
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid_succ, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_failed start";
    uint32_t userId = 100;
    uint64_t secureUid = 1;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));
    IamClient &client = IamClient::GetInstance();
    EXPECT_TRUE(client.GetSecureUid(userId, secureUid));
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_failed end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo
 * @tc.desc: Verify the iam_client GetSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo_failed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_failed start";
    uint32_t userId = 100;
    UserIam::UserAuth::SecUserInfo info;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL));
    IamClient &client = IamClient::GetInstance();
    EXPECT_FALSE(client.GetSecUserInfo(userId, info));
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_failed end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo
 * @tc.desc: Verify the iam_client GetSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo_succ, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_failed start";
    uint32_t userId = 100;
    UserIam::UserAuth::SecUserInfo info;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));
    IamClient &client = IamClient::GetInstance();
    EXPECT_TRUE(client.GetSecUserInfo(userId, info));
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_failed end";
}

/**
 * @tc.name: iam_client_HasPinProtect_001
 * @tc.desc: Verify the iam_client HasPinProtect.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_HasPinProtect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_HasPinProtect_001 start";
    uint32_t userId = 100;
    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL));
    EXPECT_EQ(IamClient::GetInstance().HasPinProtect(userId), false);
    GTEST_LOG_(INFO) << "iam_client_HasPinProtect_001 end";
}

/**
 * @tc.name: iam_client_HasPinProtect_002
 * @tc.desc: Verify the iam_client HasPinProtect.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_HasPinProtect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_HasPinProtect_002 start";
    uint32_t userId = 100;
    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));
    EXPECT_EQ(IamClient::GetInstance().HasPinProtect(userId), false);
    GTEST_LOG_(INFO) << "iam_client_HasPinProtect_002 end";
}

/**
 * @tc.name: iam_client_HasFaceFinger_001
 * @tc.desc: Verify the iam_client HasFaceFinger.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_HasFaceFinger_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_HasFaceFinger_001 start";
    uint32_t userId = 100;
    bool isExist;
    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL)).WillOnce(Return(UserIam::UserAuth::ResultCode::FAIL));
    EXPECT_EQ(IamClient::GetInstance().HasFaceFinger(userId, isExist), -ENOENT);
    GTEST_LOG_(INFO) << "iam_client_HasFaceFinger_001 end";
}

/**
 * @tc.name: iam_client_HasFaceFinger_002
 * @tc.desc: Verify the iam_client HasFaceFinger.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_HasFaceFinger_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_HasFaceFinger_002 start";
    uint32_t userId = 100;
    bool isExist;
    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));
    EXPECT_EQ(IamClient::GetInstance().HasFaceFinger(userId, isExist), 0);
    GTEST_LOG_(INFO) << "iam_client_HasFaceFinger_002 end";
}
#endif

/**
 * @tc.name: iam_client_NotifyGetSecureUid
 * @tc.desc: Verify the iam_client NotifyGetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_NotifyGetSecureUid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_NotifyGetSecureUid start";
    IamClient &client = IamClient::GetInstance();
    EXPECT_EQ(client.secureUidStatus_, 1);
    int32_t result = client.NotifyGetSecureUid();
    EXPECT_EQ(result, 0);
    EXPECT_EQ(client.secureUidStatus_, 0);
    GTEST_LOG_(INFO) << "iam_client_NotifyGetSecureUid end";
}
} // OHOS::StorageDaemon