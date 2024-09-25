/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "key_backup.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "directory_ex.h"

using namespace std;
using namespace testing::ext;
using namespace testing;
 
namespace OHOS::StorageDaemon {
constexpr static mode_t DEFAULT_WRITE_FILE_PERM = 0644;
constexpr static uint32_t MAX_FILE_NUM = 5;
constexpr uint32_t INVALID_LOOP_NUM = 0xFFFFFFFF;
class KeyBackupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KeyBackupTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void KeyBackupTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void KeyBackupTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
}

void KeyBackupTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
}

/**
 * @tc.name: KeyBackup_GetBackupDir_000
 * @tc.desc: Verify the GetBackupDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_GetBackupDir_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_GetBackupDir_000 Start";
    std::string origDir = DEVICE_EL1_DIR;
    std::string backupDir;
    EXPECT_EQ(KeyBackup::GetInstance().GetBackupDir(origDir, backupDir), 0);
    EXPECT_EQ(backupDir, DEVICE_EL1_DIR + BACKUP_NAME);

    origDir = "test";
    backupDir.clear();
    EXPECT_EQ(KeyBackup::GetInstance().GetBackupDir(origDir, backupDir), -1);
    EXPECT_EQ(backupDir, "");

    origDir = "/test";
    EXPECT_EQ(KeyBackup::GetInstance().GetBackupDir(origDir, backupDir), -1);
    EXPECT_EQ(backupDir, "");

    origDir = "test/path/";
    EXPECT_EQ(KeyBackup::GetInstance().GetBackupDir(origDir, backupDir), 0);
    EXPECT_EQ(backupDir, "test/path_bak/");

    GTEST_LOG_(INFO) << "KeyBackup_GetBackupDir_000 end";
}

/**
 * @tc.name: KeyBackup_GetLoopMaxNum_001
 * @tc.desc: Verify the GetLoopMaxNum function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_GetLoopMaxNum_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_GetLoopMaxNum_001 Start";
    EXPECT_EQ(KeyBackup::GetInstance().GetLoopMaxNum(MAX_FILE_NUM + 1), INVALID_LOOP_NUM);
    EXPECT_EQ(KeyBackup::GetInstance().GetLoopMaxNum(MAX_FILE_NUM), 31);
    GTEST_LOG_(INFO) << "KeyBackup_GetLoopMaxNum_001 end";
}

/**
 * @tc.name: KeyBackup_HandleCopyDir_001
 * @tc.desc: Verify the HandleCopyDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_HandleCopyDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_HandleCopyDir_001 Start";
    string toPath = "/data/test/KeyBackup/test3";
    string fromPath = "/data/test/KeyBackup/test2/";
    EXPECT_FALSE(access(toPath.c_str(), F_OK) == 0);
    EXPECT_FALSE(access(fromPath.c_str(), F_OK) == 0);
    EXPECT_EQ(KeyBackup::GetInstance().HandleCopyDir(fromPath, toPath), -1);

    string baseDir = "/data/test/KeyBackup/";
    EXPECT_EQ(KeyBackup::GetInstance().MkdirParent(baseDir, DEFAULT_WRITE_FILE_PERM), 0);
    EXPECT_EQ(KeyBackup::GetInstance().HandleCopyDir(fromPath, toPath), 0);

    EXPECT_EQ(KeyBackup::GetInstance().MkdirParent(fromPath, DEFAULT_WRITE_FILE_PERM), 0);
    EXPECT_TRUE(access(fromPath.c_str(), F_OK) == 0);
    EXPECT_EQ(KeyBackup::GetInstance().HandleCopyDir(fromPath, toPath), 0);
    struct FileAttr attr;
    struct FileAttr attr2;
    EXPECT_EQ(KeyBackup::GetInstance().GetAttr(toPath, attr), 0);
    EXPECT_EQ(KeyBackup::GetInstance().GetAttr(fromPath, attr2), 0);
    EXPECT_EQ(attr.mode, attr2.mode);
    EXPECT_TRUE(OHOS::ForceRemoveDirectory(baseDir));
    EXPECT_FALSE(access(baseDir.c_str(), F_OK) == 0);
    GTEST_LOG_(INFO) << "KeyBackup_HandleCopyDir_001 end";
}
}
