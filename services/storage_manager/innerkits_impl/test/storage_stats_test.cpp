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

#include "storage_stats.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class StorageStatsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_STORAGE_STATS_Marshalling_0000
 * @tc.name: Storage_Stats_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageStatsTest, Storage_Stats_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatsTest-begin Storage_Stats_Marshalling_0000";
    int64_t totalSize = 1001;
    int64_t audioSize = 1002;
    int64_t videoSize = 1003;
    int64_t imageSize = 1004;
    int64_t fileSize = 1005;
    int64_t appSize = 1006;
    StorageStats res(totalSize, audioSize, videoSize, imageSize, fileSize, appSize);
    Parcel parcel;
    res.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadInt64(), totalSize);
    EXPECT_EQ(parcel.ReadInt64(), audioSize);
    EXPECT_EQ(parcel.ReadInt64(), videoSize);
    EXPECT_EQ(parcel.ReadInt64(), imageSize);
    EXPECT_EQ(parcel.ReadInt64(), fileSize);
    EXPECT_EQ(parcel.ReadInt64(), appSize);
    GTEST_LOG_(INFO) << "StorageStatsTest-end Storage_Stats_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_STORAGE_STATS_Unmarshalling_0000
 * @tc.name: Storage_Stats_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageStatsTest, Storage_Stats_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageStatsTest-begin Storage_Stats_Unmarshalling_0000";
    int64_t totalSize = 2001;
    int64_t audioSize = 2002;
    int64_t videoSize = 2003;
    int64_t imageSize = 2004;
    int64_t fileSize = 2005;
    int64_t appSize = 2006;
    Parcel parcel;
    parcel.WriteInt64(totalSize);
    parcel.WriteInt64(audioSize);
    parcel.WriteInt64(videoSize);
    parcel.WriteInt64(imageSize);
    parcel.WriteInt64(fileSize);
    parcel.WriteInt64(appSize);
    StorageStats res;
    res = *StorageStats::Unmarshalling(parcel);
    EXPECT_EQ(res.total_, totalSize);
    EXPECT_EQ(res.audio_, audioSize);
    EXPECT_EQ(res.video_, videoSize);
    EXPECT_EQ(res.image_, imageSize);
    EXPECT_EQ(res.file_, fileSize);
    EXPECT_EQ(res.app_, appSize);
    GTEST_LOG_(INFO) << "StorageStatsTest-end Storage_Stats_Unmarshalling_0000";
}
}
