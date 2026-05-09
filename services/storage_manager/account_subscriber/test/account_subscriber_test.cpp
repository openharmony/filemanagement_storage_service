/*
* Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
static constexpr const char *SYS_PARAM_APPSPAWN_UNLOCK_MOUNT = "startup.appspawn.unlock_mount.";

class AccountSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AccountSubscriberTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup";
}

void AccountSubscriberTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown";
}

void AccountSubscriberTest::SetUp() {}

void AccountSubscriberTest::TearDown() {}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_ResetUserEventRecord_0000
* @tc.name: Account_Subscriber_ResetUserEventRecord_0000
* @tc.desc: Test function of ResetUserEventRecord
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_ResetUserEventRecord_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_ResetUserEventRecord_0000-begin";

    int32_t userId = -1;
    AccountSubscriber::GetInstance().ResetUserEventRecord(userId);

    userId = StorageService::MAX_USER_ID + 1;
    AccountSubscriber::GetInstance().ResetUserEventRecord(userId);

    userId = 1;
    AccountSubscriber::GetInstance().ResetUserEventRecord(userId);
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Account_Subscriber_ResetUserEventRecord_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_OnReceiveEvent_test_0000
* @tc.name: Account_Subscriber_OnReceiveEvent_0000
* @tc.desc: Test function of OnReceiveEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_OnReceiveEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_OnReceiveEvent_0000-begin";
    StorageService::UserChangedEventType enumType = StorageService::UserChangedEventType::EVENT_USER_UNLOCKED;
    uint32_t userId = 999;
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, enumType);

    enumType = StorageService::UserChangedEventType::EVENT_USER_SWITCHED;
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, enumType);

    EXPECT_EQ(enumType, StorageService::UserChangedEventType::EVENT_USER_SWITCHED);
    GTEST_LOG_(INFO) << "Account_Subscriber_OnReceiveEvent_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_GetUserStatus_test_0000
* @tc.name: Account_Subscriber_GetUserStatus_0000
* @tc.desc: Test function of GetUserStatus
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_GetUserStatus_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_GetUserStatus_0000-begin";

    int32_t userId = 102;
    uint32_t ret = AccountSubscriber::GetInstance().GetUserStatus(userId);
    EXPECT_TRUE(ret == 0);

    AccountSubscriber::GetInstance().userRecord_[userId] = 1;
    ret = AccountSubscriber::GetInstance().GetUserStatus(userId);
    EXPECT_TRUE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_GetUserStatus_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserUnlockEvent_test_0000
* @tc.name: Account_Subscriber_HandleUserUnlockEvent_0000
* @tc.desc: Test function of HandleUserUnlockEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserUnlockEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0000-begin";
    uint32_t userStatus = 0;
    uint32_t ret = AccountSubscriber::GetInstance().HandleUserUnlockEvent(userStatus);
    EXPECT_TRUE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserUnlockEvent_test_0001
* @tc.name: Account_Subscriber_HandleUserUnlockEvent_0001
* @tc.desc: Test function of HandleUserUnlockEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserUnlockEvent_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0001-begin";
    uint32_t userStatus = 3;
    uint32_t ret = AccountSubscriber::GetInstance().HandleUserUnlockEvent(userStatus);
    EXPECT_TRUE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0001 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserUnlockEvent_test_0002
* @tc.name: Account_Subscriber_HandleUserUnlockEvent_0002
* @tc.desc: Test function of HandleUserUnlockEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserUnlockEvent_test_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0002-begin";
    uint32_t userStatus = 2;
    uint32_t ret = AccountSubscriber::GetInstance().HandleUserUnlockEvent(userStatus);
    EXPECT_FALSE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserUnlockEvent_0002 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserSwitchedEvent_test_0000
* @tc.name: Account_Subscriber_HandleUserSwitchedEvent_test_0000
* @tc.desc: Test function of HandleUserSwitchedEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserSwitchedEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0000-begin";
    uint32_t userStatus = 0;
    uint32_t ret = AccountSubscriber::GetInstance().HandleUserSwitchedEvent(userStatus);
    EXPECT_FALSE(ret == 1);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_HandleUserSwitchedEvent_test_0001
* @tc.name: Account_Subscriber_HandleUserSwitchedEvent_test_0001
* @tc.desc: Test function of HandleUserSwitchedEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_HandleUserSwitchedEvent_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0001-begin";
    uint32_t userStatus = 0;
    uint32_t ret = AccountSubscriber::GetInstance().HandleUserSwitchedEvent(userStatus);
    EXPECT_TRUE(ret == 2);
    GTEST_LOG_(INFO) << "Account_Subscriber_HandleUserSwitchedEvent_test_0001 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_GetSystemAbility_test_0000
* @tc.name: Account_Subscriber_GetSystemAbility_test_0000
* @tc.desc: Test function of GetSystemAbility
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
 */
