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
    std::string diskId = "123";
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
    auto result5 = disk.GetFlag();
    EXPECT_EQ(result5, flag);
    disk.SetFlag(flag);
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
    std::string diskId = "123";
    int64_t sizeBytes = 1000;
    std::string sysPath = "/";
    std::string vendor = "";
    int32_t flag = USB_FLAG;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    Parcel parcel;
    auto result = disk.Marshalling(parcel);
    GTEST_LOG_(INFO) << parcel.ReadString();
    GTEST_LOG_(INFO) << parcel.ReadInt32();
    GTEST_LOG_(INFO) << parcel.ReadString();
    GTEST_LOG_(INFO) << parcel.ReadString();
    GTEST_LOG_(INFO) << parcel.ReadInt32();
    EXPECT_EQ(result, true);
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
    std::string diskId = "123";
    int64_t sizeBytes = 1000;
    std::string sysPath = "/";
    std::string vendor = "";
    int32_t flag = USB_FLAG;
    Parcel parcel;
    parcel.WriteString(diskId);
    parcel.WriteInt32(sizeBytes);
    parcel.WriteString(sysPath);
    parcel.WriteString(vendor);
    parcel.WriteInt32(flag);
    auto result = std::make_unique<Disk>();
    auto dk = std::make_unique<Disk>();
    result = dk->Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "DiskTest-end Disk_Unmarshalling_0000";
}
}