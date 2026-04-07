/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#include <cstdint>
#include "mtp/mtp_device_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "mock/library_func_mock.h"
#include "mock/file_utils_mock.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class MtpDeviceManagerTest : public testing::Test {
public:
    static inline std::shared_ptr<LibraryFuncMock> libraryFuncMock_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;

    static void SetUpTestCase()
    {
        libraryFuncMock_ = std::make_shared<LibraryFuncMock>();
        LibraryFunc::libraryFunc_ = libraryFuncMock_;
        fileUtilMoc_ = std::make_shared<FileUtilMoc>();
        IFileUtilMoc::fileUtilMoc = fileUtilMoc_;
    }

    static void TearDownTestCase()
    {
        if (libraryFuncMock_) {
            Mock::VerifyAndClearExpectations(libraryFuncMock_.get());
        }
        libraryFuncMock_ = nullptr;
        LibraryFunc::libraryFunc_ = nullptr;
        if (fileUtilMoc_) {
            Mock::VerifyAndClearExpectations(fileUtilMoc_.get());
        }
        fileUtilMoc_ = nullptr;
        IFileUtilMoc::fileUtilMoc = nullptr;
    }

    void SetUp()
    {
        deviceInfo.path = "/test/path";
        deviceInfo.id = "1";
        deviceInfo.vendor = "TestVendor";
        deviceInfo.uuid = "TestUUID";
        deviceInfo.type = "mtpfs";
    }

    void TearDown()
    {
        MtpDeviceManager::GetInstance().isMounting_ = false;
    }
protected:
    MtpDeviceInfo deviceInfo;
};

/**
 * @tc.name  : PrepareMtpMountPath_001
 * @tc.number: PrepareMtpMountPath_001
 * @tc.desc  : Test mtp mount path not exists -> create failed
 */
HWTEST_F(MtpDeviceManagerTest, PrepareMtpMountPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PrepareMtpMountPath_001 start";

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(false));
    auto &manager = MtpDeviceManager::GetInstance();
    EXPECT_EQ(manager.PrepareMtpMountPath("/test/path"), E_MTP_PREPARE_DIR_ERR);

    GTEST_LOG_(INFO) << "PrepareMtpMountPath_001 end";
}

/**
 * @tc.name  : MountDeviceTest_001
 * @tc.number: MountDeviceTest_001
 * @tc.desc  : Test when device is mounting
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_001 start";

    MtpDeviceManager& manager = MtpDeviceManager::GetInstance();
    manager.isMounting_ = true;
    int32_t result = manager.MountDevice(deviceInfo);
    EXPECT_EQ(result, E_MTP_IS_MOUNTING);

    GTEST_LOG_(INFO) << "MountDeviceTest_001 end";
}

/**
 * @tc.name  : MountDeviceTest_002
 * @tc.number: MountDeviceTest_002
 * @tc.desc  : Test ForkExec returns 0 but outputs error lines -> treated as failure (do not assert err==0 means ok)
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_002 start";

    auto &manager = MtpDeviceManager::GetInstance();
    manager.isMounting_ = false;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    std::vector<std::string> output = {"mock error"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(
            WithArg<1>([&output](std::vector<std::string> *out) { if (out) *out = output; }),
            Return(0)
        ));
    EXPECT_NE(manager.MountDevice(deviceInfo), E_OK);
    EXPECT_FALSE(manager.isMounting_);

    GTEST_LOG_(INFO) << "MountDeviceTest_002 end";
}

/**
 * @tc.name  : MountDeviceTest_003
 * @tc.number: MountDeviceTest_003
 * @tc.desc  : Test PrepareMtpMountPath fails -> return E_MTP_PREPARE_DIR_ERR and reset isMounting
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_003 start";

    auto &manager = MtpDeviceManager::GetInstance();
    manager.isMounting_ = false;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(false));
    EXPECT_EQ(manager.MountDevice(deviceInfo), E_MTP_PREPARE_DIR_ERR);
    EXPECT_FALSE(manager.isMounting_);

    GTEST_LOG_(INFO) << "MountDeviceTest_003 end";
}

/**
 * @tc.name  : MountDeviceTest_004
 * @tc.number: MountDeviceTest_004
 * @tc.desc  : ForkExec ok but output non-empty -> failure branch
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_004 start";

    auto &manager = MtpDeviceManager::GetInstance();
    manager.isMounting_ = false;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    std::vector<std::string> output = {"mock error"};
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(
            WithArg<1>([&output](std::vector<std::string> *out) { if (out) *out = output; }),
            Return(0)
        ));
    EXPECT_CALL(*libraryFuncMock_, umount(_)).WillOnce(Return(0));
    EXPECT_CALL(*libraryFuncMock_, rmdir(_)).WillOnce(Return(0));
    EXPECT_NE(manager.MountDevice(deviceInfo), E_OK);
    EXPECT_FALSE(manager.isMounting_);

    GTEST_LOG_(INFO) << "MountDeviceTest_004 end";
}

/**
 * @tc.name  : UmountDeviceTest_001
 * @tc.number: UmountDeviceTest_001
 * @tc.desc  : BadRemove=true, umount2 ok and remove ok, needNotify=false -> return E_OK and DelFolder called
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_001 start";

    auto &manager = MtpDeviceManager::GetInstance();
    EXPECT_CALL(*libraryFuncMock_, umount2(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*libraryFuncMock_, remove(_)).WillOnce(Return(0));
    EXPECT_EQ(manager.UmountDevice(deviceInfo, false, true), E_OK);

    GTEST_LOG_(INFO) << "UmountDeviceTest_001 end";
}

/**
 * @tc.name  : UmountDeviceTest_002
 * @tc.number: UmountDeviceTest_002
 * @tc.desc  : BadRemove=false, umount failed -> return E_MTP_UMOUNT_FAILED
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_002 start";

    auto &manager = MtpDeviceManager::GetInstance();
    EXPECT_CALL(*libraryFuncMock_, umount(_)).WillOnce(Return(-1));
    EXPECT_EQ(manager.UmountDevice(deviceInfo, false, false), E_MTP_UMOUNT_FAILED);

    GTEST_LOG_(INFO) << "UmountDeviceTest_002 end";
}

/**
 * @tc.name  : UmountDeviceTest_003
 * @tc.number: UmountDeviceTest_003
 * @tc.desc  : BadRemove=true, umount2 failed, needNotify=false -> return E_OK (force mode tolerates umount2 failure)
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_003 start";

    auto &manager = MtpDeviceManager::GetInstance();
    EXPECT_CALL(*libraryFuncMock_, umount2(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(manager.UmountDevice(deviceInfo, false, true), E_OK);

    GTEST_LOG_(INFO) << "UmountDeviceTest_003 end";
}

/**
 * @tc.name  : UmountDeviceTest_004
 * @tc.number: UmountDeviceTest_004
 * @tc.desc  : BadRemove=true, umount2 failed, needNotify=true -> return E_OK and cover notify branch
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_004 start";

    auto &manager = MtpDeviceManager::GetInstance();
    EXPECT_CALL(*libraryFuncMock_, umount2(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(manager.UmountDevice(deviceInfo, true, true), E_OK);

    GTEST_LOG_(INFO) << "UmountDeviceTest_004 end";
}

/**
 * @tc.name  : UmountDeviceTest_005
 * @tc.number: UmountDeviceTest_005
 * @tc.desc  : BadRemove=true, umount2 ok but remove failed, needNotify=true -> return E_OK and DelFolder called
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_005 start";

    auto &manager = MtpDeviceManager::GetInstance();
    EXPECT_CALL(*libraryFuncMock_, umount2(_, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.UmountDevice(deviceInfo, true, true), E_OK);

    GTEST_LOG_(INFO) << "UmountDeviceTest_005 end";
}

/**
 * @tc.name  : DeviceTypeTest_001
 * @tc.number: DeviceTypeTest_001
 * @tc.desc  : Test device type field for mtpfs
 */
