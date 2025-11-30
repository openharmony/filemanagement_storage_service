/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "statistic_info.h"
#include "parcel.h"

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;

class StatisticInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: StatisticInfoTest_NextDqBlk_Marshalling_001
 * @tc.desc: Test NextDqBlk Marshalling with default values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_NextDqBlk_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Marshalling_001 start";
    
    // Create a NextDqBlk with default values
    NextDqBlk dqBlk;
    
    Parcel parcel;
    bool result = dqBlk.Marshalling(parcel);
    
    EXPECT_TRUE(result);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Marshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_NextDqBlk_Marshalling_002
 * @tc.desc: Test NextDqBlk Marshalling with custom values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_NextDqBlk_Marshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Marshalling_002 start";
    
    // Create a NextDqBlk with custom values
    NextDqBlk dqBlk(
        1024,        // dqbHardLimit
        512,         // dqbBSoftLimit
        2048,        // dqbCurSpace
        1000,        // dqbIHardLimit
        500,         // dqbISoftLimit
        50,          // dqbCurInodes
        3600,        // dqbBTime
        7200,        // dqbITime
        1,           // dqbValid
        100          // dqbId
    );
    
    Parcel parcel;
    bool result = dqBlk.Marshalling(parcel);
    
    EXPECT_TRUE(result);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Marshalling_002 end";
}

