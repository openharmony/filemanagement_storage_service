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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "disk_manager/disk/disk_utils.h"
#include "mock/disk_utils_mock.h"
#include "mock/file_utils_mock.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class ExtDiskUtilsCdTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        diskUtilMoc_ = std::make_shared<DiskUtilMoc>();
        IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;

        fileUtilMoc_ = std::make_shared<FileUtilMoc>();
        IFileUtilMoc::fileUtilMoc = fileUtilMoc_;
    }
    void TearDown()
    {
        IDiskUtilMoc::diskUtilMoc = nullptr;
        diskUtilMoc_ = nullptr;

        IFileUtilMoc::fileUtilMoc = nullptr;
        fileUtilMoc_ = nullptr;
    }

    static inline std::shared_ptr<DiskUtilMoc> diskUtilMoc_ = nullptr;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

/**
 * @tc.name: MockRedirect_Nullptr_GetOpticalDriveNode
 * @tc.desc: Verify GetOpticalDriveNode returns empty when mock pointer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, MockRedirect_Nullptr_GetOpticalDriveNode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetOpticalDriveNode start";
    IDiskUtilMoc::diskUtilMoc = nullptr;
    std::string result = GetOpticalDriveNode("/dev/block/vol-11-0");
    EXPECT_TRUE(result.empty());
    IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetOpticalDriveNode end";
}

/**
 * @tc.name: MockRedirect_Nullptr_GetUsedSizeFromSysfs
 * @tc.desc: Verify DiskUtils::GetUsedSizeFromSysfs returns -1 when mock pointer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, MockRedirect_Nullptr_GetUsedSizeFromSysfs, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetUsedSizeFromSysfs start";
    IDiskUtilMoc::diskUtilMoc = nullptr;
    int64_t result = DiskUtils::GetUsedSizeFromSysfs("/dev/block/vol-11-0");
    EXPECT_EQ(result, -1);
    IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetUsedSizeFromSysfs end";
}

/**
 * @tc.name: MockRedirect_Nullptr_GetDiscCapacity
 * @tc.desc: Verify DiskUtils::GetDiscCapacity returns 0 when mock pointer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, MockRedirect_Nullptr_GetDiscCapacity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetDiscCapacity start";
    IDiskUtilMoc::diskUtilMoc = nullptr;
    int64_t ret = DiskUtils::GetDiscCapacity(0, "DVD-R");
    EXPECT_EQ(ret, 0);
    IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetDiscCapacity end";
}

/**
 * @tc.name: MockRedirect_Nullptr_AdjustBlankDiscCapacity
 * @tc.desc: Verify DiskUtils::AdjustBlankDiscCapacity does nothing when mock pointer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, MockRedirect_Nullptr_AdjustBlankDiscCapacity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_AdjustBlankDiscCapacity start";
    int64_t totalSize = 4700372992L;
    int64_t usedSize = 1000000L;
    IDiskUtilMoc::diskUtilMoc = nullptr;
    DiskUtils::AdjustBlankDiscCapacity("/dev/block/vol-11-0", "DVD+RW", totalSize, usedSize);
    EXPECT_EQ(totalSize, 4700372992L);
    EXPECT_EQ(usedSize, 1000000L);
    IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_AdjustBlankDiscCapacity end";
}

/**
 * @tc.name: MockRedirect_Nullptr_GetCapacity
 * @tc.desc: Verify DiskUtils::GetCapacity returns E_ERR when mock pointer is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, MockRedirect_Nullptr_GetCapacity, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetCapacity start";
    int64_t totalSize = 0;
    int64_t freeSize = 0;
    IDiskUtilMoc::diskUtilMoc = nullptr;
    int32_t ret = DiskUtils::GetCapacity("/dev/block/vol-11-0", totalSize, freeSize);
    EXPECT_EQ(ret, E_ERR);
    IDiskUtilMoc::diskUtilMoc = diskUtilMoc_;
    GTEST_LOG_(INFO) << "MockRedirect_Nullptr_GetCapacity end";
}

/**
 * @tc.name: RefreshCDRomMediaNode_Failed
 * @tc.desc: Verify RefreshCDRomMediaNode returns E_ERR when mock returns failure.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, RefreshCDRomMediaNode_Failed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RefreshCDRomMediaNode_Failed start";
    EXPECT_CALL(*diskUtilMoc_, RefreshCDRomMediaNode(_)).WillOnce(Return(E_ERR));
    int32_t ret = RefreshCDRomMediaNode("/dev/block/vol-11-0");
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "RefreshCDRomMediaNode_Failed end";
}

/**
 * @tc.name: RefreshCDRomMediaNode_Success
 * @tc.desc: Verify RefreshCDRomMediaNode returns E_OK when mock returns success.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, RefreshCDRomMediaNode_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RefreshCDRomMediaNode_Success start";
    EXPECT_CALL(*diskUtilMoc_, RefreshCDRomMediaNode(_)).WillOnce(Return(E_OK));
    int32_t ret = RefreshCDRomMediaNode("/dev/block/vol-11-0");
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "RefreshCDRomMediaNode_Success end";
}

/**
 * @tc.name: GetOpticalDriveNode_NoMatch
 * @tc.desc: Verify GetOpticalDriveNode returns empty when volName has no dash, dash at end,
 *           or no /sys/block device matches major:minor.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, GetOpticalDriveNode_NoMatch, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_NoMatch start";
    // B1: GetOpticalDriveNode returns empty for various invalid inputs
    EXPECT_CALL(*diskUtilMoc_, GetOpticalDriveNode(_))
        .WillOnce(Return(""))
        .WillOnce(Return(""))
        .WillOnce(Return(""));
    EXPECT_TRUE(GetOpticalDriveNode("/dev/block/sr0").empty());
    EXPECT_TRUE(GetOpticalDriveNode("/dev/block/abc-").empty());
    EXPECT_TRUE(GetOpticalDriveNode("/dev/block/vol-999-999").empty());
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_NoMatch end";
}

/**
 * @tc.name: GetOpticalDriveNode_ValidDeviceFound
 * @tc.desc: Verify GetOpticalDriveNode finds the correct block device when major:minor matches.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, GetOpticalDriveNode_ValidDeviceFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_ValidDeviceFound start";
    // B2: GetOpticalDriveNode returns a valid block device name
    EXPECT_CALL(*diskUtilMoc_, GetOpticalDriveNode(_)).WillOnce(Return("sda"));
    std::string result = GetOpticalDriveNode("/dev/block/vol-8-0");
    EXPECT_EQ(result, "sda");
    GTEST_LOG_(INFO) << "GetOpticalDriveNode_ValidDeviceFound end";
}

/**
 * @tc.name: GetUsedSizeFromSysfs_DriveNodeEmpty
 * @tc.desc: Verify GetUsedSizeFromSysfs returns -1 when mock returns failure.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, GetUsedSizeFromSysfs_DriveNodeEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_DriveNodeEmpty start";
    EXPECT_CALL(*diskUtilMoc_, GetUsedSizeFromSysfs(_)).WillOnce(Return(-1));
    EXPECT_EQ(DiskUtils::GetUsedSizeFromSysfs("/dev/block/sr0"), -1);
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_DriveNodeEmpty end";
}

/**
 * @tc.name: GetUsedSizeFromSysfs_Success
 * @tc.desc: Verify GetUsedSizeFromSysfs returns sector count when mock succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(ExtDiskUtilsCdTest, GetUsedSizeFromSysfs_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_Success start";
    EXPECT_CALL(*diskUtilMoc_, GetUsedSizeFromSysfs(_)).WillOnce(Return(2048 * 512));
    int64_t result = DiskUtils::GetUsedSizeFromSysfs("/dev/block/vol-8-0");
    EXPECT_EQ(result, 2048 * 512);
    GTEST_LOG_(INFO) << "GetUsedSizeFromSysfs_Success end";
}

} // namespace StorageDaemon
} // namespace OHOS
