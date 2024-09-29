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
#include <gtest/gtest.h>

#include "netlink/netlink_data.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
    const char* ERROR_MESSAGE = "AAAAAABBBBBBCCCCCC\0param=add\0";
    const char* DEVPATH_TEST = "DEVPATH=/dev/test\0ACTION=add\0";
    const char* SUBSYSTEM_TEST = "SUBSYSTEM=ABCABC\0ACTION=add\0";
    const char* PARAM_TEST = "ParamName=test\0ACTION=add\0";
}
using namespace testing::ext;

class NetlinkDataTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: NetLinkDataTest_Decode_001
 * @tc.desc: Verify the Decode function when msg is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(NetlinkDataTest, NetlinkDataTest_Decode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_001 start";

    NetlinkData netlinkData;
    netlinkData.Decode(ERROR_MESSAGE);

    EXPECT_TRUE(netlinkData.GetSyspath().compare("") == 0) << "check the Syspath";
    EXPECT_TRUE(netlinkData.GetDevpath().compare("") == 0) << "check the Devpath";
    EXPECT_TRUE(netlinkData.GetSubsystem().compare("") == 0) << "check the Subsystem";
    EXPECT_TRUE(netlinkData.GetAction() == NetlinkData::Actions::UNKNOWN) << "check the Action";

    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_001 end";
}

/**
 * @tc.name: NetlinkDataTest_Decode_002
 * @tc.desc: Verify the Decode function of ACTION.
 * @tc.type: FUNC
 */
HWTEST_F(NetlinkDataTest, NetlinkDataTest_Decode_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_002 start";

    std::map<std::string, NetlinkData::Actions> actions = {
        {"ACTION=add\0DEVPATH=/dev/test\0", NetlinkData::Actions::ADD},
        {"ACTION=remove\0DEVPATH=/dev/test\0", NetlinkData::Actions::REMOVE},
        {"ACTION=move\0DEVPATH=/dev/test\0", NetlinkData::Actions::MOVE},
        {"ACTION=change\0DEVPATH=/dev/test\0", NetlinkData::Actions::CHANGE},
        {"ACTION=online\0DEVPATH=/dev/test\0", NetlinkData::Actions::ONLINE},
        {"ACTION=offline\0DEVPATH=/dev/test\0", NetlinkData::Actions::OFFLINE},
        {"ACTION=bind\0DEVPATH=/dev/test\0", NetlinkData::Actions::BIND},
        {"ACTION=unbind\0DEVPATH=/dev/test\0", NetlinkData::Actions::UNBIND}
    };

    NetlinkData netlinkData;
    EXPECT_TRUE(actions.size() == netlinkData.actionMaps.size())
        << "confirm test action count as the same of the actual";

    for (auto action : actions) {
        netlinkData.Decode(action.first.c_str());
        EXPECT_TRUE(netlinkData.GetAction() == action.second) << "check the Action";
    }

    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_002 end";
}

/**
 * @tc.name: NetLinkDataTest_Decode_003
 * @tc.desc: Verify the Decode function of Devpath and Syspath.
 * @tc.type: FUNC
 */
HWTEST_F(NetlinkDataTest, NetlinkDataTest_Decode_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_003 start";

    NetlinkData netlinkData;
    netlinkData.Decode(DEVPATH_TEST);
    EXPECT_TRUE(netlinkData.GetDevpath().compare("/dev/test") == 0) << "check the Devpath";
    EXPECT_TRUE(netlinkData.GetSyspath().compare("/sys/dev/test") == 0) << "check the Syspath";

    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_003 end";
}

/**
 * @tc.name: NetLinkDataTest_Decode_004
 * @tc.desc: Verify the Decode function of Subsystem.
 * @tc.type: FUNC
 */
HWTEST_F(NetlinkDataTest, NetlinkDataTest_Decode_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_004 start";

    NetlinkData netlinkData;
    netlinkData.Decode(SUBSYSTEM_TEST);
    EXPECT_TRUE(netlinkData.GetSubsystem().compare("ABCABC") == 0) << "check the Subsystem";

    GTEST_LOG_(INFO) << "NetlinkDataTest_Decode_004 end";
}

/**
 * @tc.name: NetlinkDataTest_GetParam_001
 * @tc.desc: Verify the GetParam function when para name is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(NetlinkDataTest, NetlinkDataTest_GetParam_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetLinkDataTest_GetParam_001 start";

    NetlinkData netlinkData;
    netlinkData.Decode(PARAM_TEST);
    std::string param = netlinkData.GetParam("IncorrectParamName");
    EXPECT_TRUE(param.compare("") == 0) << "get \"\" when para name is incorrect";

    GTEST_LOG_(INFO) << "NetlinkDataTest_GetParam_001 end";
}

/**
 * @tc.name: NetLinkDataTest_GetParam_002
 * @tc.desc: Verify the GetParam function when para name is correct.
 * @tc.type: FUNC
 */
HWTEST_F(NetlinkDataTest, NetlinkDataTest_GetParam_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkDataTest_GetParam_002 start";

    NetlinkData netlinkData;
    netlinkData.Decode(PARAM_TEST);
    std::string param = netlinkData.GetParam("ParamName");
    EXPECT_TRUE(param.compare("test") == 0) << "check GetParam when para name is correct";

    GTEST_LOG_(INFO) << "NetlinkDataTest_GetParam_002 end";
}
} // STORAGE_DAEMON
} // OHOS
