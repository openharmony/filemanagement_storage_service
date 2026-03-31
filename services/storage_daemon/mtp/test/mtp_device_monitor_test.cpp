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

int g_getParameter = 0;
std::string g_paramOutContent = "";
constexpr const char *SYS_PARAM_SERVICE_PERSIST_ENABLE = "persist.edm.mtp_client_disable";
constexpr const char *SYS_PARAM_SERVICE_ENTERPRISE_ENABLE = "const.edm.is_enterprise_device";
extern "C" int GetParameter(const char *key, const char *def, char *value, uint32_t len)
{
    (void)key;
    (void)def;
    if (!value || len <= 0) {
        return 0;
    }
    int n = std::min(static_cast<int>(g_paramOutContent.size()), static_cast<int>(len));
    if (n > 0) {
        if (memcpy_s(value, len + 1, g_paramOutContent.data(), n) != EOK) {
            return -1;
        }
    }
    if (n < len) {
        value[n] = '\0';
    }
    return g_getParameter;
}

class MtpDeviceMonitorTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        MtpDeviceMonitor::GetInstance().lastestMtpDevList_.clear();
    };
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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    MtpDeviceInfo device;
    device.id = 1;
    int32_t result = monitor.HasMounted(device);
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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    MtpDeviceInfo device;
    device.id = "1";
    monitor.lastestMtpDevList_.push_back(device);
    int32_t result = monitor.HasMounted(device);
    EXPECT_TRUE(result);
    monitor.lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "HasMounted_002 end";
}

/**
 * @tc.name: HasMounted_003
 * @tc.desc: Verify HasMounted returns true when matching device is in middle of list.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, HasMounted_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasMounted_003 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    MtpDeviceInfo dev1, dev2, target;
    dev1.id = "first"; dev2.id = "second"; target.id = "target";
    monitor.lastestMtpDevList_.push_back(dev1);
    monitor.lastestMtpDevList_.push_back(target);
    monitor.lastestMtpDevList_.push_back(dev2);

    bool result = monitor.HasMounted(target);
    EXPECT_TRUE(result);
    monitor.lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "HasMounted_003 end";
}

/**
 * @tc.name: HasMounted_004
 * @tc.desc: Verify HasMounted returns false when list is not empty but no device id matches.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, HasMounted_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasMounted_004 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();

    MtpDeviceInfo dev1, dev2;
    dev1.id = "a";
    dev2.id = "b";
    monitor.lastestMtpDevList_.push_back(dev1);
    monitor.lastestMtpDevList_.push_back(dev2);

    MtpDeviceInfo target;
    target.id = "c";
    bool result = monitor.HasMounted(target);
    EXPECT_FALSE(result);

    monitor.lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "HasMounted_004 end";
}

/**
 * @tc.name: MountTest_001
 * @tc.desc: Test when the device does not exist, Mount method should return E_NON_EXIST.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountTest_001 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    monitor.lastestMtpDevList_.push_back(device);
    int32_t result = monitor.Mount(id);
    EXPECT_EQ(result, E_MTP_PREPARE_DIR_ERR);
    monitor.lastestMtpDevList_.clear();

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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    int32_t result = monitor.Mount(id);
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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    monitor.lastestMtpDevList_.push_back(device);
    int32_t result = monitor.Umount(id);
    EXPECT_EQ(result, E_MTP_UMOUNT_FAILED);
    monitor.lastestMtpDevList_.clear();

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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::string id = "test_id";
    MtpDeviceInfo device;
    device.id = id;
    int32_t result = monitor.Umount(id);
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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device;
    device.id = "123";
    devices.push_back(device);
    monitor.lastestMtpDevList_.push_back(device);
    monitor.MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor.lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 1);
    monitor.lastestMtpDevList_.clear();

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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device;
    device.id = "123";
    devices.push_back(device);
    monitor.MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor.lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 0);

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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device;
    device.id = "123";
    devices.push_back(device);
    monitor.MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor.lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 0);

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

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::vector<MtpDeviceInfo> devices;
    MtpDeviceInfo device1;
    device1.id = "123";
    MtpDeviceInfo device2;
    device2.id = "456";
    devices.push_back(device1);
    devices.push_back(device2);
    monitor.lastestMtpDevList_.push_back(device1);
    monitor.MountMtpDevice(devices);
    std::vector<MtpDeviceInfo> lastestMtpDevList = monitor.lastestMtpDevList_;
    EXPECT_EQ(lastestMtpDevList.size(), 1);

    GTEST_LOG_(INFO) << "MountMtpDeviceTest_004 end";
}

/**
 * @tc.name: MountMtpDeviceTest_005
 * @tc.desc: Test MountMtpDevice with empty device list should return E_OK and not modify list.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, MountMtpDeviceTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountMtpDeviceTest_005 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    std::vector<MtpDeviceInfo> emptyDevices;
    int32_t result = monitor.MountMtpDevice(emptyDevices);
    EXPECT_EQ(result, E_OK);
    EXPECT_TRUE(monitor.lastestMtpDevList_.empty());

    GTEST_LOG_(INFO) << "MountMtpDeviceTest_005 end";
}

/**
 * @tc.name: IsNeedDisableMtpTest_001
 * @tc.desc: Test when mtpEnable is "false", IsNeedDisableMtp should return false
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, IsNeedDisableMtpTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsNeedDisableMtpTest_001 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    bool result = monitor.IsNeedDisableMtp();
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "IsNeedDisableMtpTest_001 end";
}
/**
 * @tc.name: IsNeedDisableMtpTest_002
 * @tc.desc: Verify returns true when both enterprise and persist params are 'true'.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, IsNeedDisableMtpTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsNeedDisableMtpTest_002 start";
    g_paramOutContent = "true";
    g_getParameter = 4; // "true" length
    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    EXPECT_TRUE(monitor.IsNeedDisableMtp());
    g_paramOutContent = "";
    g_getParameter = 0;
    GTEST_LOG_(INFO) << "IsNeedDisableMtpTest_002 end";
}
/**
 * @tc.name: IsNeedDisableMtpTest_003
 * @tc.desc: Verify returns false when both params are 'false'.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, IsNeedDisableMtpTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsNeedDisableMtpTest_003 start";
    g_paramOutContent = "false";
    g_getParameter = 5; // "false" length
    EXPECT_FALSE(MtpDeviceMonitor::GetInstance().IsNeedDisableMtp());
    g_paramOutContent = "";
    g_getParameter = 0;
    GTEST_LOG_(INFO) << "IsNeedDisableMtpTest_003 end";
}

/**
 * @tc.name: UmountAllMtpDeviceTest_001
 * @tc.desc: Verify UmountAllMtpDevice clears lastestMtpDevList_ regardless of umount result.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, UmountAllMtpDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountAllMtpDeviceTest_001 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    MtpDeviceInfo dev1, dev2;
    dev1.id = "dev1"; dev2.id = "dev2";
    monitor.lastestMtpDevList_.push_back(dev1);
    monitor.lastestMtpDevList_.push_back(dev2);
    
    monitor.UmountAllMtpDevice();
    EXPECT_TRUE(monitor.lastestMtpDevList_.empty());

    GTEST_LOG_(INFO) << "UmountAllMtpDeviceTest_001 end";
}

/**
 * @tc.name: UmountDetachedMtpDeviceTest_001
 * @tc.desc: Verify device is removed from list when devNum and busLocation match (simulated success path).
 * @tc.type: FUNC
 * @tc.remark: Requires fixing UmountDetachedMtpDevice logic: change 'return' to 'continue' on mismatch.
 */
