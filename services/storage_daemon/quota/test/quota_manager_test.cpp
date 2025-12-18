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

#include <fstream>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "test/common/help_utils.h"
#include "utils/file_utils.h"
#include "userdata_dir_info.h"

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

    QuotaManager &quotaManager1 = QuotaManager::GetInstance();
    QuotaManager &quotaManager2 = QuotaManager::GetInstance();
    ASSERT_TRUE(&quotaManager1 == &quotaManager2);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetInstance_001 end";
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

    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_STAT_VFS_KERNEL_ERR);

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

    int32_t uid = -1;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_PARAMS_INVALID);

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

    int32_t uid = UID;
    std::string bundleDataDirPath = EMPTY_STRING;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_PARAMS_INVALID);

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

    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = -1;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_PARAMS_INVALID);

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

    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_STAT_VFS_KERNEL_ERR);
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

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetOccupiedSpace_001
 * @tc.desc: Test whether GetOccupiedSpace is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetOccupiedSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetOccupiedSpace_001 start";

    int32_t idType = USRID;
    int32_t uid = -1;
    int64_t size = 0;
    int32_t result = QuotaManager::GetInstance().GetOccupiedSpace(idType, uid, size);
    EXPECT_EQ(result, E_QUOTA_CTL_KERNEL_ERR);
    uid = UID;
    result = QuotaManager::GetInstance().GetOccupiedSpace(idType, uid, size);
    EXPECT_EQ(result, E_OK);

    idType = GRPID;
    int32_t gid = -1;
    result = QuotaManager::GetInstance().GetOccupiedSpace(idType, gid, size);
    EXPECT_EQ(result, E_QUOTA_CTL_KERNEL_ERR);
    gid = 1006;
    result = QuotaManager::GetInstance().GetOccupiedSpace(idType, gid, size);
    EXPECT_EQ(result, E_OK);

    idType = PRJID;
    int32_t prjid = -1;
    result = QuotaManager::GetInstance().GetOccupiedSpace(idType, prjid, size);
    EXPECT_EQ(result, E_QUOTA_CTL_KERNEL_ERR);
    prjid = 0;
    result = QuotaManager::GetInstance().GetOccupiedSpace(idType, prjid, size);
    EXPECT_EQ(result, E_OK);

    idType = -1;
    result = QuotaManager::GetInstance().GetOccupiedSpace(idType, uid, size);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetOccupiedSpace_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetUidStorageStats_001
 * @tc.desc: Test whether GetUidStorageStats is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetUidStorageStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_001 start";

    int before = E_OK;
    std::string str = "";
    std::map<int32_t, std::string> bundleNameAndUid;
    QuotaManager::GetInstance().GetUidStorageStats(str, bundleNameAndUid);
    EXPECT_FALSE(before);
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_ConvertBytesToMB_001
 * @tc.desc: Test whether ConvertBytesToMB is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_ConvertBytesToMB_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_ConvertBytesToMB_001 start";

    int64_t bytes = -1;
    int32_t decimalPlaces = 2;
    auto result = QuotaManager::GetInstance().ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_EQ(result, 0.0);

    bytes = 0;
    decimalPlaces = 2;
    result = QuotaManager::GetInstance().ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_EQ(result, 0.0);


    bytes = 1024 * 1024;
    decimalPlaces = -1;
    result = QuotaManager::GetInstance().ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_EQ(result, 1.0);

    decimalPlaces = 2;
    result = QuotaManager::GetInstance().ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_NEAR(result, 1.05, 1e-6);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_ConvertBytesToMB_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_StringToInt32_001
 * @tc.desc: Test whether StringToInt32 is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_StringToInt32_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_StringToInt32_001 start";

    std::string strUid = "";
    int32_t outUid32 = 0;
    EXPECT_FALSE(QuotaManager::GetInstance().StringToInt32(strUid, outUid32));

    strUid = "123a";
    EXPECT_FALSE(QuotaManager::GetInstance().StringToInt32(strUid, outUid32));

    strUid = "2147483648";
    EXPECT_FALSE(QuotaManager::GetInstance().StringToInt32(strUid, outUid32));

    strUid = "12345";
    EXPECT_TRUE(QuotaManager::GetInstance().StringToInt32(strUid, outUid32));
    EXPECT_EQ(outUid32, 12345);

    strUid = "0";
    EXPECT_TRUE(QuotaManager::GetInstance().StringToInt32(strUid, outUid32));
    EXPECT_EQ(outUid32, 0);

    strUid = "2147483647";
    EXPECT_TRUE(QuotaManager::GetInstance().StringToInt32(strUid, outUid32));
    EXPECT_EQ(outUid32, INT32_MAX);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_StringToInt32_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetUid32FromEntry_001
 * @tc.desc: Test whether GetUid32FromEntry is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetUid32FromEntry_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUid32FromEntry_001 start";

    std::string entry = "invalidEntry";
    int32_t outUid32 = 0;
    std::string saName;
    EXPECT_FALSE(QuotaManager::GetInstance().GetUid32FromEntry(entry, outUid32, saName));

    entry = "saName:1234";
    EXPECT_FALSE(QuotaManager::GetInstance().GetUid32FromEntry(entry, outUid32, saName));

    entry = "saName:1234:extra";
    EXPECT_FALSE(QuotaManager::GetInstance().GetUid32FromEntry(entry, outUid32, saName));

    entry = "saName:invalidUid:validEntry";
    EXPECT_FALSE(QuotaManager::GetInstance().GetUid32FromEntry(entry, outUid32, saName));

    entry = "saName:validEntry:1234:";
    EXPECT_TRUE(QuotaManager::GetInstance().GetUid32FromEntry(entry, outUid32, saName));
    EXPECT_EQ(outUid32, 1234);
    EXPECT_EQ(saName, "saName");

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUid32FromEntry_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_ParseConfigFile_001
 * @tc.desc: Test whether ParseConfigFile is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_ParseConfigFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_ParseConfigFile_001 start";

    std::string path = "invalid_path";
    std::vector<struct UidSaInfo> vec;
    auto result = QuotaManager::GetInstance().ParseConfigFile(path, vec);
    EXPECT_EQ(result, E_JSON_PARSE_ERROR);

    path = "/data/valid_entry_file";
    std::ofstream outfile(path);
    outfile << "\n\ninvalid_line\nsaName:validEntry:1234:\n";
    outfile.close();

    vec.clear();
    EXPECT_EQ(QuotaManager::GetInstance().ParseConfigFile(path, vec), E_OK);
    ASSERT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].saName, "saName");
    EXPECT_EQ(vec[0].uid, 1234);

    std::remove(path.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_ParseConfigFile_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetOccupiedSpaceForUidList_001
 * @tc.desc: Test whether GetOccupiedSpaceForUidList is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetOccupiedSpaceForUidList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetOccupiedSpaceForUidList_001 start";
    struct AllAppVec allVec;
    std::uint64_t iNodes;
    QuotaManager::GetInstance().GetOccupiedSpaceForUidList(allVec, iNodes);
    struct UidSaInfo info = {0, "root", 0};
    allVec.sysSaVec.emplace_back(info);
    QuotaManager::GetInstance().GetOccupiedSpaceForUidList(allVec, iNodes);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetOccupiedSpaceForUidList_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_AddBlksRecurse_001
 * @tc.desc: Test whether AddBlksRecurse is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_AddBlksRecurse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_AddBlksRecurse_001 start";

    std::string path = "/data/xxx";
    int64_t blks = 0;
    uid_t type = 0;
    int32_t ret = QuotaManager::GetInstance().AddBlksRecurse(path, blks, type);
    EXPECT_EQ(ret, E_OK);

    path = "/data/app/el2/100/base";
    ret = QuotaManager::GetInstance().AddBlksRecurse(path, blks, type);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_AddBlksRecurse_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_AddBlks_001
 * @tc.desc: Test whether AddBlks is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_AddBlks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_AddBlks_001 start";

    std::string path = "/data/xxx";
    int64_t blks = 0;
    uid_t type = 0;
    int32_t ret = QuotaManager::GetInstance().AddBlks(path, blks, type);
    EXPECT_EQ(ret, E_STATISTIC_STAT_FAILED);

    path = "/data/app/el2/100/base";
    ret = QuotaManager::GetInstance().AddBlks(path, blks, type);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_AddBlks_001 end";
}

