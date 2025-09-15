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

#include "volume_core.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class VolumeCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Volume_core_Get_0000
 * @tc.name: Volume_core_Get_0000
 * @tc.desc: Test function of Get interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeCoreTest, Volume_core_Get_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeCoreTest-begin Volume_core_Get_0000";
    std::string id = "100";
    int type = 2;
    std::string diskId = "100";
    int32_t state = UNMOUNTED;
    VolumeCore volumecore(id, type, diskId, state);
    auto result1 = volumecore.GetId();
    EXPECT_EQ(result1, id);
    auto result2 = volumecore.GetType();
    EXPECT_EQ(result2, type);
    auto result3 = volumecore.GetDiskId();
    EXPECT_EQ(result3, diskId);
    auto result4 = volumecore.GetState();
    EXPECT_EQ(result4, state);
    volumecore.SetState(state);
    GTEST_LOG_(INFO) << "VolumeCoreTest-end Volume_core_Get_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_core_Marshalling_0000
 * @tc.name: Volume_core_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeCoreTest, Volume_core_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeCoreTest-begin Volume_core_Marshalling_0000";
    std::string id = "200";
    int type = 2;
    std::string diskId = "200";
    int32_t state = UNMOUNTED;
    VolumeCore volumecore(id, type, diskId, state);
    Parcel parcel;
    volumecore.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadString(), id);
    EXPECT_EQ(parcel.ReadInt32(), type);
    EXPECT_EQ(parcel.ReadString(), diskId);
    EXPECT_EQ(parcel.ReadInt32(), state);
    EXPECT_EQ(parcel.ReadBool(), 0);
    GTEST_LOG_(INFO) << "VolumeCoreTest-end Volume_core_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_core_Unmarshalling_0000
 * @tc.name: Volume_core_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeCoreTest, Volume_core_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeCoreTest-begin Volume_core_Unmarshalling_0000";
    std::string id = "300";
    int type = 2;
    std::string diskId = "300";
    int32_t state = CHECKING;
    bool errorFlag = false;
    VolumeCore volumecore("400", 1, "400", 0);
    Parcel parcel;
    parcel.WriteString(id);
    parcel.WriteInt32(type);
    parcel.WriteString(diskId);
    parcel.WriteInt32(state);
    parcel.WriteBool(errorFlag);
    auto result = volumecore.Unmarshalling(parcel);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->GetId(), id);
    EXPECT_EQ(result->GetType(), type);
    EXPECT_EQ(result->GetDiskId(), diskId);
    EXPECT_EQ(result->GetState(), state);
    GTEST_LOG_(INFO) << "VolumeCoreTest-end Volume_core_Unmarshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_VolumeInfoStr_Marshalling_0000
 * @tc.name: VolumeInfoStr_Marshalling_0000
 * @tc.desc: Test function of Marshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeCoreTest, VolumeInfoStr_Marshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeCoreTest-begin VolumeInfoStr_Marshalling_0000";
    std::string volumeId = "200";
    std::string fsTypeStr = "exfat";
    std::string fsUuid = "200";
    std::string path = "/data/200";
    std::string description = "Volume 200";
    bool isDamaged = false;
    VolumeInfoStr volumeInfo(volumeId, fsTypeStr, fsUuid, path, description, isDamaged);
    Parcel parcel;
    volumeInfo.Marshalling(parcel);
    EXPECT_EQ(parcel.ReadString(), volumeId);
    EXPECT_EQ(parcel.ReadString(), fsTypeStr);
    EXPECT_EQ(parcel.ReadString(), fsUuid);
    EXPECT_EQ(parcel.ReadString(), path);
    EXPECT_EQ(parcel.ReadString(), description);
    EXPECT_EQ(parcel.ReadBool(), isDamaged);
    GTEST_LOG_(INFO) << "VolumeCoreTest-end VolumeInfoStr_Marshalling_0000";
}

/**
 * @tc.number: SUB_STORAGE_VolumeInfoStr_Unmarshalling_0000
 * @tc.name: VolumeInfoStr_Unmarshalling_0000
 * @tc.desc: Test function of Unmarshalling interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeCoreTest, VolumeInfoStr_Unmarshalling_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeCoreTest-begin VolumeInfoStr_Unmarshalling_0000";
    std::string volumeId = "300";
    std::string fsTypeStr = "exfat";
    std::string fsUuid = "300";
    std::string path = "/data/300";
    std::string description = "Volume 300";
    bool isDamaged = false;
    VolumeInfoStr volumeInfoStr;
    Parcel parcel;
    parcel.WriteString(volumeId);
    parcel.WriteString(fsTypeStr);
    parcel.WriteString(fsUuid);
    parcel.WriteString(path);
    parcel.WriteString(description);
    parcel.WriteBool(isDamaged);
    auto result = volumeInfoStr.Unmarshalling(parcel);
    EXPECT_EQ(result->volumeId, volumeId);
    EXPECT_EQ(result->fsTypeStr, fsTypeStr);
    EXPECT_EQ(result->fsUuid, fsUuid);
    EXPECT_EQ(result->path, path);
    EXPECT_EQ(result->description, description);
    EXPECT_EQ(result->isDamaged, isDamaged);
    GTEST_LOG_(INFO) << "VolumeCoreTest-end VolumeInfoStr_Unmarshalling_0000";
}
}
