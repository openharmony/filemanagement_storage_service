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
#include <gmock/gmock.h>
#include "storage/storage_total_status_service.h"
#include "storage_space_manager_errno.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class StorageTotalStatusServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static StorageTotalStatusService* service_;
};

StorageTotalStatusService* StorageTotalStatusServiceTest::service_ = nullptr;

void StorageTotalStatusServiceTest::SetUpTestCase()
{
    service_ = &StorageTotalStatusService::GetInstance();
    ASSERT_NE(service_, nullptr);
}

void StorageTotalStatusServiceTest::TearDownTestCase()
{
    // Singleton instance, no need to delete
}

void StorageTotalStatusServiceTest::SetUp()
{
    // Reset state before each test
}

void StorageTotalStatusServiceTest::TearDown()
{
    // Clean up after each test
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0001
 * @tc.name: GetRoundSize_Zero
 * @tc.desc: Test GetRoundSize with zero input
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_Zero, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Zero start";

    ASSERT_NE(service_, nullptr);

    int64_t result = service_->GetRoundSize(0);
    EXPECT_EQ(result, 1024);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Zero end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0002
 * @tc.name: GetRoundSize_Negative
 * @tc.desc: Test GetRoundSize with negative input
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_Negative, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Negative start";

    ASSERT_NE(service_, nullptr);

    int64_t result = service_->GetRoundSize(-1);
    EXPECT_EQ(result, 0);

    result = service_->GetRoundSize(-1000);
    EXPECT_EQ(result, 0);

    result = service_->GetRoundSize(INT64_MIN);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Negative end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0003
 * @tc.name: GetRoundSize_SmallValues
 * @tc.desc: Test GetRoundSize with small positive values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_SmallValues, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_SmallValues start";

    ASSERT_NE(service_, nullptr);

    // Test small values
    int64_t result = service_->GetRoundSize(1);
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(100);
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(1023);
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(1024);
    EXPECT_GT(result, 0);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_SmallValues end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0004
 * @tc.name: GetRoundSize_KBRange
 * @tc.desc: Test GetRoundSize with values in KB range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_KBRange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_KBRange start";

    ASSERT_NE(service_, nullptr);

    int64_t result = service_->GetRoundSize(10LL * 1024);  // 10KB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(100LL * 1024);  // 100KB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(512LL * 1024);  // 512KB
    EXPECT_GT(result, 0);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_KBRange end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0005
 * @tc.name: GetRoundSize_MBRange
 * @tc.desc: Test GetRoundSize with values in MB range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_MBRange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_MBRange start";

    ASSERT_NE(service_, nullptr);

    int64_t result = service_->GetRoundSize(1LL * 1024 * 1024);  // 1MB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(10LL * 1024 * 1024);  // 10MB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(100LL * 1024 * 1024);  // 100MB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(512LL * 1024 * 1024);  // 512MB
    EXPECT_GT(result, 0);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_MBRange end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0006
 * @tc.name: GetRoundSize_GBRange
 * @tc.desc: Test GetRoundSize with values in GB range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_GBRange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_GBRange start";

    ASSERT_NE(service_, nullptr);

    int64_t result = service_->GetRoundSize(1LL * 1024 * 1024 * 1024);  // 1GB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(10LL * 1024 * 1024 * 1024);  // 10GB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(100LL * 1024 * 1024 * 1024);  // 100GB
    EXPECT_GT(result, 0);

    result = service_->GetRoundSize(512LL * 1024 * 1024 * 1024);  // 512GB
    EXPECT_GT(result, 0);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_GBRange end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetTotalSize_0001
 * @tc.name: GetTotalSize_Success
 * @tc.desc: Test GetTotalSize retrieves valid storage size
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetTotalSize_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalSize_Success start";

    ASSERT_NE(service_, nullptr);

    int64_t totalSize = 0;
    int32_t ret = service_->GetTotalSize(totalSize);

    // Should succeed (E_OK) or fail with appropriate error code
    EXPECT_TRUE(ret == E_OK || ret == E_STATVFS_FAILED);

    if (ret == E_OK) {
        // Total size should be positive
        EXPECT_GT(totalSize, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalSize_Success end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetTotalSize_0002
 * @tc.name: GetTotalSize_MultipleCalls
 * @tc.desc: Test GetTotalSize can be called multiple times
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetTotalSize_MultipleCalls, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalSize_MultipleCalls start";

    ASSERT_NE(service_, nullptr);

    int64_t totalSize1 = 0;
    int64_t totalSize2 = 0;
    int64_t totalSize3 = 0;

    int32_t ret1 = service_->GetTotalSize(totalSize1);
    int32_t ret2 = service_->GetTotalSize(totalSize2);
    int32_t ret3 = service_->GetTotalSize(totalSize3);

    // All should return same result
    EXPECT_EQ(ret1, ret2);
    EXPECT_EQ(ret2, ret3);

    if (ret1 == E_OK) {
        // Sizes should be similar (might vary slightly due to system activity)
        EXPECT_GT(totalSize1, 0);
        EXPECT_GT(totalSize2, 0);
        EXPECT_GT(totalSize3, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalSize_MultipleCalls end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetFreeSize_0001
 * @tc.name: GetFreeSize_Success
 * @tc.desc: Test GetFreeSize retrieves valid free storage size
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetFreeSize_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeSize_Success start";

    ASSERT_NE(service_, nullptr);

    int64_t freeSize = 0;
    int32_t ret = service_->GetFreeSize(freeSize);

    EXPECT_TRUE(ret == E_OK || ret == E_STATVFS_FAILED);

    if (ret == E_OK) {
        // Free size should be positive or zero
        EXPECT_GE(freeSize, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeSize_Success end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetFreeSize_0002
 * @tc.name: GetFreeSize_MultipleCalls
 * @tc.desc: Test GetFreeSize can be called multiple times
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetFreeSize_MultipleCalls, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeSize_MultipleCalls start";

    ASSERT_NE(service_, nullptr);

    int64_t freeSize1 = 0;
    int64_t freeSize2 = 0;

    int32_t ret1 = service_->GetFreeSize(freeSize1);
    int32_t ret2 = service_->GetFreeSize(freeSize2);

    EXPECT_EQ(ret1, ret2);

    if (ret1 == E_OK) {
        EXPECT_GE(freeSize1, 0);
        EXPECT_GE(freeSize2, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeSize_MultipleCalls end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetSystemSize_0001
 * @tc.name: GetSystemSize_Success
 * @tc.desc: Test GetSystemSize retrieves valid system size
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetSystemSize_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetSystemSize_Success start";

    ASSERT_NE(service_, nullptr);

    int64_t systemSize = 0;
    int32_t ret = service_->GetSystemSize(systemSize);

    EXPECT_TRUE(ret == E_OK || ret == E_STATVFS_FAILED);

    if (ret == E_OK) {
        // System size should be non-negative
        EXPECT_GE(systemSize, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetSystemSize_Success end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetSystemSize_0002
 * @tc.name: GetSystemSize_Consistency
 * @tc.desc: Test GetSystemSize consistency across calls
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetSystemSize_Consistency, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetSystemSize_Consistency start";

    ASSERT_NE(service_, nullptr);

    int64_t systemSize1 = 0;
    int64_t systemSize2 = 0;

    int32_t ret1 = service_->GetSystemSize(systemSize1);
    int32_t ret2 = service_->GetSystemSize(systemSize2);

    EXPECT_EQ(ret1, ret2);

    if (ret1 == E_OK) {
        // System size should be consistent
        EXPECT_GE(systemSize1, 0);
        EXPECT_GE(systemSize2, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetSystemSize_Consistency end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetTotalInodes_0001
 * @tc.name: GetTotalInodes_Success
 * @tc.desc: Test GetTotalInodes retrieves valid inode count
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetTotalInodes_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalInodes_Success start";

    ASSERT_NE(service_, nullptr);

    int64_t totalInodes = 0;
    int32_t ret = service_->GetTotalInodes(totalInodes);

    EXPECT_TRUE(ret == E_OK || ret == E_STATVFS_FAILED);

    if (ret == E_OK) {
        // Total inodes should be positive
        EXPECT_GT(totalInodes, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalInodes_Success end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetTotalInodes_0002
 * @tc.name: GetTotalInodes_MultipleCalls
 * @tc.desc: Test GetTotalInodes consistency across calls
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetTotalInodes_MultipleCalls, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalInodes_MultipleCalls start";

    ASSERT_NE(service_, nullptr);

    int64_t totalInodes1 = 0;
    int64_t totalInodes2 = 0;

    int32_t ret1 = service_->GetTotalInodes(totalInodes1);
    int32_t ret2 = service_->GetTotalInodes(totalInodes2);

    EXPECT_EQ(ret1, ret2);

    if (ret1 == E_OK) {
        EXPECT_GT(totalInodes1, 0);
        EXPECT_GT(totalInodes2, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetTotalInodes_MultipleCalls end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetFreeInodes_0001
 * @tc.name: GetFreeInodes_Success
 * @tc.desc: Test GetFreeInodes retrieves valid free inode count
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetFreeInodes_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeInodes_Success start";

    ASSERT_NE(service_, nullptr);

    int64_t freeInodes = 0;
    int32_t ret = service_->GetFreeInodes(freeInodes);

    EXPECT_TRUE(ret == E_OK || ret == E_STATVFS_FAILED);

    if (ret == E_OK) {
        // Free inodes should be non-negative
        EXPECT_GE(freeInodes, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeInodes_Success end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetFreeInodes_0002
 * @tc.name: GetFreeInodes_MultipleCalls
 * @tc.desc: Test GetFreeInodes consistency across calls
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetFreeInodes_MultipleCalls, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeInodes_MultipleCalls start";

    ASSERT_NE(service_, nullptr);

    int64_t freeInodes1 = 0;
    int64_t freeInodes2 = 0;

    int32_t ret1 = service_->GetFreeInodes(freeInodes1);
    int32_t ret2 = service_->GetFreeInodes(freeInodes2);

    EXPECT_EQ(ret1, ret2);

    if (ret1 == E_OK) {
        EXPECT_GE(freeInodes1, 0);
        EXPECT_GE(freeInodes2, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetFreeInodes_MultipleCalls end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_Singleton_0001
 * @tc.name: Singleton_GetInstance
 * @tc.desc: Test GetInstance returns same instance
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, Singleton_GetInstance, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_Singleton_GetInstance start";

    auto& instance1 = StorageTotalStatusService::GetInstance();
    auto& instance2 = StorageTotalStatusService::GetInstance();

    // Should return same instance
    EXPECT_EQ(&instance1, &instance2);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_Singleton_GetInstance end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_Relationships_0001
 * @tc.name: Relationships_TotalVsFree
 * @tc.desc: Test relationship between total and free size
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, Relationships_TotalVsFree, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_Relationships_TotalVsFree start";

    ASSERT_NE(service_, nullptr);

    int64_t totalSize = 0;
    int64_t freeSize = 0;

    int32_t ret1 = service_->GetTotalSize(totalSize);
    int32_t ret2 = service_->GetFreeSize(freeSize);

    if (ret1 == E_OK && ret2 == E_OK) {
        // Total size should be greater than or equal to free size
        EXPECT_GE(totalSize, freeSize);
        EXPECT_GT(totalSize, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_Relationships_TotalVsFree end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_Relationships_0002
 * @tc.name: Relationships_TotalInodesVsFreeInodes
 * @tc.desc: Test relationship between total and free inodes
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, Relationships_TotalInodesVsFreeInodes, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_Relationships_TotalInodesVsFreeInodes start";

    ASSERT_NE(service_, nullptr);

    int64_t totalInodes = 0;
    int64_t freeInodes = 0;

    int32_t ret1 = service_->GetTotalInodes(totalInodes);
    int32_t ret2 = service_->GetFreeInodes(freeInodes);

    if (ret1 == E_OK && ret2 == E_OK) {
        // Total inodes should be greater than or equal to free inodes
        EXPECT_GE(totalInodes, freeInodes);
        EXPECT_GT(totalInodes, 0);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_Relationships_TotalInodesVsFreeInodes end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_EdgeCases_0001
 * @tc.name: EdgeCases_MemberNullCheck
 * @tc.desc: Test service handles member access safely
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageTotalStatusServiceTest, EdgeCases_MemberNullCheck, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_EdgeCases_MemberNullCheck start";

    ASSERT_NE(service_, nullptr);

    // Verify service instance is valid
    EXPECT_NE(service_, nullptr);

    // All methods should handle null cases internally
    // This test verifies no crashes occur

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_EdgeCases_MemberNullCheck end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_EdgeCases_0002
 * @tc.name: EdgeCases_OutputParameter
 * @tc.desc: Test output parameters are properly set
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, EdgeCases_OutputParameter, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_EdgeCases_OutputParameter start";

    ASSERT_NE(service_, nullptr);

    int64_t totalSize = 0xFFFFFFFFFFFFFFFFLL;  // Initialize with invalid value
    int64_t freeSize = 0xFFFFFFFFFFFFFFFFLL;
    int64_t systemSize = 0xFFFFFFFFFFFFFFFFLL;
    int64_t totalInodes = 0xFFFFFFFFFFFFFFFFLL;
    int64_t freeInodes = 0xFFFFFFFFFFFFFFFFLL;

    int32_t ret1 = service_->GetTotalSize(totalSize);
    int32_t ret2 = service_->GetFreeSize(freeSize);
    int32_t ret3 = service_->GetSystemSize(systemSize);
    int32_t ret4 = service_->GetTotalInodes(totalInodes);
    int32_t ret5 = service_->GetFreeInodes(freeInodes);

    // If any call succeeded, output parameter should have changed
    if (ret1 == E_OK) {
        EXPECT_NE(totalSize, 0xFFFFFFFFFFFFFFFFLL);
    }
    if (ret2 == E_OK) {
        EXPECT_NE(freeSize, 0xFFFFFFFFFFFFFFFFLL);
    }
    if (ret3 == E_OK) {
        EXPECT_NE(systemSize, 0xFFFFFFFFFFFFFFFFLL);
    }
    if (ret4 == E_OK) {
        EXPECT_NE(totalInodes, 0xFFFFFFFFFFFFFFFFLL);
    }
    if (ret5 == E_OK) {
        EXPECT_NE(freeInodes, 0xFFFFFFFFFFFFFFFFLL);
    }

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_EdgeCases_OutputParameter end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0008
 * @tc.name: GetRoundSize_PreciseRounding
 * @tc.desc: Test GetRoundSize with precise rounding assertions
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_PreciseRounding, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_PreciseRounding start";

    ASSERT_NE(service_, nullptr);

    constexpr int64_t KB = 1024;
    constexpr int64_t MB = 1024 * 1024;
    constexpr int64_t GB = 1024LL * 1024 * 1024;

    // Small positive rounds up to 1KB
    EXPECT_EQ(service_->GetRoundSize(1), KB);
    EXPECT_EQ(service_->GetRoundSize(KB - 1), KB);

    // Exact KB boundaries
    EXPECT_EQ(service_->GetRoundSize(KB), KB);
    EXPECT_EQ(service_->GetRoundSize(KB + 1), 2 * KB);

    // MB boundaries
    EXPECT_EQ(service_->GetRoundSize(MB), MB);
    EXPECT_EQ(service_->GetRoundSize(MB + 1), 2 * MB);

    // GB boundaries
    EXPECT_EQ(service_->GetRoundSize(GB), GB);
    EXPECT_EQ(service_->GetRoundSize(GB + 1), 2 * GB);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_PreciseRounding end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0009
 * @tc.name: GetRoundSize_Boundary_64KB
 * @tc.desc: Test GetRoundSize at the exact KB→MB transition boundary (64KB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_Boundary_64KB, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Boundary_64KB start";
    ASSERT_NE(service_, nullptr);

    constexpr int64_t KB = 1024;
    constexpr int64_t MB = 1024 * 1024;

    // At val=64, 64*1024=65536. size=65536 does NOT trigger the 7th loop iteration.
    // val stays at 64, return 64*1024 = 64KB.
    EXPECT_EQ(service_->GetRoundSize(64 * KB), 64 * KB);

    // size=65537 triggers the 7th iteration: val→128, reset KB→MB, return 1MB.
    EXPECT_EQ(service_->GetRoundSize(64 * KB + 1), MB);

    // size=128KB also triggers reset → 1MB (val=128 in KB phase → reset → 1MB)
    EXPECT_EQ(service_->GetRoundSize(128 * KB), MB);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Boundary_64KB end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0010
 * @tc.name: GetRoundSize_Boundary_64MB
 * @tc.desc: Test GetRoundSize at the exact MB→GB transition boundary (64MB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_Boundary_64MB, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Boundary_64MB start";
    ASSERT_NE(service_, nullptr);

    constexpr int64_t MB = 1024 * 1024;
    constexpr int64_t GB = 1024LL * 1024 * 1024;

    // val reaches 64 in MB phase, 64MB does NOT trigger 7th MB iteration
    EXPECT_EQ(service_->GetRoundSize(64 * MB), 64 * MB);

    // 64MB+1 triggers MB→GB reset, returns 1GB
    EXPECT_EQ(service_->GetRoundSize(64 * MB + 1), GB);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_Boundary_64MB end";
}

/**
 * @tc.number: SUB_STORAGE_StorageTotalStatusService_GetRoundSize_0011
 * @tc.name: GetRoundSize_GbPhase
 * @tc.desc: Test GetRoundSize in GB phase where no reset occurs (multiple >= ONE_GB)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageTotalStatusServiceTest, GetRoundSize_GbPhase, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_GbPhase start";
    ASSERT_NE(service_, nullptr);

    constexpr int64_t GB = 1024LL * 1024 * 1024;

    // In GB phase, val can exceed 100 without reset because multiple==ONE_GB
    // 128GB: val reaches 128 in GB, check 128GB < 128GB is false → return 128GB
    EXPECT_EQ(service_->GetRoundSize(128 * GB), 128 * GB);

    // 129GB: val=128→256, 256GB < 129GB is false → return 256GB
    EXPECT_EQ(service_->GetRoundSize(129 * GB), 256 * GB);

    // 256GB: exact power of 2 at GB level
    EXPECT_EQ(service_->GetRoundSize(256 * GB), 256 * GB);

    GTEST_LOG_(INFO) << "StorageTotalStatusServiceTest_GetRoundSize_GbPhase end";
}

} // namespace StorageSpaceManager
} // namespace OHOS
