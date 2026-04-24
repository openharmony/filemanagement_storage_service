/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <atomic>
#include <chrono>
#include <cstdio>
#include <gtest/gtest.h>
#include <sys/xattr.h>
#include <thread>

#include "volume/volume_manager_service.h"
#include "disk/disk_manager_service.h"
#include "mock/file_utils_mock.h"
#include "mock/mock_parameters.h"
#include "volume_core.h"
#include "storage_service_errno.h"

int32_t g_cnt = 0;
const int32_t MTP_MAX_LEN = 512;
const int32_t CNT_ZERO = 0;
const int32_t CNT_ONE = 1;
const int32_t CNT_TWO = 2;
const std::string FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE = "const.enterprise.external_storage_device.manage.enable";

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
    if (strcmp(name, "user.getfriendlyname") == 0 && g_cnt == CNT_ZERO) {
        return -1;
    }
    if (strcmp(name, "user.getfriendlyname") == 0 && g_cnt == CNT_ONE) {
        return 0;
    }
    if (strcmp(name, "user.getfriendlyname") == 0 && g_cnt == CNT_TWO) {
        for (int i = 0; i < MTP_MAX_LEN; i++) {
            static_cast<char *>(value)[i] = 'A';
        }
        return MTP_MAX_LEN;
    }
    return 0;
}

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
using namespace StorageDaemon;
class VolumeManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

void VolumeManagerServiceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void VolumeManagerServiceTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void VolumeManagerServiceTest::SetUp(void)
{
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
}

