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

#include <nlohmann/json.hpp>

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
    void CreateDeviceDir(const std::string &deviceName);
    void DeleteDeviceDir(const std::string &deviceName);

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

void ScanDeviceTest::CreateDeviceDir(const std::string &deviceName)
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
    CreateFileWithContent(devicePath + "/removable", "0\n");
    CreateFileWithContent(devicePath + "/dev", "8:16\n");
}

void ScanDeviceTest::DeleteDeviceDir(const std::string &deviceName)
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
 * @tc.desc: Test scanning with removable s* device (removable=1)
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_002 start";

    CreateDeviceDir("sdmock_b");
    CreateFileWithContent(mockSysPath + "/sdmock_b/removable", "1\n");

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 0);

    DeleteDeviceDir("sdmock_b");
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
 * @tc.desc: Test UFS device should be filtered out from data disks
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_005 start";
    CreateDeviceDir("sdmock_b");
    std::string removableFile = mockSysPath + "/sdmock_b/removable";
    std::remove(removableFile.c_str());
    std::string linkTarget = "/sys/devices/ufs/ufs0/host0/target0:00:0/0:00:00:0/block/sdmock_b";
    std::string devicePath = mockSysPath + "/sdmock_b";
    system(("rm -rf " + devicePath).c_str());
    symlink(linkTarget.c_str(), devicePath.c_str());

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 0);

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_005 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_006
 * @tc.desc: Test SATA device with removable=0 should be recognized as data disk
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_006 start";

    CreateDeviceDir("sdmock_a");

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 1);

    DeleteDeviceDir("sdmock_a");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_006 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_007
 * @tc.desc: Test NVMe device should be recognized as data disk
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_007 start";
    CreateDeviceDir("nvme1n1");

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 1);

    DeleteDeviceDir("nvme1n1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_007 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDataDisks_008
 * @tc.desc: Test device with invalid size can still be scanned
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDataDisks_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDataDisks_008 start";
    CreateDeviceDir("sdmock_b");
    CreateFileWithContent(mockSysPath + "/sdmock_b/size", "invalid_number\n");

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 1);

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("sdmock_b");
    CreateFileWithContent(mockSysPath + "/sdmock_b/removable", "1\n");

    ScanDevice scanner(mockSysPath);
    bool isRemovable = false;
    bool ret = scanner.ReadRemovableNode("sdmock_b", isRemovable);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(isRemovable);

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("sdmock_a");

    ScanDevice scanner(mockSysPath);
    bool isRemovable = false;
    bool ret = scanner.ReadRemovableNode("sdmock_a", isRemovable);

    EXPECT_TRUE(ret);
    EXPECT_FALSE(isRemovable);

    DeleteDeviceDir("sdmock_a");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadRemovableNode_003
 * @tc.desc: Test reading removable node with nonexistent device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadRemovableNode_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadRemovableNode_003 start";

    ScanDevice scanner(mockSysPath);
    bool isRemovable = false;
    bool ret = scanner.ReadRemovableNode("sdmock_a", isRemovable);
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
    bool ret = scanner.IsDataDisk("sdmock_a", false, false);
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
    bool ret = scanner.IsDataDisk("sdmock_b", false, true);
    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_003
 * @tc.desc: Test IsDataDisk with nvme0* system disk
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

    CreateDeviceDir("sdmock_a");
    std::string linkTarget = "/sys/devices/ufs/ufs0/host0/target0:00:0/0:00:00:0/block/sdmock_a";
    std::string devicePath = mockSysPath + "/sdmock_a";
    unlink(devicePath.c_str());
    symlink(linkTarget.c_str(), devicePath.c_str());

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sdmock_a", true, false);
    EXPECT_FALSE(ret);

    DeleteDeviceDir("sdmock_a");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_004 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_005
 * @tc.desc: Test IsDataDisk when readlink fails
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_005 start";
    std::string devicePath = mockSysPath + "/sdmock_a";
    mkdir(devicePath.c_str(), 0755);

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sdmock_a", true, false);
    EXPECT_FALSE(ret);

    system(("rm -rf " + devicePath).c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_005 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsDataDisk_006
 * @tc.desc: Test IsDataDisk when readlink succeeds and device is UFS
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsDataDisk_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsDataDisk_006 start";
    std::string devicePath = mockSysPath + "/sdmock_a";
    mkdir(devicePath.c_str(), 0755);
    std::string deviceDir = devicePath + "/device";
    mkdir(deviceDir.c_str(), 0755);
    std::string linkTarget = "../../devices/ufs/ufs0/host0/target0:0:0/0:0:0:0/block/sdmock_a";
    symlink(linkTarget.c_str(), devicePath.c_str());

    ScanDevice scanner(mockSysPath);
    bool ret = scanner.IsDataDisk("sdmock_a", true, false);
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    uint64_t size = scanner.GetDiskSize("sdmock_b");
    EXPECT_EQ(size, 1073741824ULL);

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskSize_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskSize_002
 * @tc.desc: Test getting disk size with nonexistent device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskSize_002 start";

    ScanDevice scanner(mockSysPath);
    uint64_t size = scanner.GetDiskSize("sdmock_b");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    std::string vendor = scanner.GetVendor("sdmock_b");
    EXPECT_EQ(vendor, "TestVendor");

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetVendor_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetVendor_002
 * @tc.desc: Test getting vendor information with nonexistent device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetVendor_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetVendor_002 start";

    ScanDevice scanner(mockSysPath);
    std::string vendor = scanner.GetVendor("sdmock_b");
    EXPECT_EQ(vendor, "Unknown");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    std::string model = scanner.GetModel("sdmock_b");
    EXPECT_EQ(model, "TestModel");

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetModel_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetModel_002
 * @tc.desc: Test getting model information with nonexistent device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetModel_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetModel_002 start";

    ScanDevice scanner(mockSysPath);
    std::string model = scanner.GetModel("sdmock_b");
    EXPECT_EQ(model, "Unknown");
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
    EXPECT_EQ(scanner.GetInterfaceType("sdmock_a"), "SATA");
    EXPECT_EQ(scanner.GetInterfaceType("sdmock_b"), "SATA");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    std::string state = scanner.GetDiskState("sdmock_b");
    EXPECT_EQ(state, "running");

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sdmock_b", false);
    EXPECT_EQ(type, MediaType::SSD);

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("sdmock_c");
    CreateFileWithContent(mockSysPath + "/sdmock_c/queue/rotational", "1\n");

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sdmock_c", false);
    EXPECT_EQ(type, MediaType::HDD);

    DeleteDeviceDir("sdmock_c");
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
 * @tc.name: Storage_Service_ScanDeviceTest_GetMediaType_004
 * @tc.desc: Test GetMediaType when rotational node read fails
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetMediaType_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_004 start";
    std::string devicePath = mockSysPath + "/sdmock_a";
    mkdir(devicePath.c_str(), 0755);

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sdmock_a", false);
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

    CreateDeviceDir("sdmock_b");
    std::string linkTarget =
        "/sys/devices/pci0000:00/0000:00:02.0/0000:01:00.0/host0/target0:00:0/0:00:00:0/block/sdmock_b";
    std::string devicePath = mockSysPath + "/sdmock_b";
    system(("rm -rf " + devicePath).c_str());
    symlink(linkTarget.c_str(), devicePath.c_str());

    ScanDevice scanner(mockSysPath);
    std::string pciePath = scanner.GetPciePath("sdmock_b");
    EXPECT_FALSE(pciePath.empty());
    EXPECT_TRUE(pciePath.find("pci0000:00") != std::string::npos);

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("sdmock_b", false, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(blockInfo.sizeBytes, 1073741824ULL);
    EXPECT_EQ(blockInfo.interfaceType, "SATA");
    EXPECT_EQ(blockInfo.vendor, "TestVendor");
    EXPECT_EQ(blockInfo.model, "TestModel");
    EXPECT_EQ(blockInfo.state, "running");
    EXPECT_EQ(blockInfo.mediaType, MediaType::SSD);

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("nvme1n1");

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("nvme1n1", true, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(blockInfo.interfaceType, "NVMe");

    DeleteDeviceDir("nvme1n1");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    std::string content;
    std::string testFile = mockSysPath + "/sdmock_b/size";

    bool ret = scanner.ReadSysfsNode(testFile, content);

    EXPECT_TRUE(ret);
    EXPECT_EQ(content, "2097152");

    DeleteDeviceDir("sdmock_b");
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
 * @tc.desc: Test ReadSysfsNode trims leading spaces
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_leading_test";
    CreateDirectory(mockSysPath);
    CreateFileWithContent(testFile, "   12345\n");
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result, "12345");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsLeadingSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001
 * @tc.desc: Test ReadSysfsNode trims trailing spaces
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_trailing_test";
    CreateDirectory(mockSysPath);
    CreateFileWithContent(testFile, "12345   \n");
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result, "12345");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsTrailingSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001
 * @tc.desc: Test ReadSysfsNode trims both leading and trailing spaces
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_both_test";
    CreateDirectory(mockSysPath);
    CreateFileWithContent(testFile, "   12345   \n");
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result, "12345");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_TrimsBothSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001
 * @tc.desc: Test ReadSysfsNode with all-spaces string returns empty string
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001 start";
    std::string testFile = mockSysPath + "/trim_all_spaces_test";
    CreateDirectory(mockSysPath);
    CreateFileWithContent(testFile, "     \n");
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result, "");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_AllSpaces_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001
 * @tc.desc: Test ReadSysfsNode handles mixed whitespace (spaces, tabs, etc.)
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001 start";
    std::string testFile = mockSysPath + "/trim_mixed_ws_test";
    CreateDirectory(mockSysPath);
    CreateFileWithContent(testFile, " \t  hello  \n");
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result, "hello");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_MixedWhitespace_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001
 * @tc.desc: Test ReadSysfsNode with no whitespace returns content as-is
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001 start";
    std::string testFile = mockSysPath + "/trim_no_ws_test";
    CreateDirectory(mockSysPath);
    CreateFileWithContent(testFile, "no_spaces");
    ScanDevice scanner(mockSysPath);
    std::string result;
    bool ret = scanner.ReadSysfsNode(testFile, result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result, "no_spaces");
    std::remove(testFile.c_str());
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_ReadSysfsNode_NoWhitespace_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskRpm_001
 * @tc.desc: Test getting disk RPM with nonexistent device
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskRpm_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskRpm_001 start";
    ScanDevice scanner(mockSysPath);
    uint32_t rpm = scanner.GetDiskRpm("nonexistent", false);
    EXPECT_EQ(rpm, 0);
    rpm = scanner.GetDiskRpm("sdmock_a", false);
    EXPECT_EQ(rpm, 0);
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
    CreateDeviceDir("nvme1n1");
    ScanDevice scanner(mockSysPath);
    std::string serial = scanner.GetSerialNumber("nvme1n1", true);
    EXPECT_EQ(serial, "Unknown");
    DeleteDeviceDir("nvme1n1");
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
    EXPECT_EQ(serial, "Unknown");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidNvmeDevice end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice
 * @tc.desc: Test getting serial number - SATA
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice start";
    CreateDeviceDir("sdmock_a");
    ScanDevice scanner(mockSysPath);
    std::string serial = scanner.GetSerialNumber("sdmock_a", false);
    EXPECT_EQ(serial, "Unknown");
    DeleteDeviceDir("sdmock_a");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestSataDevice end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice
 * @tc.desc: Test getting serial number - Invalid SATA name
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetSerialNumber_TestInvalidSataDevice start";
    ScanDevice scanner(mockSysPath);
    std::string serial = scanner.GetSerialNumber("nonexistent", false);
    EXPECT_EQ(serial, "Unknown");
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_EdgeCase_003
 * @tc.desc: Test edge case - ram device should not be data disk
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_EdgeCase_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_EdgeCase_003 start";
    CreateDeviceDir("ram");
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
    CreateDeviceDir("sdmock_b");
    CreateFileWithContent(mockSysPath + "/sdmock_b/removable", "1\n");
    CreateDeviceDir("sdmock_c");
    CreateDeviceDir("nvme1n1");

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetDataDisks();
    EXPECT_EQ(result.size(), 2);

    DeleteDeviceDir("sdmock_b");
    DeleteDeviceDir("sdmock_c");
    DeleteDeviceDir("nvme1n1");

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
    CreateDeviceDir("sdmock_b");
    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("sdmock_b", false, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(blockInfo.sizeBytes, 1073741824ULL);
    EXPECT_EQ(blockInfo.interfaceType, "SATA");
    EXPECT_EQ(blockInfo.rpm, 0);

    DeleteDeviceDir("sdmock_b");
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

    CreateDeviceDir("sdmock_b");

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("sdmock_b", false);
    EXPECT_EQ(diskId, "disk-8-16");

    DeleteDeviceDir("sdmock_b");
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
    CreateDeviceDir("sdmock_c");
    CreateFileWithContent(mockSysPath + "/sdmock_c/dev", "invalid_format\n");

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("sdmock_c", false);
    EXPECT_TRUE(diskId.empty());

    DeleteDeviceDir("sdmock_c");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskId_003
 * @tc.desc: Test GetDiskId with dev node containing newline
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskId_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_003 start";
    CreateDeviceDir("sdmock_c");
    CreateFileWithContent(mockSysPath + "/sdmock_c/dev", "8:32\n");

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("sdmock_c", false);
    EXPECT_EQ(diskId, "disk-8-32");

    DeleteDeviceDir("sdmock_c");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_003 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskId_004
 * @tc.desc: Test GetDiskId with uevent node containing newline
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskId_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_004 start";
    CreateDeviceDir("nvme1n1");
    std::string diskFile = mockSysPath + "/nvme1n1/uevent";
    FILE *fp = fopen(diskFile.c_str(), "w");
    if (fp) {
        fprintf(fp, "MAJOR=8\n");
        fprintf(fp, "MINOR=32\n");
        fclose(fp);
    }
 
    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("nvme1n1", true);
    EXPECT_EQ(diskId, "disk-8-32");
 
    DeleteDeviceDir("nvme1n1");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_004 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetUsedBytes_001
 * @tc.desc: Test getting used bytes
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetUsedBytes_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_001 start";

    CreateDeviceDir("sdmock_b");
    std::string part1Dir = mockSysPath + "/sdmock_b/sdmock_b1";
    mkdir(part1Dir.c_str(), 0755);
    CreateFileWithContent(part1Dir + "/size", "1048576\n");

    std::string part2Dir = mockSysPath + "/sdmock_b/sdmock_b2";
    mkdir(part2Dir.c_str(), 0755);
    CreateFileWithContent(part2Dir + "/size", "1048576\n");

    ScanDevice scanner(mockSysPath);
    uint64_t usedBytes = scanner.GetUsedBytes("sdmock_b");
    EXPECT_EQ(usedBytes, 1073741824ULL);

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetUsedBytes_002
 * @tc.desc: Test GetUsedBytes ignores subdirectories not ending with digit
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetUsedBytes_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_002 start";
    CreateDeviceDir("sdmock_b");
    std::string invalidDir = mockSysPath + "/sdmock_b/sdmock_b_invalid";
    mkdir(invalidDir.c_str(), 0755);
    CreateFileWithContent(invalidDir + "/size", "1000\n");

    ScanDevice scanner(mockSysPath);
    uint64_t usedBytes = scanner.GetUsedBytes("sdmock_b");
    EXPECT_EQ(usedBytes, 0);

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetUsedBytes_003
 * @tc.desc: Test GetUsedBytes with nonexistent device returns 0
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetUsedBytes_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetUsedBytes_003 start";
    ScanDevice scanner(mockSysPath);
    uint64_t usedBytes = scanner.GetUsedBytes("sdmock_b");
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

    CreateDeviceDir("sdmock_b");
    std::string part1Dir = mockSysPath + "/sdmock_b/sdmock_b1";
    mkdir(part1Dir.c_str(), 0755);
    CreateFileWithContent(part1Dir + "/size", "1048576\n");

    ScanDevice scanner(mockSysPath);
    uint64_t size = scanner.GetDiskSize("sdmock_b");
    uint64_t availableBytes = scanner.GetAvailableBytes(size, "sdmock_b");
    EXPECT_EQ(availableBytes, 536870912ULL);

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetAvailableBytes_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetAvailableBytes_002
 * @tc.desc: Test GetAvailableBytes when usedBytes > sizeBytes returns 0
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetAvailableBytes_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetAvailableBytes_002 start";
    CreateDeviceDir("sdmock_c");
    CreateFileWithContent(mockSysPath + "/sdmock_c/size", "1000\n");
    std::string part1Dir = mockSysPath + "/sdmock_c/sdmock_c1";
    mkdir(part1Dir.c_str(), 0755);
    CreateFileWithContent(part1Dir + "/size", "2000\n");

    ScanDevice scanner(mockSysPath);
    uint64_t size = scanner.GetDiskSize("sdmock_c");
    uint64_t availableBytes = scanner.GetAvailableBytes(size, "sdmock_c");
    EXPECT_EQ(availableBytes, 0);

    DeleteDeviceDir("sdmock_c");
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

    EXPECT_EQ(scanner.GetDevicePath("sdmock_a"), "/dev/block/sdmock_a");
    EXPECT_EQ(scanner.GetDevicePath("sdmock_b"), "/dev/block/sdmock_b");
    EXPECT_EQ(scanner.GetDevicePath("nvme0n1"), "/dev/block/nvme0n1");

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
        "block/sdmock_a";

    ScanDevice scanner(mockSysPath);
    std::string port = scanner.GetPort(linkTarget, false);

    EXPECT_EQ(port, "ata1");
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPort_003
 * @tc.desc: Test getting port with invalid path
 * @tc.type: FUNC
 */
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

    CreateDeviceDir("sdmock_b");
    std::string part1Dir = mockSysPath + "/sdmock_b/sdmock_b1";
    mkdir(part1Dir.c_str(), 0755);
    CreateFileWithContent(part1Dir + "/size", "1048576\n");

    ScanDevice scanner(mockSysPath);
    BlockInfo blockInfo;
    int ret = scanner.GetBlockInfo("sdmock_b", false, blockInfo);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(blockInfo.sizeBytes, 1073741824ULL);
    EXPECT_EQ(blockInfo.diskId, "disk-8-16");
    EXPECT_EQ(blockInfo.usedBytes, 536870912ULL);
    EXPECT_EQ(blockInfo.availableBytes, 536870912ULL);
    EXPECT_EQ(blockInfo.devicePath, "/dev/block/sdmock_b");

    DeleteDeviceDir("sdmock_b");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_NewFields_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_001
 * @tc.desc: Test parsing valid numeric string
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_002
 * @tc.desc: Test parsing empty string
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_003
 * @tc.desc: Test parsing pure alphabetic string
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_004
 * @tc.desc: Test parsing numeric string with trailing spaces
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_005
 * @tc.desc: Test parsing numeric string with leading spaces
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_006
 * @tc.desc: Test parsing numeric string followed by non-digit characters
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_007
 * @tc.desc: Test parsing string with newline (common in sysfs reads)
 * @tc.type: FUNC
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

/**
 * @tc.name: Storage_Service_ScanDeviceTest_ParseStringToUlongLong_008
 * @tc.desc: Test parsing negative number (unsigned type should reject minus sign)
 * @tc.type: FUNC
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
 * @tc.desc: Test parsing all-spaces string should return false
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
 * @tc.desc: Test parsing number followed by non-whitespace invalid character (e.g. 'a')
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
 * @tc.desc: Test parsing number followed by tab then invalid character
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
 * @tc.desc: Verify nvme0* devices are correctly filtered out as system disks
 * @tc.required: A
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
 * @tc.desc: Verify valid nvme data disk devices are correctly recognized
 * @tc.required: A
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
 * @tc.desc: Verify invalid format missing controller ID is rejected
 * @tc.required: A
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
 * @tc.desc: Verify invalid format missing 'n' separator is rejected
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme11"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1a1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1p1"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_04 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05
 * @tc.desc: Verify invalid format missing namespace ID is rejected
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme10n"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n "));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_05 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06
 * @tc.desc: Verify invalid format with non-digit namespace ID is rejected
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1na"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n-"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n_"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n."));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_06 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07
 * @tc.desc: Verify insufficient length invalid format is rejected
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07 start";

    ScanDevice scanner(mockSysPath);
    EXPECT_FALSE(scanner.IsValidNvmeDevice(""));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1n"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_07 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08
 * @tc.desc: Verify length 7 but invalid format (nvme1nn) is rejected
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08 start";
    ScanDevice scanner(mockSysPath);

    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme1nn"));
    EXPECT_TRUE(scanner.IsValidNvmeDevice("nvme1n0"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_08 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09
 * @tc.desc: Verify controller ID followed by no 'n' separator is rejected
 * @tc.required: A
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09 start";
    ScanDevice scanner(mockSysPath);

    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme12345"));
    EXPECT_FALSE(scanner.IsValidNvmeDevice("nvme123n"));

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_IsValidNvmeDevice_09 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_ToJson_001
 * @tc.desc: Test BlockInfo serialization to JSON
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_ToJson_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_ToJson_001 start";

    BlockInfo info;
    info.sizeBytes = 1073741824ULL;
    info.vendor = "TestVendor";
    info.model = "TestModel";
    info.interfaceType = "SATA";
    info.rpm = 5400;
    info.state = "running";
    info.mediaType = MediaType::SSD;
    info.removable = false;
    info.serialNumber = "SN12345";
    info.pciePath = "/sys/devices/pci0000:00";
    info.location = "slot1";
    info.diskId = "disk-8-16";
    info.usedBytes = 536870912ULL;
    info.availableBytes = 536870912ULL;
    info.devicePath = "/dev/block/sda";
    info.port = "ata1";

    json j = info.ToJson();
    EXPECT_EQ(j["sizeBytes"].get<uint64_t>(), 1073741824ULL);
    EXPECT_EQ(j["vendor"].get<std::string>(), "TestVendor");
    EXPECT_EQ(j["model"].get<std::string>(), "TestModel");
    EXPECT_EQ(j["interfaceType"].get<std::string>(), "SATA");
    EXPECT_EQ(j["rpm"].get<uint32_t>(), 5400);
    EXPECT_EQ(j["state"].get<std::string>(), "running");
    EXPECT_EQ(j["mediaType"].get<std::string>(), "SSD");
    EXPECT_EQ(j["removable"].get<bool>(), false);
    EXPECT_EQ(j["serialNumber"].get<std::string>(), "SN12345");
    EXPECT_EQ(j["pciePath"].get<std::string>(), "/sys/devices/pci0000:00");
    EXPECT_EQ(j["location"].get<std::string>(), "slot1");
    EXPECT_EQ(j["diskId"].get<std::string>(), "disk-8-16");
    EXPECT_EQ(j["usedBytes"].get<uint64_t>(), 536870912ULL);
    EXPECT_EQ(j["availableBytes"].get<uint64_t>(), 536870912ULL);
    EXPECT_EQ(j["devicePath"].get<std::string>(), "/dev/block/sda");
    EXPECT_EQ(j["port"].get<std::string>(), "ata1");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_ToJson_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_ToJson_Hdd
 * @tc.desc: Test BlockInfo serialization for HDD and UNKNOWN media types
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_ToJson_Hdd, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_ToJson_Hdd start";

    BlockInfo info;
    info.mediaType = MediaType::HDD;
    json j = info.ToJson();
    EXPECT_EQ(j["mediaType"].get<std::string>(), "HDD");

    info.mediaType = MediaType::UNKNOWN;
    j = info.ToJson();
    EXPECT_EQ(j["mediaType"].get<std::string>(), "UNKNOWN");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_ToJson_Hdd end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_FromJson_001
 * @tc.desc: Test BlockInfo deserialization from JSON
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_FromJson_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_FromJson_001 start";

    json j = {
        {"sizeBytes", 2147483648ULL},
        {"vendor", "VendorA"},
        {"model", "ModelB"},
        {"interfaceType", "NVMe"},
        {"rpm", 0},
        {"state", "offline"},
        {"mediaType", "SSD"},
        {"removable", true},
        {"serialNumber", "SN99999"},
        {"pciePath", "/sys/devices/pci0001"},
        {"location", "slot2"},
        {"diskId", "disk-8-32"},
        {"usedBytes", 1073741824ULL},
        {"availableBytes", 1073741824ULL},
        {"devicePath", "/dev/block/nvme1n1"},
        {"port", ""}
    };

    BlockInfo info = BlockInfo::FromJson(j);
    EXPECT_EQ(info.sizeBytes, 2147483648ULL);
    EXPECT_EQ(info.vendor, "VendorA");
    EXPECT_EQ(info.model, "ModelB");
    EXPECT_EQ(info.interfaceType, "NVMe");
    EXPECT_EQ(info.rpm, 0);
    EXPECT_EQ(info.state, "offline");
    EXPECT_EQ(info.mediaType, MediaType::SSD);
    EXPECT_EQ(info.removable, true);
    EXPECT_EQ(info.serialNumber, "SN99999");
    EXPECT_EQ(info.pciePath, "/sys/devices/pci0001");
    EXPECT_EQ(info.location, "slot2");
    EXPECT_EQ(info.diskId, "disk-8-32");
    EXPECT_EQ(info.usedBytes, 1073741824ULL);
    EXPECT_EQ(info.availableBytes, 1073741824ULL);
    EXPECT_EQ(info.devicePath, "/dev/block/nvme1n1");
    EXPECT_EQ(info.port, "");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_FromJson_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_FromJson_Defaults
 * @tc.desc: Test BlockInfo deserialization from empty JSON returns defaults
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_FromJson_Defaults, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_FromJson_Defaults start";

    json j = json::object();
    BlockInfo info = BlockInfo::FromJson(j);
    EXPECT_EQ(info.sizeBytes, 0ULL);
    EXPECT_EQ(info.vendor, "");
    EXPECT_EQ(info.model, "");
    EXPECT_EQ(info.interfaceType, "");
    EXPECT_EQ(info.rpm, 0U);
    EXPECT_EQ(info.state, "");
    EXPECT_EQ(info.mediaType, MediaType::UNKNOWN);
    EXPECT_EQ(info.removable, false);
    EXPECT_EQ(info.serialNumber, "");
    EXPECT_EQ(info.diskId, "");
    EXPECT_EQ(info.usedBytes, 0ULL);
    EXPECT_EQ(info.availableBytes, 0ULL);
    EXPECT_EQ(info.devicePath, "");
    EXPECT_EQ(info.port, "");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_FromJson_Defaults end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_001
 * @tc.desc: Test serializing BlockInfo vector to JSON string
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_001 start";

    BlockInfo info1;
    info1.sizeBytes = 1024;
    info1.vendor = "V1";
    info1.diskId = "disk-8-0";

    BlockInfo info2;
    info2.sizeBytes = 2048;
    info2.vendor = "V2";
    info2.diskId = "disk-8-1";

    std::vector<BlockInfo> infos = {info1, info2};
    std::string jsonStr = BlockInfo::SerializeVector(infos);

    EXPECT_FALSE(jsonStr.empty());

    json j = json::parse(jsonStr, nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    ASSERT_TRUE(j.is_array());
    EXPECT_EQ(j.size(), 2);
    EXPECT_EQ(j[0]["vendor"].get<std::string>(), "V1");
    EXPECT_EQ(j[0]["sizeBytes"].get<uint64_t>(), 1024);
    EXPECT_EQ(j[1]["vendor"].get<std::string>(), "V2");
    EXPECT_EQ(j[1]["sizeBytes"].get<uint64_t>(), 2048);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_Empty
 * @tc.desc: Test serializing empty BlockInfo vector returns "[]"
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_Empty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_Empty start";

    std::vector<BlockInfo> infos;
    std::string jsonStr = BlockInfo::SerializeVector(infos);
    EXPECT_EQ(jsonStr, "[]");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_SerializeVector_Empty end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_001
 * @tc.desc: Test deserializing JSON string to BlockInfo vector
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_001 start";

    BlockInfo info1;
    info1.sizeBytes = 1024;
    info1.vendor = "V1";
    info1.mediaType = MediaType::SSD;

    std::string jsonStr = BlockInfo::SerializeVector({info1});
    std::vector<BlockInfo> result = BlockInfo::DeserializeVector(jsonStr);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].sizeBytes, 1024);
    EXPECT_EQ(result[0].vendor, "V1");
    EXPECT_EQ(result[0].mediaType, MediaType::SSD);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_Empty
 * @tc.desc: Test deserializing empty string returns empty vector
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_Empty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_Empty start";

    std::vector<BlockInfo> result = BlockInfo::DeserializeVector("");
    EXPECT_EQ(result.size(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_Empty end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_InvalidJson
 * @tc.desc: Test deserializing invalid JSON string returns empty vector
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_InvalidJson, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_InvalidJson start";

    std::vector<BlockInfo> result = BlockInfo::DeserializeVector("not valid json");
    EXPECT_EQ(result.size(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_InvalidJson end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_NotArray
 * @tc.desc: Test deserializing non-array JSON returns empty vector
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_NotArray, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_NotArray start";

    std::vector<BlockInfo> result = BlockInfo::DeserializeVector("{\"key\": \"value\"}");
    EXPECT_EQ(result.size(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_DeserializeVector_NotArray end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_BlockInfo_RoundTrip
 * @tc.desc: Test BlockInfo serialize-then-deserialize round-trip consistency
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_BlockInfo_RoundTrip, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_RoundTrip start";

    BlockInfo original;
    original.sizeBytes = 999999ULL;
    original.vendor = "RoundVendor";
    original.model = "RoundModel";
    original.interfaceType = "NVMe";
    original.rpm = 100;
    original.state = "active";
    original.mediaType = MediaType::HDD;
    original.removable = true;
    original.serialNumber = "RT123";
    original.pciePath = "/pci/path";
    original.location = "loc";
    original.diskId = "disk-1-2";
    original.usedBytes = 500000ULL;
    original.availableBytes = 499999ULL;
    original.devicePath = "/dev/block/xxx";
    original.port = "ata3";

    std::string jsonStr = BlockInfo::SerializeVector({original});
    std::vector<BlockInfo> result = BlockInfo::DeserializeVector(jsonStr);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].sizeBytes, original.sizeBytes);
    EXPECT_EQ(result[0].vendor, original.vendor);
    EXPECT_EQ(result[0].model, original.model);
    EXPECT_EQ(result[0].interfaceType, original.interfaceType);
    EXPECT_EQ(result[0].rpm, original.rpm);
    EXPECT_EQ(result[0].state, original.state);
    EXPECT_EQ(result[0].mediaType, original.mediaType);
    EXPECT_EQ(result[0].removable, original.removable);
    EXPECT_EQ(result[0].serialNumber, original.serialNumber);
    EXPECT_EQ(result[0].pciePath, original.pciePath);
    EXPECT_EQ(result[0].location, original.location);
    EXPECT_EQ(result[0].diskId, original.diskId);
    EXPECT_EQ(result[0].usedBytes, original.usedBytes);
    EXPECT_EQ(result[0].availableBytes, original.availableBytes);
    EXPECT_EQ(result[0].devicePath, original.devicePath);
    EXPECT_EQ(result[0].port, original.port);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_BlockInfo_RoundTrip end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetExternalDisks_001
 * @tc.desc: Test scanning external disks with empty directory
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetExternalDisks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_001 start";

    ScanDevice scanner(mockSysPath);
    std::vector<BlockInfo> result = scanner.GetExternalDisks();
    EXPECT_EQ(result.size(), 0);

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetExternalDisks_001 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskState_002
 * @tc.desc: Test GetDiskState with nonexistent device returns Unknown
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskState_002 start";

    ScanDevice scanner(mockSysPath);
    std::string state = scanner.GetDiskState("nonexistent");
    EXPECT_EQ(state, "Unknown");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskState_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetMediaType_005
 * @tc.desc: Test GetMediaType with rotational value other than 0 or 1 returns UNKNOWN
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetMediaType_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_005 start";

    CreateDeviceDir("sdmock_d");
    CreateFileWithContent(mockSysPath + "/sdmock_d/queue/rotational", "2\n");

    ScanDevice scanner(mockSysPath);
    MediaType type = scanner.GetMediaType("sdmock_d", false);
    EXPECT_EQ(type, MediaType::UNKNOWN);

    DeleteDeviceDir("sdmock_d");
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetMediaType_005 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetDiskId_004
 * @tc.desc: Test GetDiskId with nonexistent device returns empty
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetDiskId_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_004 start";

    ScanDevice scanner(mockSysPath);
    std::string diskId = scanner.GetDiskId("nonexistent");
    EXPECT_TRUE(diskId.empty());

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetDiskId_004 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPciePath_002
 * @tc.desc: Test GetPciePath with nonexistent device returns empty
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPciePath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPciePath_002 start";

    ScanDevice scanner(mockSysPath);
    std::string pciePath = scanner.GetPciePath("nonexistent");
    EXPECT_TRUE(pciePath.empty());

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPciePath_002 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPort_004
 * @tc.desc: Test GetPort with empty path returns empty
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPort_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_004 start";

    ScanDevice scanner(mockSysPath);
    std::string port = scanner.GetPort("", false);
    EXPECT_EQ(port, "");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_004 end";
}

/**
 * @tc.name: Storage_Service_ScanDeviceTest_GetPort_005
 * @tc.desc: Test GetPort with path containing no ata info returns empty
 * @tc.type: FUNC
 */
HWTEST_F(ScanDeviceTest, Storage_Service_ScanDeviceTest_GetPort_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_005 start";

    ScanDevice scanner(mockSysPath);
    std::string port = scanner.GetPort("/sys/devices/pci0000:00/no_ata_here", false);
    EXPECT_EQ(port, "");

    GTEST_LOG_(INFO) << "Storage_Service_ScanDeviceTest_GetPort_005 end";
}

} // namespace StorageDaemon
} // namespace OHOS
