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

using namespace testing::ext;
using namespace testing;

namespace OHOS::StorageDaemon {
class IamClientUserAuthFrameTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IamClientUserAuthFrameTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void IamClientUserAuthFrameTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void IamClientUserAuthFrameTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void IamClientUserAuthFrameTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: iam_client_user_auth_frame_GetSecureUid
 * @tc.desc: Verify the iam_client_user_auth_frame GetSecureUid.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(IamClientUserAuthFrameTest, iam_client_GetSecureUid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_user_auth_frame_GetSecureUid start";
    uint32_t userId = 0;
    uint64_t secureUid = 1;
    IamClient &client = IamClient::GetInstance();
    
    EXPECT_FALSE(client.GetSecureUid(userId, secureUid));
    EXPECT_EQ(secureUid, 1);

    userId = 100;
    EXPECT_FALSE(client.GetSecureUid(userId, secureUid));
    EXPECT_EQ(secureUid, 1);
    GTEST_LOG_(INFO) << "iam_client_user_auth_frame_GetSecureUid end";
}

/**
 * @tc.name: iam_client_user_auth_frame_OnSecUserInfo
 * @tc.desc: Verify the iam_client_user_auth_frame OnSecUserInfo.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(IamClientUserAuthFrameTest, iam_client_user_auth_frame_OnSecUserInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "iam_client_user_auth_frame_OnSecUserInfo start";
    UserIam::UserAuth::SecUserInfo info;
    UserSecCallback callback;
    info.secureUid = 2;
    callback.OnSecUserInfo(0, info);
    EXPECT_TRUE(true);
    EXPECT_EQ(callback.GetSecureUid(), 2);
    GTEST_LOG_(INFO) << "iam_client_user_auth_frame_OnSecUserInfo end";
}
} // OHOS::StorageDaemon