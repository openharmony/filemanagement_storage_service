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

#include "disk/disk_config.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class DiskConfigTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_DiskConfigTest_IsMatch_001
 * @tc.desc: Verify the IsMatch function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskConfigTest, Storage_Service_DiskManagerTest_IsMatch_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_IsMatch_001 start";
    std::string sysPattern = "/devices/platform/fe2b0000.dwmmc/*";
    std::string lable = "disk";
    int flag = 0;
    auto diskConfig = std::make_shared<DiskConfig>(sysPattern, lable, flag);
    bool ret = diskConfig->IsMatch(sysPattern);
    EXPECT_TRUE(ret == true);
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_IsMatch_001 end";
}

/**
 * @tc.name: Storage_Service_DiskConfigTest_GetFlag_001
 * @tc.desc: Verify the GetFlag function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskConfigTest, Storage_Service_DiskManagerTest_GetFlag_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_GetFlag_001 start";
    std::string sysPattern = "/devices/platform/fe2b0000.dwmmc/*";
    std::string lable = "disk";
    int flag = 0;
    auto diskConfig = std::make_shared<DiskConfig>(sysPattern, lable, flag);
    int ret = diskConfig->GetFlag();
    EXPECT_TRUE(ret == flag);
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_GetFlag_001 end";
}
}
}
