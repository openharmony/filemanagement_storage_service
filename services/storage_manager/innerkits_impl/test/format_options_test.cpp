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

#include "format_options.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;

class FormatOptionsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_FormatOptions_Get_0000
 * @tc.name: FormatOptions_Get_0000
 * @tc.desc: Test function of Get/Set interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatOptionsTest, FormatOptions_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatOptionsTest-begin FormatOptions_Get_0000";
    FormatOptions options;

    // Test default values
    EXPECT_EQ(options.GetFsType(), "hmfs");
    EXPECT_TRUE(options.GetQuickFormat());
    EXPECT_EQ(options.GetVolumeName(), "");

    // Test SetFsType and GetFsType
    std::string fsType = "ext4";
    options.SetFsType(fsType);
    EXPECT_EQ(options.GetFsType(), fsType);

    // Test SetQuickFormat and GetQuickFormat
    options.SetQuickFormat(false);
    EXPECT_FALSE(options.GetQuickFormat());

    options.SetQuickFormat(true);
    EXPECT_TRUE(options.GetQuickFormat());

    // Test SetVolumeName and GetVolumeName
    std::string volName = "test_volume";
    options.SetVolumeName(volName);
    EXPECT_EQ(options.GetVolumeName(), volName);

    GTEST_LOG_(INFO) << "FormatOptionsTest-end FormatOptions_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatOptions_SetFsType_0000
 * @tc.name: FormatOptions_SetFsType_0000
 * @tc.desc: Test SetFsType with different file system types.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatOptionsTest, FormatOptions_SetFsType_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatOptionsTest-begin FormatOptions_SetFsType_0000";
    FormatOptions options;

    // Test with ext4
    options.SetFsType("ext4");
    EXPECT_EQ(options.GetFsType(), "ext4");

    // Test with vfat
    options.SetFsType("vfat");
    EXPECT_EQ(options.GetFsType(), "vfat");

    // Test with exfat
    options.SetFsType("exfat");
    EXPECT_EQ(options.GetFsType(), "exfat");

    // Test with ntfs
    options.SetFsType("ntfs");
    EXPECT_EQ(options.GetFsType(), "ntfs");

    // Test with f2fs
    options.SetFsType("f2fs");
    EXPECT_EQ(options.GetFsType(), "f2fs");

    // Test with empty string
    options.SetFsType("");
    EXPECT_EQ(options.GetFsType(), "");

    GTEST_LOG_(INFO) << "FormatOptionsTest-end FormatOptions_SetFsType_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatOptions_Marshalling_0000
 * @tc.name: FormatOptions_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatOptionsTest, FormatOptions_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatOptionsTest-begin FormatOptions_Marshalling_0000";
    FormatOptions options;
    options.SetFsType("ext4");
    options.SetQuickFormat(false);
    options.SetVolumeName("test_vol");

    Parcel parcel;
    bool ret = options.Marshalling(parcel);
    EXPECT_TRUE(ret);
    EXPECT_EQ(parcel.ReadString(), "ext4");
    EXPECT_EQ(parcel.ReadBool(), false);
    EXPECT_EQ(parcel.ReadString(), "test_vol");

    GTEST_LOG_(INFO) << "FormatOptionsTest-end FormatOptions_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatOptions_Unmarshalling_0000
 * @tc.name: FormatOptions_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatOptionsTest, FormatOptions_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatOptionsTest-begin FormatOptions_Unmarshalling_0000";
    Parcel parcel;
    std::string fsType = "vfat";
    bool quickFormat = true;
    std::string volName = "my_volume";

    parcel.WriteString(fsType);
    parcel.WriteBool(quickFormat);
    parcel.WriteString(volName);

    FormatOptions* result = FormatOptions::Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetFsType(), fsType);
    EXPECT_EQ(result->GetQuickFormat(), quickFormat);
    EXPECT_EQ(result->GetVolumeName(), volName);
    delete result;

    GTEST_LOG_(INFO) << "FormatOptionsTest-end FormatOptions_Unmarshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_FormatOptions_RoundTrip_0000
 * @tc.name: FormatOptions_RoundTrip_0000
 * @tc.desc: Test Marshalling and Unmarshalling round trip.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(FormatOptionsTest, FormatOptions_RoundTrip_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FormatOptionsTest-begin FormatOptions_RoundTrip_0000";
    FormatOptions original;
    original.SetFsType("exfat");
    original.SetQuickFormat(false);
    original.SetVolumeName("round_trip_test");

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    FormatOptions* restored = FormatOptions::Unmarshalling(parcel);
    ASSERT_TRUE(restored != nullptr);

    EXPECT_EQ(restored->GetFsType(), original.GetFsType());
    EXPECT_EQ(restored->GetQuickFormat(), original.GetQuickFormat());
    EXPECT_EQ(restored->GetVolumeName(), original.GetVolumeName());
    delete restored;

    GTEST_LOG_(INFO) << "FormatOptionsTest-end FormatOptions_RoundTrip_0000";
}
}
