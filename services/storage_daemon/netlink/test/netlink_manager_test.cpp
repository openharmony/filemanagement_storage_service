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

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "netlink/netlink_manager.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class NetlinkManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_NetlinkManagerTest_Instance_001
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkManagerTest, Storage_Service_NetlinkManagerTest_Instance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_Instance_001 start";

    NetlinkManager *netlinkManager = NetlinkManager::Instance();
    ASSERT_TRUE(netlinkManager != nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_NetlinkManagerTest_Instance_001 end";
}
} // STORAGE_DAEMON
} // OHOS