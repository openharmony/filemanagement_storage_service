/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "base_key_mock.h"

#include "crypto_delay_handler.h"
#include "fscrypt_key_v1_ext_mock.h"
#include "fscrypt_key_v2.h"
#include "fscrypt_key_v2_mock.h"


using namespace testing;

namespace OHOS::StorageDaemon {
class DelayHandlerTest : public ::testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FscryptKeyV1ExtMock> fscryptKeyExtMock_ = nullptr;
    static inline std::shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    std::shared_ptr<BaseKey> el4Key;
};

void DelayHandlerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    fscryptKeyExtMock_ = std::make_shared<FscryptKeyV1ExtMock>();
    FscryptKeyV1ExtMock::fscryptKeyV1ExtMock = fscryptKeyExtMock_;
    baseKeyMock_ = std::make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
}

void DelayHandlerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    FscryptKeyV1ExtMock::fscryptKeyV1ExtMock = nullptr;
    fscryptKeyExtMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
}

/**
 * @tc.number: StartDelayTask_WithNullKey
 * @tc.name: StartDelayTask_WithNullKey
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, StartDelayTask_WithNullKey, testing::ext::TestSize.Level1)
{
    // Expect no action taken
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    el4Key = nullptr;
    userDelayHandler->StartDelayTask(el4Key);
    EXPECT_EQ(userDelayHandler->el4Key_, nullptr);
}

/**
 * @tc.number: StartDelayTask_WithValidKey
 * @tc.name: StartDelayTask_WithValidKey
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, StartDelayTask_WithValidKey, testing::ext::TestSize.Level1)
{
    // Expect a delay task to be started
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    el4Key = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    userDelayHandler->StartDelayTask(el4Key);
    EXPECT_NE(userDelayHandler->el4Key_, nullptr);
}

/**
 * @tc.number: CancelDelayTask
 * @tc.name: CancelDelayTask
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, CancelDelayTask, testing::ext::TestSize.Level1)
{
    // Expect the delay task to be cancelled
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    userDelayHandler->cancelled_ = false;
    userDelayHandler->CancelDelayTask();
    EXPECT_TRUE(userDelayHandler->cancelled_);
}

/**
 * @tc.number: DeactiveEl3El4El5_WithNullKey
 * @tc.name: DeactiveEl3El4El5_WithNullKey
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, DeactiveEl3El4El5_WithNullKey, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    userDelayHandler->CancelDelayTask();
    EXPECT_TRUE(userDelayHandler->cancelled_);
    userDelayHandler->el4Key_ = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    userDelayHandler->DeactiveEl3El4El5();
    EXPECT_TRUE(userDelayHandler->cancelled_);
    // Expect error report for null key
}

/**
 * @tc.number: DeactiveEl3El4El5_WithCancelledTask
 * @tc.name: DeactiveEl3El4El5_WithCancelledTask
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, DeactiveEl3El4El5_WithCancelledTask, testing::ext::TestSize.Level1)
{
    // Expect no action taken as task is cancelled
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    userDelayHandler->CancelDelayTask();
    EXPECT_TRUE(userDelayHandler->cancelled_);
    userDelayHandler->el4Key_ = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    userDelayHandler->DeactiveEl3El4El5();
    EXPECT_TRUE(userDelayHandler->cancelled_);
}

/**
 * @tc.number: DeactiveEl3El4El5_WithValidKeyAndNotCancelled
 * @tc.name: DeactiveEl3El4El5_WithValidKeyAndNotCancelled
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, DeactiveEl3El4El5_WithValidKeyAndNotCancelled, testing::ext::TestSize.Level1)
{
    // Expect no action taken as task is cancelled
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    userDelayHandler->CancelDelayTask();
    EXPECT_TRUE(userDelayHandler->cancelled_);
    userDelayHandler->cancelled_ = false;
    userDelayHandler->el4Key_ = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    userDelayHandler->DeactiveEl3El4El5();
    EXPECT_FALSE(userDelayHandler->cancelled_);
}

/**
 * @tc.number: DeactiveEl3El4El5_WithValidKeyAndNotCancelled_01
 * @tc.name: DeactiveEl3El4El5_WithValidKeyAndNotCancelled_01
 * @tc.desc: Test function of DelayHandlerTest interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(DelayHandlerTest, DeactiveEl3El4El5_WithValidKeyAndNotCancelled_01, testing::ext::TestSize.Level1)
{
    // Expect no action taken as task is cancelled
    std::shared_ptr<DelayHandler> userDelayHandler = std::make_shared<DelayHandler>(100);
    userDelayHandler->CancelDelayTask();
    EXPECT_TRUE(userDelayHandler->cancelled_);
    userDelayHandler->el4Key_ = nullptr;
    userDelayHandler->DeactiveEl3El4El5();
    EXPECT_TRUE(userDelayHandler->cancelled_);
}
}
