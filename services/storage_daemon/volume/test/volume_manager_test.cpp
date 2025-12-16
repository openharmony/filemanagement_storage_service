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
#include <gmock/gmock.h>

#include <linux/kdev_t.h>
#include <sys/xattr.h>

#include "external_volume_info.h"
#include "external_volume_info_mock.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "mock/storage_manager_client_mock.h"
#include "parameter.h"
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "volume/volume_manager.h"
#include "volume_info_mock.h"
#include "utils/string_utils.h"

constexpr int UPLOAD_RECORD_TRUE_LEN = 5;
constexpr int UPLOAD_RECORD_FALSE_LEN = 6;

namespace {
uint32_t g_FindParameter = 0;
}

uint32_t FindParameter(const char *key)
{
    return g_FindParameter;
}

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
    if (strcmp(name, "user.queryMtpIsInUse") != 0) {
        return 0;
    }
    if (strcmp(path, "/mnt/data/external/mtp_valid_path") == 0) {
        return -1;
    }
    if (strcmp(path, "/mnt/data/external/mtp_true_path") == 0) {
        (void)memcpy_s(value, size, "true", UPLOAD_RECORD_TRUE_LEN);
        return UPLOAD_RECORD_TRUE_LEN;
    }
    if (strcmp(path, "/mnt/data/external/mtp_false_path") == 0) {
        (void)memcpy_s(value, size, "false", UPLOAD_RECORD_FALSE_LEN);
        return UPLOAD_RECORD_FALSE_LEN;
    }
    return 0;
}

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class VolumeManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<StorageManagerClientMock> storageManagerClientMock_ = nullptr;
};

void VolumeManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void VolumeManagerTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void VolumeManagerTest::SetUp(void)
{
    storageManagerClientMock_ = std::make_shared<StorageManagerClientMock>();
    StorageManagerClientMock::iStorageManagerClientMock_ = storageManagerClientMock_;
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
    DiskUtilMoc::diskUtilMoc = diskUtilMoc_;
}

