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

#include "utils/set_flag_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace StorageService {
namespace Test {

class SetFlagUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    std::string testRootPath_;
    std::string testEmptyDir_;
    std::string testFileDir_;
    std::string testNestedDir_;

    void CreateTestDirectories();
    void CleanupTestDirectories();
};

void SetFlagUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetFlagUtilsTest SetUpTestCase Start";
}

void SetFlagUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "SetFlagUtilsTest TearDownTestCase Start";
}

void SetFlagUtilsTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetFlagUtilsTest SetUp Start";

    testRootPath_ = "/data/service/el2/set_flag_utils_ut";
    std::filesystem::create_directories(testRootPath_);

    CreateTestDirectories();
}

void SetFlagUtilsTest::TearDown()
{
    GTEST_LOG_(INFO) << "SetFlagUtilsTest TearDown Start";
    CleanupTestDirectories();
}

void SetFlagUtilsTest::CreateTestDirectories()
{
    testEmptyDir_ = testRootPath_ + "/empty_dir";
    std::filesystem::create_directory(testEmptyDir_);

    testFileDir_ = testRootPath_ + "/file_dir";
    std::filesystem::create_directory(testFileDir_);

    std::ofstream(testFileDir_ + "/file1.txt").close();
    std::ofstream(testFileDir_ + "/file2.txt").close();

    testNestedDir_ = testFileDir_ + "/nested_dir";
    std::filesystem::create_directory(testNestedDir_);
    std::ofstream(testNestedDir_ + "/nested_file.txt").close();
}

void SetFlagUtilsTest::CleanupTestDirectories()
{
    if (!testRootPath_.empty()) {
        std::filesystem::remove_all(testRootPath_);
    }
}

