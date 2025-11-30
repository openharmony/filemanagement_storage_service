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

#include "common_event/storage_common_event_subscriber.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace StorageManager {
using namespace std;
using namespace testing::ext;

class StorageCommonEventSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp();
    void TearDown();
    static inline shared_ptr<StorageCommonEventSubscriber> subscriberPtr_ = nullptr;
};

void StorageCommonEventSubscriberTest::SetUp()
{
    subscriberPtr_ = make_shared<StorageCommonEventSubscriber>();
}

void StorageCommonEventSubscriberTest::TearDown()
{
    subscriberPtr_ = nullptr;
}

/**
* @tc.number: Storage_subscriber_SubscribeCommonEvent_test_0000
* @tc.name: Storage_subscriber_SubscribeCommonEvent_test_0000
* @tc.desc: Test function of SubscribeCommonEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_SubscribeCommonEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_SubscribeCommonEvent_test_0000 begin";
    StorageCommonEventSubscriber::SubscribeCommonEvent();
    StorageCommonEventSubscriber::SubscribeCommonEvent();
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Storage_subscriber_SubscribeCommonEvent_test_0000 end";
}

/**
* @tc.number: Storage_subscriber_OnReceiveEvent_test_0000
* @tc.name: Storage_subscriber_OnReceiveEvent_test_0000
* @tc.desc: Test function of OnReceiveEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_OnReceiveEvent_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0000 begin";
    ASSERT_TRUE(subscriberPtr_ != nullptr);
    EventFwk::CommonEventData testData;
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    testData.SetWant(want);
    subscriberPtr_->OnReceiveEvent(testData);
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0000 end";
}

/**
* @tc.number: Storage_subscriber_OnReceiveEvent_test_0001
* @tc.name: Storage_subscriber_OnReceiveEvent_test_0001
* @tc.desc: Test function of OnReceiveEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_OnReceiveEvent_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0001 begin";
    ASSERT_TRUE(subscriberPtr_ != nullptr);
    EventFwk::CommonEventData testData;
    AAFwk::Want want;
    int32_t userId = 100;
    std::string bundleName = "test";
    want.SetParam("userId", userId);
    want.SetParam("bundleName", bundleName);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    testData.SetWant(want);
    subscriberPtr_->OnReceiveEvent(testData);
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0001 end";
}

/**
* @tc.number: Storage_subscriber_OnReceiveEvent_test_0002
* @tc.name: Storage_subscriber_OnReceiveEvent_test_0002
* @tc.desc: Test function of OnReceiveEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_OnReceiveEvent_test_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0002 begin";
    ASSERT_TRUE(subscriberPtr_ != nullptr);
    EventFwk::CommonEventData testData;
    AAFwk::Want want;
    testData.SetWant(want);
    subscriberPtr_->OnReceiveEvent(testData);
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0002 end";
}

/**
* @tc.number: Storage_subscriber_OnReceiveEvent_test_0003
* @tc.name: Storage_subscriber_OnReceiveEvent_test_0003
* @tc.desc: Test function of OnReceiveEvent
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_OnReceiveEvent_test_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0003 begin";
    ASSERT_TRUE(subscriberPtr_ != nullptr);
    EventFwk::CommonEventData testData;
    AAFwk::Want want;
    int32_t userId = 100;
    std::string bundleName = "test";
    want.SetParam("userId", userId);
    want.SetParam("bundleName", bundleName);
    testData.SetWant(want);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->OnReceiveEvent(testData));


    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->OnReceiveEvent(testData));


    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->OnReceiveEvent(testData));


    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->OnReceiveEvent(testData));


    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->OnReceiveEvent(testData));
    GTEST_LOG_(INFO) << "Storage_subscriber_OnReceiveEvent_test_0003 end";
}

/**
* @tc.number: Storage_subscriber_UpdateDeviceState_test_0001
* @tc.name: Storage_subscriber_UpdateDeviceState_test_0001
* @tc.desc: Test function of UpdateDeviceState
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_UpdateDeviceState_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_UpdateDeviceState_test_0001 begin";
    ASSERT_TRUE(subscriberPtr_ != nullptr);
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->UpdateDeviceState(STATE_SCREEN_OFF, true));

    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->UpdateDeviceState(STATE_SCREEN_OFF, false));
    GTEST_LOG_(INFO) << "Storage_subscriber_UpdateDeviceState_test_0001 end";
}

/**
* @tc.number: Storage_subscriber_CheckAndTriggerStatistic_test_0001
* @tc.name: Storage_subscriber_CheckAndTriggerStatistic_test_0001
* @tc.desc: Test function of CheckAndTriggerStatistic
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(StorageCommonEventSubscriberTest, Storage_subscriber_CheckAndTriggerStatistic_test_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Storage_subscriber_CheckAndTriggerStatistic_test_0001 begin";
    ASSERT_TRUE(subscriberPtr_ != nullptr);
    subscriberPtr_->deviceState_ = STATE_SCREEN_OFF;
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->CheckAndTriggerStatistic());

    subscriberPtr_->deviceState_ = STATE_CHARGING_SCREEN_OFF;
    subscriberPtr_->batteryCapacity_ = 15;
    EXPECT_NO_FATAL_FAILURE(subscriberPtr_->CheckAndTriggerStatistic());
    GTEST_LOG_(INFO) << "Storage_subscriber_CheckAndTriggerStatistic_test_0001 end";
}
} // namespace StorageManager
} // namespace OHOS
