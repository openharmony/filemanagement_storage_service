/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <thread>
#include <chrono>

#include "dfx_report/storage_dfx_reporter.h"
#include "mock/storage_daemon_communication_mock.h"
#include "mock/storage_status_manager_mock.h"
#include "statistic_info.h"
#include "storage_total_status_service_mock.h"

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;
using namespace testing;


class StorageDfxReporterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown(){};

    static inline std::shared_ptr<StorageDaemonCommunicationMock> sdc = nullptr;
    static inline std::shared_ptr<StorageStatusManagerMock> sss = nullptr;
    static inline std::shared_ptr<StorageTotalStatusServiceMock> stss = nullptr;
};

void StorageDfxReporterTest::SetUp()
{
    StorageDfxReporter::GetInstance().lastTotalSize_ = 0;
    // Ensure no background threads are running
    StorageDfxReporter::GetInstance().isHapAndSaRunning_.store(false);
    StorageDfxReporter::GetInstance().isScanRunning_.store(false);
}

void StorageDfxReporterTest::SetUpTestCase()
{
    sdc = std::make_shared<StorageDaemonCommunicationMock>();
    IStorageDaemonCommunicationMock::storageDaemonCommunication = sdc;
    sss = std::make_shared<StorageStatusManagerMock>();
    IStorageStatusManagerMock::storageStatusManager = sss;
    stss = std::make_shared<StorageTotalStatusServiceMock>();
    StorageTotalStatusServiceBase::stss = stss;
}

void StorageDfxReporterTest::TearDownTestCase()
{
    sdc = nullptr;
    IStorageDaemonCommunicationMock::storageDaemonCommunication = nullptr;
    sss = nullptr;
    IStorageStatusManagerMock::storageStatusManager = nullptr;
    stss = nullptr;
    StorageTotalStatusServiceBase::stss = nullptr;
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001
 * @tc.desc: Verify the CheckAndTriggerHapAndSaStatistics function.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001 start";

    bool runningBefore = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(Return(-1));
    StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics();
    bool runningAfter = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_EQ(runningBefore, runningAfter);

    runningBefore = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(DoAll(SetArgReferee<0>(-1), Return(0)));
    StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics();
    runningAfter = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_EQ(runningBefore, runningAfter);

    StorageDfxReporter::GetInstance().lastHapAndSaFreeSize_ = 1000;
    StorageDfxReporter::GetInstance().lastHapAndSaTime_ = std::chrono::system_clock::time_point();
    runningBefore = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1100), Return(0)));
    StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics();
    runningAfter = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_EQ(runningBefore, runningAfter);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_002
 * @tc.desc: Verify the CheckAndTriggerHapAndSaStatistics function when task is already running.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_002,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_002 start";

    StorageDfxReporter::GetInstance().isHapAndSaRunning_.store(true);
    EXPECT_EQ(StorageDfxReporter::GetInstance().isHapAndSaRunning_.load(), true);
    StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics();
    EXPECT_EQ(StorageDfxReporter::GetInstance().isHapAndSaRunning_.load(), true);

    // Reset for next test
    StorageDfxReporter::GetInstance().isHapAndSaRunning_.store(false);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_002 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_StartReportHapAndSaStorageStatus_001
 * @tc.desc: Verify the StartReportHapAndSaStorageStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_StartReportHapAndSaStorageStatus_001,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_StartReportHapAndSaStorageStatus_001 start";

    // Set up mock expectations for the background thread
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1000), Return(0)));
    EXPECT_CALL(*stss, GetSystemSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(500), Return(0)));
    EXPECT_CALL(*sdc, GetDataSizeByPath(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(100), Return(0)));
    EXPECT_CALL(*sdc, GetRmgResourceSize(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(50), Return(0)));
    EXPECT_CALL(*sss, GetBundleNameAndUid(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*sdc, QueryOccupiedSpaceForSa(_, _)).WillRepeatedly(Return(0));

    StorageDfxReporter::GetInstance().isHapAndSaRunning_.store(false);
    StorageDfxReporter::GetInstance().StartReportHapAndSaStorageStatus();
    EXPECT_EQ(StorageDfxReporter::GetInstance().isHapAndSaRunning_.load(), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(StorageDfxReporter::GetInstance().isHapAndSaRunning_.load(), false);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_StartReportHapAndSaStorageStatus_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001
 * @tc.desc: Verify the ExecuteHapAndSaStatistics function.
 * @tc.type: FUNC
 * @tc.require: AR000XXXX
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001 start";
    int32_t userId = 0;

    // Test case 1: GetUserStorageStats fails, should return early
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(-1));
    bool runningBefore = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    StorageDfxReporter::GetInstance().ExecuteHapAndSaStatistics(userId);
    bool runningAfter = StorageDfxReporter::GetInstance().isHapAndSaRunning_.load();
    EXPECT_EQ(runningBefore, runningAfter);

    // Test case 2: Success path, need to mock all methods called
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1000), Return(0)));
    EXPECT_CALL(*stss, GetSystemSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(500), Return(0)));
    EXPECT_CALL(*sdc, GetDataSizeByPath(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(100), Return(0)));
    EXPECT_CALL(*sdc, GetRmgResourceSize(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(50), Return(0)));
    EXPECT_CALL(*sss, GetBundleNameAndUid(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*sdc, QueryOccupiedSpaceForSa(_, _)).WillRepeatedly(Return(0));
    StorageDfxReporter::GetInstance().ExecuteHapAndSaStatistics(userId);

    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001 end";
}
} // namespace StorageManager
} // namespace OHOS