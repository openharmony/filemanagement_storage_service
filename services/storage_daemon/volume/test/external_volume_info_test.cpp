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

#include <gtest/gtest.h>

#include <linux/kdev_t.h>
#include <sys/types.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "external_volume_info_mock.h"

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

    ExternalVolumeInfoMock mock;
    dev_t device = MKDEV(156, 300);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.DoCreate(device);
    EXPECT_TRUE(ret == E_OK);

    device = MKDEV(156, 301);
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_ERR));
    ret = mock.DoCreate(device);
    EXPECT_TRUE(ret == E_ERR);

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

    ExternalVolumeInfoMock mock;
    EXPECT_CALL(mock, DoDestroy()).Times(1).WillOnce(testing::Return(E_OK));
    auto ret = mock.DoDestroy();
    EXPECT_TRUE(ret == E_OK);

    EXPECT_CALL(mock, DoDestroy()).Times(1).WillOnce(testing::Return(E_ERR));
    ret = mock.DoDestroy();
    EXPECT_TRUE(ret == E_ERR);

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
    std::string mountPath = "/data";
    uint32_t mountFlags = 0;

    EXPECT_CALL(mock, DoMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_MOUNT));
    auto ret = mock.DoMount(mountPath, mountFlags);
    EXPECT_TRUE(ret == E_MOUNT);

    EXPECT_CALL(mock, DoMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.DoMount(mountPath, mountFlags);
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
    std::string mountPath = "/data";
    bool force = true;

    EXPECT_CALL(mock, DoUMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_UMOUNT));
    auto ret = mock.DoUMount(mountPath, force);
    EXPECT_TRUE(ret == E_UMOUNT);

    EXPECT_CALL(mock, DoUMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_SYS_CALL));
    ret = mock.DoUMount(mountPath, force);
    EXPECT_TRUE(ret == E_SYS_CALL);

    EXPECT_CALL(mock, DoUMount(testing::_, testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.DoUMount(mountPath, force);
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

    ExternalVolumeInfoMock mock;

    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_ERR));
    auto ret = mock.DoCheck();
    EXPECT_TRUE(ret == E_ERR);

    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_NOT_SUPPORT));
    ret = mock.DoCheck();
    EXPECT_TRUE(ret == E_NOT_SUPPORT);

    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.DoCheck();
    EXPECT_TRUE(ret == E_OK);
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

    ExternalVolumeInfoMock mock;
    std::string type = "ext2";

    EXPECT_CALL(mock, DoFormat(testing::_)).Times(1).WillOnce(testing::Return(E_NOT_SUPPORT));
    auto ret = mock.DoFormat(type);
    EXPECT_TRUE(ret == E_NOT_SUPPORT);

    EXPECT_CALL(mock, DoFormat(testing::_)).Times(1).WillOnce(testing::Return(E_ERR));
    ret = mock.DoFormat(type);
    EXPECT_TRUE(ret == E_ERR);

    EXPECT_CALL(mock, DoFormat(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    ret = mock.DoFormat(type);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest_DoFormat_001 end";
}
} // STORAGE_DAEMON
} // OHOS