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

/**
 * @tc.number: Storage_Utils_GetRoundSize_test_0001
 * @tc.name: Storage_Utils_GetRoundSize_test_0001
 * @tc.desc: Test GetRoundSize with KB/MB range values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(StorageUtilsTest, Storage_Utils_GetRoundSize_test_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0001 begin";
    constexpr int64_t kb = 1000;
    constexpr int64_t mb = 1000000;
    constexpr int64_t gb = 1000000000;

    // Small values round up to 1KB
    EXPECT_EQ(GetRoundSize(1), kb);
    EXPECT_EQ(GetRoundSize(kb - 1), kb);
    EXPECT_EQ(GetRoundSize(kb), kb);

    // KB to MB transition
    EXPECT_EQ(GetRoundSize(kb + 1), 2 * kb);
    EXPECT_EQ(GetRoundSize(mb), mb);
    EXPECT_EQ(GetRoundSize(mb + 1), 2 * mb);

    // MB to GB transition
    EXPECT_EQ(GetRoundSize(gb), gb);
    EXPECT_EQ(GetRoundSize(gb + 1), 2 * gb);
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0001 end";
}

/**
 * @tc.number: Storage_Utils_GetRoundSize_test_0002
 * @tc.name: Storage_Utils_GetRoundSize_test_0002
 * @tc.desc: Test GetRoundSize with device specs - 256GB/512GB
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(StorageUtilsTest, Storage_Utils_GetRoundSize_test_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0002 begin";
    constexpr int64_t gb = 1000000000;

    // 256GB device: rounds to 256 GB
    int64_t result = GetRoundSize(256LL * gb);
    EXPECT_EQ(result, 256LL * gb);

    // 512GB device: rounds to 512 GB
    result = GetRoundSize(512LL * gb);
    EXPECT_EQ(result, 512LL * gb);

    // 256GB + 1 → 512 GB (rounds up to next power of 2 in GB tier)
    result = GetRoundSize(256LL * gb + 1);
    EXPECT_EQ(result, 512LL * gb);
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0002 end";
}

/**
 * @tc.number: Storage_Utils_GetRoundSize_test_0003
 * @tc.name: Storage_Utils_GetRoundSize_test_0003
 * @tc.desc: Test GetRoundSize at 1024GB boundary
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(StorageUtilsTest, Storage_Utils_GetRoundSize_test_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0003 begin";
    constexpr int64_t gb = 1000000000;

    // 1024GB: val=1024 in GB tier, 1024<=1023 NO → returns 1024 GB
    int64_t result = GetRoundSize(1024LL * gb);
    EXPECT_EQ(result, 1024LL * gb);

    // 1024GB + 1: val doubles to 2048 in GB tier → returns 2048 GB
    result = GetRoundSize(1024LL * gb + 1);
    EXPECT_EQ(result, 2048LL * gb);
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0003 end";
}

/**
 * @tc.number: Storage_Utils_GetRoundSize_test_0004
 * @tc.name: Storage_Utils_GetRoundSize_test_0004
 * @tc.desc: Test GetRoundSize with large sizes in GB tier (no >1TB branch)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 1
 */
HWTEST_F(StorageUtilsTest, Storage_Utils_GetRoundSize_test_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0004 begin";
    constexpr int64_t gb = 1000000000;

    // GetRoundSize is a pure rounding function, has no >1TB branch
    // In GB tier (multiple=ONE_GB), val continues doubling

    // 1.5TB: (1500GB-1)/1000MB... Phase3: val→2048, 2048*GB >= 1500GB → return 2048GB
    int64_t result = GetRoundSize(1500LL * gb);
    EXPECT_EQ(result, 2048LL * gb);

    // 2TB: (2000GB) → val→2048, return 2048GB
    result = GetRoundSize(2000LL * gb);
    EXPECT_EQ(result, 2048LL * gb);

    // 3TB: (3000GB) → val→4096, return 4096GB
    result = GetRoundSize(3000LL * gb);
    EXPECT_EQ(result, 4096LL * gb);

    // 4TB: (4000GB) → val→4096, return 4096GB
    result = GetRoundSize(4000LL * gb);
    EXPECT_EQ(result, 4096LL * gb);
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0004 end";
}

/**
 * @tc.number: Storage_Utils_GetRoundSize_test_0006
 * @tc.name: Storage_Utils_GetRoundSize_test_0006
 * @tc.desc: Test GetRoundSize monotonicity - larger input should produce >= output
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 2
 */
HWTEST_F(StorageUtilsTest, Storage_Utils_GetRoundSize_test_0006, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0006 begin";
    constexpr int64_t gb = 1000000000;

    // Monotonicity across tier boundaries
    EXPECT_LE(GetRoundSize(511LL * gb), GetRoundSize(512LL * gb));
    EXPECT_LE(GetRoundSize(512LL * gb), GetRoundSize(513LL * gb));
    EXPECT_LE(GetRoundSize(1023LL * gb), GetRoundSize(1024LL * gb));
    EXPECT_LE(GetRoundSize(1024LL * gb), GetRoundSize(1025LL * gb));
    GTEST_LOG_(INFO) << "Storage_Utils_GetRoundSize_test_0006 end";
}
} // namespace StorageManager
} // namespace OHOS
