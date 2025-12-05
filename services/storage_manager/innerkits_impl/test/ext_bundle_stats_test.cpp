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

#include <cstdio>
#include <gtest/gtest.h>

#include "ext_bundle_stats.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class ExtBundleStatsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_EXT_BUNDLE_STATS_Marshalling_0000
 * @tc.name: Ext_Bundle_Stats_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(ExtBundleStatsTest, Bundle_Stats_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtBundleStatsTest-begin Ext_Bundle_Stats_Marshalling_0000";
    string businessName = "test_business";
    uint64_t businessSize = 1024 * 1024;
    bool showFlag = true;
    ExtBundleStats ebs(businessName, businessSize, showFlag);
    Parcel parcel;
    ebs.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadString(), businessName);
    EXPECT_EQ(parcel.ReadUint64(), businessSize);
    EXPECT_EQ(parcel.ReadBool(), showFlag);

    GTEST_LOG_(INFO) << "ExtBundleStatsTest-end Ext_Bundle_Stats_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_EXT_BUNDLE_STATS_Unmarshalling_0000
 * @tc.name: Ext_Bundle_Stats_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(ExtBundleStatsTest, Ext_Bundle_Stats_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtBundleStatsTest-begin Ext_Bundle_Stats_Unmarshalling_0000";
    string businessName = "test_business_2";
    uint64_t businessSize = 2048 * 1024;
    bool showFlag = false;
    Parcel parcel;
    parcel.WriteString(businessName);
    parcel.WriteUint64(businessSize);
    parcel.WriteBool(showFlag);
    auto result = ExtBundleStats::Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    if (result != nullptr) {
        ExtBundleStats res = *result;
        EXPECT_EQ(res.businessName_, businessName);
        EXPECT_EQ(res.businessSize_, businessSize);
        EXPECT_EQ(res.showFlag_, showFlag);
    }
    GTEST_LOG_(INFO) << "ExtBundleStatsTest-end Ext_Bundle_Stats_Unmarshalling_0000";
}
}