/**
 * @tc.name: StatisticInfoTest_NextDqBlk_Marshalling_003
 * @tc.desc: Test NextDqBlk Marshalling with maximum values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_NextDqBlk_Marshalling_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Marshalling_003 start";
    
    // Create a NextDqBlk with maximum values
    NextDqBlk dqBlk(
        UINT64_MAX,   // dqbHardLimit
        UINT64_MAX,   // dqbBSoftLimit
        UINT64_MAX,   // dqbCurSpace
        UINT64_MAX,   // dqbIHardLimit
        UINT64_MAX,   // dqbISoftLimit
        UINT64_MAX,   // dqbCurInodes
        UINT64_MAX,   // dqbBTime
        UINT64_MAX,   // dqbITime
        UINT32_MAX,   // dqbValid
        UINT32_MAX    // dqbId
    );
    
    Parcel parcel;
    bool result = dqBlk.Marshalling(parcel);
    
    EXPECT_TRUE(result);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Marshalling_003 end";
}

/**
 * @tc.name: StatisticInfoTest_NextDqBlk_Unmarshalling_001
 * @tc.desc: Test NextDqBlk Unmarshalling with valid data.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_NextDqBlk_Unmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Unmarshalling_001 start";
    
    // Create a NextDqBlk with custom values
    NextDqBlk originalDqBlk(
        1024,        // dqbHardLimit
        512,         // dqbBSoftLimit
        2048,        // dqbCurSpace
        1000,        // dqbIHardLimit
        500,         // dqbISoftLimit
        50,          // dqbCurInodes
        3600,        // dqbBTime
        7200,        // dqbITime
        1,           // dqbValid
        100          // dqbId
    );
    
    // Marshal the original object
    Parcel parcel;
    bool marshalResult = originalDqBlk.Marshalling(parcel);
    ASSERT_TRUE(marshalResult);
    
    std::unique_ptr<NextDqBlk> unmarshalledDqBlk(NextDqBlk::Unmarshalling(parcel));
    ASSERT_NE(unmarshalledDqBlk, nullptr);
    
    EXPECT_EQ(originalDqBlk.dqbHardLimit, unmarshalledDqBlk->dqbHardLimit);
    EXPECT_EQ(originalDqBlk.dqbBSoftLimit, unmarshalledDqBlk->dqbBSoftLimit);
    EXPECT_EQ(originalDqBlk.dqbCurSpace, unmarshalledDqBlk->dqbCurSpace);
    EXPECT_EQ(originalDqBlk.dqbIHardLimit, unmarshalledDqBlk->dqbIHardLimit);
    EXPECT_EQ(originalDqBlk.dqbISoftLimit, unmarshalledDqBlk->dqbISoftLimit);
    EXPECT_EQ(originalDqBlk.dqbCurInodes, unmarshalledDqBlk->dqbCurInodes);
    EXPECT_EQ(originalDqBlk.dqbBTime, unmarshalledDqBlk->dqbBTime);
    EXPECT_EQ(originalDqBlk.dqbITime, unmarshalledDqBlk->dqbITime);
    EXPECT_EQ(originalDqBlk.dqbValid, unmarshalledDqBlk->dqbValid);
    EXPECT_EQ(originalDqBlk.dqbId, unmarshalledDqBlk->dqbId);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Unmarshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_NextDqBlk_Unmarshalling_002
 * @tc.desc: Test NextDqBlk Unmarshalling with empty parcel.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_NextDqBlk_Unmarshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Unmarshalling_002 start";
    
    Parcel parcel;
    
    std::unique_ptr<NextDqBlk> dqBlk(NextDqBlk::Unmarshalling(parcel));
    
    EXPECT_NE(dqBlk, nullptr);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_Unmarshalling_002 end";
}

/**
 * @tc.name: StatisticInfoTest_NextDqBlk_MarshallingUnmarshalling_001
 * @tc.desc: Test NextDqBlk roundtrip marshalling and unmarshalling.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_NextDqBlk_MarshallingUnmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_MarshallingUnmarshalling_001 start";
    
    std::vector<NextDqBlk> originalDqBlks = {
        NextDqBlk(1024, 512, 2048, 1000, 500, 50, 3600, 7200, 1, 100),
        NextDqBlk(2048, 1024, 4096, 2000, 1000, 100, 7200, 14400, 1, 200),
        NextDqBlk(0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    };
    
    for (const auto& originalDqBlk : originalDqBlks) {
        Parcel parcel;
        bool marshalResult = originalDqBlk.Marshalling(parcel);
        ASSERT_TRUE(marshalResult);
        
        std::unique_ptr<NextDqBlk> unmarshalledDqBlk(NextDqBlk::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDqBlk, nullptr);
        
        EXPECT_EQ(originalDqBlk.dqbHardLimit, unmarshalledDqBlk->dqbHardLimit);
        EXPECT_EQ(originalDqBlk.dqbBSoftLimit, unmarshalledDqBlk->dqbBSoftLimit);
        EXPECT_EQ(originalDqBlk.dqbCurSpace, unmarshalledDqBlk->dqbCurSpace);
        EXPECT_EQ(originalDqBlk.dqbIHardLimit, unmarshalledDqBlk->dqbIHardLimit);
        EXPECT_EQ(originalDqBlk.dqbISoftLimit, unmarshalledDqBlk->dqbISoftLimit);
        EXPECT_EQ(originalDqBlk.dqbCurInodes, unmarshalledDqBlk->dqbCurInodes);
        EXPECT_EQ(originalDqBlk.dqbBTime, unmarshalledDqBlk->dqbBTime);
        EXPECT_EQ(originalDqBlk.dqbITime, unmarshalledDqBlk->dqbITime);
        EXPECT_EQ(originalDqBlk.dqbValid, unmarshalledDqBlk->dqbValid);
        EXPECT_EQ(originalDqBlk.dqbId, unmarshalledDqBlk->dqbId);
    }
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_NextDqBlk_MarshallingUnmarshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_DirSpaceInfo_Marshalling_001
 * @tc.desc: Test DirSpaceInfo Marshalling with default values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_DirSpaceInfo_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Marshalling_001 start";
    
    DirSpaceInfo dirInfo;
    
    Parcel parcel;
    bool result = dirInfo.Marshalling(parcel);
    
    EXPECT_TRUE(result);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Marshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_DirSpaceInfo_Marshalling_002
 * @tc.desc: Test DirSpaceInfo Marshalling with custom values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_DirSpaceInfo_Marshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Marshalling_002 start";
    
    DirSpaceInfo dirInfo("/data/test/path", 1000, 1024);
    
    Parcel parcel;
    bool result = dirInfo.Marshalling(parcel);
    
    EXPECT_TRUE(result);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Marshalling_002 end";
}

/**
 * @tc.name: StatisticInfoTest_DirSpaceInfo_Unmarshalling_001
 * @tc.desc: Test DirSpaceInfo Unmarshalling with valid data.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_DirSpaceInfo_Unmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Unmarshalling_001 start";
    
    DirSpaceInfo originalDirInfo("/data/test/path", 1000, 1024);
    
    Parcel parcel;
    bool marshalResult = originalDirInfo.Marshalling(parcel);
    ASSERT_TRUE(marshalResult);
    
    std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel));
    ASSERT_NE(unmarshalledDirInfo, nullptr);
    
    EXPECT_EQ(originalDirInfo.path, unmarshalledDirInfo->path);
    EXPECT_EQ(originalDirInfo.size, unmarshalledDirInfo->size);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Unmarshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_DirSpaceInfo_Unmarshalling_002
 * @tc.desc: Test DirSpaceInfo Unmarshalling with empty parcel.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_DirSpaceInfo_Unmarshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Unmarshalling_002 start";
    
    Parcel parcel;
    
    std::unique_ptr<DirSpaceInfo> dirInfo(DirSpaceInfo::Unmarshalling(parcel));
    
    EXPECT_NE(dirInfo, nullptr);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_Unmarshalling_002 end";
}

/**
 * @tc.name: StatisticInfoTest_CombinedUsage_001
 * @tc.desc: Test combined usage of NextDqBlk and DirSpaceInfo.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_CombinedUsage_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_CombinedUsage_001 start";
    
    NextDqBlk dqBlk(1024, 512, 2048, 1000, 500, 50, 3600, 7200, 1, 100);
    
    Parcel parcel1;
    bool marshalResult1 = dqBlk.Marshalling(parcel1);
    ASSERT_TRUE(marshalResult1);
    
    DirSpaceInfo dirInfo("/data/test/path", 1000, 1024);
    
    bool marshalResult2 = dirInfo.Marshalling(parcel1);
    ASSERT_TRUE(marshalResult2);
    
    std::unique_ptr<NextDqBlk> unmarshalledDqBlk(NextDqBlk::Unmarshalling(parcel1));
    ASSERT_NE(unmarshalledDqBlk, nullptr);
    
    std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel1));
    ASSERT_NE(unmarshalledDirInfo, nullptr);
    
    EXPECT_EQ(dqBlk.dqbHardLimit, unmarshalledDqBlk->dqbHardLimit);
    EXPECT_EQ(dqBlk.dqbBSoftLimit, unmarshalledDqBlk->dqbBSoftLimit);
    EXPECT_EQ(dqBlk.dqbCurSpace, unmarshalledDqBlk->dqbCurSpace);
    EXPECT_EQ(dqBlk.dqbIHardLimit, unmarshalledDqBlk->dqbIHardLimit);
    EXPECT_EQ(dqBlk.dqbISoftLimit, unmarshalledDqBlk->dqbISoftLimit);
    EXPECT_EQ(dqBlk.dqbCurInodes, unmarshalledDqBlk->dqbCurInodes);
    EXPECT_EQ(dqBlk.dqbBTime, unmarshalledDqBlk->dqbBTime);
    EXPECT_EQ(dqBlk.dqbITime, unmarshalledDqBlk->dqbITime);
    EXPECT_EQ(dqBlk.dqbValid, unmarshalledDqBlk->dqbValid);
    EXPECT_EQ(dqBlk.dqbId, unmarshalledDqBlk->dqbId);
    
    EXPECT_EQ(dirInfo.path, unmarshalledDirInfo->path);
    EXPECT_EQ(dirInfo.size, unmarshalledDirInfo->size);
    
    GTEST_LOG_(INFO) << "StatisticInfoTest_CombinedUsage_001 end";
}

} // namespace StorageManager
} // namespace OHOS
