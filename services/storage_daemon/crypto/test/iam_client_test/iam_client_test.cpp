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
#include <thread>

#include "iam_client.h"
#include "storage_service_errno.h"
#include "user_idm_client_impl_mock.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS::StorageDaemon {
class IamClientTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
    static inline shared_ptr<UserIdmClientMoc> userIdmClientImplMock_ = nullptr;
};

void IamClientTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    userIdmClientImplMock_ = make_shared<UserIdmClientMoc>();
    UserIdmClientMoc::userIdmClientMoc = userIdmClientImplMock_;
}

void IamClientTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    UserIdmClientMoc::userIdmClientMoc = nullptr;
    userIdmClientImplMock_ = nullptr;
}

#ifdef USER_AUTH_FRAMEWORK
/**
 * @tc.name: iam_client_GetSecureUid_001
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_001 start";
    uint64_t secureUid = 1;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));

    EXPECT_TRUE(IamClient::GetInstance().GetSecureUid(1, secureUid));
    EXPECT_EQ(secureUid, 0);
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_001 end";
}

/**
 * @tc.name: iam_client_GetSecureUid_002
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_002 start";
    
    std::vector<UserIam::UserAuth::ResultCode> retCodes {
        UserIam::UserAuth::ResultCode::SUCCESS,
        UserIam::UserAuth::ResultCode::NOT_ENROLLED,
        UserIam::UserAuth::ResultCode::GENERAL_ERROR
    };

    for (auto retCode : retCodes) {
        uint64_t secureUid = 1;

        EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(retCode));

        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            IamClient::GetInstance().secureUidStatus_ = SUCCESS;
            IamClient::GetInstance().iamCon_.notify_one();
        }).detach();
        EXPECT_TRUE(IamClient::GetInstance().GetSecureUid(1, secureUid));
        EXPECT_EQ(secureUid, 0);
    }

    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_002 end";
}

/**
 * @tc.name: iam_client_GetSecureUid_003
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_003 start";

    std::vector<UserIam::UserAuth::ResultCode> retCodes {
        UserIam::UserAuth::ResultCode::SUCCESS,
        UserIam::UserAuth::ResultCode::NOT_ENROLLED,
        UserIam::UserAuth::ResultCode::GENERAL_ERROR
    };

    for (auto retCode : retCodes) {
        uint64_t secureUid = 1;

        EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
            .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
            .WillOnce(Return(retCode));

        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            IamClient::GetInstance().secureUidStatus_ = SUCCESS;
            IamClient::GetInstance().iamCon_.notify_one();
        }).detach();
        EXPECT_TRUE(IamClient::GetInstance().GetSecureUid(1, secureUid));
        EXPECT_EQ(secureUid, 0);
    }
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_003 end";
}

/**
 * @tc.name: iam_client_GetSecureUid_004
 * @tc.desc: Verify the iam_client GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecureUid_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_004 start";
    uint64_t secureUid = 1;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT));

    EXPECT_FALSE(IamClient::GetInstance().GetSecureUid(1, secureUid));
    GTEST_LOG_(INFO) << "iam_client_GetSecureUid_004 end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo_001
 * @tc.desc: Verify the iam_client GetSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_001 start";
    UserIam::UserAuth::SecUserInfo info;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::SUCCESS));

    EXPECT_TRUE(IamClient::GetInstance().GetSecUserInfo(1, info));
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_001 end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo_002
 * @tc.desc: Verify the iam_client GetSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_002 start";
    
    std::vector<UserIam::UserAuth::ResultCode> retCodes {
        UserIam::UserAuth::ResultCode::SUCCESS,
        UserIam::UserAuth::ResultCode::NOT_ENROLLED,
        UserIam::UserAuth::ResultCode::GENERAL_ERROR
    };

    for (auto retCode : retCodes) {
        UserIam::UserAuth::SecUserInfo info;

        EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(retCode));

        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            IamClient::GetInstance().secUserInfoState_ = SEC_USER_INFO_SUCCESS;
            IamClient::GetInstance().iamCon_.notify_one();
        }).detach();
        EXPECT_TRUE(IamClient::GetInstance().GetSecUserInfo(1, info));
    }

    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_002 end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo_003
 * @tc.desc: Verify the iam_client GetSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_003 start";

    std::vector<UserIam::UserAuth::ResultCode> retCodes {
        UserIam::UserAuth::ResultCode::SUCCESS,
        UserIam::UserAuth::ResultCode::NOT_ENROLLED,
        UserIam::UserAuth::ResultCode::GENERAL_ERROR
    };

    for (auto retCode : retCodes) {
        UserIam::UserAuth::SecUserInfo info;

        EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
            .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
            .WillOnce(Return(retCode));

        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            IamClient::GetInstance().secUserInfoState_ = SEC_USER_INFO_SUCCESS;
            IamClient::GetInstance().iamCon_.notify_one();
        }).detach();
        EXPECT_TRUE(IamClient::GetInstance().GetSecUserInfo(1, info));
    }
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_003 end";
}

/**
 * @tc.name: iam_client_GetSecUserInfo_004
 * @tc.desc: Verify the iam_client GetSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: IAVEX9
 */
HWTEST_F(IamClientTest, iam_client_GetSecUserInfo_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_004 start";
    UserIam::UserAuth::SecUserInfo info;

    EXPECT_CALL(*userIdmClientImplMock_, GetSecUserInfo(_, _))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT))
        .WillOnce(Return(UserIam::UserAuth::ResultCode::TIMEOUT));

    EXPECT_FALSE(IamClient::GetInstance().GetSecUserInfo(1, info));
    GTEST_LOG_(INFO) << "iam_client_GetSecUserInfo_004 end";
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
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        IamClient::GetInstance().secUserInfoState_ = SEC_USER_INFO_SUCCESS;
        IamClient::GetInstance().iamCon_.notify_one();
    }).detach();
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
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        IamClient::GetInstance().secUserInfoState_ = SEC_USER_INFO_SUCCESS;
        IamClient::GetInstance().iamCon_.notify_one();
    }).detach();
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