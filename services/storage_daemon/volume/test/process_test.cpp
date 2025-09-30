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
 * @tc.name: Storage_Service_ProcessTest_GetPath_001
 * @tc.desc: Verify the GetPath function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ProcessTest, Storage_Service_ProcessTest_GetXXX_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_GetXXX_001 start";

    std::string volId = "vol-1-1";
    std::string diskId = "disk-1-1";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    uint32_t mountFlags = 0;
    ExternalVolumeInfoMock mock;
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoMount(testing::_)).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device, isUserdata);
    EXPECT_TRUE(ret == E_OK);

    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_OK);

    auto mountPath = mock.GetMountPath();
    Process ps(mountPath);
    ps.UpdatePidByPath();
    EXPECT_TRUE(ps.GetPath() == mountPath);
    ps.GetPids();

    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_GetXXX_001 end";
}

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
    EXPECT_TRUE(process2.CheckFds("/proc/111111"));
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_CheckFds_001 end";
}

/**
 * @tc.name: Storage_Service_ProcessTest_KillProcess_001
 * @tc.desc: Verify the KillProcess function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ProcessTest, Storage_Service_ProcessTest_KillProcess_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_KillProcess_001 start";
    Process process1("/path/to/process");
    process1.KillProcess(0);

    Process process2("/path/to/process");
    process2.pids_ = {12345, 56789};
    process2.KillProcess(SIGKILL);
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_KillProcess_001 end";
}
} // STORAGE_DAEMON
} // OHOS
