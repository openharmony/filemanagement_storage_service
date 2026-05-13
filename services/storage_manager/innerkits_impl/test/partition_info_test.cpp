/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "partition_info.h"
#include "partition_options.h"
#include "partition_table_info.h"

namespace {
using namespace OHOS;
using namespace StorageManager;

class PartitionInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_PartitionInfo_Get_0000
 * @tc.name: PartitionInfo_Get_0000
 * @tc.desc: Test function of Get/Set interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionInfoTest, PartitionInfo_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionInfoTest-begin PartitionInfo_Get_0000";
    PartitionInfo info;
    info.SetPartitionNum(1);
    info.SetDiskId("disk-8-0");
    info.SetStartSector(2048);
    info.SetEndSector(4096);
    info.SetSizeBytes(1024);
    info.SetFsType("ext4");
    EXPECT_EQ(info.GetPartitionNum(), 1);
    EXPECT_EQ(info.GetDiskId(), "disk-8-0");
    EXPECT_EQ(info.GetStartSector(), 2048);
    EXPECT_EQ(info.GetEndSector(), 4096);
    EXPECT_EQ(info.GetSizeBytes(), 1024);
    EXPECT_EQ(info.GetFsType(), "ext4");
    GTEST_LOG_(INFO) << "PartitionInfoTest-end PartitionInfo_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionInfo_Marshalling_0000
 * @tc.name: PartitionInfo_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionInfoTest, PartitionInfo_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionInfoTest-begin PartitionInfo_Marshalling_0000";
    PartitionInfo info;
    info.SetPartitionNum(1);
    info.SetDiskId("disk-8-0");
    info.SetStartSector(2048);
    info.SetEndSector(4096);
    info.SetSizeBytes(1024);
    info.SetFsType("ext4");
    Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "PartitionInfoTest-end PartitionInfo_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionInfo_Unmarshalling_0000
 * @tc.name: PartitionInfo_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionInfoTest, PartitionInfo_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionInfoTest-begin PartitionInfo_Unmarshalling_0000";
    Parcel parcel;
    parcel.WriteUint32(1);
    parcel.WriteString("disk-8-0");
    parcel.WriteUint64(2048);
    parcel.WriteUint64(4096);
    parcel.WriteUint64(1024);
    parcel.WriteString("ext4");
    parcel.RewindRead(0);
    PartitionInfo* result = PartitionInfo::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 1);
    EXPECT_EQ(result->GetDiskId(), "disk-8-0");
    EXPECT_EQ(result->GetStartSector(), 2048);
    EXPECT_EQ(result->GetEndSector(), 4096);
    EXPECT_EQ(result->GetSizeBytes(), 1024);
    EXPECT_EQ(result->GetFsType(), "ext4");
    delete result;
    GTEST_LOG_(INFO) << "PartitionInfoTest-end PartitionInfo_Unmarshalling_0000";
}

class PartitionOptionsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_PartitionOptions_Get_0000
 * @tc.name: PartitionOptions_Get_0000
 * @tc.desc: Test function of Get/Set interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_Get_0000";
    PartitionOptions options;
    options.SetPartitionNum(1);
    options.SetStartSector(2048);
    options.SetEndSector(4096);
    std::string typeCode = "0x8300";
    options.SetTypeCode(typeCode);
    EXPECT_EQ(options.GetPartitionNum(), 1);
    EXPECT_EQ(options.GetStartSector(), 2048);
    EXPECT_EQ(options.GetEndSector(), 4096);
    EXPECT_EQ(options.GetTypeCode(), "0x8300");
    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionOptions_Marshalling_0000
 * @tc.name: PartitionOptions_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_Marshalling_0000";
    PartitionOptions options;
    options.SetPartitionNum(1);
    options.SetStartSector(2048);
    options.SetEndSector(4096);
    std::string typeCode = "0x8300";
    options.SetTypeCode(typeCode);
    Parcel parcel;
    bool ret = options.Marshalling(parcel);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionOptions_Unmarshalling_0000
 * @tc.name: PartitionOptions_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_Unmarshalling_0000";
    Parcel parcel;
    parcel.WriteUint32(1);
    parcel.WriteUint64(2048);
    parcel.WriteUint64(4096);
    parcel.WriteString("0x8300");
    parcel.RewindRead(0);
    PartitionOptions* result = PartitionOptions::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 1);
    EXPECT_EQ(result->GetStartSector(), 2048);
    EXPECT_EQ(result->GetEndSector(), 4096);
    EXPECT_EQ(result->GetTypeCode(), "0x8300");
    delete result;
    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_Unmarshalling_0000";
}

class PartitionTableInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_PartitionTableInfo_Get_0000
 * @tc.name: PartitionTableInfo_Get_0000
 * @tc.desc: Test function of Get/Set interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Get_0000";
    PartitionTableInfo info;
    info.SetDiskId("disk-8-0");
    info.SetTableType("gpt");
    info.SetPartitionCount(2);
    info.SetTotalSector(1234567);
    info.SetSectorSize(512);
    info.SetAlignSector(2048);
    EXPECT_EQ(info.GetDiskId(), "disk-8-0");
    EXPECT_EQ(info.GetTableType(), "gpt");
    EXPECT_EQ(info.GetPartitionCount(), 2);
    EXPECT_EQ(info.GetTotalSector(), 1234567);
    EXPECT_EQ(info.GetSectorSize(), 512);
    EXPECT_EQ(info.GetAlignSector(), 2048);
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionTableInfo_Partitions_0000
 * @tc.name: PartitionTableInfo_Partitions_0000
 * @tc.desc: Test function of Partitions interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Partitions_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Partitions_0000";
    PartitionTableInfo info;
    std::vector<PartitionInfo> partitions;
    PartitionInfo partition1;
    partition1.SetPartitionNum(1);
    partition1.SetDiskId("disk-8-0");
    partitions.push_back(partition1);
    info.SetPartitions(partitions);
    EXPECT_EQ(info.GetPartitions().size(), 1);
    EXPECT_EQ(info.GetPartitions()[0].GetPartitionNum(), 1);
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Partitions_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionTableInfo_Marshalling_0000
 * @tc.name: PartitionTableInfo_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Marshalling_0000";
    PartitionTableInfo info;
    info.SetDiskId("disk-8-0");
    info.SetTableType("gpt");
    info.SetPartitionCount(1);
    info.SetTotalSector(1234567);
    info.SetSectorSize(512);
    info.SetAlignSector(2048);
    Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_PartitionTableInfo_Unmarshalling_0000
 * @tc.name: PartitionTableInfo_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Unmarshalling_0000";
    Parcel parcel;
    parcel.WriteString("disk-8-0");
    parcel.WriteString("gpt");
    parcel.WriteUint32(1);
    parcel.WriteUint64(1234567);
    parcel.WriteUint32(512);
    parcel.WriteUint32(2048);
    parcel.WriteUint32(0);
    PartitionTableInfo* result = PartitionTableInfo::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetDiskId(), "disk-8-0");
    EXPECT_EQ(result->GetTableType(), "gpt");
    EXPECT_EQ(result->GetPartitionCount(), 1);
    delete result;
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Unmarshalling_0000";
}
}
