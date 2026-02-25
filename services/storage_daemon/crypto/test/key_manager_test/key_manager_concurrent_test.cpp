/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <atomic>
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <vector>

#include "key_manager.h"
#include "mock/uece_activation_callback_mock.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"

using namespace std;
using namespace testing::ext;
using namespace testing;
using namespace OHOS::StorageManager;

namespace OHOS::StorageDaemon {
class KeyManagerConcurrentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static inline sptr<UeceActivationCallbackMock> mockCallback_ = nullptr;
};

void KeyManagerConcurrentTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "KeyManagerConcurrentTest SetUpTestCase";
}

void KeyManagerConcurrentTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "KeyManagerConcurrentTest TearDownTestCase";
}

void KeyManagerConcurrentTest::SetUp()
{
    mockCallback_ = new UeceActivationCallbackMock();
}

void KeyManagerConcurrentTest::TearDown()
{
    mockCallback_ = nullptr;
}

/**
 * @tc.name: KeyManager_Concurrent_NotifyAndUnregister_001
 * @tc.desc: 测试 NotifyUeceActivation 和 UnregisterUeceActivationCallback 并发调用
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(KeyManagerConcurrentTest, KeyManager_Concurrent_NotifyAndUnregister_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "KeyManager_Concurrent_NotifyAndUnregister_001 Start";
#ifdef EL5_FILEKEY_MANAGER
    int ret = KeyManager::GetInstance().RegisterUeceActivationCallback(mockCallback_);
    EXPECT_EQ(ret, E_OK);

    int32_t callbackRetValue = E_OK;
    EXPECT_CALL(*mockCallback_, OnEl5Activation(_, _, _, _))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<3>(callbackRetValue), Return(E_OK)));

    std::thread notifyThread([] {
        for (int i = 0; i < 10000; i++) {
            KeyManager::GetInstance().RegisterUeceActivationCallback(mockCallback_);
            KeyManager::GetInstance().NotifyUeceActivation(100, E_OK, true);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });

    std::thread unregisterThread([] {
        for (int i = 0; i < 10000; i++) {
            KeyManager::GetInstance().UnregisterUeceActivationCallback();
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });

    notifyThread.join();
    unregisterThread.join();
#endif
    GTEST_LOG_(INFO) << "KeyManager_Concurrent_NotifyAndUnregister_001 End";
}
} // namespace OHOS::StorageDaemon
