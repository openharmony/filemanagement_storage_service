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

#include "init_param.h"
#include "fscrypt_sysparam.h"
#include "storage_radar_c.h"
#include "hisysevent_c.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS::StorageDaemon {
class StorageRadarTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StorageRadarTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void StorageRadarTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void StorageRadarTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StorageRadarTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SysParam_ReportSetPolicyResult_001
 * @tc.desc: Verify the ReportSetPolicyResult.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(StorageRadarTest, SysParam_ReportSetPolicyResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SysParam_ReportSetPolicyResult_001 start";
    const char *pathNull = nullptr;
    const char *reasonNull = nullptr;
    const char *funcNull = nullptr;
    const char *path = "/data/service/el1";
    const char *reason = "strdup_err";
    const char *func = "setPolicy";

    ReportSetPolicyResult(pathNull, reason, 10, func, 10);
    ASSERT_TRUE(pathNull == nullptr);

    ReportSetPolicyResult(path, reasonNull, 10, func, 10);
    ASSERT_TRUE(reasonNull == nullptr);

    ReportSetPolicyResult(path, reason, 10, funcNull, 10);
    ASSERT_TRUE(funcNull == nullptr);

    ReportSetPolicyResult(path, reason, 10, func, 10);
    EXPECT_EQ(func, "setPolicy");
    GTEST_LOG_(INFO) << "SysParam_ReportSetPolicyResult_001 end";
}
} // OHOS::StorageDaemon
