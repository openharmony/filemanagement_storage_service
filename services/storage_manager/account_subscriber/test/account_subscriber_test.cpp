/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <gtest/gtest.h>

#include "account_subscriber/account_subscriber.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace StorageManager {
using namespace std;
using namespace testing::ext;

class AccountSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<AccountSubscriber> accountSubscriberPtr_ = nullptr;
};

void AccountSubscriberTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup";
}

void AccountSubscriberTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown";
}

void AccountSubscriberTest::SetUp()
{
    accountSubscriberPtr_ = make_shared<AccountSubscriber>();
}

void AccountSubscriberTest::TearDown()
{
    accountSubscriberPtr_ = nullptr;
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_Subscriber_0000
* @tc.name: Account_Subscriber_Subscriber_0000
* @tc.desc: Test function of Subscriber interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_Subscriber_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_Subscriber_0000-begin";
    //accountSubscriber_ == nullptr
    AccountSubscriber::Subscriber();
    //accountSubscriber_ != nullptr
    AccountSubscriber::Subscriber();
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Account_Subscriber_Subscriber_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_ResetUserEventRecord_0000
* @tc.name: Account_Subscriber_ResetUserEventRecord_0000
* @tc.desc: Test function of ResetUserEventRecord interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_ResetUserEventRecord_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_ResetUserEventRecord_0000-begin";

    int32_t userId = 99;
    AccountSubscriber::ResetUserEventRecord(userId);
    userId = 10737;
    AccountSubscriber::ResetUserEventRecord(userId);
    userId = 100;
    AccountSubscriber::ResetUserEventRecord(userId);
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Account_Subscriber_ResetUserEventRecord_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_OnReceiveEvent_test_0000
* @tc.name: Account_Subscriber_OnReceiveEvent_0000
* @tc.desc: Test function of OnReceiveEvent interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_OnReceiveEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_OnReceiveEvent_0000-begin";
    EventFwk::CommonEventData testData;
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    testData.SetWant(want);
    accountSubscriberPtr_->OnReceiveEvent(testData);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    testData.SetWant(want);
    accountSubscriberPtr_->OnReceiveEvent(testData);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);
    testData.SetWant(want);
    accountSubscriberPtr_->OnReceiveEvent(testData);

    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Account_Subscriber_OnReceiveEvent_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_GetUserStatus_test_0000
* @tc.name: Account_Subscriber_GetUserStatus_0000
* @tc.desc: Test function of GetUserStatus interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_GetUserStatus_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_GetUserStatus_0000-begin";
    int32_t userId = 102;
    uint32_t ret = accountSubscriberPtr_->GetUserStatus(userId);
    EXPECT_TRUE(ret == 0);
    GTEST_LOG_(INFO) << "Account_Subscriber_GetUserStatus_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserUnlockEvent_test_0000
* @tc.name: Account_Subscriber_HandleUserUnlockEvent_0000
* @tc.desc: Test function of HandleUserUnlockEvent interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserUnlockEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0000-begin";
    uint32_t userStatus = 0;
    uint32_t ret = accountSubscriberPtr_->HandleUserUnlockEvent(userStatus);
    EXPECT_TRUE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserUnlockEvent_test_0001
* @tc.name: Account_Subscriber_HandleUserUnlockEvent_0001
* @tc.desc: Test function of HandleUserUnlockEvent interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserUnlockEvent_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0001-begin";
    uint32_t userStatus = 3;
    uint32_t ret = accountSubscriberPtr_->HandleUserUnlockEvent(userStatus);
    EXPECT_TRUE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0001 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserUnlockEvent_test_0002
* @tc.name: Account_Subscriber_HandleUserUnlockEvent_0002
* @tc.desc: Test function of HandleUserUnlockEvent interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserUnlockEvent_test_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0002-begin";
    uint32_t userStatus = 2;
    uint32_t ret = accountSubscriberPtr_->HandleUserUnlockEvent(userStatus);
    EXPECT_FALSE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0002 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserSwitchedEvent_test_0000
* @tc.name: Account_Subscriber_HandleUserSwitchedEvent_test_0000
* @tc.desc: Test function of HandleUserSwitchedEvent interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserSwitchedEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0000-begin";
    uint32_t userStatus = 0;
    uint32_t ret = accountSubscriberPtr_->HandleUserSwitchedEvent(userStatus);
    EXPECT_FALSE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserSwitchedEvent_test_0001
* @tc.name: Account_Subscriber_HandleUserSwitchedEvent_test_0001
* @tc.desc: Test function of HandleUserSwitchedEvent interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserSwitchedEvent_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0001-begin";
    uint32_t userStatus = 0;
    uint32_t ret = accountSubscriberPtr_->HandleUserSwitchedEvent(userStatus);
    EXPECT_TRUE(ret == 2);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0001 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_GetSystemAbility_test_0000
* @tc.name: Account_Subscriber_GetSystemAbility_test_0000
* @tc.desc: Test function of GetSystemAbility interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_GetSystemAbility_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_GetSystemAbility_test_0000-begin";
    accountSubscriberPtr_->GetSystemAbility();
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Account_Subscriber_GetSystemAbility_test_0000 end";
}

}
}