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

#include <cstdio>
#include <gtest/gtest.h>

#include "format_params.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;

class FormatParamsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_FormatParams_Get_0000
 * @tc.name: FormatParams_Get_0000
 * @tc.desc: Test function of Get/Set interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatParamsTest, FormatParams_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatParamsTest-begin FormatParams_Get_0000";
    FormatParams formatParams;

    // Test default values
    EXPECT_EQ(formatParams.GetFsType(), "hmfs");
    EXPECT_TRUE(formatParams.GetQuickFormat());
    EXPECT_EQ(formatParams.GetVolumeName(), "");

    // Test SetFsType and GetFsType
    std::string fsType = "ext4";
    formatParams.SetFsType(fsType);
    EXPECT_EQ(formatParams.GetFsType(), fsType);

    // Test SetQuickFormat and GetQuickFormat
    formatParams.SetQuickFormat(false);
    EXPECT_FALSE(formatParams.GetQuickFormat());

    formatParams.SetQuickFormat(true);
    EXPECT_TRUE(formatParams.GetQuickFormat());

    // Test SetVolumeName and GetVolumeName
    std::string volName = "test_volume";
    formatParams.SetVolumeName(volName);
    EXPECT_EQ(formatParams.GetVolumeName(), volName);

    GTEST_LOG_(INFO) << "FormatParamsTest-end FormatParams_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatParams_SetFsType_0000
 * @tc.name: FormatParams_SetFsType_0000
 * @tc.desc: Test SetFsType with different file system types.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatParamsTest, FormatParams_SetFsType_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatParamsTest-begin FormatParams_SetFsType_0000";
    FormatParams formatParams;

    // Test with ext4
    formatParams.SetFsType("ext4");
    EXPECT_EQ(formatParams.GetFsType(), "ext4");

    // Test with vfat
    formatParams.SetFsType("vfat");
    EXPECT_EQ(formatParams.GetFsType(), "vfat");

    // Test with exfat
    formatParams.SetFsType("exfat");
    EXPECT_EQ(formatParams.GetFsType(), "exfat");

    // Test with ntfs
    formatParams.SetFsType("ntfs");
    EXPECT_EQ(formatParams.GetFsType(), "ntfs");

    // Test with f2fs
    formatParams.SetFsType("f2fs");
    EXPECT_EQ(formatParams.GetFsType(), "f2fs");

    // Test with empty string
    formatParams.SetFsType("");
    EXPECT_EQ(formatParams.GetFsType(), "");

    GTEST_LOG_(INFO) << "FormatParamsTest-end FormatParams_SetFsType_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatParams_Marshalling_0000
 * @tc.name: FormatParams_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatParamsTest, FormatParams_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatParamsTest-begin FormatParams_Marshalling_0000";
    FormatParams formatParams;
    formatParams.SetFsType("ext4");
    formatParams.SetQuickFormat(false);
    formatParams.SetVolumeName("test_vol");

    Parcel parcel;
    bool ret = formatParams.Marshalling(parcel);
    EXPECT_TRUE(ret);
    EXPECT_EQ(parcel.ReadString(), "ext4");
    EXPECT_EQ(parcel.ReadBool(), false);
    EXPECT_EQ(parcel.ReadString(), "test_vol");

    GTEST_LOG_(INFO) << "FormatParamsTest-end FormatParams_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatParams_Unmarshalling_0000
 * @tc.name: FormatParams_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatParamsTest, FormatParams_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatParamsTest-begin FormatParams_Unmarshalling_0000";
    Parcel parcel;
    std::string fsType = "vfat";
    bool quickFormat = true;
    std::string volName = "my_volume";

    parcel.WriteString(fsType);
    parcel.WriteBool(quickFormat);
    parcel.WriteString(volName);

    FormatParams* result = FormatParams::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetFsType(), fsType);
    EXPECT_EQ(result->GetQuickFormat(), quickFormat);
    EXPECT_EQ(result->GetVolumeName(), volName);
    delete result;

    GTEST_LOG_(INFO) << "FormatParamsTest-end FormatParams_Unmarshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatParams_RoundTrip_0000
 * @tc.name: FormatParams_RoundTrip_0000
 * @tc.desc: Test Marshalling and Unmarshalling round trip.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatParamsTest, FormatParams_RoundTrip_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatParamsTest-begin FormatParams_RoundTrip_0000";
    FormatParams original;
    original.SetFsType("exfat");
    original.SetQuickFormat(false);
    original.SetVolumeName("round_trip_test");

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    FormatParams* restored = FormatParams::Unmarshalling(parcel);
    ASSERT_TRUE(restored != nullptr);

    EXPECT_EQ(restored->GetFsType(), original.GetFsType());
    EXPECT_EQ(restored->GetQuickFormat(), original.GetQuickFormat());
    EXPECT_EQ(restored->GetVolumeName(), original.GetVolumeName());
    delete restored;

    GTEST_LOG_(INFO) << "FormatParamsTest-end FormatParams_RoundTrip_0000";
}
}
