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
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include "cache_clean_controller/clean_record_store.h"
#include "storage_space_manager_errno.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class CleanRecordStoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static CleanRecordStore* store_;
    static std::string testDbDir_;
};

CleanRecordStore* CleanRecordStoreTest::store_ = nullptr;
std::string CleanRecordStoreTest::testDbDir_ = "/data/service/el1/public/database/storage_space_manager";

void CleanRecordStoreTest::SetUpTestCase()
{
    store_ = DelayedSingleton<CleanRecordStore>::GetInstance().get();
    
    constexpr int dirPermission = 0755;
    mkdir("/data/service/el1/public/database", dirPermission);
    mkdir(testDbDir_.c_str(), dirPermission);
}

void CleanRecordStoreTest::TearDownTestCase()
{
    if (store_ != nullptr) {
        store_->Close();
    }
}

void CleanRecordStoreTest::SetUp()
{
    ASSERT_NE(store_, nullptr);
    store_->Init();
}

void CleanRecordStoreTest::TearDown()
{
    if (store_ != nullptr) {
        store_->Close();
    }
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Init_0001
 * @tc.name: Init_Success
 * @tc.desc: Test initializing CleanRecordStore successfully
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Init_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Init_Success start";

    ASSERT_NE(store_, nullptr);
    
    int32_t ret = store_->Init();
    EXPECT_EQ(ret, E_OK);
    
    ret = store_->Init();
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Init_Success end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0001
 * @tc.name: Insert_ValidData
 * @tc.desc: Test inserting valid record data
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Insert_ValidData, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_ValidData start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    NativeRdb::ValuesBucket values;
    int64_t testTime = 1718515200000LL;
    values.PutLong("clean_time", testTime);
    values.PutLong("freed_size", 1024000LL);
    values.PutLong("clean_before", 2048000LL);
    values.PutLong("clean_after", 1024000LL);

    int64_t rowId = store_->Insert(values);
    EXPECT_GE(rowId, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_ValidData end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0002
 * @tc.name: Insert_MultipleRecords
 * @tc.desc: Test inserting multiple records
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Insert_MultipleRecords, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_MultipleRecords start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    for (int i = 0; i < 5; i++) {
        NativeRdb::ValuesBucket values;
        values.PutLong("clean_time", 1718515200000LL + i * 1000000);
        values.PutLong("freed_size", 1024000LL * (i + 1));
        values.PutLong("clean_before", 2048000LL * (i + 1));
        values.PutLong("clean_after", 1024000LL * (i + 1));
        
        int64_t rowId = store_->Insert(values);
        EXPECT_GE(rowId, 0);
    }

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_MultipleRecords end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0001
 * @tc.name: Get_TimeRange
 * @tc.desc: Test getting records by time range
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Get_TimeRange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_TimeRange start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    for (int i = 0; i < 3; i++) {
        NativeRdb::ValuesBucket values;
        values.PutLong("clean_time", baseTime + i * 1000000);
        values.PutLong("freed_size", 1024000LL);
        values.PutLong("clean_before", 2048000LL);
        values.PutLong("clean_after", 1024000LL);
        store_->Insert(values);
    }

    int64_t startTime = baseTime - 1000000;
    int64_t endTime = baseTime + 4000000;
    
    auto resultSet = store_->Get(startTime, endTime);
    ASSERT_NE(resultSet, nullptr);

    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_GE(rowCount, 3);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_TimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0002
 * @tc.name: Get_EmptyRange
 * @tc.desc: Test getting records with empty time range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Get_EmptyRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_EmptyRange start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t startTime = 1718515200000LL;
    int64_t endTime = 1718515200000LL + 1000;
    
    auto resultSet = store_->Get(startTime, endTime);
    ASSERT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_EmptyRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Delete_0001
 * @tc.name: Delete_OldRecords
 * @tc.desc: Test deleting old records by time
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Delete_OldRecords, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_OldRecords start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    for (int i = 0; i < 5; i++) {
        NativeRdb::ValuesBucket values;
        values.PutLong("clean_time", baseTime + i * 1000000);
        values.PutLong("freed_size", 1024000LL);
        values.PutLong("clean_before", 2048000LL);
        values.PutLong("clean_after", 1024000LL);
        store_->Insert(values);
    }

    int64_t deleteTime = baseTime + 2500000;
    int32_t deletedRows = store_->Delete(deleteTime);
    EXPECT_GE(deletedRows, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_OldRecords end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Delete_0002
 * @tc.name: Delete_NoRecordsMatch
 * @tc.desc: Test deleting when no records match
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Delete_NoRecordsMatch, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_NoRecordsMatch start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t deleteTime = 0;
    int32_t deletedRows = store_->Delete(deleteTime);
    EXPECT_EQ(deletedRows, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_NoRecordsMatch end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0001
 * @tc.name: QueryByResultString_ValidJson
 * @tc.desc: Test querying by result JSON string
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_ValidJson, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_ValidJson start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = R"({"weekly_revenue":[{"week_index":1,"revenue":1024000}]})";
    
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);

    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_GE(rowCount, 1);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_ValidJson end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Close_0001
 * @tc.name: Close_Success
 * @tc.desc: Test closing the database store
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Close_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Close_Success start";

    ASSERT_NE(store_, nullptr);
    store_->Init();
    
    store_->Close();

    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", 1718515200000LL);
    
    int64_t rowId = store_->Insert(values);
    EXPECT_NE(rowId, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Close_Success end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Close_0002
 * @tc.name: Close_MultipleTimes
 * @tc.desc: Test closing multiple times should not crash
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Close_MultipleTimes, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Close_MultipleTimes start";

    ASSERT_NE(store_, nullptr);
    store_->Init();
    
    store_->Close();
    store_->Close();
    store_->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Close_MultipleTimes end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0002
 * @tc.name: QueryByResultString_EmptyJson
 * @tc.desc: Test querying by empty JSON string
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_EmptyJson, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_EmptyJson start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = "";
    
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_EmptyJson end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0003
 * @tc.name: QueryByResultString_InvalidJson
 * @tc.desc: Test querying by invalid JSON string
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_InvalidJson, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_InvalidJson start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = "invalid_json_string";
    
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_InvalidJson end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0004
 * @tc.name: QueryByResultString_ComplexJson
 * @tc.desc: Test querying by complex JSON string with multiple records
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_ComplexJson, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_ComplexJson start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = R"({
        "weekly_revenue": [
            {"week_index": 1, "revenue": 1024000},
            {"week_index": 2, "revenue": 2048000},
            {"week_index": 3, "revenue": 3072000}
        ]
    })";
    
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);

    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_GE(rowCount, 1);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_ComplexJson end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0005
 * @tc.name: QueryByResultString_LargeJson
 * @tc.desc: Test querying by large JSON string
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_LargeJson, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_LargeJson start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = R"({"weekly_revenue":[)";
    for (int i = 0; i < 100; i++) {
        if (i > 0) {
            resultJson += ",";
        }
        resultJson += R"({"week_index":)" + std::to_string(i) +
            R"(,"revenue":)" + std::to_string(i * 1024000) + R"(})";
    }
    resultJson += R"(]})";
    
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);

    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_GE(rowCount, 1);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_LargeJson end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0006
 * @tc.name: QueryByResultString_SpecialCharacters
 * @tc.desc: Test querying by JSON string with special characters
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_SpecialCharacters, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_SpecialCharacters start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = R"({
        "weekly_revenue":[
            { "week_index":1, "revenue":1024000, "note":"Test with \"quotes\" and \\backslash" }
        ]
    })";
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_SpecialCharacters end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0003
 * @tc.name: Get_VerifyResultSet
 * @tc.desc: Test Get method returns valid result set with correct data
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Get_VerifyResultSet, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_VerifyResultSet start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    int64_t expectedFreedSize = 1024000LL;
    
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", baseTime);
    values.PutLong("freed_size", expectedFreedSize);
    values.PutLong("clean_before", 2048000LL);
    values.PutLong("clean_after", 1024000LL);
    store_->Insert(values);

    auto resultSet = store_->Get(baseTime - 1000, baseTime + 1000);
    ASSERT_NE(resultSet, nullptr);

    int32_t rowCount = -1;
    auto ret = resultSet->GetRowCount(rowCount);
    EXPECT_EQ(ret, E_OK);
    EXPECT_GE(rowCount, 1);
    
    if (rowCount > 0) {
        EXPECT_EQ(resultSet->GoToFirstRow(), E_OK);
        int32_t columnIndex = -1;
        EXPECT_EQ(resultSet->GetColumnIndex("freed_size", columnIndex), E_OK);
        
        int64_t freedSize = 0;
        EXPECT_EQ(resultSet->GetLong(columnIndex, freedSize), E_OK);
        EXPECT_EQ(freedSize, expectedFreedSize);
    }
    store_->Delete(9223372036854775807LL);
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_VerifyResultSet end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0003
 * @tc.name: Insert_VerifyDataIntegrity
 * @tc.desc: Test inserting data and verify all fields are stored correctly
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Insert_VerifyDataIntegrity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_VerifyDataIntegrity start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t testTime = 1718515200000LL;
    int64_t expectedFreedSize = 2048000LL;
    int64_t expectedCleanBefore = 4096000LL;
    int64_t expectedCleanAfter = 2048000LL;
    
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", testTime);
    values.PutLong("freed_size", expectedFreedSize);
    values.PutLong("clean_before", expectedCleanBefore);
    values.PutLong("clean_after", expectedCleanAfter);

    int64_t rowId = store_->Insert(values);
    EXPECT_GE(rowId, 0);

    auto resultSet = store_->Get(testTime - 1000, testTime + 1000);
    ASSERT_NE(resultSet, nullptr);

    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    if (rowCount > 0) {
        resultSet->GoToFirstRow();
        
        int32_t columnIndex = -1;
        resultSet->GetColumnIndex("clean_time", columnIndex);
        int64_t actualTime = 0;
        resultSet->GetLong(columnIndex, actualTime);
        EXPECT_EQ(actualTime, testTime);
        
        resultSet->GetColumnIndex("freed_size", columnIndex);
        int64_t actualFreedSize = 0;
        resultSet->GetLong(columnIndex, actualFreedSize);
        EXPECT_EQ(actualFreedSize, expectedFreedSize);
        
        resultSet->GetColumnIndex("clean_before", columnIndex);
        int64_t actualCleanBefore = 0;
        resultSet->GetLong(columnIndex, actualCleanBefore);
        EXPECT_EQ(actualCleanBefore, expectedCleanBefore);
        
        resultSet->GetColumnIndex("clean_after", columnIndex);
        int64_t actualCleanAfter = 0;
        resultSet->GetLong(columnIndex, actualCleanAfter);
        EXPECT_EQ(actualCleanAfter, expectedCleanAfter);
    }
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_VerifyDataIntegrity end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0004
 * @tc.name: Insert_NotInitialized
 * @tc.desc: Test inserting without initialization
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Insert_NotInitialized, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_NotInitialized start";

    ASSERT_NE(store_, nullptr);
    
    store_->Close();
    
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", 1718515200000LL);
    values.PutLong("freed_size", 1024000LL);
    
    int64_t rowId = store_->Insert(values);
    EXPECT_EQ(rowId, E_IO_ERROR);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_NotInitialized end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0004
 * @tc.name: Get_NotInitialized
 * @tc.desc: Test getting records without initialization
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Get_NotInitialized, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_NotInitialized start";

    ASSERT_NE(store_, nullptr);
    
    store_->Close();
    
    auto resultSet = store_->Get(1000, 2000);
    EXPECT_EQ(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_NotInitialized end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Delete_0003
 * @tc.name: Delete_NotInitialized
 * @tc.desc: Test deleting without initialization
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Delete_NotInitialized, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_NotInitialized start";

    ASSERT_NE(store_, nullptr);
    
    store_->Close();
    
    int32_t result = store_->Delete(1718515200000LL);
    EXPECT_EQ(result, E_IO_ERROR);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_NotInitialized end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0007
 * @tc.name: QueryByResultString_NotInitialized
 * @tc.desc: Test querying by result string without initialization
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_NotInitialized, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_NotInitialized start";

    ASSERT_NE(store_, nullptr);
    
    store_->Close();
    
    std::string resultJson = R"({"weekly_revenue":[{"week_index":1,"revenue":1024000}]})";
    auto resultSet = store_->QueryByResultString(resultJson);
    EXPECT_EQ(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_NotInitialized end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0005
 * @tc.name: Get_ReverseTimeRange
 * @tc.desc: Test getting records with start time > end time
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Get_ReverseTimeRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_ReverseTimeRange start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", baseTime);
    values.PutLong("freed_size", 1024000LL);
    store_->Insert(values);
    
    int64_t startTime = baseTime + 10000;
    int64_t endTime = baseTime - 10000;
    
    auto resultSet = store_->Get(startTime, endTime);
    ASSERT_NE(resultSet, nullptr);
    
    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(rowCount, 0);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_ReverseTimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Delete_0004
 * @tc.name: Delete_AllRecords
 * @tc.desc: Test deleting all records
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Delete_AllRecords, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_AllRecords start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    for (int i = 0; i < 10; i++) {
        NativeRdb::ValuesBucket values;
        values.PutLong("clean_time", baseTime + i * 1000000);
        values.PutLong("freed_size", 1024000LL);
        store_->Insert(values);
    }

    int64_t deleteTime = baseTime + 20000000;
    int32_t deletedRows = store_->Delete(deleteTime);
    EXPECT_GE(deletedRows, 10);
    
    auto resultSet = store_->Get(baseTime - 1000, baseTime + 20000000);
    ASSERT_NE(resultSet, nullptr);
    
    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(rowCount, 0);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_AllRecords end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0006
 * @tc.name: Get_NoMatchingRecords
 * @tc.desc: Test getting records when no records match the time range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Get_NoMatchingRecords, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_NoMatchingRecords start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", baseTime);
    values.PutLong("freed_size", 1024000LL);
    store_->Insert(values);
    
    int64_t startTime = baseTime + 10000000;
    int64_t endTime = baseTime + 20000000;
    
    auto resultSet = store_->Get(startTime, endTime);
    ASSERT_NE(resultSet, nullptr);
    
    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(rowCount, 0);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_NoMatchingRecords end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0005
 * @tc.name: Insert_PartialData
 * @tc.desc: Test inserting with partial data (missing some fields)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Insert_PartialData, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_PartialData start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", 1718515200000LL);
    
    int64_t rowId = store_->Insert(values);
    EXPECT_GE(rowId, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_PartialData end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Init_0002
 * @tc.name: Init_AfterClose
 * @tc.desc: Test initializing after closing
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Init_AfterClose, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Init_AfterClose start";

    ASSERT_NE(store_, nullptr);
    
    store_->Init();
    
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", 1718515200000LL);
    int64_t rowId1 = store_->Insert(values);
    EXPECT_GE(rowId1, 0);
    
    store_->Close();
    
    int32_t ret = store_->Init();
    EXPECT_EQ(ret, E_OK);
    
    values.PutLong("clean_time", 1718515300000LL);
    int64_t rowId2 = store_->Insert(values);
    EXPECT_GE(rowId2, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Init_AfterClose end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0006
 * @tc.name: Insert_LargeValues
 * @tc.desc: Test inserting with very large values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Insert_LargeValues, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_LargeValues start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", 9223372036854775807LL);
    values.PutLong("freed_size", 9223372036854775807LL);
    values.PutLong("clean_before", 9223372036854775807LL);
    values.PutLong("clean_after", 9223372036854775807LL);
    
    int64_t rowId = store_->Insert(values);
    EXPECT_GE(rowId, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_LargeValues end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0007
 * @tc.name: Get_SameStartEndTime
 * @tc.desc: Test getting records with same start and end time
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Get_SameStartEndTime, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_SameStartEndTime start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", baseTime);
    values.PutLong("freed_size", 1024000LL);
    store_->Insert(values);
    
    auto resultSet = store_->Get(baseTime, baseTime);
    ASSERT_NE(resultSet, nullptr);
    
    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(rowCount, 0);
    store_->Delete(9223372036854775807LL);
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_SameStartEndTime end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Delete_0005
 * @tc.name: Delete_BySpecificTime
 * @tc.desc: Test deleting records by specific time threshold
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordStoreTest, Delete_BySpecificTime, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_BySpecificTime start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int64_t baseTime = 1718515200000LL;
    for (int i = 0; i < 10; i++) {
        NativeRdb::ValuesBucket values;
        values.PutLong("clean_time", baseTime + i * 1000000);
        values.PutLong("freed_size", 1024000LL);
        store_->Insert(values);
    }

    int64_t thresholdTime = baseTime + 5000000;
    int32_t deletedRows = store_->Delete(thresholdTime);
    EXPECT_GE(deletedRows, 0);
    EXPECT_LE(deletedRows, 5);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_BySpecificTime end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_QueryByResultString_0008
 * @tc.name: QueryByResultString_UnicodeCharacters
 * @tc.desc: Test querying with Unicode characters in JSON
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, QueryByResultString_UnicodeCharacters, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_UnicodeCharacters start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    std::string resultJson = R"({"weekly_revenue":[{"week_index":1,"revenue":1024000,"note":"测试中文"}]})";
    
    auto resultSet = store_->QueryByResultString(resultJson);
    ASSERT_NE(resultSet, nullptr);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_QueryByResultString_UnicodeCharacters end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Insert_0007
 * @tc.name: Insert_ZeroValues
 * @tc.desc: Test inserting with zero values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Insert_ZeroValues, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_ZeroValues start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    NativeRdb::ValuesBucket values;
    values.PutLong("clean_time", 0);
    values.PutLong("freed_size", 0);
    values.PutLong("clean_before", 0);
    values.PutLong("clean_after", 0);
    
    int64_t rowId = store_->Insert(values);
    EXPECT_GE(rowId, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Insert_ZeroValues end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Get_0008
 * @tc.name: Get_NegativeTimeRange
 * @tc.desc: Test getting records with negative time values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Get_NegativeTimeRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_NegativeTimeRange start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    auto resultSet = store_->Get(-1000, -500);
    ASSERT_NE(resultSet, nullptr);
    
    int32_t rowCount = -1;
    resultSet->GetRowCount(rowCount);
    EXPECT_GE(rowCount, 0);
    
    resultSet->Close();

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Get_NegativeTimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordStore_Delete_0006
 * @tc.name: Delete_ZeroTime
 * @tc.desc: Test deleting with zero time threshold
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordStoreTest, Delete_ZeroTime, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_ZeroTime start";

    ASSERT_NE(store_, nullptr);
    store_->Init();

    int32_t deletedRows = store_->Delete(0);
    EXPECT_GE(deletedRows, 0);

    GTEST_LOG_(INFO) << "CleanRecordStoreTest_Delete_ZeroTime end";
}

} // namespace StorageSpaceManager
} // namespace OHOS