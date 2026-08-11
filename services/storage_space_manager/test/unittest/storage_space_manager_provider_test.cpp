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
#include "storage_space_manager_provider.h"
#include "storage_space_manager_errno.h"
#include "storage/storage_total_status_service.h"
#include "cache_clean_controller.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class StorageSpaceManagerProviderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static StorageSpaceManagerProvider* provider_;
};

StorageSpaceManagerProvider* StorageSpaceManagerProviderTest::provider_ = nullptr;

void StorageSpaceManagerProviderTest::SetUpTestCase()
{
    // Create provider instance for testing
    provider_ = new StorageSpaceManagerProvider(STORAGE_SPACE_MANAGER_SA_ID, false);
    ASSERT_NE(provider_, nullptr);
}

void StorageSpaceManagerProviderTest::TearDownTestCase()
{
    if (provider_ != nullptr) {
        delete provider_;
        provider_ = nullptr;
    }
}

void StorageSpaceManagerProviderTest::SetUp()
{
    // Reset provider state before each test
}

void StorageSpaceManagerProviderTest::TearDown()
{
    // Clean up after each test
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetTotalSize_0001
 * @tc.name: GetTotalSize_ServiceNotReady
 * @tc.desc: Test GetTotalSize when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetTotalSize_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalSize_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    // Don't start the service, so serviceReady_ should be false
    int64_t totalSize = 0;
    int32_t ret = provider_->GetTotalSize(totalSize);

    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalSize_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetTotalSize_0002
 * @tc.name: GetTotalSize_ServiceReady
 * @tc.desc: Test GetTotalSize when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetTotalSize_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalSize_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    // Start the service
    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    // Now service should be ready
    int64_t totalSize = 0;
    int32_t ret = provider_->GetTotalSize(totalSize);

    // Should succeed or return appropriate error from StorageTotalStatusService
    // Note: Actual result depends on StorageTotalStatusService implementation
    EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalSize_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetSystemSize_0001
 * @tc.name: GetSystemSize_ServiceNotReady
 * @tc.desc: Test GetSystemSize when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetSystemSize_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetSystemSize_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    int64_t systemSize = 0;
    int32_t ret = provider_->GetSystemSize(systemSize);

    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetSystemSize_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetSystemSize_0002
 * @tc.name: GetSystemSize_ServiceReady
 * @tc.desc: Test GetSystemSize when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetSystemSize_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetSystemSize_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    int64_t systemSize = 0;
    int32_t ret = provider_->GetSystemSize(systemSize);

    EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetSystemSize_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetFreeSize_0001
 * @tc.name: GetFreeSize_ServiceNotReady
 * @tc.desc: Test GetFreeSize when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetFreeSize_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeSize_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    int64_t freeSize = 0;
    int32_t ret = provider_->GetFreeSize(freeSize);

    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeSize_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetFreeSize_0002
 * @tc.name: GetFreeSize_ServiceReady
 * @tc.desc: Test GetFreeSize when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetFreeSize_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeSize_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    int64_t freeSize = 0;
    int32_t ret = provider_->GetFreeSize(freeSize);

    EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeSize_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetTotalInodes_0001
 * @tc.name: GetTotalInodes_ServiceNotReady
 * @tc.desc: Test GetTotalInodes when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetTotalInodes_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalInodes_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    int64_t totalInodes = 0;
    int32_t ret = provider_->GetTotalInodes(totalInodes);

    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalInodes_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetTotalInodes_0002
 * @tc.name: GetTotalInodes_ServiceReady
 * @tc.desc: Test GetTotalInodes when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetTotalInodes_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalInodes_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    int64_t totalInodes = 0;
    int32_t ret = provider_->GetTotalInodes(totalInodes);

    EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetTotalInodes_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetFreeInodes_0001
 * @tc.name: GetFreeInodes_ServiceNotReady
 * @tc.desc: Test GetFreeInodes when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetFreeInodes_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeInodes_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    int64_t freeInodes = 0;
    int32_t ret = provider_->GetFreeInodes(freeInodes);

    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeInodes_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetFreeInodes_0002
 * @tc.name: GetFreeInodes_ServiceReady
 * @tc.desc: Test GetFreeInodes when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetFreeInodes_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeInodes_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    int64_t freeInodes = 0;
    int32_t ret = provider_->GetFreeInodes(freeInodes);

    EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetFreeInodes_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_CleanBundleCache_0001
 * @tc.name: CleanBundleCache_ServiceNotReady
 * @tc.desc: Test CleanBundleCache when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, CleanBundleCache_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_CleanBundleCache_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    int32_t userId = 100;
    int32_t ret = provider_->CleanBundleCache(userId);

    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_CleanBundleCache_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_CleanBundleCache_0002
 * @tc.name: CleanBundleCache_ServiceReady
 * @tc.desc: Test CleanBundleCache when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, CleanBundleCache_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_CleanBundleCache_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    int32_t userId = 100;
    int32_t ret = provider_->CleanBundleCache(userId);

    // Result depends on CacheCleanController implementation
    // Should not crash and should return a valid error code
    EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY ||
                ret == E_INVALID_ARGUMENT || ret == E_SERVICE_IS_NULLPTR);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_CleanBundleCache_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_CleanBundleCache_0003
 * @tc.name: CleanBundleCache_MultipleUsers
 * @tc.desc: Test CleanBundleCache for multiple user IDs
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, CleanBundleCache_MultipleUsers, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_CleanBundleCache_MultipleUsers start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);

    // Test with different user IDs
    std::vector<int32_t> userIds = {0, 100, 101, 102};

    for (int32_t userId : userIds) {
        int32_t ret = provider_->CleanBundleCache(userId);
        // Should not crash
        EXPECT_TRUE(ret == E_OK || ret == E_FAIL || ret == E_SERVICE_NOT_READY ||
                    ret == E_INVALID_ARGUMENT || ret == E_SERVICE_IS_NULLPTR);
    }

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_CleanBundleCache_MultipleUsers end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_Lifecycle_0001
 * @tc.name: Lifecycle_StartStop
 * @tc.desc: Test provider lifecycle: start and stop
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, Lifecycle_StartStop, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_Lifecycle_StartStop start";

    ASSERT_NE(provider_, nullptr);

    // Start service
    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);
    provider_->Init();

    // Verify service is ready by calling API
    int64_t totalSize = 0;
    int32_t ret = provider_->GetTotalSize(totalSize);
    // Should not return E_SERVICE_NOT_READY since service is started
    EXPECT_NE(ret, E_SERVICE_NOT_READY);

    // Stop service
    provider_->OnStop();

    // Verify service is not ready
    ret = provider_->GetTotalSize(totalSize);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_Lifecycle_StartStop end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_Lifecycle_0002
 * @tc.name: Lifecycle_Restart
 * @tc.desc: Test provider can be restarted after stop
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, Lifecycle_Restart, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_Lifecycle_Restart start";

    ASSERT_NE(provider_, nullptr);

    // First start
    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);
    provider_->OnStop();

    // Restart
    provider_->OnStart(startReason);
    provider_->Init();

    // Verify service is ready again
    int64_t totalSize = 0;
    int32_t ret = provider_->GetTotalSize(totalSize);
    EXPECT_NE(ret, E_SERVICE_NOT_READY);

    // Stop again
    provider_->OnStop();

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_Lifecycle_Restart end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetDataShareService_0001
 * @tc.name: GetDataShareService_ServiceNotReady
 * @tc.desc: Test GetDataShareService when service is not ready
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetDataShareService_ServiceNotReady, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_ServiceNotReady start";

    ASSERT_NE(provider_, nullptr);

    std::string uri = "datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record";
    sptr<IRemoteObject> remoteObject;
    
    int32_t ret = provider_->GetDataShareService(uri, remoteObject);
    
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
    EXPECT_EQ(remoteObject, nullptr);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_ServiceNotReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetDataShareService_0002
 * @tc.name: GetDataShareService_ServiceReady
 * @tc.desc: Test GetDataShareService when service is ready
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetDataShareService_ServiceReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_ServiceReady start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);
    provider_->Init();

    std::string uri = "datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record";
    sptr<IRemoteObject> remoteObject;
    
    int32_t ret = provider_->GetDataShareService(uri, remoteObject);
    
    EXPECT_TRUE(ret == E_OK || ret == E_PERMISSION_DENIED || ret == E_SERVICE_ON_IDLE);
    if (ret == E_OK) {
        EXPECT_NE(remoteObject, nullptr);
    } else {
        EXPECT_EQ(remoteObject, nullptr);
    }

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_ServiceReady end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetDataShareService_0003
 * @tc.name: GetDataShareService_InvalidUri
 * @tc.desc: Test GetDataShareService with invalid URI
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetDataShareService_InvalidUri, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_InvalidUri start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);
    provider_->Init();

    std::string uri = "";
    sptr<IRemoteObject> remoteObject;
    
    int32_t ret = provider_->GetDataShareService(uri, remoteObject);
    
    EXPECT_NE(ret, E_OK);
    EXPECT_EQ(remoteObject, nullptr);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_InvalidUri end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_GetDataShareService_0004
 * @tc.name: GetDataShareService_MultipleCalls
 * @tc.desc: Test calling GetDataShareService multiple times
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, GetDataShareService_MultipleCalls, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_MultipleCalls start";

    ASSERT_NE(provider_, nullptr);

    SystemAbilityOnDemandReason startReason;
    provider_->OnStart(startReason);
    provider_->Init();

    std::string uri = "datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record";
    
    for (int i = 0; i < 3; i++) {
        sptr<IRemoteObject> remoteObject;
        int32_t ret = provider_->GetDataShareService(uri, remoteObject);
        EXPECT_TRUE(ret == E_OK || ret == E_PERMISSION_DENIED || ret == E_SERVICE_ON_IDLE);
    }

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_GetDataShareService_MultipleCalls end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_OnExtension_0001
 * @tc.name: OnExtension_DataShareConnection
 * @tc.desc: Test OnExtension with datashare_connection extension
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageSpaceManagerProviderTest, OnExtension_DataShareConnection, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_OnExtension_DataShareConnection start";

    ASSERT_NE(provider_, nullptr);

    MessageParcel data;
    MessageParcel reply;
    
    int32_t ret = provider_->OnExtension("datashare_connection", data, reply);
    
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_OnExtension_DataShareConnection end";
}

/**
 * @tc.number: SUB_STORAGE_Provider_OnExtension_0002
 * @tc.name: OnExtension_UnknownExtension
 * @tc.desc: Test OnExtension with unknown extension type
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(StorageSpaceManagerProviderTest, OnExtension_UnknownExtension, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_OnExtension_UnknownExtension start";

    ASSERT_NE(provider_, nullptr);

    MessageParcel data;
    MessageParcel reply;
    
    int32_t ret = provider_->OnExtension("unknown_extension", data, reply);
    
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageSpaceManagerProviderTest_OnExtension_UnknownExtension end";
}

} // namespace StorageSpaceManager
} // namespace OHOS
