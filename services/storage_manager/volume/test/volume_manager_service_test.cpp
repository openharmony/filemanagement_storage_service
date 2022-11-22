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
#include "volume_core.h"
#include "storage_service_errno.h"

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
    std::string volumeId = "vol-1-1";
    int32_t fsType = 1;
    std::string diskId = "disk-1-1";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    if (vmService != nullptr) {
        vmService->OnVolumeCreated(vc);
        result = vmService->Mount(volumeId);
        vmService->OnVolumeDestroyed(volumeId);
    }
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0001
 * @tc.name: Volume_manager_service_Mount_0001
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0001";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-2";
    int32_t fsType = 1;
    std::string diskId = "disk-1-2";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    if (vmService != nullptr) {
        result = vmService->Mount(volumeId);
    }
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Unmount_0000
 * @tc.name: Volume_manager_service_Unmount_0000
 * @tc.desc: Test function of Unmount interface for FAILED.
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
    std::string volumeId = "vol-1-3";
    int32_t fsType = 1;
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    if (vmService != nullptr) {
        vc.SetState(VolumeState::MOUNTED);
        result = vmService->Unmount(volumeId);
    }
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Unmount_0001
 * @tc.name: Volume_manager_service_Unmount_0001
 * @tc.desc: Test function of Unmount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Unmount_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Unmount_0001";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-4";
    int32_t fsType = 1;
    std::string diskId = "disk-1-4";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    if (vmService != nullptr) {
        vc.SetState(VolumeState::MOUNTED);
        vmService->OnVolumeCreated(vc);
        result = vmService->Unmount(volumeId);
        vmService->OnVolumeDestroyed(volumeId);
    }
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0001";
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
    std::string volumeId = "vol-1-5";
    int type = 1;
    std::string diskId = "disk-1-5";
    VolumeCore vc(volumeId, type, diskId);
    if (vmService != nullptr) {
        vmService->OnVolumeCreated(vc);
        vmService->OnVolumeDestroyed(volumeId);
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
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeDestroyed_0000
 * @tc.name: Volume_manager_service_OnVolumeDestroyed_0000
 * @tc.desc: Test function of OnVolumeDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeDestroyed_0000";
    std::shared_ptr<VolumeManagerService> vmService =
        DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "";
    if (vmService != nullptr) {
        vmService->OnVolumeDestroyed(volumeId);
    }
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeDestroyed_0000";
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
    EXPECT_EQ(result.size(), 0);
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
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService->OnVolumeCreated(vc);
    vmService->OnVolumeMounted(volumeId, fsType, fsUuid, path, description);
    std::shared_ptr<VolumeExternal> result = vmService->GetVolumeByUuid(fsUuid);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeByUuid_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetVolumeById_0000
 * @tc.name: Volume_manager_service_GetVolumeById_0000
 * @tc.desc: Test function of GetVolumeById interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetVolumeById_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetVolumeById_0000";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-8";
    int32_t fsType = 1;
    std::string diskId = "disk-1-8";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService->OnVolumeCreated(vc);
    VolumeExternal ve;
    int32_t result = vmService->GetVolumeById(volumeId, ve);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeById_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetVolumeById_0001
 * @tc.name: Volume_manager_service_GetVolumeById_0001
 * @tc.desc: Test function of GetVolumeById interface for ERROR which volumeId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetVolumeById_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetVolumeById_0001";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-9";
    VolumeExternal ve;
    int32_t result = vmService->GetVolumeById(volumeId, ve);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeById_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_SetVolumeDescription_0000
 * @tc.name: Volume_manager_service_SetVolumeDescription_0000
 * @tc.desc: Test function of SetVolumeDescription interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_SetVolumeDescription_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_SetVolumeDescription_0000";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-10";
    int32_t fsType = 1;
    std::string diskId = "disk-1-10";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService->OnVolumeCreated(vc);
    std::string fsUuid = "uuid-2";
    std::string description = "description-1";
    int32_t result = vmService->SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_SetVolumeDescription_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Format_0000
 * @tc.name: Volume_manager_service_Format_0000
 * @tc.desc: Test function of Format interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_Format_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_Format_0000";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService->OnVolumeCreated(vc);
    string fsTypes = "fs-1";
    int32_t result = vmService->Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_Format_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Format_0001
 * @tc.name: Volume_manager_service_Format_0000
 * @tc.desc: Test function of Format interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_Format_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_Format_0001";
    std::shared_ptr<VolumeManagerService> vmService =
            DelayedSingleton<VolumeManagerService>::GetInstance();
    std::string volumeId = "vol-1-12";
    int fsType = 1;
    std::string fsUuid = "uuid-3";
    std::string diskId = "disk-1-12";
    VolumeCore vc(volumeId, fsType, diskId, VolumeState::MOUNTED);
    vmService->OnVolumeCreated(vc);
    VolumeExternal ve;
    vmService->GetVolumeById(volumeId, ve);
    string fsTypes = "fs-1";
    int32_t result = vmService->Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_Format_0001";
}
} // namespace
