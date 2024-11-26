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
#include "mtp/mtp_device_monitor.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/disk_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
class MtpDeviceMonitorTest : public testing::Test {

public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};

    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.name: HasMounted_001
 * @tc.desc: Verify the Decode function of Devpath and Syspath.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, HasMounted_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasMounted_001 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    MtpDeviceInfo device;
    device.id = 1;
    int32_t result = monitor->HasMounted(device);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "HasMounted_001 end";
}

/**
 * @tc.name: HasMounted_002
 * @tc.desc: Verify the Decode function of Devpath and Syspath.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, HasMounted_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasMounted_002 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    MtpDeviceInfo device;
    device.id = "1";
    monitor->lastestMtpDevList_.push_back(device);
    int32_t result = monitor->HasMounted(device);
    EXPECT_TRUE(result);
    monitor->lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "HasMounted_002 end";
}

/**
 * @tc.name: MountTest_001
 * @tc.desc: Test when the device does not exist, Mount method should return E_NON_EXIST.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountTest_001 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    monitor->lastestMtpDevList_.push_back(device);
    int32_t result = monitor->Mount(id);
    EXPECT_EQ(result, E_MTP_PREPARE_DIR_ERR);
    monitor->lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "MountTest_001 end";
}

/**
 * @tc.name: MountTest_002
 * @tc.desc: Test when MountDevice fails, Mount method should return an error code.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountTest_002 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    int32_t result = monitor->Mount(id);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "MountTest_002 end";
}

/**
 * @tc.name: UmountTest_001
 * @tc.desc: Test Umount method returns error when UmountDevice fails.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, UmountTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountTest_001 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    monitor->lastestMtpDevList_.push_back(device);
    int32_t result = monitor->Umount(id);
    EXPECT_EQ(result, E_MTP_UMOUNT_FAILED);
    monitor->lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "UmountTest_001 end";
}

/**
 * @tc.name: UmountTest_002
 * @tc.desc: Test Umount method returns error when UmountDevice fails.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, UmountTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountTest_002 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    int32_t result = monitor->Umount(id);
    EXPECT_EQ(result, E_NON_EXIST);

    GTEST_LOG_(INFO) << "UmountTest_002 end";
}

/**
 * @tc.name: MountMtpDeviceTest_001
 * @tc.desc: Test when device is already mounted, MountMtpDevice should not mount it again.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountMtpDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountMtpDeviceTest_001 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device;
    device.id = "123";
    devices.push_back(device);
    monitor->lastestMtpDevList_.push_back(device);
    monitor->MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor->lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 1);
    monitor->lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "MountMtpDeviceTest_001 end";
}

/**
 * @tc.name: MountMtpDeviceTest_002
 * @tc.desc: Test when device is ejected, MountMtpDevice should not mount it.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountMtpDeviceTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountMtpDeviceTest_002 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device;
    device.id = "123";
    devices.push_back(device);
    monitor->hasEjectedDevices_.push_back(device);
    monitor->MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor->lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 0);
    monitor->hasEjectedDevices_.clear();

    GTEST_LOG_(INFO) << "MountMtpDeviceTest_002 end";
}

/**
 * @tc.name: MountMtpDeviceTest_003
 * @tc.desc:  Test when device is invalid, MountMtpDevice should not mount it.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountMtpDeviceTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountMtpDeviceTest_003 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device;
    device.id = "123";
    devices.push_back(device);
    monitor->invalidMtpDevices_.push_back(device);
    monitor->MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor->lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 0);
    monitor->invalidMtpDevices_.clear();

    GTEST_LOG_(INFO) << "MountMtpDeviceTest_003 end";
}

/**
 * @tc.name: MountMtpDeviceTest_004
 * @tc.desc: Test when device is valid, MountMtpDevice should mount it.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountMtpDeviceTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountMtpDeviceTest_004 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device1;
    device1.id = "123";
    MtpDeviceInfo device2;
    device2.id = "456";
    devices.push_back(device1);
    devices.push_back(device2);
    monitor->lastestMtpDevList_.push_back(device1);
    monitor->MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor->lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 1);

    GTEST_LOG_(INFO) << "MountMtpDeviceTest_004 end";
}

/**
 * @tc.name: IsNeedDisableMtp_001
 * @tc.desc: Test when mtpEnable is "false", IsNeedDisableMtp should return false
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, IsNeedDisableMtp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsNeedDisableMtp_001 start";

    auto monitor = DelayedSingleton<MtpDeviceMonitor>::GetInstance();
    bool result = monitor->IsNeedDisableMtp();
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "IsNeedDisableMtp_001 end";
}
} // STORAGE_DAEMON
} // OHOS
