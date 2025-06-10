/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <openssl/sha.h>
#include <securec.h>
#include <sstream>
#include <libmtp.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <gtest/gtest.h>
#include <fuse_opt.h>
#include <unistd.h>
#include "mtpfs_tmp_files_pool.h"
#include "mtpfs_util.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace testing::ext;
using namespace testing;

class MtpfsTmpFilesPoolTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.name: Mtpfs_CreateTmpDir_001
 * @tc.desc: Verify the CreateTmpDir function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsTmpFilesPoolTest, MtpfsTmpFilesPoolTest_CreateTmpDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsTmpFilesPoolTest_CreateTmpDir_001 start";
    auto mtptmpfilespool = std::make_shared<MtpFsTmpFilesPool>();
    mtptmpfilespool->tmpDir_ = "";
    bool result = mtptmpfilespool->CreateTmpDir();
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "MtpfsTmpFilesPoolTest_CreateTmpDir_001 end";
}

/**
 * @tc.name: Mtpfs_RemoveTmpDir_001
 * @tc.desc: Verify the RemoveTmpDir function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsTmpFilesPoolTest, MtpfsTmpFilesPoolTest_RemoveTmpDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsTmpFilesPoolTest_RemoveTmpDir_001 start";
    auto mtptmpfilespool = std::make_shared<MtpFsTmpFilesPool>();
    mtptmpfilespool->tmpDir_ = "";
    bool result = mtptmpfilespool->RemoveTmpDir();
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "MtpfsTmpFilesPoolTest_RemoveTmpDir_001 end";
}

/**
 * @tc.name: MtpfsTmpFilesPoolTest_AddFileTest_001
 * @tc.desc: 测试 AddFile 函数在调用时能够正确将文件插入到集合中
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsTmpFilesPoolTest, MtpfsTmpFilesPoolTest_AddFileTest_001, TestSize.Level1) {
    std::string path = "/path/to/file";
    std::string tmpPath = "/path/to/tmpPath";
    MtpFsTypeTmpFile tmpFile = MtpFsTypeTmpFile(std::string(path), tmpPath, 1);

    MtpFsTmpFilesPool tmpFilesPool;
    tmpFilesPool.AddFile(tmpFile);

    auto found = std::find(tmpFilesPool.tmpFilePool_.begin(), tmpFilesPool.tmpFilePool_.end(), tmpFile);
    EXPECT_NE(found, tmpFilesPool.tmpFilePool_.end());
    EXPECT_EQ(found->PathTmp(), "/path/to/tmpPath");
}

/**
 * @tc.name: MtpfsTmpFilesPoolTest_RemoveFileTest_001
 * @tc.desc: 测试当文件路径存在时,RemoveFile 函数能够成功移除该路径
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsTmpFilesPoolTest, MtpfsTmpFilesPoolTest_RemoveFileTest_001, TestSize.Level1) {
    std::string path1 = "/path/to/file1";
    std::string tmpPath1 = "/path/to/tmpPath1";
    MtpFsTypeTmpFile tmpFile1 = MtpFsTypeTmpFile(std::string(path1), tmpPath1, 1);

    std::string path2 = "/path/to/file2";
    std::string tmpPath2 = "/path/to/tmpPath2";
    MtpFsTypeTmpFile tmpFile2 = MtpFsTypeTmpFile(std::string(path2), tmpPath2, 2);

    MtpFsTmpFilesPool tmpPool;
    tmpPool.AddFile(tmpFile1);
    tmpPool.AddFile(tmpFile2);

    std::string remove = "/path/to/file1";
    tmpPool.RemoveFile(remove);
    EXPECT_EQ(std::find(tmpPool.tmpFilePool_.begin(), tmpPool.tmpFilePool_.end(), remove), tmpPool.tmpFilePool_.end());
}
}
}