HWTEST_F(AccountSubscriberTest, Account_Subscriber_GetSystemAbility_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_GetSystemAbility_test_0000-begin";
    AccountSubscriber::GetInstance().GetSystemAbility();
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Account_Subscriber_GetSystemAbility_test_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_NotifyUserChangedEvent_test_0000
* @tc.name: Account_Subscriber_NotifyUserChangedEvent_test_0000
* @tc.desc: Test NotifyUserChangedEvent when user unlocked and switched
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_NotifyUserChangedEvent_test_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_NotifyUserChangedEvent_0000 begin";
    int32_t userId = 100;

    AccountSubscriber::GetInstance().userRecord_[userId] = 0;
    StorageService::UserChangedEventType enumType = StorageService::UserChangedEventType::EVENT_USER_UNLOCKED;
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, enumType);
    EXPECT_EQ(AccountSubscriber::GetInstance().userRecord_[userId], 1); // 1: bit set
    enumType = StorageService::UserChangedEventType::EVENT_USER_SWITCHED;
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, enumType);
    EXPECT_EQ(AccountSubscriber::GetInstance().userRecord_.count(userId), 0);
    GTEST_LOG_(INFO) << "Account_Subscriber_NotifyUserChangedEvent_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_NotifyUserChangedEvent_NoMount_0000
* @tc.name: Account_Subscriber_NotifyUserChangedEvent_NoMount_0000
* @tc.desc: Test NotifyUserChangedEvent does not trigger MountCryptoPathAgain when only unlocked
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_NotifyUserChangedEvent_NoMount_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_NotifyUserChangedEvent_NoMount_0000-begin";
    int32_t userId = 101;

    AccountSubscriber::GetInstance().userRecord_[userId] = 0;
    StorageService::UserChangedEventType enumType = StorageService::UserChangedEventType::EVENT_USER_UNLOCKED;
    AccountSubscriber::GetInstance().NotifyUserChangedEvent(userId, enumType);

    EXPECT_EQ(AccountSubscriber::GetInstance().userRecord_[userId], 1); // 1: bit set
    GTEST_LOG_(INFO) << "Account_Subscriber_NotifyUserChangedEvent_NoMount_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_SendUserLockStatusToAppSpawn_0000
* @tc.name: Account_Subscriber_SendUserLockStatusToAppSpawn_0000
* @tc.desc: Test SendUserLockStatusToAppSpawn with DECRYPTED status
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_SendUserLockStatusToAppSpawn_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_SendUserLockStatusToAppSpawn_0000-begin";
    int32_t userId = 100;
    bool lockStatus = false;
    int32_t ret = AccountSubscriber::GetInstance().SendUserLockStatusToAppSpawn(userId, lockStatus);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "Account_Subscriber_SendUserLockStatusToAppSpawn_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_MountCryptoPathAgain_0000
