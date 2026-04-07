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
 * @tc.name: StatisticInfoTest_UidSaInfo_Marshalling_001
 * @tc.desc: Test UidSaInfo Marshalling with default values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_UidSaInfo_Marshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Marshalling_001 start";

    UidSaInfo uidSaInfo;

    Parcel parcel;
    bool result = uidSaInfo.Marshalling(parcel);

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Marshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_UidSaInfo_Marshalling_002
 * @tc.desc: Test UidSaInfo Marshalling with custom values.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_UidSaInfo_Marshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Marshalling_002 start";

    UidSaInfo uidSaInfo(1001, "com.ohos.test", 2048, 100);

    Parcel parcel;
    bool result = uidSaInfo.Marshalling(parcel);

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Marshalling_002 end";
}

/**
 * @tc.name: StatisticInfoTest_UidSaInfo_Unmarshalling_001
 * @tc.desc: Test UidSaInfo Unmarshalling with valid data.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_UidSaInfo_Unmarshalling_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Unmarshalling_001 start";

    UidSaInfo originalUidSaInfo(1001, "com.ohos.test", 2048, 100);

    Parcel parcel;
    bool marshalResult = originalUidSaInfo.Marshalling(parcel);
    ASSERT_TRUE(marshalResult);

    std::unique_ptr<UidSaInfo> unmarshalledUidSaInfo(UidSaInfo::Unmarshalling(parcel));
    ASSERT_NE(unmarshalledUidSaInfo, nullptr);

    EXPECT_EQ(originalUidSaInfo.uid, unmarshalledUidSaInfo->uid);
    EXPECT_EQ(originalUidSaInfo.saName, unmarshalledUidSaInfo->saName);
    EXPECT_EQ(originalUidSaInfo.size, unmarshalledUidSaInfo->size);
    EXPECT_EQ(originalUidSaInfo.iNodes, unmarshalledUidSaInfo->iNodes);

    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Unmarshalling_001 end";
}

/**
 * @tc.name: StatisticInfoTest_UidSaInfo_Unmarshalling_002
 * @tc.desc: Test UidSaInfo Unmarshalling with empty parcel.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_UidSaInfo_Unmarshalling_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Unmarshalling_002 start";

    Parcel parcel;

    std::unique_ptr<UidSaInfo> uidSaInfo(UidSaInfo::Unmarshalling(parcel));

    EXPECT_NE(uidSaInfo, nullptr);

    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Unmarshalling_002 end";
}

/**
 * @tc.name: StatisticInfoTest_UidSaInfo_Roundtrip_001
 * @tc.desc: Test UidSaInfo roundtrip marshalling and unmarshalling.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_UidSaInfo_Roundtrip_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Roundtrip_001 start";

    std::vector<UidSaInfo> originalUidSaInfos = {
        UidSaInfo(1001, "com.ohos.test1", 2048, 100),
        UidSaInfo(2002, "com.ohos.test2", 4096, 200),
        UidSaInfo(3003, "com.ohos.test3", 8192, 300),
        UidSaInfo(0, "", 0, 0)
    };

    for (const auto& originalUidSaInfo : originalUidSaInfos) {
        Parcel parcel;
        bool marshalResult = originalUidSaInfo.Marshalling(parcel);
        ASSERT_TRUE(marshalResult);

        std::unique_ptr<UidSaInfo> unmarshalledUidSaInfo(UidSaInfo::Unmarshalling(parcel));
        ASSERT_NE(unmarshalledUidSaInfo, nullptr);

        EXPECT_EQ(originalUidSaInfo.uid, unmarshalledUidSaInfo->uid);
        EXPECT_EQ(originalUidSaInfo.saName, unmarshalledUidSaInfo->saName);
        EXPECT_EQ(originalUidSaInfo.size, unmarshalledUidSaInfo->size);
        EXPECT_EQ(originalUidSaInfo.iNodes, unmarshalledUidSaInfo->iNodes);
    }

    GTEST_LOG_(INFO) << "StatisticInfoTest_UidSaInfo_Roundtrip_001 end";
}

/**
 * @tc.name: StatisticInfoTest_AllAppVec_001
 * @tc.desc: Test AllAppVec structure.
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatisticInfoTest, StatisticInfoTest_AllAppVec_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatisticInfoTest_AllAppVec_001 start";

    AllAppVec allAppVec;

    allAppVec.sysSaVec = {UidSaInfo(1001, "sa1", 1024, 10)};
    allAppVec.sysAppVec = {UidSaInfo(2002, "app1", 2048, 20)};
    allAppVec.userAppVec = {UidSaInfo(3003, "user1", 4096, 30)};
    allAppVec.otherAppVec = {UidSaInfo(4004, "other1", 8192, 40)};

    EXPECT_EQ(allAppVec.sysSaVec.size(), 1);
    EXPECT_EQ(allAppVec.sysAppVec.size(), 1);
    EXPECT_EQ(allAppVec.userAppVec.size(), 1);
    EXPECT_EQ(allAppVec.otherAppVec.size(), 1);

    EXPECT_EQ(allAppVec.sysSaVec[0].uid, 1001);
    EXPECT_EQ(allAppVec.sysAppVec[0].uid, 2002);
    EXPECT_EQ(allAppVec.userAppVec[0].uid, 3003);
    EXPECT_EQ(allAppVec.otherAppVec[0].uid, 4004);

    GTEST_LOG_(INFO) << "StatisticInfoTest_AllAppVec_001 end";
}
} // namespace StorageManager
} // namespace OHOS