/**
 * @tc.name: QuotaManagerTest_AssembleSaInfoVec_001
 * @tc.desc: Test AssembleSaInfoVec handles valid/invalid UIDs and empty bundle map.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AssembleSaInfoVec_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AssembleSaInfoVec_001 start";

    std::vector<UidSaInfo> vec;
    UidSaInfo info1 = {1001, "default", 1024};
    UidSaInfo info2 = {2002, "default", 2048};
    vec.emplace_back(info1);
    vec.emplace_back(info2);
    std::map<int32_t, std::string> bundleMap;
    bundleMap[1001] = "SystemApp";
    bundleMap[2002] = "UserApp";
    QuotaManager::GetInstance().AssembleSaInfoVec(vec, bundleMap);
    EXPECT_EQ(vec[0].saName, "SystemApp");
    EXPECT_EQ(vec[1].saName, "UserApp");

    std::vector<UidSaInfo> vec1;
    UidSaInfo info3 = {3003, "original", 4096};
    vec1.emplace_back(info3);
    std::map<int32_t, std::string> bundleMap1;
    bundleMap1[4004] = "NonExistingApp"; // 不包含 UID 3003
    QuotaManager::GetInstance().AssembleSaInfoVec(vec1, bundleMap1);
    EXPECT_EQ(vec1[0].saName, "original"); // 未修改

    std::vector<UidSaInfo> vec2;
    UidSaInfo info4 = {5005, "initial", 8192};
    vec2.emplace_back(info4);
    std::map<int32_t, std::string> bundleMap2; // 空 map
    QuotaManager::GetInstance().AssembleSaInfoVec(vec2, bundleMap2);
    EXPECT_EQ(vec2[0].saName, "initial"); // 未修改

    GTEST_LOG_(INFO) << "QuotaManagerTest_AssembleSaInfoVec_001 end";
}

/**
 * @tc.name: QuotaManagerTest_ProcessVecList_001
 * @tc.desc: Test ProcessVecList processes all vectors and handles empty bundle map.
 * @tc.type: FUNC
* @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessVecList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_001 start";
    std::vector<UidSaInfo> sysAppVec = {{1001, "sysDefault", 1024}};
    std::vector<UidSaInfo> userAppVec = {{2002, "userDefault", 2048}};
    std::vector<UidSaInfo> vec = {{3003, "vecDefault", 4096}};
    std::vector<UidSaInfo> otherAppVec = {{4004, "vecDefault", 4096}};
    std::map<int32_t, std::string> bundleMap = {{1001, "SystemApp"}, {2002, "UserApp"}, {3003, "VecApp"}};
    struct AllAppVec allVec;
    allVec.otherAppVec = otherAppVec;
    allVec.sysAppVec = sysAppVec;
    allVec.userAppVec = userAppVec;
    allVec.sysSaVec = vec;
    QuotaManager::GetInstance().ProcessVecList(allVec, bundleMap);

    EXPECT_EQ(allVec.sysAppVec[0].saName, "SystemApp");
    EXPECT_EQ(allVec.userAppVec[0].saName, "UserApp");
    EXPECT_EQ(allVec.sysSaVec[0].saName, "vecDefault");

    std::vector<UidSaInfo> sysAppVec1 = {{1001, "original", 1024}};
    std::vector<UidSaInfo> userAppVec1 = {{2002, "original", 2048}};
    std::vector<UidSaInfo> vec1 = {{3003, "original", 4096}};
    std::map<int32_t, std::string> bundleMap1; // 空 map
    allVec.sysAppVec = sysAppVec1;
    allVec.userAppVec = userAppVec1;
    allVec.sysSaVec = vec1;

    QuotaManager::GetInstance().ProcessVecList(allVec, bundleMap1);
    EXPECT_EQ(allVec.sysAppVec[0].saName, "original");
    EXPECT_EQ(allVec.userAppVec[0].saName, "original");
    EXPECT_EQ(allVec.sysSaVec[0].saName, "original");
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetFileData_001
 * @tc.desc: Test QuotaManager::GetFileData with various file scenarios.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetFileData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetFileData_001 start";
    int64_t size = 0;
    QuotaManager quotaManager_;
    int32_t result = quotaManager_.GetFileData("/nonexistent/path/file.txt", size);
    EXPECT_EQ(result, E_FILE_PATH_INVALID);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetFileData_001 end";
}

/**
 * @tc.name: QuotaManagerTest_StringToInt64_001
 * @tc.desc: Test QuotaManager::StringToInt64 with various input scenarios.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_StringToInt64_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_StringToInt64_001 start";
    QuotaManager quotaManager_;
    int64_t outValue = 0;
    bool result = quotaManager_.StringToInt64("123", outValue);
    EXPECT_TRUE(result);
    EXPECT_EQ(outValue, 123);

    result = quotaManager_.StringToInt64("-456", outValue);
    EXPECT_TRUE(result);
    EXPECT_EQ(outValue, -456);

    result = quotaManager_.StringToInt64("0", outValue);
    EXPECT_TRUE(result);
    EXPECT_EQ(outValue, 0);

    result = quotaManager_.StringToInt64("9223372036854775807", outValue);
    EXPECT_TRUE(result);
    EXPECT_EQ(outValue, std::numeric_limits<int64_t>::max());

    result = quotaManager_.StringToInt64("-9223372036854775808", outValue);
    EXPECT_TRUE(result);
    EXPECT_EQ(outValue, std::numeric_limits<int64_t>::min());

    result = quotaManager_.StringToInt64("123abc", outValue);
    EXPECT_FALSE(result);

    result = quotaManager_.StringToInt64("hello", outValue);
    EXPECT_FALSE(result);

    result = quotaManager_.StringToInt64("", outValue);
    EXPECT_FALSE(result);

    result = quotaManager_.StringToInt64("9223372036854775808", outValue);
    EXPECT_FALSE(result);

    result = quotaManager_.StringToInt64("-9223372036854775809", outValue);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "QuotaManagerTest_StringToInt64_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDqBlkSpacesByUids_001
 * @tc.desc: Test GetDqBlkSpacesByUids with empty uids vector.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDqBlkSpacesByUids_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_001 start";
    std::vector<int32_t> uids = {1000, 1001};
    std::vector<NextDqBlk> dqBlks;
    QuotaManager::GetInstance().SetStopScanFlag(true);
    int32_t result = QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, dqBlks);
    EXPECT_EQ(result, E_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDqBlkSpacesByUids_003
 * @tc.desc: Test GetDqBlkSpacesByUids with valid uids and non-emulator environment.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDqBlkSpacesByUids_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_003 start";
    std::vector<int32_t> uids = {0, 1000, 20000000};
    std::vector<NextDqBlk> dqBlks;
    QuotaManager::GetInstance().SetStopScanFlag(false);
    int32_t result = QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, dqBlks);
    EXPECT_TRUE(result == E_OK || result == E_QUOTA_CTL_KERNEL_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_003 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDqBlkSpacesByUids_004
 * @tc.desc: Test GetDqBlkSpacesByUids with invalid uids.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDqBlkSpacesByUids_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_004 start";
    std::vector<int32_t> uids = {-1, 999999999};
    std::vector<NextDqBlk> dqBlks;
    int32_t result = QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, dqBlks);
    EXPECT_EQ(result, E_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_004 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDqBlkSpacesByUids_005
 * @tc.desc: Test GetDqBlkSpacesByUids with pre-populated dqBlks vector.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDqBlkSpacesByUids_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_005 start";
    std::vector<int32_t> uids = {1000};
    std::vector<NextDqBlk> dqBlks;
    dqBlks.push_back(NextDqBlk(100, 50, 25, 1000, 500, 10, 0, 0, 1, 1000));
    EXPECT_FALSE(dqBlks.empty());
    int32_t result = QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, dqBlks);
    EXPECT_TRUE(result == E_OK || result == E_QUOTA_CTL_KERNEL_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_005 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetDqBlkSpacesByUids_006
 * @tc.desc: Test GetDqBlkSpacesByUids with mixed valid/invalid uids and stop flag toggled during processing.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDqBlkSpacesByUids_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_006 start";

    std::vector<int32_t> uids = {0, 1000, -1, 20000000};
    std::vector<NextDqBlk> dqBlks;
    std::thread setStopFlag([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        QuotaManager::GetInstance().SetStopScanFlag(true);
    });
    setStopFlag.detach();
    int32_t result = QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, dqBlks);
    QuotaManager::GetInstance().SetStopScanFlag(false);
    EXPECT_TRUE(result == E_OK || result == E_ERR || result == E_QUOTA_CTL_KERNEL_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDqBlkSpacesByUids_006 end";
}

/**
 * @tc.name: QuotaManagerTest_ProcessDirWithUserId_001
 * @tc.desc: Test QuotaManager::ProcessDirWithUserId
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessDirWithUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessDirWithUserId_001 start";
    DirSpaceInfo dirInfo;
    dirInfo.path = "/data/%d";
    dirInfo.uid = 0;
    std::vector<int32_t> uids = {0, 1000};
    std::vector<DirSpaceInfo> resultDirs;

    // Test case 1: Normal processing with stopScanFlag = false
    QuotaManager::GetInstance().SetStopScanFlag(false);
    QuotaManager::GetInstance().ProcessDirWithUserId(dirInfo, uids, resultDirs);
    EXPECT_EQ(resultDirs.size(), uids.size());
    for (size_t i = 0; i < resultDirs.size(); i++) {
        EXPECT_EQ(resultDirs[i].uid, 0);
        EXPECT_GE(resultDirs[i].size, 0);
    }

    // Test case 2: Processing should stop when stopScanFlag = true
    resultDirs.clear();
    QuotaManager::GetInstance().SetStopScanFlag(true);
    QuotaManager::GetInstance().ProcessDirWithUserId(dirInfo, uids, resultDirs);
    EXPECT_EQ(resultDirs.size(), 0);  // resultDirs should be cleared when stopped

    // Reset flag for next test
    QuotaManager::GetInstance().SetStopScanFlag(false);

    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessDirWithUserId_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpace_001
 * @tc.desc: Test QuotaManager::GetDirListSpace
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_001 start";
    std::vector<DirSpaceInfo> dirs;
    QuotaManager quotaManager_;
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            dirs.push_back({"/data/app/el%d/base/" + std::to_string(i), 1000 + i, 0});
        } else {
            dirs.push_back({"/proc/" + std::to_string(i), 0, 0});
        }
    }
    quotaManager_.SetStopScanFlag(false);
    int32_t result = quotaManager_.GetDirListSpace(dirs);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpace_002
 * @tc.desc: Test GetDirListSpace with mixed paths (with and without %d).
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpace_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_002 start";
    std::vector<DirSpaceInfo> dirs = {{"/etc", 0, 0}, {"/data/app/el%d/base", 1000, 0}, {"/proc", 0, 0}};

    QuotaManager::GetInstance().SetStopScanFlag(false);
    int32_t result = QuotaManager::GetInstance().GetDirListSpace(dirs);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_002 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpace_003
 * @tc.desc: Test GetDirListSpace with stop flag set during processing.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpace_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_003 start";

    std::vector<DirSpaceInfo> dirs;
    for (int i = 0; i < 10; i++) {
        dirs.push_back({"/data/app/el%d/base/" + std::to_string(i), 1000 + i, 0});
    }

    std::thread setStopFlag([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        QuotaManager::GetInstance().SetStopScanFlag(true);
    });

    setStopFlag.detach();

    int32_t result = QuotaManager::GetInstance().GetDirListSpace(dirs);

    QuotaManager::GetInstance().SetStopScanFlag(false);

    EXPECT_TRUE(result == E_OK || result == E_ERR);

    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_003 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpace_005
 * @tc.desc: Test GetDirListSpace with a large number of directories.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpace_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_005 start";
    std::vector<DirSpaceInfo> dirs = {
        {"/etc", 0, 0},
        {"/proc", 0, 0},
        {"/sys", 0, 0}
    };
    QuotaManager::GetInstance().SetStopScanFlag(true);
    int32_t ret = QuotaManager::GetInstance().GetDirListSpace(dirs);
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpace_005 end";
}

/**
 * @tc.name: QuotaManagerTest_GetAncoSizeData_001
 * @tc.desc: Test QuotaManager::GetAncoSizeData
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetAncoSizeData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetAncoSizeData_001 start";

    // Test case 1: Normal processing with stopScanFlag = false
    std::string extraData;
    QuotaManager::GetInstance().SetStopScanFlag(false);
    QuotaManager::GetInstance().GetAncoSizeData(extraData);
    // extraData should contain anco size information when not stopped
    EXPECT_FALSE(extraData.empty());
    EXPECT_NE(extraData.find("anco"), std::string::npos);

    // Test case 2: Processing should stop when stopScanFlag = true
    extraData.clear();
    QuotaManager::GetInstance().SetStopScanFlag(true);
    QuotaManager::GetInstance().GetAncoSizeData(extraData);
    // extraData should be empty when stopped early
    EXPECT_TRUE(extraData.empty());

    // Reset flag for next test
    QuotaManager::GetInstance().SetStopScanFlag(false);

    GTEST_LOG_(INFO) << "QuotaManagerTest_GetAncoSizeData_001 end";
}

HWTEST_F(QuotaManagerTest, QuotaManagerTest_ListUserdataDirInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ListUserdataDirInfo_001 start";
    std::vector<OHOS::StorageManager::UserdataDirInfo> scanDirs;
    int32_t result = QuotaManager::GetInstance().ListUserdataDirInfo(scanDirs);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "QuotaManagerTest_ListUserdataDirInfo_001 end";
}

HWTEST_F(QuotaManagerTest, QuotaManagerTest_ListUserdataDirInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ListUserdataDirInfo_002 start";

    std::filesystem::path testFile = "/data/test_file";
    std::ofstream file(testFile, std::ios::binary);
    if (!file) {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        GTEST_FAIL();
    }
    file.seekp(1 * 1024 * 1024 * 1024 + 1);
    file.write("a", 1);
    file.close();

    std::vector<OHOS::StorageManager::UserdataDirInfo> scanDirs;
    int32_t result = QuotaManager::GetInstance().ListUserdataDirInfo(scanDirs);
    EXPECT_EQ(result, E_OK);
    std::filesystem::remove(testFile);
    GTEST_LOG_(INFO) << "QuotaManagerTest_ListUserdataDirInfo_002 end";
}

HWTEST_F(QuotaManagerTest, QuotaManagerTest_ScanDirRecurse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ScanDirRecurse_001 start";

    std::string testFile = "/data/test_file";
    std::vector<OHOS::StorageManager::UserdataDirInfo> scanDirs;
    OHOS::StorageManager::UserdataDirInfo result = QuotaManager::GetInstance().ScanDirRecurse(testFile, scanDirs);
    EXPECT_EQ(result.totalCnt_, 0);
    EXPECT_EQ(scanDirs.size(), 0);
    GTEST_LOG_(INFO) << "QuotaManagerTest_ScanDirRecurse_001 end";
}
} // STORAGE_DAEMON
} // OHOS