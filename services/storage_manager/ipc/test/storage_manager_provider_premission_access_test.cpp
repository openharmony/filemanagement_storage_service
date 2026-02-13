/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "disk.h"
#include "message_parcel.h"
#include "mock/uece_activation_callback_mock.h"
#include "storage_manager.h"
#include "storage_manager_provider.h"
#include "storage_service_errno.h"
#include "test/common/help_utils.h"
#include "user/multi_user_manager_service.h"
#include "volume_core.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
namespace OHOS {

int g_pStatus  = -1;

namespace Security::AccessToken {
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return g_pStatus ;
}
} // namespace Security::AccessToken
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
class StorageManagerProviderTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp();
    void TearDown();

    StorageManagerProvider *storageManagerProviderTest_;
};

void StorageManagerProviderTest::SetUp(void)
{
    storageManagerProviderTest_ = new StorageManagerProvider(STORAGE_MANAGER_MANAGER_ID);
}

void StorageManagerProviderTest::TearDown(void)
{
    if (storageManagerProviderTest_ != nullptr) {
        delete storageManagerProviderTest_;
        storageManagerProviderTest_ = nullptr;
    }
}

/**
 * @tc.name: StorageManagerProviderTest_MountFileMgrFuse_001
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "../mnt/mtp/device/storage/usb";
    int32_t fuseFd = -1;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    EXPECT_EQ(fuseFd, -1);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_001
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 1001;
    std::string path = "../mnt/mtp/device/storage/usb";
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsFileOccupied_001
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_IsFileOccupied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFileOccupied_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string path = "../test/file";
    std::vector<std::string> inputList = {"file1", "file2"};
    std::vector<std::string> outputList;
    bool isOccupy = false;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->IsFileOccupied(path, inputList, outputList, isOccupy);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFileOccupied_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_QueryUsbIsInUse_001
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_QueryUsbIsInUse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string diskPath = "../dev/sda1";
    bool isInUse = false;
    g_pStatus  = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    auto ret = storageManagerProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_QueryUsbIsInUse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_IsFilePathInvalid_001
 * @tc.desc: Verify the IsFilePathInvalid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_IsFilePathInvalid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFilePathInvalid_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    const std::array<std::tuple<std::string, bool>, 15> testCases = {
        std::make_tuple<std::string, bool>("/valid/path/file.txt", true),
        std::make_tuple<std::string, bool>("../invalid/path", false),
        std::make_tuple<std::string, bool>("/valid/../path", false),
        std::make_tuple<std::string, bool>("/path/to/..", false),
        std::make_tuple<std::string, bool>("", true),
        std::make_tuple<std::string, bool>("/", true),
        std::make_tuple<std::string, bool>("../a/../b", false),
        std::make_tuple<std::string, bool>("a/../b", false),
        std::make_tuple<std::string, bool>("../a/b", false),
        std::make_tuple<std::string, bool>("//../a", false),
        std::make_tuple<std::string, bool>("/valid%path/file.txt", true),
        std::make_tuple<std::string, bool>("a../b", true),
        std::make_tuple<std::string, bool>("../", false),
        std::make_tuple<std::string, bool>("/path/..more", true),
        std::make_tuple<std::string, bool>("/a/..b", true)};
    for (auto testCase : testCases) {
        const std::string &input = std::get<0>(testCase);
        const bool expected = std::get<1>(testCase);
        EXPECT_NE(storageManagerProviderTest_->IsFilePathInvalid(input), expected);
    }
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_IsFilePathInvalid_001 end";
}
} // namespace StorageManager
} // namespace OHOS
