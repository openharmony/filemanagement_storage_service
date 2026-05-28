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
    int32_t diskType = USB_FLAG;
    bool removable = true;
    std::list<std::string> volumeIds = {"vol1", "vol2"};
    std::string extraInfo = "{\"vendor\":\"test\",\"product\":\"disk\"}";
    std::string vendor = "TestVendor";
    std::string sysPath = "/sys/block/sda";
    Disk disk(diskId, sizeBytes, diskType, removable, volumeIds, extraInfo, vendor, sysPath);
    auto result1 = disk.GetDiskId();
    EXPECT_EQ(result1, diskId);
    auto result2 = disk.GetSizeBytes();
    EXPECT_EQ(result2, sizeBytes);
    auto result3 = disk.GetDiskType();
    EXPECT_EQ(result3, diskType);
    auto result4 = disk.GetRemovable();
    EXPECT_EQ(result4, removable);
    auto result5 = disk.GetVolumeIds();
    EXPECT_EQ(result5.size(), volumeIds.size());
    auto result6 = disk.GetExtraInfo();
    EXPECT_EQ(result6, extraInfo);
    auto result7 = disk.GetVendor();
    EXPECT_EQ(result7, vendor);
    auto result8 = disk.GetSysPath();
    EXPECT_EQ(result8, sysPath);
    auto result9 = disk.GetRemovable();
    EXPECT_EQ(result9, removable);
    auto result10 = disk.GetExtraInfo();
    EXPECT_EQ(result10, extraInfo);
    auto result11 = disk.GetVolumeIds();
    EXPECT_EQ(result11, volumeIds);
    disk.SetDiskType(diskType);
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
    int32_t diskType = USB_FLAG;
    bool removable = false;
    std::list<std::string> volumeIds = {"vol1"};
    std::string extraInfo = "{\"vendor\":\"test\"}";
    std::string vendor = "Vendor200";
    std::string sysPath = "/sys/block/sdb";
    Disk disk(diskId, sizeBytes, diskType, removable, volumeIds, extraInfo, vendor, sysPath);
    Parcel parcel;
    disk.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadString(), diskId);
    EXPECT_EQ(parcel.ReadInt64(), sizeBytes);
    EXPECT_EQ(parcel.ReadInt32(), diskType);
    EXPECT_EQ(parcel.ReadBool(), removable);
    EXPECT_EQ(parcel.ReadString(), extraInfo);
    uint32_t volSize = parcel.ReadUint32();
    EXPECT_EQ(volSize, static_cast<uint32_t>(volumeIds.size()));
    EXPECT_EQ(parcel.ReadString(), "vol1");
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
    int32_t diskType = USB_FLAG;
    bool removable = true;
    std::string extraInfo = "{\"vendor\":\"test\"}";
    std::list<std::string> volumeIds = {"vol1", "vol2"};
    Parcel parcel;
    parcel.WriteString(diskId);
    parcel.WriteInt64(sizeBytes);
    parcel.WriteInt32(diskType);
    parcel.WriteBool(removable);
    parcel.WriteString(extraInfo);
    parcel.WriteUint32(static_cast<uint32_t>(volumeIds.size()));
    for (const auto &volId : volumeIds) {
        parcel.WriteString(volId);
    }
    auto result = Disk::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetDiskId(), diskId);
    EXPECT_EQ(result->GetSizeBytes(), sizeBytes);
    EXPECT_EQ(result->GetDiskType(), diskType);
    EXPECT_EQ(result->GetRemovable(), removable);
    EXPECT_EQ(result->GetExtraInfo(), extraInfo);
    EXPECT_EQ(result->GetVolumeIds().size(), volumeIds.size());
    delete result;
    GTEST_LOG_(INFO) << "DiskTest-end Disk_Unmarshalling_0000";
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
    int32_t diskType = USB_FLAG;
    bool removable = false;
    std::list<std::string> volumeIds;
    std::string extraInfo;
    std::string vendor;
    std::string sysPath;
    Disk disk(diskId, sizeBytes, diskType, removable, volumeIds, extraInfo, vendor, sysPath);

    auto result = disk.GetRemovable();
    EXPECT_EQ(result, removable);

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
    int32_t diskType = USB_FLAG;
    bool removable = true;
    std::list<std::string> volumeIds;
    std::string extraInfo = "{\"vendor\":\"TestVendor\",\"product\":\"TestProduct\"}";
    std::string vendor;
    std::string sysPath;
    Disk disk(diskId, sizeBytes, diskType, removable, volumeIds, extraInfo, vendor, sysPath);

    auto result = disk.GetExtraInfo();
    EXPECT_EQ(result, extraInfo);

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

    EXPECT_EQ(disk.GetDiskId(), "");
    EXPECT_EQ(disk.GetSizeBytes(), 0);
    EXPECT_EQ(disk.GetDiskType(), DISK_TYPE_UNKNOWN);
    EXPECT_EQ(disk.GetRemovable(), true);
    EXPECT_EQ(disk.GetVolumeIds().size(), static_cast<size_t>(0));
    EXPECT_EQ(disk.GetExtraInfo(), "");
    EXPECT_EQ(disk.GetVendor(), "");
    EXPECT_EQ(disk.GetSysPath(), "");

    GTEST_LOG_(INFO) << "DiskTest-end Disk_DefaultConstructor_0000";
}
}
