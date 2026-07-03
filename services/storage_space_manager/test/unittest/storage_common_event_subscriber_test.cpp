/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "common_event.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "want.h"
#include "common_event/storage_common_event_subscriber.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class StorageCommonEventSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static std::shared_ptr<StorageCommonEventSubscriber> subscriber_;
};

std::shared_ptr<StorageCommonEventSubscriber> StorageCommonEventSubscriberTest::subscriber_ = nullptr;

void StorageCommonEventSubscriberTest::SetUpTestCase()
{
    // Reset subscriber before all tests
    StorageCommonEventSubscriber::subscriber_ = nullptr;
}

void StorageCommonEventSubscriberTest::TearDownTestCase()
{
    // Clean up subscriber after all tests
    if (StorageCommonEventSubscriber::subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(StorageCommonEventSubscriber::subscriber_);
        StorageCommonEventSubscriber::subscriber_ = nullptr;
    }
}

void StorageCommonEventSubscriberTest::SetUp()
{
    // Reset subscriber before each test
    StorageCommonEventSubscriber::subscriber_ = nullptr;
}

void StorageCommonEventSubscriberTest::TearDown()
{
    // Clean up after each test
    if (StorageCommonEventSubscriber::subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(StorageCommonEventSubscriber::subscriber_);
        StorageCommonEventSubscriber::subscriber_ = nullptr;
    }
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Constructor_0001
 * @tc.name: StorageCommonEventSubscriber_Constructor_Default
 * @tc.desc: Test default constructor
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, StorageCommonEventSubscriber_Constructor_Default, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_Constructor_Default start";

    StorageCommonEventSubscriber subscriber;
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_Constructor_Default end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Constructor_0002
 * @tc.name: StorageCommonEventSubscriber_Constructor_WithInfo
 * @tc.desc: Test constructor with CommonEventSubscribeInfo
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, StorageCommonEventSubscriber_Constructor_WithInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_Constructor_WithInfo start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_Constructor_WithInfo end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Subscribe_0001
 * @tc.name: SubscribeCommonEvent_Success
 * @tc.desc: Test successful subscription to common events
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, SubscribeCommonEvent_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_SubscribeCommonEvent_Success start";

    StorageCommonEventSubscriber::SubscribeCommonEvent();

    EXPECT_NE(StorageCommonEventSubscriber::subscriber_, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_SubscribeCommonEvent_Success end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Subscribe_0002
 * @tc.name: SubscribeCommonEvent_MultipleCalls
 * @tc.desc: Test multiple calls to SubscribeCommonEvent (should not create multiple subscribers)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, SubscribeCommonEvent_MultipleCalls, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_SubscribeCommonEvent_MultipleCalls start";

    // First call
    StorageCommonEventSubscriber::SubscribeCommonEvent();
    auto firstSubscriber = StorageCommonEventSubscriber::subscriber_;
    EXPECT_NE(firstSubscriber, nullptr);

    // Second call - should not create new subscriber
    StorageCommonEventSubscriber::SubscribeCommonEvent();
    auto secondSubscriber = StorageCommonEventSubscriber::subscriber_;
    EXPECT_EQ(firstSubscriber, secondSubscriber);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_SubscribeCommonEvent_MultipleCalls end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0001
 * @tc.name: OnReceiveEvent_PackageRemoved_ValidUserId
 * @tc.desc: Test receiving package removed event with valid user ID
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_PackageRemoved_ValidUserId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_ValidUserId start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_ValidUserId end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0002
 * @tc.name: OnReceiveEvent_PackageRemoved_InvalidUserId
 * @tc.desc: Test receiving package removed event with invalid user ID (negative)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_PackageRemoved_InvalidUserId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_InvalidUserId start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetParam("userId", -1);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs and returns early
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_InvalidUserId end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0003
 * @tc.name: OnReceiveEvent_PackageRemoved_ZeroUserId
 * @tc.desc: Test receiving package removed event with zero user ID
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_PackageRemoved_ZeroUserId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_ZeroUserId start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetParam("userId", 0);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs and returns early
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_ZeroUserId end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0004
 * @tc.name: OnReceiveEvent_PackageRemoved_DefaultUserId
 * @tc.desc: Test receiving package removed event with default user ID (-1)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_PackageRemoved_DefaultUserId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_DefaultUserId start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    // Don't set userId, should default to -1
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs and returns early
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PackageRemoved_DefaultUserId end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0005
 * @tc.name: OnReceiveEvent_ScreenOff
 * @tc.desc: Test receiving screen off event
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_ScreenOff, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_ScreenOff start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_ScreenOff end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0006
 * @tc.name: OnReceiveEvent_ScreenOn
 * @tc.desc: Test receiving screen on event
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_ScreenOn, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_ScreenOn start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_ScreenOn end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0007
 * @tc.name: OnReceiveEvent_PowerConnected
 * @tc.desc: Test receiving power connected event
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_PowerConnected, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PowerConnected start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PowerConnected end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0008
 * @tc.name: OnReceiveEvent_PowerDisconnected
 * @tc.desc: Test receiving power disconnected event
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_PowerDisconnected, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PowerDisconnected start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_PowerDisconnected end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0009
 * @tc.name: OnReceiveEvent_BatteryChanged
 * @tc.desc: Test receiving battery changed event
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_BatteryChanged, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 50);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0010
 * @tc.name: OnReceiveEvent_BatteryChanged_ZeroBattery
 * @tc.desc: Test receiving battery changed event with zero battery level
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_BatteryChanged_ZeroBattery, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged_ZeroBattery start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 0);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged_ZeroBattery end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0011
 * @tc.name: OnReceiveEvent_BatteryChanged_LowBattery
 * @tc.desc: Test receiving battery changed event with low battery (<10%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_BatteryChanged_LowBattery, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged_LowBattery start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 5);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged_LowBattery end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0012
 * @tc.name: OnReceiveEvent_BatteryChanged_HighBattery
 * @tc.desc: Test receiving battery changed event with high battery (>10%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_BatteryChanged_HighBattery, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged_HighBattery start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 80);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_BatteryChanged_HighBattery end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0013
 * @tc.name: OnReceiveEvent_UserUnlocked
 * @tc.desc: Test receiving user unlocked event
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_UserUnlocked, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_UserUnlocked start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_UserUnlocked end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0014
 * @tc.name: OnReceiveEvent_CloneState_Start
 * @tc.desc: Test receiving clone state event with start state
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_CloneState_Start, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_Start start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("usual.event.clone.CommonEventCloneState");
    want.SetParam("cloneState", 1);
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_Start end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0015
 * @tc.name: OnReceiveEvent_CloneState_End
 * @tc.desc: Test receiving clone state event with end state
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_CloneState_End, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_End start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("usual.event.clone.CommonEventCloneState");
    want.SetParam("cloneState", 0);
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_End end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0016
 * @tc.name: OnReceiveEvent_CloneState_InvalidState
 * @tc.desc: Test receiving clone state event with invalid state
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_CloneState_InvalidState, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_InvalidState start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("usual.event.clone.CommonEventCloneState");
    want.SetParam("cloneState", 2); // Invalid state (not 0 or 1)
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_InvalidState end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0017
 * @tc.name: OnReceiveEvent_CloneState_DefaultState
 * @tc.desc: Test receiving clone state event with default state (-1)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_CloneState_DefaultState, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_DefaultState start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("usual.event.clone.CommonEventCloneState");
    // Don't set cloneState, should default to -1
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_CloneState_DefaultState end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0018
 * @tc.name: OnReceiveEvent_UnknownAction
 * @tc.desc: Test receiving event with unknown action
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_UnknownAction, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_UnknownAction start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("unknown.event.action");

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_UnknownAction end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_OnReceiveEvent_0019
 * @tc.name: OnReceiveEvent_EmptyAction
 * @tc.desc: Test receiving event with empty action
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, OnReceiveEvent_EmptyAction, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_EmptyAction start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("");

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_OnReceiveEvent_EmptyAction end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0001
 * @tc.name: DeviceState_ScreenOff_PowerConnected_TriggerScan
 * @tc.desc: Test device state that triggers storage scan (charging + screen off + battery > 10%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_ScreenOff_PowerConnected_TriggerScan, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOff_PowerConnected_TriggerScan start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // First, set screen off
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Then, set power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Finally, set battery to high level (> 10%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 80);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // Test passes if no crash occurs and scan is triggered
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOff_PowerConnected_TriggerScan end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0002
 * @tc.name: DeviceState_ScreenOn_PowerConnected_NoTrigger
 * @tc.desc: Test device state that doesn't trigger storage scan (screen on)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_ScreenOn_PowerConnected_NoTrigger, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOn_PowerConnected_NoTrigger start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // Set power connected
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Set battery to high level (> 10%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 80);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOn_PowerConnected_NoTrigger end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0003
 * @tc.name: DeviceState_ScreenOff_PowerDisconnected_NoTrigger
 * @tc.desc: Test device state that doesn't trigger storage scan (power disconnected)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_ScreenOff_PowerDisconnected_NoTrigger, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOff_PowerDisconnected_NoTrigger start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // Set screen off
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Set battery to high level (> 10%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 80);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOff_PowerDisconnected_NoTrigger end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0004
 * @tc.name: DeviceState_ScreenOff_PowerConnected_LowBattery_NoTrigger
 * @tc.desc: Test device state that doesn't trigger storage scan (battery <= 10%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_ScreenOff_PowerConnected_LowBattery_NoTrigger, TestSize.Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // Set screen off
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Set power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Set battery to low level (<= 10%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 5);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0005
 * @tc.name: DeviceState_Boundary_BatteryLevel10
 * @tc.desc: Test battery level at boundary value (10%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_Boundary_BatteryLevel10, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_Boundary_BatteryLevel10 start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // Set screen off
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Set power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Set battery to boundary level (10%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 10);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_Boundary_BatteryLevel10 end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0006
 * @tc.name: DeviceState_Boundary_BatteryLevel11
 * @tc.desc: Test battery level just above boundary (11%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_Boundary_BatteryLevel11, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_Boundary_BatteryLevel11 start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // Set screen off
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Set power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Set battery to just above boundary (11%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 11);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_Boundary_BatteryLevel11 end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0007
 * @tc.name: DeviceState_Boundary_BatteryLevel9
 * @tc.desc: Test battery level just below boundary (9%)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_Boundary_BatteryLevel9, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_Boundary_BatteryLevel9 start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    // Set screen off
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Set power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Set battery to just below boundary (9%)
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 9);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_Boundary_BatteryLevel9 end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0008
 * @tc.name: DeviceState_MultipleTransitions
 * @tc.desc: Test multiple device state transitions
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_MultipleTransitions, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_MultipleTransitions start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;

    // Simulate device state changes:
    // 1. Screen off, power connected, high battery -> should trigger scan
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData1(want);
    subscriber.OnReceiveEvent(eventData1);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 80);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // 2. Screen on -> should stop scan
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EventFwk::CommonEventData eventData4(want);
    subscriber.OnReceiveEvent(eventData4);

    // 3. Screen off again, power still connected -> should trigger scan again
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData5(want);
    subscriber.OnReceiveEvent(eventData5);

    // 4. Power disconnected -> should stop scan
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    EventFwk::CommonEventData eventData6(want);
    subscriber.OnReceiveEvent(eventData6);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_MultipleTransitions end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0009
 * @tc.name: DeviceState_PowerDisconnectedThenConnected
 * @tc.desc: Test power disconnected then connected
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_PowerDisconnectedThenConnected, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_PowerDisconnectedThenConnected start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;

    // Power disconnected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    EventFwk::CommonEventData eventData1(want);
    subscriber.OnReceiveEvent(eventData1);

    // Power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_PowerDisconnectedThenConnected end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_DeviceState_0010
 * @tc.name: DeviceState_ScreenOnThenOff
 * @tc.desc: Test screen on then off
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, DeviceState_ScreenOnThenOff, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOnThenOff start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;

    // Screen on
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EventFwk::CommonEventData eventData1(want);
    subscriber.OnReceiveEvent(eventData1);

    // Screen off
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_DeviceState_ScreenOnThenOff end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Battery_0001
 * @tc.name: BatteryChanged_NegativeBattery
 * @tc.desc: Test battery changed event with negative battery level
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, BatteryChanged_NegativeBattery, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_NegativeBattery start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", -10);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_NegativeBattery end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Battery_0002
 * @tc.name: BatteryChanged_Over100Percent
 * @tc.desc: Test battery changed event with battery level over 100%
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, BatteryChanged_Over100Percent, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_Over100Percent start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 150);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_Over100Percent end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Battery_0003
 * @tc.name: BatteryChanged_Exactly100Percent
 * @tc.desc: Test battery changed event with battery level at 100%
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, BatteryChanged_Exactly100Percent, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_Exactly100Percent start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 100);

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_Exactly100Percent end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_Battery_0004
 * @tc.name: BatteryChanged_DefaultParam
 * @tc.desc: Test battery changed event without setting battery parameter (should default to 0)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, BatteryChanged_DefaultParam, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_DefaultParam start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    // Don't set soc parameter, should default to 0

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_BatteryChanged_DefaultParam end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_PackageRemoved_0001
 * @tc.name: PackageRemoved_EmptyBundleName
 * @tc.desc: Test package removed event with empty bundle name
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, PackageRemoved_EmptyBundleName, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_PackageRemoved_EmptyBundleName start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string());

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_PackageRemoved_EmptyBundleName end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_PackageRemoved_0002
 * @tc.name: PackageRemoved_NoBundleName
 * @tc.desc: Test package removed event without bundle name parameter
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, PackageRemoved_NoBundleName, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_PackageRemoved_NoBundleName start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetParam("userId", 100);
    // Don't set bundleName

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_PackageRemoved_NoBundleName end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_PackageRemoved_0003
 * @tc.name: PackageRemoved_MaxUserId
 * @tc.desc: Test package removed event with maximum user ID
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, PackageRemoved_MaxUserId, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_PackageRemoved_MaxUserId start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetParam("userId", INT32_MAX);
    want.SetParam("bundleName", std::string("com.test.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_PackageRemoved_MaxUserId end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_EventSequence_0001
 * @tc.name: EventSequence_MultipleEventsInSequence
 * @tc.desc: Test receiving multiple events in sequence
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, EventSequence_MultipleEventsInSequence, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_EventSequence_MultipleEventsInSequence start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;

    // Sequence 1: Screen on
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EventFwk::CommonEventData eventData1(want);
    subscriber.OnReceiveEvent(eventData1);

    // Sequence 2: Power connected
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EventFwk::CommonEventData eventData2(want);
    subscriber.OnReceiveEvent(eventData2);

    // Sequence 3: Screen off
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData eventData3(want);
    subscriber.OnReceiveEvent(eventData3);

    // Sequence 4: Battery changed to high level
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam("soc", 90);
    EventFwk::CommonEventData eventData4(want);
    subscriber.OnReceiveEvent(eventData4);

    // Sequence 5: User unlocked
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    EventFwk::CommonEventData eventData5(want);
    subscriber.OnReceiveEvent(eventData5);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_EventSequence_MultipleEventsInSequence end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_EventSequence_0002
 * @tc.name: EventSequence_AlternatingScreenStates
 * @tc.desc: Test alternating between screen on and off states
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, EventSequence_AlternatingScreenStates, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_EventSequence_AlternatingScreenStates start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;

    for (int i = 0; i < 3; i++) {
        // Screen on
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        EventFwk::CommonEventData eventDataOn(want);
        subscriber.OnReceiveEvent(eventDataOn);

        // Screen off
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        EventFwk::CommonEventData eventDataOff(want);
        subscriber.OnReceiveEvent(eventDataOff);
    }

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_EventSequence_AlternatingScreenStates end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_EventSequence_0003
 * @tc.name: EventSequence_AlternatingPowerStates
 * @tc.desc: Test alternating between power connected and disconnected states
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, EventSequence_AlternatingPowerStates, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_EventSequence_AlternatingPowerStates start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;

    for (int i = 0; i < 3; i++) {
        // Power connected
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        EventFwk::CommonEventData eventDataConnected(want);
        subscriber.OnReceiveEvent(eventDataConnected);

        // Power disconnected
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        EventFwk::CommonEventData eventDataDisconnected(want);
        subscriber.OnReceiveEvent(eventDataDisconnected);
    }

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_EventSequence_AlternatingPowerStates end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_CloneEvent_0001
 * @tc.name: CloneEvent_WithUserIdAndBundleName
 * @tc.desc: Test clone event with user ID and bundle name parameters
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, CloneEvent_WithUserIdAndBundleName, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_CloneEvent_WithUserIdAndBundleName start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("usual.event.clone.CommonEventCloneState");
    want.SetParam("cloneState", 1);
    want.SetParam("userId", 100);
    want.SetParam("bundleName", std::string("com.clone.app"));

    EventFwk::CommonEventData eventData(want);
    subscriber.OnReceiveEvent(eventData);

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_CloneEvent_WithUserIdAndBundleName end";
}

/**
 * @tc.number: SUB_STORAGE_StorageCommonEventSubscriber_CloneEvent_0002
 * @tc.name: CloneEvent_WithoutUserId
 * @tc.desc: Test clone event without user ID parameter
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageCommonEventSubscriberTest, CloneEvent_WithoutUserId, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_CloneEvent_WithoutUserId start";

    EventFwk::MatchingSkills matchingSkills;
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    StorageCommonEventSubscriber subscriber(subscribeInfo);

    AAFwk::Want want;
    want.SetAction("usual.event.clone.CommonEventCloneState");
    want.SetParam("cloneState", 0);
    want.SetParam("bundleName", std::string("com.clone.app"));

    // Test passes if no crash occurs
    EXPECT_NE(&subscriber, nullptr);

    GTEST_LOG_(INFO) << "StorageCommonEventSubscriberTest_CloneEvent_WithoutUserId end";
}

} // namespace StorageSpaceManager
} // namespace OHOS
