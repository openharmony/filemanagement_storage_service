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

#include "storage_service_errno.h"
#include "storage/volume_storage_status_service.h"
#include "volume/volume_manager_service.h"
#include "volume_core.h"

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
 * @tc.desc: Test GetFreeSizeOfVolume with empty UUID should return E_NON_EXIST.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetFreeSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetFreeSizeOfVolume_0000 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    string volumeUuid ="";
    int64_t freeSize;
    int32_t result = service.GetFreeSizeOfVolume(volumeUuid, freeSize);

    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetFreeSizeOfVolume_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetTotalSizeOfVolume_0000
 * @tc.name: Volume_storage_status_service_GetTotalSizeOfVolume_0000
 * @tc.desc: Test GetTotalSizeOfVolume with empty UUID should return E_NON_EXIST.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetTotalSizeOfVolume_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetTotalSizeOfVolume_0000 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    string volumeUuid ="";
    int64_t totalSize;
    int32_t result = service.GetTotalSizeOfVolume(volumeUuid, totalSize);

    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetTotalSizeOfVolume_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetFreeSizeOfVolume_0001
 * @tc.name: Volume_storage_status_service_GetFreeSizeOfVolume_0001
 * @tc.desc: Test GetFreeSizeOfVolume with non-existent volume UUID should return E_NON_EXIST.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetFreeSizeOfVolume_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetFreeSizeOfVolume_0001 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string nonExistentUuid = "uuid-does-not-exist-12345";
    int64_t freeSize = 0;
    int32_t result = service.GetFreeSizeOfVolume(nonExistentUuid, freeSize);

    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetFreeSizeOfVolume_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetTotalSizeOfVolume_0001
 * @tc.name: Volume_storage_status_service_GetTotalSizeOfVolume_0001
 * @tc.desc: Test GetTotalSizeOfVolume with non-existent volume UUID should return E_NON_EXIST.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetTotalSizeOfVolume_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetTotalSizeOfVolume_0001 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string nonExistentUuid = "uuid-does-not-exist-67890";
    int64_t totalSize = 0;
    int32_t result = service.GetTotalSizeOfVolume(nonExistentUuid, totalSize);

    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetTotalSizeOfVolume_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetVolumePath_0001
 * @tc.name: Volume_storage_status_service_GetVolumePath_0001
 * @tc.desc: Test GetVolumePath with non-existent volume UUID should return empty string.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetVolumePath_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumePath_0001 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string nonExistentUuid = "uuid-path-not-exist";
    string result = service.GetVolumePath(nonExistentUuid);

    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumePath_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetVolumePath_0002
 * @tc.name: Volume_storage_status_service_GetVolumePath_0002
 * @tc.desc: Test GetVolumePath with empty UUID should return empty string.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetVolumePath_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumePath_0002 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string emptyUuid = "";
    string result = service.GetVolumePath(emptyUuid);

    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumePath_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0004
 * @tc.name: Volume_storage_status_service_IsOddDevice_0004
 * @tc.desc: Test IsOddDevice with non-existent UUID should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0004 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string nonExistentUuid = "uuid-odd-not-exist";
    bool result = service.IsOddDevice(nonExistentUuid);

    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0004 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0005
 * @tc.name: Volume_storage_status_service_IsOddDevice_0005
 * @tc.desc: Test IsOddDevice with empty UUID should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0005 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string emptyUuid = "";
    bool result = service.IsOddDevice(emptyUuid);

    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0005 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetVolumeById_0002
 * @tc.name: Volume_storage_status_service_GetVolumeById_0002
 * @tc.desc: Test GetVolumeById with non-existent UUID should return E_NON_EXIST.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetVolumeById_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumeById_0002 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string nonExistentUuid = "uuid-by-id-not-exist";
    VolumeExternal result;
    int32_t ret = service.GetVolumeById(nonExistentUuid, result);

    EXPECT_EQ(ret, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumeById_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0006
 * @tc.name: Volume_storage_status_service_IsOddDevice_0006
 * @tc.desc: Test IsOddDevice with UDF filesystem type should return true.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0006 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-udf-test-1";
    int32_t fsType = FsType::UDF;
    string diskId = "disk-udf-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::UDF);
    volumePtr->SetFsUuid("test-uuid-udf");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, true);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0006 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0007
 * @tc.name: Volume_storage_status_service_IsOddDevice_0007
 * @tc.desc: Test IsOddDevice with ISO9660 filesystem type should return true.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0007 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-iso-test-1";
    int32_t fsType = FsType::ISO9660;
    string diskId = "disk-iso-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::ISO9660);
    volumePtr->SetFsUuid("test-uuid-iso");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, true);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0007 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0008
 * @tc.name: Volume_storage_status_service_IsOddDevice_0008
 * @tc.desc: Test IsOddDevice with NTFS filesystem type should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0008 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-ntfs-test-1";
    int32_t fsType = FsType::NTFS;
    string diskId = "disk-ntfs-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::NTFS);
    volumePtr->SetFsUuid("test-uuid-ntfs");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, false);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0008 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0009
 * @tc.name: Volume_storage_status_service_IsOddDevice_0009
 * @tc.desc: Test IsOddDevice with EXFAT filesystem type should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0009 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-exfat-test-1";
    int32_t fsType = FsType::EXFAT;
    string diskId = "disk-exfat-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::EXFAT);
    volumePtr->SetFsUuid("test-uuid-exfat");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, false);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0009 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0010
 * @tc.name: Volume_storage_status_service_IsOddDevice_0010
 * @tc.desc: Test IsOddDevice with MTP filesystem type should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0010 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-mtp-test-1";
    int32_t fsType = FsType::MTP;
    string diskId = "disk-mtp-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::MTP);
    volumePtr->SetFsUuid("test-uuid-mtp");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, false);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0010 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0011
 * @tc.name: Volume_storage_status_service_IsOddDevice_0011
 * @tc.desc: Test IsOddDevice with PTP filesystem type should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0011 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-ptp-test-1";
    int32_t fsType = FsType::PTP;
    string diskId = "disk-ptp-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::PTP);
    volumePtr->SetFsUuid("test-uuid-ptp");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, false);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0011 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_IsOddDevice_0012
 * @tc.name: Volume_storage_status_service_IsOddDevice_0012
 * @tc.desc: Test IsOddDevice with VFAT filesystem type should return false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_IsOddDevice_0012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0012 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-vfat-test-1";
    int32_t fsType = FsType::VFAT;
    string diskId = "disk-vfat-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsType(FsType::VFAT);
    volumePtr->SetFsUuid("test-uuid-vfat");

    bool result = service.IsOddDevice(volumePtr->GetUuid());

    EXPECT_EQ(result, false);
    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_IsOddDevice_0012 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetVolumeById_0003
 * @tc.name: Volume_storage_status_service_GetVolumeById_0003
 * @tc.desc: Test GetVolumeById with existing volume should return E_OK.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetVolumeById_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumeById_0003 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();

    string volumeId = "vol-byid-test-1";
    int32_t fsType = 1;
    string diskId = "disk-byid-test";
    VolumeCore vc(volumeId, fsType, diskId);

    vmService.OnVolumeCreated(vc);
    std::shared_ptr<VolumeExternal> volumePtr = vmService.volumeMap_[volumeId];
    ASSERT_NE(volumePtr, nullptr);

    volumePtr->SetFsUuid("test-uuid-byid");

    VolumeExternal result;
    int32_t ret = service.GetVolumeById(volumePtr->GetUuid(), result);

    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(result.GetUuid(), "test-uuid-byid");

    vmService.volumeMap_.erase(volumeId);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumeById_0003 end";
}

/**
 * @tc.number: SUB_STORAGE_Volume_storage_status_service_GetVolumeById_0004
 * @tc.name: Volume_storage_status_service_GetVolumeById_0004
 * @tc.desc: Test GetVolumeById with empty UUID should return E_NON_EXIST.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000H0372
 */
HWTEST_F(VolumeStorageStatusServiceTest,
    Volume_storage_status_service_GetVolumeById_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumeById_0004 start";
    VolumeStorageStatusService& service = VolumeStorageStatusService::GetInstance();

    string emptyUuid = "";
    VolumeExternal result;
    int32_t ret = service.GetVolumeById(emptyUuid, result);

    EXPECT_EQ(ret, E_NON_EXIST);
    GTEST_LOG_(INFO) << "Volume_storage_status_service_GetVolumeById_0004 end";
}
}
}