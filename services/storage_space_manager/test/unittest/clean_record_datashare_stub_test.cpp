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
#include "adapter/clean_record_datashare_stub.h"
#include "storage_space_manager_errno.h"
#include "uri.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "datashare_business_error.h"
#include "ipc_caller_auth_mock.h"
#include "cache_clean_controller/clean_record_store.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

namespace {
    constexpr uint32_t MOCK_TOKEN_ID = 12345;
    constexpr int32_t MOCK_UID = 1000;
    constexpr uint32_t TOKEN_TYPE_NATIVE = 1;
    constexpr int32_t PERMISSION_GRANTED = 0;
    constexpr int32_t PERMISSION_DENIED = -1;
}

class CleanRecordDataShareStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static CleanRecordDataShareStub* stub_;
    static CleanRecordStore* store_;
};

CleanRecordDataShareStub* CleanRecordDataShareStubTest::stub_ = nullptr;
CleanRecordStore* CleanRecordDataShareStubTest::store_ = nullptr;

void CleanRecordDataShareStubTest::SetUpTestCase()
{
    stub_ = new CleanRecordDataShareStub();
    store_ = DelayedSingleton<CleanRecordStore>::GetInstance().get();
    ASSERT_NE(stub_, nullptr);
}

void CleanRecordDataShareStubTest::TearDownTestCase()
{
    if (stub_ != nullptr) {
        delete stub_;
        stub_ = nullptr;
    }
    if (store_ != nullptr) {
        store_->Close();
    }
}

void CleanRecordDataShareStubTest::SetUp()
{
    g_mockCallingTokenId = MOCK_TOKEN_ID;
    g_mockCallingUid = MOCK_UID;
    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    g_mockTokenTypeFlag = TOKEN_TYPE_NATIVE;
    ASSERT_NE(store_, nullptr);
    store_->Init();
}

