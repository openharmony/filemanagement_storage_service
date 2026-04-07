/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <memory>

#include "netlink/netlink_manager.h"
#include "netlink_listener_real_mock.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;
using namespace std;

class NetlinkManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
    static inline shared_ptr<NetlinkListenerRealMoc> netlinkListenerMoc_ = nullptr;
};

void NetlinkManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    netlinkListenerMoc_ = make_shared<NetlinkListenerRealMoc>();
    NetlinkListenerRealMoc::netlinkListenerMoc = netlinkListenerMoc_;
}

void NetlinkManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    NetlinkListenerRealMoc::netlinkListenerMoc = nullptr;
    netlinkListenerMoc_ = nullptr;
}

/**
 * @tc.name: Storage_Service_NetlinkManagerTest_Instance_001
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkManagerTest, Storage_Service_NetlinkManagerTest_Instance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_Instance_001 start";

    NetlinkManager &netlinkManager1 = NetlinkManager::Instance();
    NetlinkManager &netlinkManager2 = NetlinkManager::Instance();
    ASSERT_TRUE(&netlinkManager1 == &netlinkManager2);

    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_Instance_001 end";
}

/**
 * @tc.name: Storage_Service_NetlinkManagerTest_Stop_001
 * @tc.desc: Verify Stop returns E_OK when manager has no active handler.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkManagerTest, Storage_Service_NetlinkManagerTest_Stop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_Stop_001 start";

    NetlinkManager &netlinkManager = NetlinkManager::Instance();
    auto ret = netlinkManager.Stop();
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_Stop_001 end";
}

/**
 * @tc.name: Storage_Service_NetlinkManagerTest_Sta0rtStop_001
 * @tc.desc: Verify Start/Stop execute without crash under current runtime capability.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkManagerTest, Storage_Service_NetlinkManagerTest_StartStop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_StartStop_001 start";

    NetlinkManager &netlinkManager = NetlinkManager::Instance();
    auto startRet = netlinkManager.Start();
    EXPECT_TRUE(startRet == E_OK || startRet == E_ERR);

    auto stopRet = netlinkManager.Stop();
    EXPECT_TRUE(stopRet == E_OK || stopRet == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_StartStop_001 end";
}

/**
 * @tc.name: Storage_Service_NetlinkManagerTest_StartStop_002
 * @tc.desc: Verify Start/Stop execute without crash under current runtime capability.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkManagerTest, Storage_Service_NetlinkManagerTest_StartStop_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_StartStop_002 start";

    NetlinkManager &netlinkManager = NetlinkManager::Instance();
    EXPECT_CALL(*netlinkListenerMoc_, StartListener).WillOnce(Return(-1));
    auto startRet = netlinkManager.Start();
    EXPECT_TRUE(startRet == E_ERR);

    EXPECT_CALL(*netlinkListenerMoc_, StopListener).WillOnce(Return(-1));
    auto stopRet = netlinkManager.Stop();
    EXPECT_TRUE(stopRet == E_ERR);
    
    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_StartStop_002 end";
}
} // STORAGE_DAEMON
} // OHOS

