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
#include "parcel.h"
#include "partition_table_info.h"
#include "partition_info.h"

namespace {
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace StorageManager;

class PartitionTableInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: PartitionTableInfo_GetDiskId_001
 * @tc.desc: Verify GetDiskId returns the correct disk ID.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetDiskId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetDiskId_001";

    PartitionTableInfo info;
    std::string diskId = "disk-8-0";
    info.SetDiskId(diskId);
    std::string result = info.GetDiskId();

    EXPECT_EQ(result, diskId);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetDiskId_001";
}

/**
 * @tc.name: PartitionTableInfo_GetTableType_001
 * @tc.desc: Verify GetTableType returns GPT type correctly.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetTableType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetTableType_001";

    PartitionTableInfo info;
    std::string tableType = "GPT";
    info.SetTableType(tableType);
    std::string result = info.GetTableType();

    EXPECT_EQ(result, tableType);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetTableType_001";
}

/**
 * @tc.name: PartitionTableInfo_GetTableType_002
 * @tc.desc: Verify GetTableType returns MBR type correctly.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetTableType_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetTableType_002";

    PartitionTableInfo info;
    std::string tableType = "MBR";
    info.SetTableType(tableType);
    std::string result = info.GetTableType();

    EXPECT_EQ(result, tableType);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetTableType_002";
}

/**
 * @tc.name: PartitionTableInfo_GetPartitionCount_001
 * @tc.desc: Verify GetPartitionCount returns the correct partition count.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetPartitionCount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetPartitionCount_001";

    PartitionTableInfo info;
    uint32_t partitionCount = 4;
    info.SetPartitionCount(partitionCount);
    uint32_t result = info.GetPartitionCount();

    EXPECT_EQ(result, partitionCount);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetPartitionCount_001";
}

/**
 * @tc.name: PartitionTableInfo_GetTotalSector_001
 * @tc.desc: Verify GetTotalSector returns the correct total sector count.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetTotalSector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetTotalSector_001";

    PartitionTableInfo info;
    uint64_t totalSector = 1234567;
    info.SetTotalSector(totalSector);
    uint64_t result = info.GetTotalSector();

    EXPECT_EQ(result, totalSector);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetTotalSector_001";
}

/**
 * @tc.name: PartitionTableInfo_GetSectorSize_001
 * @tc.desc: Verify GetSectorSize returns the correct sector size.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetSectorSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetSectorSize_001";

    PartitionTableInfo info;
    uint32_t sectorSize = 512;
    info.SetSectorSize(sectorSize);
    uint32_t result = info.GetSectorSize();

    EXPECT_EQ(result, sectorSize);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetSectorSize_001";
}

/**
 * @tc.name: PartitionTableInfo_GetAlignSector_001
 * @tc.desc: Verify GetAlignSector returns the correct align sector.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetAlignSector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetAlignSector_001";

    PartitionTableInfo info;
    uint32_t alignSector = 2048;
    info.SetAlignSector(alignSector);
    uint32_t result = info.GetAlignSector();

    EXPECT_EQ(result, alignSector);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetAlignSector_001";
}

/**
 * @tc.name: PartitionTableInfo_GetPartitions_001
 * @tc.desc: Verify GetPartitions returns the correct partition list.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_GetPartitions_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_GetPartitions_001";

    PartitionTableInfo info;
    std::vector<PartitionInfo> partitions;
    PartitionInfo partition1;
    partition1.SetPartitionNum(1);
    partitions.push_back(partition1);
    info.SetPartitions(partitions);

    std::vector<PartitionInfo> result = info.GetPartitions();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].GetPartitionNum(), 1);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_GetPartitions_001";
}

/**
 * @tc.name: PartitionTableInfo_Marshalling_001
 * @tc.desc: Verify Marshalling serializes PartitionTableInfo correctly.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Marshalling_001";

    PartitionTableInfo info;
    info.SetDiskId("disk-8-0");
    info.SetTableType("GPT");
    info.SetPartitionCount(2);
    info.SetTotalSector(1234567);
    info.SetSectorSize(512);
    info.SetAlignSector(2048);

    Parcel parcel;
    bool result = info.Marshalling(parcel);

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Marshalling_001";
}

/**
 * @tc.name: PartitionTableInfo_Unmarshalling_001
 * @tc.desc: Verify Unmarshalling deserializes PartitionTableInfo correctly with GPT type.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Unmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Unmarshalling_001";

    PartitionTableInfo info;
    info.SetDiskId("disk-8-0");
    info.SetTableType("GPT");
    info.SetPartitionCount(2);
    info.SetTotalSector(1234567);
    info.SetSectorSize(512);
    info.SetAlignSector(2048);

    Parcel parcel;
    info.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionTableInfo* result = PartitionTableInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetDiskId(), "disk-8-0");
    EXPECT_EQ(result->GetTableType(), "GPT");
    EXPECT_EQ(result->GetPartitionCount(), 2);
    EXPECT_EQ(result->GetTotalSector(), 1234567);
    EXPECT_EQ(result->GetSectorSize(), 512);
    EXPECT_EQ(result->GetAlignSector(), 2048);

    delete result;

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Unmarshalling_001";
}

/**
 * @tc.name: PartitionTableInfo_Unmarshalling_002
 * @tc.desc: Verify Unmarshalling deserializes PartitionTableInfo correctly with MBR type and partitions.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_Unmarshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_Unmarshalling_002";

    PartitionTableInfo info;
    info.SetDiskId("disk-8-1");
    info.SetTableType("MBR");
    info.SetPartitionCount(1);
    info.SetTotalSector(204800);
    info.SetSectorSize(512);
    info.SetAlignSector(2048);

    std::vector<PartitionInfo> partitions;
    PartitionInfo partition1;
    partition1.SetPartitionNum(1);
    partitions.push_back(partition1);
    info.SetPartitions(partitions);

    Parcel parcel;
    info.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionTableInfo* result = PartitionTableInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetDiskId(), "disk-8-1");
    EXPECT_EQ(result->GetTableType(), "MBR");
    EXPECT_EQ(result->GetPartitionCount(), 1);
    std::vector<PartitionInfo> resultPartitions = result->GetPartitions();
    EXPECT_EQ(resultPartitions.size(), 1);
    EXPECT_EQ(resultPartitions[0].GetPartitionNum(), 1);

    delete result;

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_Unmarshalling_002";
}

/**
 * @tc.name: PartitionTableInfo_MarshallingUnmarshalling_001
 * @tc.desc: Verify Marshalling and Unmarshalling with empty values for boundary testing.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(PartitionTableInfoTest, PartitionTableInfo_MarshallingUnmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionTableInfoTest-begin PartitionTableInfo_MarshallingUnmarshalling_001";

    PartitionTableInfo info;
    info.SetDiskId("");
    info.SetTableType("");
    info.SetPartitionCount(0);
    info.SetTotalSector(0);
    info.SetSectorSize(0);
    info.SetAlignSector(0);

    Parcel parcel;
    info.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionTableInfo* result = PartitionTableInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetDiskId(), "");
    EXPECT_EQ(result->GetTableType(), "");
    EXPECT_EQ(result->GetPartitionCount(), 0);
    EXPECT_EQ(result->GetTotalSector(), 0);
    EXPECT_EQ(result->GetSectorSize(), 0);
    EXPECT_EQ(result->GetAlignSector(), 0);

    delete result;

    GTEST_LOG_(INFO) << "PartitionTableInfoTest-end PartitionTableInfo_MarshallingUnmarshalling_001";
}
} // namespace
