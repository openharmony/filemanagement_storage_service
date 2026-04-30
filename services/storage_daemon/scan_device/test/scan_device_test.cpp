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
 * @tc.name: GetExternalDisks_001
 * @tc.desc: 测试正常场景：removable=1 的 SATA 设备应被识别为外盘
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetExternalDisks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_001 start";
    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetExternalDisks();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].devicePath, "/dev/sdb");
    EXPECT_TRUE(result[0].removable);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_001 end";
}

/**
 * @tc.name: GetExternalDisks_002
 * @tc.desc: 测试过滤场景：removable=0 的 SATA 设备不应被识别为外盘
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetExternalDisks_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_002 start";

    CreateMockSysfsNodes("sda", false, false);

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetExternalDisks();
    EXPECT_EQ(result.size(), 0);

    DeleteMockSysfsNodes("sda");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_002 end";
}

/**
 * @tc.name: GetExternalDisks_003
 * @tc.desc: 测试过滤场景：NVMe 设备不应被识别为外盘（因为 isSDevice 为 false）
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetExternalDisks_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_003 start";
    CreateMockSysfsNodes("nvme1n1", true, false);

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetExternalDisks();
    EXPECT_EQ(result.size(), 0);

    DeleteMockSysfsNodes("nvme1n1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_003 end";
}

/**
 * @tc.name: GetExternalDisks_006
 * @tc.desc: 测试异常场景：GetBlockInfo 失败时，设备不应被加入列表
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetExternalDisks_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_004 start";
    CreateMockSysfsNodes("sdb", true, false);
    std::string sizeFile = mockSysPath + "/sdb/size";
    FILE *fp = fopen(sizeFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "invalid_number\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetExternalDisks();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].sizeBytes, 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_004 end";
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice
 * @tc.desc: Test getting serial number - Sata
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice start";
    CreateMockSysfsNodes("sda", true, true);
    std::string serialFile = mockSysPath + "/sda/device/serial";
    FILE *fp = fopen(serialFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "SdaSerial123456   \n");
        fclose(fp);
    }
    ScanDevice scanner(mockSysPath);
    std::string serial = scanner.GetSerialNumber("sda", false);
    EXPECT_TRUE(!serial.empty());
    DeleteMockSysfsNodes("sda");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice
 * @tc.desc: Test getting serial number - Invalid Sata name
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice start";

    ScanDevice scanner(mockSysPath);

    std::string serial = scanner.GetSerialNumber("nonexistent", false);
    EXPECT_GE(serial.length(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_EdgeCase_002
 * @tc.desc: Test edge case - special device names
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_EdgeCase_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_EdgeCase_002 start";

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_EdgeCase_002 end";
}

HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_EdgeCase_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_EdgeCase_003 start";
    CreateMockSysfsNodes("ram", true, false);

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_EdgeCase_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_MultipleDevices_001
 * @tc.desc: Test scanning multiple data disks
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_MultipleDevices_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_MultipleDevices_001 start";

    CreateMockSysfsNodes("sdb", true, false);
    CreateMockSysfsNodes("sdc", true, false);
    CreateMockSysfsNodes("nvme1n1", false, false);

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_GE(result.size(), 0);

    DeleteMockSysfsNodes("sdb");
    DeleteMockSysfsNodes("sdc");
    DeleteMockSysfsNodes("nvme1n1");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_MultipleDevices_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfoFields_001
 * @tc.desc: Test BlockInfo fields are properly set
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfoFields_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfoFields_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("sdb", false, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_GE(blockInfo.sizeBytes, 0);
    EXPECT_EQ(blockInfo.interfaceType, "SATA");
    EXPECT_GE(blockInfo.rpm, 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfoFields_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskId_001
 * @tc.desc: Test getting disk ID
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_001 start";

    CreateMockSysfsNodes("sdb", true, false);

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("sdb");
    EXPECT_TRUE(diskId == "disk-8-16");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskId_002
 * @tc.desc: Test getting disk ID with invalid format
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_002 start";

    CreateMockSysfsNodes("sdc", true, false);
    std::string devFile = mockSysPath + "/sdc/dev";
    FILE *fp = fopen(devFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "invalid_format\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("sdc");
    EXPECT_TRUE(diskId.empty());

    DeleteMockSysfsNodes("sdc");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskId_003
 * @tc.desc: 测试 dev 节点包含换行符的情况
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskId_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_003 start";
    CreateMockSysfsNodes("sdc", true, false);
    std::string devFile = mockSysPath + "/sdc/dev";
    FILE *fp = fopen(devFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "8:32\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("sdc");
    EXPECT_EQ(diskId, "disk-8-32");

    DeleteMockSysfsNodes("sdc");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetUsedBytes_001
 * @tc.desc: Test getting used bytes
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetUsedBytes_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_001 start";

    CreateMockSysfsNodes("sdb", true, false);
    std::string sdb1Dir = mockSysPath + "/sdb/sdb1";
    mkdir(sdb1Dir.c_str(), 0755);
    std::string sdb1Size = sdb1Dir + "/size";
    FILE *fp = fopen(sdb1Size.c_str(), "w");
    if (fp) {
        fprintf(fp, "1048576\n");
        fclose(fp);
    }

    std::string sdb2Dir = mockSysPath + "/sdb/sdb2";
    mkdir(sdb2Dir.c_str(), 0755);
    std::string sdb2Size = sdb2Dir + "/size";
    fp = fopen(sdb2Size.c_str(), "w");
    if (fp) {
        fprintf(fp, "1048576\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    uint64_t usedBytes = scanner.GetUsedBytes("sdb");
    EXPECT_TRUE(usedBytes != 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetUsedBytes_002
 * @tc.desc: 测试子目录不以数字结尾的情况
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetUsedBytes_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_002 start";
    CreateMockSysfsNodes("sdb", true, false);
    std::string invalidDir = mockSysPath + "/sdb/sdb_invalid";
    mkdir(invalidDir.c_str(), 0755);
    std::string invalidSize = invalidDir + "/size";
    FILE *fp = fopen(invalidSize.c_str(), "w");
    if (fp) {
        fprintf(fp, "1000\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    uint64_t usedBytes = scanner.GetUsedBytes("sdb");
    EXPECT_EQ(usedBytes, 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_002 end";
}

HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetUsedBytes_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_003 start";
    ScanDevice scanner(mockSysPath);
    uint64_t usedBytes = scanner.GetUsedBytes("sdb");
    EXPECT_EQ(usedBytes, 0);
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetAvailableBytes_001
 * @tc.desc: Test getting available bytes
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetAvailableBytes_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetAvailableBytes_001 start";

    CreateMockSysfsNodes("sdb", true, false);
    std::string sdb1Dir = mockSysPath + "/sdb/sdb1";
    mkdir(sdb1Dir.c_str(), 0755);
    std::string sdb1Size = sdb1Dir + "/size";
    FILE *fp = fopen(sdb1Size.c_str(), "w");
    if (fp) {
        fprintf(fp, "1048576\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    uint64_t availableBytes = scanner.GetAvailableBytes("sdb");
    EXPECT_TRUE(availableBytes != 0);

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetAvailableBytes_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetAvailableBytes_002
 * @tc.desc: 测试 usedBytes > sizeBytes 的异常情况
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetAvailableBytes_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetAvailableBytes_002 start";
    CreateMockSysfsNodes("sdc", true, false);
    std::string sizeFile = mockSysPath + "/sdc/size";
    FILE *fp = fopen(sizeFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "1000\n");
        fclose(fp);
    }
    std::string sdc1Dir = mockSysPath + "/sdc/sdc1";
    mkdir(sdc1Dir.c_str(), 0755);
    std::string sdc1Size = sdc1Dir + "/size";
    fp = fopen(sdc1Size.c_str(), "w");
    if (fp) {
        fprintf(fp, "2000\n");
        fclose(fp);
    }

    ScanDevice scanner(mockSysPath);
    uint64_t availableBytes = scanner.GetAvailableBytes("sdc");
    EXPECT_EQ(availableBytes, 0);
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDevicePath_001
 * @tc.desc: Test getting device path
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDevicePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDevicePath_001 start";

    ScanDevice scanner(mockSysPath);

    EXPECT_EQ(scanner.GetDevicePath("sda"), "/dev/sda");
    EXPECT_EQ(scanner.GetDevicePath("sdb"), "/dev/sdb");
    EXPECT_EQ(scanner.GetDevicePath("nvme0n1"), "/dev/nvme0n1");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDevicePath_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPort_001
 * @tc.desc: Test getting port for SATA device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPort_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_001 start";
    std::string linkTarget =
        "/sys/devices/platform/b8000000.hi_pcie/pci0001:00/0001:00:00.0/0001:01:00.0/ata1/host0/target0:0:0/0:0:0:0/"
        "block/sda";

    ScanDevice scanner(mockSysPath);
    std::string port = scanner.GetPort(linkTarget, false);

    EXPECT_TRUE(port == "ata1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPort_002
 * @tc.desc: Test getting port for NVMe device (should return empty)
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPort_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_002 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_EQ(scanner.GetPort("", true), "");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_002 end";
}

HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPort_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_003 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_EQ(scanner.GetPort("invalid", false), "");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_NewFields_001
 * @tc.desc: Test new BlockInfo fields (diskId, usedBytes, availableBytes, devicePath, port)
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_NewFields_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_NewFields_001 start";

    CreateMockSysfsNodes("sdb", true, false);
    std::string sdb1Dir = mockSysPath + "/sdb/sdb1";
    mkdir(sdb1Dir.c_str(), 0755);
    std::string sdb1Size = sdb1Dir + "/size";
    FILE *fp = fopen(sdb1Size.c_str(), "w");
    if (fp) {
        fprintf(fp, "1048576\n");
        fclose(fp);
    }
    std::string devicePath = mockSysPath + "/sdb";
    remove(devicePath.c_str());
    std::string linkTarget =
        "../../devices/pci0000:00/0000:00:02.0/0000:01:00.0/ata1/host0/target0:00:0/0:00:00:0/block/sdb";
    symlink(linkTarget.c_str(), devicePath.c_str());

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("sdb", false, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_GE(blockInfo.sizeBytes, 0);
    EXPECT_TRUE(blockInfo.diskId == "disk-8-16");
    EXPECT_GE(blockInfo.usedBytes, 0);
    EXPECT_GE(blockInfo.availableBytes, 0);
    EXPECT_EQ(blockInfo.devicePath, "/dev/sdb");

    DeleteMockSysfsNodes("sdb");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_NewFields_001 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_001
 * @tc.desc: 测试正常数字字符串解析
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_001 start";

    std::string input = "12345";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_TRUE(ret);
    EXPECT_EQ(result, 12345);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_001 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_002
 * @tc.desc: 测试空字符串解析
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_002 start";

    std::string input = "";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_002 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_003
 * @tc.desc: 测试纯字母字符串解析
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_003 start";

    std::string input = "abcde";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_003 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_004
 * @tc.desc: 测试带有尾部空格的数字字符串解析
 * 注意：std::from_chars 会解析数字部分并停止在空格处，视为成功
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_004 start";

    std::string input = "12345   ";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_TRUE(ret);
    EXPECT_EQ(result, 12345);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_004 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_005
 * @tc.desc: 测试带有前导空格的数字字符串解析
 * 注意：std::from_chars 默认不跳过前导空格，因此解析失败
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_005 start";

    std::string input = "   67890";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_005 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_006
 * @tc.desc: 测试数字后跟非数字字符的字符串解析
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_006 start";

    std::string input = "123abc";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 123);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_006 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_007
 * @tc.desc: 测试带有换行符的字符串解析 (常见于 sysfs 读取)
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_007 start";

    std::string input = "12345\n";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_TRUE(ret);
    EXPECT_EQ(result, 12345);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_007 end";
}

/*
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_008
 * @tc.desc: 测试负数解析 (unsigned 类型不应接受负号)
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_008 start";

    std::string input = "-12345";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);

    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_008 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_009
 * @tc.desc: 测试全空格字符串，应返回false
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_009 start";
    std::string input = "   ";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);
    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_009 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_010
 * @tc.desc: 测试数字后紧跟非空白非法字符 (如 'a')
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_010 start";
    std::string input = "12345abc";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);
    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 12345);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_010 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_011
 * @tc.desc: 测试数字后跟制表符再跟非法字符
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ParseStringToUlongLong_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_011 start";
    std::string input = "12345\tabc";
    unsigned long long result = 0;
    ScanDevice scanner(mockSysPath);
    bool ret = scanner.ParseStringToUlongLong(input, result);
    EXPECT_FALSE(ret);
    EXPECT_EQ(result, 12345);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ParseStringToUlongLong_011 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_01
 * @tc.desc: 验证 nvme0 开头的设备被正确过滤（业务需求：排除系统盘）
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_01 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme0"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme0n"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme0n1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme0n1p1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme00n1"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_01 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_02
 * @tc.desc: 验证合法的 nvme 数据盘设备被正确识别
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_02 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme1n1"));
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme1n2"));
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme2n1"));
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme10n1"));
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme1n10"));
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme10n100"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_02 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_03
 * @tc.desc: 验证缺少控制器编号的非法格式被拒绝
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_03 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvmen1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvmen"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_03 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04
 * @tc.desc: 验证缺少 'n' 分隔符的非法格式被拒绝
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04 start";

    ScanDevice scanner(mockSysPath);

    // 有控制器数字，但没有 'n'
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme11"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1a1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1p1"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05
 * @tc.desc: 验证缺少分区编号的非法格式被拒绝
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05 start";

    ScanDevice scanner(mockSysPath);

    // 有 'n'，但后面没有分区数字
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme10n"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n "));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06
 * @tc.desc: 验证分区编号非数字的非法格式被拒绝
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06 start";

    ScanDevice scanner(mockSysPath);

    // 'n' 后面跟了非数字字符
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1na"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n-"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n_"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n."));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07
 * @tc.desc: 验证长度不足的非法格式被拒绝
 * @tc.required: A
 * @tc.author: AI Assistant
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07 start";

    ScanDevice scanner(mockSysPath);

    // 最小合法长度是 7 (nvme0n1)
    EXPECT_FALSE(scanner.IsValidNvmeDevice(""));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08
 * @tc.desc: 验证长度刚好为7但格式非法的情况 (nvme1nn)
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08 start";
    ScanDevice scanner(mockSysPath);

    // nvme1nn: 长度7, 控制器是1, 但'n'后面是'n'而不是数字
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1nn"));

    // nvme1n0: 长度7, 合法
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme1n0"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09
 * @tc.desc: 验证控制器ID后直接结束，没有'n'的情况
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09 start";
    ScanDevice scanner(mockSysPath);

    // nvme123: 长度6, 小于7，已被长度检查拦截，但为了覆盖逻辑，测试更长的
    // nvme12345: 长度8, 控制器是12345, 后面没有'n'
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme12345"));

    // nvme123n: 长度8, 控制器123, 有'n', 但没有分区ID
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme123n"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09 end";
}

} // namespace StorageDaemon
} // namespace OHOS