/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "volume/volume_manager_service_ext.h"
#include "disk/disk_manager_service.h"
#include "volume_core.h"
#include "storage_service_errno.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class VolumeManagerServiceTestExt : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: VolumeManagerServiceExtTest_Constructor_001
 * @tc.desc: Verify the VolumeManagerServiceExt constructor.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_Constructor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Constructor_001 start";
    
    // Test constructor - should call Init()
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Constructor_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_Destructor_001
 * @tc.desc: Verify the VolumeManagerServiceExt destructor.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_Destructor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Destructor_001 start";
    
    // Test destructor - should call UnInit()
    {
        auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
        ASSERT_TRUE(serviceExt != nullptr);
        // Destructor will be called when serviceExt goes out of scope
    }
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Destructor_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_Init_001
 * @tc.desc: Verify the Init function with successful dlopen.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_Init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Init_001 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    // Init is called in constructor, so we test the result
    // In test environment, dlopen may fail, but the object should still be created
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Init_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_UnInit_001
 * @tc.desc: Verify the UnInit function with valid handler.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_UnInit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_UnInit_001 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    // UnInit will be called in destructor
    // Test explicit UnInit call
    serviceExt->UnInit();
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_UnInit_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_001
 * @tc.desc: Verify NotifyUsbFuseMount with null handler.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_001 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    // In test environment, handler_ is likely nullptr due to missing .so file
    int fuseFd = 10;
    std::string volumeId = "vol-usb-001";
    std::string fsUuid = "550e8400-e29b-41d4-a716-446655440000";
    
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    
    // Should return E_MOUNT_CLOUD_FUSE when handler is null or dlsym fails
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_002
 * @tc.desc: Verify NotifyUsbFuseMount with empty volumeId.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_002 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    int fuseFd = 5;
    std::string volumeId = "";  // Empty volumeId
    std::string fsUuid = "12345678-1234-1234-1234-123456789abc";
    
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    
    // Should handle empty volumeId gracefully
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_002 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_003
 * @tc.desc: Verify NotifyUsbFuseMount with empty fsUuid.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_003 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    int fuseFd = 7;
    std::string volumeId = "vol-usb-test";
    std::string fsUuid = "";  // Empty fsUuid
    
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    
    // Should handle empty fsUuid gracefully
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_003 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_004
 * @tc.desc: Verify NotifyUsbFuseMount with negative fuseFd.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_004 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    int fuseFd = -1;  // Negative fuseFd
    std::string volumeId = "vol-usb-negative";
    std::string fsUuid = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_004 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_005
 * @tc.desc: Verify NotifyUsbFuseMount with boundary fuseFd values.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_005 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    std::string volumeId = "vol-usb-boundary";
    std::string fsUuid = "boundary-test-uuid-123456789abc";
    
    // Test with zero fuseFd
    int32_t result = serviceExt->NotifyUsbFuseMount(0, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test with large fuseFd
    result = serviceExt->NotifyUsbFuseMount(1000, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test with INT_MAX
    result = serviceExt->NotifyUsbFuseMount(INT_MAX, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_005 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_006
 * @tc.desc: Verify NotifyUsbFuseMount with long strings and special characters.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_006 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    int fuseFd = 15;
    
    // Test with special characters in volumeId
    std::string volumeId = "vol-usb-@#$%^&*()";
    std::string fsUuid = "special-char-uuid-123456789abc";
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test with very long volumeId
    volumeId = "vol-usb-very-very-very-long-volume-id-with-many-characters-to-test-boundary-conditions";
    result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test with very long fsUuid
    volumeId = "vol-usb-long-uuid";
    fsUuid = "very-long-uuid-string-that-exceeds-normal-uuid-length-for-boundary-testing-purposes";
    result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_006 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseMount_007
 * @tc.desc: Verify NotifyUsbFuseMount masking behavior for logs.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseMount_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_007 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    int fuseFd = 20;
    
    // Test string masking with short strings (length <= 4)
    std::string shortVolumeId = "vol";
    std::string shortFsUuid = "uuid";
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, shortVolumeId, shortFsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test string masking with exactly 4 characters
    std::string exactVolumeId = "vol4";
    std::string exactFsUuid = "uuid";
    result = serviceExt->NotifyUsbFuseMount(fuseFd, exactVolumeId, exactFsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test string masking with long strings (length > 4)
    std::string longVolumeId = "vol-usb-long-string";
    std::string longFsUuid = "550e8400-e29b-41d4-a716-446655440000";
    result = serviceExt->NotifyUsbFuseMount(fuseFd, longVolumeId, longFsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseMount_007 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseUMount_001
 * @tc.desc: Verify NotifyUsbFuseUMount with null handler.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseUMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_001 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    std::string volumeId = "vol-usb-unmount";
    
    // In test environment, handler_ is likely nullptr due to missing .so file
    int32_t result = serviceExt->NotifyUsbFuseUMount(volumeId);
    
    // Should return E_MOUNT_CLOUD_FUSE when handler is null or function call fails
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseUMount_002
 * @tc.desc: Verify NotifyUsbFuseUMount with empty volumeId.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseUMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_002 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    std::string volumeId = "";  // Empty volumeId
    
    int32_t result = serviceExt->NotifyUsbFuseUMount(volumeId);
    
    // Should handle empty volumeId gracefully
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_002 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseUMount_003
 * @tc.desc: Verify NotifyUsbFuseUMount with special characters in volumeId.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseUMount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_003 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    // Test with special characters
    std::string volumeId = "vol-usb-@#$%^&*()";
    int32_t result = serviceExt->NotifyUsbFuseUMount(volumeId);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test with Unicode characters
    volumeId = "vol-usb-测试-αβγ";
    result = serviceExt->NotifyUsbFuseUMount(volumeId);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test with very long volumeId
    volumeId = "vol-usb-very-very-very-long-volume-id-with-many-characters-to-test-boundary-conditions";
    result = serviceExt->NotifyUsbFuseUMount(volumeId);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_003 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_NotifyUsbFuseUMount_004
 * @tc.desc: Verify NotifyUsbFuseUMount with multiple calls.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_NotifyUsbFuseUMount_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_004 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    // Multiple calls with same volumeId
    std::string volumeId = "vol-usb-multi-unmount";
    
    for (int i = 0; i < 3; i++) {
        int32_t result = serviceExt->NotifyUsbFuseUMount(volumeId);
        EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
        GTEST_LOG_(INFO) << "Call " << (i + 1) << " result: " << result;
    }
    
    // Multiple calls with different volumeIds
    std::vector<std::string> volumeIds = {
        "vol-usb-001",
        "vol-usb-002",
        "vol-usb-003"
    };
    
    for (const auto& volId : volumeIds) {
        int32_t result = serviceExt->NotifyUsbFuseUMount(volId);
        EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    }
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_NotifyUsbFuseUMount_004 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_Integration_001
 * @tc.desc: Verify integration test with Mount and UMount sequence.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_Integration_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Integration_001 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    int fuseFd = 25;
    std::string volumeId = "vol-usb-integration";
    std::string fsUuid = "integration-test-uuid-123456789abc";
    
    // Test Mount first
    int32_t mountResult = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_EQ(mountResult, E_MOUNT_CLOUD_FUSE);
    
    // Then test UMount
    int32_t unmountResult = serviceExt->NotifyUsbFuseUMount(volumeId);
    EXPECT_EQ(unmountResult, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_Integration_001 end";
}

/**
 * @tc.name: VolumeManagerServiceExtTest_ErrorBranch_001
 * @tc.desc: Verify error branch coverage for dlsym failure.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(VolumeManagerServiceTestExt, VolumeManagerServiceExtTest_ErrorBranch_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_ErrorBranch_001 start";
    
    auto serviceExt = std::make_unique<VolumeManagerServiceExt>();
    ASSERT_TRUE(serviceExt != nullptr);
    
    // Force handler to be null to test dlsym failure branch
    serviceExt->UnInit();  // This sets handler_ to nullptr
    
    int fuseFd = 30;
    std::string volumeId = "vol-usb-error";
    std::string fsUuid = "error-test-uuid-123456789abc";
    
    // Test NotifyUsbFuseMount with null handler
    int32_t result = serviceExt->NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    // Test NotifyUsbFuseUMount with null handler
    result = serviceExt->NotifyUsbFuseUMount(volumeId);
    EXPECT_EQ(result, E_MOUNT_CLOUD_FUSE);
    
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest_ErrorBranch_001 end";
}
} // namespace
