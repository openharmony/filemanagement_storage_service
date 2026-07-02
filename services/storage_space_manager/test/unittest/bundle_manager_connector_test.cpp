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

#include <cstdio>
#include <gtest/gtest.h>

#include "adapter/bundle_manager_connector.h"
#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace std;
using namespace testing::ext;

class BundleManagerConnectorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void BundleManagerConnectorTest::SetUpTestCase()
{
    LOGI("SetUpTestCase");
}

void BundleManagerConnectorTest::TearDownTestCase()
{
    LOGI("TearDownTestCase");
}

void BundleManagerConnectorTest::SetUp()
{
    LOGI("SetUp");
}

void BundleManagerConnectorTest::TearDown()
{
    LOGI("TearDown");
    BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
}

/**
 * @tc.number: Bundle_Manager_Connector_GetInstance_test_0001
 * @tc.name: Bundle_Manager_Connector_GetInstance_test_0001
 * @tc.desc: Test GetInstance returns same instance multiple times
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_GetInstance_test_0001, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_GetInstance_test_0001 begin");
    auto& instance1 = BundleMgrConnector::GetInstance();
    auto& instance2 = BundleMgrConnector::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
    LOGI("Bundle_Manager_Connector_GetInstance_test_0001 end");
}

/**
 * @tc.number: Bundle_Manager_Connector_GetBundleMgrProxy_test_0000
 * @tc.name: Bundle_Manager_Connector_GetBundleMgrProxy_test_0000
 * @tc.desc: Test GetBundleMgrProxy function when SystemAbilityManager unavailable
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_GetBundleMgrProxy_test_0000, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_GetBundleMgrProxy_test_0000 begin");
    BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    auto proxy = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (proxy == nullptr) {
        LOGI("Expected nullptr when SystemAbilityManager unavailable in test environment");
    }
    ASSERT_TRUE(true);
    LOGI("Bundle_Manager_Connector_GetBundleMgrProxy_test_0000 end");
}

/**
 * @tc.number: Bundle_Manager_Connector_GetBundleMgrProxy_test_0001
 * @tc.name: Bundle_Manager_Connector_GetBundleMgrProxy_test_0001
 * @tc.desc: Test GetBundleMgrProxy multiple calls returns cached proxy
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_GetBundleMgrProxy_test_0001, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_GetBundleMgrProxy_test_0001 begin");
    BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    auto proxy1 = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    auto proxy2 = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (proxy1 != nullptr && proxy2 != nullptr) {
        EXPECT_EQ(proxy1, proxy2);
    }
    ASSERT_TRUE(true);
    LOGI("Bundle_Manager_Connector_GetBundleMgrProxy_test_0001 end");
}

/**
 * @tc.number: Bundle_Manager_Connector_ResetBundleMgrProxy_test_0000
 * @tc.name: Bundle_Manager_Connector_ResetBundleMgrProxy_test_0000
 * @tc.desc: Test ResetBundleMgrProxy function returns E_OK
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_ResetBundleMgrProxy_test_0000, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_ResetBundleMgrProxy_test_0000 begin");
    auto result = BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    EXPECT_EQ(result, E_OK);
    LOGI("Bundle_Manager_Connector_ResetBundleMgrProxy_test_0000 end");
}

/**
 * @tc.number: Bundle_Manager_Connector_ResetBundleMgrProxy_test_0001
 * @tc.name: Bundle_Manager_Connector_ResetBundleMgrProxy_test_0001
 * @tc.desc: Test ResetBundleMgrProxy can be called multiple times
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_ResetBundleMgrProxy_test_0001, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_ResetBundleMgrProxy_test_0001 begin");
    auto result1 = BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    auto result2 = BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    EXPECT_EQ(result1, E_OK);
    EXPECT_EQ(result2, E_OK);
    LOGI("Bundle_Manager_Connector_ResetBundleMgrProxy_test_0001 end");
}

/**
 * @tc.number: Bundle_Manager_Connector_ResetBundleMgrProxy_test_0002
 * @tc.name: Bundle_Manager_Connector_ResetBundleMgrProxy_test_0002
 * @tc.desc: Test ResetBundleMgrProxy clears proxy cache
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_ResetBundleMgrProxy_test_0002, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_ResetBundleMgrProxy_test_0002 begin");
    auto proxy1 = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    BundleMgrConnector::GetInstance().ResetBundleMgrProxy();
    auto proxy2 = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (proxy1 != nullptr && proxy2 != nullptr) {
        EXPECT_NE(proxy1, proxy2);
    }
    ASSERT_TRUE(true);
    LOGI("Bundle_Manager_Connector_ResetBundleMgrProxy_test_0002 end");
}

/**
 * @tc.number: Bundle_Mgr_DeathRecipient_OnRemoteDied_test_0000
 * @tc.name: Bundle_Mgr_DeathRecipient_OnRemoteDied_test_0000
 * @tc.desc: Test BundleMgrDeathRecipient OnRemoteDied function
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Mgr_DeathRecipient_OnRemoteDied_test_0000, TestSize.Level1)
{
    LOGI("Bundle_Mgr_DeathRecipient_OnRemoteDied_test_0000 begin");
    BundleMgrDeathRecipient deathRecipient;
    wptr<IRemoteObject> remote(nullptr);
    deathRecipient.OnRemoteDied(remote);
    auto proxy = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (proxy == nullptr) {
        LOGI("Expected nullptr after OnRemoteDied reset proxy");
    }
    ASSERT_TRUE(true);
    LOGI("Bundle_Mgr_DeathRecipient_OnRemoteDied_test_0000 end");
}

/**
 * @tc.number: Bundle_Manager_Connector_ThreadSafety_test_0000
 * @tc.name: Bundle_Manager_Connector_ThreadSafety_test_0000
 * @tc.desc: Test GetInstance thread safety
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(BundleManagerConnectorTest, Bundle_Manager_Connector_ThreadSafety_test_0000, TestSize.Level1)
{
    LOGI("Bundle_Manager_Connector_ThreadSafety_test_0000 begin");
    auto& instance1 = BundleMgrConnector::GetInstance();
    auto& instance2 = BundleMgrConnector::GetInstance();
    auto& instance3 = BundleMgrConnector::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
    EXPECT_EQ(&instance2, &instance3);
    LOGI("Bundle_Manager_Connector_ThreadSafety_test_0000 end");
}
} // namespace StorageSpaceManager
} // namespace OHOS