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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/sysmacros.h>

#include "disk/disk_info.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "mock/storage_manager_client_mock.h"
#include "disk_info_test_mock.h"
#include "netlink/netlink_data.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "volume/volume_manager.h"
#include "volume/external_volume_info.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class DiskInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<StorageManagerClientMock> storageManagerClientMock_ = nullptr;
};

void DiskInfoTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void DiskInfoTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void DiskInfoTest::SetUp(void)
{
    diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
    DiskUtilMoc::diskUtilMoc = diskUtilMoc_;

    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;

    storageManagerClientMock_ = std::make_shared<StorageManagerClientMock>();
    StorageManagerClientMock::iStorageManagerClientMock_ = storageManagerClientMock_;
}

void DiskInfoTest::TearDown(void)
{
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;

    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;

    StorageManagerClientMock::iStorageManagerClientMock_ = nullptr;
    storageManagerClientMock_ = nullptr;
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Destroy_001
 * @tc.desc: Verify the Destroy function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Destroy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Destroy_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, Destroy()).WillOnce(testing::Return(E_OK));
    int ret = mock->Destroy();

    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Destroy_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Destroy_002
 * @tc.desc: Verify the Destroy function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Destroy_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Destroy_002 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, Destroy()).WillOnce(testing::Return(E_ERR));
    int ret = mock->Destroy();

    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Destroy_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ReadPartition_001
 * @tc.desc: Verify the ReadPartition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ReadPartition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartition_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, ReadPartition()).WillOnce(testing::Return(E_OK));
    int ret = mock->ReadPartition();

    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartition_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ReadPartition_002
 * @tc.desc: Verify the ReadPartition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ReadPartition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartition_002 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, ReadPartition()).WillOnce(testing::Return(E_ERR));
    int ret = mock->ReadPartition();

    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartition_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreateVolume_001
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreateVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreateVolume_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, CreateVolume(testing::_)).WillOnce(testing::Return(E_OK));
    int ret = mock->CreateVolume(device);

    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreateVolume_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreateVolume_002
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreateVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreateVolume_002 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, CreateVolume(testing::_)).WillOnce(testing::Return(E_ERR));
    int ret = mock->CreateVolume(device);

    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreateVolume_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Partition_001
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Partition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, Partition()).WillOnce(testing::Return(E_OK));
    int ret = mock->Partition();

    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Partition_002
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Partition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_002 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, Partition()).WillOnce(testing::Return(E_ERR));
    int ret = mock->Partition();

    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetDevice_001
 * @tc.desc: Verify the GetDevice function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDevice_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevice_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDevice()).WillOnce(testing::Return(device));
    dev_t ret = mock->GetDevice();

    EXPECT_TRUE(ret == device);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetId_001
 * @tc.desc: Verify the GetDiskId function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetId_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    std::string id = StringPrintf("disk-%d-%d", major(device), minor(device));
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDiskId()).WillOnce(testing::Return(id));
    std::string ret = mock->GetDiskId();

    EXPECT_TRUE(ret == id);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetId_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetDevPath_001
 * @tc.desc: Verify the GetDevPath function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDevPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDevPath()).WillOnce(testing::Return(devPath));
    std::string ret = mock->GetDevPath();

    EXPECT_TRUE(ret == devPath);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetTotalSize_001
 * @tc.desc: Verify the GetTotalSize function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetTotalSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetTotalSize_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    uint64_t mock_size = 1;
    EXPECT_CALL(*mock, GetTotalSize()).WillOnce(testing::Return(mock_size));
    uint64_t ret = mock->GetTotalSize();

    EXPECT_TRUE(ret == mock_size);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetSysPath_001
 * @tc.desc: Verify the GetSysPath function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetSysPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetSysPath()).WillOnce(testing::Return(sysPath));
    std::string ret = mock->GetSysPath();

    EXPECT_TRUE(ret == sysPath);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetDevVendor_001
 * @tc.desc: Verify the GetDevVendor function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDevVendor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevVendor_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    std::string path(sysPath + "/device/manfid");
    std::string str;
    ReadFile(path, &str);
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDevVendor()).WillOnce(testing::Return(str));
    std::string ret = mock->GetDevVendor();

    EXPECT_TRUE(ret == str);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetDevFlag_001
 * @tc.desc: Verify the GetDevFlag function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDevFlag_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevFlag_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDiskType()).WillOnce(testing::Return(flag));
    int ret = mock->GetDiskType();

    EXPECT_TRUE(ret == flag);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetDevInfo_001
 * @tc.desc: Verify the GetDevInfo function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDevInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevInfo_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->GetDevice();
    diskInfo->GetDevPath();

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevInfo_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Partition_003
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Partition_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_003 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_CALL(*fileUtilMoc_,
        ForkExec(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_WEXITSTATUS));
    int ret = diskInfo->Partition();

    EXPECT_EQ(ret, E_WEXITSTATUS);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Destroy_003
 * @tc.desc: Verify the Destroy function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Destroy_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Destroy_003 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    int ret = diskInfo->Destroy();
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Destroy_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreateVolume_003
 * @tc.desc: Verify the CreateVolume function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreateVolume_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreateVolume_003 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto mock = std::make_shared<DiskInfoTestMock>(diskName, sysPath, devPath, device, flag);
    EXPECT_CALL(*mock, CreateVolume(testing::_)).WillOnce(testing::Return(E_ERR));
    int ret = mock->CreateVolume(device);

    EXPECT_TRUE(ret == E_ERR);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreateVolume_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Partition_004
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Partition_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_004 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_CALL(*fileUtilMoc_,
        ForkExec(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_WEXITSTATUS));
    int ret = diskInfo->Partition();
    
    EXPECT_EQ(ret, E_WEXITSTATUS);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_004 end";
}

