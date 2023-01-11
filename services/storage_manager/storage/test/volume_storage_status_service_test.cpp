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

#include "storage_service_errno.h"
#include "storage/volume_storage_status_service.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;

class VolumeStorageStatusServiceTest : public testing::Test {
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
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetFreeSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetFreeSizeOfVolume_0000 start";
    std::shared_ptr<VolumeStorageStatusService> service = DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    string volumeUuid ="";
    int64_t freeSize;
    int32_t result = service->GetFreeSizeOfVolume(volumeUuid, freeSize);

    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetFreeSizeOfVolume_0000 end";
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
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetTotalSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetTotalSizeOfVolume_0000 start";
    std::shared_ptr<VolumeStorageStatusService> service = DelayedSingleton<VolumeStorageStatusService>::GetInstance();
    string volumeUuid ="";
    int64_t totalSize;
    int32_t result = service->GetTotalSizeOfVolume(volumeUuid, totalSize);

    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetTotalSizeOfVolume_0000 end";
}
}