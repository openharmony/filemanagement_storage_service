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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "anco_key_manager.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace OHOS::StorageDaemon;

class AncoKeyManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};


void AncoKeyManagerTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AncoKeyManagerTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AncoKeyManagerTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AncoKeyManagerTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: SetAnDirectoryElpolicy_0100
 * @tc.desc: Verify the SetAnDirectoryElpolicy_0100 function.
 * @tc.type: FUNC
 * @tc.require: SR20231213615940
 */
HWTEST_F(AncoKeyManagerTest, Set_Anco_Directory_El_Policy_utils_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnKeyManagerTest_SetAnDirectoryElpolicy_0100 start";
    const std::string path = "";
    const std::string policyType = "";
    const uint32_t user = 0;
    EXPECT_EQ(AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(path, policyType, user),
              OHOS::E_JSON_PARSE_ERROR);
    GTEST_LOG_(INFO) << "AnKeyManagerTest_SetAnDirectoryElpolicy_0100 end";
}

/**
 * @tc.name: SetAnDirectoryElpolicy_0200
 * @tc.desc: Verify the SetAnDirectoryElpolicy_0200 function.
 * @tc.type: FUNC
 * @tc.require: SR20231213615940
 */
HWTEST_F(AncoKeyManagerTest, Set_Anco_Directory_El_Policy_utils_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnKeyManagerTest_SetAnDirectoryElpolicy_0200 start";
    const std::string path = "/data/virt_serivce/rgm_manager/rgm_homs/config/storage/test.json";
    const std::string policyType = "encryption=Require_Sys_EL1";
    const uint32_t user = 0;
    auto result = AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(path, policyType, user);
    EXPECT_NE(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "AnKeyManagerTest_SetAnDirectoryElpolicy_0200 end";
}

/**
 * @tc.name: SetAnDirectoryElpolicy_0200
 * @tc.desc: Verify the SetAnDirectoryElpolicy_0200 function.
 * @tc.type: FUNC
 * @tc.require: SR20231213615940
 */
HWTEST_F(AncoKeyManagerTest, Set_Anco_Directory_El_Policy_utils_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnKeyManagerTest_SetAnDirectoryElpolicy_0300 start";
    const std::string path = "/data/virt_serivce/rgm_manager/rgm_homs/config/storage/direnc.json";
    const std::string policyType = "encryption=Require_Sys_EL1";
    uint32_t user = 0;
    auto result = AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(path, policyType, user);
    EXPECT_NE(result, OHOS::E_OK);
    user = 100;
    result = AncoKeyManager::GetInstance()->SetAncoDirectoryElPolicy(path, policyType, user);
    EXPECT_NE(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "AnKeyManagerTest_SetAnDirectoryElpolicy_0300 end";
}

/**
 * @tc.name: ReadFileAndCreateDir_0100
 * @tc.desc: Verify the ReadFileAndCreateDir_0100 function.
 * @tc.type: FUNC
 * @tc.require: SR20231213615940
 */
HWTEST_F(AncoKeyManagerTest, Read_File_And_Create_Dir_utils_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnKeyManagerTest_ReadFileAndCreateDir_0100 start";
    const std::string path = "";
    const std::string policyType = "";
    std::vector<FileList> fileList = {};
    auto result = AncoKeyManager::GetInstance()->ReadFileAndCreateDir(path, policyType, fileList);
    EXPECT_EQ(result, OHOS::E_JSON_PARSE_ERROR);
    GTEST_LOG_(INFO) << "AnKeyManagerTest_ReadFileAndCreateDir_0100 end";
}

/**
 * @tc.name: ReadFileAndCreateDir_0200
 * @tc.desc: Verify the ReadFileAndCreateDir_0200 function.
 * @tc.type: FUNC
 * @tc.require: SR20231213615940
 */
HWTEST_F(AncoKeyManagerTest, Read_File_And_Create_Dir_utils_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnKeyManagerTest_ReadFileAndCreateDir_0200 start";
    const std::string path = "/data/virt_serivce/rgm_manager/rgm_homs/config/storage/test.json";
    const std::string policyType = "encryption=Require_Sys_EL1";
    std::vector<FileList> fileList = {};
    auto result = AncoKeyManager::GetInstance()->ReadFileAndCreateDir(path, policyType, fileList);
    EXPECT_NE(result, OHOS::E_OK);
    GTEST_LOG_(INFO) << "AnKeyManagerTest_ReadFileAndCreateDir_0200 end";
}

/**
 * @tc.name: CreatePolicyDir_0100
 * @tc.desc: Verify the CreatePolicyDir_0100 function.
 * @tc.type: FUNC
 * @tc.require: SR20231213615940
 */
HWTEST_F(AncoKeyManagerTest, Create_Policy_Dir_utils_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnKeyManagerTest_CreatePolicyDir_0100 start";
    const AncoDirInfo ancoDirInfo;
    const std::string policyType = "";
    std::vector<FileList> fileList = {};
    auto result = AncoKeyManager::GetInstance()->CreatePolicyDir(ancoDirInfo, policyType, fileList);
    EXPECT_EQ(result, OHOS::E_JSON_PARSE_ERROR);
    GTEST_LOG_(INFO) << "AnKeyManagerTest_CreatePolicyDir_0100 end";
}
} // namespace StorageDaemon
} // namespace OHOS