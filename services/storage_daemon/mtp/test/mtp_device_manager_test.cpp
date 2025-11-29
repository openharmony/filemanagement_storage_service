/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#include "mtp/mtp_device_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/disk_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
class MtpDeviceManagerTest : public testing::Test {
protected:
    MtpDeviceInfo deviceInfo;

public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp()
    {
        deviceInfo.path = "/test/path";
        deviceInfo.id = 1;
        deviceInfo.vendor = "TestVendor";
        deviceInfo.uuid = "TestUUID";
    }

    void TearDown() {}
};

/**
 * @tc.name  : MountDeviceTest_001
 * @tc.number: MountDeviceTest_001
 * @tc.desc  : Test when device is mounting
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_001 start";

    MtpDeviceManager& manager = MtpDeviceManager::GetInstance();
    manager.isMounting = true;
    int32_t result = manager.MountDevice(deviceInfo);
    EXPECT_EQ(result, E_MTP_IS_MOUNTING);

    GTEST_LOG_(INFO) << "MountDeviceTest_001 end";
}

/**
 * @tc.name  : MountDeviceTest_002
 * @tc.number: MountDeviceTest_002
 * @tc.desc  : Test when device is not mounting
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_002 start";

    MtpDeviceManager& manager = MtpDeviceManager::GetInstance();
    manager.isMounting = false;
    int32_t result = manager.MountDevice(deviceInfo);
    EXPECT_EQ(result, E_MTP_PREPARE_DIR_ERR);

    GTEST_LOG_(INFO) << "MountDeviceTest_002 end";
}

/**
 * @tc.name  : MountDeviceTest_003
 * @tc.number: MountDeviceTest_003
 * @tc.desc  : Test when device is not mounting
 */
HWTEST_F(MtpDeviceManagerTest, MountDeviceTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountDeviceTest_003 start";

    MtpDeviceManager& manager = MtpDeviceManager::GetInstance();
    manager.isMounting = false;
    deviceInfo.path = "/mnt/data/external";
    int32_t result = manager.MountDevice(deviceInfo);
    EXPECT_EQ(result, E_WEXITSTATUS);

    GTEST_LOG_(INFO) << "MountDeviceTest_003 end";
}

/**
 * @tc.name  : UmountDeviceTest_001
 * @tc.number: UmountDeviceTest_001
 * @tc.desc  : Test when umount and remove both succeed
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_001 start";

    MtpDeviceManager& manager = MtpDeviceManager::GetInstance();
    deviceInfo.path = "/test/path";
    EXPECT_EQ(manager.UmountDevice(deviceInfo, false, false), E_MTP_UMOUNT_FAILED);

    GTEST_LOG_(INFO) << "UmountDeviceTest_001 end";
}

/**
 * @tc.name  : UmountDeviceTest_002
 * @tc.number: UmountDeviceTest_002
 * @tc.desc  : Test when umount and remove both succeed
 */
HWTEST_F(MtpDeviceManagerTest, UmountDeviceTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDeviceTest_002 start";

    MtpDeviceManager& manager = MtpDeviceManager::GetInstance();
    deviceInfo.path = "/test/path";
    EXPECT_EQ(manager.UmountDevice(deviceInfo, true, false), E_MTP_UMOUNT_FAILED);

    GTEST_LOG_(INFO) << "UmountDeviceTest_002 end";
}
} // STORAGE_DAEMON
} // OHOS
