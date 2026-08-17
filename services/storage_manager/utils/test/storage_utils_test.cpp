/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <string>

#include "utils/storage_utils.h"

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;

class StorageUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_IsFilePathInvalid_ValidPath
 * @tc.name: IsFilePathInvalid_ValidPath
 * @tc.desc: Test valid path returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsFilePathInvalid_ValidPath, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFilePathInvalid_ValidPath start";

    EXPECT_FALSE(IsFilePathInvalid("/mnt/data/100/userExternal/sub"));

    GTEST_LOG_(INFO) << "IsFilePathInvalid_ValidPath end";
}

/**
 * @tc.number: SUB_STORAGE_IsFilePathInvalid_RelativeTraversal
 * @tc.name: IsFilePathInvalid_RelativeTraversal
 * @tc.desc: Test path with ../ returns true.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsFilePathInvalid_RelativeTraversal, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFilePathInvalid_RelativeTraversal start";

    EXPECT_TRUE(IsFilePathInvalid("/mnt/data/../etc"));

    GTEST_LOG_(INFO) << "IsFilePathInvalid_RelativeTraversal end";
}

/**
 * @tc.number: SUB_STORAGE_IsFilePathInvalid_TrailingDotDot
 * @tc.name: IsFilePathInvalid_TrailingDotDot
 * @tc.desc: Test path ending with /.. returns true.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsFilePathInvalid_TrailingDotDot, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFilePathInvalid_TrailingDotDot start";

    EXPECT_TRUE(IsFilePathInvalid("/mnt/data/sub/.."));

    GTEST_LOG_(INFO) << "IsFilePathInvalid_TrailingDotDot end";
}

/**
 * @tc.number: SUB_STORAGE_IsPathStartWithFileMgr_ValidPrefix
 * @tc.name: IsPathStartWithFileMgr_ValidPrefix
 * @tc.desc: Test path with valid FileMgr prefix returns true.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsPathStartWithFileMgr_ValidPrefix, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_ValidPrefix start";

    EXPECT_TRUE(IsPathStartWithFileMgr(100, "/mnt/data/100/userExternal/sub"));

    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_ValidPrefix end";
}

/**
 * @tc.number: SUB_STORAGE_IsPathStartWithFileMgr_TooShort
 * @tc.name: IsPathStartWithFileMgr_TooShort
 * @tc.desc: Test too short path returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsPathStartWithFileMgr_TooShort, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_TooShort start";

    EXPECT_FALSE(IsPathStartWithFileMgr(100, "/mnt/data/"));

    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_TooShort end";
}

/**
 * @tc.number: SUB_STORAGE_IsPathStartWithFileMgr_Mismatch
 * @tc.name: IsPathStartWithFileMgr_Mismatch
 * @tc.desc: Test path with mismatched prefix returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsPathStartWithFileMgr_Mismatch, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_Mismatch start";

    EXPECT_FALSE(IsPathStartWithFileMgr(100, "/data/local/evil"));

    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_Mismatch end";
}

/**
 * @tc.number: SUB_STORAGE_IsPathStartWithFileMgr_PrefixMismatchLong
 * @tc.name: IsPathStartWithFileMgr_PrefixMismatchLong
 * @tc.desc: Test path long enough but with mismatched prefix returns false (covers if2 true).
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsPathStartWithFileMgr_PrefixMismatchLong, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_PrefixMismatchLong start";

    EXPECT_FALSE(IsPathStartWithFileMgr(100, "/mnt/data/100/userInternal/sub"));

    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_PrefixMismatchLong end";
}

/**
 * @tc.number: SUB_STORAGE_IsPathStartWithFileMgr_Empty
 * @tc.name: IsPathStartWithFileMgr_Empty
 * @tc.desc: Test empty path returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsPathStartWithFileMgr_Empty, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_Empty start";

    EXPECT_FALSE(IsPathStartWithFileMgr(100, ""));

    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_Empty end";
}

/**
 * @tc.number: SUB_STORAGE_IsPathStartWithFileMgr_EqualPrefixLen
 * @tc.name: IsPathStartWithFileMgr_EqualPrefixLen
 * @tc.desc: Test path whose length equals prefix length returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H0372
 */
HWTEST_F(StorageUtilsTest, IsPathStartWithFileMgr_EqualPrefixLen, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_EqualPrefixLen start";

    EXPECT_FALSE(IsPathStartWithFileMgr(100, "/mnt/data/100/userExternal/"));

    GTEST_LOG_(INFO) << "IsPathStartWithFileMgr_EqualPrefixLen end";
}
} // namespace StorageManager
} // namespace OHOS
