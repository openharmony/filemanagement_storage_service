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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/sysmacros.h>

#include "disk/disk_info.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "disk_info_test_mock.h"
#include "netlink/netlink_data.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class DiskInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
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
}

void DiskInfoTest::TearDown(void)
{
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;

    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Create_001
 * @tc.desc: Verify the Create function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Create_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Create_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, Create()).WillOnce(testing::Return(E_OK));
    int ret = mock->Create();

    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Create_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_Create_002
 * @tc.desc: Verify the Create function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_Create_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Create_002 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, Create()).WillOnce(testing::Return(E_ERR));
    int ret = mock->Create();

    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Create_002 end";
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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDevice()).WillOnce(testing::Return(device));
    dev_t ret = mock->GetDevice();

    EXPECT_TRUE(ret == device);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_Partition_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetId_001
 * @tc.desc: Verify the GetId function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetId_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    std::string id = StringPrintf("disk-%d-%d", major(device), minor(device));
    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetId()).WillOnce(testing::Return(id));
    std::string ret = mock->GetId();

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDevPath()).WillOnce(testing::Return(devPath));
    std::string ret = mock->GetDevPath();

    EXPECT_TRUE(ret == devPath);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetDevDSize_001
 * @tc.desc: Verify the GetDevDSize function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDevDSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDevPath_001 start";

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    uint64_t mock_size = 1;
    EXPECT_CALL(*mock, GetDevDSize()).WillOnce(testing::Return(mock_size));
    uint64_t ret = mock->GetDevDSize();

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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
    std::string path(sysPath + "/device/manfid");
    std::string str;
    ReadFile(path, &str);
    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);

    EXPECT_CALL(*mock, GetDevFlag()).WillOnce(testing::Return(flag));
    int ret = mock->GetDevFlag();

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto mock = std::make_shared<DiskInfoTestMock>(sysPath, devPath, device, flag);
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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    diskInfo->ReadMetadata();
    EXPECT_EQ(diskInfo->GetDevDSize(), -1);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);

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

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVTYPE=disk\0\
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

    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);

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
    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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
    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
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
}
}
