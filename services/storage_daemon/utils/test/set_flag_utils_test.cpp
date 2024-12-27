/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"
#include "common/help_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/set_flag_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class SetFlagUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: SetFlagUtilsTest_SetFileDelFlags_001
 * @tc.desc: Verify the SetFileDelFlags function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtilsTest_SetFileDelFlags_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtilsTest_SetFileDelFlags_001 start";
    std::string path = "/data/test/tdd";
    DeleteFile(path);
    DelFolder(path);
    EXPECT_FALSE(StorageService::SetFlagUtils::SetFileDelFlags(path));
    EXPECT_FALSE(StorageService::SetFlagUtils::SetDirDelFlags(path));
    EXPECT_TRUE(CreateFolder(path));
    EXPECT_FALSE(StorageService::SetFlagUtils::SetFileDelFlags(path));
    EXPECT_FALSE(StorageService::SetFlagUtils::SetDirDelFlags(path));

    std::string fileName = path + "/test.txt";
    auto fd = open(fileName.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);
    EXPECT_FALSE(StorageService::SetFlagUtils::SetFileDelFlags(fileName));
    EXPECT_FALSE(StorageService::SetFlagUtils::SetDirDelFlags(fileName));
    EXPECT_TRUE(DeleteFile(path) == 0);
    EXPECT_TRUE(DelFolder(path));
    GTEST_LOG_(INFO) << "SetFlagUtilsTest_SetFileDelFlags_001 end";
}

/**
 * @tc.name: SetFlagUtilsTest_ParseDirPath_001
 * @tc.desc: Verify the ParseDirPath function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtilsTest_ParseDirPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtilsTest_ParseDirPath_001 start";
    std::string basePath = "/data/test/tdd";
    std::string path = "/data/test/tdd/temp";
    DeleteFile(basePath);
    DelFolder(basePath);
    StorageService::SetFlagUtils::ParseDirPath(path);
    EXPECT_TRUE(CreateFolder(path));

    std::string fileName = basePath + "/test.txt";
    auto fd = open(fileName.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);
    close(fd);
    StorageService::SetFlagUtils::ParseDirPath(fileName);
    StorageService::SetFlagUtils::ParseDirPath(path);

    std::string wrongPath1 = basePath + "/storage_daemon/sd/99";
    EXPECT_TRUE(CreateFolder(wrongPath1));
    StorageService::SetFlagUtils::ParseDirPath(wrongPath1);

    std::string wrongPath2 = basePath + "/storage_daemon/sd/200000";
    EXPECT_TRUE(CreateFolder(wrongPath1));
    StorageService::SetFlagUtils::ParseDirPath(wrongPath2);

    std::string subPath = basePath + "/storage_daemon/sd/101";
    EXPECT_TRUE(CreateFolder(subPath));
    StorageService::SetFlagUtils::ParseDirPath(subPath);
    EXPECT_TRUE(DeleteFile(path) == 0);
    EXPECT_TRUE(DelFolder(path));

    StorageService::SetFlagUtils::ParseDirAllPath();
    GTEST_LOG_(INFO) << "SetFlagUtilsTest_ParseDirPath_001 end";
}
} // STORAGE_DAEMON
} // OHOS
