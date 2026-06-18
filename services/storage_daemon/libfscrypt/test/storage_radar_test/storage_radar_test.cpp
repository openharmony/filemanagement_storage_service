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
#include "utils/storage_radar.h"

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

HWTEST_F(StorageRadarTest, StorageRadar_ReportStorageStatusRadar_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_001 start";
    std::string funcName = "TestFunc";
    std::string extraData = "test_extra_data";
    StorageService::StorageRadar::ReportStorageStatusRadar(funcName, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportStorageStatusRadar_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_002 start";
    std::string funcName = "";
    std::string extraData = "";
    StorageService::StorageRadar::ReportStorageStatusRadar(funcName, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordStorageStatusResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_001 start";
    StorageService::RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 0,
        .funcName = "TestFunc",
        .extraData = "test_data"
    };
    std::string eventName = "FILE_STORAGE_STATUS_STATISTIC";
    bool result = StorageService::StorageRadar::GetInstance().RecordStorageStatusResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_001 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordStorageStatusResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_002 start";
    StorageService::RadarParameter param = {
        .orgPkg = "",
        .userId = -1,
        .funcName = "",
        .extraData = ""
    };
    std::string eventName = "FILE_STORAGE_STATUS_STATISTIC";
    bool result = StorageService::StorageRadar::GetInstance().RecordStorageStatusResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_002 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordStorageStatusResult_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_003 start";
    StorageService::RadarParameter param = {
        .orgPkg = "test_org",
        .userId = 100,
        .funcName = "LongFunctionNameTest",
        .extraData = "long_extra_data_content_test"
    };
    std::string eventName = "TEST_EVENT";
    bool result = StorageService::StorageRadar::GetInstance().RecordStorageStatusResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_003 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordStorageStatusResult_003 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportActiveUserKey_001 start";
    std::string funcName = "ActiveUserKey";
    uint32_t userId = 100;
    int ret = 0;
    std::string keyElxLevel = "ECE";
    StorageService::StorageRadar::ReportActiveUserKey(funcName, userId, ret, keyElxLevel);
    GTEST_LOG_(INFO) << "StorageRadar_ReportActiveUserKey_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportActiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportActiveUserKey_002 start";
    std::string funcName = "";
    uint32_t userId = 0;
    int ret = -1;
    std::string keyElxLevel = "";
    StorageService::StorageRadar::ReportActiveUserKey(funcName, userId, ret, keyElxLevel);
    GTEST_LOG_(INFO) << "StorageRadar_ReportActiveUserKey_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportGetStorageStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportGetStorageStatus_001 start";
    std::string funcName = "GetStorageStatus";
    uint32_t userId = 100;
    int ret = 0;
    std::string orgPkg = "test_pkg";
    StorageService::StorageRadar::ReportGetStorageStatus(funcName, userId, ret, orgPkg);
    GTEST_LOG_(INFO) << "StorageRadar_ReportGetStorageStatus_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportVolumeOperation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportVolumeOperation_001 start";
    std::string funcName = "MountVolume";
    int ret = 0;
    StorageService::StorageRadar::ReportVolumeOperation(funcName, ret);
    GTEST_LOG_(INFO) << "StorageRadar_ReportVolumeOperation_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportVolumeOperation_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportVolumeOperation_002 start";
    std::string funcName = "UnmountVolume";
    int ret = -1;
    StorageService::StorageRadar::ReportVolumeOperation(funcName, ret);
    GTEST_LOG_(INFO) << "StorageRadar_ReportVolumeOperation_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportUserKeyResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportUserKeyResult_001 start";
    std::string funcName = "GenerateUserKeys";
    uint32_t userId = 100;
    int ret = 0;
    std::string keyElxLevel = "ECE";
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportUserKeyResult(funcName, userId, ret, keyElxLevel, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportUserKeyResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportUserManager_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportUserManager_001 start";
    std::string funcName = "CreateUser";
    uint32_t userId = 100;
    int ret = 0;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportUserManager(funcName, userId, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportUserManager_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportSaSizeResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportSaSizeResult_001 start";
    std::string funcName = "GetSaSize";
    int ret = 0;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportSaSizeResult(funcName, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportSaSizeResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportSpaceRadar_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportSpaceRadar_001 start";
    std::string funcName = "GetSpace";
    int ret = 0;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportSpaceRadar(funcName, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportSpaceRadar_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportUpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportUpdateUserAuth_001 start";
    std::string funcName = "UpdateUserAuth";
    uint32_t userId = 100;
    int ret = 0;
    std::string keyLevel = "ECE";
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportUpdateUserAuth(funcName, userId, ret, keyLevel, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportUpdateUserAuth_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportFbexResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportFbexResult_001 start";
    std::string funcName = "FbexOperation";
    uint32_t userId = 100;
    int ret = 0;
    std::string keyLevel = "ECE";
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportFbexResult(funcName, userId, ret, keyLevel, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportFbexResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportIamResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportIamResult_001 start";
    std::string funcName = "IamOperation";
    uint32_t userId = 100;
    int ret = 0;
    StorageService::StorageRadar::ReportIamResult(funcName, userId, ret);
    GTEST_LOG_(INFO) << "StorageRadar_ReportIamResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportHuksResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportHuksResult_001 start";
    std::string funcName = "HuksOperation";
    int ret = 0;
    StorageService::StorageRadar::ReportHuksResult(funcName, ret);
    GTEST_LOG_(INFO) << "StorageRadar_ReportHuksResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportMtpResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportMtpResult_001 start";
    std::string funcName = "MtpOperation";
    int ret = 0;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportMtpResult(funcName, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportMtpResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportKeyRingResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportKeyRingResult_001 start";
    std::string funcName = "KeyRingOperation";
    int ret = 0;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportKeyRingResult(funcName, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportKeyRingResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportOsAccountResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportOsAccountResult_001 start";
    std::string funcName = "OsAccountOperation";
    int32_t ret = 0;
    unsigned int userId = 100;
    StorageService::StorageRadar::ReportOsAccountResult(funcName, ret, userId);
    GTEST_LOG_(INFO) << "StorageRadar_ReportOsAccountResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportEl5KeyMgrResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportEl5KeyMgrResult_001 start";
    std::string funcName = "El5KeyMgrOperation";
    int32_t ret = 0;
    unsigned int userId = 100;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportEl5KeyMgrResult(funcName, ret, userId, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportEl5KeyMgrResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportTEEClientResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportTEEClientResult_001 start";
    std::string funcName = "TEEClientOperation";
    int32_t ret = 0;
    unsigned int userId = 100;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportTEEClientResult(funcName, ret, userId, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportTEEClientResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportCommonResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportCommonResult_001 start";
    std::string funcName = "CommonOperation";
    int32_t ret = 0;
    unsigned int userId = 100;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportCommonResult(funcName, ret, userId, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportCommonResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportBundleMgrResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportBundleMgrResult_001 start";
    std::string funcName = "BundleMgrOperation";
    int32_t ret = 0;
    unsigned int userId = 100;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportBundleMgrResult(funcName, ret, userId, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportBundleMgrResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportStorageUsage_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageUsage_001 start";
    StorageService::BizStage stage = StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_HIGH;
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportStorageUsage(stage, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageUsage_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordFunctionResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordFunctionResult_001 start";
    StorageService::RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 100,
        .funcName = "TestFunc",
        .bizScene = StorageService::BizScene::STORAGE_START,
        .bizStage = StorageService::BizStage::BIZ_STAGE_SA_START,
        .keyElxLevel = "NA",
        .errorCode = 0,
        .extraData = "test_data",
        .toCallPkg = "test_pkg"
    };
    std::string eventName = "FILE_STORAGE_MANAGER_FAULT";
    bool result = StorageService::StorageRadar::GetInstance().RecordFunctionResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordFunctionResult_001 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordFunctionResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordFunctionResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordFunctionResult_002 start";
    StorageService::RadarParameter param = {
        .orgPkg = "",
        .userId = -1,
        .funcName = "",
        .bizScene = StorageService::BizScene::STORAGE_START,
        .bizStage = StorageService::BizStage::BIZ_STAGE_SA_START,
        .keyElxLevel = "",
        .errorCode = -1,
        .extraData = "",
        .toCallPkg = ""
    };
    std::string eventName = "FILE_STORAGE_MANAGER_FAULT";
    bool result = StorageService::StorageRadar::GetInstance().RecordFunctionResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordFunctionResult_002 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordFunctionResult_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordCurrentTime_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordCurrentTime_001 start";
    int64_t time1 = StorageService::StorageRadar::RecordCurrentTime();
    EXPECT_TRUE(time1 > 0);
    int64_t time2 = StorageService::StorageRadar::RecordCurrentTime();
    EXPECT_TRUE(time2 >= time1);
    GTEST_LOG_(INFO) << "StorageRadar_RecordCurrentTime_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportDuration_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportDuration_001 start";
    std::string funcName = "TestFunc";
    int64_t startTime = StorageService::StorageRadar::RecordCurrentTime();
    std::string result = StorageService::StorageRadar::ReportDuration(funcName, startTime, 20, 100);
    GTEST_LOG_(INFO) << "StorageRadar_ReportDuration_001 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_ReportDuration_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportDuration_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportDuration_002 start";
    std::string funcName = "";
    int64_t startTime = 0;
    std::string result = StorageService::StorageRadar::ReportDuration(funcName, startTime, 20, 100);
    GTEST_LOG_(INFO) << "StorageRadar_ReportDuration_002 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_ReportDuration_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportSetQuotaByBaseline_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportSetQuotaByBaseline_001 start";
    std::string funcName = "SetQuotaByBaseline";
    std::string extraData = "test_data";
    StorageService::StorageRadar::ReportSetQuotaByBaseline(funcName, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportSetQuotaByBaseline_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportFucBehavior_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportFucBehavior_001 start";
    std::string funcName = "TestFunc";
    uint32_t userId = 100;
    std::string extraData = "test_data";
    int32_t ret = 0;
    StorageService::StorageRadar::ReportFucBehavior(funcName, userId, extraData, ret);
    GTEST_LOG_(INFO) << "StorageRadar_ReportFucBehavior_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportFucBehavior_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportFucBehavior_002 start";
    std::string funcName = "";
    uint32_t userId = 0;
    std::string extraData = "";
    int32_t ret = -1;
    StorageService::StorageRadar::ReportFucBehavior(funcName, userId, extraData, ret);
    GTEST_LOG_(INFO) << "StorageRadar_ReportFucBehavior_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordFaultResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_001 start";
    StorageService::RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 100,
        .funcName = "GetStorageStatus",
        .bizScene = StorageService::BizScene::SPACE_STATISTICS,
        .bizStage = StorageService::BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS,
        .errorCode = -1,
        .extraData = "storage_status_failed",
        .toCallPkg = "test_pkg"
    };
    std::string eventName = "FILE_STORAGE_STATUS_FAULT";
    bool result = StorageService::StorageRadar::GetInstance().RecordFaultResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_001 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_001 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordFaultResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_002 start";
    StorageService::RadarParameter param = {
        .orgPkg = "",
        .userId = -1,
        .funcName = "",
        .bizScene = StorageService::BizScene::SPACE_STATISTICS,
        .bizStage = StorageService::BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS,
        .errorCode = -1,
        .extraData = "",
        .toCallPkg = ""
    };
    std::string eventName = "FILE_STORAGE_STATUS_FAULT";
    bool result = StorageService::StorageRadar::GetInstance().RecordFaultResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_002 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_002 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_RecordFaultResult_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_003 start";
    StorageService::RadarParameter param = {
        .orgPkg = "storageService",
        .userId = 0,
        .funcName = "GetUserStorageStats",
        .bizScene = StorageService::BizScene::SPACE_STATISTICS,
        .bizStage = StorageService::BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS,
        .errorCode = 0,
        .extraData = "test_extra_data_content",
        .toCallPkg = "test_to_call_pkg"
    };
    std::string eventName = "FILE_STORA";
    bool result = StorageService::StorageRadar::GetInstance().RecordFaultResult(param, eventName);
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_003 result: " << result;
    GTEST_LOG_(INFO) << "StorageRadar_RecordFaultResult_003 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportStorageStatusRadar_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_003 start";
    std::string funcName = "GetStorageStatus";
    int ret = -1;
    std::string extraData = "storage_status_failed";
    StorageService::StorageRadar::ReportStorageStatusRadar(funcName, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_003 end";
}

HWTEST_F(StorageRadarTest, StorageRadar_ReportStorageStatusRadar_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_004 start";
    std::string funcName = "";
    int ret = 0;
    std::string extraData = "";
    StorageService::StorageRadar::ReportStorageStatusRadar(funcName, ret, extraData);
    GTEST_LOG_(INFO) << "StorageRadar_ReportStorageStatusRadar_004 end";
}

} // OHOS::StorageDaemon
