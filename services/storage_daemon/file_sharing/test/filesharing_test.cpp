/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>

#include <sys/xattr.h>

#include "file_sharing/acl.h"
#include "file_sharing/file_sharing.h"
#include "parameter.h"
#include "utils/file_utils.h"

using namespace testing::ext;
using namespace OHOS::StorageDaemon;

namespace {
class FileSharingTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FileSharingTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void FileSharingTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void FileSharingTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void FileSharingTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: FileSharingTest_001
 * @tc.desc: Verify that SetupFile() works as expected
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_001 starts";

    int rc = PrepareFileSharingDir(TOB_SCENE);
    EXPECT_EQ(rc, 0);

    EXPECT_TRUE(IsDir(SHARE_TOB_DIR));

    rc = SetupDirAcl(TOB_SCENE);
    EXPECT_EQ(rc, 0);
    EXPECT_TRUE(getxattr(SHARE_TOB_DIR.c_str(), ACL_XATTR_DEFAULT, nullptr, 0) > 0);

    EXPECT_TRUE(RmDirRecurse(SHARE_TOB_DIR));

    GTEST_LOG_(INFO) << "FileSharingTest_001 ends";
}

/**
 * @tc.name: FileSharingTest_002
 * @tc.desc: Verify that SetupFile() works as expected
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_002 starts";

    int rc = PrepareFileSharingDir(TOC_SCENE);
    EXPECT_EQ(rc, 0);

    EXPECT_TRUE(IsDir(PUBLIC_DIR));
    EXPECT_TRUE(IsDir(FILE_SHARING_DIR));

    rc = SetupDirAcl(TOC_SCENE);
    EXPECT_EQ(rc, 0);
    EXPECT_TRUE(getxattr(PUBLIC_DIR.c_str(), ACL_XATTR_DEFAULT, nullptr, 0) > 0);

    EXPECT_TRUE(RmDirRecurse(PUBLIC_DIR));
    EXPECT_TRUE(RmDirRecurse(FILE_SHARING_DIR));

    GTEST_LOG_(INFO) << "FileSharingTest_002 ends";
}

/**
 * @tc.name: FileSharingTest_003
 * @tc.desc: Verify that SetupFile() works as expected
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_003 starts";

    int rc = PrepareFileSharingDir(TOD_SCENE);
    EXPECT_EQ(rc, 0);

    EXPECT_TRUE(IsDir(SHARE_TOB_DIR));
    EXPECT_TRUE(IsDir(PUBLIC_DIR));
    EXPECT_TRUE(IsDir(FILE_SHARING_DIR));

    rc = SetupDirAcl(TOD_SCENE);
    EXPECT_EQ(rc, 0);
    EXPECT_TRUE(getxattr(SHARE_TOB_DIR.c_str(), ACL_XATTR_DEFAULT, nullptr, 0) > 0);
    EXPECT_TRUE(getxattr(PUBLIC_DIR.c_str(), ACL_XATTR_DEFAULT, nullptr, 0) > 0);

    EXPECT_TRUE(RmDirRecurse(SHARE_TOB_DIR));
    EXPECT_TRUE(RmDirRecurse(PUBLIC_DIR));
    EXPECT_TRUE(RmDirRecurse(FILE_SHARING_DIR));

    GTEST_LOG_(INFO) << "FileSharingTest_003 ends";
}
}