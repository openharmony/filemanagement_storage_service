/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

const std::string BUNDLE_NAME = "com.ohos.bundleName-0-1";
const std::string BUNDLE_PATH = "/data/app/el2/100/base/com.ohos.bundleName-0-1";
const int32_t UID = 20000000;
const int32_t LIMITSIZE = 1000;
const std::string EMPTY_STRING = "";

class QuotaManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetInstance_001
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetInstance_001 start";

    QuotaManager *QuotaManager = QuotaManager::GetInstance();
    ASSERT_TRUE(QuotaManager != nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetInstance_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetInstance_002
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetInstance_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetInstance_002 start";

    QuotaManager *quotaManager1 = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager1 != nullptr);

    QuotaManager *quotaManager2 = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager2 != nullptr);

    ASSERT_TRUE(quotaManager1 == quotaManager2);
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetInstance_002 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_SetBundleQuota_001
 * @tc.desc: Test whether SetBundleQuota is called normally.(bundleName is empty)
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_SetBundleQuota_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_001 start";

    QuotaManager *quotaManager = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager != nullptr);

    std::string bundleName = EMPTY_STRING;
    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = quotaManager->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_SetBundleQuota_002
 * @tc.desc: Test whether SetBundleQuota is called normally.(uid < 0)
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_SetBundleQuota_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_002 start";

    QuotaManager *quotaManager = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager != nullptr);

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = -1;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = quotaManager->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_002 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_SetBundleQuota_003
 * @tc.desc: Test whether SetBundleQuota is called normally.(bundleDataDirPath is empty)
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_SetBundleQuota_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_003 start";

    QuotaManager *quotaManager = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager != nullptr);

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = UID;
    std::string bundleDataDirPath = EMPTY_STRING;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = quotaManager->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_003 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_SetBundleQuota_004
 * @tc.desc: Test whether SetBundleQuota is called normally.(limitSizeMb < 0)
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_SetBundleQuota_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_004 start";

    QuotaManager *quotaManager = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager != nullptr);

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = -1;
    int32_t result = quotaManager->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_004 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_SetBundleQuota_005
 * @tc.desc: Test whether CreateBundleDataDir is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_SetBundleQuota_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_005 start";

    QuotaManager *quotaManager = QuotaManager::GetInstance();
    ASSERT_TRUE(quotaManager != nullptr);

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = quotaManager->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_SYS_CALL);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_SetBundleQuota_005 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_CheckOverLongPath_001
 * @tc.desc: Test overLong path.
 * @tc.type: FUNC
 * @tc.require: AR20240111379420
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_CheckOverLongPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_CheckOverLongPath_001 start";

    std::string bundleDataDirPath = BUNDLE_PATH;
    uint32_t len = bundleDataDirPath.length();
    int32_t result = CheckOverLongPath(bundleDataDirPath);
    EXPECT_EQ(result, len);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_CheckOverLongPath_001 end";
}
} // STORAGE_DAEMON
} // OHOS