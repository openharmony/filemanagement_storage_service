/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <fcntl.h>

#include <gtest/gtest.h>

#include "file_sharing/acl.h"
#include "utils/file_utils.h"

using namespace testing::ext;
using namespace OHOS::StorageDaemon;

namespace {
const std::string PATH_TEST = "/data/file_sharing_setacl_test";
std::string randomId = "0";

class SetAclTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SetAclTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    constexpr int randomIdUpperBound = 1000;
    std::srand(std::time(nullptr));
    randomId = std::to_string(std::rand() % randomIdUpperBound); // [0, 999]
    GTEST_LOG_(INFO) << "SetAclTest: randomId = " << randomId;
}

void SetAclTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

// input testcase setup step，setup invoked before each testcases
void SetAclTest::SetUp(void)
{
    (void)remove(PATH_TEST.c_str());
}

// input testcase teardown step，teardown invoked after each testcases
void SetAclTest::TearDown(void)
{
    (void)remove(PATH_TEST.c_str());
}

/**
 * @tc.name: SetAclTest_001
 * @tc.desc: AclSetDefault() on a regular file should fail.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(SetAclTest, SetAclTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAclTest_001 starts";

    mode_t mode = S_IRWXU | S_IRWXG | S_IXOTH; // 0771
    int fd = creat(PATH_TEST.c_str(), mode);
    ASSERT_TRUE(fd > 0) << "file creation failed";

    int rc = AclSetDefault(PATH_TEST, "not care");
    EXPECT_TRUE(rc == -1) << "path is not a dir";

    GTEST_LOG_(INFO) << "SetAclTest_001 ends";
}

/**
 * @tc.name: SetAclTest_002
 * @tc.desc: AclSetDefault() with wrong ACL should fail.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(SetAclTest, SetAclTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAclTest_002 starts";

    mode_t mode = S_IRWXU | S_IRWXG | S_IXOTH; // 0771
    int rc = MkDir(PATH_TEST, mode);
    ASSERT_TRUE(rc == 0) << "directory creation failed";

    rc = AclSetDefault(PATH_TEST, "u:rot:rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "g:rot:rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "o:rot:rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "o:root:r-x");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";
    rc = AclSetDefault(PATH_TEST, std::string("o:") + randomId + ":r-x");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "m:root:--x");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";
    rc = AclSetDefault(PATH_TEST, std::string("m:") + randomId + ":--x");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "w:root:rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";
    rc = AclSetDefault(PATH_TEST, std::string("w:") + randomId + ":rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "u:root;rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";
    rc = AclSetDefault(PATH_TEST, std::string("u:") + randomId + ";rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "u;root;rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";
    rc = AclSetDefault(PATH_TEST, std::string("u;") + randomId + ";rw");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "u:root:");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";
    rc = AclSetDefault(PATH_TEST, std::string("u:") + randomId + ":");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "u::rv-");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    rc = AclSetDefault(PATH_TEST, "g::v--");
    EXPECT_TRUE(rc == -1) << "ACL entry is wrong";

    GTEST_LOG_(INFO) << "SetAclTest_002 ends";
}

/**
 * @tc.name: SetAclTest_003
 * @tc.desc: AclSetDefault() with proper arguments should succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(SetAclTest, SetAclTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAclTest_003 starts";

    mode_t mode = S_IRWXU | S_IRWXG | S_IXOTH; // 0771
    int rc = MkDir(PATH_TEST, mode);
    ASSERT_TRUE(rc == 0) << "directory creation failed";

    rc = AclSetDefault(PATH_TEST, "u::rwx");
    EXPECT_TRUE(rc == 0) << "it should succeed";
    rc = AclSetDefault(PATH_TEST, "g::rwx");
    EXPECT_TRUE(rc == 0) << "it should succeed";

    rc = AclSetDefault(PATH_TEST, "u:root:rw");
    EXPECT_TRUE(rc == 0) << "it should succeed";
    rc = AclSetDefault(PATH_TEST, std::string("u:") + randomId + ":rw");
    EXPECT_TRUE(rc == 0) << "it should succeed";

    rc = AclSetDefault(PATH_TEST, "g:root:r-x");
    EXPECT_TRUE(rc == 0) << "it should succeed";
    rc = AclSetDefault(PATH_TEST, std::string("g:") + randomId + ":r-x");
    EXPECT_TRUE(rc == 0) << "it should succeed";

    rc = AclSetDefault(PATH_TEST, "o::-");
    EXPECT_TRUE(rc == 0) << "it should succeed";

    rc = AclSetDefault(PATH_TEST, "m::rwx");
    EXPECT_TRUE(rc == 0) << "it should succeed";

    GTEST_LOG_(INFO) << "SetAclTest_003 ends";
}
}
