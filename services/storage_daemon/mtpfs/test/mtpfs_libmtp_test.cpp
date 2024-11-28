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
#include <fstream>
#include <gtest/gtest.h>
#include <fuse_opt.h>
#include "mtpfs_libmtp.h"
#include <libmtp.h>
#include <cstdlib>
#include <unistd.h>
#include "mtpfs_util.h"
#include "storage_service_log.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

class MtpfsLibmtpTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.name: Mtpfs_LIBMTPFreeFilesAndFolders_001
 * @tc.desc: Verify the LIBMTPFreeFilesAndFolders function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsLibmtpTest, MtpfsLibmtpTest_LIBMTPFreeFilesAndFolders_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsLibmtpTest_LIBMTPFreeFilesAndFolders_001 start";
    LIBMTP_file_t** file = nullptr;
    LIBMTPFreeFilesAndFolders(file);
    EXPECT_EQ(file, nullptr);
    GTEST_LOG_(INFO) << "MtpfsLibmtpTest_LIBMTPFreeFilesAndFolders_001 end";
}