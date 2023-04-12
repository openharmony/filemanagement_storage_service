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

#include <gtest/gtest.h>
#include <linux/kdev_t.h>

#include "external_volume_info_mock.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class ExternalVolumeInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoCreate_001
 * @tc.desc: Verify the DoCreate function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoCreate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoCreate_001 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 300);
    std::string diskId = "disk-156-300";
    std::string volId = "vol-156-301";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    ret = vol.Destroy();

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoCreate_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoDestroy_001
 * @tc.desc: Verify the DoDestroy function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoDestroy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoDestroy_001 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 400);
    std::string diskId = "disk-156-400";
    std::string volId = "vol-156-401";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    ret = vol.Destroy();
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoDestroy_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoMount_001
 * @tc.desc: Verify the DoMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoMount_001 start";

    ExternalVolumeInfoMock mock;
    uint32_t mountFlags = 0;

    EXPECT_CALL(mock, DoMount(testing::_)).Times(1).WillOnce(testing::Return(E_MOUNT));
    auto ret = mock.DoMount(mountFlags);
    EXPECT_TRUE(ret == E_MOUNT);

    EXPECT_CALL(mock, DoMount(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.DoMount(mountFlags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoMount_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoUMount_001
 * @tc.desc: Verify the DoUMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoUMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoUMount_001 start";

    ExternalVolumeInfoMock mock;
    bool force = true;

    EXPECT_CALL(mock, DoUMount(testing::_)).Times(1).WillOnce(testing::Return(E_UMOUNT));
    auto ret = mock.DoUMount(force);
    EXPECT_TRUE(ret == E_UMOUNT);

    EXPECT_CALL(mock, DoUMount(testing::_)).Times(1).WillOnce(testing::Return(E_SYS_CALL));
    ret = mock.DoUMount(force);
    EXPECT_TRUE(ret == E_SYS_CALL);

    EXPECT_CALL(mock, DoUMount(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.DoUMount(force);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoUMount_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoCheck_001
 * @tc.desc: Verify the DoCheck function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoCheck_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoCheck_001 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 600);
    std::string diskId = "disk-156-600";
    std::string volId = "vol-156-601";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    ret = vol.Check();
    EXPECT_EQ(ret, E_ERR);
    ret = vol.Destroy();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoCheck_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoFormat_001
 * @tc.desc: Verify the DoFormat function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoFormat_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_001 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 700);
    std::string diskId = "disk-156-700";
    std::string volId = "vol-156-701";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    std::string flag = "exfat";
    ret = vol.Format(flag);
    EXPECT_EQ(ret, E_OK);
    ret = vol.Destroy();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoFormat_002
 * @tc.desc: Verify the DoFormat function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoFormat_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_002 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 701);
    std::string diskId = "disk-156-701";
    std::string volId = "vol-156-702";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    std::string flag = "vfat";
    ret = vol.Format(flag);
    EXPECT_EQ(ret, E_OK);
    ret = vol.Destroy();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_002 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoFormat_003
 * @tc.desc: Verify the DoFormat function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoFormat_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_003 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 702);
    std::string diskId = "disk-156-702";
    std::string volId = "vol-156-703";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    std::string flag = "ntfs";
    ret = vol.Format(flag);
    EXPECT_EQ(ret, E_NOT_SUPPORT);
    ret = vol.Destroy();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_003 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_DoSetVolDesc_001
 * @tc.desc: Verify the DoSetVolDesc function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_DoSetVolDesc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoSetVolDesc_001 start";

    ExternalVolumeInfo vol;
    dev_t device = MKDEV(156, 800);
    std::string diskId = "disk-156-800";
    std::string volId = "vol-156-801";
    int32_t ret = vol.Create(volId, diskId, device);
    EXPECT_EQ(ret, E_OK);
    std::string des = "label1";
    ret = vol.SetVolumeDescription(des);
    EXPECT_EQ(ret, E_NOT_SUPPORT);
    ret = vol.Destroy();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoSetVolDesc_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_GetFsType_001
 * @tc.desc: Verify the GetFsType function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_GetFsType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetFsType_001 start";

    ExternalVolumeInfo *evi = new ExternalVolumeInfo();
    auto ret = evi->GetFsType();
    GTEST_LOG_(INFO) << ret;
    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetFsType_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_GetFsUuid_001
 * @tc.desc: Verify the GetFsUuid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_GetFsUuid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetFsUuid_001 start";

    ExternalVolumeInfo *evi = new ExternalVolumeInfo();
    std::string ret = evi->GetFsUuid();
    GTEST_LOG_(INFO) << ret;

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetFsUuid_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_GetFsLabel_001
 * @tc.desc: Verify the GetFsLabel function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_GetFsLabel_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetFsLabel_001 start";

    ExternalVolumeInfo *evi = new ExternalVolumeInfo();
    std::string ret = evi->GetFsLabel();
    GTEST_LOG_(INFO) << ret;

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetFsLabel_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_GetMountPath_001
 * @tc.desc: Verify the GetMountPath function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(ExternalVolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_GetMountPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetMountPath_001 start";

    ExternalVolumeInfo *evi = new ExternalVolumeInfo();
    std::string ret = evi->GetMountPath();
    GTEST_LOG_(INFO) << ret;

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetMountPath_001 end";
}
} // STORAGE_DAEMON
} // OHOS
