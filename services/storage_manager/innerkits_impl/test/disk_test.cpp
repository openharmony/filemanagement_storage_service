/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <gtest/gtest.h>

#include "disk.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class DiskTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Disk_Get_0000
 * @tc.name: Disk_Get_0000
 * @tc.desc: Test function of Get interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskTest, Disk_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_Get_0000";
    std::string diskId = "100";
    int64_t sizeBytes = 1000;
    std::string sysPath = "/";
    std::string vendor = "";
    int32_t flag = USB_FLAG;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    auto result1 = disk.GetDiskId();
    EXPECT_EQ(result1, diskId);
    auto result2 = disk.GetSizeBytes();
    EXPECT_EQ(result2, sizeBytes);
    auto result3 = disk.GetSysPath();
    EXPECT_EQ(result3, sysPath);
    auto result4 = disk.GetVendor();
    EXPECT_EQ(result4, vendor);
    auto result5 = disk.GetDiskType();
    EXPECT_EQ(result5, flag);
    disk.SetDiskType(flag);
    GTEST_LOG_(INFO) << "DiskTest-end Disk_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_Marshalling_0000
 * @tc.name: Disk_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskTest, Disk_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_Marshalling_0000";
    std::string diskId = "200";
    int64_t sizeBytes = 2000;
    std::string sysPath = "/";
    std::string vendor = "";
    int32_t flag = USB_FLAG;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    Parcel parcel;
    disk.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadString(), diskId);
    EXPECT_EQ(parcel.ReadInt32(), sizeBytes);
    EXPECT_EQ(parcel.ReadString(), sysPath);
    EXPECT_EQ(parcel.ReadString(), vendor);
    EXPECT_EQ(parcel.ReadInt32(), flag);
    GTEST_LOG_(INFO) << "DiskTest-end Disk_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_Unmarshalling_0000
 * @tc.name: Disk_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskTest, Disk_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_Unmarshalling_0000";
    std::string diskId = "300";
    int64_t sizeBytes = 3000;
    std::string sysPath = "/";
    std::string vendor = "";
    int32_t flag = USB_FLAG;
    Parcel parcel;
    parcel.WriteString(diskId);
    parcel.WriteInt32(sizeBytes);
    parcel.WriteString(sysPath);
    parcel.WriteString(vendor);
    parcel.WriteInt32(flag);
    Disk disk("400", 4000, "/", "", SD_FLAG);
    auto result = disk.Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetDiskId(), diskId);
    EXPECT_EQ(result->GetSizeBytes(), sizeBytes);
    EXPECT_EQ(result->GetSysPath(), sysPath);
    EXPECT_EQ(result->GetVendor(), vendor);
    EXPECT_EQ(result->GetDiskType(), flag);
    GTEST_LOG_(INFO) << "DiskTest-end Disk_Unmarshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_GetMediaType_0000
 * @tc.name: Disk_GetMediaType_0000
 * @tc.desc: Test function of GetMediaType interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskTest, Disk_GetMediaType_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_GetMediaType_0000";
    std::string diskId = "disk-8-0";
    int64_t sizeBytes = 1000;
    std::string sysPath = "/sys/block/sda";
    std::string vendor = "TestVendor";
    int32_t diskType = USB_FLAG;
    Disk disk(diskId, sizeBytes, sysPath, vendor, diskType);

    // GetMediaType returns default value (0 = SSD)
    auto mediaType = disk.GetMediaType();
    EXPECT_EQ(mediaType, 0);

    GTEST_LOG_(INFO) << "DiskTest-end Disk_GetMediaType_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_GetRemovable_0000
 * @tc.name: Disk_GetRemovable_0000
 * @tc.desc: Test function of GetRemovable interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskTest, Disk_GetRemovable_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_GetRemovable_0000";
    std::string diskId = "disk-8-0";
    int64_t sizeBytes = 1000;
    std::string sysPath = "/sys/block/sda";
    std::string vendor = "TestVendor";
    int32_t diskType = USB_FLAG;
    Disk disk(diskId, sizeBytes, sysPath, vendor, diskType);

    // GetRemovable returns default value (1)
    auto removable = disk.GetRemovable();
    EXPECT_EQ(removable, 1);

    GTEST_LOG_(INFO) << "DiskTest-end Disk_GetRemovable_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_GetExtraInfo_0000
 * @tc.name: Disk_GetExtraInfo_0000
 * @tc.desc: Test function of GetExtraInfo interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskTest, Disk_GetExtraInfo_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_GetExtraInfo_0000";
    std::string diskId = "disk-8-0";
    int64_t sizeBytes = 1000;
    std::string sysPath = "/sys/block/sda";
    std::string vendor = "TestVendor";
    int32_t diskType = USB_FLAG;
    Disk disk(diskId, sizeBytes, sysPath, vendor, diskType);

    // GetExtraInfo returns default value (empty string)
    auto extraInfo = disk.GetExtraInfo();
    EXPECT_EQ(extraInfo, "");

    GTEST_LOG_(INFO) << "DiskTest-end Disk_GetExtraInfo_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_DefaultConstructor_0000
 * @tc.name: Disk_DefaultConstructor_0000
 * @tc.desc: Test function of default constructor for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskTest, Disk_DefaultConstructor_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskTest-begin Disk_DefaultConstructor_0000";
    Disk disk;

    // Test default values
    EXPECT_EQ(disk.GetDiskId(), "");
    EXPECT_EQ(disk.GetSizeBytes(), 0);
    EXPECT_EQ(disk.GetSysPath(), "");
    EXPECT_EQ(disk.GetVendor(), "");
    EXPECT_EQ(disk.GetDiskType(), 0);
    EXPECT_EQ(disk.GetMediaType(), 0);
    EXPECT_EQ(disk.GetRemovable(), 1);
    EXPECT_EQ(disk.GetExtraInfo(), "");

    GTEST_LOG_(INFO) << "DiskTest-end Disk_DefaultConstructor_0000";
}
}