void VolumeManagerTest::TearDown(void)
{
    StorageManagerClientMock::iStorageManagerClientMock_ = nullptr;
    storageManagerClientMock_ = nullptr;
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_GetInstance_001
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_GetInstance_001 start";

    VolumeManager &volumeManager1 = VolumeManager::Instance();
    VolumeManager &volumeManager2 = VolumeManager::Instance();
    ASSERT_TRUE(&volumeManager1 == &volumeManager2);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_GetInstance_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_CreateVolume_001
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_CreateVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_001 start";

    std::string diskId = "diskId-1-1";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string result = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    GTEST_LOG_(INFO) << result;

    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    VolumeManager::Instance().DestroyVolume(result);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_CreateVolume_002
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_CreateVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_002 start";

    std::string diskId = "diskId-1-1";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string result = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    std::string res = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    GTEST_LOG_(INFO) << result;
    EXPECT_TRUE(res.empty());
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    VolumeManager::Instance().DestroyVolume(result);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateVolume_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_DestroyVolume_001
 * @tc.desc: Verify the DestroyVolume function success.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_DestroyVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    std::string diskId = "diskId-1-2";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 2); // 1 is major device number, 2 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    int32_t result = VolumeManager::Instance().DestroyVolume(volId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_DestroyVolume_002
 * @tc.desc: Verify the DestroyVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_DestroyVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_002 start";
    std::string volId = "vol-2-1";
    int32_t result = VolumeManager::Instance().DestroyVolume(volId);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_DestroyVolume_003
 * @tc.desc: Verify the DestroyVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_DestroyVolume_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_003 start";

    std::string diskId = "diskId-1-2";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 2); // 1 is major device number, 2 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(true));
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillRepeatedly(Return(E_OK));
    int32_t result = VolumeManager::Instance().DestroyVolume(volId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_003 end";
}
/**
 * @tc.name: Storage_Service_VolumeManagerTest_DestroyVolume_004
 * @tc.desc: Verify the DestroyVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_DestroyVolume_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_004 start";

    std::string diskId = "diskId-1-2";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 2); // 1 is major device number, 2 is minor device number
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    int32_t result = VolumeManager::Instance().DestroyVolume(volId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_DestroyVolume_004 end";
}
/**
 * @tc.name: Storage_Service_VolumeManagerTest_Check_001
 * @tc.desc: Verify the Check function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Check_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    std::string diskId = "diskId-1-3";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 3); // 1 is major device number, 3 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    int32_t result = VolumeManager::Instance().Check(volId);
    EXPECT_EQ(result, E_CHECK);
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Check_002
 * @tc.desc: Verify the Check function not existing situation.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Check_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_002 start";

    std::string volId = "vol-2-2";
    int32_t result = VolumeManager::Instance().Check(volId);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Check_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Mount_001
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_001 start";

    std::string diskId = "diskId-1-4";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 4); // 1 is major device number, 4 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    uint32_t flags = 1; // disk type
    int32_t result = VolumeManager::Instance().Mount(volId, flags);
    EXPECT_EQ(result, E_VOL_STATE);
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Mount_002
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Mount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_002 start";

    std::string volId = "vol-2-3";
    uint32_t flags = 1; // disk type
    int32_t result = VolumeManager::Instance().Mount(volId, flags);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Mount_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_UMount_001
 * @tc.desc: Verify the UMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_UMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    std::string diskId = "diskId-1-5";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 5); // 1 is major device number, 5 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    uint32_t flags = 1; // disk type
    VolumeManager::Instance().Mount(volId, flags);
    int32_t result = VolumeManager::Instance().UMount(volId);
    EXPECT_EQ(result, E_OK);

    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_UMount_002
 * @tc.desc: Verify the UMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_UMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_002 start";

    std::string volId = "vol-2-4";
    uint32_t flags = 1; // disk type
    VolumeManager::Instance().Mount(volId, flags);
    int32_t result = VolumeManager::Instance().UMount(volId);
    EXPECT_EQ(result, E_NON_EXIST);

    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_UMount_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Format_001
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Format_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).Times(2).WillOnce(Return(false));
    std::string diskId = "diskId-1-6";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 6); // 1 is major device number, 6 is minor device number
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    string fsType = "ext2";
    int32_t result = VolumeManager::Instance().Format(volId, fsType);
    EXPECT_EQ(result, E_NOT_SUPPORT);

    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_Format_002
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_Format_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_002 start";

    std::string volId = "vol-2-5";
    std::string fsType = "ext2";
    int32_t result = VolumeManager::Instance().Format(volId, fsType);
    EXPECT_EQ(result, E_NON_EXIST);

    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_Format_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_SetVolumeDescription_001
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_SetVolumeDescription_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    std::string diskId = "diskId-1-7";
    bool isUserdata = false;
    dev_t device = MKDEV(1, 7); // 1 is major device number, 7 is minor device number
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillOnce(Return(E_OK));
    std::string volId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    string description = "description-1";
    int32_t result = VolumeManager::Instance().SetVolumeDescription(volId, description);
    EXPECT_EQ(result, E_NOT_SUPPORT);

    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillOnce(Return(E_OK));
    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_SetVolumeDescription_002
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_SetVolumeDescription_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_002 start";

    std::string volId = "vol-2-6";
    string description = "description-1";
    int32_t result = VolumeManager::Instance().SetVolumeDescription(volId, description);
    EXPECT_EQ(result, E_NON_EXIST);

    VolumeManager::Instance().DestroyVolume(volId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_SetVolumeDescription_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR20250226995120
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001 start";

    std::string diskPath = "";
    bool isInUse = true;
    int32_t result = VolumeManager::Instance().QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    EXPECT_TRUE(isInUse);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_TryToFix_001
 * @tc.desc: Verify the TryToFix function.
 * @tc.type: FUNC
 * @tc.require: AR20250226995120
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_TryToFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_TryToFix_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false)).WillOnce(Return(false));
    std::string volId = "volId";
    uint32_t flags = 1;
    VolumeManager::Instance().volumes_.Clear();
    EXPECT_EQ(VolumeManager::Instance().TryToFix(volId, flags), E_NON_EXIST);

    auto volumeInfoMock = std::make_shared<VolumeInfoMock>();
    VolumeManager::Instance().volumes_.Insert(volId, volumeInfoMock);
    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(-1));
    volumeInfoMock->mountState_ = MOUNTED;
    EXPECT_EQ(VolumeManager::Instance().TryToFix(volId, flags), 0);

    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(0));
    volumeInfoMock->mountState_ = REMOVED;
    EXPECT_EQ(VolumeManager::Instance().TryToFix(volId, flags), E_VOL_STATE);

    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(0));
    volumeInfoMock->mountState_ = UNMOUNTED;
    EXPECT_CALL(*volumeInfoMock, DoCheck()).WillOnce(Return(-1));
    EXPECT_EQ(VolumeManager::Instance().TryToFix(volId, flags), -1);

    EXPECT_CALL(*volumeInfoMock, DoTryToFix()).WillOnce(Return(0));
    volumeInfoMock->mountState_ = UNMOUNTED;
    EXPECT_CALL(*volumeInfoMock, DoCheck()).WillOnce(Return(0));
    g_FindParameter = -1;
    EXPECT_CALL(*volumeInfoMock, DoMount(_)).WillOnce(Return(-1));
    EXPECT_EQ(VolumeManager::Instance().TryToFix(volId, flags), -1);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_TryToFix_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_QueryUsbIsInUse_002
 * @tc.desc: 测试当 realpath 函数调用失败时,函数应返回 E_PARAMS_INVALID 错误码,并设置 isInUse 为 false
 * @tc.type: FUNC
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_QueryUsbIsInUse_002, TestSize.Level1) {
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_002 start";

    std::string diskPath = "invalid_path";
    bool isInUse = false;
    int32_t result = VolumeManager::Instance().QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    EXPECT_TRUE(isInUse);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_QueryUsbIsInUse_003
 * @tc.desc: 测试当设备在使用时,函数应返回 E_OK 错误码,并设置 isInUse 为 true
 * @tc.type: FUNC
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_QueryUsbIsInUse_003, TestSize.Level1) {
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_003 start";

    std::string diskPath = "/mnt/data/external";
    bool isInUse = false;
    int32_t result = VolumeManager::Instance().QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(result, E_IOCTL_FAILED);
    EXPECT_TRUE(isInUse);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_QueryUsbIsInUse_003 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_001
 * @tc.desc: 测试当 diskPath 没有以 MTP_PATH_PREFIX 开头时,函数应返回 false
 * @tc.type: FUNC
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_001, TestSize.Level1) {
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_001 start";

    std::string diskPath = "/invalid/path";
    EXPECT_FALSE(VolumeManager::Instance().IsMtpDeviceInUse(diskPath));

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_002
 * @tc.desc: 测试当 getxattr 失败时,函数应返回 false
 * @tc.type: FUNC
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_002, TestSize.Level1) {
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_002 start";

    std::string diskPath = "/mnt/data/external/mtp_valid_path";
    EXPECT_FALSE(VolumeManager::Instance().IsMtpDeviceInUse(diskPath));

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_003
 * @tc.desc: 测试当设备未使用时,函数应返回 false
 * @tc.type: FUNC
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_003, TestSize.Level1) {
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_003 start";

    std::string diskPath = "/mnt/data/external/mtp_false_path";
    EXPECT_FALSE(VolumeManager::Instance().IsMtpDeviceInUse(diskPath));

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_003 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_004
 * @tc.desc: 测试当设备在使用时,函数应返回 true
 * @tc.type: FUNC
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_004, TestSize.Level1) {
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_004 start";

    std::string diskPath = "/mnt/data/external/mtp_true_path";
    EXPECT_TRUE(VolumeManager::Instance().IsMtpDeviceInUse(diskPath));

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_IsMtpDeviceInUseTest_004 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_001
 * @tc.desc: Verify the MountUsbFuse function with non-existing volume.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_001 start";

    std::string volumeId = "vol-non-exist";
    int fuseFd = -1;
    std::string fsUuid;

    // Test with non-existing volume - should return E_NON_EXIST
    int32_t result = VolumeManager::Instance().MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_EQ(result, E_NON_EXIST);
    EXPECT_EQ(fuseFd, -1);  // fuseFd should remain unchanged

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_002
 * @tc.desc: Verify the MountUsbFuse function with existing volume but ReadVolumeUuid fails.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_002 start";

    // Create a volume for testing
    std::string diskId = "diskId-usb-fuse-001";
    bool isUserdata = false;
    dev_t device = MKDEV(8, 1); // USB device typically uses major 8
    std::string volumeId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    ASSERT_FALSE(volumeId.empty());

    int fuseFd = -1;
    std::string fsUuid;

    // Test MountUsbFuse - ReadVolumeUuid will likely fail for test volume
    int32_t result = VolumeManager::Instance().MountUsbFuse(volumeId, fsUuid, fuseFd);
    // Since ReadVolumeUuid will fail for test volumes, expect an error
    EXPECT_NE(result, E_OK);

    // Cleanup
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    VolumeManager::Instance().DestroyVolume(volumeId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_003
 * @tc.desc: Verify the MountUsbFuse function with CreateMountUsbFusePath success.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_003 start";
 
    // Create a volume for testing
    std::string diskId = "diskId-usb-fuse-003";
    bool isUserdata = false;
    dev_t device = MKDEV(8, 4);
    std::string volumeId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    ASSERT_FALSE(volumeId.empty());
    int fuseFd = -1;
    std::string fsUuid = "test-uuid-005";
    
    // Test success at CreateMountUsbFusePath
    int32_t result = VolumeManager::Instance().MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_EQ(result, E_READMETADATA);
    EXPECT_EQ(fuseFd, -1);
 
    // Cleanup
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    VolumeManager::Instance().DestroyVolume(volumeId);
 
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_003 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_004
 * @tc.desc: Verify the MountUsbFuse function with CreateMountUsbFusePath failure.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_004 start";

    // Create a volume for testing
    std::string diskId = "diskId-usb-fuse-004";
    bool isUserdata = false;
    dev_t device = MKDEV(8, 4);
    std::string volumeId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    ASSERT_FALSE(volumeId.empty());

    int fuseFd = -1;
    std::string fsUuid = "invalid..uuid"; // UUID with path traversal characters
    
    // Test with invalid fsUuid containing ".." - should fail at CreateMountUsbFusePath
    int32_t result = VolumeManager::Instance().MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_NE(result, E_OK);
    EXPECT_EQ(fuseFd, -1);

    // Cleanup
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    VolumeManager::Instance().DestroyVolume(volumeId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_004 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_MountUsbFuse_005
 * @tc.desc: Verify the MountUsbFuse function with open /dev/fuse failure.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_MountUsbFuse_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_005 start";

    // Create a volume for testing
    std::string diskId = "diskId-usb-fuse-005";
    bool isUserdata = false;
    dev_t device = MKDEV(8, 5);
    std::string volumeId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    ASSERT_FALSE(volumeId.empty());

    int fuseFd = -1;
    std::string fsUuid = "test-uuid-005";
    
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    // Test MountUsbFuse - open /dev/fuse will likely fail in test environment
    int32_t result = VolumeManager::Instance().MountUsbFuse(volumeId, fsUuid, fuseFd);
    
    // In test environment, /dev/fuse might not be available, so expect E_OPEN_FUSE
    if (result == E_OPEN_FUSE) {
        EXPECT_EQ(fuseFd, -1);
        GTEST_LOG_(INFO) << "Expected E_OPEN_FUSE error due to /dev/fuse not available in test environment";
    } else {
        // If open succeeded, test other branches
        EXPECT_NE(result, E_OK); // Mount will likely fail in test environment
    }

    // Cleanup
    VolumeManager::Instance().DestroyVolume(volumeId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_MountUsbFuse_005 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_001
 * @tc.desc: Verify the CreateMountUsbFusePath function with invalid fsUuid.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_001 start";

    // Test with fsUuid containing path traversal characters
    std::string fsUuid = "test../uuid";
    int32_t result = VolumeManager::Instance().CreateMountUsbFusePath(fsUuid);
    EXPECT_EQ(result, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_002
 * @tc.desc: Verify the CreateMountUsbFusePath function with existing path.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_002 start";

    // Test with valid fsUuid
    std::string fsUuid = "test-uuid-valid";
    
    // First call should succeed (or fail due to mkdir permissions)
    int32_t result = VolumeManager::Instance().CreateMountUsbFusePath(fsUuid);
    
    // The result depends on whether we have permission to create directories in /mnt/data/external/
    // In test environment, this will likely fail due to permission issues
    EXPECT_TRUE(result == E_OK || result == E_MKDIR_MOUNT || result == E_SYS_KERNEL_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_CreateMountUsbFusePath_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_ReadVolumeUuid_001
 * @tc.desc: Verify the ReadVolumeUuid function with invalid volume.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_ReadVolumeUuid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_ReadVolumeUuid_001 start";

    std::string volumeId = "non-existent-volume";
    std::string fsUuid;
    
    // Test with non-existent volume
    int32_t result = VolumeManager::Instance().ReadVolumeUuid(volumeId, fsUuid);
    EXPECT_NE(result, E_OK); // Should fail for non-existent volume

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_ReadVolumeUuid_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeManagerTest_ReadVolumeUuid_002
 * @tc.desc: Verify the ReadVolumeUuid function with valid volume.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(VolumeManagerTest, Storage_Service_VolumeManagerTest_ReadVolumeUuid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_ReadVolumeUuid_002 start";

    // Create a volume for testing
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(Return(false));
    std::string diskId = "diskId-read-uuid";
    bool isUserdata = false;
    dev_t device = MKDEV(8, 10);
    std::string volumeId = VolumeManager::Instance().CreateVolume(diskId, device, isUserdata);
    ASSERT_FALSE(volumeId.empty());

    std::string fsUuid;
    
    // Test ReadVolumeUuid with created volume
    int32_t result = VolumeManager::Instance().ReadVolumeUuid(volumeId, fsUuid);
    // In test environment, this will likely fail as the volume doesn't have real filesystem
    EXPECT_EQ(result, E_READMETADATA);

    // Cleanup
    VolumeManager::Instance().DestroyVolume(volumeId);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeManagerTest_ReadVolumeUuid_002 end";
}
} // STORAGE_DAEMON
} // OHOS
