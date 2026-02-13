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
#include <sys/mount.h>
#include "external_volume_info.h"
#include "process.h"
#include "external_volume_info_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class ProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Service_ProcessTest_GetPath_001
 * @tc.desc: Verify the GetPath function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ProcessTest, Storage_Service_ProcessTest_GetXXX_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_GetXXX_001 start";

    std::string volId = "vol-1-1";
    std::string diskId = "disk-1-1";
    dev_t device = MKDEV(1, 1); // 1 is major device number, 1 is minor device number
    uint32_t mountFlags = 0;
    ExternalVolumeInfoMock mock;
    EXPECT_CALL(mock, DoCreate(testing::_)).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoCheck()).Times(1).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DoMount(testing::_)).Times(1).WillOnce(testing::Return(E_OK));

    auto ret = mock.Create(volId, diskId, device);
    EXPECT_TRUE(ret == E_OK);

    ret = mock.Check();
    EXPECT_TRUE(ret == E_OK);

    ret = mock.Mount(mountFlags);
    EXPECT_TRUE(ret == E_OK);

    auto mountPath = mock.GetMountPath();
    Process ps(mountPath);
    EXPECT_TRUE(ps.GetPath() == mountPath);

    GTEST_LOG_(INFO) << "Storage_Service_ProcessTest_GetXXX_001 end";
}
} // STORAGE_DAEMON
} // OHOS
