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
#include <linux/kdev_t.h>
#include <sys/mount.h>
#include "external_volume_info.h"
#include "process.h"
#include "external_volume_info_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class ProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_ProcessTest_CheckSubDir_001
 * @tc.desc: Verify the CheckSubDir function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ProcessTest, Storage_Service_ProcessTest_CheckSubDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_CheckSubDir_001 start";
    Process process1("/usr/local/bin");
    EXPECT_FALSE(process1.CheckSubDir("/usr/local"));

    Process process2("/usr/local/bin");
    EXPECT_FALSE(process2.CheckSubDir("/usr/local/"));

    Process process3("/usr/local/bin");
    EXPECT_FALSE(process3.CheckSubDir("/usr/locala"));

    Process process4("/usr/local/bin");
    EXPECT_TRUE(process4.CheckSubDir("/usr/local/bin/extra"));

    Process process5("/usr/local/bin");
    EXPECT_FALSE(process5.CheckSubDir("/usr/local/b"));

    Process process6("/usr/local/bin");
    EXPECT_FALSE(process6.CheckSubDir(""));

    Process process7("/usr/local/bin");
    EXPECT_TRUE(process7.CheckSubDir("/usr/local/bin/"));

    Process process8("/usr/local/bin");
    EXPECT_TRUE(process8.CheckSubDir("/usr/local/bin"));

    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_CheckSubDir_001 end";
}

/**
 * @tc.name: Storage_Service_ProcessTest_CheckFds_001
 * @tc.desc: Verify the CheckFds function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ProcessTest, Storage_Service_ProcessTest_CheckFds_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_CheckFds_001 start";
    Process process1("/path/to/process");
    EXPECT_FALSE(process1.CheckFds("/proc/1"));

    Process process2("/path/to/process");
    EXPECT_FALSE(process2.CheckFds("/proc/111111"));
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_CheckFds_001 end";
}

/**
 * @tc.name: Storage_Service_ProcessTest_UpdatePidAndKill_001
 * @tc.desc: Verify the UpdatePidAndKill function to fix TOCTOU vulnerability.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ProcessTest, Storage_Service_ProcessTest_UpdatePidAndKill_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_UpdatePidAndKill_001 start";
    Process process1("/path/to/process");
    auto ret = process1.UpdatePidAndKill(0);
    EXPECT_TRUE(ret == E_OK);

    Process process2("/path/to/process");
    ret = process2.UpdatePidAndKill(SIGKILL);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_UpdatePidAndKill_001 end";
}
} // STORAGE_DAEMON
} // OHOS
