/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "gtest/gtest.h"
#include "utils/redaction_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class RedactionUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: RedactionUtilsTest_MountRedactionFs_001
 * @tc.desc: Mount redaction fs.
 * @tc.type: FUNC
 * @tc.require: SR000HC2RL
 */
HWTEST_F(RedactionUtilsTest, RedactionUtilsTest_MountRedactionFs_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RedactionUtilsTest_MountRedactionFs_001 start";

    constexpr int32_t MAIN_USER_ID = 100;
    EXPECT_FALSE(RedactionUtils::SupportedRedactionFs());
    bool isMounted = RedactionUtils::CheckRedactionFsMounted(MAIN_USER_ID);
    EXPECT_TRUE(isMounted);
    if (isMounted) {
        RedactionUtils::UMountRedactionFs(MAIN_USER_ID);
        RedactionUtils::MountRedactionFs(MAIN_USER_ID);
    } else {
        RedactionUtils::MountRedactionFs(MAIN_USER_ID);
    }
    isMounted = RedactionUtils::CheckRedactionFsMounted(MAIN_USER_ID);
    EXPECT_TRUE(isMounted);
    GTEST_LOG_(INFO) << "RedactionUtilsTest_MountRedactionFs_001 end";
}
} // namespace StorageDaemon
} // namespace OHOS
