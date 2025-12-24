/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "volume_external.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class VolumeExternalTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Volume_external_Get_0000
 * @tc.name: Volume_external_Get_0000
 * @tc.desc: Test function of Get interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeExternalTest, Volume_external_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeExternalTest-begin Volume_external_Get_0000";
    int32_t flags = 1;
    int32_t fsType = 1;
    std::string fsUuid = "100";
    std::string path = "/";
    std::string description = "";
    VolumeExternal volumeexternal;
    volumeexternal.SetFsType(fsType);
    volumeexternal.SetFsUuid(fsUuid);
    volumeexternal.SetPath(path);
    volumeexternal.SetDescription(description);
    volumeexternal.SetFlags(flags);
    auto result1 = volumeexternal.GetFsType();
    EXPECT_EQ(result1, fsType);
    auto result2 = volumeexternal.GetUuid();
    EXPECT_EQ(result2, fsUuid);
    auto result3 = volumeexternal.GetPath();
    EXPECT_EQ(result3, path);
    auto result4 = volumeexternal.GetDescription();
    EXPECT_EQ(result4, description);

    auto result5 = volumeexternal.GetFsTypeString();
    EXPECT_EQ(result5, FS_TYPE_MAP[fsType]);

    volumeexternal.SetFsType(FsType::UNDEFINED);
    auto result6 = volumeexternal.GetFsTypeString();
    EXPECT_EQ(result6, "undefined");
    auto result7 = volumeexternal.GetFlags();
    EXPECT_EQ(result7, flags);
    volumeexternal.Reset();
    GTEST_LOG_(INFO) << "VolumeExternalTest-end Volume_external_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_external_Marshalling_0000
 * @tc.name: Volume_external_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeExternalTest, Volume_external_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeExternalTest-begin Volume_external_Marshalling_0000";
    std::string id = "200";
    int type = 2;
    std::string diskId = "200";
    int32_t state = UNMOUNTED;
    VolumeCore volumecore(id, type, diskId, state);
    VolumeExternal volumeexternal;
    int32_t flags = 1;
    int32_t fsType = 1;
    std::string fsUuid = "200";
    std::string path = "/";
    std::string description = "";
    Parcel parcel;
    volumeexternal.SetFlags(flags);
    volumeexternal.SetFsType(fsType);
    volumeexternal.SetFsUuid(fsUuid);
    volumeexternal.SetPath(path);
    volumeexternal.SetDescription(description);
    auto result = volumeexternal.Marshalling(parcel);
    GTEST_LOG_(INFO) << parcel.ReadInt32();
    GTEST_LOG_(INFO) << parcel.ReadInt32();
    GTEST_LOG_(INFO) << parcel.ReadString();
    GTEST_LOG_(INFO) << parcel.ReadString();
    GTEST_LOG_(INFO) << parcel.ReadString();
    GTEST_LOG_(INFO) << parcel.ReadBool();
    EXPECT_EQ(parcel.ReadInt32(), flags);
    EXPECT_EQ(parcel.ReadInt32(), fsType);
    EXPECT_EQ(parcel.ReadString(), fsUuid);
    EXPECT_EQ(parcel.ReadString(), path);
    EXPECT_EQ(parcel.ReadString(), description);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "VolumeExternalTest-end Volume_external_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_external_Unmarshalling_0000
 * @tc.name: Volume_external_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeExternalTest, Volume_external_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeExternalTest-begin Volume_external_Unmarshalling_0000";
    std::string id = "300";
    int32_t flags = 1;
    int type = 2;
    std::string diskId = "300";
    int32_t state = UNMOUNTED;
    std::string fsTypeStr = "f2fs";
    bool errorFlag = false;
    int32_t fsType = 1;
    std::string fsUuid = "300";
    std::string path = "/";
    std::string description = "";
    Parcel parcel;
    parcel.WriteString(id);
    parcel.WriteInt32(type);
    parcel.WriteString(diskId);
    parcel.WriteInt32(state);
    parcel.WriteBool(errorFlag);
    parcel.WriteString(fsTypeStr);
    parcel.WriteInt32(flags);
    parcel.WriteInt32(fsType);
    parcel.WriteString(fsUuid);
    parcel.WriteString(path);
    parcel.WriteString(description);
    VolumeExternal volumeexternal;
    volumeexternal.SetFlags(2);
    volumeexternal.SetFsType(2);
    volumeexternal.SetFsUuid("400");
    volumeexternal.SetPath("/");
    volumeexternal.SetDescription("");
    auto result = volumeexternal.Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetFlags(), flags);
    EXPECT_EQ(result->GetFsType(), fsType);
    EXPECT_EQ(result->GetUuid(), fsUuid);
    EXPECT_EQ(result->GetPath(), path);
    EXPECT_EQ(result->GetDescription(), description);
    EXPECT_EQ(result->GetFsTypeString(), FS_TYPE_MAP[fsType]);
    EXPECT_EQ(result->VolumeCore::GetFsType(), fsTypeStr);
    GTEST_LOG_(INFO) << "VolumeExternalTest-end Volume_external_Unmarshalling_0000";
}
}
