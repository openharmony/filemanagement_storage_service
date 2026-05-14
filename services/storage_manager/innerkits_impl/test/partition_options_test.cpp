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
#include "partition_options.h"

namespace {
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace StorageManager;

class PartitionOptionsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: PartitionOptions_GetPartitionNum_001
 * @tc.desc: Verify GetPartitionNum returns the correct partition number.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_GetPartitionNum_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_GetPartitionNum_001";

    PartitionOptions options;
    options.SetPartitionNum(1);
    int32_t result = options.GetPartitionNum();

    EXPECT_EQ(result, 1);

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_GetPartitionNum_001";
}

/**
 * @tc.name: PartitionOptions_GetStartSector_001
 * @tc.desc: Verify GetStartSector returns the correct start sector.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_GetStartSector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_GetStartSector_001";

    PartitionOptions options;
    uint64_t startSector = 2048;
    options.SetStartSector(startSector);
    uint64_t result = options.GetStartSector();

    EXPECT_EQ(result, startSector);

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_GetStartSector_001";
}

/**
 * @tc.name: PartitionOptions_GetEndSector_001
 * @tc.desc: Verify GetEndSector returns the correct end sector.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_GetEndSector_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_GetEndSector_001";

    PartitionOptions options;
    uint64_t endSector = 102400;
    options.SetEndSector(endSector);
    uint64_t result = options.GetEndSector();

    EXPECT_EQ(result, endSector);

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_GetEndSector_001";
}

/**
 * @tc.name: PartitionOptions_GetTypeCode_001
 * @tc.desc: Verify GetTypeCode returns the correct type code.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_GetTypeCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_GetTypeCode_001";

    PartitionOptions options;
    std::string typeCode = "ext4";
    options.SetTypeCode(typeCode);
    std::string result = options.GetTypeCode();

    EXPECT_EQ(result, typeCode);

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_GetTypeCode_001";
}

/**
 * @tc.name: PartitionOptions_Marshalling_001
 * @tc.desc: Verify Marshalling serializes PartitionOptions correctly.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_Marshalling_001";

    PartitionOptions options;
    options.SetPartitionNum(1);
    options.SetStartSector(2048);
    options.SetEndSector(102400);
    std::string typeCode = "ext4";
    options.SetTypeCode(typeCode);

    Parcel parcel;
    bool result = options.Marshalling(parcel);

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_Marshalling_001";
}

/**
 * @tc.name: PartitionOptions_Unmarshalling_001
 * @tc.desc: Verify Unmarshalling deserializes PartitionOptions correctly with ext4 type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_Unmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_Unmarshalling_001";

    PartitionOptions options;
    options.SetPartitionNum(1);
    options.SetStartSector(2048);
    options.SetEndSector(102400);
    std::string typeCode = "ext4";
    options.SetTypeCode(typeCode);

    Parcel parcel;
    options.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionOptions* result = PartitionOptions::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 1);
    EXPECT_EQ(result->GetStartSector(), 2048);
    EXPECT_EQ(result->GetEndSector(), 102400);
    EXPECT_EQ(result->GetTypeCode(), "ext4");

    delete result;

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_Unmarshalling_001";
}

/**
 * @tc.name: PartitionOptions_Unmarshalling_002
 * @tc.desc: Verify Unmarshalling deserializes PartitionOptions correctly with vfat type.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_Unmarshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_Unmarshalling_002";

    PartitionOptions options;
    options.SetPartitionNum(2);
    options.SetStartSector(102400);
    options.SetEndSector(204800);
    std::string typeCode = "vfat";
    options.SetTypeCode(typeCode);

    Parcel parcel;
    options.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionOptions* result = PartitionOptions::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 2);
    EXPECT_EQ(result->GetStartSector(), 102400);
    EXPECT_EQ(result->GetEndSector(), 204800);
    EXPECT_EQ(result->GetTypeCode(), "vfat");

    delete result;

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_Unmarshalling_002";
}

/**
 * @tc.name: PartitionOptions_MarshallingUnmarshalling_001
 * @tc.desc: Verify Marshalling and Unmarshalling with zero values for boundary testing.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(PartitionOptionsTest, PartitionOptions_MarshallingUnmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PartitionOptionsTest-begin PartitionOptions_MarshallingUnmarshalling_001";

    PartitionOptions options;
    options.SetPartitionNum(0);
    options.SetStartSector(0);
    options.SetEndSector(0);
    std::string typeCode = "";
    options.SetTypeCode(typeCode);

    Parcel parcel;
    options.Marshalling(parcel);
    parcel.RewindRead(0);

    PartitionOptions* result = PartitionOptions::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPartitionNum(), 0);
    EXPECT_EQ(result->GetStartSector(), 0);
    EXPECT_EQ(result->GetEndSector(), 0);
    EXPECT_EQ(result->GetTypeCode(), "");

    delete result;

    GTEST_LOG_(INFO) << "PartitionOptionsTest-end PartitionOptions_MarshallingUnmarshalling_001";
}
} // namespace
