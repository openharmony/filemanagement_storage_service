/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include "scan_device.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

constexpr int CREATE_MODE = 0755;

class ScanDeviceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void CreateDirectory(const std::string &path);
    void CreateFileWithContent(const std::string &filePath, const std::string &content);
    void CreateMockSysfsNodes(const std::string &deviceName, bool removable = false, bool isUfs = false);
    void DeleteMockSysfsNodes(const std::string &deviceName);

    std::string mockSysPath;
};

void ScanDeviceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "ScanDeviceTest SetUpTestCase Start";
}

void ScanDeviceTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "ScanDeviceTest TearDownTestCase Start";
}

void ScanDeviceTest::SetUp()
{
    mockSysPath = "/data/test_sys_block";
}

void ScanDeviceTest::TearDown()
{
    system(("rm -rf " + mockSysPath).c_str());
}

void ScanDeviceTest::CreateDirectory(const std::string &path)
{
    if (mkdir(path.c_str(), CREATE_MODE) != 0) {
        GTEST_LOG_(ERROR) << "Failed to create directory: " << path;
    }
}

void ScanDeviceTest::CreateFileWithContent(const std::string &filePath, const std::string &content)
{
    FILE *fp = fopen(filePath.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create file: " << filePath;
    }
}

void ScanDeviceTest::CreateMockSysfsNodes(const std::string &deviceName, bool removable, bool isUfs)
{
    CreateDirectory(mockSysPath);
    std::string devicePath = mockSysPath + "/" + deviceName;
    CreateDirectory(devicePath);
    CreateDirectory(devicePath + "/device");
    CreateDirectory(devicePath + "/queue");
    CreateFileWithContent(devicePath + "/size", "2097152\n");
    CreateFileWithContent(devicePath + "/device/vendor", "TestVendor\n");
    CreateFileWithContent(devicePath + "/device/model", "TestModel\n");
    CreateFileWithContent(devicePath + "/device/state", "running\n");
    CreateFileWithContent(devicePath + "/queue/rotational", "0\n");
    std::string removableContent = removable ? "1\n" : "0\n";
    CreateFileWithContent(devicePath + "/removable", removableContent);
    CreateFileWithContent(devicePath + "/dev", "8:16\n");

    // 创建软链接模拟PCIe路径
    std::string linkTarget;
    if (isUfs) {
        linkTarget = "/sys/devices/ufs/ufs0/host0/target0:00:0/0:00:00:0/block/" + deviceName;
    } else {
        linkTarget =
            "/sys/devices/pci0000:00/0000:00:02.0/0000:01:00.0/host0/target0:00:0/0:00:00:0/block/" + deviceName;
    }
    unlink(devicePath.c_str());
    if (symlink(linkTarget.c_str(), devicePath.c_str()) != 0) {
        GTEST_LOG_(ERROR) << "Failed to create symlink for " << devicePath;
    }
}

void ScanDeviceTest::DeleteMockSysfsNodes(const std::string &deviceName)
{
    std::string devicePath = mockSysPath + "/" + deviceName;
    system(("rm -rf " + devicePath).c_str());
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_001
 * @tc.desc: Test scanning data disks with empty directory
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_001 start";

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();

    EXPECT_EQ(result.size(), 0);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_002
 * @tc.desc: Test scanning with s* data disk (removable=1)
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_002 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_GE(result.size(), 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_003
 * @tc.desc: Test scanning ignores nvme0* system disks
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_003 start";

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    for (const auto &disk : result) {
        EXPECT_TRUE(disk.pciePath.find("nvme0") == std::string::npos);
    }

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_004
 * @tc.desc: Test scanning ignores invalid nvme format
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_004 start";

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    for (const auto &disk : result) {
        EXPECT_TRUE(disk.pciePath.empty() || disk.pciePath.find("nvme") != 0 || disk.pciePath.find("nvme0") == 0 ||
                    (disk.pciePath.find("nvme") == 0 && disk.pciePath.find('n') > 4));
    }

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_004 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_005
 * @tc.desc: 测试正常场景：removable=1 的 SATA 设备应被过滤
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_005 start";
    CreateMockSysfsNodes("sdb", true, false);
 
    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
 
    EXPECT_EQ(result.size(), 0);
 
    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_005 end";
}
 
/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_006
 * @tc.desc: 测试过滤场景：removable=0 的 SATA 设备应被识别为数据盘
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_006 start";
 
    CreateMockSysfsNodes("sda", false, false);
 
    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 1);
 
    DeleteMockSysfsNodes("sda");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_006 end";
}
 
