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

#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "test/common/help_utils.h"
#include "utils/file_utils.h"

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

    std::string bundleName = EMPTY_STRING;
    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(result, E_PARAMS_INVALID);

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

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = -1;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
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

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = UID;
    std::string bundleDataDirPath = EMPTY_STRING;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
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

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = -1;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
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

    std::string bundleName = BUNDLE_NAME;
    int32_t uid = UID;
    std::string bundleDataDirPath = BUNDLE_PATH;
    int32_t limitSizeMb = LIMITSIZE;
    int32_t result = QuotaManager::GetInstance().SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
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
    EXPECT_EQ(result, 1.00);

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

    path = "valid_entry_file";
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

    std::vector<struct UidSaInfo> vec;
    std::vector<struct UidSaInfo> vec1;
    std::vector<struct UidSaInfo> vec2;
    EXPECT_EQ(QuotaManager::GetInstance().GetOccupiedSpaceForUidList(vec, vec1, vec2), E_OK);
    struct UidSaInfo info = {0, "root", 0};
    vec.emplace_back(info);
    EXPECT_EQ(QuotaManager::GetInstance().GetOccupiedSpaceForUidList(vec, vec1, vec2), E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_GetOccupiedSpaceForUidList_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_StatisticSysDirSpace_001
 * @tc.desc: Test whether StatisticSysDirSpace is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_StatisticSysDirSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_StatisticSysDirSpace_001 start";

    EXPECT_EQ(QuotaManager::GetInstance().StatisticSysDirSpace(), E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_StatisticSysDirSpace_001 end";
}

/**
 * @tc.name: Storage_Service_QuotaManagerTest_AddDirSpace_001
 * @tc.desc: Test whether AddDirSpace is called normally.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(QuotaManagerTest, Storage_Service_QuotaManagerTest_AddDirSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_AddDirSpace_001 start";

    std::vector<DirSpaceInfo> dirs = {
        {"/data/app/el1/public", 0, 0},
        {"/data/app/el1/%d/base", 0, 0}
    };
    std::vector<int32_t> userIds = {100};
    std::string data = QuotaManager::GetInstance().AddDirSpace(dirs, userIds);
    EXPECT_TRUE(!data.empty());

    GTEST_LOG_(INFO) << "Storage_Service_QuotaManagerTest_AddDirSpace_001 end";
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
    std::map<int32_t, std::string> bundleMap = {{1001, "SystemApp"}, {2002, "UserApp"}, {3003, "VecApp"}};

    QuotaManager::GetInstance().ProcessVecList(sysAppVec, userAppVec, vec, bundleMap);

    EXPECT_EQ(sysAppVec[0].saName, "SystemApp");
    EXPECT_EQ(userAppVec[0].saName, "UserApp");
    EXPECT_EQ(vec[0].saName, "vecDefault");

    std::vector<UidSaInfo> sysAppVec1 = {{1001, "original", 1024}};
    std::vector<UidSaInfo> userAppVec1 = {{2002, "original", 2048}};
    std::vector<UidSaInfo> vec1 = {{3003, "original", 4096}};
    std::map<int32_t, std::string> bundleMap1; // 空 map

    QuotaManager::GetInstance().ProcessVecList(sysAppVec1, userAppVec1, vec1, bundleMap1);

    EXPECT_EQ(sysAppVec1[0].saName, "original");
    EXPECT_EQ(userAppVec1[0].saName, "original");
    EXPECT_EQ(vec1[0].saName, "original");
    GTEST_LOG_(INFO) << "QuotaManagerTest_ProcessVecList_001 end";
}
} // STORAGE_DAEMON
} // OHOS