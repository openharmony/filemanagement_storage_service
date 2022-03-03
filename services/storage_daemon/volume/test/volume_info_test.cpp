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

#include "help_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "volume_info_mock.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace StorageTest;
class VolumeInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Create_001
 * @tc.desc: Verify the Create function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Create_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Create_001 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-1";
    std::string diskId = "101";
    dev_t device = MKDEV(156, 301);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);

    volId = "vol-1-2";
    diskId = "102";
    device = MKDEV(156, 302);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_ERR));
    ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Create_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Destroy_001
 * @tc.desc: Verify the Destroy function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Destroy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Destroy_001 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-3";
    std::string diskId = "103";
    dev_t device = MKDEV(156, 303);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);

    EXPECT_CALL(mock, DoDestroy()).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.Destroy();
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Destroy_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Destroy_002
 * @tc.desc: Verify the Destroy function when DoDestroy() return error.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Destroy_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Destroy_002 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-4";
    std::string diskId = "104";
    dev_t device = MKDEV(156, 304);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);

    EXPECT_CALL(mock, DoDestroy()).Times(1).WillOnce(testing::Return(E_ERR));
    ret = mock.Destroy();
    EXPECT_TRUE(ret == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Destroy_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Mount_001
 * @tc.desc: Verify the Mount function when mount state is incorrect.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_001 start";

    VolumeInfoMock mock;
    uint32_t mountFlags = 0;

    auto ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_VOL_STATE);
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_001 end";
}

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Mount_002
 * @tc.desc: Verify the Mount function when mount dir exists.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Mount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_002 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-5";
    std::string diskId = "105";
    dev_t device = MKDEV(156, 305);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    std::string mountPath("/mnt/" + volId);
    bool bRet = StorageTestUtils::MkDir(mountPath, S_IRWXU | S_IRWXG | S_IRWXO);
    EXPECT_TRUE(bRet);

    uint32_t mountFlags = 0;
    ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_MOUNT);

    StorageTestUtils::RmDirRecurse(mountPath);
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_002 end";
}

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Mount_003
 * @tc.desc: Verify the Mount function when DoMount return error.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Mount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_003 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-6";
    std::string diskId = "106";
    dev_t device = MKDEV(156, 306);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_MOUNT));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    uint32_t mountFlags = 0;
    ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_MOUNT);

    StorageTestUtils::RmDirRecurse("/mnt/" + volId);
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_003 end";
}

/**
 * @tc.name: Storage_Service_VolumeInfoTest_Mount_004
 * @tc.desc: Verify the Mount function when args are normal.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_VolumeInfoTest_Mount_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_004 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-7";
    std::string diskId = "107";
    dev_t device = MKDEV(156, 307);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    uint32_t mountFlags = 0;
    ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_OK);

    StorageTestUtils::RmDirRecurse("/mnt/" + volId);
    GTEST_LOG_(INFO) << "Storage_Service_VolumeInfoTest_Mount_004 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_UMount_001
 * @tc.desc: Verify the UMount function when mount state is incorrect.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_UMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_UMount_001 start";

    VolumeInfoMock mock;
    EXPECT_CALL(mock, DoDestroy()).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Destroy();
    EXPECT_TRUE(ret == E_OK);

    bool force = true;
    ret = mock.UMount(force);
    EXPECT_TRUE(ret == E_VOL_STATE);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_UMount_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_UMount_002
 * @tc.desc: Verify the UMount function when args are normal.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_UMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_UMount_002 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-8";
    std::string diskId = "108";
    dev_t device = MKDEV(156, 308);
    uint32_t mountFlags = 0;
    bool force = true;

    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoUMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_OK);

    ret = mock.UMount(force);
    EXPECT_TRUE(ret == E_OK);

    StorageTestUtils::RmDirRecurse("/mnt/" + volId);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_UMount_002 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_Check_001
 * @tc.desc: Verify the Check function when mount state is incorrect.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_Check_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Check_001 start";

    VolumeInfoMock mock;
    EXPECT_CALL(mock, DoDestroy()).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Destroy();
    EXPECT_TRUE(ret == E_OK);

    ret = mock.Check();
    EXPECT_TRUE(ret == E_VOL_STATE);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Check_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_Check_002
 * @tc.desc: Verify the Check function when args are normal.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_Check_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Check_002 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-9";
    std::string diskId = "109";
    dev_t device = MKDEV(156, 309);

    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Check_002 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_Format_001
 * @tc.desc: Verify the Format function when mount state is incorrect.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_Format_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Format_001 start";
    LOGI("Storage_Service_ExternalVolumeInfoTest_Format_001 start");
    VolumeInfoMock mock;
    std::string volId = "vol-1-10";
    std::string diskId = "110";
    dev_t device = MKDEV(156, 310);

    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    std::string type = "ext2";
    ret = mock.Format(type);
    EXPECT_TRUE(ret == E_VOL_STATE);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Format_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_Format_002
 * @tc.desc: Verify the Format function when args are normal.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_Format_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Format_002 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-11";
    std::string diskId = "111";
    dev_t device = MKDEV(156, 311);
    std::string type = "ext2";

    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoFormat(testing::_)).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);
    ret = mock.Format(type);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_Format_002 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest_GetXXX_001
 * @tc.desc: Verify the GetVolumeId, GetVolumeType, GetDiskId, GetState, GetMountPath function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(VolumeInfoTest, Storage_Service_ExternalVolumeInfoTest_GetXXX_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetXXX_001 start";

    VolumeInfoMock mock;
    std::string volId = "vol-1-12";
    std::string diskId = "112";
    dev_t device = MKDEV(156, 312);

    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);

    EXPECT_TRUE(mock.GetVolumeId() == volId);
    EXPECT_TRUE(mock.GetVolumeType() == EXTERNAL);
    EXPECT_TRUE(mock.GetDiskId() == diskId);
    EXPECT_TRUE(mock.GetState() == UNMOUNTED);
    EXPECT_TRUE(mock.GetMountPath() == "/mnt/" + volId);

    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_GetXXX_001 end";
}
} // STORAGE_DAEMON
} // OHOS