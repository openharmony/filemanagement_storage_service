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

#include <fstream>
#include <gtest/gtest.h>

#include "disk/disk_manager.h"
#include "disk_info_test_mock.h"
#include "message_parcel.h"
#include "netlink/netlink_data.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

const int CONFIG_PARAM_NUM = 6;
static constexpr const char* CONFIG_PATH = "/system/etc/storage_daemon/disk_config";

class DiskManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown() {};
};

void DiskManagerTest::SetUp()
{
    DiskManager &diskManager = DiskManager::Instance();
    std::ifstream infile;
    infile.open(CONFIG_PATH);
    if (!infile) {
        LOGE("Cannot open config");
        return ;
    }

    while (infile) {
        std::string line;
        std::getline(infile, line);
        if (line.empty()) {
            LOGI("Param config complete");
            break;
        }

        std::string token = " ";
        auto split = SplitLine(line, token);
        if (split.size() != CONFIG_PARAM_NUM) {
            LOGE("Invalids config line: number of parameters is incorrect");
            continue;
        }

        auto it = split.begin();
        if (*it != "sysPattern") {
            LOGE("Invalids config line: no sysPattern");
            continue;
        }

        auto sysPattern = *(++it);
        if (*(++it) != "label") {
            LOGE("Invalids config line: no label");
            continue;
        }

        auto label = *(++it);
        if (*(++it) != "flag") {
            LOGE("Invalids config line: no flag");
            continue;
        }

        it++;
        int flag = std::atoi((*it).c_str());
        auto diskConfig =  std::make_shared<DiskConfig>(sysPattern, label, flag);
        diskManager.AddDiskConfig(diskConfig);
    }
    infile.close();
}

HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_AddDiskConfig_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_AddDiskConfig_001 start";

    DiskManager diskManager;

    std::string sysPattern = "/devices/platform/fe2b0000.dwmmc/*";
    std::string lable = "disk";
    int flag = 0;
    auto diskConfig = std::make_shared<DiskConfig>(sysPattern, lable, flag);
    diskManager.AddDiskConfig(diskConfig);
    EXPECT_EQ(1, diskManager.diskConfig_.size());
    EXPECT_EQ(diskManager.diskConfig_.front(), diskConfig);
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_AddDiskConfig_001 end";
}

HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_ReplayUevent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_ReplayUevent_001 start";

    DiskManager diskManager;

    size_t originalSize = diskManager.disk_.size();
    diskManager.ReplayUevent();
    EXPECT_EQ(originalSize, diskManager.disk_.size());

    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_ReplayUevent_001 end";
}

HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_MatchConfig_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_001 start";

    DiskManager &diskManager = DiskManager::Instance();

    char msg[1024] = { "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0\
                        \0DEVPATH=/class/input/input9/mouse2\0SUBSYSTEM=input\0SEQNUM=1064\0\
                        \0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0\
                        \0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"};
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    auto diskInfo = diskManager.MatchConfig(data.get());
    ASSERT_TRUE(diskInfo == nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_001 end";
}

HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_MatchConfig_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_002 start";

    DiskManager &diskManager = DiskManager::Instance();

    static char diskEventMsg[] = {
        "add@/class/input/input9/mouse2\0ACTION=add\0DEVNAME=sda\0DEVTYPE=disk\0"
        "\0DEVPATH=/devices/platform/fe2b0000.dwmmc/*\0SUBSYSTEM=input\0SEQNUM=1064\0"
        "\0PHYSDEVPATH=/devices/pci0000:00/0000:00:1d.1/usb2/2?2/2?2:1.0\0"
        "\0PHYSDEVBUS=usb\0PHYSDEVDRIVER=usbhid\0MAJOR=13\0MINOR=34\0"
    };

    auto data = std::make_unique<NetlinkData>();
    data.get()->Decode(const_cast<char*>(diskEventMsg));

    auto diskInfo1 = diskManager.MatchConfig(data.get());

    ASSERT_TRUE(diskInfo1 == nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_002 end";
}

HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_MatchConfig_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_003 start";

    DiskManager &diskManager = DiskManager::Instance();
    auto diskInfo1 = diskManager.MatchConfig(nullptr);
    EXPECT_TRUE(diskInfo1 == nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_003 end";
}

HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_MatchConfig_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_004 start";

    DiskManager &diskManager = DiskManager::Instance();

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
    auto diskInfo = diskManager.MatchConfig(data.get());
    ASSERT_TRUE(diskInfo != nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_004 end";
}


HWTEST_F(DiskManagerTest, Storage_Service_DiskManagerTest_MatchConfig_InvalidMajorMinor, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_InvalidMajorMinor start";

    DiskManager &diskManager = DiskManager::Instance();

    char msg[1024] = {
        "add@/devices/platform/hiusb/hiusb_port/hiusb-port1/ea200000.hiusbc/"
        "xhci-hcd.1/usb1/1-1/1-1:1.0/host0/target0:0:0/0:0:0:0/block/sr0\0"
        "ACTION=add\0"
        "DEVPATH=/devices/platform/hiusb/hiusb_port/hiusb-port1/ea200000.hiusbc/xhci-hcd.1/"
        "usb1/1-1/1-1:1.0/host0/target0:0:0/0:0:0:0/block/sr0\0"
        "SUBSYSTEM=block\0"
        "MAJOR=abc\0"
        "MINOR=xyz\0"
        "DEVNAME=sr0\0"
        "DEVTYPE=disk\0"
        "SEQNUM=6989\0"
    };
    auto data = std::make_unique<NetlinkData>();
    data->Decode(msg);
    auto diskInfo = diskManager.MatchConfig(data.get());
    EXPECT_TRUE(diskInfo == nullptr);

    GTEST_LOG_(INFO) << "Storage_Service_DiskManagerTest_MatchConfig_InvalidMajorMinor end";
}

} // STORAGE_DAEMON
} // OHOS
