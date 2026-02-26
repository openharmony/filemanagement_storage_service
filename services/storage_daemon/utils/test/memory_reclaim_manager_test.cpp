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

#include "memory_reclaim_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <fcntl.h>
#include <thread>
#include <unistd.h>

namespace OHOS {
namespace StorageDaemon {
namespace Test {
using namespace testing;
using namespace testing::ext;

class MemoryReclaimManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}

    void SetUp() override {}

    void TearDown() override {}
};

/**
 * @tc.name: MemoryReclaimManagerTest_WriteToProcFile_FileNotFound
 * @tc.desc: Test WriteToProcFile when file doesn't exist (covers if(fd == -1))
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(MemoryReclaimManagerTest, WriteToProcFile_FileNotFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteToProcFile_FileNotFound start";
    // 测试不存在的文件
    EXPECT_FALSE(MemoryReclaimManager::WriteToProcFile("/tmp/nonexistent_file_xyz_123456", "test content"));

    GTEST_LOG_(INFO) << "WriteToProcFile_FileNotFound end";
}

/**
 * @tc.name: MemoryReclaimManagerTest_WriteToProcFile_AllScenarios
 * @tc.desc: Test WriteToProcFile all scenarios in one case:
 *           1) open fail (fd == -1) using nonexistent path under /data/service
 *           2) success path (return true) using normal file under /data/service
 *           3) write fail after open success (written < 0) using symlink under /data/service -> /dev/full (if exists)
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(MemoryReclaimManagerTest, WriteToProcFile_AllScenarios, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteToProcFile_AllScenarios start";
 
    const std::string baseDir = "/data/service";
 
    // 1) open fail: file doesn't exist (covers if(fd == -1))
    const std::string notExistPath = baseDir + "/nonexistent_file_xyz_123456";
    EXPECT_FALSE(MemoryReclaimManager::WriteToProcFile(notExistPath, "test content"));
 
    // 2) success: create a writable file then write (covers return true)
    const std::string okPath = baseDir + "/memory_reclaim_manager_write_test.txt";
    const std::string content = "hello_ut_content";
    FILE *fp = fopen(okPath.c_str(), "w");
    ASSERT_NE(fp, nullptr);
    fclose(fp);
    EXPECT_TRUE(MemoryReclaimManager::WriteToProcFile(okPath, content));
    (void)remove(okPath.c_str());
 
    // 3) write fail after open success (covers written < 0)
    //    Use /data/service symlink -> /dev/full (common on Linux: open ok, write fails with ENOSPC)
    const std::string devFull = "/dev/full";
    const std::string linkPath = baseDir + "/dev_full_link_for_ut";
    if (access(devFull.c_str(), F_OK) != 0) {
        GTEST_LOG_(INFO) << "/dev/full not exist, skip write-fail sub-scenario";
        GTEST_LOG_(INFO) << "WriteToProcFile_AllScenarios end";
    } else {
        (void)unlink(linkPath.c_str());
        int ret = symlink(devFull.c_str(), linkPath.c_str());
        ASSERT_EQ(ret, 0);
        EXPECT_FALSE(MemoryReclaimManager::WriteToProcFile(linkPath, "test content"));
        (void)unlink(linkPath.c_str());
    
        GTEST_LOG_(INFO) << "WriteToProcFile_AllScenarios end";
    }
}
} // namespace Test
} // namespace StorageDaemon
} // namespace OHOS