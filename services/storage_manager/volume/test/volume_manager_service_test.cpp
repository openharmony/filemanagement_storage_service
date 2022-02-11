/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "volume/volume_manager_service.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class VolumeManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0000
 * @tc.name: Volume_manager_service_Mount_0000
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0000";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "100";
    int32_t fsType = 1;
    std::string fsUuid = "100";
    std::string path = "/";
    std::string description = "100";
    std::string diskId = "100";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    if (vmService != nullptr) {
        vmService->OnVolumeCreated(vc);
        vmService->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
        vmService->Unmount(volumeId);
        result = vmService->Mount(volumeId);
    }
    EXPECT_EQ(result, 0);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Unmount_0000
 * @tc.name: Volume_manager_service_Unmount_0000
 * @tc.desc: Test function of Unmount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Unmount_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Unmount_0000";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "101";
    int32_t fsType = 1;
    std::string fsUuid = "101";
    std::string path = "/";
    std::string description = "101";
    std::string diskId = "101";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    if (vmService != nullptr) {
        vmService->OnVolumeCreated(vc);
        vmService->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
        result = vmService->Unmount(volumeId);
    }
    EXPECT_EQ(result, 0);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeCreated_0000
 * @tc.name: Volume_manager_service_OnVolumeCreated_0000
 * @tc.desc: Test function of OnVolumeCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeCreated_0000";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string id = "200";
    int type = 1;
    std::string diskId = "200";
    VolumeCore vc(id, type, diskId);
    if (vmService != nullptr) {
        vmService->OnVolumeCreated(vc);
    }
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeMounted_0000
 * @tc.name: Volume_manager_service_OnVolumeMounted_0000
 * @tc.desc: Test function of OnVolumeMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeMounted_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeMounted_0000";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "";
    int32_t fsType = 1;
    std::string fsUuid = "";
    std::string path = "";
    std::string description = "";
    if (vmService != nullptr) {
        vmService->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
    }
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeMounted_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeDestoryed_0000
 * @tc.name: Volume_manager_service_OnVolumeDestoryed_0000
 * @tc.desc: Test function of OnVolumeDestoryed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeDestoryed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeDestoryed_0000";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "";
    if (vmService != nullptr) {
        vmService->OnVolumeDestoryed(volumeId);
    }
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeDestoryed_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetAllVolumes_0000
 * @tc.name: Volume_manager_service_GetAllVolumes_0000
 * @tc.desc: Test function of GetAllVolumes interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetAllVolumes_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetAllVolumes_0000";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::vector<VolumeExternal> result = vmService->GetAllVolumes();
    EXPECT_NE(result.size(), 0);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetAllVolumes_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetVolumeByUuid_0000
 * @tc.name: Volume_manager_service_GetVolumeByUuid_0000
 * @tc.desc: Test function of GetVolumeByUuid interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetVolumeByUuid_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetVolumeByUuid_0000";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeUuid = "200";
    int32_t fsType = 1;
    std::string fsUuid = "200";
    std::string path = "/";
    std::string description = "200";
    std::string diskId = "200";
    VolumeCore vc(volumeUuid, fsType, diskId);
    vmService->OnVolumeCreated(vc);
    vmService->OnVolumeMounted(volumeUuid, fsType, fsUuid, path, description);
    std::shared_ptr<VolumeExternal> result = vmService->GetVolumeByUuid(volumeUuid);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeByUuid_0000";
}
} // namespace