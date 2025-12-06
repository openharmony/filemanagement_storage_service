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

#include "userdata_dir_info.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class UserdataDirInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Userdata_Dir_Info_Marshalling_0000
 * @tc.name: Userdata_Dir_Info_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20251022750568
 */
HWTEST_F(UserdataDirInfoTest, Userdata_Dir_Info_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UserdataDirInfoTest-begin Bundle_Stats_Marshalling_0000";
    std::string path = "/data";
    int64_t totalSize = 1002;
    int64_t totalCnt = 1003;
    UserdataDirInfo bs(path, totalSize, totalCnt);
    Parcel parcel;
    bs.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadString(), path);
    EXPECT_EQ(parcel.ReadInt64(), totalSize);
    EXPECT_EQ(parcel.ReadInt32(), totalCnt);
    GTEST_LOG_(INFO) << "UserdataDirInfoTest-end Bundle_Stats_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_BUNDLE_STATS_Unmarshalling_0000
 * @tc.name: Bundle_Stats_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20251022750568
 */
HWTEST_F(UserdataDirInfoTest, Userdata_Dir_Info_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UserdataDirInfoTest-begin Bundle_Stats_Unmarshalling_0000";
    std::string path = "/data";
    int64_t totalSize = 2002;
    int64_t totalCnt = 2003;
    Parcel parcel;
    parcel.WriteString(path);
    parcel.WriteInt64(totalSize);
    parcel.WriteInt32(totalCnt);
    UserdataDirInfo res;
    res = *UserdataDirInfo::Unmarshalling(parcel);
    EXPECT_EQ(res.path_, path);
    EXPECT_EQ(res.totalSize_, totalSize);
    EXPECT_EQ(res.totalCnt_, totalCnt);
    GTEST_LOG_(INFO) << "UserdataDirInfoTest-end Bundle_Stats_Unmarshalling_0000";
}
}