* @tc.name: Account_Subscriber_MountCryptoPathAgain_0000
* @tc.desc: Test MountCryptoPathAgain function
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_MountCryptoPathAgain_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_MountCryptoPathAgain_0000-begin";
    int32_t userId = 100;
    AccountSubscriber::GetInstance().MountCryptoPathAgain(userId);
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_.count(userId), 0);
    GTEST_LOG_(INFO) << "Account_Subscriber_MountCryptoPathAgain_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_OnUnlockParamChanged_0000
* @tc.name: Account_Subscriber_OnUnlockParamChanged_0000
* @tc.desc: Test OnUnlockParamChanged with null parameters
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_OnUnlockParamChanged_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_OnUnlockParamChanged_0000-begin";
    int32_t userId = 100;
    AccountSubscriber::GetInstance().paramChangedMap_[userId] = PARAM_UNKNOWN;
    AccountSubscriber::GetInstance().OnUnlockParamChanged(nullptr, "0", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_UNKNOWN);
    AccountSubscriber::GetInstance().OnUnlockParamChanged("test.key", nullptr, &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_UNKNOWN);
    GTEST_LOG_(INFO) << "Account_Subscriber_OnUnlockParamChanged_0000 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_OnUnlockParamChanged_0001
* @tc.name: Account_Subscriber_OnUnlockParamChanged_0001
* @tc.desc: Test OnUnlockParamChanged with valid parameters and value equals "0"
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_OnUnlockParamChanged_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_OnUnlockParamChanged_0001-begin";
    AccountSubscriber::GetInstance().paramChangedMap_.clear();
    int32_t userId = 100;
    AccountSubscriber::GetInstance().paramChangedMap_[userId] = PARAM_UNKNOWN;
    std::string testKey = std::string("test.key") +
        std::string(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT) + std::to_string(userId);

    AccountSubscriber::GetInstance().OnUnlockParamChanged(
        testKey.c_str(),
        "0", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_UNKNOWN);

    testKey = std::string(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT);
    AccountSubscriber::GetInstance().OnUnlockParamChanged(
        testKey.c_str(),
        "0", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_UNKNOWN);

    testKey = std::string(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT) + std::string("test.key");
    AccountSubscriber::GetInstance().OnUnlockParamChanged(
        testKey.c_str(),
        "0", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_UNKNOWN);

    int32_t tmpUserId = 101;
    testKey = std::string(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT) + std::to_string(tmpUserId);
    AccountSubscriber::GetInstance().OnUnlockParamChanged(
        testKey.c_str(),
        "0", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_UNKNOWN);
    GTEST_LOG_(INFO) << "Account_Subscriber_OnUnlockParamChanged_0001 end";
}

/**
* @tc.number: SUB_STORAGE_Account_Subscriber_OnUnlockParamChanged_0002
* @tc.name: Account_Subscriber_OnUnlockParamChanged_0002
* @tc.desc: Test OnUnlockParamChanged with valid parameters and value not equals "0"
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: issueI9G5A0
*/
HWTEST_F(AccountSubscriberTest, Account_Subscriber_OnUnlockParamChanged_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Account_Subscriber_OnUnlockParamChanged_0002-begin";
    AccountSubscriber::GetInstance().paramChangedMap_.clear();
    int32_t userId = 100;
    AccountSubscriber::GetInstance().paramChangedMap_[userId] = PARAM_UNKNOWN;
    std::string testKey = std::string(SYS_PARAM_APPSPAWN_UNLOCK_MOUNT) + std::to_string(userId);
    AccountSubscriber::GetInstance().OnUnlockParamChanged(testKey.c_str(),
        "1", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_FAIL);

    AccountSubscriber::GetInstance().OnUnlockParamChanged(testKey.c_str(),
        "0", &AccountSubscriber::GetInstance());
    EXPECT_EQ(AccountSubscriber::GetInstance().paramChangedMap_[userId], PARAM_SUCCESS);
    GTEST_LOG_(INFO) << "Account_Subscriber_OnUnlockParamChanged_0002 end";
}
} // namespace StorageManager
} // namespace OHOS