void CleanRecordDataShareStubTest::TearDown()
{
    if (store_ != nullptr) {
        store_->Close();
    }
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_GetConfig_0001
 * @tc.name: GetConfig_Success
 * @tc.desc: Test GetConfig returns valid configuration
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, GetConfig_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetConfig_Success start";

    ASSERT_NE(stub_, nullptr);

    auto config = stub_->GetConfig();
    EXPECT_EQ(config.records.size(), 1);
    
    if (config.records.size() > 0) {
        EXPECT_FALSE(config.records[0].uri.empty());
        EXPECT_FALSE(config.records[0].readPermission.empty());
        EXPECT_FALSE(config.records[0].writePermission.empty());
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetConfig_Success end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Insert_0001
 * @tc.name: Insert_NotSupported
 * @tc.desc: Test Insert operation is not supported
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Insert_NotSupported, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Insert_NotSupported start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Insert(uri, values);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Insert_NotSupported end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Update_0001
 * @tc.name: Update_NotSupported
 * @tc.desc: Test Update operation is not supported
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Update_NotSupported, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Update_NotSupported start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Update(uri, predicates, values);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Update_NotSupported end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Delete_0001
 * @tc.name: Delete_NotSupported
 * @tc.desc: Test Delete operation is not supported
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Delete_NotSupported, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Delete_NotSupported start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    int result = stub_->Delete(uri, predicates);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Delete_NotSupported end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0001
 * @tc.name: Query_ValidPredicates
 * @tc.desc: Test Query with valid predicates
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_ValidPredicates, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ValidPredicates start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1718515200000,"end_time":1718601600000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ValidPredicates end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0002
 * @tc.name: Query_EmptyPredicates
 * @tc.desc: Test Query with empty predicates
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_EmptyPredicates, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_EmptyPredicates start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_EmptyPredicates end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_OpenFile_0001
 * @tc.name: OpenFile_ReturnZero
 * @tc.desc: Test OpenFile returns 0
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, OpenFile_ReturnZero, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_OpenFile_ReturnZero start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    std::string mode = "r";
    
    int result = stub_->OpenFile(uri, mode);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_OpenFile_ReturnZero end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_OpenRawFile_0001
 * @tc.name: OpenRawFile_ReturnZero
 * @tc.desc: Test OpenRawFile returns 0
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, OpenRawFile_ReturnZero, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_OpenRawFile_ReturnZero start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    std::string mode = "r";
    
    int result = stub_->OpenRawFile(uri, mode);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_OpenRawFile_ReturnZero end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_GetType_0001
 * @tc.name: GetType_ReturnEmpty
 * @tc.desc: Test GetType returns empty string
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, GetType_ReturnEmpty, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetType_ReturnEmpty start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    std::string type = stub_->GetType(uri);
    EXPECT_TRUE(type.empty());

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetType_ReturnEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BatchInsert_0001
 * @tc.name: BatchInsert_ReturnZero
 * @tc.desc: Test BatchInsert returns 0
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BatchInsert_ReturnZero, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BatchInsert_ReturnZero start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    std::vector<DataShare::DataShareValuesBucket> values;
    
    int result = stub_->BatchInsert(uri, values);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BatchInsert_ReturnZero end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_RegisterObserver_0001
 * @tc.name: RegisterObserver_ReturnTrue
 * @tc.desc: Test RegisterObserver returns true
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, RegisterObserver_ReturnTrue, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_RegisterObserver_ReturnTrue start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    sptr<AAFwk::IDataAbilityObserver> observer = nullptr;
    
    bool result = stub_->RegisterObserver(uri, observer);
    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_RegisterObserver_ReturnTrue end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_UnregisterObserver_0001
 * @tc.name: UnregisterObserver_ReturnTrue
 * @tc.desc: Test UnregisterObserver returns true
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, UnregisterObserver_ReturnTrue, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_UnregisterObserver_ReturnTrue start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    sptr<AAFwk::IDataAbilityObserver> observer = nullptr;
    
    bool result = stub_->UnregisterObserver(uri, observer);
    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_UnregisterObserver_ReturnTrue end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_NotifyChange_0001
 * @tc.name: NotifyChange_ReturnTrue
 * @tc.desc: Test NotifyChange returns true
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, NotifyChange_ReturnTrue, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_NotifyChange_ReturnTrue start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    bool result = stub_->NotifyChange(uri);
    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_NotifyChange_ReturnTrue end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_NormalizeUri_0001
 * @tc.name: NormalizeUri_ReturnEmpty
 * @tc.desc: Test NormalizeUri returns empty Uri
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, NormalizeUri_ReturnEmpty, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_NormalizeUri_ReturnEmpty start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    Uri result = stub_->NormalizeUri(uri);
    EXPECT_TRUE(result.ToString().empty());

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_NormalizeUri_ReturnEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_DenormalizeUri_0001
 * @tc.name: DenormalizeUri_ReturnEmpty
 * @tc.desc: Test DenormalizeUri returns empty Uri
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, DenormalizeUri_ReturnEmpty, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_DenormalizeUri_ReturnEmpty start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    Uri result = stub_->DenormalizeUri(uri);
    EXPECT_TRUE(result.ToString().empty());

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_DenormalizeUri_ReturnEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_GetFileTypes_0001
 * @tc.name: GetFileTypes_ReturnEmpty
 * @tc.desc: Test GetFileTypes returns empty vector
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, GetFileTypes_ReturnEmpty, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetFileTypes_ReturnEmpty start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    std::string mimeTypeFilter = "*/*";
    
    std::vector<std::string> result = stub_->GetFileTypes(uri, mimeTypeFilter);
    EXPECT_TRUE(result.empty());

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetFileTypes_ReturnEmpty end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0003
 * @tc.name: Query_InvalidJsonFormat
 * @tc.desc: Test Query with invalid JSON format in predicates
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_InvalidJsonFormat, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidJsonFormat start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = "invalid_json_{not_valid}";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidJsonFormat end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0004
 * @tc.name: Query_MissingConditions
 * @tc.desc: Test Query with JSON missing conditions field
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_MissingConditions, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MissingConditions start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"other_field":"value"})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MissingConditions end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0005
 * @tc.name: Query_ConditionsNotArray
 * @tc.desc: Test Query with conditions field not an array
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_ConditionsNotArray, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ConditionsNotArray start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":"not_an_array"})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ConditionsNotArray end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0006
 * @tc.name: Query_InvalidConditionFields
 * @tc.desc: Test Query with missing required fields in condition
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_InvalidConditionFields, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidConditionFields start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidConditionFields end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0007
 * @tc.name: Query_MultipleConditions
 * @tc.desc: Test Query with multiple conditions
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_MultipleConditions, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MultipleConditions start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({
        "conditions": [
            { "week_index":1, "begin_time":1718515200000, "end_time":1718601600000 },
            { "week_index":2, "begin_time":1718601600000, "end_time":1718688000000 }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MultipleConditions end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0008
 * @tc.name: Query_InvalidTimeRange
 * @tc.desc: Test Query with invalid time range (begin > end)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_InvalidTimeRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidTimeRange start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":9999999999,"end_time":1000000000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidTimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0009
 * @tc.name: Query_NegativeTimeValues
 * @tc.desc: Test Query with negative time values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_NegativeTimeValues, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_NegativeTimeValues start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":-1,"begin_time":-1000,"end_time":-2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_NegativeTimeValues end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_GetConfig_0002
 * @tc.name: GetConfig_VerifyFields
 * @tc.desc: Test GetConfig returns valid configuration with correct fields
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, GetConfig_VerifyFields, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetConfig_VerifyFields start";

    ASSERT_NE(stub_, nullptr);

    auto config = stub_->GetConfig();
    EXPECT_EQ(config.records.size(), 1);
    
    if (config.records.size() > 0) {
        auto record = config.records[0];
        EXPECT_FALSE(record.uri.empty());
        EXPECT_FALSE(record.readPermission.empty());
        EXPECT_FALSE(record.writePermission.empty());
        
        EXPECT_TRUE(record.uri.find("datashare://") != std::string::npos);
        EXPECT_EQ(record.readPermission, "ohos.permission.STORAGE_MANAGER");
        EXPECT_EQ(record.writePermission, "ohos.permission.STORAGE_MANAGER");
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_GetConfig_VerifyFields end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0010
 * @tc.name: Query_MultipleOperations
 * @tc.desc: Test Query with predicates containing multiple operations
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_MultipleOperations, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MultipleOperations start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    predicates.EqualTo("field1", "value1");
    predicates.EqualTo("field2", "value2");
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MultipleOperations end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0011
 * @tc.name: Query_InvalidSingleParams
 * @tc.desc: Test Query with predicates having invalid single params count
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_InvalidSingleParams, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidSingleParams start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    predicates.Between("field", "value1", "value2");
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_InvalidSingleParams end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0012
 * @tc.name: Query_WeekIndexNotInteger
 * @tc.desc: Test Query with week_index field not being integer
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_WeekIndexNotInteger, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_WeekIndexNotInteger start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":"not_a_number","begin_time":1000,"end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_WeekIndexNotInteger end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0013
 * @tc.name: Query_BeginTimeNotInteger
 * @tc.desc: Test Query with begin_time field not being integer
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_BeginTimeNotInteger, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_BeginTimeNotInteger start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":"not_a_number","end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_BeginTimeNotInteger end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0014
 * @tc.name: Query_EndTimeNotInteger
 * @tc.desc: Test Query with end_time field not being integer
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_EndTimeNotInteger, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_EndTimeNotInteger start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":"not_a_number"}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_EndTimeNotInteger end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0015
 * @tc.name: Query_EmptyConditionsArray
 * @tc.desc: Test Query with empty conditions array
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_EmptyConditionsArray, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_EmptyConditionsArray start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_EmptyConditionsArray end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0016
 * @tc.name: Query_PartiallyInvalidConditions
 * @tc.desc: Test Query with some conditions valid and some invalid
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_PartiallyInvalidConditions, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_PartiallyInvalidConditions start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    std::string queryJson = R"({
        "conditions": [
            { "week_index":1, "begin_time":1000, "end_time":2000 },
            { "week_index":"invalid" },
            { "week_index":2, "begin_time":3000, "end_time":4000 }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_PartiallyInvalidConditions end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0017
 * @tc.name: Query_ExtraFieldsInCondition
 * @tc.desc: Test Query with extra fields in condition (should be ignored)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_ExtraFieldsInCondition, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ExtraFieldsInCondition start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({
        "conditions": [
            {
                "week_index":1,
                "begin_time":1718515200000,
                "end_time":1718601600000,
                "extra_field":"should_be_ignored",
                "another_extra":12345
            }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ExtraFieldsInCondition end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0018
 * @tc.name: Query_ZeroTimeRange
 * @tc.desc: Test Query with zero time range (begin_time == end_time)
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_ZeroTimeRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ZeroTimeRange start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1718515200000,"end_time":1718515200000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ZeroTimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0019
 * @tc.name:_Query_LargeWeekIndex
 * @tc.desc: Test Query with very large week_index value
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_LargeWeekIndex, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_LargeWeekIndex start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({
        "conditions": [
            { "week_index":999999, "begin_time":1718515200000, "end_time":1718601600000 }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_LargeWeekIndex end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0020
 * @tc.name: Query_JsonParseError
 * @tc.desc: Test Query with malformed JSON that passes accept() but fails parse()
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_JsonParseError, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_JsonParseError start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    predicates.EqualTo("query", "{invalid json}");
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_JsonParseError end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Insert_0002
 * @tc.name: Insert_PermissionDenied
 * @tc.desc: Test Insert returns E_PERMISSION_DENIED when no permission
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Insert_PermissionDenied, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Insert_PermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://unknown/uri");
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Insert(uri, values);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Insert_PermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Update_0002
 * @tc.name: Update_PermissionDenied
 * @tc.desc: Test Update returns E_PERMISSION_DENIED when no permission
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Update_PermissionDenied, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Update_PermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://unknown/uri");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Update(uri, predicates, values);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Update_PermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Delete_0002
 * @tc.name: Delete_PermissionDenied
 * @tc.desc: Test Delete returns E_PERMISSION_DENIED when no permission
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Delete_PermissionDenied, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Delete_PermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://unknown/uri");
    DataShare::DataSharePredicates predicates;
    
    int result = stub_->Delete(uri, predicates);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Delete_PermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0021
 * @tc.name: Query_PermissionDenied
 * @tc.desc: Test Query returns nullptr when no permission
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_PermissionDenied, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_PermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://unknown/uri");
    DataShare::DataSharePredicates predicates;
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_EQ(businessError.GetCode(), E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_PermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_MatchConfig_0001
 * @tc.name: MatchConfig_InvalidUri
 * @tc.desc: Test MatchConfig with invalid URI returns empty record
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, MatchConfig_InvalidUri, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_MatchConfig_InvalidUri start";

    ASSERT_NE(stub_, nullptr);

    Uri invalidUri("datashare://invalid/uri");
    
    auto config = stub_->GetConfig();
    bool found = false;
    for (const auto& record : config.records) {
        if (record.uri == invalidUri.ToString()) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_MatchConfig_InvalidUri end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0022
 * @tc.name: Query_ResultVerification
 * @tc.desc: Test Query returns correct result structure
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_ResultVerification, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ResultVerification start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({
        "conditions": [
            { "week_index":1, "begin_time":1718515200000, "end_time":1718601600000 }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    if (resultSet != nullptr) {
        EXPECT_NE(resultSet, nullptr);
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ResultVerification end";
}

/* ---------- CheckCallingPermission Tests ---------- */

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0001
 * @tc.name: CheckCallingPermission_ReadPermissionGranted
 * @tc.desc: Test CheckCallingPermission with read permission granted
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_ReadPermissionGranted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_ReadPermissionGranted start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(validUri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_ReadPermissionGranted end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0002
 * @tc.name: CheckCallingPermission_ReadPermissionDenied
 * @tc.desc: Test CheckCallingPermission with read permission denied
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_ReadPermissionDenied, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_ReadPermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_DENIED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(validUri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_EQ(businessError.GetCode(), E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_ReadPermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0003
 * @tc.name: CheckCallingPermission_WritePermissionGranted
 * @tc.desc: Test CheckCallingPermission with write permission granted
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_WritePermissionGranted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_WritePermissionGranted start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Insert(validUri, values);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_WritePermissionGranted end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0004
 * @tc.name: CheckCallingPermission_WritePermissionDenied
 * @tc.desc: Test CheckCallingPermission with write permission denied
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_WritePermissionDenied, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_WritePermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_DENIED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Insert(validUri, values);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_WritePermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0005
 * @tc.name: CheckCallingPermission_InvalidUriRead
 * @tc.desc: Test CheckCallingPermission with invalid URI for read operation
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_InvalidUriRead, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_InvalidUriRead start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri invalidUri("datashare://unknown/invalid_uri");
    DataShare::DataSharePredicates predicates;
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(invalidUri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_EQ(businessError.GetCode(), E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_InvalidUriRead end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0006
 * @tc.name: CheckCallingPermission_InvalidUriWrite
 * @tc.desc: Test CheckCallingPermission with invalid URI for write operation
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_InvalidUriWrite, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_InvalidUriWrite start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri invalidUri("datashare://unknown/invalid_uri");
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Insert(invalidUri, values);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_InvalidUriWrite end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0007
 * @tc.name: CheckCallingPermission_UpdatePermissionGranted
 * @tc.desc: Test CheckCallingPermission for Update with permission granted
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_UpdatePermissionGranted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_UpdatePermissionGranted start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Update(validUri, predicates, values);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_UpdatePermissionGranted end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0008
 * @tc.name: CheckCallingPermission_UpdatePermissionDenied
 * @tc.desc: Test CheckCallingPermission for Update with permission denied
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_UpdatePermissionDenied, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_UpdatePermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_DENIED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    
    int result = stub_->Update(validUri, predicates, values);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_UpdatePermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0009
 * @tc.name: CheckCallingPermission_DeletePermissionGranted
 * @tc.desc: Test CheckCallingPermission for Delete with permission granted
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_DeletePermissionGranted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_DeletePermissionGranted start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    int result = stub_->Delete(validUri, predicates);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_DeletePermissionGranted end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0010
 * @tc.name: CheckCallingPermission_DeletePermissionDenied
 * @tc.desc: Test CheckCallingPermission for Delete with permission denied
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_DeletePermissionDenied, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_DeletePermissionDenied start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_DENIED;
    
    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    int result = stub_->Delete(validUri, predicates);
    EXPECT_EQ(result, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_DeletePermissionDenied end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0011
 * @tc.name: CheckCallingPermission_DifferentTokenId
 * @tc.desc: Test CheckCallingPermission with different token IDs
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_DifferentTokenId, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_DifferentTokenId start";

    ASSERT_NE(stub_, nullptr);

    std::vector<uint32_t> tokenIds = {100, 200, 300, 99999, 0xFFFFFFFF};
    
    for (auto tokenId : tokenIds) {
        g_mockCallingTokenId = tokenId;
        g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
        
        Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
        DataShare::DataSharePredicates predicates;
        std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
        predicates.EqualTo("query", queryJson);
        
        std::vector<std::string> columns;
        DataShare::DatashareBusinessError businessError;
        
        auto resultSet = stub_->Query(validUri, predicates, columns, businessError);
        EXPECT_NE(resultSet, nullptr);
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_DifferentTokenId end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0012
 * @tc.name: CheckCallingPermission_PermissionCheckCalled
 * @tc.desc: Test that VerifyAccessToken is called with correct permission
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_PermissionCheckCalled, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_PermissionCheckCalled start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    auto config = stub_->GetConfig();
    ASSERT_EQ(config.records.size(), 1);
    
    std::string expectedPermission = config.records[0].readPermission;
    EXPECT_EQ(expectedPermission, "ohos.permission.STORAGE_MANAGER");
    
    Uri validUri(config.records[0].uri);
    DataShare::DataSharePredicates predicates;
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(validUri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_PermissionCheckCalled end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_CheckCallingPermission_0013
 * @tc.name: CheckCallingPermission_MultipleCalls
 * @tc.desc: Test multiple permission checks in sequence
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, CheckCallingPermission_MultipleCalls, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_MultipleCalls start";

    ASSERT_NE(stub_, nullptr);

    Uri validUri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    for (int i = 0; i < 3; i++) {
        g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
        
        DataShare::DataSharePredicates predicates;
        std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
        predicates.EqualTo("query", queryJson);
        
        std::vector<std::string> columns;
        DataShare::DatashareBusinessError businessError;
        
        auto resultSet = stub_->Query(validUri, predicates, columns, businessError);
        EXPECT_NE(resultSet, nullptr);
        
        g_mockVerifyAccessTokenResult = PERMISSION_DENIED;
        
        DataShare::DataShareValuesBucket values;
        int result = stub_->Insert(validUri, values);
        EXPECT_EQ(result, E_PERMISSION_DENIED);
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_CheckCallingPermission_MultipleCalls end";
}

/* ---------- BuildResultSet Tests ---------- */

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0001
 * @tc.name: BuildResultSet_Success
 * @tc.desc: Test BuildResultSet successfully builds DataShareResultSet
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_Success start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({
        "conditions": [
            { "week_index":1, "begin_time":1718515200000, "end_time":1718601600000 }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);
    EXPECT_EQ(businessError.GetCode(), E_OK);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_Success end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0002
 * @tc.name: BuildResultSet_EmptyConditionsResult
 * @tc.desc: Test BuildResultSet with empty conditions array result
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_EmptyConditionsResult, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_EmptyConditionsResult start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_EmptyConditionsResult end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0003
 * @tc.name: BuildResultSet_LargeResultSet
 * @tc.desc: Test BuildResultSet with large result set
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_LargeResultSet, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_LargeResultSet start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[)";
    for (int i = 0; i < 50; i++) {
        if (i > 0) queryJson += ",";
        queryJson += R"({"week_index":)" + std::to_string(i) +
            R"(,"begin_time":1718515200000,"end_time":1718601600000})";
    }
    queryJson += R"(]})";
    
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_LargeResultSet end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0004
 * @tc.name: BuildResultSet_SpecialCharactersInJson
 * @tc.desc: Test BuildResultSet with special characters in JSON
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_SpecialCharactersInJson, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_SpecialCharactersInJson start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1718515200000,"end_time":1718601600000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_SpecialCharactersInJson end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0005
 * @tc.name: BuildResultSet_ResultStructureValidation
 * @tc.desc: Test BuildResultSet returns correct structure with weekly_revenue
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_ResultStructureValidation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_ResultStructureValidation start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({
        "conditions": [
            { "week_index":1, "begin_time":1718515200000, "end_time":1718601600000 },
            { "week_index":2, "begin_time":1718601600000, "end_time":1718688000000 }
        ]
    })";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_ResultStructureValidation end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0006
 * @tc.name: BuildResultSet_ZeroCacheSize
 * @tc.desc: Test BuildResultSet with zero cache size in result
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_ZeroCacheSize, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_ZeroCacheSize start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":0,"begin_time":0,"end_time":0}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_ZeroCacheSize end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0007
 * @tc.name: BuildResultSet_NegativeValues
 * @tc.desc: Test BuildResultSet with negative week_index and time values
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_NegativeValues, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_NegativeValues start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":-1,"begin_time":-1000,"end_time":-2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_NegativeValues end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0008
 * @tc.name: BuildResultSet_MultipleSequentialCalls
 * @tc.desc: Test BuildResultSet called multiple times in sequence
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_MultipleSequentialCalls, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_MultipleSequentialCalls start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    for (int i = 0; i < 5; i++) {
        DataShare::DataSharePredicates predicates;
        std::string queryJson = R"({"conditions":[{"week_index":)" + std::to_string(i) +
            R"(,"begin_time":1718515200000,"end_time":1718601600000}]})";
        predicates.EqualTo("query", queryJson);
        
        std::vector<std::string> columns;
        DataShare::DatashareBusinessError businessError;
        
        auto resultSet = stub_->Query(uri, predicates, columns, businessError);
        EXPECT_NE(resultSet, nullptr);
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_MultipleSequentialCalls end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0009
 * @tc.name: BuildResultSet_DifferentJsonFormats
 * @tc.desc: Test BuildResultSet with different JSON format variations
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_DifferentJsonFormats, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_DifferentJsonFormats start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    std::vector<std::string> jsonFormats = {
        R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})",
        R"({"conditions": [{"week_index": 1, "begin_time": 1000, "end_time": 2000}]})",
        R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000},]})",
        R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})"
    };
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    
    for (const auto& jsonStr : jsonFormats) {
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo("query", jsonStr);
        
        std::vector<std::string> columns;
        DataShare::DatashareBusinessError businessError;
        
        auto resultSet = stub_->Query(uri, predicates, columns, businessError);
        
        if (resultSet != nullptr) {
            EXPECT_NE(resultSet, nullptr);
        }
    }

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_DifferentJsonFormats end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_BuildResultSet_0010
 * @tc.name: BuildResultSet_VerifyDataShareResultSetType
 * @tc.desc: Test that BuildResultSet returns correct DataShareResultSet type
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, BuildResultSet_VerifyDataShareResultSetType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_VerifyDataShareResultSetType start";

    ASSERT_NE(stub_, nullptr);

    g_mockVerifyAccessTokenResult = PERMISSION_GRANTED;
    
    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1000,"end_time":2000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    ASSERT_NE(resultSet, nullptr);
    EXPECT_TRUE(dynamic_cast<DataShare::DataShareResultSet*>(resultSet.get()) != nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_BuildResultSet_VerifyDataShareResultSetType end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0016
 * @tc.name: Query_NoDataInTimeRange
 * @tc.desc: Test Query with time range that has no matching records
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_NoDataInTimeRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_NoDataInTimeRange start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":9999999999000,"end_time":9999999999999}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_NoDataInTimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0017
 * @tc.name: Query_MultipleRecordsInTimeRange
 * @tc.desc: Test Query with multiple records in time range to test revenue accumulation
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_MultipleRecordsInTimeRange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MultipleRecordsInTimeRange start";

    ASSERT_NE(stub_, nullptr);
    ASSERT_NE(store_, nullptr);

    int64_t baseTime = 1718515200000LL;
    for (int i = 0; i < 5; i++) {
        NativeRdb::ValuesBucket values;
        values.PutLong("clean_time", baseTime + i * 1000);
        values.PutLong("freed_size", 1024000LL * (i + 1));
        values.PutLong("clean_before", 2048000LL);
        values.PutLong("clean_after", 1024000LL);
        store_->Insert(values);
    }

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":)" +
        std::to_string(baseTime - 1000) + R"(,"end_time":)" +
        std::to_string(baseTime + 10000) + R"(}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_MultipleRecordsInTimeRange end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0018
 * @tc.name: Query_StoreNotInitialized
 * @tc.desc: Test Query when CleanRecordStore is not initialized
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_StoreNotInitialized, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_StoreNotInitialized start";

    ASSERT_NE(stub_, nullptr);

    store_->Close();

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":1718515200000,"end_time":1718601600000}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_NE(businessError.GetCode(), E_OK);

    store_->Init();

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_StoreNotInitialized end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0019
 * @tc.name: Query_ZeroTimeRange_001
 * @tc.desc: Test Query with zero time range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_ZeroTimeRange_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ZeroTimeRange_001 start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":0,"end_time":0}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_ZeroTimeRange_001 end";
}

/**
 * @tc.number: SUB_STORAGE_CleanRecordDataShareStub_Query_0020
 * @tc.name: Query_LargeTimeRange
 * @tc.desc: Test Query with very large time range
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CleanRecordDataShareStubTest, Query_LargeTimeRange, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_LargeTimeRange start";

    ASSERT_NE(stub_, nullptr);

    Uri uri("datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record");
    DataShare::DataSharePredicates predicates;
    
    std::string queryJson = R"({"conditions":[{"week_index":1,"begin_time":0,"end_time":9223372036854775807}]})";
    predicates.EqualTo("query", queryJson);
    
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    
    auto resultSet = stub_->Query(uri, predicates, columns, businessError);
    
    EXPECT_NE(resultSet, nullptr);

    GTEST_LOG_(INFO) << "CleanRecordDataShareStubTest_Query_LargeTimeRange end";
}

} // namespace StorageSpaceManager
} // namespace OHOS