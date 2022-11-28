/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storage/volume_storage_status_service.h"
// #include <sys/statvfs.h>

#include "storage_service_errno.h"
// #include "storage_service_log.h"
// #include "volume/volume_manager_service.h"
#include "storage/volume_storage_status_service.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;

class volume_storage_status_service_test : public testing::Test {
    public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetFreeSizeOfVolume_0000
 * @tc.name: Volume_storage_status_service_GetFreeSizeOfVolume_0000
 * @tc.desc: Test function of GetFreeSizeOfVolume interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(volume_storage_status_service_test, Volume_storage_status_service_GetFreeSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "volume_storage_status_service_test-begin Volume_storage_status_service_GetFreeSizeOfVolume_0000";
    std::shared_ptr<VolumeStorageStatusService> service = DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    string volumeUuid ="";
    int64_t result = service->GetFreeSizeOfVolume(volumeUuid);

    EXPECT_EQ(result, E_ERR);
    GTEST_LOG_(INFO) << "volume_storage_status_service_test-end Volume_storage_status_service_GetFreeSizeOfVolume_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetTotalSizeOfVolume_0000
 * @tc.name: Volume_storage_status_service_GetTotalSizeOfVolume_0000
 * @tc.desc: Test function of GetTotalSizeOfVolume interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(volume_storage_status_service_test, Volume_storage_status_service_GetTotalSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "volume_storage_status_service_test-begin Volume_storage_status_service_GetTotalSizeOfVolume_0000";
    std::shared_ptr<VolumeStorageStatusService> service = DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    string volumeUuid ="";
    int64_t result = service->GetTotalSizeOfVolume(volumeUuid);

    EXPECT_EQ(result, E_ERR);
    GTEST_LOG_(INFO) << "volume_storage_status_service_test-end Volume_storage_status_service_GetTotalSizeOfVolume_0000";
}
}