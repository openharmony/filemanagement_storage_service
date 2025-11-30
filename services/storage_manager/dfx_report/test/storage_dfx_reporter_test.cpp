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

#include "dfx_report/storage_dfx_reporter.h"
#include "mock/storage_daemon_communication_mock.h"
#include "mock/storage_status_service_mock.h"
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
    void SetUp(){};
    void TearDown(){};

    static inline std::shared_ptr<StorageDaemonCommunicationMock> sdc = nullptr;
    static inline std::shared_ptr<StorageStatusServiceMock> sss = nullptr;
    static inline std::shared_ptr<StorageTotalStatusServiceMock> stss = nullptr;
};

void StorageDfxReporterTest::SetUpTestCase()
{
    sdc = std::make_shared<StorageDaemonCommunicationMock>();
    IStorageDaemonCommunicationMock::storageDaemonCommunication = sdc;
    sss = std::make_shared<StorageStatusServiceMock>();
    IStorageStatusServiceMock::storageStatusService = sss;
    stss = std::make_shared<StorageTotalStatusServiceMock>();
    StorageTotalStatusServiceBase::stss = stss;
}

void StorageDfxReporterTest::TearDownTestCase()
{
    sdc = nullptr;
    IStorageDaemonCommunicationMock::storageDaemonCommunication = nullptr;
    sss = nullptr;
    IStorageStatusServiceMock::storageStatusService = nullptr;
    stss = nullptr;
    StorageTotalStatusServiceBase::stss = nullptr;
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CheckTimeIntervalTriggered_001
 * @tc.desc: Verify the CheckTimeIntervalTriggered function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CheckTimeIntervalTriggered_001,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckTimeIntervalTriggered_001 start";
    std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::from_time_t(0);
    int64_t hoursDiff = 0;
    bool ret = StorageDfxReporter::GetInstance().CheckTimeIntervalTriggered(lastTime, 0, hoursDiff);
    EXPECT_EQ(ret, false);
    lastTime = std::chrono::system_clock::now();
    ret = StorageDfxReporter::GetInstance().CheckTimeIntervalTriggered(lastTime, 0, hoursDiff);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckTimeIntervalTriggered_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CheckValueChangeTriggered_001
 * @tc.desc: Verify the CheckValueChangeTriggered function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CheckValueChangeTriggered_001,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckValueChangeTriggered_001 start";
    int64_t valueDiff = 0;
    bool ret = StorageDfxReporter::GetInstance().CheckValueChangeTriggered(1, 0, 0, valueDiff);
    EXPECT_EQ(ret, true);
    ret = StorageDfxReporter::GetInstance().CheckValueChangeTriggered(1, 0, 100, valueDiff);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckValueChangeTriggered_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001
 * @tc.desc: Verify the CheckAndTriggerHapAndSaStatistics function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001 start";
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(Return(-1));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics());
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(DoAll(SetArgReferee<0>(-1), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics());
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics());
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CheckAndTriggerHapAndSaStatistics_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001
 * @tc.desc: Verify the ExecuteHapAndSaStatistics function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001 start";
    int32_t userId = 0;
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(-1));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().ExecuteHapAndSaStatistics(userId));
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(0));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().ExecuteHapAndSaStatistics(userId));
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_ExecuteHapAndSaStatistics_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CollectStorageStats_001
 * @tc.desc: Verify the CollectStorageStats function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CollectStorageStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CollectStorageStats_001 start";
    std::ostringstream extraData;
    int32_t userId = 100;
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(Return(0));
    EXPECT_CALL(*stss, GetSystemSize(_)).WillOnce(Return(0));
    int32_t ret = StorageDfxReporter::GetInstance().CollectStorageStats(userId, extraData);
    EXPECT_EQ(ret, 0);
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(-1));
    ret = StorageDfxReporter::GetInstance().CollectStorageStats(userId, extraData);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CollectStorageStats_001 end";
}


