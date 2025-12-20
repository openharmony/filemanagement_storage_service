/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <memory>

#include "ipc/storage_manager_client.h"
#include "disk/disk_info.h"
#include "storage_service_errno.h"
#include "volume_core.h"
#include "volume/external_volume_info.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;
class StorageManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    StorageManagerClient *storageManagerClient_;
};

void StorageManagerClientTest::SetUp(void)
{
    storageManagerClient_ = new StorageManagerClient();
}

void StorageManagerClientTest::TearDown(void)
{
    if (storageManagerClient_ != nullptr) {
        delete storageManagerClient_;
        storageManagerClient_ = nullptr;
    }
}

/**
 * @tc.name: StorageManagerClientTest_NotifyDiskCreated_001
 * @tc.desc: Verify the NotifyDiskCreated function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyDiskCreated_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyDiskCreated_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    std::string sysPath = "test";
    std::string devPath = "test";
    dev_t device = 1;
    int flag = 1;
    DiskInfo diskInfo(sysPath, devPath, device, flag);
    auto ret = storageManagerClient_->NotifyDiskCreated(diskInfo);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyDiskCreated_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyDiskDestroyed_001
 * @tc.desc: Verify the NotifyDiskDestroyed function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyDiskDestroyed_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyDiskDestroyed_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    std::string id = "test";
    auto ret = storageManagerClient_->NotifyDiskDestroyed(id);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyDiskDestroyed_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyVolumeCreated_001
 * @tc.desc: Verify the NotifyVolumeCreated function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyVolumeCreated_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeCreated_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    std::shared_ptr<VolumeInfo> info = nullptr;
    auto ret = storageManagerClient_->NotifyVolumeCreated(info);
    EXPECT_TRUE(ret == E_PARAMS_INVALID);

    info = std::make_shared<ExternalVolumeInfo>();
    ret = storageManagerClient_->NotifyVolumeCreated(info);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeCreated_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyVolumeMounted_001
 * @tc.desc: Verify the NotifyVolumeMounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyVolumeMounted_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeMounted_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    std::shared_ptr<VolumeInfo> info = nullptr;
    auto ret = storageManagerClient_->NotifyVolumeMounted(info);
    EXPECT_TRUE(ret == E_PARAMS_INVALID);

    info = std::make_shared<ExternalVolumeInfo>();
    ret = storageManagerClient_->NotifyVolumeMounted(info);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeMounted_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyVolumeStateChanged_001
 * @tc.desc: Verify the NotifyVolumeStateChanged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyVolumeStateChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeStateChanged_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    StorageManager::VolumeState state = StorageManager::VolumeState::UNMOUNTED;
    auto ret = storageManagerClient_->NotifyVolumeStateChanged("", state);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeStateChanged_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyVolumeDamaged_001
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyVolumeDamaged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeDamaged_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    std::shared_ptr<VolumeInfo> info = nullptr;
    auto ret = storageManagerClient_->NotifyVolumeDamaged(info);
    EXPECT_TRUE(ret == E_PARAMS_INVALID);

    info = std::make_shared<ExternalVolumeInfo>();
    ret = storageManagerClient_->NotifyVolumeDamaged(info);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyVolumeDamaged_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyMtpMounted_001
 * @tc.desc: Verify the NotifyMtpMounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyMtpMounted_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyMtpMounted_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    auto ret = storageManagerClient_->NotifyMtpMounted("", "", "", "");
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyMtpMounted_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_NotifyMtpUnmounted_001
 * @tc.desc: Verify the NotifyMtpUnmounted function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_NotifyMtpUnmounted_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyMtpUnmounted_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    auto ret = storageManagerClient_->NotifyMtpUnmounted("", "", false);
    EXPECT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_NotifyMtpUnmounted_001 end";
}

/**
 * @tc.name: StorageManagerClientTest_IsUsbFuseByType_001
 * @tc.desc: Verify the IsUsbFuseByType function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerClientTest, StorageManagerClientTest_IsUsbFuseByType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerClientTest_IsUsbFuseByType_001 start";

    ASSERT_TRUE(storageManagerClient_ != nullptr);

    auto enabled = true;
    auto ret = storageManagerClient_->IsUsbFuseByType("f2fs", enabled);
    EXPECT_TRUE(ret == E_OK);
    EXPECT_FALSE(enabled);

    GTEST_LOG_(INFO) << "StorageManagerClientTest_IsUsbFuseByType_001 end";
}
} // StorageDaemon
} // OHOS
