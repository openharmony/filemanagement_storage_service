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
    auto mtptmpfilespool = std::make_shared<MtpfsTmpFilesPool>();
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
    auto mtptmpfilespool = std::make_shared<MtpfsTmpFilesPool>();
    mtptmpfilespool->tmpDir_ = "";
    bool result = mtptmpfilespool->RemoveTmpDir();
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "MtpfsTmpFilesPoolTest_RemoveTmpDir_001 end";
}
}
}