/**
 * @tc.name: Storage_Service_ParseAndValidateManfid_001
 * @tc.desc: Reading manfid from sysfs path.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ParseAndValidateManfid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ParseAndValidateManfid_001 start";

    std::string sysPath;
    std::string devPath;
    unsigned int major = 0;
    unsigned int minor = 0;
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    uint32_t manfid;

    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("0x1A3F", manfid));
    EXPECT_EQ(0x1A3F, manfid);
    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("0x1a3f", manfid));
    EXPECT_EQ(0x1A3F, manfid);
    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("01a3f", manfid));
    EXPECT_EQ(0x1A3F, manfid);
    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("0X1a3f", manfid));
    EXPECT_EQ(0x1A3F, manfid);
    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("1", manfid));
    EXPECT_EQ(0x1, manfid);
    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("DEAD", manfid));
    EXPECT_EQ(0xDEAD, manfid);
    EXPECT_TRUE(diskInfo->ParseAndValidateManfid("  0xBEEF  ", manfid));
    EXPECT_EQ(0xBEEF, manfid);

    GTEST_LOG_(INFO) << "Storage_Service_ParseAndValidateManfid_001 end";
}

/**
 * @tc.name: Storage_Service_ParseAndValidateManfid_002
 * @tc.desc: Reading manfid from sysfs path.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ParseAndValidateManfid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ParseAndValidateManfid_002 start";

    std::string sysPath;
    std::string devPath;
    unsigned int major = 0;
    unsigned int minor = 0;
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    uint32_t manfid;

    EXPECT_FALSE(diskInfo->ParseAndValidateManfid("0xGHIJ", manfid));
    EXPECT_FALSE(diskInfo->ParseAndValidateManfid("123Z", manfid));
    EXPECT_FALSE(diskInfo->ParseAndValidateManfid("", manfid));
    EXPECT_FALSE(diskInfo->ParseAndValidateManfid("    ", manfid));

    GTEST_LOG_(INFO) << "Storage_Service_ParseAndValidateManfid_002 end";
}

/**
 * @tc.name: Storage_Service_ReadMetadata_001
 * @tc.desc: Verify the ReadMetadata function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ReadMetadata_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->ReadMetadata();
    EXPECT_EQ(diskInfo->GetTotalSize(), -1);

    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_001 end";
}

/**
 * @tc.name: Storage_Service_ReadMetadata_002
 * @tc.desc: Verify the ReadMetadata function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ReadMetadata_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_002 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*diskUtilMoc_, GetDevSize(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ReadFile(testing::_, testing::_)).WillOnce(testing::Return(false));
    diskInfo->ReadMetadata();
    EXPECT_EQ(diskInfo->GetDevVendor(), "");

    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_002 end";
}

/**
 * @tc.name: Storage_Service_ReadMetadata_003
 * @tc.desc: Verify the ReadMetadata function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ReadMetadata_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_003 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*diskUtilMoc_, GetDevSize(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ReadFile(testing::_, testing::_)).WillOnce(testing::Return(true));
    diskInfo->ReadMetadata();
    EXPECT_EQ(diskInfo->GetDevVendor(), "");

    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_003 end";
}

/**
 * @tc.name: Storage_Service_ReadMetadata_004
 * @tc.desc: Verify the ReadMetadata function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ReadMetadata_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_004 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=179\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*diskUtilMoc_, GetDevSize(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ReadFile(testing::_, testing::_)).WillOnce(testing::Return(false));
    diskInfo->ReadMetadata();
    EXPECT_EQ(diskInfo->GetDevVendor(), "");

    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_004 end";
}

/**
 * @tc.name: Storage_Service_ReadMetadata_005
 * @tc.desc: Verify the ReadMetadata function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_ReadMetadata_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_005 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                    \0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0\
                    \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                    \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=179\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);

    EXPECT_CALL(*diskUtilMoc_, GetDevSize(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(*fileUtilMoc_, ReadFile(testing::_, testing::_)).WillOnce(testing::Return(true));
    diskInfo->ReadMetadata();
    EXPECT_EQ(diskInfo->GetDevVendor(), "Invalid");

    GTEST_LOG_(INFO) << "Storage_Service_ReadMetadata_005 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ReadPartition_003
 * @tc.desc: Verify the ReadPartition function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ReadPartition_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartition_003 start";
    std::string sysPath = "/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0";
    std::string devPath = "/devices/platform/fe2b0000.dwmmc/*";
    unsigned int major = 13;
    unsigned int minor = 14;
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_CALL(*fileUtilMoc_,
        ForkExec(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    int ret = diskInfo->ReadPartition();
    EXPECT_TRUE(ret == E_ERR);
    EXPECT_CALL(*fileUtilMoc_,
        ForkExec(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    ret = diskInfo->ReadPartition();
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartition_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ReadPartitionCD_001
 * @tc.desc: Verify the ReadPartitionCD function, when ejectStatus == "1".
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ReadPartitionCD_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartitionCD_001 start";
    std::string devPath = "/devices/platform/xhci-hcd.1/";
    std::string sysPath = "/sys" + devPath;
    std::string volId = "vol-11-0";
    std::string diskId = "disk-11-0";
    dev_t device = makedev(11, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    diskInfo->volumeId_.push_back(volId);
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(testing::_, testing::_)).WillRepeatedly(testing::Return(""));
    EXPECT_EQ(diskInfo->ReadPartition("1"), E_ERR);

    auto &volManager = VolumeManager::Instance();
    auto volInfo = std::make_shared<ExternalVolumeInfo>();
    ASSERT_TRUE(volInfo != nullptr);
    volInfo->Create(volId, diskId, device, false, 1);
    volManager.volumes_.insert(make_pair(volId, volInfo));
    volInfo->mountState_ = REMOVED;
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, Eject(testing::_)).WillOnce(testing::Return(E_ERR));
    EXPECT_CALL(*diskUtilMoc_, DestroyDiskNode(testing::_)).WillRepeatedly(testing::Return(E_OK));
    EXPECT_EQ(diskInfo->ReadPartition("1"), E_ERR);

    volManager.volumes_.insert(make_pair(volId, volInfo));
    volInfo->mountState_ = REMOVED;
    EXPECT_CALL(*diskUtilMoc_, Eject(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_EQ(diskInfo->ReadPartition("1"), E_OK);
    diskInfo->volumeId_.clear();
    volManager.volumes_.clear();

    VolumeManager::Instance().DestroyVolume(volId);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartitionCD_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ReadPartitionCD_002
 * @tc.desc: Verify the ReadPartitionCD function, when ejectStatus == "1".
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ReadPartitionCD_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartitionCD_002 start";
    std::string devPath = "/devices/platform/xhci-hcd.1/";
    std::string sysPath = "/sys" + devPath;
    std::string volId = "vol-11-0";
    std::string diskId = "disk-11-0";
    dev_t device = makedev(11, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_CALL(*diskUtilMoc_, IsExistCD(_, _)).WillRepeatedly([&](const std::string &diskPath, bool &isExistCD) {
        isExistCD = false;
        return E_ERR;
    });
    EXPECT_EQ(diskInfo->ReadPartition("0"), E_OK);

    diskInfo->volumeId_.push_back(volId);
    EXPECT_CALL(*diskUtilMoc_, IsExistCD(_, _)).WillRepeatedly([&](const std::string &diskPath, bool &isExistCD) {
        isExistCD = false;
        return E_OK;
    });
    EXPECT_EQ(diskInfo->ReadPartition("0"), E_OK);

    auto &volManager = VolumeManager::Instance();
    auto volInfo = std::make_shared<ExternalVolumeInfo>();
    ASSERT_TRUE(volInfo != nullptr);
    volInfo->Create(volId, diskId, device, false, 1);
    volManager.volumes_.insert(make_pair(volId, volInfo));
    diskInfo->volumeId_.push_back(volId);
    EXPECT_CALL(*diskUtilMoc_, GetBlkidData(testing::_, testing::_)).WillRepeatedly(testing::Return(""));
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeStateChanged(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*storageManagerClientMock_, NotifyVolumeCreated(_)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*diskUtilMoc_, IsExistCD(_, _)).WillRepeatedly([&](const std::string &diskPath, bool &isExistCD) {
        isExistCD = true;
        return E_ERR;
    });
    EXPECT_EQ(diskInfo->ReadPartition("0"), E_OK);


    EXPECT_CALL(*diskUtilMoc_, IsExistCD(_, _)).WillRepeatedly([&](const std::string &diskPath, bool &isExistCD) {
        isExistCD = true;
        return E_OK;
    });
    EXPECT_EQ(diskInfo->ReadPartition("0"), E_OK);
    
    volManager.volumes_.clear();
    diskInfo->volumeId_.clear();
    volManager.DestroyVolume(volId);
    EXPECT_CALL(*diskUtilMoc_, IsExistCD(_, _)).WillRepeatedly([&](const std::string &diskPath, bool &isExistCD) {
        isExistCD = true;
        return E_OK;
    });
    EXPECT_EQ(diskInfo->ReadPartition("0"), E_ERR);

    diskInfo->volumeId_.clear();
    volManager.volumes_.clear();
    volManager.DestroyVolume(volId);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ReadPartitionCD_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_FilterOutput_001
 * @tc.desc: Verify the FilterOutput function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_FilterOutput_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_FilterOutput_001 start";
    std::string sysPath = "/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0";
    std::string devPath = "/devices/platform/fe2b0000.dwmmc/*";
    unsigned int major = 13;
    unsigned int minor = 14;
    dev_t device = makedev(major, minor);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    std::vector<std::string> lines;
    std::vector<std::string> output = {"test1", "test2"};
    diskInfo->FilterOutput(lines, output);
    EXPECT_TRUE(lines.empty());

    output = {"DISK test1", "test2"};
    diskInfo->FilterOutput(lines, output);
    EXPECT_TRUE(!lines.empty());
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_FilterOutput_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Partition_005
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Partition_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_005 start";

    char msg[1024] = {
        "change@/devices/platform/hiusb/hiusb_port/hiusb-port1/ea200000.hiusbc/"
        "xhci-hcd.1/usb1/1-1/1-1:1.0/host0/target0:0:0/0:0:0:0/block/sr0\0"
        "ACTION=change\0"
        "DEVPATH=/devices/platform/hiusb/hiusb_port/hiusb-port1/ea200000.hiusbc/xhci-hcd.1/"
        "usb1/1-1/1-1:1.0/host0/target0:0:0/0:0:0:0/block/sr0\0"
        "SUBSYSTEM=block\0"
        "DISK_EJECT_REQUEST=1\0"
        "MAJOR=11\0"
        "MINOR=0\0"
        "DEVNAME=sr0\0"
        "DEVTYPE=disk\0"
        "SEQNUM=6988\0"
    };
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    unsigned int major = std::stoi(data->GetParam("MAJOR"));
    unsigned int minor = std::stoi(data->GetParam("MINOR"));
    dev_t device = makedev(major, minor);
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    int ret = diskInfo->Partition();
    EXPECT_TRUE(ret == E_NOT_SUPPORT);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_005 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ProcessPartitionChanges_001
 * @tc.desc: Verify the ProcessPartitionChanges function with added lines.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ProcessPartitionChanges_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_001 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->sgdiskLines_ = {"DISK mbr", "PART 1 0x07"};
    std::vector<std::string> newLines = {"DISK mbr", "PART 1 0x07", "PART 2 0x07"};

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillRepeatedly(Return(E_OK));

    diskInfo->ProcessPartitionChanges(newLines, 2, false);

    EXPECT_EQ(diskInfo->sgdiskLines_.size(), newLines.size());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ProcessPartitionChanges_002
 * @tc.desc: Verify the ProcessPartitionChanges function with removed lines.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ProcessPartitionChanges_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_002 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->sgdiskLines_ = {"DISK mbr", "PART 1 0x07", "PART 2 0x07"};
    std::vector<std::string> newLines = {"DISK mbr", "PART 1 0x07"};

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillRepeatedly(Return(E_OK));

    diskInfo->ProcessPartitionChanges(newLines, 2, false);

    EXPECT_EQ(diskInfo->sgdiskLines_.size(), newLines.size());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ProcessPartitionChanges_003
 * @tc.desc: Verify the ProcessPartitionChanges function with both added and removed lines.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ProcessPartitionChanges_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_003 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->sgdiskLines_ = {"DISK gpt", "PART 1", "PART 2", "PART 3"};
    std::vector<std::string> newLines = {"DISK gpt", "PART 1", "PART 3", "PART 4"};

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).WillRepeatedly(Return(E_OK));

    diskInfo->ProcessPartitionChanges(newLines, 4, false);

    EXPECT_EQ(diskInfo->sgdiskLines_.size(), newLines.size());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_ProcessPartitionChanges_004
 * @tc.desc: Verify the ProcessPartitionChanges function with no changes.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_ProcessPartitionChanges_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_004 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->sgdiskLines_ = {"DISK gpt", "PART 1", "PART 2"};
    std::vector<std::string> newLines = {"DISK gpt", "PART 1", "PART 2"};

    diskInfo->ProcessPartitionChanges(newLines, 2, false);

    EXPECT_EQ(diskInfo->sgdiskLines_.size(), newLines.size());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_ProcessPartitionChanges_004 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetPartitionTable_001
 * @tc.desc: Verify the GetPartitionTable function with valid partition table.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetPartitionTable_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_001 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    OHOS::StorageManager::PartitionTableInfo partitionTableInfo;

    std::vector<std::string> mockOutput = {
        "Disk /dev/block/disk-8-0: 1234567 sectors, 596 MB",
        "Sector size (logical/physical): 512/512",
        "Disk identifier (GUID): 01234567-89AB-CDEF-0123-456789ABCDEF",
        "Partition table holds up to 128 entries",
        "Partitions will be aligned on 2048-sector boundaries",
        "First usable sector is 2048, last usable sector is 1234566",
        "Number  Start (sector)  End (sector)  Size       Code  Name",
        "   1            2048       102400   49.0 MB    0700  data",
        "   2          102400       512000   200 MB     8300  system"
    };

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockOutput), Return(E_OK)));

    int32_t ret = diskInfo->GetPartitionTable(partitionTableInfo);

    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(partitionTableInfo.GetDiskId(), "disk-8-0");
    EXPECT_GT(partitionTableInfo.GetPartitionCount(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetPartitionTable_002
 * @tc.desc: Verify the GetPartitionTable function with empty output.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetPartitionTable_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_002 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    OHOS::StorageManager::PartitionTableInfo partitionTableInfo;

    std::vector<std::string> mockOutput = {};

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockOutput), Return(E_OK)));

    int32_t ret = diskInfo->GetPartitionTable(partitionTableInfo);

    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetPartitionTable_003
 * @tc.desc: Verify the GetPartitionTable function with command execution failure.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetPartitionTable_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_003 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    OHOS::StorageManager::PartitionTableInfo partitionTableInfo;

    std::vector<std::string> mockOutput = {};

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockOutput), Return(E_GET_PARTITION_ERROR)));

    int32_t ret = diskInfo->GetPartitionTable(partitionTableInfo);

    EXPECT_EQ(ret, E_GET_PARTITION_ERROR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetPartitionTable_004
 * @tc.desc: Verify the GetPartitionTable function with GPT table type.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetPartitionTable_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_004 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    OHOS::StorageManager::PartitionTableInfo partitionTableInfo;

    std::vector<std::string> mockOutput = {
        "Disk /dev/block/disk-8-0: 1234567 sectors",
        "Sector size (logical/physical): 512/512",
        "Partition table holds up to 128 entries",
        "Disk identifier (GUID): test-guid",
        "Partitions will be aligned on 2048-sector boundaries",
        "First usable sector is 2048, last usable sector is 1234566",
        "Number  Start (sector)  End (sector)  Size       Code  Name",
        "   1            2048       102400   49.0 MB    8300  system"
    };

    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockOutput), Return(E_OK)));

    int32_t ret = diskInfo->GetPartitionTable(partitionTableInfo);

    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(partitionTableInfo.GetTableType(), "GPT");

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetPartitionTable_004 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_EjectDisk_001
 * @tc.desc: Verify the EjectDisk function when Destroy fails.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_EjectDisk_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_EjectDisk_001 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_CALL(*diskUtilMoc_, Eject(testing::_)).WillOnce(testing::Return(E_ERR));
    
    int ret = diskInfo->EjectDisk();
    EXPECT_EQ(ret, E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_EjectDisk_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_EjectDisk_002
 * @tc.desc: Verify the EjectDisk function when Eject fails but Destroy succeeds.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_EjectDisk_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_EjectDisk_002 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    // Mock Eject to return E_ERR
    EXPECT_CALL(*diskUtilMoc_, Eject(testing::_)).WillOnce(testing::Return(E_ERR));
    
    int ret = diskInfo->EjectDisk();
    EXPECT_EQ(ret, E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_EjectDisk_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_EjectDisk_003
 * @tc.desc: Verify the EjectDisk function when both Destroy and Eject succeed.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_EjectDisk_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_EjectDisk_003 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    // Mock Eject to return E_OK
    EXPECT_CALL(*diskUtilMoc_, Eject(testing::_)).WillOnce(testing::Return(E_OK));
    
    int ret = diskInfo->EjectDisk();
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_EjectDisk_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreatePartition_002
 * @tc.desc: Verify the CreatePartition function with invalid type code.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreatePartition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_002 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->lastUsableSector_ = 1234566;
    diskInfo->sectorSize_ = 512;
    diskInfo->alignSector_ = 2048;

    OHOS::StorageManager::PartitionParams partitionParams;
    std::string typeCode = "invalid_type";
    options.SetTypeCode(typeCode);
    options.SetStartSector(2048);
    options.SetEndSector(102400);

    int32_t ret = diskInfo->CreatePartition(options);

    EXPECT_EQ(ret, E_CREATE_PARTITION_NOT_SUPPORT);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreatePartition_003
 * @tc.desc: Verify the CreatePartition function with sector out of range.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreatePartition_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_003 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->lastUsableSector_ = 1234566;
    diskInfo->sectorSize_ = 512;
    diskInfo->alignSector_ = 2048;

    OHOS::StorageManager::PartitionParams partitionParams;
    std::string typeCode = "ext4";
    options.SetTypeCode(typeCode);
    options.SetStartSector(9999999);
    options.SetEndSector(102400);

    int32_t ret = diskInfo->CreatePartition(options);

    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreatePartition_004
 * @tc.desc: Verify the CreatePartition function with CD/DVD disk type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreatePartition_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_004 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sr0";
    int flag = static_cast<int>(DiskInfo::DiskType::CD_DVD_BD);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    OHOS::StorageManager::PartitionParams partitionParams;
    std::string typeCode = "ext4";
    options.SetTypeCode(typeCode);
    options.SetStartSector(2048);
    options.SetEndSector(102400);

    int32_t ret = diskInfo->CreatePartition(options);

    EXPECT_EQ(ret, E_CREATE_PARTITION_NOT_SUPPORT);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_004 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_CreatePartition_005
 * @tc.desc: Verify the CreatePartition function with minimum size validation for vfat.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_CreatePartition_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_005 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->lastUsableSector_ = 1234566;
    diskInfo->sectorSize_ = 512;
    diskInfo->alignSector_ = 2048;

    OHOS::StorageManager::PartitionParams partitionParams;
    std::string typeCode = "vfat";
    options.SetTypeCode(typeCode);
    options.SetStartSector(2048);
    options.SetEndSector(2048 + 100);

    int32_t ret = diskInfo->CreatePartition(options);

    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_CreatePartition_005 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_DeletePartition_002
 * @tc.desc: Verify the DeletePartition function with CD/DVD disk type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_DeletePartition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_DeletePartition_002 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sr0";
    int flag = static_cast<int>(DiskInfo::DiskType::CD_DVD_BD);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    uint32_t partitionNum = 1;

    int32_t ret = diskInfo->DeletePartition(partitionNum);

    EXPECT_EQ(ret, E_DELETE_PARTITION_NOT_SUPPORT);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_DeletePartition_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_FormatPartition_002
 * @tc.desc: Verify the FormatPartition function with invalid fs type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_FormatPartition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_FormatPartition_002 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    uint32_t partitionNum = 1;
    OHOS::StorageManager::FormatParams formatParams;
    std::string fsType = "invalid_fs";
    formatParams.SetFsType(fsType);

    int32_t ret = diskInfo->FormatPartition(partitionNum, formatParams);

    EXPECT_EQ(ret, E_FORMAT_PARTITION_NOT_SUPPORT);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_FormatPartition_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_FormatPartition_003
 * @tc.desc: Verify the FormatPartition function with CD/DVD disk type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_FormatPartition_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_FormatPartition_003 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sr0";
    int flag = static_cast<int>(DiskInfo::DiskType::CD_DVD_BD);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    uint32_t partitionNum = 1;
    OHOS::StorageManager::FormatParams formatParams;
    std::string fsType = "vfat";
    formatParams.SetFsType(fsType);

    int32_t ret = diskInfo->FormatPartition(partitionNum, formatParams);

    EXPECT_EQ(ret, E_FORMAT_PARTITION_NOT_SUPPORT);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_FormatPartition_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetMediaType_001
 * @tc.desc: Verify the GetMediaType function returns default value.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetMediaType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetMediaType_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    // GetMediaType returns default value (UNKNOWN_MEDIA_TYPE)
    int32_t mediaType = diskInfo->GetMediaType();
    EXPECT_EQ(mediaType, DiskInfo::MediaType::UNKNOWN_MEDIA_TYPE);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetMediaType_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetRemovable_001
 * @tc.desc: Verify the GetRemovable function returns default value.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetRemovable_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetRemovable_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    // GetRemovable returns default value (true)
    bool removable = diskInfo->GetRemovable();
    EXPECT_TRUE(removable);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetRemovable_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetExtraInfo_001
 * @tc.desc: Verify the GetExtraInfo function returns default value.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetExtraInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetExtraInfo_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    // GetExtraInfo returns default value (empty string)
    std::string extraInfo = diskInfo->GetExtraInfo();
    EXPECT_EQ(extraInfo, "");

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetExtraInfo_001 end";
}
}
}
