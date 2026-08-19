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

#include <gtest/gtest.h>
#include <cerrno>
#include <sys/sysmacros.h>

#include "disk_manager/disk/dm_device.h"
#include "dm_device_mock.h"

using namespace testing::ext;
namespace OHOS {
namespace StorageDaemon {

static constexpr const char *TEST_DEV_PATH = "/dev/block/test_dev";
// Mock 默认 deviceBytes=1GB，totalSectors=2097152
static constexpr uint64_t MOCK_TOTAL_SECTORS = 1024ULL * 1024 * 1024 / 512;

// ==================== 测试夹具 ====================

class DmDeviceTest : public testing::Test {
public:
    void SetUp() override
    {
        g_mock.SetUp(true);
    }
    void TearDown() override
    {
        g_mock.TearDown();
    }
};

// ==================== 正常路径（Create 含 Exists 恢复） ====================

HWTEST_F(DmDeviceTest, Create_DeviceExistsRecovery, TestSize.Level1)
{
    // 模拟进程恢复场景：dm 设备已存在于内核
    // statusFail=false 让 DM_DEV_STATUS 成功，Exists 发现设备已存在，直接恢复状态
    g_mock.statusFail = false;
    DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_TRUE(dm.Create());
    EXPECT_EQ(dm.GetDeviceDev(), makedev(253, 0));
}

HWTEST_F(DmDeviceTest, Create_DeviceNotExistsThenCreate, TestSize.Level1)
{
    // 模拟首次创建场景：dm 设备不存在，Exists 返回 false，走正常创建流程
    // statusFail 默认 true，DM_DEV_STATUS 返回 ENXIO
    DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_TRUE(dm.Create());
    EXPECT_EQ(dm.GetDeviceDev(), makedev(253, 0));
}

HWTEST_F(DmDeviceTest, Create_Success, TestSize.Level1)
{
    // startSector=0（整盘映射）
    DmDevice dm1(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_TRUE(dm1.Create());
    EXPECT_EQ(dm1.GetDeviceDev(), makedev(253, 0));

    // startSector!=0（分区偏移映射）
    DmDevice dm2(TEST_DEV_PATH, 4096, MOCK_TOTAL_SECTORS - 8192);
    EXPECT_TRUE(dm2.Create());
    EXPECT_EQ(dm2.GetDeviceDev(), makedev(253, 0));
}

HWTEST_F(DmDeviceTest, Create_Idempotent, TestSize.Level2)
{
    DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    ASSERT_TRUE(dm.Create());
    // 已 ACTIVE 时再次调用直接返回 true
    EXPECT_TRUE(dm.Create());
}

// ==================== 参数校验 ====================

HWTEST_F(DmDeviceTest, Create_InvalidParams, TestSize.Level1)
{
    // sectorCount=0 时直接拒绝
    DmDevice dm1(TEST_DEV_PATH, 0, 0);
    EXPECT_FALSE(dm1.Create());

    // 路径含空格时直接拒绝（DM linear 参数以空格分隔）
    DmDevice dm2("/dev/block/bad path", 0, 100);
    EXPECT_FALSE(dm2.Create());

    // start + count 超出总扇区数
    g_mock.deviceBytes = 512 * 100;
    DmDevice dm3(TEST_DEV_PATH, 0, 200);
    EXPECT_FALSE(dm3.Create());

    // start + count 溢出
    DmDevice dm4(TEST_DEV_PATH, 50, UINT64_MAX - 49);
    EXPECT_FALSE(dm4.Create());

    // dmName 为空（路径 basename 为空，如 "/"）时拒绝
    DmDevice dm5("/", 0, 100);
    EXPECT_FALSE(dm5.Create());

    // 获取总扇区数为 0 时拒绝
    g_mock.blkGetSize64Fail = true;
    DmDevice dm6(TEST_DEV_PATH, 0, 100);
    EXPECT_FALSE(dm6.Create());
}

// ==================== I/O 失败 ====================

HWTEST_F(DmDeviceTest, Create_IoFail, TestSize.Level2)
{
    // open control 失败
    g_mock.openControlFail = true;
    DmDevice dm1(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_FALSE(dm1.Create());

    // CreateDevice 失败
    g_mock.openControlFail = false;
    g_mock.createFail = true;
    DmDevice dm2(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_FALSE(dm2.Create());

    // LoadTable 失败
    g_mock.createFail = false;
    g_mock.loadTableFail = true;
    DmDevice dm3(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_FALSE(dm3.Create());

    // ResumeDevice 失败
    g_mock.loadTableFail = false;
    g_mock.resumeFail = true;
    DmDevice dm4(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_FALSE(dm4.Create());
}

// ==================== CreateDevice/LoadTable 内部失败 ====================

HWTEST_F(DmDeviceTest, LoadTable_StrncpySFail, TestSize.Level2)
{
    // 调用链: Exists.InitDmIoctl#1 -> CreateDevice#2 -> LoadTable.InitDmIoctl#3 -> LoadTable.target_type#4
    // 让第4次 strncpy_s 失败（target_type 写入），后续 RemoveDevice 的 #5 正常
    g_mock.strncpySFailAt = 4;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        EXPECT_FALSE(dm.Create());
    }
    // CreateDevice 成功但 LoadTable 失败 -> 析构应 RemoveDevice
    EXPECT_TRUE(g_mock.dmRemoveCalled);
}

HWTEST_F(DmDeviceTest, LoadTable_StrcpySFail, TestSize.Level2)
{
    // strcpy_s 只在 LoadTable 中调用一次（params），第1次就失败
    g_mock.strcpySFailAt = 1;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        EXPECT_FALSE(dm.Create());
    }
    EXPECT_TRUE(g_mock.dmRemoveCalled);
}

// ==================== 析构行为 ====================

HWTEST_F(DmDeviceTest, Destructor_CreateDeviceFailNoRemove, TestSize.Level1)
{
    g_mock.createFail = true;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        EXPECT_FALSE(dm.Create());
    }
    EXPECT_FALSE(g_mock.dmRemoveCalled);
}

HWTEST_F(DmDeviceTest, Destructor_LoadTableFailRemove, TestSize.Level1)
{
    g_mock.loadTableFail = true;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        EXPECT_FALSE(dm.Create());
    }
    EXPECT_TRUE(g_mock.dmRemoveCalled);
}

HWTEST_F(DmDeviceTest, Destructor_ResumeDeviceFailRemove, TestSize.Level1)
{
    g_mock.resumeFail = true;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        EXPECT_FALSE(dm.Create());
    }
    EXPECT_TRUE(g_mock.dmRemoveCalled);
}

HWTEST_F(DmDeviceTest, Destructor_ActiveNoRemove, TestSize.Level1)
{
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        ASSERT_TRUE(dm.Create());
    }
    EXPECT_FALSE(g_mock.dmRemoveCalled);
}

HWTEST_F(DmDeviceTest, Destructor_RemoveFailNoCrash, TestSize.Level1)
{
    g_mock.loadTableFail = true;
    g_mock.removeFail = true;
    int beforeClose = g_mock.closeCount;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
        EXPECT_FALSE(dm.Create());
    }
    EXPECT_TRUE(g_mock.dmRemoveCalled);
    EXPECT_GT(g_mock.closeCount, beforeClose);
}

HWTEST_F(DmDeviceTest, Destructor_NoFdNoClose, TestSize.Level1)
{
    // 源设备打开失败 → totalSectors=0 → 构造提前退出，fd_ 保持 -1
    g_mock.openSourceFail = true;
    {
        DmDevice dm(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    }
    EXPECT_FALSE(g_mock.dmRemoveCalled);
}

// ==================== GetDeviceDev ====================

HWTEST_F(DmDeviceTest, GetDeviceDev_States, TestSize.Level1)
{
    // 未激活时返回 0
    DmDevice dm1(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    EXPECT_EQ(dm1.GetDeviceDev(), static_cast<dev_t>(0));

    // Create 成功后返回正确的 dev_t
    DmDevice dm2(TEST_DEV_PATH, 0, MOCK_TOTAL_SECTORS - 4096);
    ASSERT_TRUE(dm2.Create());
    EXPECT_EQ(dm2.GetDeviceDev(), makedev(253, 0));
}

}  // namespace StorageDaemon
}  // namespace OHOS
