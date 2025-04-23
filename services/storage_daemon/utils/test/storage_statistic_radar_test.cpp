/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"
#include "utils/storage_statistics_radar.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class StorageStatisticRadarTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    static RadarStatisticInfo getRadarInfo();
    void SetUp() {};
    void TearDown() {};
};

RadarStatisticInfo StorageStatisticRadarTest::getRadarInfo()
{
    RadarStatisticInfo info;
    info.keyLoadSuccCount = 1;
    info.keyLoadFailCount = 2;
    info.keyUnloadSuccCount = 3;
    info.keyUnloadFailCount = 4;
    info.userAddSuccCount = 5;
    info.userAddFailCount = 6;
    info.userRemoveSuccCount = 7;
    info.userRemoveFailCount = 8;
    info.userStartSuccCount = 9;
    info.userStartFailCount = 10;
    info.userStopSuccCount = 11;
    info.userStopFailCount = 12;
    return info;
}

/**
 * @tc.name: StorageStatisticRadarTest_CreateStatisticFile_001
 * @tc.desc: Verify the CreateStatisticFile function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageStatisticRadarTest, StorageStatisticRadarTest_CreateStatisticFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_CreateStatisticFile_001 start";
    std::string PATH_STORAGE_RADAR = "/data/service/el1/public/storage_daemon/radar/StorageStatisticFile.json";
    StorageStatisticRadar radar;

    std::map<uint32_t, RadarStatisticInfo> statistics;
    RadarStatisticInfo info = StorageStatisticRadarTest::getRadarInfo();
    statistics.insert(std::make_pair(1, info));
    std::string jsonStr = radar.CreateJsonString(statistics);

    SaveStringToFileSync(PATH_STORAGE_RADAR.c_str(), jsonStr);
    EXPECT_TRUE(radar.CreateStatisticFile());

    std::remove(PATH_STORAGE_RADAR.c_str());
    EXPECT_TRUE(radar.CreateStatisticFile());
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_CreateStatisticFile_001 end";
}

/**
 * @tc.name: StorageStatisticRadarTest_CleanStatisticFile_001
 * @tc.desc: Verify the CleanStatisticFile function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageStatisticRadarTest, StorageStatisticRadarTest_CleanStatisticFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_CleanStatisticFile_001 start";
    StorageStatisticRadar radar;
    std::string countInfo = "1,2,3,4,5,6,7,8,9,10,11";
    std::string PATH_STORAGE_RADAR = "/data/service/el1/public/storage_daemon/radar/StorageStatisticFile.json";

    std::map<uint32_t, RadarStatisticInfo> statistics;
    RadarStatisticInfo info = StorageStatisticRadarTest::getRadarInfo();
    statistics.insert(std::make_pair(1, info));
    std::string jsonStr = radar.CreateJsonString(statistics);
    SaveStringToFileSync(PATH_STORAGE_RADAR.c_str(), jsonStr);

    radar.CleanStatisticFile();
    std::ifstream inFile(PATH_STORAGE_RADAR.c_str());
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.empty());

    std::remove(PATH_STORAGE_RADAR.c_str());
    std::ifstream inFile1(PATH_STORAGE_RADAR.c_str());
    std::string content1((std::istreambuf_iterator<char>(inFile1)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content1.empty());
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_CleanStatisticFile_001 end";
}

/**
 * @tc.name: StorageStatisticRadarTest_GetCountInfoString_001
 * @tc.desc: Verify the GetCountInfoString function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageStatisticRadarTest, StorageStatisticRadarTest_GetCountInfoString_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_GetCountInfoString_001 start";
    StorageStatisticRadar radar;
    RadarStatisticInfo info = StorageStatisticRadarTest::getRadarInfo();
    std::string expected = "1,2,3,4,5,6,7,8,9,10,11,12";
    EXPECT_EQ(expected, radar.GetCountInfoString(info));
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_GetCountInfoString_001 end";
}

/**
 * @tc.name: StorageStatisticRadarTest_CreateJsonString_001
 * @tc.desc: Verify the CreateJsonString function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageStatisticRadarTest, StorageStatisticRadarTest_CreateJsonString_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_CreateJsonString_001 start";
    StorageStatisticRadar radar;
    std::map<uint32_t, RadarStatisticInfo> statistics;
    RadarStatisticInfo info = StorageStatisticRadarTest::getRadarInfo();
    statistics.insert(std::make_pair(1, info));
    std::string expected = "{\"storageStatisticFile\":[{\"userId\":1,\"oprateCount\":\"1,2,3,4,5,6,7,8,9,10,11,12\"}]}";
    std::string jsonStr = radar.CreateJsonString(statistics);
    jsonStr.erase(std::remove_if(jsonStr.begin(), jsonStr.end(), [](char c) {
        return c == '\n' || c == '\t';
    }), jsonStr.end());
    EXPECT_EQ(expected, jsonStr);
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_CreateJsonString_001 end";
}

/**
 * @tc.name: StorageStatisticRadarTest_ReadStatisticFile_001
 * @tc.desc: Verify the ReadStatisticFile function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageStatisticRadarTest, StorageStatisticRadarTest_ReadStatisticFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_ReadStatisticFile_001 start";

    StorageStatisticRadar radar;
    std::string PATH_STORAGE_RADAR = "/data/service/el1/public/storage_daemon/radar/StorageStatisticFile.json";
    std::remove(PATH_STORAGE_RADAR.c_str());

    std::map<uint32_t, RadarStatisticInfo> statistics1;
    EXPECT_FALSE(radar.ReadStatisticFile(statistics1));
    EXPECT_TRUE(statistics1.empty());

    std::string radarInfo = "";
    SaveStringToFileSync(PATH_STORAGE_RADAR.c_str(), radarInfo);
    std::map<uint32_t, RadarStatisticInfo> statistics2;
    EXPECT_FALSE(radar.ReadStatisticFile(statistics2));
    EXPECT_TRUE(statistics2.empty());
    std::remove(PATH_STORAGE_RADAR.c_str());

    std::string radarInfo1 = "test json";
    SaveStringToFileSync(PATH_STORAGE_RADAR.c_str(), radarInfo1);
    std::map<uint32_t, RadarStatisticInfo> statistics3;
    EXPECT_FALSE(radar.ReadStatisticFile(statistics3));
    EXPECT_TRUE(statistics3.empty());
    std::remove(PATH_STORAGE_RADAR.c_str());

    std::string expected = "{\"storageStatisticFile\":\"userId\":1,\"oprateCount\":\"1,2,3,4,5,6,7,8,9,10,11,12\"}]}";
    SaveStringToFileSync(PATH_STORAGE_RADAR.c_str(), expected);
    std::remove(PATH_STORAGE_RADAR.c_str());
    EXPECT_FALSE(radar.ReadStatisticFile(statistics3));
    EXPECT_TRUE(statistics3.empty());
    std::remove(PATH_STORAGE_RADAR.c_str());

    std::map<uint32_t, RadarStatisticInfo> statistics4;
    RadarStatisticInfo info = StorageStatisticRadarTest::getRadarInfo();
    statistics4.insert(std::make_pair(1, info));
    std::string jsonStr = radar.CreateJsonString(statistics4);
    SaveStringToFileSync(PATH_STORAGE_RADAR.c_str(), jsonStr);
    std::map<uint32_t, RadarStatisticInfo> statistics;
    EXPECT_TRUE(radar.ReadStatisticFile(statistics));
    EXPECT_FALSE(statistics.empty());
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_ReadStatisticFile_001 end";
}

/**
 * @tc.name: StorageStatisticRadarTest_ParseJsonInfo_001
 * @tc.desc: Verify the ParseJsonInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageStatisticRadarTest, StorageStatisticRadarTest_ParseJsonInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_ParseJsonInfo_001 start";
    StorageStatisticRadar radar;
    std::map<uint32_t, RadarStatisticInfo> statistics;
    std::string countInfo = "1,2,3,4,5,6,7,8,9,10,11,12";
    EXPECT_TRUE(radar.ParseJsonInfo(1, countInfo, statistics));
    EXPECT_FALSE(statistics.empty());

    std::map<uint32_t, RadarStatisticInfo> statistics2;
    std::string countInfo11 = "1,2,3,4,5,6,7,8,9,10,11";
    EXPECT_FALSE(radar.ParseJsonInfo(1, countInfo11, statistics2));
    EXPECT_TRUE(statistics2.empty());
    GTEST_LOG_(INFO) << "StorageStatisticRadarTest_ParseJsonInfo_001 end";
}
} // STORAGE_DAEMON
} // OHOS