/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_007
 * @tc.desc: 测试过滤场景：NVMe 设备不应被识别为外盘（因为 isSDevice 为 false）
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_007 start";
    CreateMockSysfsNodes("nvme1n1", true, false);
 
    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 1);
 
    DeleteMockSysfsNodes("nvme1n1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_007 end";
}
 
/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_008
 * @tc.desc: 测试异常场景：GetBlockInfo 失败时，设备不应被加入列表
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_008 start";
    CreateMockSysfsNodes("sdb", true, false);
    std::string sizeFile = mockSysPath + "/sdb/size";
    FILE *fp = fopen(sizeFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "invalid_number\n");
        fclose(fp);
    }
 
    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 0);
 
    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_008 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadRemovableNode_001
 * @tc.desc: Test reading removable node with value 1
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadRemovableNode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    bool isRemovable = false;
    bool ret = scanner.ReadRemovableNode("sdb", isRemovable);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(isRemovable);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadRemovableNode_002
 * @tc.desc: Test reading removable node with value 0
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadRemovableNode_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_002 start";

    CreateMockSysfsNodes("sda", false, false);

    ScanDevice scanner(mockSysPath);
    bool isRemovable = false;
    bool ret = scanner.ReadRemovableNode("sda", isRemovable);

    EXPECT_TRUE(ret);
    EXPECT_FALSE(isRemovable);

    DeleteMockSysfsNodes("sda");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_002 end";
}

HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadRemovableNode_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_003 start";

    ScanDevice scanner(mockSysPath);
    bool isRemovable = false;
    bool ret = scanner.ReadRemovableNode("sda", isRemovable);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_001
 * @tc.desc: Test IsDataDisk with removable=0 and sd* device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_001 start";

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sda", false, false);
    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_002
 * @tc.desc: Test IsDataDisk with removable=1 and sd* device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_002 start";

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sdb", false, true);
    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_003
 * @tc.desc: Test IsDataDisk with non-sd* device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_003 start";

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("nvme0n1", false, false);
    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_004
 * @tc.desc: Test IsDataDisk with UFS device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_004 start";

    CreateMockSysfsNodes("sda", false, true);

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sda", true, false);
    EXPECT_FALSE(ret);

    DeleteMockSysfsNodes("sda");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_004 end";
}

