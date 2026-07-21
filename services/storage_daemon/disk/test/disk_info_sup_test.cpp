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
    int32_t type = 0;
    auto ret = diskInfo->CreateMBRVolume(type, device, 1);   
    EXPECT_FALSE(ret);

    std::vector<int32_t> typeList{ 0x06, 0x07, 0x0b, 0x0c, 0x0e, 0x1b, 0x83 };
    for (std::size_t i = 0; i < typeList.size(); i++) {
        ret = diskInfo->CreateMBRVolume(typeList[i], device, 1);
        EXPECT_TRUE(ret);
        ret = diskInfo->CreateMBRVolume(typeList[i], device, 1);
        EXPECT_TRUE(ret);
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
    EXPECT_EQ(ret, E_OK);
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
    DiskInfo::Table table = DiskInfo::Table::MBR;
    bool foundPart = false;
    std::string diskName = data->GetDiskName();
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    std::vector<std::string> testData = {"123", "80000000"};
    std::vector<std::string>::iterator it = testData.begin();
    const std::vector<std::string>::iterator end = testData.end();
    diskInfo->CreateTableVolume(it, end, table, foundPart, device);
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_CreateTableVolume_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_FilterOutput_Normal_001
 * @tc.desc: Verify the FilterOutput function with complete input in a single chunk.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_FilterOutput_Normal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_Normal_001 start";

    std::string diskName = "sda";
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_NE(diskInfo, nullptr);

    std::vector<std::string> output = {
        "WARNING: before dump\n"
        "DISK gpt 259:0 100\n"
        "PART 1 0x0c00 2048 102400 system\n"
        "PART 2 0x0c00 10240 204800 userdata\n"
    };
    std::vector<std::string> lines;
    diskInfo->FilterOutput(lines, output);

    EXPECT_EQ(lines.size(), 3);
    EXPECT_EQ(std::find(lines.begin(), lines.end(), "WARNING: before dump"), lines.end());
    EXPECT_NE(std::find(lines.begin(), lines.end(), "DISK gpt 259:0 100"), lines.end());
    EXPECT_NE(std::find(lines.begin(), lines.end(), "PART 1 0x0c00 2048 102400 system"), lines.end());
    EXPECT_NE(std::find(lines.begin(), lines.end(), "PART 2 0x0c00 10240 204800 userdata"), lines.end());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_Normal_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_FilterOutput_CrossChunkSplit_002
 * @tc.desc: Verify FilterOutput stitches a line split across two chunks.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_FilterOutput_CrossChunkSplit_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_CrossChunkSplit_002 start";

    std::string diskName = "sda";
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_NE(diskInfo, nullptr);

    std::vector<std::string> output;
    output.push_back("DISK gpt 259:0 100\nPART 1 0x0c00 2048 102400 system\nPART 2 0x0c00 10240 204800 user");
    output.push_back("data\n");

    std::vector<std::string> lines;
    diskInfo->FilterOutput(lines, output);

    auto userdataIt = std::find(lines.begin(), lines.end(), "PART 2 0x0c00 10240 204800 userdata");
    EXPECT_NE(userdataIt, lines.end());
    EXPECT_EQ(std::find(lines.begin(), lines.end(), "PART 2 0x0c00 10240 204800 user"), lines.end());
    EXPECT_EQ(std::find(lines.begin(), lines.end(), "data"), lines.end());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_CrossChunkSplit_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_FilterOutput_NoDiskPrefix_003
 * @tc.desc: Verify FilterOutput keeps lines empty when no DISK prefix line exists.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_FilterOutput_NoDiskPrefix_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_NoDiskPrefix_003 start";

    std::string diskName = "sda";
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_NE(diskInfo, nullptr);

    std::vector<std::string> output = {
        "Partition table holds up to 128 entries\n"
        "DISKTYPE gpt\n"
        "Number  Start  End\n"
    };
    std::vector<std::string> lines;
    diskInfo->FilterOutput(lines, output);

    EXPECT_TRUE(lines.empty());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_NoDiskPrefix_003 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoSupTest_FilterOutput_Deduplicate_004
 * @tc.desc: Verify FilterOutput removes duplicate lines.
 * @tc.type: FUNC
 * @tc.require: IBCO84
 */
HWTEST_F(DiskInfoSupTest, Storage_Service_DiskInfoSupTest_FilterOutput_Deduplicate_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_Deduplicate_004 start";

    std::string diskName = "sda";
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_NE(diskInfo, nullptr);

    std::vector<std::string> output = {
        "DISK gpt\nPART 1 system\n",
        "PART 1 system\n"
    };
    std::vector<std::string> lines;
    diskInfo->FilterOutput(lines, output);

    size_t partCount = std::count(lines.begin(), lines.end(), "PART 1 system");
    EXPECT_EQ(partCount, 1);
    EXPECT_EQ(lines.size(), 2);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoSupTest_FilterOutput_Deduplicate_004 end";
}
}
}
