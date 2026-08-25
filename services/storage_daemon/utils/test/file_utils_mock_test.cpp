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

#include <csetjmp>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "storage_service_errno.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

// Function pointers that default to real system calls.
// Tests override them to inject failures for pipe/fork.
static int (*g_sPipeFn)(int[2]) = ::pipe;
static pid_t (*g_sForkFn)() = ::fork;

} // namespace StorageDaemon
} // namespace OHOS

// Mock implementations that delegate to function pointers.
// file_utils.cpp (compiled via file_utils_for_mock_test.cpp) has
// #define pipe PipeMock and #define fork ForkMock, so all pipe/fork
// calls in the source are redirected here.
// extern "C" is required because pipe/fork are C library functions with C linkage.

extern "C" int PipeMock(int fd[2])
{
    return OHOS::StorageDaemon::g_sPipeFn(fd);
}

extern "C" pid_t ForkMock()
{
    return OHOS::StorageDaemon::g_sForkFn();
}

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class FileUtilsMockTest : public testing::Test {
public:
    void SetUp() override
    {
        g_sPipeFn = ::pipe;
        g_sForkFn = ::fork;
    }
    void TearDown() override {}
};

/**
 * @tc.name: FileUtilsMockTest_ForkExecToFile_ForkFailed
 * @tc.desc: Verify ForkExecToFile returns E_FORK when fork() fails.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsMockTest, FileUtilsMockTest_ForkExecToFile_ForkFailed, TestSize.Level1)
{
    std::vector<std::string> cmd = {"echo", "test"};
    std::string outputFilePath = "/data/service/fork_exec_mock_fork_fail.txt";
    std::vector<std::string> output;
    g_sForkFn = []() -> pid_t { errno = EAGAIN; return -1; };
    int ret = ForkExecToFile(cmd, outputFilePath, &output);
    EXPECT_EQ(ret, E_FORK);
}

} // namespace StorageDaemon
} // namespace OHOS
