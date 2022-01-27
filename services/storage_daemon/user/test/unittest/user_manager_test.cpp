/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "user/user_manager.h"
#include "utils/errno.h"
#include "utils/log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

namespace {
    constexpr int32_t USER_ID1 = 100;
    constexpr int32_t USER_ID2 = 200;
}

class UserManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
};

void UserManagerTest::SetUp()
{
    GTEST_LOG_(INFO) << "UserManagerTest SetUp";
}

void UserManagerTest::TearDown()
{
    GTEST_LOG_(INFO) << "UserManagerTest TearDown";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_Instance_001
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_Instance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::Instance();
    EXPECT_NE(userManager, nullptr);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_Instance_002
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_Instance_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_002 start";

    std::shared_ptr<UserManager> userManagerFirst = UserManager::Instance();
    EXPECT_NE(userManagerFirst, nullptr);
    std::shared_ptr<UserManager> userManagerSecond = UserManager::Instance();
    EXPECT_NE(userManagerSecond, nullptr);

    EXPECT_EQ(userManagerFirst, userManagerSecond);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_Instance_002 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_AddUser_001
 * @tc.desc: Verify the AddUser function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_AddUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_AddUser_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::Instance();
    EXPECT_NE(userManager, nullptr);
    int32_t ret = userManager->AddUser(USER_ID1);
    EXPECT_EQ(E_OK, ret);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_AddUser_001 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_AddUser_002
 * @tc.desc: Verify the AddUser function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_AddUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_AddUser_002 start";

    std::shared_ptr<UserManager> userManager = UserManager::Instance();
    EXPECT_NE(userManager, nullptr);
    int32_t ret = userManager->AddUser(USER_ID1);
    EXPECT_NE(E_OK, ret);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_AddUser_002 end";
}

/**
 * @tc.name: Storage_Manager_UserManagerTest_RemoveUser_001
 * @tc.desc: Verify the RemoveUser function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(UserManagerTest, Storage_Manager_UserManagerTest_RemoveUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_RemoveUser_001 start";

    std::shared_ptr<UserManager> userManager = UserManager::Instance();
    EXPECT_NE(userManager, nullptr);

    userManager->AddUser(USER_ID2);
    int32_t ret = userManager->RemoveUser(USER_ID2);
    EXPECT_EQ(E_OK, ret);

    GTEST_LOG_(INFO) << "Storage_Manager_UserManagerTest_RemoveUser_001 end";
}

} // STORAGE_DAEMON
} // OHOS