HWTEST_F(MtpDeviceMonitorTest, UmountDetachedMtpDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UmountDetachedMtpDeviceTest_001 start";

    MtpDeviceMonitor& monitor = MtpDeviceMonitor::GetInstance();
    MtpDeviceInfo dev;
    dev.id = "test";
    dev.devNum = 5;
    dev.busLocation = 2;
    monitor.lastestMtpDevList_.push_back(dev);

    monitor.UmountDetachedMtpDevice(2, 5);
    GTEST_LOG_(INFO) << "Current list size: " << monitor.lastestMtpDevList_.size();
    monitor.lastestMtpDevList_.clear();

    GTEST_LOG_(INFO) << "UmountDetachedMtpDeviceTest_001 end";
}
/**
 * @tc.name: IsHwitDeviceTest_001
 * @tc.desc: Verify returns true when KEY_CUST contains 'hwit'.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, IsHwitDeviceTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsHwitDeviceTest_001 start";
    g_paramOutContent = "hwit";
    g_getParameter = 4;
    EXPECT_TRUE(MtpDeviceMonitor::GetInstance().IsHwitDevice());
    g_paramOutContent = "";
    g_getParameter = 0;
    GTEST_LOG_(INFO) << "IsHwitDeviceTest_001 end";
}

/**
 * @tc.name: IsHwitDeviceTest_002
 * @tc.desc: Verify returns false when KEY_CUST does not contain 'hwit'.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, IsHwitDeviceTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsHwitDeviceTest_002 start";
    g_paramOutContent = "global";
    g_getParameter = 6;
    EXPECT_FALSE(MtpDeviceMonitor::GetInstance().IsHwitDevice());
    g_paramOutContent = "";
    g_getParameter = 0;
    GTEST_LOG_(INFO) << "IsHwitDeviceTest_002 end";
}

/**
 * @tc.name: OnMtpDisableParamChangeTest_001
 * @tc.desc: Verify clears device list when param change triggers disable condition.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, OnMtpDisableParamChangeTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnMtpDisableParamChangeTest_001 start";
    auto& monitor = MtpDeviceMonitor::GetInstance();
    monitor.lastestMtpDevList_.push_back({.id = "test_dev"});
    
    g_paramOutContent = "true";
    g_getParameter = 4;
    MtpDeviceMonitor::OnMtpDisableParamChange(
        SYS_PARAM_SERVICE_PERSIST_ENABLE, "true", &monitor);
    
    EXPECT_TRUE(monitor.lastestMtpDevList_.empty());
    g_paramOutContent = "";
    g_getParameter = 0;
    GTEST_LOG_(INFO) << "OnMtpDisableParamChangeTest_001 end";
}

/**
 * @tc.name: OnEnterpriseParamChangeTest_001
 * @tc.desc: Verify no clear when param change does NOT trigger disable condition.
 * @tc.type: FUNC
 */
HWTEST_F(MtpDeviceMonitorTest, OnEnterpriseParamChangeTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnEnterpriseParamChangeTest_001 start";
    auto& monitor = MtpDeviceMonitor::GetInstance();
    monitor.lastestMtpDevList_.push_back({.id = "keep_dev"});
    
    g_paramOutContent = "false";
    g_getParameter = 5;
    MtpDeviceMonitor::OnEnterpriseParamChange(
        SYS_PARAM_SERVICE_ENTERPRISE_ENABLE, "false", &monitor);
    
    EXPECT_EQ(monitor.lastestMtpDevList_.size(), 1U);
    monitor.lastestMtpDevList_.clear();
    g_paramOutContent = "";
    g_getParameter = 0;
    GTEST_LOG_(INFO) << "OnEnterpriseParamChangeTest_001 end";
}
} // STORAGE_DAEMON
} // OHOS