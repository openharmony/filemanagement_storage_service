/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <gtest/gtest.h>

#include "mock/config_policy_utils.h"
#include "storage/storage_quota_controller.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "parameter.h"
#include <fstream>
#include <string>
#include <vector>
#include "cJSON.h"

namespace {
char g_cfgName[] { "/system/etc/storage_statistic_baseline.json" };
using namespace std;
using namespace OHOS;
using namespace StorageManager;
using namespace testing::ext;
using namespace testing;

class StorageQuotaControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::string configPath = "/system/etc/storage_statistic_baseline.json";
        std::string backupPath = "/system/etc/storage_statistic_baseline.json.bak";
        if (access(configPath.c_str(), F_OK) == 0) {
            std::ifstream src(configPath, std::ios::binary);
            std::ofstream dst(backupPath, std::ios::binary);
            dst << src.rdbuf();
        }
    };
    static void TearDownTestCase()
    {
        std::string configPath = "/system/etc/storage_statistic_baseline.json";
        std::string backupPath = "/system/etc/storage_statistic_baseline.json.bak";
        if (access(backupPath.c_str(), F_OK) == 0) {
            std::ifstream src(backupPath, std::ios::binary);
            std::ofstream dst(configPath, std::ios::binary);
            dst << src.rdbuf();
            std::remove(backupPath.c_str());
        }
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: StorageQuotaControllerTest_UpdateBaseLineByUid_0000
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageQuotaControllerTest, StorageQuotaControllerTest_UpdateBaseLineByUid_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest-begin UpdateBaseLineByUid_0000";
    std::string configPath = "/system/etc/storage_statistic_baseline.json";
    std::string backupPath = "/system/etc/storage_statistic_baseline.json.bak";
    if (access(configPath.c_str(), F_OK) == 0) {
        std::ifstream src(configPath, std::ios::binary);
        std::ofstream dst(backupPath, std::ios::binary);
        dst << src.rdbuf();
    }
    std::ofstream ofs4(configPath);
    ofs4 << R"({"storage.statistic.baseline":[{"uid":0,"baseline":0}]})";
    ofs4.close();
    NiceMock<ConfigPolicyUtilsMock> cfgPolicyUtils;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    EXPECT_NO_FATAL_FAILURE(StorageQuotaController::GetInstance().UpdateBaseLineByUid());
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(nullptr));
    EXPECT_NO_FATAL_FAILURE(StorageQuotaController::GetInstance().UpdateBaseLineByUid());
    std::remove(configPath.c_str());

    if (access(backupPath.c_str(), F_OK) == 0) {
        std::ifstream src(backupPath, std::ios::binary);
        std::ofstream dst(configPath, std::ios::binary);
        dst << src.rdbuf();
        std::remove(backupPath.c_str());
    }
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest-end UpdateBaseLineByUid_0000";
}

