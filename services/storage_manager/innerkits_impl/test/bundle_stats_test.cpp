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

#include "bundle_stats.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class BundleStatsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_BUNDLE_STATS_Marshalling_0000
 * @tc.name: Bundle_Stats_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(BundleStatsTest, Bundle_Stats_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleStatsTest-begin Bundle_Stats_Marshalling_0000";
    int64_t appSize = 1001;
    int64_t cacheSize = 1002;
    int64_t dataSize = 1003;
    BundleStats bs(appSize, cacheSize, dataSize);
    Parcel parcel;
    bs.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadInt64(), appSize);
    EXPECT_EQ(parcel.ReadInt64(), cacheSize);
    EXPECT_EQ(parcel.ReadInt64(), dataSize);
    GTEST_LOG_(INFO) << "BundleStatsTest-end Bundle_Stats_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_BUNDLE_STATS_Unmarshalling_0000
 * @tc.name: Bundle_Stats_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(BundleStatsTest, Bundle_Stats_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BundleStatsTest-begin Bundle_Stats_Unmarshalling_0000";
    int64_t appSize = 2001;
    int64_t cacheSize = 2002;
    int64_t dataSize = 2003;
    Parcel parcel;
    parcel.WriteInt64(appSize);
    parcel.WriteInt64(cacheSize);
    parcel.WriteInt64(dataSize);
    BundleStats res;
    res = *BundleStats::Unmarshalling(parcel);
    EXPECT_EQ(res.appSize_, appSize);
    EXPECT_EQ(res.cacheSize_, cacheSize);
    EXPECT_EQ(res.dataSize_, dataSize);
    GTEST_LOG_(INFO) << "BundleStatsTest-end Bundle_Stats_Unmarshalling_0000";
}
}