/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_CollectBundleStatistics_001
 * @tc.desc: Verify the CollectBundleStatistics function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_CollectBundleStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CollectBundleStatistics_001 start";
    std::ostringstream extraData;
    int32_t userId = 100;
    EXPECT_CALL(*sss, GetBundleNameAndUid(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*sdc, QueryOccupiedSpaceForSa(_, _)).WillOnce(Return(0));
    int32_t ret = StorageDfxReporter::GetInstance().CollectBundleStatistics(userId, extraData);
    EXPECT_EQ(ret, 0);

    EXPECT_CALL(*sss, GetBundleNameAndUid(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(*sdc, QueryOccupiedSpaceForSa(_, _)).WillOnce(Return(-1));
    ret = StorageDfxReporter::GetInstance().CollectBundleStatistics(userId, extraData);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_CollectBundleStatistics_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_UpdateHapAndSaState_001
 * @tc.desc: Verify the UpdateHapAndSaState function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_UpdateHapAndSaState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_UpdateHapAndSaState_001 start";
    int64_t res = -1;
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(Return(-1));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().UpdateHapAndSaState());
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(DoAll(SetArgReferee<0>(res), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().UpdateHapAndSaState());
    res = 100;
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(DoAll(SetArgReferee<0>(res), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().UpdateHapAndSaState());
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_UpdateHapAndSaState_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_ConvertBytesToMB_001
 * @tc.desc: Verify the ConvertBytesToMB function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_ConvertBytesToMB_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_ConvertBytesToMB_001 start";
    double ret = StorageDfxReporter::GetInstance().ConvertBytesToMB(-1024, 2);
    EXPECT_EQ(ret, 0.0);
    ret = StorageDfxReporter::GetInstance().ConvertBytesToMB(-1024, 2);
    EXPECT_EQ(ret, 0.0);
    ret = StorageDfxReporter::GetInstance().ConvertBytesToMB(1024, 2);
    EXPECT_EQ(ret, 0.0);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_ConvertBytesToMB_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_001
 * @tc.desc: Verify the GetStorageStatsInfo function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_001,
         TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_001 start";
    int32_t userId = 0;
    StorageStats storageStats;
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(0));
    int32_t ret = StorageDfxReporter::GetInstance().GetStorageStatsInfo(userId, storageStats);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_002
 * @tc.desc: Verify the GetStorageStatsInfo function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_002,
         TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_002 start";
    int32_t userId = 0;
    StorageStats storageStats;
    EXPECT_CALL(*sss, GetUserStorageStats(_, _, _)).WillOnce(Return(-1));
    int32_t ret = StorageDfxReporter::GetInstance().GetStorageStatsInfo(userId, storageStats);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetStorageStatsInfo_002 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_GetMetaDataSize_001
 * @tc.desc: Verify the GetMetaDataSize function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_GetMetaDataSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetMetaDataSize_001 start";
    std::ostringstream extraData;
    EXPECT_CALL(*sdc, GetDataSizeByPath(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(-1)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().GetMetaDataSize(extraData));

    EXPECT_CALL(*sdc, GetDataSizeByPath(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(-1), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().GetMetaDataSize(extraData));

    EXPECT_CALL(*sdc, GetDataSizeByPath(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().GetMetaDataSize(extraData));

    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetMetaDataSize_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDfxReporterTest_GetAncoDataSize_001
 * @tc.desc: Verify the GetAncoDataSize function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDfxReporterTest, Storage_Service_StorageDfxReporterTest_GetAncoDataSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetAncoDataSize_001 start";
    std::ostringstream extraData;
    EXPECT_CALL(*sdc, GetDataSizeByPath(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(0)));
    EXPECT_NO_FATAL_FAILURE(StorageDfxReporter::GetInstance().GetAncoDataSize(extraData));
    GTEST_LOG_(INFO) << "Storage_Service_StorageDfxReporterTest_GetAncoDataSize_001 end";
}
} // namespace StorageManager
} // namespace OHOS