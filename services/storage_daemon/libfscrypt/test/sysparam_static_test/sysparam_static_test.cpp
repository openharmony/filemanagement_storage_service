/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "fscrypt_sysparam.h"
#include "init_param.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS::StorageDaemon {
    const int MAX_LEN = 100;

class SysparamStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SysparamStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SysparamStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SysparamStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SysparamStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SysParam_SysParamTest_001
 * @tc.desc: Verify the GetFscryptParameter.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(SysparamStaticTest, SysParam_SysParamTest_GetFscryptParameter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SysParam_SysParamTest_GetFscryptParameter_001 start";
    char value[MAX_LEN];
    unsigned int len = MAX_LEN - 1;
    std::string FSCRYPT_POLICY_KEY = "fscrypt.policy.config";
    // Test key is null
    EXPECT_EQ(GetFscryptParameter(NULL, "default", value, &len), -EINVAL);
    // Test key&value is null
    EXPECT_EQ(GetFscryptParameter(NULL, "default", NULL, &len), -EINVAL);
    // Test key&len is null
    EXPECT_EQ(GetFscryptParameter(NULL, "default", value, NULL), -EINVAL);
    // Test key&valuve&len is null
    EXPECT_EQ(GetFscryptParameter(NULL, "default", NULL, NULL), -EINVAL);
    // Test value&len is null
    EXPECT_EQ(GetFscryptParameter(FSCRYPT_POLICY_KEY.c_str(), "default", NULL, NULL), -EINVAL);
    // Test value is null
    EXPECT_EQ(GetFscryptParameter(FSCRYPT_POLICY_KEY.c_str(), "default", NULL, &len), -EINVAL);
    // Test len is null
    EXPECT_EQ(GetFscryptParameter(FSCRYPT_POLICY_KEY.c_str(), "default", value, NULL), -EINVAL);


    // Test normal case
    EXPECT_EQ(GetFscryptParameter(FSCRYPT_POLICY_KEY.c_str(), "", value, &len), 0);

    GTEST_LOG_(INFO) << "SysParam_SysParamTest_GetFscryptParameter_001 end";
}

/**
 * @tc.name: SysParam_SysParamTest_001
 * @tc.desc: Verify the SetFscryptParameter.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(SysparamStaticTest, SysParam_SysParamTest_SetFscryptParameter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SysParam_SysParamTest_SetFscryptParameter_001 start";
    // Test key is null
    EXPECT_EQ(SetFscryptParameter(NULL, "value"), -EINVAL);
    // Test key&value is null
    EXPECT_EQ(SetFscryptParameter("policy", NULL), -EINVAL);

    // Test normal case
    EXPECT_EQ(SetFscryptParameter("policy", "value"), 0);

    GTEST_LOG_(INFO) << "SysParam_SysParamTest_SetFscryptParameter_001 end";
}
} // OHOS::StorageDaemon