/**
 * @tc.name: StorageQuotaControllerTest_ReadCcmConfigFile_0000
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageQuotaControllerTest, StorageQuotaControllerTest_ReadCcmConfigFile_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest-begin ReadCcmConfigFile_0000";
    std::string configPath = "/system/etc/storage_statistic_baseline.json";
    std::string backupPath = "/system/etc/storage_statistic_baseline.json.bak";
    if (access(configPath.c_str(), F_OK) == 0) {
        std::ifstream src(configPath, std::ios::binary);
        std::ofstream dst(backupPath, std::ios::binary);
        dst << src.rdbuf();
    }
    std::ofstream ofs4(configPath);
    ofs4 << R"({"storage.statistic.baseline":[{"uid":0,"baseline":0}]})";
    ofs4.close();
    std::vector<BaselineCfg> params;
    NiceMock<ConfigPolicyUtilsMock> cfgPolicyUtils;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    int32_t ret = StorageQuotaController::GetInstance().ReadCcmConfigFile(params);
    EXPECT_EQ(ret, E_OK);
    std::remove(configPath.c_str());

    if (access(backupPath.c_str(), F_OK) == 0) {
        std::ifstream src(backupPath, std::ios::binary);
        std::ofstream dst(configPath, std::ios::binary);
        dst << src.rdbuf();
        std::remove(backupPath.c_str());
    }
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest-end ReadCcmConfigFile_0000";
}

/**
 * @tc.name: StorageQuotaControllerTest_ReadCcmConfigFile_0001
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageQuotaControllerTest, StorageQuotaControllerTest_ReadCcmConfigFile_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest_ReadCcmConfigFile_0001 start";

    std::string configPath = "/system/etc/storage_statistic_baseline.json";
    std::string backupPath = "/system/etc/storage_statistic_baseline.json.bak";
    if (access(configPath.c_str(), F_OK) == 0) {
        std::ifstream src(configPath, std::ios::binary);
        std::ofstream dst(backupPath, std::ios::binary);
        dst << src.rdbuf();
    }
    std::vector<BaselineCfg> params1;
    NiceMock<ConfigPolicyUtilsMock> cfgPolicyUtils;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(nullptr));
    int32_t ret0 = StorageQuotaController::GetInstance().ReadCcmConfigFile(params1);
    EXPECT_EQ(ret0, E_PARAMS_INVALID);

    std::remove(configPath.c_str());
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    int32_t ret1 = StorageQuotaController::GetInstance().ReadCcmConfigFile(params1);
    EXPECT_EQ(ret1, E_PARAMS_INVALID);

    std::ofstream ofs(configPath);
    ofs << "";
    ofs.close();
    std::vector<BaselineCfg> params2;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    int32_t ret2 = StorageQuotaController::GetInstance().ReadCcmConfigFile(params2);
    EXPECT_EQ(ret2, E_PARAMS_INVALID);
    std::remove(configPath.c_str());

    if (access(backupPath.c_str(), F_OK) == 0) {
        std::ifstream src(backupPath, std::ios::binary);
        std::ofstream dst(configPath, std::ios::binary);
        dst << src.rdbuf();
        std::remove(backupPath.c_str());
    }
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest_ReadCcmConfigFile_0001 end";
}

/**
 * @tc.name: StorageQuotaControllerTest_ReadCcmConfigFile_0002
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageQuotaControllerTest, StorageQuotaControllerTest_ReadCcmConfigFile_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest_ReadCcmConfigFile_0002 start";

    std::string configPath = "/system/etc/storage_statistic_baseline.json";
    std::string backupPath = "/system/etc/storage_statistic_baseline.json.bak";
    if (access(configPath.c_str(), F_OK) == 0) {
        std::ifstream src(configPath, std::ios::binary);
        std::ofstream dst(backupPath, std::ios::binary);
        dst << src.rdbuf();
    }

    std::ofstream ofs2(configPath);
    ofs2 << "not a json";
    ofs2.close();
    std::vector<BaselineCfg> params3;
    NiceMock<ConfigPolicyUtilsMock> cfgPolicyUtils;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    int32_t ret3 = StorageQuotaController::GetInstance().ReadCcmConfigFile(params3);
    EXPECT_EQ(ret3, E_PARAMS_INVALID);
    std::remove(configPath.c_str());

    std::ofstream ofs3(configPath);
    ofs3 << R"({"otherfield":[{"uid":1000,"baseline":1024}]})";
    ofs3.close();
    std::vector<BaselineCfg> params4;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    int32_t ret4 = StorageQuotaController::GetInstance().ReadCcmConfigFile(params4);
    EXPECT_EQ(ret4, E_PARAMS_INVALID);
    std::remove(configPath.c_str());

    std::ofstream ofs4(configPath);
    ofs4 << R"({"storage.statistic.baseline":[{"uid":-10,"baseline":1024}]})";
    ofs4.close();
    std::vector<BaselineCfg> params5;
    EXPECT_CALL(cfgPolicyUtils, GetOneCfgFile).WillOnce(testing::Return(g_cfgName));
    int32_t ret5 = StorageQuotaController::GetInstance().ReadCcmConfigFile(params5);
    EXPECT_EQ(ret5, E_OK);
    std::remove(configPath.c_str());

    if (access(backupPath.c_str(), F_OK) == 0) {
        std::ifstream src(backupPath, std::ios::binary);
        std::ofstream dst(configPath, std::ios::binary);
        dst << src.rdbuf();
        std::remove(backupPath.c_str());
    }
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest_ReadCcmConfigFile_0002 end";
}

/**
 * @tc.name: StorageQuotaControllerTest_SaveUidAndBaseLine_0000
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageQuotaControllerTest, StorageQuotaControllerTest_SaveUidAndBaseLine_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest-begin SaveUidAndBaseLine_0000";
    std::vector<BaselineCfg> params;
    int32_t ret = StorageQuotaController::GetInstance().SaveUidAndBaseLine(nullptr, params);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageQuotaControllerTest-end UpdateBaseLineByUid_0000";
}
}
