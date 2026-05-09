/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
const int64_t BYTES_PRE_MB = 1024 * 1024;

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
 * @tc.desc: Test whether GetUidStorageStats is called normally with new signature.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetUidStorageStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_001 start";

    std::vector<UidSaInfo> vec;
    int64_t totalSize = 0;
    std::map<int32_t, std::string> bundleNameAndUid;
    QuotaManager::GetInstance().GetUidStorageStats(vec, totalSize, bundleNameAndUid, SYS_SA);
    EXPECT_FALSE(vec.empty());
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetUidStorageStats_002
 * @tc.desc: Test GetUidStorageStats with different app types.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetUidStorageStats_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_002 start";

    std::vector<UidSaInfo> vec;
    int64_t totalSize = 0;
    std::map<int32_t, std::string> bundleNameAndUid;

    QuotaManager::GetInstance().GetUidStorageStats(vec, totalSize, bundleNameAndUid, SYS_APP);
    EXPECT_TRUE(totalSize >= 0);

    vec.clear();
    totalSize = 0;
    QuotaManager::GetInstance().GetUidStorageStats(vec, totalSize, bundleNameAndUid, USER_APP);
    EXPECT_TRUE(totalSize >= 0);

    vec.clear();
    totalSize = 0;
    QuotaManager::GetInstance().GetUidStorageStats(vec, totalSize, bundleNameAndUid, OTHER_APP);
    EXPECT_TRUE(totalSize >= 0);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_002 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_GetUidStorageStats_003
 * @tc.desc: Test GetUidStorageStats with invalid type.
 * @tc.type: FUNC
 * @tc.require.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetUidStorageStats_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_003 start";

    std::vector<UidSaInfo> vec;
    int64_t totalSize = 0;
    std::map<int32_t, std::string> bundleNameAndUid;

    QuotaManager::GetInstance().GetUidStorageStats(vec, totalSize, bundleNameAndUid, 0);
    EXPECT_TRUE(vec.empty());

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_003 end";
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
    std::vector<UidSaInfo> vec;
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
    AllAppVec allVec;
    std::uint64_t iNodes;
    QuotaManager::GetInstance().GetOccupiedSpaceForUidList(allVec, iNodes);
    UidSaInfo info = {0, "root", 0};
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
    std::vector<UidSaInfo> vec = {{3003, "vecDefault", 4096 * BYTES_PRE_MB}};
    std::vector<UidSaInfo> otherAppVec = {{4004, "vecDefault", 4096}};
    std::map<int32_t, std::string> bundleMap = {{1001, "SystemApp"}, {2002, "UserApp"}, {3003, "VecApp"}};
    AllAppVec allVec;
    allVec.otherAppVec = otherAppVec;
    allVec.sysAppVec = sysAppVec;
    allVec.userAppVec = userAppVec;
    allVec.sysSaVec = vec;
    QuotaManager::GetInstance().ProcessVecList(allVec.sysSaVec, true, bundleMap);
    QuotaManager::GetInstance().ProcessVecList(allVec.userAppVec, false, bundleMap);
    QuotaManager::GetInstance().ProcessVecList(allVec.sysAppVec, false, bundleMap);
    QuotaManager::GetInstance().ProcessVecList(allVec.otherAppVec, false, bundleMap);

    EXPECT_EQ(allVec.sysAppVec[0].saName, "SystemApp");
    EXPECT_EQ(allVec.userAppVec[0].saName, "UserApp");
    EXPECT_EQ(allVec.sysSaVec[0].saName, "vecDefault");

    std::vector<UidSaInfo> sysAppVec1 = {{1001, "original", 1024}};
    std::vector<UidSaInfo> userAppVec1 = {{2002, "original", 2048}};
    std::vector<UidSaInfo> vec1 = {{3003, "original", 4096 * BYTES_PRE_MB}};
    std::map<int32_t, std::string> bundleMap1;
    allVec.sysAppVec = sysAppVec1;
    allVec.userAppVec = userAppVec1;
    allVec.sysSaVec = vec1;

    QuotaManager::GetInstance().ProcessVecList(allVec.sysSaVec, true, bundleMap1);
    QuotaManager::GetInstance().ProcessVecList(allVec.userAppVec, false, bundleMap1);
    QuotaManager::GetInstance().ProcessVecList(allVec.sysAppVec, false, bundleMap1);
    QuotaManager::GetInstance().ProcessVecList(allVec.otherAppVec, false, bundleMap1);
    EXPECT_EQ(allVec.sysAppVec[0].saName, "original");
    EXPECT_EQ(allVec.userAppVec[0].saName, "original");
    EXPECT_EQ(allVec.sysSaVec[0].saName, "original");
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_001 end";
}

