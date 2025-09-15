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

#include <cstdio>
#include <gtest/gtest.h>
#include <sys/xattr.h>

#include "volume/volume_manager_service.h"
#include "disk/disk_manager_service.h"
#include "mock/file_utils_mock.h"
#include "volume_core.h"
#include "storage_service_errno.h"

int32_t g_cnt = 0;
const int32_t MTP_MAX_LEN = 512;
const int32_t CNT_ZERO = 0;
const int32_t CNT_ONE = 1;
const int32_t CNT_TWO = 2;
ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
    if (strcmp(name, "user.getfriendlyname") == 0 && g_cnt == CNT_ZERO) {
        return -1;
    }
    if (strcmp(name, "user.getfriendlyname") == 0 && g_cnt == CNT_ONE) {
        return 0;
    }
    if (strcmp(name, "user.getfriendlyname") == 0 && g_cnt == CNT_TWO) {
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
    void SetUp() {};
    void TearDown() {};
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

void VolumeManagerServiceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
}

void VolumeManagerServiceTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(testing::_)).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(testing::_)).WillOnce(testing::Return(true));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(testing::_)).WillOnce(testing::Return(true));
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_.ReadVal(volumeId);
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(testing::_)).WillOnce(testing::Return(false));
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_.ReadVal(volumeId);
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(testing::_)).WillOnce(testing::Return(true));
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_.ReadVal(volumeId);
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(testing::_)).WillOnce(testing::Return(false));
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-3";
    std::string diskId = "disk-1-3";
    VolumeCore vc(volumeId, FsType::MTP, diskId, VolumeState::UNMOUNTED);
    int32_t result;
    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_.ReadVal(volumeId);
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
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
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    int32_t result = vmService.Unmount(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});

    dmService.diskMap_.Insert(diskId, diskPtr);
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.volumeMap_.Clear();
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});

    dmService.diskMap_.Insert(diskId, diskPtr);
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.volumeMap_.Clear();
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});

    dmService.diskMap_.Insert(diskId, diskPtr);
    vmService.OnVolumeDamaged(StorageManager::VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, true});
    VolumeExternal ve;
    int32_t res = vmService.GetVolumeById(volumeId, ve);
    EXPECT_EQ(res, E_OK);
    vmService.volumeMap_.Clear();
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int32_t fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId, VolumeState::UNMOUNTED);
    vmService.OnVolumeCreated(vc);
    std::string fsUuid = "uuid-2";
    std::string description = "description-2";
    ASSERT_NE(vmService.volumeMap_.Contains(volumeId), false);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_.ReadVal(volumeId);
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).Times(2).WillOnce(testing::Return(true));
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
    g_cnt = 0;
    vmService.NotifyMtpMounted(id, path, desc, uuid);
    g_cnt = 0;
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
    g_cnt = 1;
    vmService.NotifyMtpMounted(id, path, desc, uuid);
    g_cnt = 0;
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
    g_cnt = 2;
    vmService.NotifyMtpMounted(id, path, desc, uuid);
    g_cnt = 0;
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
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(true));
    auto &vmService =VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    string fsTypes = "fs-1";
    int32_t result = vmService.Format(volumeId, fsTypes);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_Format_0003";
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
    int32_t result = vmService.MountUsbFuse(volumeId);
    EXPECT_EQ(result, E_NON_EXIST);
    vmService.volumeMap_.Erase(volumeId);
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
    int32_t result = vmService.MountUsbFuse(volumeId);
    // Check method will fail because volume is not properly setup
    EXPECT_NE(result, E_OK);
    vmService.volumeMap_.Erase(volumeId);
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
    vmService.volumeMap_.Insert(volumeId, make_shared<VolumeExternal>(vc));
    int32_t result = vmService.MountUsbFuse(volumeId);
    // The communication will fail due to lack of proper setup
    EXPECT_NE(result, E_OK);
    vmService.volumeMap_.Erase(volumeId);
    GTEST_LOG_(INFO) << "VolumeManagerServiceTest-end Volume_manager_service_MountUsbFuse_0003";
}
} // namespace
