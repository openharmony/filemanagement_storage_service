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

} // namespace Test
} // namespace StorageDaemon
} // namespace OHOS