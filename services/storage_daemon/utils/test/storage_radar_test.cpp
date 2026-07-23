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

#include "utils/storage_radar.h"
#include "storage_service_errno.h"

#include <gtest/gtest.h>
#include <string>
#include <chrono>
#include <cstdint>
#include <thread>

namespace OHOS {
namespace StorageService {
namespace Test {
using namespace testing;
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_MS { 50 };
} // namespace

class StorageRadarTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown();
};

void StorageRadarTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_MS));
}

/**
 * @tc.name: StorageRadarTest_ReportActiveUserKey_001
 * @tc.desc: Verify ReportActiveUserKey with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportActiveUserKey_001, TestSize.Level1)
{
    std::string funcName = "ActiveUserKey";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::string keyLevel = "EL2";
    GTEST_LOG_(INFO) << "StorageRadarTest_ReportActiveUserKey_001 Start";
    ASSERT_NO_FATAL_FAILURE(StorageRadar::ReportActiveUserKey(funcName, userId, ret, keyLevel));
    GTEST_LOG_(INFO) << "StorageRadarTest_ReportActiveUserKey_001 end";
}

/**
 * @tc.name: StorageRadarTest_ReportActiveUserKey_002
 * @tc.desc: Verify ReportActiveUserKey with zero userId.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportActiveUserKey_002, TestSize.Level1)
{
    std::string funcName = "ActiveUserKey";
    uint32_t userId = 0;
    int32_t ret = E_OK;
    std::string keyLevel = "EL1";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportActiveUserKey(funcName, userId, ret, keyLevel));
}

/**
 * @tc.name: StorageRadarTest_ReportActiveUserKey_003
 * @tc.desc: Verify ReportActiveUserKey with empty keyLevel.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportActiveUserKey_003, TestSize.Level1)
{
    std::string funcName = "ActiveUserKey";
    uint32_t userId = 100;
    int32_t ret = E_ERR;
    std::string keyLevel;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportActiveUserKey(funcName, userId, ret, keyLevel));
}

/**
 * @tc.name: StorageRadarTest_ReportActiveUserKey_004
 * @tc.desc: Verify ReportActiveUserKey with max userId boundary.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportActiveUserKey_004, TestSize.Level1)
{
    std::string funcName = "ActiveUserKey";
    uint32_t userId = 0xFFFFFFFF;
    int32_t ret = E_OK;
    std::string keyLevel = "EL5";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportActiveUserKey(funcName, userId, ret, keyLevel));
}

/**
 * @tc.name: StorageRadarTest_ReportGetStorageStatus_001
 * @tc.desc: Verify ReportGetStorageStatus with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportGetStorageStatus_001, TestSize.Level1)
{
    std::string funcName = "GetStorageStatus";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::string orgPkg = "com.test.app";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportGetStorageStatus(funcName, userId, ret, orgPkg));
}

/**
 * @tc.name: StorageRadarTest_ReportGetStorageStatus_002
 * @tc.desc: Verify ReportGetStorageStatus with empty orgPkg.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportGetStorageStatus_002, TestSize.Level1)
{
    std::string funcName = "GetStorageStatus";
    uint32_t userId = 100;
    int32_t ret = E_ERR;
    std::string orgPkg;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportGetStorageStatus(funcName, userId, ret, orgPkg));
}

/**
 * @tc.name: StorageRadarTest_ReportVolumeOperation_001
 * @tc.desc: Verify ReportVolumeOperation with success return.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportVolumeOperation_001, TestSize.Level1)
{
    std::string funcName = "Mount";
    int32_t ret = E_OK;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportVolumeOperation(funcName, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportVolumeOperation_002
 * @tc.desc: Verify ReportVolumeOperation with error return.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportVolumeOperation_002, TestSize.Level1)
{
    std::string funcName = "Unmount";
    int32_t ret = E_ERR;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportVolumeOperation(funcName, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportVolumeOperation_003
 * @tc.desc: Verify ReportVolumeOperation with empty funcName.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportVolumeOperation_003, TestSize.Level1)
{
    std::string funcName;
    int32_t ret = E_OK;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportVolumeOperation(funcName, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportUserKeyResult_001
 * @tc.desc: Verify ReportUserKeyResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserKeyResult_001, TestSize.Level1)
{
    std::string funcName = "GenerateUserKeys";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::string keyLevel = "EL2";
    std::string extraData = "normal";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserKeyResult(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportUserKeyResult_002
 * @tc.desc: Verify ReportUserKeyResult with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserKeyResult_002, TestSize.Level1)
{
    std::string funcName = "InitGlobalKey";
    uint32_t userId = 100;
    int32_t ret = E_ERR;
    std::string keyLevel = "EL1";
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserKeyResult(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportUserKeyResult_003
 * @tc.desc: Verify ReportUserKeyResult with empty keyLevel and error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserKeyResult_003, TestSize.Level1)
{
    std::string funcName = "DeleteUserKeys";
    uint32_t userId = 0;
    int32_t ret = -1;
    std::string keyLevel;
    std::string extraData = "delete_failed";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserKeyResult(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportUserManager_001
 * @tc.desc: Verify ReportUserManager with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserManager_001, TestSize.Level1)
{
    std::string funcName = "PrepareUserDirs";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::string extraData = "success";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserManager(funcName, userId, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportUserManager_002
 * @tc.desc: Verify ReportUserManager with empty extraData and error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserManager_002, TestSize.Level1)
{
    std::string funcName = "DestroyUserDirs";
    uint32_t userId = 105;
    int32_t ret = E_ERR;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserManager(funcName, userId, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportUpdateUserAuth_001
 * @tc.desc: Verify ReportUpdateUserAuth with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUpdateUserAuth_001, TestSize.Level1)
{
    std::string funcName = "UpdateUserAuth";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::string keyLevel = "EL2";
    std::string extraData = "auth_updated";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUpdateUserAuth(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportUpdateUserAuth_002
 * @tc.desc: Verify ReportUpdateUserAuth with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUpdateUserAuth_002, TestSize.Level1)
{
    std::string funcName = "UpdateUserAuth";
    uint32_t userId = 100;
    int32_t ret = E_ERR;
    std::string keyLevel = "EL4";
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUpdateUserAuth(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportFbexResult_001
 * @tc.desc: Verify ReportFbexResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportFbexResult_001, TestSize.Level1)
{
    std::string funcName = "FbexEncrypt";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::string keyLevel = "EL2";
    std::string extraData = "fbex_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFbexResult(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportFbexResult_002
 * @tc.desc: Verify ReportFbexResult with empty keyLevel and error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportFbexResult_002, TestSize.Level1)
{
    std::string funcName = "FbexDecrypt";
    uint32_t userId = 0;
    int32_t ret = E_ERR;
    std::string keyLevel;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFbexResult(funcName, userId, ret, keyLevel, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportCommonResult_001
 * @tc.desc: Verify ReportCommonResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportCommonResult_001, TestSize.Level1)
{
    std::string funcName = "CommonOp";
    int32_t ret = E_OK;
    unsigned int userId = 100;
    std::string extraData = "normal";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportCommonResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportCommonResult_002
 * @tc.desc: Verify ReportCommonResult with zero userId and empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportCommonResult_002, TestSize.Level1)
{
    std::string funcName = "CommonOp";
    int32_t ret = E_ERR;
    unsigned int userId = 0;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportCommonResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportIamResult_001
 * @tc.desc: Verify ReportIamResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportIamResult_001, TestSize.Level1)
{
    std::string funcName = "IamAuth";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportIamResult(funcName, userId, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportIamResult_002
 * @tc.desc: Verify ReportIamResult with error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportIamResult_002, TestSize.Level1)
{
    std::string funcName = "IamAuth";
    uint32_t userId = 100;
    int32_t ret = E_ERR;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportIamResult(funcName, userId, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportHuksResult_001
 * @tc.desc: Verify ReportHuksResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportHuksResult_001, TestSize.Level1)
{
    std::string funcName = "HuksGenerate";
    int32_t ret = E_OK;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportHuksResult(funcName, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportHuksResult_002
 * @tc.desc: Verify ReportHuksResult with error ret and empty funcName.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportHuksResult_002, TestSize.Level1)
{
    std::string funcName;
    int32_t ret = E_ERR;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportHuksResult(funcName, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportMtpResult_001
 * @tc.desc: Verify ReportMtpResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportMtpResult_001, TestSize.Level1)
{
    std::string funcName = "MtpMount";
    int32_t ret = E_OK;
    std::string extraData = "mtp_mounted";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportMtpResult(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportMtpResult_002
 * @tc.desc: Verify ReportMtpResult with empty extraData and error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportMtpResult_002, TestSize.Level1)
{
    std::string funcName = "MtpUnmount";
    int32_t ret = E_ERR;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportMtpResult(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportStorageUsage_001
 * @tc.desc: Verify ReportStorageUsage with normal stage.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageUsage_001, TestSize.Level1)
{
    BizStage stage = BizStage::BIZ_STAGE_THRESHOLD_CLEAN_HIGH;
    std::string extraData = "clean_high";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageUsage(stage, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportStorageUsage_002
 * @tc.desc: Verify ReportStorageUsage with different stage and empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageUsage_002, TestSize.Level1)
{
    BizStage stage = BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_LOW;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageUsage(stage, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportKeyRingResult_001
 * @tc.desc: Verify ReportKeyRingResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportKeyRingResult_001, TestSize.Level1)
{
    std::string funcName = "KeyRingOp";
    int32_t ret = E_OK;
    std::string extraData = "keyring_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportKeyRingResult(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportKeyRingResult_002
 * @tc.desc: Verify ReportKeyRingResult with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportKeyRingResult_002, TestSize.Level1)
{
    std::string funcName = "KeyRingOp";
    int32_t ret = E_ERR;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportKeyRingResult(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportOsAccountResult_001
 * @tc.desc: Verify ReportOsAccountResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportOsAccountResult_001, TestSize.Level1)
{
    std::string funcName = "OsAccountCreate";
    int32_t ret = E_OK;
    unsigned int userId = 100;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportOsAccountResult(funcName, ret, userId));
}

/**
 * @tc.name: StorageRadarTest_ReportOsAccountResult_002
 * @tc.desc: Verify ReportOsAccountResult with zero userId.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportOsAccountResult_002, TestSize.Level1)
{
    std::string funcName = "OsAccountRemove";
    int32_t ret = E_ERR;
    unsigned int userId = 0;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportOsAccountResult(funcName, ret, userId));
}

/**
 * @tc.name: StorageRadarTest_ReportEl5KeyMgrResult_001
 * @tc.desc: Verify ReportEl5KeyMgrResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportEl5KeyMgrResult_001, TestSize.Level1)
{
    std::string funcName = "El5KeyMgr";
    int32_t ret = E_OK;
    unsigned int userId = 100;
    std::string extraData = "el5_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportEl5KeyMgrResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportEl5KeyMgrResult_002
 * @tc.desc: Verify ReportEl5KeyMgrResult with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportEl5KeyMgrResult_002, TestSize.Level1)
{
    std::string funcName = "El5KeyMgr";
    int32_t ret = E_ERR;
    unsigned int userId = 0;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportEl5KeyMgrResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportTEEClientResult_001
 * @tc.desc: Verify ReportTEEClientResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportTEEClientResult_001, TestSize.Level1)
{
    std::string funcName = "TEEClientOp";
    int32_t ret = E_OK;
    unsigned int userId = 100;
    std::string extraData = "tee_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportTEEClientResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportTEEClientResult_002
 * @tc.desc: Verify ReportTEEClientResult with empty extraData and error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportTEEClientResult_002, TestSize.Level1)
{
    std::string funcName = "TEEClientOp";
    int32_t ret = E_ERR;
    unsigned int userId = 0;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportTEEClientResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportBundleMgrResult_001
 * @tc.desc: Verify ReportBundleMgrResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportBundleMgrResult_001, TestSize.Level1)
{
    std::string funcName = "GetBundleStats";
    int32_t ret = E_OK;
    unsigned int userId = 100;
    std::string extraData = "bundle_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportBundleMgrResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportBundleMgrResult_002
 * @tc.desc: Verify ReportBundleMgrResult with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportBundleMgrResult_002, TestSize.Level1)
{
    std::string funcName = "GetBundleStats";
    int32_t ret = E_ERR;
    unsigned int userId = 0;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportBundleMgrResult(funcName, ret, userId, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportSaSizeResult_001
 * @tc.desc: Verify ReportSaSizeResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportSaSizeResult_001, TestSize.Level1)
{
    std::string funcName = "GetSystemDataSize";
    int32_t ret = E_OK;
    std::string extraData = "size_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportSaSizeResult(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportSaSizeResult_002
 * @tc.desc: Verify ReportSaSizeResult with empty extraData and error ret.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportSaSizeResult_002, TestSize.Level1)
{
    std::string funcName = "GetSystemDataSize";
    int32_t ret = E_ERR;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportSaSizeResult(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportSpaceRadar_001
 * @tc.desc: Verify ReportSpaceRadar with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportSpaceRadar_001, TestSize.Level1)
{
    std::string funcName = "GetTotalSize";
    int32_t ret = E_OK;
    std::string extraData = "space_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportSpaceRadar(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportSpaceRadar_002
 * @tc.desc: Verify ReportSpaceRadar with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportSpaceRadar_002, TestSize.Level1)
{
    std::string funcName = "GetFreeSize";
    int32_t ret = E_ERR;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportSpaceRadar(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportStorageStatusRadar_001
 * @tc.desc: Verify ReportStorageStatusRadar (overload 1) with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageStatusRadar_001, TestSize.Level1)
{
    std::string orgPkgName = "com.test.app";
    std::string extraData = "status_normal";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageStatusRadar(orgPkgName, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportStorageStatusRadar_002
 * @tc.desc: Verify ReportStorageStatusRadar (overload 1) with empty orgPkgName.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageStatusRadar_002, TestSize.Level1)
{
    std::string orgPkgName;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageStatusRadar(orgPkgName, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportStorageStatusRadar_003
 * @tc.desc: Verify ReportStorageStatusRadar (overload 2) with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageStatusRadar_003, TestSize.Level1)
{
    std::string funcName = "GetStorageStatus";
    int32_t ret = E_OK;
    std::string extraData = "status_ok";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageStatusRadar(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportStorageStatusRadar_004
 * @tc.desc: Verify ReportStorageStatusRadar (overload 2) with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageStatusRadar_004, TestSize.Level1)
{
    std::string funcName = "GetStorageStatus";
    int32_t ret = E_ERR;
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageStatusRadar(funcName, ret, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportSetQuotaByBaseline_001
 * @tc.desc: Verify ReportSetQuotaByBaseline with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportSetQuotaByBaseline_001, TestSize.Level1)
{
    std::string funcName = "SetBundleQuota";
    std::string extraData = "quota_set";
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportSetQuotaByBaseline(funcName, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportSetQuotaByBaseline_002
 * @tc.desc: Verify ReportSetQuotaByBaseline with empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportSetQuotaByBaseline_002, TestSize.Level1)
{
    std::string funcName = "SetBundleQuota";
    std::string extraData;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportSetQuotaByBaseline(funcName, extraData));
}

/**
 * @tc.name: StorageRadarTest_ReportFucBehavior_001
 * @tc.desc: Verify ReportFucBehavior with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportFucBehavior_001, TestSize.Level1)
{
    std::string funcName = "Mount";
    uint32_t userId = 100;
    std::string extraData = "behavior_ok";
    int32_t ret = E_OK;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFucBehavior(funcName, userId, extraData, ret));
}

/**
 * @tc.name: StorageRadarTest_ReportFucBehavior_002
 * @tc.desc: Verify ReportFucBehavior with error ret and empty extraData.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportFucBehavior_002, TestSize.Level1)
{
    std::string funcName = "Unmount";
    uint32_t userId = 0;
    std::string extraData;
    int32_t ret = E_ERR;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFucBehavior(funcName, userId, extraData, ret));
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_001
 * @tc.desc: Verify RecordFunctionResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_001, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 100,
        .funcName = "Mount",
        .bizScene = BizScene::EXTERNAL_VOLUME_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_MOUNT,
        .keyElxLevel = "NA",
        .errorCode = E_OK,
        .extraData = "normal",
        .toCallPkg = "",
    };
    std::string eventName = FILE_STORAGE_MANAGER_FAULT;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_002
 * @tc.desc: Verify RecordFunctionResult with error code.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_002, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "os_account",
        .userId = 100,
        .funcName = "UpdateUserAuth",
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_UPDATE_USER_AUTH,
        .keyElxLevel = "EL2",
        .errorCode = E_ERR,
        .extraData = "auth_failed",
        .toCallPkg = "iam",
    };
    std::string eventName = FILE_STORAGE_MANAGER_FAULT;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_003
 * @tc.desc: Verify RecordFunctionResult with empty fields.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_003, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "",
        .userId = 0,
        .funcName = "",
        .bizScene = BizScene::STORAGE_START,
        .bizStage = BizStage::BIZ_STAGE_SA_START,
        .keyElxLevel = "",
        .errorCode = 0,
        .extraData = "",
        .toCallPkg = "",
    };
    std::string eventName = FILE_STORAGE_FAULT;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_004
 * @tc.desc: Verify RecordFunctionResult with default eventName.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_004, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 100,
        .funcName = "PrepareUserDirs",
        .bizScene = BizScene::USER_MOUNT_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_USER_MOUNT,
        .keyElxLevel = "NA",
        .errorCode = E_OK,
        .extraData = "default_event",
        .toCallPkg = "",
    };
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param));
}

/**
 * @tc.name: StorageRadarTest_RecordStorageStatusResult_001
 * @tc.desc: Verify RecordStorageStatusResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordStorageStatusResult_001, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "com.test.app",
        .userId = 0,
        .extraData = "status_normal",
    };
    std::string eventName = FILE_STORAGE_STATUS_STATISTIC;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordStorageStatusResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_RecordStorageStatusResult_002
 * @tc.desc: Verify RecordStorageStatusResult with empty fields.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordStorageStatusResult_002, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "",
        .userId = 0,
        .extraData = "",
    };
    std::string eventName = FILE_STORAGE_STATUS_STATISTIC;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordStorageStatusResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_RecordFaultResult_001
 * @tc.desc: Verify RecordFaultResult with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFaultResult_001, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 100,
        .funcName = "Mount",
        .bizScene = BizScene::EXTERNAL_VOLUME_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_MOUNT,
        .keyElxLevel = "NA",
        .errorCode = E_ERR,
        .extraData = "mount_failed",
        .toCallPkg = "",
    };
    std::string eventName = FILE_STORAGE_STATUS_FAULT;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFaultResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_RecordFaultResult_002
 * @tc.desc: Verify RecordFaultResult with empty fields.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFaultResult_002, TestSize.Level1)
{
    RadarParameter param = {
        .orgPkg = "",
        .userId = 0,
        .funcName = "",
        .bizScene = BizScene::STORAGE_START,
        .bizStage = BizStage::BIZ_STAGE_SA_START,
        .keyElxLevel = "",
        .errorCode = 0,
        .extraData = "",
        .toCallPkg = "",
    };
    std::string eventName = FILE_STORAGE_STATUS_FAULT;
    EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFaultResult(param, eventName));
}

/**
 * @tc.name: StorageRadarTest_ReportStatistics_001
 * @tc.desc: Verify ReportStatistics with normal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStatistics_001, TestSize.Level1)
{
    uint32_t userId = 100;
    StorageDaemon::RadarStatisticInfo radarInfo = {
        .keyLoadSuccCount = 10,
        .keyLoadFailCount = 1,
        .keyUnloadSuccCount = 5,
        .keyUnloadFailCount = 0,
        .userAddSuccCount = 3,
        .userAddFailCount = 0,
        .userRemoveSuccCount = 2,
        .userRemoveFailCount = 1,
        .userStartSuccCount = 4,
        .userStartFailCount = 0,
        .userStopSuccCount = 3,
        .userStopFailCount = 1,
    };
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStatistics(userId, radarInfo));
}

/**
 * @tc.name: StorageRadarTest_ReportStatistics_002
 * @tc.desc: Verify ReportStatistics with zero userId and zero counts.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStatistics_002, TestSize.Level1)
{
    uint32_t userId = 0;
    StorageDaemon::RadarStatisticInfo radarInfo = {};
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStatistics(userId, radarInfo));
}

/**
 * @tc.name: StorageRadarTest_ReportStatistics_003
 * @tc.desc: Verify ReportStatistics with max uint64 counts.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStatistics_003, TestSize.Level1)
{
    uint32_t userId = 100;
    StorageDaemon::RadarStatisticInfo radarInfo = {
        .keyLoadSuccCount = UINT64_MAX,
        .keyLoadFailCount = UINT64_MAX,
        .keyUnloadSuccCount = UINT64_MAX,
        .keyUnloadFailCount = UINT64_MAX,
        .userAddSuccCount = UINT64_MAX,
        .userAddFailCount = UINT64_MAX,
        .userRemoveSuccCount = UINT64_MAX,
        .userRemoveFailCount = UINT64_MAX,
        .userStartSuccCount = UINT64_MAX,
        .userStartFailCount = UINT64_MAX,
        .userStopSuccCount = UINT64_MAX,
        .userStopFailCount = UINT64_MAX,
    };
    EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStatistics(userId, radarInfo));
}

/**
 * @tc.name: StorageRadarTest_RecordCurrentTime_001
 * @tc.desc: Verify RecordCurrentTime returns positive value.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordCurrentTime_001, TestSize.Level1)
{
    int64_t time1 = StorageRadar::RecordCurrentTime();
    EXPECT_GT(time1, 0);
}

/**
 * @tc.name: StorageRadarTest_RecordCurrentTime_002
 * @tc.desc: Verify RecordCurrentTime returns increasing values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordCurrentTime_002, TestSize.Level1)
{
    int64_t time1 = StorageRadar::RecordCurrentTime();
    int64_t time2 = StorageRadar::RecordCurrentTime();
    EXPECT_GE(time2, time1);
}

/**
 * @tc.name: StorageRadarTest_ReportDuration_001
 * @tc.desc: Verify ReportDuration with duration under threshold.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportDuration_001, TestSize.Level1)
{
    std::string funcName = "Mount";
    int64_t startTime = StorageRadar::RecordCurrentTime();
    int64_t threshold = DEFAULT_DELAY_TIME_THRESH;
    uint32_t userId = 100;
    std::string result = StorageRadar::ReportDuration(funcName, startTime, threshold, userId);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("ms"), std::string::npos);
}

/**
 * @tc.name: StorageRadarTest_ReportDuration_002
 * @tc.desc: Verify ReportDuration with zero startTime (long duration).
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportDuration_002, TestSize.Level1)
{
    std::string funcName = "Unmount";
    int64_t startTime = 0;
    int64_t threshold = DEFAULT_DELAY_TIME_THRESH;
    uint32_t userId = 100;
    std::string result = StorageRadar::ReportDuration(funcName, startTime, threshold, userId);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("ms"), std::string::npos);
}

/**
 * @tc.name: StorageRadarTest_ReportDuration_003
 * @tc.desc: Verify ReportDuration with high threshold.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportDuration_003, TestSize.Level1)
{
    std::string funcName = "Format";
    int64_t startTime = StorageRadar::RecordCurrentTime();
    int64_t threshold = DELAY_TIME_THRESH_HIGH;
    uint32_t userId = 0;
    std::string result = StorageRadar::ReportDuration(funcName, startTime, threshold, userId);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: StorageRadarTest_ReportDuration_004
 * @tc.desc: Verify ReportDuration with default parameters.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportDuration_004, TestSize.Level1)
{
    std::string funcName = "Partition";
    int64_t startTime = StorageRadar::RecordCurrentTime();
    std::string result = StorageRadar::ReportDuration(funcName, startTime);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("ms"), std::string::npos);
}

/**
 * @tc.name: StorageRadarTest_ReportDuration_005
 * @tc.desc: Verify ReportDuration with very large startTime (future time).
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportDuration_005, TestSize.Level1)
{
    std::string funcName = "Check";
    int64_t startTime = INT64_MAX;
    int64_t threshold = DEFAULT_DELAY_TIME_THRESH;
    uint32_t userId = 100;
    std::string result = StorageRadar::ReportDuration(funcName, startTime, threshold, userId);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: StorageRadarTest_GetInstance_001
 * @tc.desc: Verify GetInstance returns valid reference.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_GetInstance_001, TestSize.Level1)
{
    StorageRadar &instance1 = StorageRadar::GetInstance();
    StorageRadar &instance2 = StorageRadar::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_005
 * @tc.desc: Verify RecordFunctionResult with all BizScene values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_005, TestSize.Level1)
{
    std::string eventName = FILE_STORAGE_MANAGER_FAULT;
    for (int32_t i = 0; i <= static_cast<int32_t>(BizScene::MTP_DEVICE_MANAGER); i++) {
        RadarParameter param = {
            .orgPkg = "storageService",
            .userId = 100,
            .funcName = "TestScene",
            .bizScene = static_cast<BizScene>(i),
            .bizStage = BizStage::BIZ_STAGE_SA_START,
            .keyElxLevel = "NA",
            .errorCode = E_OK,
            .extraData = "scene_test",
            .toCallPkg = "",
        };
        EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param, eventName));
    }
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_006
 * @tc.desc: Verify RecordFunctionResult with BizStage values (part 1).
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_006, TestSize.Level1)
{
    std::string eventName = FILE_STORAGE_MANAGER_FAULT;
    std::vector<BizStage> stages = {
        BizStage::BIZ_STAGE_SA_START, BizStage::BIZ_STAGE_CONNECT,
        BizStage::BIZ_STAGE_ENABLE, BizStage::BIZ_STAGE_SA_STOP,
        BizStage::BIZ_STAGE_PREPARE_ADD_USER, BizStage::BIZ_STAGE_START_USER,
        BizStage::BIZ_STAGE_STOP_USER, BizStage::BIZ_STAGE_REMOVE_USER,
        BizStage::BIZ_STAGE_GENERATE_USER_KEYS, BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        BizStage::BIZ_STAGE_UPDATE_USER_AUTH, BizStage::BIZ_STAGE_INACTIVE_USER_KEY,
        BizStage::BIZ_STAGE_DELETE_USER_KEYS, BizStage::BIZ_STAGE_CREATE_RECOVERY_KEY,
        BizStage::BIZ_STAGE_LOCK_USER_SCREEN, BizStage::BIZ_STAGE_UNLOCK_USER_SCREEN,
        BizStage::BIZ_STAGE_GET_FILE_ENCRYPT_STATUS, BizStage::BIZ_STAGE_UPDATE_KEY_CONTEXT,
        BizStage::BIZ_STAGE_INIT_GLOBAL_KEY,
    };
    for (const auto &stage : stages) {
        RadarParameter param = {
            .orgPkg = "storageService", .userId = 100, .funcName = "TestStage",
            .bizScene = BizScene::STORAGE_START, .bizStage = stage,
            .keyElxLevel = "NA", .errorCode = E_OK, .extraData = "stage_test", .toCallPkg = "",
        };
        EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param, eventName));
    }
}

/**
 * @tc.name: StorageRadarTest_RecordFunctionResult_007
 * @tc.desc: Verify RecordFunctionResult with BizStage values (part 2).
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFunctionResult_007, TestSize.Level1)
{
    std::string eventName = FILE_STORAGE_MANAGER_FAULT;
    std::vector<BizStage> stages = {
        BizStage::BIZ_STAGE_GET_TOTAL_SIZE, BizStage::BIZ_STAGE_GET_FREE_SIZE,
        BizStage::BIZ_STAGE_GET_SYSTEM_SIZE, BizStage::BIZ_STAGE_GET_BUNDLE_STATS,
        BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS, BizStage::BIZ_STAGE_MOUNT,
        BizStage::BIZ_STAGE_UNMOUNT, BizStage::BIZ_STAGE_PARTITION,
        BizStage::BIZ_STAGE_FORMAT, BizStage::BIZ_STAGE_SET_VOLUME_DESCRIPTION,
        BizStage::BIZ_STAGE_SET_BUNDLE_QUOTA, BizStage::BIZ_STAGE_GET_ALL_VOLUMES,
        BizStage::BIZ_STAGE_THRESHOLD_CLEAN_HIGH, BizStage::BIZ_STAGE_THRESHOLD_CLEAN_MEDIUM,
        BizStage::BIZ_STAGE_THRESHOLD_CLEAN_LOW, BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_LOW,
        BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_MEDIUM, BizStage::BIZ_STAGE_THRESHOLD_GET_CCM_PARA,
        BizStage::BIZ_STAGE_USER_MOUNT, BizStage::BIZ_STAGE_MTPFS_MTP_DEVICE,
        BizStage::BIZ_STAGE_NOT_PERMISSION,
    };
    for (const auto &stage : stages) {
        RadarParameter param = {
            .orgPkg = "storageService", .userId = 100, .funcName = "TestStage",
            .bizScene = BizScene::STORAGE_START, .bizStage = stage,
            .keyElxLevel = "NA", .errorCode = E_OK, .extraData = "stage_test", .toCallPkg = "",
        };
        EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFunctionResult(param, eventName));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportStorageUsage_003
 * @tc.desc: Verify ReportStorageUsage with all threshold stages.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportStorageUsage_003, TestSize.Level1)
{
    std::vector<BizStage> stages = {
        BizStage::BIZ_STAGE_THRESHOLD_CLEAN_HIGH,
        BizStage::BIZ_STAGE_THRESHOLD_CLEAN_MEDIUM,
        BizStage::BIZ_STAGE_THRESHOLD_CLEAN_LOW,
        BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_LOW,
        BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_MEDIUM,
        BizStage::BIZ_STAGE_THRESHOLD_GET_CCM_PARA,
    };
    for (const auto &stage : stages) {
        std::string extraData = "threshold_test";
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportStorageUsage(stage, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportVolumeOperation_004
 * @tc.desc: Verify ReportVolumeOperation with all volume operations.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportVolumeOperation_004, TestSize.Level1)
{
    std::vector<std::string> ops = {"Mount", "Unmount", "Format", "Partition", "SetVolumeDescription"};
    for (const auto &op : ops) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportVolumeOperation(op, E_OK));
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportVolumeOperation(op, E_ERR));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportUserKeyResult_004
 * @tc.desc: Verify ReportUserKeyResult with all key levels.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserKeyResult_004, TestSize.Level1)
{
    std::string funcName = "GenerateUserKeys";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::vector<std::string> keyLevels = {"EL1", "EL2", "EL3", "EL4", "EL5", "NA"};
    for (const auto &level : keyLevels) {
        std::string extraData = "key_" + level;
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserKeyResult(funcName, userId, ret, level, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportActiveUserKey_005
 * @tc.desc: Verify ReportActiveUserKey with all key levels.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportActiveUserKey_005, TestSize.Level1)
{
    std::string funcName = "ActiveUserKey";
    uint32_t userId = 100;
    int32_t ret = E_OK;
    std::vector<std::string> keyLevels = {"EL1", "EL2", "EL3", "EL4", "EL5"};
    for (const auto &level : keyLevels) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportActiveUserKey(funcName, userId, ret, level));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportFucBehavior_003
 * @tc.desc: Verify ReportFucBehavior with various userIds.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportFucBehavior_003, TestSize.Level1)
{
    std::string funcName = "TestBehavior";
    std::string extraData = "behavior_test";
    std::vector<uint32_t> userIds = {0, 100, 105, 1000, 0xFFFFFFFF};
    for (const auto &uid : userIds) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFucBehavior(funcName, uid, extraData, E_OK));
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFucBehavior(funcName, uid, extraData, E_ERR));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportGetStorageStatus_003
 * @tc.desc: Verify ReportGetStorageStatus with various userIds.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportGetStorageStatus_003, TestSize.Level1)
{
    std::string funcName = "GetStorageStatus";
    std::string orgPkg = "com.test.app";
    std::vector<uint32_t> userIds = {0, 100, 105, 10737, 0xFFFFFFFF};
    for (const auto &uid : userIds) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportGetStorageStatus(funcName, uid, E_OK, orgPkg));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportUserManager_003
 * @tc.desc: Verify ReportUserManager with various userIds and ret values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUserManager_003, TestSize.Level1)
{
    std::string funcName = "PrepareUserDirs";
    std::vector<uint32_t> userIds = {0, 100, 105, 10737};
    std::vector<int32_t> rets = {E_OK, E_ERR, E_PARAMS_INVALID, E_USERID_RANGE};
    for (const auto &uid : userIds) {
        for (const auto &ret : rets) {
            std::string extraData = "uid_" + std::to_string(uid) + "_ret_" + std::to_string(ret);
            EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUserManager(funcName, uid, ret, extraData));
        }
    }
}

/**
 * @tc.name: StorageRadarTest_ReportUpdateUserAuth_003
 * @tc.desc: Verify ReportUpdateUserAuth with all key levels.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportUpdateUserAuth_003, TestSize.Level1)
{
    std::string funcName = "UpdateUserAuth";
    uint32_t userId = 100;
    std::string extraData = "auth_test";
    std::vector<std::string> keyLevels = {"EL1", "EL2", "EL3", "EL4", "EL5", "NA"};
    for (const auto &level : keyLevels) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUpdateUserAuth(funcName, userId, E_OK, level, extraData));
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportUpdateUserAuth(funcName, userId, E_ERR, level, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportFbexResult_003
 * @tc.desc: Verify ReportFbexResult with various ret values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportFbexResult_003, TestSize.Level1)
{
    std::string funcName = "FbexOp";
    uint32_t userId = 100;
    std::string keyLevel = "EL2";
    std::string extraData = "fbex_test";
    std::vector<int32_t> rets = {E_OK, E_ERR, -1, -100, 100};
    for (const auto &ret : rets) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportFbexResult(funcName, userId, ret, keyLevel, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportCommonResult_003
 * @tc.desc: Verify ReportCommonResult with various funcNames.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportCommonResult_003, TestSize.Level1)
{
    int32_t ret = E_OK;
    unsigned int userId = 100;
    std::string extraData = "common_test";
    std::vector<std::string> funcNames = {"Func1", "Func2", "", "VeryLongFunctionNameForTest"};
    for (const auto &name : funcNames) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportCommonResult(name, ret, userId, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportIamResult_003
 * @tc.desc: Verify ReportIamResult with various userIds.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportIamResult_003, TestSize.Level1)
{
    std::string funcName = "IamAuth";
    std::vector<uint32_t> userIds = {0, 100, 105, 10737};
    for (const auto &uid : userIds) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportIamResult(funcName, uid, E_OK));
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportIamResult(funcName, uid, E_ERR));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportHuksResult_003
 * @tc.desc: Verify ReportHuksResult with various ret values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportHuksResult_003, TestSize.Level1)
{
    std::string funcName = "HuksOp";
    std::vector<int32_t> rets = {E_OK, E_ERR, -1, -100, 1, 100};
    for (const auto &ret : rets) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportHuksResult(funcName, ret));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportMtpResult_003
 * @tc.desc: Verify ReportMtpResult with various funcNames and rets.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportMtpResult_003, TestSize.Level1)
{
    std::string extraData = "mtp_test";
    std::vector<std::string> funcNames = {"MtpMount", "MtpUnmount", ""};
    std::vector<int32_t> rets = {E_OK, E_ERR};
    for (const auto &name : funcNames) {
        for (const auto &ret : rets) {
            EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportMtpResult(name, ret, extraData));
        }
    }
}

/**
 * @tc.name: StorageRadarTest_ReportKeyRingResult_003
 * @tc.desc: Verify ReportKeyRingResult with various ret values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportKeyRingResult_003, TestSize.Level1)
{
    std::string funcName = "KeyRingOp";
    std::string extraData = "keyring_test";
    std::vector<int32_t> rets = {E_OK, E_ERR, -1, 1};
    for (const auto &ret : rets) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportKeyRingResult(funcName, ret, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportOsAccountResult_003
 * @tc.desc: Verify ReportOsAccountResult with various funcNames.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportOsAccountResult_003, TestSize.Level1)
{
    int32_t ret = E_OK;
    unsigned int userId = 100;
    std::vector<std::string> funcNames = {"OsAccountCreate", "OsAccountRemove", ""};
    for (const auto &name : funcNames) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportOsAccountResult(name, ret, userId));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportEl5KeyMgrResult_003
 * @tc.desc: Verify ReportEl5KeyMgrResult with various ret values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportEl5KeyMgrResult_003, TestSize.Level1)
{
    std::string funcName = "El5KeyMgr";
    unsigned int userId = 100;
    std::string extraData = "el5_test";
    std::vector<int32_t> rets = {E_OK, E_ERR, -1, 1};
    for (const auto &ret : rets) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportEl5KeyMgrResult(funcName, ret, userId, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_ReportBundleMgrResult_003
 * @tc.desc: Verify ReportBundleMgrResult with various userIds.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_ReportBundleMgrResult_003, TestSize.Level1)
{
    std::string funcName = "GetBundleStats";
    int32_t ret = E_OK;
    std::string extraData = "bundle_test";
    std::vector<unsigned int> userIds = {0, 100, 105, 10737};
    for (const auto &uid : userIds) {
        EXPECT_NO_FATAL_FAILURE(StorageRadar::ReportBundleMgrResult(funcName, ret, uid, extraData));
    }
}

/**
 * @tc.name: StorageRadarTest_RecordStorageStatusResult_003
 * @tc.desc: Verify RecordStorageStatusResult with various orgPkg values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordStorageStatusResult_003, TestSize.Level1)
{
    std::string eventName = FILE_STORAGE_STATUS_STATISTIC;
    std::vector<std::string> orgPkgs = {"com.test.app", "storageService", "", "very_long_package_name_test"};
    for (const auto &pkg : orgPkgs) {
        RadarParameter param = {
            .orgPkg = pkg,
            .userId = 0,
            .extraData = "status_test",
        };
        EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordStorageStatusResult(param, eventName));
    }
}

/**
 * @tc.name: StorageRadarTest_RecordFaultResult_003
 * @tc.desc: Verify RecordFaultResult with various BizScene values.
 * @tc.type: FUNC
 */
HWTEST_F(StorageRadarTest, StorageRadarTest_RecordFaultResult_003, TestSize.Level1)
{
    std::string eventName = FILE_STORAGE_STATUS_FAULT;
    for (int32_t i = 0; i <= static_cast<int32_t>(BizScene::MTP_DEVICE_MANAGER); i++) {
        RadarParameter param = {
            .orgPkg = "storageService",
            .userId = 100,
            .funcName = "FaultTest",
            .bizScene = static_cast<BizScene>(i),
            .bizStage = BizStage::BIZ_STAGE_MOUNT,
            .keyElxLevel = "NA",
            .errorCode = E_ERR,
            .extraData = "fault_scene_test",
            .toCallPkg = "",
        };
        EXPECT_NO_FATAL_FAILURE(StorageRadar::GetInstance().RecordFaultResult(param, eventName));
    }
}
} // namespace Test
} // namespace StorageService
} // namespace OHOS
