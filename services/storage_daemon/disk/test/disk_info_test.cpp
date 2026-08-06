/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include <sys/sysmacros.h>

#include "disk/disk_info.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class DiskInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDiskId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDiskId_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_EQ(diskInfo->GetDiskId(), "disk-8-0");

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDiskId_001 end";
}

HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetDiskType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDiskType_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, DiskInfo::DiskType::USB_FLASH);
    ASSERT_TRUE(diskInfo != nullptr);
    EXPECT_EQ(diskInfo->GetDiskType(), DiskInfo::DiskType::USB_FLASH);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetDiskType_001 end";
}
}
}
