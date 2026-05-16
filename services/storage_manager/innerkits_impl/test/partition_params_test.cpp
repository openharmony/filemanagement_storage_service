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
#include "partition_params.h"

namespace {
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace StorageManager;

class PartitionParamsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: PartitionParams_GetPartitionNum_001
 * @tc.desc: Verify GetPartitionNum returns the correct partition number.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_GetPartitionNum_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_GetPartitionNum_001";

    PartitionParams partitionParams;
    partitionParams.SetPartitionNum(1);
    int32_t result = partitionParams.GetPartitionNum();

    EXPECT_EQ(result, 1);

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_GetPartitionNum_001";
}

/**
 * @tc.name: PartitionParams_GetStartSector_001
 * @tc.desc: Verify GetStartSector returns the correct start sector.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_GetStartSector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_GetStartSector_001";

    PartitionParams partitionParams;
    uint64_t startSector = 2048;
    partitionParams.SetStartSector(startSector);
    uint64_t result = partitionParams.GetStartSector();

    EXPECT_EQ(result, startSector);

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_GetStartSector_001";
}

/**
 * @tc.name: PartitionParams_GetEndSector_001
 * @tc.desc: Verify GetEndSector returns the correct end sector.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_GetEndSector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_GetEndSector_001";

    PartitionParams partitionParams;
    uint64_t endSector = 102400;
    partitionParams.SetEndSector(endSector);
    uint64_t result = partitionParams.GetEndSector();

    EXPECT_EQ(result, endSector);

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_GetEndSector_001";
}

/**
 * @tc.name: PartitionParams_GetTypeCode_001
 * @tc.desc: Verify GetTypeCode returns the correct type code.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_GetTypeCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_GetTypeCode_001";

    PartitionParams partitionParams;
    std::string typeCode = "ext4";
    partitionParams.SetTypeCode(typeCode);
    std::string result = partitionParams.GetTypeCode();

    EXPECT_EQ(result, typeCode);

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_GetTypeCode_001";
}

/**
 * @tc.name: PartitionParams_Marshalling_001
 * @tc.desc: Verify Marshalling serializes PartitionParams correctly.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_Marshalling_001";

    PartitionParams partitionParams;
    partitionParams.SetPartitionNum(1);
    partitionParams.SetStartSector(2048);
    partitionParams.SetEndSector(102400);
    std::string typeCode = "ext4";
    partitionParams.SetTypeCode(typeCode);

    Parcel parcel;
    bool result = partitionParams.Marshalling(parcel);

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_Marshalling_001";
}

/**
 * @tc.name: PartitionParams_Unmarshalling_001
 * @tc.desc: Verify Unmarshalling deserializes PartitionParams correctly with ext4 type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_Unmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_Unmarshalling_001";

    PartitionParams partitionParams;
    partitionParams.SetPartitionNum(1);
    partitionParams.SetStartSector(2048);
    partitionParams.SetEndSector(102400);
    std::string typeCode = "ext4";
    partitionParams.SetTypeCode(typeCode);

    Parcel parcel;
    partitionParams.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionParams* result = PartitionParams::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 1);
    EXPECT_EQ(result->GetStartSector(), 2048);
    EXPECT_EQ(result->GetEndSector(), 102400);
    EXPECT_EQ(result->GetTypeCode(), "ext4");

    delete result;

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_Unmarshalling_001";
}

/**
 * @tc.name: PartitionParams_Unmarshalling_002
 * @tc.desc: Verify Unmarshalling deserializes PartitionParams correctly with vfat type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_Unmarshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_Unmarshalling_002";

    PartitionParams partitionParams;
    partitionParams.SetPartitionNum(2);
    partitionParams.SetStartSector(102400);
    partitionParams.SetEndSector(204800);
    std::string typeCode = "vfat";
    partitionParams.SetTypeCode(typeCode);

    Parcel parcel;
    partitionParams.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionParams* result = PartitionParams::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 2);
    EXPECT_EQ(result->GetStartSector(), 102400);
    EXPECT_EQ(result->GetEndSector(), 204800);
    EXPECT_EQ(result->GetTypeCode(), "vfat");

    delete result;

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_Unmarshalling_002";
}

/**
 * @tc.name: PartitionParams_MarshallingUnmarshalling_001
 * @tc.desc: Verify Marshalling and Unmarshalling with zero values for boundary testing.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionParamsTest, PartitionParams_MarshallingUnmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionParamsTest-begin PartitionParams_MarshallingUnmarshalling_001";

    PartitionParams partitionParams;
    partitionParams.SetPartitionNum(0);
    partitionParams.SetStartSector(0);
    partitionParams.SetEndSector(0);
    std::string typeCode = "";
    partitionParams.SetTypeCode(typeCode);

    Parcel parcel;
    partitionParams.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionParams* result = PartitionParams::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 0);
    EXPECT_EQ(result->GetStartSector(), 0);
    EXPECT_EQ(result->GetEndSector(), 0);
    EXPECT_EQ(result->GetTypeCode(), "");

    delete result;

    GTEST_LOG_(INFO) << "PartitionParamsTest-end PartitionParams_MarshallingUnmarshalling_001";
}
} // namespace