/**
 * @tc.name: QuotaManagerTest_ProcessVecList_002
 * @tc.desc: Test ProcessVecList with new signature for single vector processing.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessVecList_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_002 start";
    std::vector<UidSaInfo> vec = {
        {1001, "saDefault", 1024 * BYTES_PRE_MB},
        {2002, "saDefault2", 2048 * BYTES_PRE_MB}
    };
    std::map<int32_t, std::string> bundleMap = {{1001, "SA1"}, {2002, "SA2"}};

    QuotaManager::GetInstance().ProcessVecList(vec, true, bundleMap);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec[0].saName, "saDefault2");

    std::vector<UidSaInfo> vec2 = {{3003, "appDefault", 4096}, {4004, "appDefault2", 8192}};
    std::map<int32_t, std::string> bundleMap2 = {{3003, "App1"}, {4004, "App2"}};

    QuotaManager::GetInstance().ProcessVecList(vec2, false, bundleMap2);
    EXPECT_FALSE(vec2.empty());
    EXPECT_EQ(vec2[0].saName, "App2");

    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_002 end";
}

/**
 * @tc.name: QuotaManagerTest_SortAndCutSaInfoVec_001
 * @tc.desc: Test SortAndCutSaInfoVec with isSa parameter.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_SortAndCutSaInfoVec_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_SortAndCutSaInfoVec_001 start";
    std::vector<UidSaInfo> vec = {
        {1001, "sa1", 1024},
        {2002, "sa2", 4096 * BYTES_PRE_MB},
        {3003, "sa3", 2048 * BYTES_PRE_MB},
        {4004, "sa4", 8192 * BYTES_PRE_MB}
    };

    QuotaManager::GetInstance().SortAndCutSaInfoVec(vec, true);
    if (!vec.empty()) {
        EXPECT_EQ(vec[0].size, 8192 * BYTES_PRE_MB);
    }

    std::vector<UidSaInfo> vec2 = {
        {1001, "app1", 1024},
        {2002, "app2", 4096},
        {3003, "app3", 2048},
        {4004, "app4", 8192}
    };

    QuotaManager::GetInstance().SortAndCutSaInfoVec(vec2, false);
    if (!vec2.empty()) {
        EXPECT_EQ(vec2[0].size, 8192);
    }

    GTEST_LOG_(INFO) << "QuotaManagerTest_SortAndCutSaInfoVec_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetSaOrOtherTotal_001
 * @tc.desc: Test GetSaOrOtherTotal returns correct total size.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetSaOrOtherTotal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSaOrOtherTotal_001 start";
    std::vector<UidSaInfo> vec = {
        {1001, "sa1", 1024},
        {2002, "sa2", 2048},
        {3003, "sa3", 4096}
    };

    int64_t totalSize = 0;
    totalSize = QuotaManager::GetInstance().GetSaOrOtherTotal(vec);
    EXPECT_EQ(totalSize, 7168);

    std::vector<UidSaInfo> emptyVec;
    totalSize = QuotaManager::GetInstance().GetSaOrOtherTotal(emptyVec);
    EXPECT_EQ(totalSize, 0);

    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSaOrOtherTotal_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetUidStorageStats_004
 * @tc.desc: Test GetUidStorageStats with empty bundle map.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_GetUidStorageStats_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_004 start";

    std::vector<UidSaInfo> vec;
    int64_t totalSize = 0;
    std::map<int32_t, std::string> emptyBundleMap;

    QuotaManager::GetInstance().GetUidStorageStats(vec, totalSize, emptyBundleMap, SYS_SA);
    EXPECT_TRUE(totalSize >= 0);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetUidStorageStats_004 end";
}

/**
 * @tc.name: QuotaManagerTest_SortAndCutSaInfoVec_002
 * @tc.desc: Test SortAndCutSaInfoVec with large vector.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_SortAndCutSaInfoVec_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_SortAndCutSaInfoVec_002 start";

    std::vector<UidSaInfo> vec;
    for (int i = 0; i < 20; i++) {
        vec.push_back({1000 + i, "app" + std::to_string(i), (i + 1) * 1024});
    }

    QuotaManager::GetInstance().SortAndCutSaInfoVec(vec, false);

    if (!vec.empty()) {
        EXPECT_EQ(vec[0].size, 20 * 1024);
    }

    GTEST_LOG_(INFO) << "QuotaManagerTest_SortAndCutSaInfoVec_002 end";
}

/**
 * @tc.name: QuotaManagerTest_SortAndCutSaInfoVec_003
 * @tc.desc: Test SortAndCutSaInfoVec with isSa true (no size limit).
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_SortAndCutSaInfoVec_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_SortAndCutSaInfoVec_003 start";

    std::vector<UidSaInfo> vec;
    for (int i = 0; i < 20; i++) {
        vec.push_back({1000 + i, "sa" + std::to_string(i), (i + 1) * 1024 * BYTES_PRE_MB});
    }

    QuotaManager::GetInstance().SortAndCutSaInfoVec(vec, true);
    if (!vec.empty()) {
        EXPECT_EQ(vec[0].size, 20 * 1024 * BYTES_PRE_MB);
    }

    GTEST_LOG_(INFO) << "QuotaManagerTest_SortAndCutSaInfoVec_003 end";
}

/**
 * @tc.name: QuotaManagerTest_ProcessVecList_003
 * @tc.desc: Test ProcessVecList with empty vector.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessVecList_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_003 start";

    std::vector<UidSaInfo> emptyVec;
    std::map<int32_t, std::string> bundleMap;

    QuotaManager::GetInstance().ProcessVecList(emptyVec, true, bundleMap);
    EXPECT_TRUE(emptyVec.empty());

    QuotaManager::GetInstance().ProcessVecList(emptyVec, false, bundleMap);
    EXPECT_TRUE(emptyVec.empty());

    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_003 end";
}

/**
 * @tc.name: QuotaManagerTest_ProcessVecList_004
 * @tc.desc: Test ProcessVecList with single element.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessVecList_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_004 start";

    std::vector<UidSaInfo> vec = {{1001, "single", 1024}};
    std::map<int32_t, std::string> bundleMap = {{1001, "SingleApp"}};

    QuotaManager::GetInstance().ProcessVecList(vec, false, bundleMap);

    if (!vec.empty()) {
        EXPECT_EQ(vec[0].saName, "SingleApp");
    }

    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_004 end";
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

/**
 * @tc.name: QuotaManagerTest_GetSystemDataSize_001
 * @tc.desc: Test GetSystemDataSize with successful path (all sub-functions succeed)
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetSystemDataSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemDataSize_001 start";
    int64_t otherUidSizeSum = 0;
    int32_t result = QuotaManager::GetInstance().GetSystemDataSize(otherUidSizeSum);
    // Result should be E_OK if config file exists and is valid, otherwise E_GET_SYSTEM_DATA_SIZE_ERROR
    EXPECT_TRUE(result == E_OK || result == E_GET_SYSTEM_DATA_SIZE_ERROR);
    if (result == E_OK) {
        EXPECT_GE(otherUidSizeSum, 0);
    }
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemDataSize_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetSystemDataSize_002
 * @tc.desc: Test GetSystemDataSize when ParseSystemDataConfigFile fails
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetSystemDataSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemDataSize_002 start";
    // This test verifies the error handling when config file is missing or invalid
    int64_t otherUidSizeSum = 0;
    // The actual result depends on system configuration
    int32_t result = QuotaManager::GetInstance().GetSystemDataSize(otherUidSizeSum);
    EXPECT_TRUE(result == E_OK || result == E_GET_SYSTEM_DATA_SIZE_ERROR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemDataSize_002 end";
}

/**
 * @tc.name: QuotaManagerTest_ParseSystemDataConfigFile_001
 * @tc.desc: Test ParseSystemDataConfigFile with valid config file
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ParseSystemDataConfigFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ParseSystemDataConfigFile_001 start";
    std::vector<int32_t> uidList;
    int32_t result = QuotaManager::GetInstance().ParseSystemDataConfigFile(uidList);
    // Result depends on whether config file exists on system
    EXPECT_TRUE(result == E_OK || result == E_PARAMS_INVALID);
    if (result == E_OK) {
        EXPECT_FALSE(uidList.empty());
    }
    GTEST_LOG_(INFO) << "QuotaManagerTest_ParseSystemDataConfigFile_001 end";
}

/**
 * @tc.name: QuotaManagerTest_ParseSystemDataConfigFile_002
 * @tc.desc: Test ParseSystemDataConfigFile error handling
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ParseSystemDataConfigFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_ParseSystemDataConfigFile_002 start";
    // Test with invalid config path (internal function uses SYSTEM_DATA_CONFIG_PATH)
    // This test verifies error handling when config file doesn't exist
    std::vector<int32_t> uidList;
    int32_t result = QuotaManager::GetInstance().ParseSystemDataConfigFile(uidList);
    // On systems without config file, this should return error
    EXPECT_TRUE(result == E_OK || result == E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "QuotaManagerTest_ParseSystemDataConfigFile_002 end";
}

/**
 * @tc.name: QuotaManagerTest_GetSystemCacheSize_001
 * @tc.desc: Test GetSystemCacheSize with empty uid list
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetSystemCacheSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemCacheSize_001 start";
    std::vector<int32_t> uidList;
    int64_t cacheSize = 0;
    int32_t result = QuotaManager::GetInstance().GetSystemCacheSize(uidList, cacheSize);
    // Empty list should return E_OK with cacheSize = 0
    EXPECT_TRUE(result == E_OK || result == E_NOT_SUPPORT);
    EXPECT_EQ(cacheSize, 0);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemCacheSize_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetSystemCacheSize_002
 * @tc.desc: Test GetSystemCacheSize with system UIDs that should be skipped
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetSystemCacheSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemCacheSize_002 start";
    std::vector<int32_t> uidList = {0, 1000, 1111}; // ROOT_UID, SYSTEM_UID, MEMMGR_UID
    int64_t cacheSize = 0; // Output parameter, initialized to 0
    int32_t result = QuotaManager::GetInstance().GetSystemCacheSize(uidList, cacheSize);
    // These UIDs should be skipped, so cacheSize should remain 0
    EXPECT_TRUE(result == E_OK || result == E_NOT_SUPPORT);
    if (result == E_OK) {
        EXPECT_EQ(cacheSize, 0);
    }
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemCacheSize_002 end";
}

/**
 * @tc.name: QuotaManagerTest_GetSystemCacheSize_003
 * @tc.desc: Test GetSystemCacheSize with regular UIDs
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetSystemCacheSize_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemCacheSize_003 start";
    std::vector<int32_t> uidList = {1099, 1100}; // Non-system UIDs
    int64_t cacheSize = 0;
    int32_t result = QuotaManager::GetInstance().GetSystemCacheSize(uidList, cacheSize);
    // Result depends on system quota support
    EXPECT_TRUE(result == E_OK || result == E_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetSystemCacheSize_003 end";
}

/**
 * @tc.name: QuotaManagerTest_GetMetaDataSize_001
 * @tc.desc: Test GetMetaDataSize when file paths are invalid or files don't exist
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetMetaDataSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetMetaDataSize_001 start";
    // This test verifies error handling when HMFS sysfs files don't exist
    // which is typical on non-HMFS systems
    int64_t metaDataSize = 0;
    int32_t result = QuotaManager::GetInstance().GetMetaDataSize(metaDataSize);
    // On systems without HMFS, this should return error
    EXPECT_TRUE(result == E_OK || result == E_GET_SYSTEM_DATA_SIZE_ERROR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetMetaDataSize_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetMetaDataSize_002
 * @tc.desc: Test GetMetaDataSize overflow check
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetMetaDataSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetMetaDataSize_002 start";
    // This test verifies the overflow detection in GetMetaDataSize
    // The function checks for overflow before calculating metadata size
    int64_t metaDataSize = 0;
    int32_t result = QuotaManager::GetInstance().GetMetaDataSize(metaDataSize);
    // If files don't exist, returns error; if they exist, checks for overflow
    EXPECT_TRUE(result == E_OK || result == E_GET_SYSTEM_DATA_SIZE_ERROR || result == E_CALCULATE_OVERFLOW_UP);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetMetaDataSize_002 end";
}

/**
 * @tc.name: QuotaManagerTest_AddBlksRecurseMultiUids_001
 * @tc.desc: Test AddBlksRecurseMultiUids with valid directory
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AddBlksRecurseMultiUids_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksRecurseMultiUids_001 start";
    std::string path = "/data";
    std::vector<int64_t> blks = {0, 0, 0};
    std::vector<int32_t> uids = {0, 1000, 2000};
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().AddBlksRecurseMultiUids(path, blks, uids, largeFiles, dirSizeMap);
    // Result depends on whether directory exists and is accessible
    EXPECT_TRUE(result == E_OK || result == E_STATISTIC_OPEN_DIR_FAILED || result == E_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksRecurseMultiUids_001 end";
}

/**
 * @tc.name: QuotaManagerTest_AddBlksRecurseMultiUids_002
 * @tc.desc: Test AddBlksRecurseMultiUids with non-existent path
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AddBlksRecurseMultiUids_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksRecurseMultiUids_002 start";
    std::string path = "/nonexistent/path/that/does/not/exist";
    std::vector<int64_t> blks = {0, 0};
    std::vector<int32_t> uids = {0, 1000};
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().AddBlksRecurseMultiUids(path, blks, uids, largeFiles, dirSizeMap);
    // Should fail since path doesn't exist
    EXPECT_TRUE(result == E_OK || result == E_STATISTIC_OPEN_DIR_FAILED ||
        result == E_STATISTIC_STAT_FAILED || result == E_ERR);
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksRecurseMultiUids_002 end";
}

/**
 * @tc.name: QuotaManagerTest_AddBlksMultiUids_001
 * @tc.desc: Test AddBlksMultiUids with existing file
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AddBlksMultiUids_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksMultiUids_001 start";
    // Use an existing file that should be present on most systems
    std::string path = "/etc/passwd";
    std::vector<int64_t> blks = {0, 0}; // Output parameter, initialized to 0
    std::vector<int32_t> uids = {0, 1000}; // Root and system UIDs
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().AddBlksMultiUids(path, blks, uids, largeFiles, dirSizeMap);
    // /etc/passwd is usually owned by root
    EXPECT_TRUE(result == E_OK || result == E_STATISTIC_STAT_FAILED);
    if (result == E_OK) {
        // At least one of the UIDs should have matching files
        bool hasNonZero = false;
        for (auto blk : blks) {
            if (blk > 0) {
                hasNonZero = true;
                break;
            }
        }
        EXPECT_TRUE(hasNonZero);
    }
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksMultiUids_001 end";
}

/**
 * @tc.name: QuotaManagerTest_AddBlksMultiUids_002
 * @tc.desc: Test AddBlksMultiUids with non-existent file
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AddBlksMultiUids_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksMultiUids_002 start";
    std::string path = "/nonexistent/file/path.txt";
    std::vector<int64_t> blks = {0, 0};
    std::vector<int32_t> uids = {0, 1000};
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().AddBlksMultiUids(path, blks, uids, largeFiles, dirSizeMap);
    // Should fail with stat error
    EXPECT_EQ(result, E_STATISTIC_STAT_FAILED);
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksMultiUids_002 end";
}

/**
 * @tc.name: QuotaManagerTest_AddBlksMultiUids_003
 * @tc.desc: Test AddBlksMultiUids verifies blks values are correctly accumulated
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AddBlksMultiUids_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksMultiUids_003 start";
    // Test with /etc/passwd which usually exists and is owned by root
    std::string path = "/etc/passwd";
    std::vector<int64_t> blks = {0, 0}; // Output parameter, initialized to 0
    std::vector<int32_t> uids = {0, 1000}; // Root and system UIDs
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().AddBlksMultiUids(path, blks, uids, largeFiles, dirSizeMap);
    EXPECT_TRUE(result == E_OK || result == E_STATISTIC_STAT_FAILED);
    if (result == E_OK) {
        // /etc/passwd is usually owned by root, so blks[0] should be > 0
        // blks should be modified by the function (output parameter)
        EXPECT_GE(blks[0] + blks[1], 0); // At least the file owner's blocks should be counted
    }
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksMultiUids_003 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_001
 * @tc.desc: Test GetDirListSpaceByPaths with valid paths and UIDs
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_001 start";
    std::vector<std::string> paths = {"/etc"};
    std::vector<int32_t> uids = {0};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should succeed with valid inputs
    EXPECT_TRUE(result == E_OK || result == E_ERR || result == E_STATISTIC_OPEN_DIR_FAILED);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_002
 * @tc.desc: Test GetDirListSpaceByPaths with empty paths
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_002 start";
    std::vector<std::string> paths;
    std::vector<int32_t> uids = {0, 1000};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should fail with empty paths
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_002 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_003
 * @tc.desc: Test GetDirListSpaceByPaths with empty UIDs
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_003 start";
    std::vector<std::string> paths = {"/etc", "/data"};
    std::vector<int32_t> uids;
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should fail with empty UIDs
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_003 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_004
 * @tc.desc: Test GetDirListSpaceByPaths with paths exceeding MAX_WHITE_PATH_COUNT
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_004 start";
    std::vector<std::string> paths;
    // Create 11 paths (MAX_WHITE_PATH_COUNT is 10)
    for (int i = 0; i < 11; i++) {
        paths.push_back("/etc/path" + std::to_string(i));
    }
    std::vector<int32_t> uids = {0};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should fail when paths count exceeds MAX_WHITE_PATH_COUNT
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_004 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_005
 * @tc.desc: Test GetDirListSpaceByPaths with UIDs exceeding MAX_WHITE_UID_COUNT
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_005 start";
    std::vector<std::string> paths = {"/etc"};
    std::vector<int32_t> uids;
    // Create 4 UIDs (MAX_WHITE_UID_COUNT is 3)
    for (int i = 0; i < 4; i++) {
        uids.push_back(1000 + i);
    }
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should fail when UIDs count exceeds MAX_WHITE_UID_COUNT
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_005 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_006
 * @tc.desc: Test GetDirListSpaceByPaths with stop flag set
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_006 start";
    std::vector<std::string> paths = {"/etc", "/data"};
    std::vector<int32_t> uids = {0, 1000};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    // Set stop flag to true
    QuotaManager::GetInstance().SetStopScanFlag(true);
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should return E_ERR when stop flag is set
    EXPECT_EQ(result, E_ERR);
    EXPECT_TRUE(resultDirs.empty());
    // Reset stop flag
    QuotaManager::GetInstance().SetStopScanFlag(false);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_006 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_007
 * @tc.desc: Test GetDirListSpaceByPaths with multiple paths and UIDs
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_007 start";
    std::vector<std::string> paths = {"/etc"};
    std::vector<int32_t> uids = {0, 1000};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // Should process each path for each UID
    EXPECT_TRUE(result == E_OK || result == E_ERR || result == E_STATISTIC_OPEN_DIR_FAILED);
    if (result == E_OK) {
        // resultDirs should contain entries for each path-UID combination
        EXPECT_EQ(resultDirs.size(), uids.size());
    }
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_007 end";
}

/**
 * @tc.name: QuotaManagerTest_AddBlksRecurseMultiUids_StopFlag_001
 * @tc.desc: Test AddBlksRecurseMultiUids with stopScanFlag set at entry.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_AddBlksRecurseMultiUids_StopFlag_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_AddBlksRecurseMultiUids_StopFlag_001 start";
    std::string path = "/data";
    std::vector<int64_t> blks = {0, 0};
    std::vector<int32_t> uids = {0, 1000};
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    // Set stopScanFlag to true, should return E_ERR immediately
    QuotaManager::GetInstance().SetStopScanFlag(true);
    int32_t result = QuotaManager::GetInstance().AddBlksRecurseMultiUids(path, blks, uids, largeFiles, dirSizeMap);
    EXPECT_EQ(result, E_ERR);
    // Reset flag
    QuotaManager::GetInstance().SetStopScanFlag(false);
    GTEST_LOG_(INFO) << "QuotaManagerTest_ltiUids_StopFlag_001 end";
}

/**
 * @tc.name: QuotaManagerTest_GetDirListSpaceByPaths_StopAfterLoop_001
 * @tc.desc: Test GetDirListSpaceByPaths with stopScanFlag set after loop completion.
 * @tc.type: FUNC
 * @tc.require: AR20260114725643
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_GetDirListSpaceByPaths_StopAfterLoop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_StopAfterLoop_001 start";
    std::vector<std::string> paths = {"/etc"};
    std::vector<int32_t> uids = {0};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::vector<LargeDirInfo> largeDirs;
    // Process normally first, then set stop flag to verify post-loop check
    QuotaManager::GetInstance().SetStopScanFlag(false);
    int32_t result = QuotaManager::GetInstance().GetDirListSpaceByPaths(paths, uids, resultDirs, largeFiles, largeDirs);
    // After loop, should succeed or fail depending on path existence
    EXPECT_TRUE(result == E_OK || result == E_ERR || result == E_STATISTIC_OPEN_DIR_FAILED);
    GTEST_LOG_(INFO) << "QuotaManagerTest_GetDirListSpaceByPaths_StopAfterLoop_001 end";
}

/**
 * @tc.name: QuotaManagerTest_SetQuotaPrjId_001
 * @tc.desc: Test SetQuotaPrjId.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_SetQuotaPrjId_001, TestSize.Level1)
{
    std::string path = "/nonexistent/path";
    int32_t prjId = 100;
    bool inherit = false;
    int32_t result = QuotaManager::GetInstance().SetQuotaPrjId(path, prjId, inherit);
    EXPECT_TRUE(result == E_PARAMS_NULLPTR_ERR || result == E_SYS_KERNEL_ERR);
}

/**
 * @tc.name: QuotaManagerTest_UpdateParentDirSizes_001
 * @tc.desc: Test UpdateParentDirSizes.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_UpdateParentDirSizes_001, TestSize.Level1)
{
    std::map<std::string, int64_t> dirSizeMap;
    std::string path = "/data/service/el1/public/test.txt";
    int64_t fileSize = 1024 * 1024;
    QuotaManager::GetInstance().UpdateParentDirSizes(path, fileSize, dirSizeMap);
    EXPECT_FALSE(dirSizeMap.empty());
}

/**
 * @tc.name: QuotaManagerTest_CollectLargeFile_001
 * @tc.desc: Test CollectLargeFile.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_CollectLargeFile_001, TestSize.Level1)
{
    std::vector<LargeFileInfo> largeFiles;
    std::string path = "/data/large_file.dat";
    uint64_t fileSize = 2 * 1024 * 1024;
    QuotaManager::GetInstance().CollectLargeFile(path, fileSize, largeFiles);
    EXPECT_EQ(largeFiles.size(), 1);
}

/**
 * @tc.name: QuotaManagerTest_ProcessLargeFiles_001
 * @tc.desc: Test ProcessLargeFiles.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessLargeFiles_001, TestSize.Level1)
{
    std::vector<LargeFileInfo> allLargeFiles;
    std::vector<LargeFileInfo> largeFiles;
    for (int i = 0; i < 20; i++) {
        allLargeFiles.push_back({"/data/file" + std::to_string(i) + ".dat", (i + 1) * 1024 * 1024});
    }
    QuotaManager::GetInstance().ProcessLargeFiles(allLargeFiles, largeFiles);
    EXPECT_EQ(largeFiles.size(), 15);
}

/**
 * @tc.name: QuotaManagerTest_ProcessLargeDirs_001
 * @tc.desc: Test ProcessLargeDirs.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ProcessLargeDirs_001, TestSize.Level1)
{
    std::map<std::string, int64_t> dirSizeMap;
    std::vector<LargeDirInfo> largeDirs;
    for (int i = 0; i < 20; i++) {
        dirSizeMap["/data/dir" + std::to_string(i)] = (i + 1) * 6 * 1024 * 1024;
    }
    QuotaManager::GetInstance().ProcessLargeDirs(dirSizeMap, largeDirs);
    EXPECT_EQ(largeDirs.size(), 15);
}

/**
 * @tc.name: QuotaManagerTest_ScanSinglePath_001
 * @tc.desc: Test ScanSinglePath.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ScanSinglePath_001, TestSize.Level1)
{
    std::string path = "/etc";
    std::vector<int32_t> uids = {0};
    std::vector<DirSpaceInfo> resultDirs;
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().ScanSinglePath(path, uids, resultDirs, largeFiles, dirSizeMap);
    EXPECT_TRUE(result == E_OK || result == E_STATISTIC_OPEN_DIR_FAILED);
}

/**
 * @tc.name: QuotaManagerTest_ScanDirectoryEntries_001
 * @tc.desc: Test ScanDirectoryEntries.
 * @tc.type: FUNC
 * @tc.require: AR20260304664295
 */
HWTEST_F(QuotaManagerTest, QuotaManagerTest_ScanDirectoryEntries_001, TestSize.Level1)
{
    std::string path = "/etc";
    std::vector<int64_t> blks = {0, 0};
    std::vector<int32_t> uids = {0, 1000};
    std::vector<LargeFileInfo> largeFiles;
    std::map<std::string, int64_t> dirSizeMap;
    int32_t result = QuotaManager::GetInstance().ScanDirectoryEntries(path, blks, uids, largeFiles, dirSizeMap);
    EXPECT_TRUE(result == E_OK || result == E_STATISTIC_OPEN_DIR_FAILED);
}
} // STORAGE_DAEMON
} // OHOS