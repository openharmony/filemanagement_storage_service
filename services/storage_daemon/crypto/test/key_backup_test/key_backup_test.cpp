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
const string TEST_PATH = "/data/tdd";
constexpr const char *BACKUP_NAME = "_bak";
class KeyBackupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

struct FileNode {
    std::string baseName;
    std::string origFile;
    std::string backFile;
    bool isSame;
};

void KeyBackupTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    ForceCreateDirectory(TEST_PATH);
}

void KeyBackupTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    ForceRemoveDirectory(TEST_PATH);
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
    EXPECT_EQ(backupDir, std::string(DEVICE_EL1_DIR) + BACKUP_NAME);

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

/**
 * @tc.name: KeyBackup_GetRealPath_001
 * @tc.desc: Verify the GetRealPath function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_GetRealPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_GetRealPath_001 Start";
    std::string path(PATH_MAX + 1, 't');
    std::string realPath;
    EXPECT_FALSE(KeyBackup::GetInstance().GetRealPath(path, realPath));
    path.clear();
    path = "test";
    EXPECT_FALSE(KeyBackup::GetInstance().GetRealPath(path, realPath));

    path.clear();
    EXPECT_TRUE(KeyBackup::GetInstance().GetRealPath(TEST_PATH, realPath));
    std::string eptPath = "/data/tdd";
    EXPECT_EQ(eptPath, realPath);
    GTEST_LOG_(INFO) << "KeyBackup_GetRealPath_001 end";
}

/**
 * @tc.name: KeyBackup_WriteStringToFile_001
 * @tc.desc: Verify the WriteStringToFile function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_WriteStringToFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_WriteStringToFile_001 Start";
    std::string path = TEST_PATH + "/test/test.txt";
    std::string payload = "this is a test content";
    EXPECT_FALSE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    path.clear();

    path = TEST_PATH + "/test.txt";
    EXPECT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    unlink(path.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_WriteStringToFile_001 end";
}

/**
 * @tc.name: KeyBackup_ReadFileToString_001
 * @tc.desc: Verify the ReadFileToString function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_ReadFileToString_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_ReadFileToString_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    unlink(path.c_str());
    std::string content;
    EXPECT_FALSE(KeyBackup::GetInstance().ReadFileToString(path, content));
    EXPECT_FALSE(KeyBackup::GetInstance().ReadFileToString(path, content));

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    EXPECT_TRUE(KeyBackup::GetInstance().ReadFileToString(path, content));
    EXPECT_EQ(content, payload);
    unlink(path.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_ReadFileToString_001 end";
}

/**
 * @tc.name: KeyBackup_CompareFile_001
 * @tc.desc: Verify the CompareFile function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CompareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CompareFile_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    std::string path2 = TEST_PATH + "/test2.txt";
    unlink(path.c_str());
    unlink(path2.c_str());
    std::string content;
    std::string content2;
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), -1);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), -1);

    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path2));
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), 0);
    unlink(path.c_str());
    unlink(path2.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CompareFile_001 end";
}

/**
 * @tc.name: KeyBackup_CheckAndCopyOneFile_001
 * @tc.desc: Verify the CheckAndCopyOneFile function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CheckAndCopyOneFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CheckAndCopyOneFile_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    std::string path2 = TEST_PATH + "/test2.txt";
    unlink(path.c_str());
    unlink(path2.c_str());
    std::string content;
    std::string content2;
    EXPECT_EQ(KeyBackup::GetInstance().CheckAndCopyOneFile(path, path2), -1);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    EXPECT_EQ(KeyBackup::GetInstance().CheckAndCopyOneFile(path, path2), 0);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), 0);

    EXPECT_EQ(KeyBackup::GetInstance().CheckAndCopyOneFile(path, path2), 0);

    unlink(path.c_str());
    unlink(path2.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CheckAndCopyOneFile_001 end";
}

/**
 * @tc.name: KeyBackup_CleanFile_001
 * @tc.desc: Verify the CleanFile function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CleanFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CleanFile_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    unlink(path.c_str());

    KeyBackup::GetInstance().CleanFile(path);
    KeyBackup::GetInstance().CleanFile(TEST_PATH);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    struct stat statbuf;
    stat(path.c_str(), &statbuf);
    EXPECT_EQ(statbuf.st_size, payload.size());
    KeyBackup::GetInstance().CleanFile(path);
    stat(path.c_str(), &statbuf);
    EXPECT_EQ(statbuf.st_size, 0);
    unlink(path.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CleanFile_001 end";
}

/**
 * @tc.name: KeyBackup_FsyncFile_001
 * @tc.desc: Verify the FsyncFile function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_FsyncFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_FsyncFile_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    unlink(path.c_str());

    KeyBackup::GetInstance().FsyncFile(path);
    KeyBackup::GetInstance().FsyncFile(TEST_PATH);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    KeyBackup::GetInstance().FsyncFile(path);
    unlink(path.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_FsyncFile_001 end";
}

/**
 * @tc.name: KeyBackup_RemoveNode_001
 * @tc.desc: Verify the RemoveNode function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_RemoveNode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_RemoveNode_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    unlink(path.c_str());

    EXPECT_EQ(KeyBackup::GetInstance().RemoveNode(path), 0);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    EXPECT_EQ(KeyBackup::GetInstance().RemoveNode(path), 0);
    EXPECT_NE(access(path.c_str(), 0), 0);
    GTEST_LOG_(INFO) << "KeyBackup_FsyncFile_001 end";
}

/**
 * @tc.name: KeyBackup_RemoveNode_002
 * @tc.desc: Verify the RemoveNode function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_RemoveNode_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_RemoveNode_002 Start";
    std::string path = TEST_PATH + "/test.txt";
    string baseDir = TEST_PATH + "/test";
    string subDir = baseDir + "/node";
    EXPECT_EQ(KeyBackup::GetInstance().MkdirParent(subDir, DEFAULT_WRITE_FILE_PERM), 0);

    std::string fileName = baseDir + "/test.txt";
    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, fileName));
    EXPECT_EQ(KeyBackup::GetInstance().RemoveNode(baseDir), 0);
    EXPECT_NE(access(baseDir.c_str(), 0), 0);
    EXPECT_NE(access(subDir.c_str(), 0), 0);
    EXPECT_NE(access(fileName.c_str(), 0), 0);
    GTEST_LOG_(INFO) << "KeyBackup_RemoveNode_002 end";
}

/**
 * @tc.name: KeyBackup_CheckAndCopyFiles_001
 * @tc.desc: Verify the CheckAndCopyFiles function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CheckAndCopyFiles_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CheckAndCopyFiles_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    std::string path2 = TEST_PATH + "/bak.txt";

    KeyBackup::GetInstance().CheckAndCopyFiles(path, path2);
    EXPECT_NE(access(path.c_str(), 0), 0);
    EXPECT_NE(access(path2.c_str(), 0), 0);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    KeyBackup::GetInstance().CheckAndCopyFiles(path, path2);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), 0);

    string bakDir = TEST_PATH + "/bak/test";
    KeyBackup::GetInstance().CheckAndCopyFiles(TEST_PATH, bakDir);
    EXPECT_NE(access(bakDir.c_str(), 0), 0);

    bakDir.clear();
    bakDir = "/data/bak";
    KeyBackup::GetInstance().CheckAndCopyFiles(TEST_PATH, bakDir);
    EXPECT_EQ(access(bakDir.c_str(), 0), 0);
    string fileName = bakDir + "/test.txt";
    string fileName2 = bakDir + "/bak.txt";
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, fileName), 0);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path2, fileName2), 0);

    EXPECT_EQ(KeyBackup::GetInstance().RemoveNode(bakDir), 0);
    unlink(path.c_str());
    unlink(path2.c_str());

    KeyBackup::GetInstance().CheckAndCopyFiles(TEST_PATH, bakDir);
    EXPECT_EQ(access(bakDir.c_str(), 0), 0);
    EXPECT_EQ(KeyBackup::GetInstance().RemoveNode(bakDir), 0);
    GTEST_LOG_(INFO) << "KeyBackup_CheckAndCopyFiles_001 end";
}

/**
 * @tc.name: KeyBackup_CreateBackup_001
 * @tc.desc: Verify the CreateBackup function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CreateBackup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CreateBackup_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    std::string path2 = TEST_PATH + "/test2.txt";
    unlink(path.c_str());
    unlink(path2.c_str());
    KeyBackup::GetInstance().CreateBackup(path, path2, true);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    std::string payload2 = "test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload2, path2));
    KeyBackup::GetInstance().CreateBackup(path, path2, true);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), 0);

    KeyBackup::GetInstance().CleanFile(path2);
    EXPECT_NE(KeyBackup::GetInstance().CompareFile(path, path2), 0);
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload2, path2));
    KeyBackup::GetInstance().CreateBackup(path, path2, false);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), 0);
    unlink(path.c_str());
    unlink(path2.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CreateBackup_001 end";
}

/**
 * @tc.name: KeyBackup_CreateBackup_002
 * @tc.desc: Verify the CreateBackup function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CreateBackup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CreateBackup_002 Start";
    std::string path = TEST_PATH + "/test.txt";
    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    std::string baseDir = "/data/bak";
    std::string path2 = baseDir + "/test.txt";

    KeyBackup::GetInstance().CreateBackup(TEST_PATH, baseDir, true);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, path2), 0);
    unlink(path.c_str());
    EXPECT_EQ(KeyBackup::GetInstance().RemoveNode(baseDir), 0);
    GTEST_LOG_(INFO) << "KeyBackup_CreateBackup_002 end";
}

/**
 * @tc.name: KeyBackup_IsRegFile_001
 * @tc.desc: Verify the IsRegFile function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_IsRegFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_IsRegFile_001 Start";
    std::string path = TEST_PATH + "/test.txt";
    unlink(path.c_str());
    EXPECT_FALSE(KeyBackup::GetInstance().IsRegFile(path));
    EXPECT_FALSE(KeyBackup::GetInstance().IsRegFile(TEST_PATH));

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, path));
    EXPECT_TRUE(KeyBackup::GetInstance().IsRegFile(path));
    GTEST_LOG_(INFO) << "KeyBackup_IsRegFile_001 end";
}

/**
 * @tc.name: KeyBackup_ListAndCheckDir_001
 * @tc.desc: Verify the ListAndCheckDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_ListAndCheckDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_ListAndCheckDir_001 Start";
    std::string errPath = "test";
    KeyBackup::GetInstance().ListAndCheckDir(errPath);

    std::string path = TEST_PATH + "/test.txt";
    unlink(path.c_str());
    KeyBackup::GetInstance().ListAndCheckDir(path);

    std::string bkpDir = TEST_PATH + "_bak";
    std::string bkpPath = bkpDir + "/test.txt";
    EXPECT_NE(access(path.c_str(), 0), 0);
    ForceCreateDirectory(bkpDir);
    std::string payload = "this is a test22222 content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, bkpPath));
    KeyBackup::GetInstance().ListAndCheckDir(path);
    EXPECT_EQ(access(path.c_str(), 0), 0);
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(path, bkpPath), 0);

    KeyBackup::GetInstance().ListAndCheckDir(path);
    unlink(path.c_str());
    unlink(bkpPath.c_str());
    ForceRemoveDirectory(bkpDir);
    GTEST_LOG_(INFO) << "KeyBackup_ListAndCheckDir_001 end";
}

/**
 * @tc.name: KeyBackup_AddOrigFileToList_001
 * @tc.desc: Verify the AddOrigFileToList function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_AddOrigFileToList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_AddOrigFileToList_001 Start";
    std::string fileName = ".";
    std::string origDir = TEST_PATH;
    std::vector<struct FileNode> fileList;
    KeyBackup::GetInstance().AddOrigFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 0);

    fileName = "..";
    KeyBackup::GetInstance().AddOrigFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 0);

    fileName = "test.txt";
    std::string filePath = origDir + "/" + fileName;
    unlink(filePath.c_str());
    KeyBackup::GetInstance().AddOrigFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 0);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, filePath));
    KeyBackup::GetInstance().AddOrigFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 1);
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_AddOrigFileToList_001 end";
}

/**
 * @tc.name: KeyBackup_AddBackupFileToList_001
 * @tc.desc: Verify the AddBackupFileToList function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_AddBackupFileToList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_AddBackupFileToList_001 Start";
    std::string fileName = ".";
    std::string origDir = TEST_PATH;
    std::vector<struct FileNode> fileList;
    struct FileNode fl;
    fl.baseName = "test2.txt";
    fl.origFile = "";
    fl.backFile = origDir + "/" + fl.baseName;
    fl.isSame = false;
    fileList.push_back(fl);

    KeyBackup::GetInstance().AddBackupFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 1);

    fileName = "..";
    KeyBackup::GetInstance().AddBackupFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 1);

    fileName = "test.txt";
    std::string filePath = origDir + "/" + fileName;
    unlink(filePath.c_str());
    KeyBackup::GetInstance().AddBackupFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 1);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, filePath));
    KeyBackup::GetInstance().AddBackupFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 2);
    EXPECT_EQ(fileList[1].isSame, false);

    KeyBackup::GetInstance().AddBackupFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 2);
    EXPECT_EQ(fileList[1].isSame, false);

    fileList[1].origFile = filePath;
    KeyBackup::GetInstance().AddBackupFileToList(fileName, origDir, fileList);
    EXPECT_EQ(fileList.size(), 2);
    EXPECT_EQ(fileList[1].isSame, true);
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_AddBackupFileToList_001 end";
}

/**
 * @tc.name: KeyBackup_GetDiffFilesNum_001
 * @tc.desc: Verify the GetDiffFilesNum function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_GetDiffFilesNum_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_GetDiffFilesNum_001 Start";
    std::string fileName = ".";
    std::string origDir = TEST_PATH;
    std::vector<struct FileNode> fileList;
    struct FileNode fl;
    fl.baseName = "test2.txt";
    fl.origFile = "";
    fl.backFile = origDir + "/" + fl.baseName;
    fl.isSame = false;
    fileList.push_back(fl);

    fl.baseName = "test.txt";
    fl.origFile = "";
    fl.backFile = origDir + "/" + fl.baseName;
    fl.isSame = true;
    fileList.push_back(fl);

    EXPECT_EQ(KeyBackup::GetInstance().GetDiffFilesNum(fileList), 1);
    GTEST_LOG_(INFO) << "KeyBackup_GetDiffFilesNum_001 end";
}

/**
 * @tc.name: KeyBackup_CreateTempDirForMixFiles_001
 * @tc.desc: Verify the CreateTempDirForMixFiles function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CreateTempDirForMixFiles_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CreateTempDirForMixFiles_001 Start";
    std::string backupDir = TEST_PATH + "/test/test.txt";
    std::string tempDir;
    std::string eptDir = TEST_PATH + "/test/temp";

    EXPECT_NE(KeyBackup::GetInstance().CreateTempDirForMixFiles(backupDir, tempDir), 0);
    EXPECT_EQ(tempDir, eptDir);
    tempDir.clear();
    eptDir = TEST_PATH + "/temp";
    backupDir = TEST_PATH + "/test.txt";
    EXPECT_EQ(KeyBackup::GetInstance().CreateTempDirForMixFiles(backupDir, tempDir), 0);
    EXPECT_EQ(tempDir, eptDir);
    EXPECT_EQ(access(tempDir.c_str(), 0), 0);
    GTEST_LOG_(INFO) << "KeyBackup_CreateTempDirForMixFiles_001 end";
}

/**
 * @tc.name: KeyBackup_CopySameFilesToTempDir_001
 * @tc.desc: Verify the CopySameFilesToTempDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CopySameFilesToTempDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CopySameFilesToTempDir_001 Start";
    std::string backupDir = TEST_PATH + "/test/test.txt";
    std::string tempDir;
    std::vector<struct FileNode> fileList;
    struct FileNode fl;
    fl.baseName = "test2.txt";
    fl.origFile = "";
    fl.backFile = TEST_PATH + "/" + fl.baseName;
    fl.isSame = false;
    fileList.push_back(fl);
    EXPECT_EQ(KeyBackup::GetInstance().CopySameFilesToTempDir(backupDir, tempDir, fileList), -1);

    tempDir.clear();
    backupDir = TEST_PATH + "/test.txt";
    EXPECT_EQ(KeyBackup::GetInstance().CopySameFilesToTempDir(backupDir, tempDir, fileList), -1);
    EXPECT_NE(access(tempDir.c_str(), 0), 0);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, fl.backFile));
    EXPECT_EQ(KeyBackup::GetInstance().CopySameFilesToTempDir(backupDir, tempDir, fileList), 0);
    std::string tmpFile = tempDir + "/" + fl.baseName;
    EXPECT_EQ(KeyBackup::GetInstance().CompareFile(fl.backFile, tmpFile), 0);
    KeyBackup::GetInstance().RemoveNode(tempDir);
    unlink(fl.backFile.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CreateTempDirForMixFiles_001 end";
}

/**
 * @tc.name: KeyBackup_CopySameFilesToTempDir_002
 * @tc.desc: Verify the CopySameFilesToTempDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CopySameFilesToTempDir_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CopySameFilesToTempDir_002 Start";
    std::string backupDir = TEST_PATH + "/test1.txt";
    std::string tempDir;
    std::vector<struct FileNode> fileList;
    struct FileNode fl;
    fl.baseName = "test2.txt";
    std::string file1 = TEST_PATH + "/" + fl.baseName;
    fl.origFile = file1;
    fl.backFile = "";
    fl.isSame = true;
    fileList.push_back(fl);

    fl.baseName = "test1.txt";
    std::string file2 = TEST_PATH + "/" + fl.baseName;
    fl.origFile = file2;
    fl.backFile = "";
    fl.isSame = false;
    fileList.push_back(fl);

    fl.baseName = "test.txt";
    std::string file3 = TEST_PATH + "/" + fl.baseName;
    fl.origFile = file3;
    fl.backFile = file3;
    fl.isSame = false;
    fileList.push_back(fl);
    EXPECT_EQ(KeyBackup::GetInstance().CopySameFilesToTempDir(backupDir, tempDir, fileList), -1);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, file1));
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, file2));
    EXPECT_EQ(KeyBackup::GetInstance().CopySameFilesToTempDir(backupDir, tempDir, fileList), 0);
    EXPECT_EQ(fileList.size(), 1);
    KeyBackup::GetInstance().RemoveNode(tempDir);
    unlink(file2.c_str());
    unlink(file1.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CopySameFilesToTempDir_002 end";
}

/**
 * @tc.name: KeyBackup_CopyMixFilesToTempDir_001
 * @tc.desc: Verify the CopyMixFilesToTempDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_CopyMixFilesToTempDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_CopyMixFilesToTempDir_001 Start";
    uint32_t diffNum = 2;
    uint32_t num = 2;
    std::string tempDir = TEST_PATH + "/temp";
    std::vector<struct FileNode> fileList;
    struct FileNode fl;
    fl.baseName = "test2.txt";
    std::string file1 = TEST_PATH + "/" + fl.baseName;
    fl.origFile = file1;
    fl.backFile = "";
    fl.isSame = true;
    fileList.push_back(fl);

    fl.baseName = "test1.txt";
    std::string file2 = TEST_PATH + "/" + fl.baseName;
    fl.origFile = "";
    fl.backFile = file2;
    fl.isSame = false;
    fileList.push_back(fl);

    EXPECT_EQ(KeyBackup::GetInstance().CopyMixFilesToTempDir(diffNum, num, tempDir, fileList), -1);

    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, file1));
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, file2));
    ForceCreateDirectory(tempDir);
    EXPECT_EQ(KeyBackup::GetInstance().CopyMixFilesToTempDir(diffNum, num, tempDir, fileList), 0);
    KeyBackup::GetInstance().RemoveNode(tempDir);
    unlink(file2.c_str());
    unlink(file1.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_CopyMixFilesToTempDir_001 end";
}

/**
 * @tc.name: KeyBackup_GetFileList_001
 * @tc.desc: Verify the GetFileList function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(KeyBackupTest, KeyBackup_GetFileList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyBackup_GetFileList_001 Start";
    std::string origDir;
    std::string backDir;
    std::vector<struct FileNode> fileList;
    uint32_t diffNum = 0;
    EXPECT_EQ(KeyBackup::GetInstance().GetFileList(origDir, backDir, fileList, diffNum), -1);

    origDir = TEST_PATH;
    backDir = "/data/temp";
    EXPECT_EQ(KeyBackup::GetInstance().GetFileList(origDir, backDir, fileList, diffNum), -1);
    ForceCreateDirectory(backDir);

    std::string f1 = TEST_PATH + "/test1.txt";
    std::string f2 = TEST_PATH + "/test2.txt";
    std::string f3 = backDir + "/test3.txt";
    std::string f4 = backDir + "/test4.txt";
    std::string payload = "this is a test content";
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, f1));
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, f2));
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, f3));
    ASSERT_TRUE(KeyBackup::GetInstance().WriteStringToFile(payload, f4));
    EXPECT_EQ(KeyBackup::GetInstance().GetFileList(origDir, backDir, fileList, diffNum), 0);
    KeyBackup::GetInstance().RemoveNode(backDir);
    unlink(f1.c_str());
    unlink(f2.c_str());
    GTEST_LOG_(INFO) << "KeyBackup_GetFileList_001 end";
}
}