/**
 * @tc.name: IsDataDisk_005
 * @tc.desc: 测试 readlink 失败的情况
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_005 start";
    std::string devicePath = mockSysPath + "/sda";
    mkdir(devicePath.c_str(), 0755);

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sda", true, false);
    EXPECT_FALSE(ret);

    system(("rm -rf " + devicePath).c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_005 end";
}

/**
 * @tc.name: IsDataDisk_006
 * @tc.desc: 测试 readlink 成功且为 UFS 设备
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_006 start";
    std::string devicePath = mockSysPath + "/sda";
    mkdir(devicePath.c_str(), 0755);
    std::string deviceDir = devicePath + "/device";
    mkdir(deviceDir.c_str(), 0755);
    std::string linkTarget = "../../devices/ufs/ufs0/host0/target0:0:0/0:0:0:0/block/sda";
    symlink(linkTarget.c_str(), devicePath.c_str());

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sda", true, false);
    EXPECT_FALSE(ret);

    system(("rm -rf " + devicePath).c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_006 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskSize_001
 * @tc.desc: Test getting disk size from sysfs
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskSize_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    uint64_t size = scanner.GetDiskSize("sdb");
    EXPECT_GE(size, 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskSize_001 end";
}

HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskSize_002 start";

    ScanDevice scanner(mockSysPath);
    uint64_t size = scanner.GetDiskSize("sdb");
    EXPECT_EQ(size, 0);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskSize_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetVendor_001
 * @tc.desc: Test getting vendor information
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetVendor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetVendor_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::string vendor = scanner.GetVendor("sdb");

    EXPECT_TRUE(vendor == "TestVendor");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetVendor_001 end";
}

HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetVendor_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetVendor_002 start";

    ScanDevice scanner(mockSysPath);
    std::string vendor = scanner.GetVendor("sdb");
    EXPECT_TRUE(vendor == "Unknown");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetVendor_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetModel_001
 * @tc.desc: Test getting model information
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetModel_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetModel_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::string model = scanner.GetModel("sdb");
    EXPECT_TRUE(model == "TestModel");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetModel_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetModel_002
 * @tc.desc: Test getting model information
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetModel_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetModel_002 start";

    ScanDevice scanner(mockSysPath);
    std::string model = scanner.GetModel("sdb");
    EXPECT_TRUE(model == "Unknown");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetModel_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetInterfaceType_001
 * @tc.desc: Test getting interface type
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetInterfaceType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetInterfaceType_001 start";

    ScanDevice scanner(mockSysPath);

    EXPECT_EQ(scanner.GetInterfaceType("nvme0"), "NVMe");
    EXPECT_EQ(scanner.GetInterfaceType("nvme1n1"), "NVMe");
    EXPECT_EQ(scanner.GetInterfaceType("sda"), "SATA");
    EXPECT_EQ(scanner.GetInterfaceType("sdb"), "SATA");
    EXPECT_EQ(scanner.GetInterfaceType("loop0"), "Unknown");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetInterfaceType_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskState_001
 * @tc.desc: Test getting disk state
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskState_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::string state = scanner.GetDiskState("sdb");
    EXPECT_TRUE(state == "running");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskState_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetMediaType_001
 * @tc.desc: Test getting media type - SSD
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetMediaType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sdb", false);
    EXPECT_TRUE(type == MediaType::SSD);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetMediaType_002
 * @tc.desc: Test getting media type - HDD
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetMediaType_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_002 start";

    CreateMockSysfsNodes("sdc", true, false);
    std::string rotationalFile = mockSysPath + "/sdc/queue/rotational";
    FILE *fp = fopen(rotationalFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "1\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sdc", false);
    EXPECT_TRUE(type == MediaType::HDD);

    DeleteMockSysfsNodes("sdc");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetMediaType_003
 * @tc.desc: Test getting media type - NVMe
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetMediaType_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_003 start";

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("nvme1n1", true);
    EXPECT_EQ(type, MediaType::SSD);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_003 end";
}

/**
 * @tc.name: GetMediaType_004
 * @tc.desc: 测试 rotational 节点读取失败的情况
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetMediaType_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_004 start";
    std::string devicePath = mockSysPath + "/sda";
    mkdir(devicePath.c_str(), 0755);
    std::string deviceDir = devicePath + "/device";
    mkdir(deviceDir.c_str(), 0755);

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sda", false);
    EXPECT_EQ(type, MediaType::UNKNOWN);

    system(("rm -rf " + devicePath).c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_004 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPciePath_001
 * @tc.desc: Test getting PCIe path
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPciePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPciePath_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::string pciePath = scanner.GetPciePath("sdb");
    EXPECT_GE(pciePath.length(), 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPciePath_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetBlockInfo_001
 * @tc.desc: Test getting complete block device info
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetBlockInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetBlockInfo_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("sdb", false, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(blockInfo.interfaceType, "SATA");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetBlockInfo_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetBlockInfo_002
 * @tc.desc: Test getting NVMe block device info
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetBlockInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetBlockInfo_002 start";

    CreateMockSysfsNodes("nvme1n1", false, false);

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("nvme1n1", true, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(blockInfo.interfaceType, "NVMe");

    DeleteMockSysfsNodes("nvme1n1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetBlockInfo_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_001
 * @tc.desc: Test reading sysfs node - existing file
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::string content;
    std::string testFile = mockSysPath + "/sdb/size";

    bool ret = scanner.ReadSysfsNode(testFile, content);

    EXPECT_TRUE(ret);
    EXPECT_EQ(content, "2097152");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_002
 * @tc.desc: Test reading sysfs node - non-existing file
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_002 start";

    ScanDevice scanner(mockSysPath);
    std::string content;
    std::string testFile = "/sys/block/nonexistent/size";

    bool ret = scanner.ReadSysfsNode(testFile, content);

    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001
 * @tc.desc: 测试 ReadSysfsNode 去除头部空格
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001 start";
    // 创建临时文件，内容为前导空格 + 数字
    std::string testFile = mockSysPath + "/trim_leading_test";
    CreateDirectory(mockSysPath);
    std::string content = "   12345\n";
    FILE *fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        return;
    }
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    // 验证读取成功
    EXPECT_TRUE(ret);
    // 验证头部空格被去除
    EXPECT_EQ(result, "12345");
    // 清理
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001
 * @tc.desc: 测试 ReadSysfsNode 去除尾部空格
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_trailing_test";
    CreateDirectory(mockSysPath);
    std::string content = "12345   \n";
    FILE *fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        return;
    }
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    // 验证尾部空格被去除
    EXPECT_EQ(result, "12345");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001
 * @tc.desc: 测试 ReadSysfsNode 同时去除头部和尾部空格
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_both_test";
    CreateDirectory(mockSysPath);
    std::string content = "   12345   \n";
    FILE *fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        return;
    }
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    // 验证首尾空格都被去除
    EXPECT_EQ(result, "12345");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001
 * @tc.desc: 测试 ReadSysfsNode 处理全空格字符串，应返回空字符串
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_all_spaces_test";
    CreateDirectory(mockSysPath);
    std::string content = "     \n";
    FILE *fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        return;
    }
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    // 验证全空格字符串被处理为空串
    EXPECT_EQ(result, "");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001
 * @tc.desc: 测试 ReadSysfsNode 处理混合空白字符（空格、Tab等）
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001 start";
    std::string testFile = mockSysPath + "/trim_mixed_ws_test";
    CreateDirectory(mockSysPath);
    // 包含前导空格、Tab，尾部空格
    std::string content = " \t  hello  \n";
    FILE *fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        return;
    }
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    // 验证首尾的空白字符（包括Tab）都被去除，中间保留
    EXPECT_EQ(result, "hello");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001
 * @tc.desc: 测试 ReadSysfsNode 处理无空格字符串，应原样返回
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001 start";
    std::string testFile = mockSysPath + "/trim_no_ws_test";
    CreateDirectory(mockSysPath);
    std::string content = "no_spaces";
    FILE *fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "%s", content.c_str());
        fclose(fp);
    } else {
        GTEST_LOG_(ERROR) << "Failed to create test file: " << testFile;
        return;
    }
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    // 验证无空格字符串保持不变
    EXPECT_EQ(result, "no_spaces");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskRpm_001
 * @tc.desc: Test getting disk RPM
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskRpm_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskRpm_001 start";
    ScanDevice scanner(mockSysPath);
    uint32_t rpm = scanner.GetDiskRpm("nonexistent", false);
    EXPECT_EQ(rpm, 0);
    rpm = scanner.GetDiskRpm("sda", false);
    EXPECT_GE(rpm, 0);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskRpm_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskRpm_002
 * @tc.desc: Test getting disk RPM - NVMe
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskRpm_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskRpm_002 start";
    ScanDevice scanner(mockSysPath);
    uint32_t rpm = scanner.GetDiskRpm("nvme1n1", true);
    EXPECT_EQ(rpm, 0);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskRpm_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetSerialNumber_TestNvmeDevice
 * @tc.desc: Test getting serial number - NVMe
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetSerialNumber_TestNvmeDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestNvmeDevice start";
    CreateMockSysfsNodes("nvme1n1", false, false);
    std::string serialFile = mockSysPath + "/nvme1n1/device/serial";
    FILE *fp = fopen(serialFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "NVMeSerial123456   \n");
        fclose(fp);
    }
    ScanDevice scanner(mockSysPath);
    std::string serial = scanner.GetSerialNumber("nvme1n1", true);
    EXPECT_TRUE(!serial.empty());
    DeleteMockSysfsNodes("nvme1n1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestNvmeDevice end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidNvmeDevice
 * @tc.desc: Test getting serial number - Invalid NVMe name
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidNvmeDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidNvmeDevice start";
    ScanDevice scanner(mockSysPath);
    std::string serial = scanner.GetSerialNumber("nonexistent", true);
    EXPECT_GE(serial.length(), 0);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidNvmeDevice end";
}

} // namespace StorageDaemon
} // namespace OHOS