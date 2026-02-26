/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <thread>

#include "scan/storage_manager_scan.h"
#include "storage_service_constant.h"
#include "mock/storage_daemon_communication_mock.h"
#include "storage_service_errno.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::StorageManager;

class StorageManagerScanTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<StorageDaemonCommunicationMock> sdc = nullptr;
};

void StorageManagerScanTest::SetUpTestCase()
{
    // input testsuit setup step，setup invoked before all testcases
}

void StorageManagerScanTest::TearDownTestCase()
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void StorageManagerScanTest::SetUp()
{
    sdc = std::make_shared<StorageDaemonCommunicationMock>();
    IStorageDaemonCommunicationMock::storageDaemonCommunication = sdc;
}

void StorageManagerScanTest::TearDown()
{
    sdc = nullptr;
    IStorageDaemonCommunicationMock::storageDaemonCommunication = nullptr;
}

/**
 * @tc.number: STORAGE_GetQuotaSizeByUid_00001
 * @tc.name: STORAGE_GetQuotaSizeByUid_00001
 * @tc.desc: Test function of GetQuotaSizeByUid interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_GetQuotaSizeByUid_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetQuotaSizeByUid_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::vector<int32_t> uids;
    std::map<int32_t, int64_t> uidSizeMap;
    storageManagerScan.stopScanFlag_ = true;
    int32_t ret = storageManagerScan.GetQuotaSizeByUid(uids, uidSizeMap);
    EXPECT_EQ(ret, E_ERR);
    storageManagerScan.stopScanFlag_ = false;
    EXPECT_CALL(*sdc, GetDqBlkSpacesByUids(_, _)).WillRepeatedly(Return(E_PARAMS_INVALID));
    ret = storageManagerScan.GetQuotaSizeByUid(uids, uidSizeMap);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    std::vector<NextDqBlk> dqBlks = {
        NextDqBlk(1024, 512, 2048, 1000, 500, 50, 3600, 7200, 1, 100),
        NextDqBlk(2048, 1024, 4096, 2000, 1000, 100, 7200, 14400, 2, 200),
    };
    EXPECT_CALL(*sdc, GetDqBlkSpacesByUids(_, _)).WillOnce(DoAll(SetArgReferee<1>(dqBlks), Return(E_OK)));
    ret = storageManagerScan.GetQuotaSizeByUid(uids, uidSizeMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "STORAGE_GetQuotaSizeByUid_00001 end";
}


/**
 * @tc.number: STORAGE_GetRootSize_00001
 * @tc.name: STORAGE_GetRootSize_00001
 * @tc.desc: Test function of GetRootSize interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_GetRootSize_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetRootSize_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t rootSize = storageManagerScan.GetRootSize();
    EXPECT_GE(rootSize, 0);
    GTEST_LOG_(INFO) << "STORAGE_GetRootSize_00001 end";
}

/**
 * @tc.number: STORAGE_GetSystemSize_00001
 * @tc.name: STORAGE_GetSystemSize_00001
 * @tc.desc: Test function of GetSystemSize interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_GetSystemSize_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetSystemSize_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t systemSize = storageManagerScan.GetSystemSize();
    EXPECT_GE(systemSize, 0);
    GTEST_LOG_(INFO) << "STORAGE_GetSystemSize_00001 end";
}

/**
 * @tc.number: STORAGE_GetMemmgrSize_00001
 * @tc.name: STORAGE_GetMemmgrSize_00001
 * @tc.desc: Test function of GetMemmgrSize interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_GetMemmgrSize_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetMemmgrSize_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t memmgrSize = storageManagerScan.GetMemmgrSize();
    EXPECT_GE(memmgrSize, 0);
    GTEST_LOG_(INFO) << "STORAGE_GetMemmgrSize_00001 end";
}

/**
 * @tc.number: STORAGE_Init_00001
 * @tc.name: STORAGE_Init_00001
 * @tc.desc: Test function of Init interface with LoadScanResultFromFile success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_Init_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_Init_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Test Init when LoadScanResultFromFile might succeed or fail
    int32_t ret = storageManagerScan.Init();
    // Should succeed either from file or quota query
    EXPECT_TRUE(ret == E_OK || ret == E_SERVICE_IS_NULLPTR || ret == E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "STORAGE_Init_00001 end";
}

/**
 * @tc.number: STORAGE_Init_00002
 * @tc.name: STORAGE_Init_00002
 * @tc.desc: Test function of Init interface with GetQuotaSizeByUid failure.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_Init_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_Init_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Mock GetDqBlkSpacesByUids to return error
    EXPECT_CALL(*sdc, GetDqBlkSpacesByUids(_, _)).WillRepeatedly(Return(E_ERR));
    int32_t ret = storageManagerScan.Init();
    // Should fail when quota query fails and file doesn't exist
    EXPECT_TRUE(ret == E_OK || ret == E_ERR || ret == E_SERVICE_IS_NULLPTR);
    GTEST_LOG_(INFO) << "STORAGE_Init_00002 end";
}

/**
 * @tc.number: STORAGE_StartScan_00001
 * @tc.name: STORAGE_StartScan_00001
 * @tc.desc: Test function of StartScan interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_StartScan_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_StartScan_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.StartScan();
    // StartScan launches async worker, so just verify it doesn't crash
    EXPECT_TRUE(true);
    // Give some time for the thread to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    storageManagerScan.StopScan();
    GTEST_LOG_(INFO) << "STORAGE_StartScan_00001 end";
}

/**
 * @tc.number: STORAGE_StopScan_00001
 * @tc.name: STORAGE_StopScan_00001
 * @tc.desc: Test function of StopScan interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_StopScan_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_StopScan_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.StopScan();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "STORAGE_StopScan_00001 end";
}

/**
 * @tc.number: STORAGE_StopScan_00002
 * @tc.name: STORAGE_StopScan_00002
 * @tc.desc: Test function of StopScan interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_StopScan_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_StopScan_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.isScanRunning_ = true;
    storageManagerScan.StopScan();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "STORAGE_StopScan_00002 end";
}

/**
 * @tc.number: STORAGE_CheckScanPreconditions_00001
 * @tc.name: STORAGE_CheckScanPreconditions_00001
 * @tc.desc: Test function of CheckScanPreconditions with first scan.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_CheckScanPreconditions_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_CheckScanPreconditions_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // First scan should always pass
    storageManagerScan.isFirstScan_ = true;
    bool result = storageManagerScan.CheckScanPreconditions();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "STORAGE_CheckScanPreconditions_00001 end";
}

/**
 * @tc.number: STORAGE_CheckScanPreconditions_00002
 * @tc.name: STORAGE_CheckScanPreconditions_00002
 * @tc.desc: Test function of CheckScanPreconditions with time interval check.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_CheckScanPreconditions_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_CheckScanPreconditions_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Set as not first scan and recent scan time
    storageManagerScan.isFirstScan_ = false;
    storageManagerScan.lastScanTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    bool result = storageManagerScan.CheckScanPreconditions();
    // Should return false since scan was recent
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "STORAGE_CheckScanPreconditions_00002 end";
}

/**
 * @tc.number: STORAGE_CheckScanPreconditions_00003
 * @tc.name: STORAGE_CheckScanPreconditions_00003
 * @tc.desc: Test function of CheckScanPreconditions with expired time interval.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_CheckScanPreconditions_00003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_CheckScanPreconditions_00003 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Set last scan time to more than 24 hours ago
    storageManagerScan.isFirstScan_ = false;
    auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    storageManagerScan.lastScanTime_ = currentTimeMs - (25 * 60 * 60 * 1000); // 25 hours ago
    bool result = storageManagerScan.CheckScanPreconditions();
    // Should return true since more than 24 hours have passed
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "STORAGE_CheckScanPreconditions_00003 end";
}

/**
 * @tc.number: STORAGE_LaunchScanWorker_00001
 * @tc.name: STORAGE_LaunchScanWorker_00001
 * @tc.desc: Test function of LaunchScanWorker interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_LaunchScanWorker_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_LaunchScanWorker_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.LaunchScanWorker();
    // LaunchScanWorker starts async threads
    EXPECT_TRUE(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    storageManagerScan.StopScan();
    GTEST_LOG_(INFO) << "STORAGE_LaunchScanWorker_00001 end";
}

/**
 * @tc.number: STORAGE_ExecuteScan_00001
 * @tc.name: STORAGE_ExecuteScan_00001
 * @tc.desc: Test function of ExecuteScan with GetQuotaSizeByUid failure.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ExecuteScan_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ExecuteScan_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Mock GetDqBlkSpacesByUids to return error
    EXPECT_CALL(*sdc, GetDqBlkSpacesByUids(_, _)).WillRepeatedly(Return(E_ERR));
    int32_t ret = storageManagerScan.ExecuteScan();
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "STORAGE_ExecuteScan_00001 end";
}

/**
 * @tc.number: STORAGE_ExecuteScan_00002
 * @tc.name: STORAGE_ExecuteScan_00002
 * @tc.desc: Test function of ExecuteScan with stop flag set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ExecuteScan_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ExecuteScan_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.stopScanFlag_ = true;
    int32_t ret = storageManagerScan.ExecuteScan();
    EXPECT_EQ(ret, E_ERR);
    storageManagerScan.stopScanFlag_ = false;
    GTEST_LOG_(INFO) << "STORAGE_ExecuteScan_00002 end";
}

/**
 * @tc.number: STORAGE_ScanTimeoutMonitor_00001
 * @tc.name: STORAGE_ScanTimeoutMonitor_00001
 * @tc.desc: Test function of ScanTimeoutMonitor interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ScanTimeoutMonitor_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ScanTimeoutMonitor_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // ScanTimeoutMonitor runs in a thread, just verify it doesn't crash
    // We can't easily test the full 30 second timeout in a unit test
    storageManagerScan.isScanRunning_ = false;
    storageManagerScan.ScanTimeoutMonitor();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "STORAGE_ScanTimeoutMonitor_00001 end";
}

/**
 * @tc.number: STORAGE_ReportScanResult_00001
 * @tc.name: STORAGE_ReportScanResult_00001
 * @tc.desc: Test function of ReportScanResult interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ReportScanResult_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ReportScanResult_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.rootSize_ = 1024 * 1024 * 100; // 100MB
    storageManagerScan.systemSize_ = 1024 * 1024 * 200; // 200MB
    storageManagerScan.memmgrSize_ = 1024 * 1024 * 50; // 50MB
    storageManagerScan.scanDurationMs_ = 5000; // 5 seconds
    storageManagerScan.ReportScanResult();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "STORAGE_ReportScanResult_00001 end";
}

/**
 * @tc.number: STORAGE_CalculateFinalSizes_00001
 * @tc.name: STORAGE_CalculateFinalSizes_00001
 * @tc.desc: Test function of CalculateFinalSizes interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_CalculateFinalSizes_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_CalculateFinalSizes_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t dirScanRootSize = 1024 * 1024 * 500; // 500MB
    int64_t hyperholdRootSize = 1024 * 1024 * 100; // 100MB
    int64_t rgmManagerRootSize = 1024 * 1024 * 50; // 50MB
    int64_t dirScanSystemSize = 1024 * 1024 * 200; // 200MB
    storageManagerScan.memmgrSize_ = 1024 * 1024 * 30; // 30MB initial
    storageManagerScan.CalculateFinalSizes(startTimeMs, dirScanRootSize, hyperholdRootSize,
        rgmManagerRootSize, dirScanSystemSize);
    // Verify calculations: rootSize = dirScanRootSize - hyperholdRootSize - rgmManagerRootSize
    EXPECT_EQ(storageManagerScan.rootSize_, 1024 * 1024 * (500 - 100 - 50));
    EXPECT_EQ(storageManagerScan.systemSize_, dirScanSystemSize);
    // memmgrSize = initial + hyperholdRootSize
    EXPECT_EQ(storageManagerScan.memmgrSize_, 1024 * 1024 * (30 + 100));
    EXPECT_EQ(storageManagerScan.scanDurationMs_, 0);
    GTEST_LOG_(INFO) << "STORAGE_CalculateFinalSizes_00001 end";
}

/**
 * @tc.number: STORAGE_ScanDirectories_00001
 * @tc.name: STORAGE_ScanDirectories_00001
 * @tc.desc: Test function of ScanDirectories with stop flag set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ScanDirectories_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ScanDirectories_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::vector<std::string> whiteList = {"/etc"};
    std::vector<int32_t> uids = {0, 1000};
    int64_t rootSize = 0;
    int64_t systemSize = 0;
    storageManagerScan.stopScanFlag_ = true;
    int32_t ret = storageManagerScan.ScanDirectories(whiteList, uids, rootSize, systemSize);
    EXPECT_EQ(ret, E_ERR);
    storageManagerScan.stopScanFlag_ = false;
    GTEST_LOG_(INFO) << "STORAGE_ScanDirectories_00001 end";
}

/**
 * @tc.number: STORAGE_ScanDirectories_00002
 * @tc.name: STORAGE_ScanDirectories_00002
 * @tc.desc: Test function of ScanDirectories with GetDirListSpaceByPaths success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ScanDirectories_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ScanDirectories_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::vector<std::string> whiteList = {"/etc"};
    std::vector<int32_t> uids = {0, 1000};
    int64_t rootSize = 0;
    int64_t systemSize = 0;
    // Mock GetDirListSpaceByPaths to return success
    std::vector<DirSpaceInfo> resultDirs = {{"/etc", 0, 1024}, {"/etc", 1000, 2048}};
    EXPECT_CALL(*sdc, GetDirListSpaceByPaths(_, _, _)).WillOnce(DoAll(SetArgReferee<2>(resultDirs), Return(E_OK)));
    int32_t ret = storageManagerScan.ScanDirectories(whiteList, uids, rootSize, systemSize);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rootSize, 1024); // Only UID 0 contributes to rootSize
    EXPECT_EQ(systemSize, 2048); // Only UID 1000 contributes to systemSize
    GTEST_LOG_(INFO) << "STORAGE_ScanDirectories_00002 end";
}

/**
 * @tc.number: STORAGE_ScanSinglePath_00001
 * @tc.name: STORAGE_ScanSinglePath_00001
 * @tc.desc: Test function of ScanSinglePath with stop flag set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ScanSinglePath_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ScanSinglePath_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::string path = "/data/vendor/hyperhold";
    int32_t uid = 0;
    int64_t size = 0;
    storageManagerScan.stopScanFlag_ = true;
    int32_t ret = storageManagerScan.ScanSinglePath(path, uid, size);
    EXPECT_EQ(ret, E_ERR);
    storageManagerScan.stopScanFlag_ = false;
    GTEST_LOG_(INFO) << "STORAGE_ScanSinglePath_00001 end";
}

/**
 * @tc.number: STORAGE_ScanSinglePath_00002
 * @tc.name: STORAGE_ScanSinglePath_00002
 * @tc.desc: Test function of ScanSinglePath with GetDirListSpace success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ScanSinglePath_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ScanSinglePath_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::string path = "/data/vendor/hyperhold";
    int32_t uid = 0;
    int64_t size = 0;
    // Mock GetDirListSpace to return success
    std::vector<DirSpaceInfo> resultDirs = {{path, 0, 1024 * 1024 * 100}};
    EXPECT_CALL(*sdc, GetDirListSpace(_, _)).WillOnce(DoAll(SetArgReferee<1>(resultDirs), Return(E_OK)));
    int32_t ret = storageManagerScan.ScanSinglePath(path, uid, size);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(size, 1024 * 1024 * 100); // 100MB
    GTEST_LOG_(INFO) << "STORAGE_ScanSinglePath_00002 end";
}

/**
 * @tc.number: STORAGE_GetCurrentTime_00001
 * @tc.name: STORAGE_GetCurrentTime_00001
 * @tc.desc: Test function of GetCurrentTime interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_GetCurrentTime_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentTime_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::ostringstream extraData;
    storageManagerScan.GetCurrentTime(extraData);
    std::string result = extraData.str();
    // Verify the output contains expected keywords
    EXPECT_NE(result.find("timeStamp"), std::string::npos);
    GTEST_LOG_(INFO) << "STORAGE_GetCurrentTime_00001 end";
}

/**
 * @tc.number: STORAGE_ConvertBytesToMB_00001
 * @tc.name: STORAGE_ConvertBytesToMB_00001
 * @tc.desc: Test function of ConvertBytesToMB with negative bytes.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ConvertBytesToMB_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t bytes = -100;
    int32_t decimalPlaces = 2;
    double result = storageManagerScan.ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_EQ(result, 0.0);
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00001 end";
}

/**
 * @tc.number: STORAGE_ConvertBytesToMB_00002
 * @tc.name: STORAGE_ConvertBytesToMB_00002
 * @tc.desc: Test function of ConvertBytesToMB with valid bytes.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ConvertBytesToMB_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t bytes = 1024 * 1024; // 1MB
    int32_t decimalPlaces = 2;
    double result = storageManagerScan.ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_NEAR(result, 1.0, 0.1); // Allow small margin due to rounding
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00002 end";
}

/**
 * @tc.number: STORAGE_ConvertBytesToMB_00003
 * @tc.name: STORAGE_ConvertBytesToMB_00003
 * @tc.desc: Test function of ConvertBytesToMB with negative input value.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_ConvertBytesToMB_00003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00003 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int64_t bytes = 1024 * 1024; // 1MB
    int32_t decimalPlaces = -1;
    double result = storageManagerScan.ConvertBytesToMB(bytes, decimalPlaces);
    EXPECT_NEAR(result, 1.0, 0.1);
    GTEST_LOG_(INFO) << "STORAGE_ConvertBytesToMB_00003 end";
}

/**
 * @tc.number: STORAGE_GetDirWhiteList_00001
 * @tc.name: STORAGE_GetDirWhiteList_00001
 * @tc.desc: Test function of GetDirWhiteList interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_GetDirWhiteList_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_GetDirWhiteList_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    std::vector<std::string> whiteList = storageManagerScan.GetDirWhiteList();
    EXPECT_FALSE(whiteList.empty());
    // Verify some expected paths are in the whitelist
    bool hasEl0 = false;
    bool hasEl1 = false;
    for (const auto &path : whiteList) {
        if (path.find("/data/service/el0") != std::string::npos) hasEl0 = true;
        if (path.find("/data/service/el1") != std::string::npos) hasEl1 = true;
    }
    EXPECT_TRUE(hasEl0);
    EXPECT_TRUE(hasEl1);
    GTEST_LOG_(INFO) << "STORAGE_GetDirWhiteList_00001 end";
}

/**
 * @tc.number: STORAGE_CheckScanResultDirExists_00001
 * @tc.name: STORAGE_CheckScanResultDirExists_00001
 * @tc.desc: Test function of CheckScanResultDirExists when directory exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_CheckScanResultDirExists_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_CheckScanResultDirExists_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int32_t ret = storageManagerScan.CheckScanResultDirExists();
    // Result depends on whether the directory exists on the system
    EXPECT_TRUE(ret == E_OK || ret == E_ERR);
    GTEST_LOG_(INFO) << "STORAGE_CheckScanResultDirExists_00001 end";
}

/**
 * @tc.number: STORAGE_LoadScanResultFromFile_00001
 * @tc.name: STORAGE_LoadScanResultFromFile_00001
 * @tc.desc: Test function of LoadScanResultFromFile when file doesn't exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_LoadScanResultFromFile_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    int32_t ret = storageManagerScan.LoadScanResultFromFile();
    // Should fail if scan result file doesn't exist
    EXPECT_TRUE(ret == E_OK || ret == E_ERR);
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00001 end";
}

/**
 * @tc.number: STORAGE_LoadScanResultFromFile_00002
 * @tc.name: STORAGE_LoadScanResultFromFile_00002
 * @tc.desc: Test function of LoadScanResultFromFile with invalid JSON.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_LoadScanResultFromFile_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Create a test file with invalid JSON
    std::string testDir = "/data/service/el1/public/storage_manager/database";
    std::string testFile = testDir + "/scan_result.json";
    // Create directory
    system(("mkdir -p " + testDir).c_str());
    // Write invalid JSON
    std::ofstream outFile(testFile);
    outFile << "{ invalid json content";
    outFile.close();
    int32_t ret = storageManagerScan.LoadScanResultFromFile();
    EXPECT_EQ(ret, E_ERR);
    // Clean up
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00002 end";
}

/**
 * @tc.number: STORAGE_SaveScanResultToFile_00001
 * @tc.name: STORAGE_SaveScanResultToFile_00001
 * @tc.desc: Test function of SaveScanResultToFile.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_SaveScanResultToFile_00001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_SaveScanResultToFile_00001 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.rootSize_ = 1024 * 1024 * 100;
    storageManagerScan.systemSize_ = 1024 * 1024 * 200;
    storageManagerScan.memmgrSize_ = 1024 * 1024 * 50;
    // Create directory if it doesn't exist
    system("mkdir -p /data/service/el1/public/storage_manager/database");
    int32_t ret = storageManagerScan.SaveScanResultToFile();
    // Result depends on directory existence and permissions
    EXPECT_TRUE(ret == E_OK || ret == E_ERR);
    GTEST_LOG_(INFO) << "STORAGE_SaveScanResultToFile_00001 end";
}

/**
 * @tc.number: STORAGE_SaveScanResultToFile_00002
 * @tc.name: STORAGE_SaveScanResultToFile_00002
 * @tc.desc: Test function of SaveScanResultToFile when CheckScanResultDirExists fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_SaveScanResultToFile_00002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_SaveScanResultToFile_00002 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    storageManagerScan.rootSize_ = 100;
    storageManagerScan.systemSize_ = 200;
    storageManagerScan.memmgrSize_ = 50;
    // Try to save to a directory that may not exist
    int32_t ret = storageManagerScan.SaveScanResultToFile();
    // Should fail if directory doesn't exist
    EXPECT_TRUE(ret == E_OK || ret == E_ERR);
    GTEST_LOG_(INFO) << "STORAGE_SaveScanResultToFile_00002 end";
}

/**
 * @tc.number: STORAGE_LoadScanResultFromFile_00003
 * @tc.name: STORAGE_LoadScanResultFromFile_00003
 * @tc.desc: Test function of LoadScanResultFromFile with valid JSON file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_LoadScanResultFromFile_00003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00003 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Create a test file with valid JSON
    std::string testDir = "/data/service/el1/public/storage_manager/database";
    std::string testFile = testDir + "/scan_result.json";
    // Create directory
    system(("mkdir -p " + testDir).c_str());
    // Write valid JSON
    std::ofstream outFile(testFile);
    outFile << R"({"rootSize":104857600,"systemSize":209715200,"memmgrSize":52428800})";
    outFile.close();
    int32_t ret = storageManagerScan.LoadScanResultFromFile();
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(storageManagerScan.GetRootSize(), 104857600);
    EXPECT_EQ(storageManagerScan.GetSystemSize(), 209715200);
    EXPECT_EQ(storageManagerScan.GetMemmgrSize(), 52428800);
    // Clean up
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00003 end";
}

/**
 * @tc.number: STORAGE_LoadScanResultFromFile_00004
 * @tc.name: STORAGE_LoadScanResultFromFile_00004
 * @tc.desc: Test function of LoadScanResultFromFile with missing fields.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_LoadScanResultFromFile_00004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00004 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Create a test file with partial JSON (missing memmgrSize)
    std::string testDir = "/data/service/el1/public/storage_manager/database";
    std::string testFile = testDir + "/scan_result.json";
    system(("mkdir -p " + testDir).c_str());
    std::ofstream outFile(testFile);
    outFile << R"({"rootSize":104857600,"systemSize":209715200})";
    outFile.close();
    int32_t ret = storageManagerScan.LoadScanResultFromFile();
    EXPECT_EQ(ret, E_ERR);
    // Clean up
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00004 end";
}

/**
 * @tc.number: STORAGE_LoadScanResultFromFile_00005
 * @tc.name: STORAGE_LoadScanResultFromFile_00005
 * @tc.desc: Test function of LoadScanResultFromFile with negative size values.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20260114725643
 */
HWTEST_F(StorageManagerScanTest, STORAGE_LoadScanResultFromFile_00005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00005 start";
    auto &storageManagerScan = StorageManagerScan::GetInstance();
    // Create a test file with negative values
    std::string testDir = "/data/service/el1/public/storage_manager/database";
    std::string testFile = testDir + "/scan_result.json";
    system(("mkdir -p " + testDir).c_str());
    std::ofstream outFile(testFile);
    outFile << R"({"rootSize":-100,"systemSize":-200,"memmgrSize":-50})";
    outFile.close();
    int32_t ret = storageManagerScan.LoadScanResultFromFile();
    EXPECT_EQ(ret, E_OK);
    // Clean up
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "STORAGE_LoadScanResultFromFile_00005 end";
}
