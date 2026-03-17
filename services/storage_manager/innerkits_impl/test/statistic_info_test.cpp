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

/**
 * @tc.name: StatisticInfoTest_DirSpaceInfo_SpecialPaths_001
 * @tc.desc: Test DirSpaceInfo with special path characters.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_DirSpaceInfo_SpecialPaths_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_SpecialPaths_001 start";

    std::vector<std::string> specialPaths = {
        "/data/test/path/with/../../relative",
        "/data/test/path/with/./current",
        "/data/test/path with spaces",
        "/data/test/path/with/unicode/测试",
        "/data/test/path/with/symbols/!@#$%^&*()",
        "/data/test/path/with/multiple////slashes",
        "/data/test/path/with/null/character\0_in_middle",
        "relative/path/without/leading/slash",
        "./relative/current/path",
        "../relative/parent/path"
    };

    for (const auto& path : specialPaths) {
        DirSpaceInfo dirInfo(path, 1000, 1024);
        Parcel parcel;
        bool result = dirInfo.Marshalling(parcel);
        EXPECT_TRUE(result) << "Failed for path: " << path;

        std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDirInfo, nullptr) << "Unmarshalling failed for path: " << path;
        EXPECT_EQ(path, unmarshalledDirInfo->path);
    }

    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_SpecialPaths_001 end";
}

/**
 * @tc.name: StatisticInfoTest_DirSpaceInfo_MultipleMarshalling_001
 * @tc.desc: Test DirSpaceInfo with multiple marshalling operations.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_DirSpaceInfo_MultipleMarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_MultipleMarshalling_001 start";

    std::vector<DirSpaceInfo> dirInfos;
    for (int i = 0; i < 10; i++) {
        std::string path = "/data/test/path" + std::to_string(i);
        dirInfos.push_back(DirSpaceInfo(path, i * 1000, i * 1024));
    }

    Parcel parcel;
    for (const auto& dirInfo : dirInfos) {
        bool result = dirInfo.Marshalling(parcel);
        EXPECT_TRUE(result);
    }

    // Unmarshal all
    for (size_t i = 0; i < dirInfos.size(); i++) {
        std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDirInfo, nullptr);
        EXPECT_EQ(dirInfos[i].path, unmarshalledDirInfo->path);
    }

    GTEST_LOG_(INFO) << "StatisticInfoTest_DirSpaceInfo_MultipleMarshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_MixedTypes_Marshalling_001
 * @tc.desc: Test mixed marshalling of NextDqBlk and DirSpaceInfo.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_MixedTypes_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_MixedTypes_Marshalling_001 start";

    Parcel parcel;

    // Marshal alternating types
    for (int i = 0; i < 5; i++) {
        NextDqBlk dqBlk(i * 1024, i * 512, i * 2048, i * 1000, i * 500,
                        i * 50, i * 3600, i * 7200, i % 2, i);
        bool result1 = dqBlk.Marshalling(parcel);
        EXPECT_TRUE(result1);

        std::string path = "/data/test/path" + std::to_string(i);
        DirSpaceInfo dirInfo(path, i * 1000, i * 1024);
        bool result2 = dirInfo.Marshalling(parcel);
        EXPECT_TRUE(result2);
    }

    // Unmarshal alternating types
    for (int i = 0; i < 5; i++) {
        std::unique_ptr<NextDqBlk> unmarshalledDqBlk(NextDqBlk::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDqBlk, nullptr);
        EXPECT_EQ(i, unmarshalledDqBlk->dqbId);

        std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDirInfo, nullptr);
        std::string expectedPath = "/data/test/path" + std::to_string(i);
        EXPECT_EQ(expectedPath, unmarshalledDirInfo->path);
    }

    GTEST_LOG_(INFO) << "StatisticInfoTest_MixedTypes_Marshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_Performance_LargeData_001
 * @tc.desc: Test performance with large amount of data.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_Performance_LargeData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_Performance_LargeData_001 start";

    Parcel parcel;

    // Marshal 100 NextDqBlk objects
    for (int i = 0; i < 100; i++) {
        NextDqBlk dqBlk(i * 1024, i * 512, i * 2048, i * 1000, i * 500,
                        i * 50, i * 3600, i * 7200, i % 2, i);
        bool result = dqBlk.Marshalling(parcel);
        EXPECT_TRUE(result);
    }

    // Marshal 100 DirSpaceInfo objects
    for (int i = 0; i < 100; i++) {
        std::string path = "/data/test/long/path/directory/" + std::to_string(i);
        DirSpaceInfo dirInfo(path, i * 1000000, i * 1024000);
        bool result = dirInfo.Marshalling(parcel);
        EXPECT_TRUE(result);
    }

    // Unmarshal all NextDqBlk objects
    for (int i = 0; i < 100; i++) {
        std::unique_ptr<NextDqBlk> unmarshalledDqBlk(NextDqBlk::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDqBlk, nullptr);
        EXPECT_EQ(i, unmarshalledDqBlk->dqbId);
    }

    // Unmarshal all DirSpaceInfo objects
    for (int i = 0; i < 100; i++) {
        std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDirInfo, nullptr);
    }

    GTEST_LOG_(INFO) << "StatisticInfoTest_Performance_LargeData_001 end";
}

/**
 * @tc.name: StatisticInfoTest_EdgeCases_001
 * @tc.desc: Test edge cases for marshalling and unmarshalling.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_EdgeCases_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_EdgeCases_001 start";

    // Test with very large NextDqBlk time values
    NextDqBlk largeTimeDqBlk(1024, 512, 2048, 1000, 500, 50,
                            999999999, 999999999, 1, 100);
    Parcel parcel1;
    bool result1 = largeTimeDqBlk.Marshalling(parcel1);
    EXPECT_TRUE(result1);

    std::unique_ptr<NextDqBlk> unmarshalledLargeTime(NextDqBlk::Unmarshalling(parcel1));
    ASSERT_NE(unmarshalledLargeTime, nullptr);
    EXPECT_EQ(999999999, unmarshalledLargeTime->dqbBTime);

    // Test with DirSpaceInfo containing special characters in path
    std::string specialPath = "/data/test/path/with\ttabs\nnewlines\rcarriage\\returns";
    DirSpaceInfo specialPathInfo(specialPath, 1000, 1024);
    Parcel parcel2;
    bool result2 = specialPathInfo.Marshalling(parcel2);
    EXPECT_TRUE(result2);

    std::unique_ptr<DirSpaceInfo> unmarshalledSpecial(DirSpaceInfo::Unmarshalling(parcel2));
    ASSERT_NE(unmarshalledSpecial, nullptr);
    EXPECT_EQ(specialPath, unmarshalledSpecial->path);

    // Test with NextDqBlk having odd valid values
    NextDqBlk oddValidDqBlk(1024, 512, 2048, 1000, 500, 50,
                          3600, 7200, 12345, 100);
    Parcel parcel3;
    bool result3 = oddValidDqBlk.Marshalling(parcel3);
    EXPECT_TRUE(result3);

    std::unique_ptr<NextDqBlk> unmarshalledOddValid(NextDqBlk::Unmarshalling(parcel3));
    ASSERT_NE(unmarshalledOddValid, nullptr);
    EXPECT_EQ(12345, unmarshalledOddValid->dqbValid);

    // Test with DirSpaceInfo having very large path
    std::string veryLargePath;
    for (int i = 0; i < 1000; i++) {
        veryLargePath += "/subdir" + std::to_string(i);
    }
    DirSpaceInfo veryLargePathInfo(veryLargePath, 1000, 1024);
    Parcel parcel4;
    bool result4 = veryLargePathInfo.Marshalling(parcel4);
    EXPECT_TRUE(result4);

    std::unique_ptr<DirSpaceInfo> unmarshalledVeryLarge(DirSpaceInfo::Unmarshalling(parcel4));
    ASSERT_NE(unmarshalledVeryLarge, nullptr);
    EXPECT_EQ(veryLargePath, unmarshalledVeryLarge->path);

    GTEST_LOG_(INFO) << "StatisticInfoTest_EdgeCases_001 end";
}

/**
 * @tc.name: StatisticInfoTest_SequentialOperations_001
 * @tc.desc: Test sequential marshalling and unmarshalling operations.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_SequentialOperations_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_SequentialOperations_001 start";

    // Create and marshal objects sequentially
    std::vector<NextDqBlk> dqBlks;
    std::vector<DirSpaceInfo> dirInfos;

    Parcel parcel;

    for (int i = 0; i < 20; i++) {
        dqBlks.push_back(NextDqBlk(i * 1024, i * 512, i * 2048, i * 1000,
                                  i * 500, i * 50, i * 3600, i * 7200, i % 2, i));
        bool result1 = dqBlks.back().Marshalling(parcel);
        EXPECT_TRUE(result1);

        std::string path = "/data/test/path" + std::to_string(i);
        dirInfos.push_back(DirSpaceInfo(path, i * 1000, i * 1024));
        bool result2 = dirInfos.back().Marshalling(parcel);
        EXPECT_TRUE(result2);
    }

    // Unmarshal and verify sequentially
    for (int i = 0; i < 20; i++) {
        std::unique_ptr<NextDqBlk> unmarshalledDqBlk(NextDqBlk::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDqBlk, nullptr);
        EXPECT_EQ(dqBlks[i].dqbId, unmarshalledDqBlk->dqbId);
        EXPECT_EQ(dqBlks[i].dqbHardLimit, unmarshalledDqBlk->dqbHardLimit);

        std::unique_ptr<DirSpaceInfo> unmarshalledDirInfo(DirSpaceInfo::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledDirInfo, nullptr);
        EXPECT_EQ(dirInfos[i].path, unmarshalledDirInfo->path);
        EXPECT_EQ(dirInfos[i].size, unmarshalledDirInfo->size);
    }

    GTEST_LOG_(INFO) << "StatisticInfoTest_SequentialOperations_001 end";
}
} // namespace StorageManager
} // namespace OHOS
