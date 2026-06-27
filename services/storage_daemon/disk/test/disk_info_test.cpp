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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/sysmacros.h>

#include "disk/disk_info.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "mock/storage_manager_client_mock.h"
#include "disk_info_test_mock.h"
#include "netlink/netlink_data.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class DiskInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<StorageManagerClientMock> storageManagerClientMock_ = nullptr;
};

void DiskInfoTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void DiskInfoTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void DiskInfoTest::SetUp(void)
{
    diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
    DiskUtilMoc::diskUtilMoc = diskUtilMoc_;

    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;

    storageManagerClientMock_ = std::make_shared<StorageManagerClientMock>();
    StorageManagerClientMock::iStorageManagerClientMock_ = storageManagerClientMock_;
}

void DiskInfoTest::TearDown(void)
{
    DiskUtilMoc::diskUtilMoc = nullptr;
    diskUtilMoc_ = nullptr;

    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;

    StorageManagerClientMock::iStorageManagerClientMock_ = nullptr;
    storageManagerClientMock_ = nullptr;
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetRemovable_001
 * @tc.desc: Verify the GetRemovable function returns default value.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetRemovable_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetRemovable_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    // GetRemovable returns default value (true)
    bool removable = diskInfo->GetRemovable();
    EXPECT_TRUE(removable);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetRemovable_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_GetExtraInfo_001
 * @tc.desc: Verify the GetExtraInfo function returns default value.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_GetExtraInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetExtraInfo_001 start";

    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(8, 0);
    int flag = 0;
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, flag);
    ASSERT_TRUE(diskInfo != nullptr);

    // GetExtraInfo returns default value (empty string)
    std::string extraInfo = diskInfo->GetExtraInfo();
    EXPECT_EQ(extraInfo, "");

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_GetExtraInfo_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_SetExtraInfo_001
 * @tc.desc: Verify the SetExtraInfo function with normal disk.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_SetExtraInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_SetExtraInfo_001 start";

    unsigned int major = 8;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/test";
    dev_t device = makedev(major, minor);
    std::string diskName = "sda";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    EXPECT_CALL(*diskUtilMoc_, GetScsiBusNum(testing::_)).WillRepeatedly(testing::Return("0"));

    diskInfo->SetExtraInfo();

    std::string extraInfo = diskInfo->GetExtraInfo();
    EXPECT_FALSE(extraInfo.empty());

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_SetExtraInfo_001 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_SetExtraInfo_002
 * @tc.desc: Verify the SetExtraInfo function with CD/DVD disk type.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_SetExtraInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_SetExtraInfo_002 start";

    unsigned int major = 11;
    unsigned int minor = 0;
    std::string sysPath = "/devices/platform/test";
    std::string devPath = "/dev/block/sr0";
    dev_t device = makedev(major, minor);
    std::string diskName = "sr0";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device,
        static_cast<int>(DiskInfo::DiskType::CD_DVD_BD));
    ASSERT_TRUE(diskInfo != nullptr);

    EXPECT_CALL(*diskUtilMoc_, GetScsiBusNum(testing::_)).WillRepeatedly(testing::Return("0"));
    EXPECT_CALL(*diskUtilMoc_, GetOpticalDriveType(testing::_)).WillOnce(testing::Return("DVD"));
    EXPECT_CALL(*diskUtilMoc_, GetCDType(testing::_)).WillOnce(testing::Return("DVD-ROM"));
    EXPECT_CALL(*diskUtilMoc_, GetOpticalDriveMaxWriteSpeed(testing::_, testing::_))
        .WillOnce(testing::Invoke([](const std::string &path, int32_t &speed) {
            speed = 8;
            return E_OK;
        }));
    EXPECT_CALL(*diskUtilMoc_, GetOddDriverType(testing::_)).WillOnce(testing::Return("usb-storage"));

    diskInfo->SetExtraInfo();

    std::string extraInfo = diskInfo->GetExtraInfo();
    EXPECT_FALSE(extraInfo.empty());
    EXPECT_NE(extraInfo.find("ODD_INFO"), std::string::npos);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_SetExtraInfo_002 end";
}

/**
 * @tc.name: Storage_Service_DiskInfoTest_SetExtraInfo_003
 * @tc.desc: Verify the SetExtraInfo function with empty vendor and product.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(DiskInfoTest, Storage_Service_DiskInfoTest_SetExtraInfo_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_SetExtraInfo_003 start";

    unsigned int major = 8;
    unsigned int minor = 1;
    std::string sysPath = "/devices/platform/test2";
    std::string devPath = "/dev/block/test2";
    dev_t device = makedev(major, minor);
    std::string diskName = "sdb";
    auto diskInfo = std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, 0);
    ASSERT_TRUE(diskInfo != nullptr);

    EXPECT_CALL(*diskUtilMoc_, GetScsiBusNum(testing::_)).WillRepeatedly(testing::Return("1"));

    diskInfo->SetExtraInfo();

    std::string extraInfo = diskInfo->GetExtraInfo();
    EXPECT_FALSE(extraInfo.empty());
    EXPECT_NE(extraInfo.find("vendor"), std::string::npos);
    EXPECT_NE(extraInfo.find("product"), std::string::npos);

    GTEST_LOG_(INFO) << "Storage_Service_DiskInfoTest_SetExtraInfo_003 end";
}
}
}