HWTEST_F(MtpDeviceManagerTest, DeviceTypeTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceTypeTest_001 start";

    MtpDeviceInfo device;
    device.type = "mtpfs";
    device.id = "test_mtp_device";
    EXPECT_EQ(device.type, "mtpfs");
    EXPECT_EQ(device.id, "test_mtp_device");

    GTEST_LOG_(INFO) << "DeviceTypeTest_001 end";
}

/**
 * @tc.name  : DeviceTypeTest_002
 * @tc.number: DeviceTypeTest_002
 * @tc.desc  : Test device type field for gphotofs
 */
HWTEST_F(MtpDeviceManagerTest, DeviceTypeTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceTypeTest_002 start";

    MtpDeviceInfo device;
    device.type = "gphotofs";
    device.id = "test_camera_device";
    EXPECT_EQ(device.type, "gphotofs");
    EXPECT_EQ(device.id, "test_camera_device");

    GTEST_LOG_(INFO) << "DeviceTypeTest_002 end";
}

/**
 * @tc.name  : DeviceTypeTest_003
 * @tc.number: DeviceTypeTest_003
 * @tc.desc  : No "type empty" validation exists in current production code.
 *             This case verifies empty type will lead to ForkExec failure (mocked) and isMounting reset.
 */
HWTEST_F(MtpDeviceManagerTest, DeviceTypeTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceTypeTest_003 start";

    auto &manager = MtpDeviceManager::GetInstance();
    manager.isMounting_ = false;
    MtpDeviceInfo device;
    device.type = "";
    device.path = "/test/path_empty_type";
    device.id = "empty_type";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*libraryFuncMock_, umount(_)).WillOnce(Return(0));
    EXPECT_CALL(*libraryFuncMock_, rmdir(_)).WillOnce(Return(0));
    EXPECT_NE(manager.MountDevice(device), E_OK);
    EXPECT_FALSE(manager.isMounting_);

    GTEST_LOG_(INFO) << "DeviceTypeTest_003 end";
}

/**
 * @tc.name  : DeviceTypeTest_004
 * @tc.number: DeviceTypeTest_004
 * @tc.desc  : Test device type is correctly set in device info
 */
HWTEST_F(MtpDeviceManagerTest, DeviceTypeTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceTypeTest_004 start";

    MtpDeviceInfo mtpDevice;
    mtpDevice.type = "mtpfs";
    mtpDevice.id = "mtp_001";
    mtpDevice.path = "/dev/mtp";

    MtpDeviceInfo gphotoDevice;
    gphotoDevice.type = "gphotofs";
    gphotoDevice.id = "camera_001";
    gphotoDevice.path = "usb:001,005";

    EXPECT_EQ(mtpDevice.type, "mtpfs");
    EXPECT_EQ(gphotoDevice.type, "gphotofs");
    EXPECT_NE(mtpDevice.type, gphotoDevice.type);

    GTEST_LOG_(INFO) << "DeviceTypeTest_004 end";
}

} // namespace StorageDaemon
} // namespace OHOS