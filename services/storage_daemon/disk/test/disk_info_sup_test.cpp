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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/sysmacros.h>

#include "disk/disk_info.h"
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

class DiskInfoSupTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_CreateMBRVolume_001
 * @tc.desc: Verify the CreateMBRVolume function.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_CreateMBRVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateMBRVolume_001 start";

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
    int32_t type = 0;
    auto ret = diskInfo->CreateMBRVolume(type, device);
    EXPECT_FALSE(ret);

    std::vector<int32_t> typeList{ 0x06, 0x07, 0x0b, 0x0c, 0x0e, 0x1b, 0x83 };
    for (std::size_t i = 0; i < typeList.size(); i++) {
        ret = diskInfo->CreateMBRVolume(typeList[i], device);
        EXPECT_TRUE(ret);
        ret = diskInfo->CreateMBRVolume(typeList[i], device);
        EXPECT_FALSE(ret);
        ret = diskInfo->Destroy();
        GTEST_LOG_(INFO) << ret;
    }
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateMBRVolume_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_ReadDiskLines_001
 * @tc.desc: Verify the ReadDiskLines function.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_ReadDiskLines_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_ReadDiskLines_001 start";

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
    std::vector<std::string> lines = {
        "",
        "DISK test",
        "DISK mbr",
        "PART",
        "DISK gpt",
        "PART 0",
        "PART 5",
    };
    int32_t maxVols = 3;
    bool isUserdata = false;
    auto ret = diskInfo->ReadDiskLines(lines, maxVols, isUserdata);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_ReadDiskLines_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_ReadDiskLines_002
 * @tc.desc: Verify the ReadDiskLines function.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_ReadDiskLines_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_ReadDiskLines_002 start";

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
    std::vector<std::string> lines = {
        "DISK gpt",
        "PART 4",
        "DISK gpt",
        "PART 6",
        "DISK mbr",
        "PART 2 7",
        "DISK mbr",
        "PART 7 3",
    };
    int32_t maxVols = 3;
    bool isUserdata = false;
    auto ret = diskInfo->ReadDiskLines(lines, maxVols, isUserdata);
    EXPECT_EQ(ret, E_OK);
    ret = diskInfo->Destroy();
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_ReadDiskLines_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_CreateTableVolume_001
 * @tc.desc: Verify the CreateTableVolume function.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_CreateTableVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateTableVolume_001 start";

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
    DiskInfo::Table table = DiskInfo::Table::MBR;
    bool foundPart = false;
    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
    int32_t maxVols = 3;
    std::vector<std::string> testData = {"80000000", "123"};
    std::vector<std::string>::iterator it = testData.begin();
    const std::vector<std::string>::iterator end = testData.end();
    diskInfo->CreateTableVolume(it, end, table, foundPart, device);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateTableVolume_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_CreateTableVolume_002
 * @tc.desc: Verify the CreateTableVolume function.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_CreateTableVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateTableVolume_002 start";

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
    DiskInfo::Table table = DiskInfo::Table::MBR;
    bool foundPart = false;
    auto diskInfo = std::make_shared<DiskInfo>(sysPath, devPath, device, flag);
    int32_t maxVols = 3;
    std::vector<std::string> testData = {"FFFFFFFFFFFFFFF7", "123"};
    std::vector<std::string>::iterator it = testData.begin();
    const std::vector<std::string>::iterator end = testData.end();
    diskInfo->CreateTableVolume(it, end, table, foundPart, device);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateTableVolume_002 end";
}
}
}