/**
 * @tc.name: SetFlagUtils_SetDelFlagsRecursive_EmptyDirectory_001
 * @tc.desc: Test ParseDirPath with empty directory - no files to process
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDelFlagsRecursive_EmptyDirectory_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_EmptyDirectory_001 Start";

    EXPECT_TRUE(std::filesystem::exists(testEmptyDir_));
    EXPECT_TRUE(std::filesystem::is_directory(testEmptyDir_));

    SetFlagUtils::SetDelFlagsRecursive(testEmptyDir_);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_EmptyDirectory_001 end";
}

/**
 * @tc.name: SetFlagUtils_SetDelFlagsRecursive_WithFiles_002
 * @tc.desc: Test ParseDirPath with directory containing files
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDelFlagsRecursive_WithFiles_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_WithFiles_002 Start";

    EXPECT_TRUE(std::filesystem::exists(testFileDir_));
    EXPECT_TRUE(std::filesystem::is_directory(testFileDir_));
    EXPECT_TRUE(std::filesystem::exists(testFileDir_ + "/file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(testFileDir_ + "/file2.txt"));

    int fileCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(testFileDir_)) {
        (void)entry;
        fileCount++;
    }
    EXPECT_EQ(fileCount, 3);

    SetFlagUtils::SetDelFlagsRecursive(testFileDir_);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_WithFiles_002 end";
}

/**
 * @tc.name: SetFlagUtils_SetDelFlagsRecursive_NestedDirectories_003
 * @tc.desc: Test ParseDirPath with nested directory structure
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDelFlagsRecursive_NestedDirectories_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_NestedDirectories_003 Start";

    EXPECT_TRUE(std::filesystem::exists(testNestedDir_));
    EXPECT_TRUE(std::filesystem::is_directory(testNestedDir_));
    EXPECT_TRUE(std::filesystem::exists(testNestedDir_ + "/nested_file.txt"));

    SetFlagUtils::SetDelFlagsRecursive(testFileDir_);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_NestedDirectories_003 end";
}

/**
 * @tc.name: SetFlagUtils_SetDelFlagsRecursive_NonExistentPath_004
 * @tc.desc: Test ParseDirPath with non-existent path - should handle gracefully
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDelFlagsRecursive_NonExistentPath_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_NonExistentPath_004 Start";

    std::string nonExistentPath = testRootPath_ + "/non_existent_dir";

    EXPECT_FALSE(std::filesystem::exists(nonExistentPath));
    SetFlagUtils::SetDelFlagsRecursive(nonExistentPath);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDelFlagsRecursive_NonExistentPath_004 end";
}

/**
 * @tc.name: SetFlagUtils_SetFileDelFlags_ValidFile_005
 * @tc.desc: Test SetFileDelFlags with valid file path
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetFileDelFlags_ValidFile_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetFileDelFlags_ValidFile_005 Start";

    std::string testFile = testFileDir_ + "/file1.txt";

    EXPECT_TRUE(std::filesystem::exists(testFile));
    EXPECT_FALSE(std::filesystem::is_directory(testFile));

    SetFlagUtils::SetFileDelFlags(testFile);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetFileDelFlags_ValidFile_005 end";
}

/**
 * @tc.name: SetFlagUtils_SetFileDelFlags_NonExistentFile_006
 * @tc.desc: Test SetFileDelFlags with non-existent file path
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetFileDelFlags_NonExistentFile_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetFileDelFlags_NonExistentFile_006 Start";

    std::string nonExistentFile = testRootPath_ + "/non_existent_file.txt";

    EXPECT_FALSE(std::filesystem::exists(nonExistentFile));

    SetFlagUtils::SetFileDelFlags(nonExistentFile);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetFileDelFlags_NonExistentFile_006 end";
}

/**
 * @tc.name: SetFlagUtils_SetDirDelFlags_ValidDirectory_007
 * @tc.desc: Test SetDirDelFlags with valid directory path
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDirDelFlags_ValidDirectory_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDirDelFlags_ValidDirectory_007 Start";

    EXPECT_TRUE(std::filesystem::exists(testFileDir_));
    EXPECT_TRUE(std::filesystem::is_directory(testFileDir_));

    SetFlagUtils::SetDirDelFlags(testFileDir_);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDirDelFlags_ValidDirectory_007 end";
}

/**
 * @tc.name: SetFlagUtils_SetDirDelFlags_EmptyDirectory_008
 * @tc.desc: Test SetDirDelFlags with empty directory
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDirDelFlags_EmptyDirectory_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDirDelFlags_EmptyDirectory_008 Start";

    EXPECT_TRUE(std::filesystem::exists(testEmptyDir_));
    EXPECT_TRUE(std::filesystem::is_directory(testEmptyDir_));

    SetFlagUtils::SetDirDelFlags(testEmptyDir_);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDirDelFlags_EmptyDirectory_008 end";
}

/**
 * @tc.name: SetFlagUtils_SetDirDelFlags_NonExistentDirectory_009
 * @tc.desc: Test SetDirDelFlags with non-existent directory path
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_SetDirDelFlags_NonExistentDirectory_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_SetDirDelFlags_NonExistentDirectory_009 Start";

    std::string nonExistentDir = testRootPath_ + "/non_existent_dir";

    EXPECT_FALSE(std::filesystem::exists(nonExistentDir));

    SetFlagUtils::SetDirDelFlags(nonExistentDir);

    GTEST_LOG_(INFO) << "SetFlagUtils_SetDirDelFlags_NonExistentDirectory_009 end";
}

/**
 * @tc.name: SetFlagUtils_Integration_ParseAndSet_010
 * @tc.desc: Integration test - ParseDirPath followed by SetFileDelFlags
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_Integration_ParseAndSet_010, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_Integration_ParseAndSet_010 Start";

    EXPECT_TRUE(std::filesystem::exists(testFileDir_));

    SetFlagUtils::SetDelFlagsRecursive(testFileDir_);

    EXPECT_TRUE(std::filesystem::exists(testFileDir_ + "/file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(testFileDir_ + "/file2.txt"));

    GTEST_LOG_(INFO) << "SetFlagUtils_Integration_ParseAndSet_010 end";
}

/**
 * @tc.name: SetFlagUtils_Boundary_LongPath_011
 * @tc.desc: Test with very long path name
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_Boundary_LongPath_011, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_Boundary_LongPath_011 Start";

    std::string longDirName = "a";
    for (int i = 0; i < 100; i++) {
        longDirName += "a";
    }

    std::string longPath = testRootPath_ + "/" + longDirName;
    std::filesystem::create_directory(longPath);
    std::ofstream(longPath + "/test.txt").close();

    EXPECT_TRUE(std::filesystem::exists(longPath));

    SetFlagUtils::SetDelFlagsRecursive(longPath);

    GTEST_LOG_(INFO) << "SetFlagUtils_Boundary_LongPath_011 end";
}

/**
 * @tc.name: SetFlagUtils_Concurrency_MultipleCalls_012
 * @tc.desc: Test multiple concurrent calls to ParseDirPath
 * @tc.type: FUNC
 */
HWTEST_F(SetFlagUtilsTest, SetFlagUtils_Concurrency_MultipleCalls_012, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "SetFlagUtils_Concurrency_MultipleCalls_012 Start";

    std::vector<std::string> testDirs;
    for (int i = 0; i < 3; i++) {
        std::string dirPath = testRootPath_ + "/dir_" + std::to_string(i);
        std::filesystem::create_directory(dirPath);
        std::ofstream(dirPath + "/file.txt").close();
        testDirs.push_back(dirPath);
    }

    std::vector<std::thread> threads;
    for (const auto& dir : testDirs) {
        threads.emplace_back([dir]() {
            SetFlagUtils::SetDelFlagsRecursive(dir);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    GTEST_LOG_(INFO) << "SetFlagUtils_Concurrency_MultipleCalls_012 end";
}

}  // namespace Test
}  // namespace StorageService
}  // namespace OHOS