void VolumeManagerServiceTest::TearDown()
{
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-1";
    int32_t fsType = 1;
    std::string diskId = "disk-1-1";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    result = vmService.Mount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
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
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-2";
    int32_t fsType = 1;
    std::string diskId = "disk-1-2";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    result = vmService.Mount(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0002
 * @tc.name: Volume_manager_service_Mount_0002
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0002";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    result = vmService.Mount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0003
 * @tc.name: Volume_manager_service_Mount_0003
 * @tc.desc: Test function of Mount interface for Isfuse =true,IsPathMounted=ture.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0003";
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, true);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, true);
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);
    volumePtr->SetFsType(FsType::MTP);
    result = vmService.Mount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0003";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0004
 * @tc.name: Volume_manager_service_Mount_0004
 * @tc.desc: Test function of Mount interface for Isfuse =true.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0004";
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, true);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, true);
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);
    volumePtr->SetFsType(FsType::MTP);
    result = vmService.Mount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0004";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0005
 * @tc.name: Volume_manager_service_Mount_0005
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0005";
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);
    volumePtr->SetFsType(FsType::MTP);
    result = vmService.Mount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0005";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Mount_0006
 * @tc.name: Volume_manager_service_Mount_0006
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Mount_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Mount_0006";
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);
    volumePtr->SetFsType(FsType::MTP);
    result = vmService.Mount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Mount_0006";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    int32_t fsType = 1;
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    vc.SetState(VolumeState::MOUNTED);
    result = vmService.Unmount(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_TryToFix_0001
 * @tc.name: Volume_manager_service_TryToFix_0001
 * @tc.desc: Test function of TryToFix interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_TryToFix_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_TryToFix_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-2";
    int32_t fsType = 1;
    std::string diskId = "disk-1-2";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    result = vmService.TryToFix(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_TryToFix_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_TryToFix_0002
 * @tc.name: Volume_manager_service_TryToFix_0002
 * @tc.desc: Test function of TryToFix interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_TryToFix_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_TryToFix_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    int32_t result;
    result = vmService.TryToFix(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_TryToFix_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_TryToFix_0003
 * @tc.name: Volume_manager_service_TryToFix_0003
 * @tc.desc: Test function of TryToFix interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_TryToFix_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_TryToFix_0003";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-8";
    std::string diskId = "disk-1-8";
    VolumeCore vc(volumeId, FsType::MTP, diskId);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    result = vmService.TryToFix(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_TryToFix_0003";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-4";
    int32_t fsType = 1;
    std::string diskId = "disk-1-4";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    vc.SetState(VolumeState::MOUNTED);
    vmService.OnVolumeCreated(vc);
    result = vmService.Unmount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::REMOVED);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Unmount_0002
 * @tc.name: Volume_manager_service_Unmount_0002
 * @tc.desc: Test function of Unmount interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Unmount_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Unmount_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    int32_t fsType = 1;
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, fsType, diskId);
    int32_t result;
    vc.SetState(VolumeState::MOUNTED);
    vmService.OnVolumeCreated(vc);
    result = vmService.Unmount(volumeId);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::REMOVED);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Unmount_0003
 * @tc.name: Volume_manager_service_Unmount_0003
 * @tc.desc: Test function of Unmount interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Unmount_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Unmount_0003";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-4";
    int32_t fsType = 1;
    std::string diskId = "disk-1-4";
    std::string fsTypeStr = "exfat";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    int32_t result = vmService.Unmount(volumeId);
    EXPECT_EQ(result, E_VOL_UMOUNT_ERR);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Unmount_0003";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-5";
    int type = 1;
    std::string diskId = "disk-1-5";
    VolumeCore vc(volumeId, type, diskId);
    vmService.OnVolumeCreated(vc);
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-5";
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "";
    std::string path = "";
    std::string description = "";
    vmService.OnVolumeMounted(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, false});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeMounted_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeDamaged_0000
 * @tc.name: Volume_manager_service_OnVolumeDamaged_0000
 * @tc.desc: Test function of OnVolumeDamaged interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeDamaged_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeDamaged_0000";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-5";
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "";
    std::string path = "";
    std::string description = "";
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeDamaged_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeDamaged_0001
 * @tc.name: Volume_manager_service_OnVolumeDamaged_0001
 * @tc.desc: Test function of OnVolumeDamaged interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeDamaged_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeDamaged_0001";
    auto &vmService =VolumeManagerService::GetInstance();
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";

    int64_t sizeBytes = 1024;
    std::string vendor = "vendor-6";
    int32_t flag = 1;
    Disk disk(diskId, sizeBytes, path, vendor, flag);
    VolumeCore vc(volumeId, fsType, diskId);

    auto diskPtr = std::make_shared<Disk>(disk);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});

    dmService.diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.volumeMap_.clear();
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeDamaged_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeDamaged_0002
 * @tc.name: Volume_manager_service_OnVolumeDamaged_0002
 * @tc.desc: Test function of OnVolumeDamaged interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeDamaged_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeDamaged_0002";
    auto &vmService =VolumeManagerService::GetInstance();
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";

    int64_t sizeBytes = 1024;
    std::string vendor = "vendor-6";
    int32_t flag = 2;
    Disk disk(diskId, sizeBytes, path, vendor, flag);
    VolumeCore vc(volumeId, fsType, diskId);

    auto diskPtr = std::make_shared<Disk>(disk);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});

    dmService.diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.volumeMap_.clear();
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeDamaged_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeDamaged_0003
 * @tc.name: Volume_manager_service_OnVolumeDamaged_0003
 * @tc.desc: Test function of OnVolumeDamaged interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeDamaged_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeDamaged_0003";
    auto &vmService =VolumeManagerService::GetInstance();
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";

    int64_t sizeBytes = 1024;
    std::string vendor = "vendor-6";
    int32_t flag = 3;
    Disk disk(diskId, sizeBytes, path, vendor, flag);
    VolumeCore vc(volumeId, fsType, diskId);

    auto diskPtr = std::make_shared<Disk>(disk);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});

    dmService.diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.volumeMap_.clear();
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeDamaged_0003";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-5";
    vmService.OnVolumeStateChanged(volumeId, VolumeState::BAD_REMOVAL);
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_NON_EXIST);
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::vector<VolumeExternal> result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 0);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetAllVolumes_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetAllVolumes_0001
 * @tc.name: Volume_manager_service_GetAllVolumes_0001
 * @tc.desc: Test function of GetAllVolumes interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetAllVolumes_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetAllVolumes_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::vector<VolumeExternal> result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 0);

    std::string volumeId = "vol-8-1";
    std::string diskId = "disk-8-1";
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(VolumeCore(volumeId, 1, diskId))));
    dmService.diskMap_.insert(make_pair(diskId, std::make_shared<Disk>(Disk(diskId, 1, "/", "test", 1))));
    result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 1);

    volumeId = "vol-11-1";
    diskId = "disk-11-1";
    std::shared_ptr<VolumeExternal> vc = make_shared<VolumeExternal>(VolumeCore(volumeId, UDF, diskId));
    EXPECT_NE(vc, nullptr);
    vc->SetFsType(UDF);
    vmService.volumeMap_.insert(make_pair(volumeId, vc));
    dmService.diskMap_.insert(make_pair(diskId, std::make_shared<Disk>(Disk(diskId, 1, "/", "test", 3))));
    result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 2);

    volumeId = "vol-11-2";
    diskId = "disk-11-2";
    std::shared_ptr<VolumeExternal> vc1 = make_shared<VolumeExternal>(VolumeCore(volumeId, UDF, diskId));
    EXPECT_NE(vc1, nullptr);
    vc1->SetFsType(ISO9660);
    vmService.volumeMap_.insert(make_pair(volumeId, vc1));
    dmService.diskMap_.insert(make_pair(diskId, std::make_shared<Disk>(Disk(diskId, 1, "/", "test", 3))));
    result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 3);
    vmService.volumeMap_.clear();
    dmService.diskMap_.clear();
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetAllVolumes_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetAllVolumes_0002
 * @tc.name: Volume_manager_service_GetAllVolumes_0002
 * @tc.desc: Test function of GetAllVolumes interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetAllVolumes_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetAllVolumes_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    DiskManagerService& dmService = DiskManagerService::GetInstance();

    std::string volumeId = "vol-8-1";
    std::string diskId = "disk-8-1";
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(VolumeCore(volumeId, 1, diskId))));
    dmService.diskMap_.insert(make_pair(diskId, std::make_shared<Disk>(Disk(diskId, 1, "/", "test", 1))));
    std::vector<VolumeExternal> result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 1);

    volumeId = "vol-11-1";
    diskId = "disk-11-1";
    dmService.diskMap_.insert(make_pair(diskId, std::make_shared<Disk>(Disk(diskId, 1, "/", "test", 3))));
    result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 2);

    volumeId = "vol-11-2";
    diskId = "disk-11-2";
    dmService.diskMap_.insert(make_pair(diskId, std::make_shared<Disk>(Disk(diskId, 1, "/", "test", 3))));
    result = vmService.GetAllVolumes();
    EXPECT_EQ(result.size(), 3);
    vmService.volumeMap_.clear();
    dmService.diskMap_.clear();
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetAllVolumes_0002";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    vmService.OnVolumeMounted(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, false});
    std::shared_ptr<VolumeExternal> result = vmService.GetVolumeByUuid(fsUuid);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeByUuid_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetVolumeByUuid_0001
 * @tc.name: Volume_manager_service_GetVolumeByUuid_0001
 * @tc.desc: Test function of GetVolumeByUuid interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetVolumeByUuid_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetVolumeByUuid_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-6";
    int32_t fsType = 1;
    std::string fsTypeStr = "ntfs";
    std::string fsUuid = "uuid-1";
    std::string path = "/";
    std::string description = "description-1";
    std::string diskId = "disk-1-6";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    vmService.OnVolumeMounted(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, false});
    VolumeExternal ve;
    int32_t ret = vmService.GetVolumeByUuid(fsUuid, ve);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeByUuid_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetVolumeByUuid_0002
 * @tc.name: Volume_manager_service_GetVolumeByUuid_0002
 * @tc.desc: Test function of GetVolumeByUuid interface for ERROR which volumeUuid not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetVolumeByUuid_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetVolumeByUuid_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string fsUuid = "uuid-2";
    VolumeExternal ve;
    int32_t ret = vmService.GetVolumeByUuid(fsUuid, ve);
    EXPECT_EQ(ret, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeByUuid_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetVolumeByUuid_0003
 * @tc.name: Volume_manager_service_GetVolumeByUuid_0003
 * @tc.desc: Test function of GetVolumeByUuid interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_GetVolumeByUuid_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_GetVolumeByUuid_0003";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-8";
    int32_t fsType = 5;
    std::string fsUuid = "uuid-8";
    std::string diskId = "disk-1-8";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> result = vmService.GetVolumeByUuid("uuid-9");
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_GetVolumeByUuid_0003";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-8";
    int32_t fsType = 1;
    std::string diskId = "disk-1-8";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    VolumeExternal ve;
    int32_t result = vmService.GetVolumeById(volumeId, ve);
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-9";
    VolumeExternal ve;
    int32_t result = vmService.GetVolumeById(volumeId, ve);
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-10";
    int32_t fsType = 1;
    std::string diskId = "disk-1-10";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string fsUuid = "uuid-2";
    std::string description = "description-1";
    int32_t result = vmService.SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_SetVolumeDescription_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_SetVolumeDescription_0001
 * @tc.name: Volume_manager_service_SetVolumeDescription_0001
 * @tc.desc: Test function of SetVolumeDescription interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_SetVolumeDescription_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_SetVolumeDescription_0001";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int32_t fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId, VolumeState::UNMOUNTED);
    vmService.OnVolumeCreated(vc);
    std::string fsUuid = "uuid-2";
    std::string description = "description-2";
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetFsUuid(fsUuid);
    int32_t result = vmService.SetVolumeDescription(fsUuid, description);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_SetVolumeDescription_0001";
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
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    string fsTypes = "fs-1";
    int32_t result = vmService.Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_Format_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Format_0001
 * @tc.name: Volume_manager_service_Format_0001
 * @tc.desc: Test function of Format interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_proxy_Format_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_proxy_Format_0001";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-12";
    int fsType = 1;
    std::string fsUuid = "uuid-3";
    std::string diskId = "disk-1-12";
    VolumeCore vc(volumeId, fsType, diskId, VolumeState::MOUNTED);
    vmService.OnVolumeCreated(vc);
    VolumeExternal ve;
    vmService.GetVolumeById(volumeId, ve);
    string fsTypes = "fs-1";
    int32_t result = vmService.Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_proxy_Format_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Format_0002
 * @tc.name: Volume_manager_service_Format_0002
 * @tc.desc: Test function of Format interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Format_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Format_0002";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId;
    int fsType = 1;
    std::string fsUuid = "uuid-3";
    std::string diskId = "disk-1-12";
    VolumeCore vc(volumeId, fsType, diskId, VolumeState::MOUNTED);
    vmService.OnVolumeCreated(vc);
    VolumeExternal ve;
    vmService.GetVolumeById(volumeId, ve);
    string fsTypes = "fs-1";
    int32_t result = vmService.Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Format_0002";
}
/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_NotifyMtpMounted_0001
 * @tc.name: Volume_manager_service_NotifyMtpMounted_0001
 * @tc.desc: Test function of NotifyMtpMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_NotifyMtpMounted_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_NotifyMtpMounted_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string id = "vol-1-6";
    std::string path = "/mnt/data/external/1";
    std::string desc = "description-1";
    std::string uuid = "uuid-1";
    std::string fsType = "mtp";
    vmService.NotifyMtpMounted(id, path, desc, uuid, fsType);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_NotifyMtpMounted_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_NotifyMtpMounted_0002
 * @tc.name: Volume_manager_service_NotifyMtpMounted_0002
 * @tc.desc: Test function of NotifyMtpMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_NotifyMtpMounted_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_NotifyMtpMounted_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string id = "vol-1-6";
    std::string path = "/mnt/data/external/1";
    std::string desc = "description-1";
    std::string uuid = "uuid-1";
    std::string fsType = "mtpfs";
    vmService.NotifyMtpMounted(id, path, desc, uuid, fsType);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_NotifyMtpMounted_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_NotifyMtpMounted_0003
 * @tc.name: Volume_manager_service_NotifyMtpMounted_0003
 * @tc.desc: Test function of NotifyMtpMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_NotifyMtpMounted_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_NotifyMtpMounted_0003";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string id = "vol-1-6";
    std::string path = "/mnt/data/external/1";
    std::string desc = "description-1";
    std::string uuid = "uuid-1";
    std::string fsType = "gphotofs";
    vmService.NotifyMtpMounted(id, path, desc, uuid, fsType);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_NotifyMtpMounted_0003";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Format_0003
 * @tc.name: Volume_manager_service_Format_0003
 * @tc.desc: Test function of Format interface for E_NOT_SUPPORT.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Format_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Format_0003";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    string fsTypes = "fs-1";
    int32_t result = vmService.Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Format_0003";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_IsUsbFuseByType_0001
 * @tc.name: Volume_manager_service_IsUsbFuseByType_0001
 * @tc.desc: Test function of IsUsbFuseByType interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_IsUsbFuseByType_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_IsUsbFuseByType_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string fsType = "exfat";
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, true);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, true);
    auto enabled = vmService.IsUsbFuseByType(fsType);
    EXPECT_TRUE(enabled);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_IsUsbFuseByType_Format_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_MountUsbFuse_0001
 * @tc.name: Volume_manager_service_MountUsbFuse_0001
 * @tc.desc: Test function of MountUsbFuse interface for volume state is not unmounted.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_MountUsbFuse_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_MountUsbFuse_0001";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-fuse-3";
    int32_t fsType = 1;
    std::string diskId = "disk-fuse-3";
    VolumeCore vc(volumeId, fsType, diskId);
    vc.SetState(VolumeState::MOUNTED);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    int32_t result = vmService.MountUsbFuse(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_MountUsbFuse_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_MountUsbFuse_0002
 * @tc.name: Volume_manager_service_MountUsbFuse_0002
 * @tc.desc: Test function of MountUsbFuse interface for Check method fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_MountUsbFuse_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_MountUsbFuse_0002";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-fuse-4";
    int32_t fsType = 1;
    std::string diskId = "disk-fuse-4";
    VolumeCore vc(volumeId, fsType, diskId);
    vc.SetState(VolumeState::UNMOUNTED);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    int32_t result = vmService.MountUsbFuse(volumeId);
    // Check method will fail because volume is not properly setup
    EXPECT_NE(result, E_OK);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_MountUsbFuse_0002";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_MountUsbFuse_0003
 * @tc.name: Volume_manager_service_MountUsbFuse_0003
 * @tc.desc: Test function of MountUsbFuse interface for MountUsbFuse communication fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_MountUsbFuse_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_MountUsbFuse_0003";
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-fuse-5";
    int32_t fsType = 1;
    std::string diskId = "disk-fuse-5";
    VolumeCore vc(volumeId, fsType, diskId);
    vc.SetState(VolumeState::UNMOUNTED);
    vmService.volumeMap_.insert(make_pair(volumeId, make_shared<VolumeExternal>(vc)));
    int32_t result = vmService.MountUsbFuse(volumeId);
    // The communication will fail due to lack of proper setup
    EXPECT_NE(result, E_OK);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_MountUsbFuse_0003";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyEncryptVolumeStateChanged_0001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyEncryptVolumeStateChanged_0001";
    auto &vmService =VolumeManagerService::GetInstance();
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "vol-fuse-1";
    auto volumePtr = make_shared<VolumeExternal>();
    vmService.volumeMap_.insert(make_pair("vol-fuse", volumePtr));
    vmService.NotifyEncryptVolumeStateChanged(volumeInfoStr);
    EXPECT_EQ(vmService.volumeMap_.size(), 7);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyEncryptVolumeStateChanged_0001";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyEncryptVolumeStateChanged_0002,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyEncryptVolumeStateChanged_0002";
    auto &vmService =VolumeManagerService::GetInstance();
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "vol-fuse-1";
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, nullptr));
    vmService.NotifyEncryptVolumeStateChanged(volumeInfoStr);
    EXPECT_EQ(vmService.volumeMap_.size(), 8);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyEncryptVolumeStateChanged_0002";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyEncryptVolumeStateChanged_0003,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyEncryptVolumeStateChanged_0003";
    auto &vmService =VolumeManagerService::GetInstance();
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "vol-fuse-3";
    volumeInfoStr.description = "";
    auto volumePtr = make_shared<VolumeExternal>();
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, volumePtr));
    std::string diskId = "disk-1-6";
    auto diskPtr = std::make_shared<Disk>();
    diskPtr->flag_ = SD_FLAG;
    volumePtr->diskId_ = diskId;
    DiskManagerService::GetInstance().diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.NotifyEncryptVolumeStateChanged(volumeInfoStr);
    EXPECT_EQ(vmService.volumeMap_.size(), 9);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyEncryptVolumeStateChanged_0003";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyEncryptVolumeStateChanged_0004,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyEncryptVolumeStateChanged_0004";
    auto &vmService =VolumeManagerService::GetInstance();
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "vol-fuse-3";
    volumeInfoStr.description = "";
    auto volumePtr = make_shared<VolumeExternal>();
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, volumePtr));
    std::string diskId = "disk-1-6";
    auto diskPtr = std::make_shared<Disk>();
    diskPtr->flag_ = USB_FLAG;
    volumePtr->diskId_ = diskId;
    DiskManagerService::GetInstance().diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.NotifyEncryptVolumeStateChanged(volumeInfoStr);
    EXPECT_EQ(vmService.volumeMap_.size(), 9);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyEncryptVolumeStateChanged_0004";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyEncryptVolumeStateChanged_0005,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyEncryptVolumeStateChanged_0005";
    auto &vmService =VolumeManagerService::GetInstance();
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "vol-fuse-3";
    volumeInfoStr.description = "";
    auto volumePtr = make_shared<VolumeExternal>();
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, volumePtr));
    std::string diskId = "disk-1-6";
    auto diskPtr = std::make_shared<Disk>();
    diskPtr->flag_ = CD_FLAG;
    volumePtr->diskId_ = diskId;
    DiskManagerService::GetInstance().diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.NotifyEncryptVolumeStateChanged(volumeInfoStr);
    EXPECT_EQ(vmService.volumeMap_.size(), 9);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyEncryptVolumeStateChanged_0005";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyEncryptVolumeStateChanged_0006,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyEncryptVolumeStateChanged_0006";
    auto &vmService =VolumeManagerService::GetInstance();
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "vol-fuse-3";
    volumeInfoStr.description = "";
    auto volumePtr = make_shared<VolumeExternal>();
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, volumePtr));
    std::string diskId = "disk-1-6";
    auto diskPtr = std::make_shared<Disk>();
    diskPtr->flag_ = 4;
    volumePtr->diskId_ = diskId;
    DiskManagerService::GetInstance().diskMap_.insert(make_pair(diskId, diskPtr));
    vmService.NotifyEncryptVolumeStateChanged(volumeInfoStr);
    EXPECT_EQ(vmService.volumeMap_.size(), 9);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyEncryptVolumeStateChanged_0006";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_NotifyMtpMounted_0004
 * @tc.name: Volume_manager_service_NotifyMtpMounted_0004
 * @tc.desc: Test function of NotifyMtpMounted interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_NotifyMtpMounted_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Storage_manager_NotifyMtpMounted_0004";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string id = "vol-1-7";
    std::string path = "/mnt/data/external/1";
    std::string desc = "description-1";
    std::string uuid = "uuid-1";
    std::string fsType = "mtpfs";
    g_cnt = CNT_TWO;
    vmService.NotifyMtpMounted(id, path, desc, uuid, fsType);

    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[id];
    std::string description(MTP_MAX_LEN, 'A');
    EXPECT_EQ(volumePtr->description_, description);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Storage_manager_NotifyMtpMounted_0004";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_OnVolumeStateChanged_0001
 * @tc.name: Volume_manager_service_OnVolumeStateChanged_0001
 * @tc.desc: Test function for OnVolumeStateChanged
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_OnVolumeStateChanged_0001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_OnVolumeStateChanged_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    vmService.volumeMap_.clear();
    VolumeState state = VolumeState::UNMOUNTED;
    std::string volumeId = "volumeId-nullptr";
    vmService.volumeMap_.insert(make_pair(volumeId, nullptr));
    EXPECT_EQ(vmService.volumeMap_.size(), 1);
    vmService.OnVolumeStateChanged(volumeId, state);

    vmService.volumeMap_.clear();
    EXPECT_EQ(vmService.volumeMap_.size(), 0);

    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_OnVolumeStateChanged_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Check_0001
 * @tc.name: Volume_manager_service_Check_0001
 * @tc.desc: Test function for Check
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Check_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Check_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    vmService.volumeMap_.clear();
    EXPECT_EQ(vmService.volumeMap_.size(), 0);

    std::string volumeId = "volumeId-nullptr";
    int32_t res = vmService.Check(volumeId);
    EXPECT_EQ(res, E_NON_EXIST);

    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Check_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_Format_0004
 * @tc.name: Volume_manager_service_Format_0004
 * @tc.desc: Test function for Format
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Format_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Format_0004";
    auto &vmService = VolumeManagerService::GetInstance();
    vmService.volumeMap_.clear();
    EXPECT_EQ(vmService.volumeMap_.size(), 0);
    std::string volumeId = "volumeId-nullptr";
    std::string fsType = "exfat";
    int32_t res = vmService.Format(volumeId, fsType);
    EXPECT_EQ(res, E_NON_EXIST);

    vmService.volumeMap_.insert(make_pair(volumeId, nullptr));
    EXPECT_EQ(vmService.volumeMap_.size(), 1);
    res = vmService.Format(volumeId, fsType);
    EXPECT_EQ(res, E_PARAMS_INVALID);

    vmService.volumeMap_.clear();
    EXPECT_EQ(vmService.volumeMap_.size(), 0);
    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "volumeId-1234";
    volumeInfoStr.description = "";
    auto volumePtr = make_shared<VolumeExternal>();
    volumePtr->fsType_ = FsType::MTP;
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, volumePtr));
    res = vmService.Format(volumeInfoStr.volumeId, fsType);
    EXPECT_EQ(res, E_NOT_SUPPORT);

    vmService.volumeMap_.clear();
    EXPECT_EQ(vmService.volumeMap_.size(), 0);

    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Format_0004";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_NotifyMtpUnmounted_0001
 * @tc.name: Volume_manager_service_NotifyMtpUnmounted_0001
 * @tc.desc: Test function for NotifyMtpUnmounted
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_NotifyMtpUnmounted_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_NotifyMtpUnmounted_0001";

    auto &vmService = VolumeManagerService::GetInstance();
    vmService.volumeMap_.clear();
    std::string id = "volumeId-nullptr";
    bool isBadRemove = true;
    vmService.NotifyMtpUnmounted(id, isBadRemove);
    EXPECT_EQ(vmService.volumeMap_.size(), 0);

    VolumeInfoStr volumeInfoStr;
    volumeInfoStr.volumeId = "volumeId-1234";
    volumeInfoStr.description = "";
    auto volumePtr = make_shared<VolumeExternal>();
    vmService.volumeMap_.insert(make_pair(volumeInfoStr.volumeId, volumePtr));
    EXPECT_EQ(vmService.volumeMap_.size(), 1);

    vmService.NotifyMtpUnmounted(volumeInfoStr.volumeId, isBadRemove);
    EXPECT_EQ(vmService.volumeMap_.size(), 0);

    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_NotifyMtpUnmounted_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_GetAllVolumes_ThreadSafety_0001
 * @tc.name: Volume_manager_service_GetAllVolumes_ThreadSafety_0001
 * @tc.desc: Concurrent GetAllVolumes + OnVolumeCreated + OnVolumeStateChanged(REMOVED)
 *           to verify thread safety of volumeMap_ access with mutex protection.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(VolumeManagerServiceTest, Storage_manager_GetAllVolumes_ThreadSafety_0001, testing::ext::TestSize.Level1)
{
    auto &vmService = VolumeManagerService::GetInstance();
    vmService.volumeMap_.clear();

    int32_t loopCount = 100;
    int32_t volumeCount = 20;
    std::atomic<bool> startFlag{false};
    auto getAllVolumesThread = [&startFlag, &loopCount, &vmService]() {
        while (!startFlag.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < loopCount; i++) {
            auto result = vmService.GetAllVolumes();
        }
    };

    auto insertThread = [&startFlag, &loopCount, &volumeCount, &vmService]() {
        while (!startFlag.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < loopCount; i++) {
            for (int32_t j = 0; j < volumeCount; j++) {
                std::string volumeId = "vol-thread-ins-" + std::to_string(i) + "-" + std::to_string(j);
                std::string diskId = "disk-thread-ins-" + std::to_string(i) + "-" + std::to_string(j);
                VolumeCore vc(volumeId, 1, diskId);
                auto volumePtr = make_shared<VolumeExternal>(vc);
                std::lock_guard<std::mutex> lock(vmService.volumeMapMutex_);
                vmService.volumeMap_.insert(make_pair(volumePtr->GetId(), volumePtr));
            }
        }
    };

    auto deleteThread = [&startFlag, &loopCount, &volumeCount, &vmService]() {
        while (!startFlag.load()) {
            std::this_thread::yield();
        }
        for (int32_t i = 0; i < loopCount; i++) {
            for (int32_t j = 0; j < volumeCount; j++) {
                std::string volumeId = "vol-thread-ins-" + std::to_string(i) + "-" + std::to_string(j);
                std::lock_guard<std::mutex> lock(vmService.volumeMapMutex_);
                vmService.volumeMap_.erase(volumeId);
            }
        }
    };

    std::thread t1(getAllVolumesThread);
    std::thread t2(insertThread);
    std::thread t3(deleteThread);

    startFlag.store(true);

    t1.join();
    t2.join();
    t3.join();
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Eject_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Eject_0000";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-8-1";
    int32_t result = vmService.Eject(volumeId);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Eject_0000";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Eject_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Eject_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "";
    int32_t result = vmService.Eject(volumeId);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Eject_0001";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_Eject_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_Eject_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-non-exist";
    int32_t result = vmService.Eject(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Eject_0002";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_GetOpticalDriveOpsProgress_0000,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_GetOpticalDriveOpsProgress_0000";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-8-1";
    uint32_t progress = 0;
    int32_t result = vmService.GetOpticalDriveOpsProgress(volumeId, progress);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_GetOpticalDriveOpsProgress_0000";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_GetOpticalDriveOpsProgress_0001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_GetOpticalDriveOpsProgress_0001";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "";
    uint32_t progress = 0;
    int32_t result = vmService.GetOpticalDriveOpsProgress(volumeId, progress);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_GetOpticalDriveOpsProgress_0001";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_GetOpticalDriveOpsProgress_0002,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_GetOpticalDriveOpsProgress_0002";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-non-exist";
    uint32_t progress = 100;
    int32_t result = vmService.GetOpticalDriveOpsProgress(volumeId, progress);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_GetOpticalDriveOpsProgress_0002";
}

HWTEST_F(VolumeManagerServiceTest, Volume_manager_service_GetOpticalDriveOpsProgress_0003,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-begin Volume_manager_service_GetOpticalDriveOpsProgress_0003";
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-8-2";
    uint32_t progress = 50;
    int32_t result = vmService.GetOpticalDriveOpsProgress(volumeId, progress);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_GetOpticalDriveOpsProgress_0003";
}
} // namespace
