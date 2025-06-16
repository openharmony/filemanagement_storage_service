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
 
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <system_ability_definition.h>
 
#include "accesstoken_kit.h"
#include "disk.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "storage_manager.h"
#include "storage_manager_provider.h"
#include "storage_service_errno.h"
#include "test/common/help_utils.h"
#include "user/multi_user_manager_service.h"
#include "volume_core.h"
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return Security::AccessToken::TOKEN_NATIVE;
}
 
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}
 
int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    nativeTokenInfoRes.processName = "foundation";
    return 0;
}
}
 
pid_t g_testCallingUid = 1009;
 
namespace OHOS {
pid_t IPCSkeleton::GetCallingUid()
{
    return g_testCallingUid;
}
 
uint32_t IPCSkeleton::GetCallingTokenID()
{
    uint32_t callingTokenID = 100;
    return callingTokenID;
}
}
 
namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
class StorageManagerProviderTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
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
 
class ScopedTestUid {
public:
    explicit ScopedTestUid(pid_t newUid) : oldUid(g_testCallingUid) { g_testCallingUid = newUid; }
    ~ScopedTestUid() { g_testCallingUid = oldUid; }
private:
    pid_t oldUid;
};
 
/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_001
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(volId, fsTypeStr, uuid, path, description);
    EXPECT_EQ(ret, E_OK);

    
    int32_t fsType = 1;
    std::string diskId = "disk-1-6";
    VolumeCore vc(volId, fsType, diskId);
    storageManagerProviderTest_->NotifyVolumeCreated(vc);
    ret = storageManagerProviderTest_->NotifyVolumeDamaged(volId, fsTypeStr, uuid, path, description);
    EXPECT_EQ(ret, E_OK);

    int64_t sizeBytes = 1024;
    std::string vendor = "vendor-1";
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, path, vendor, 1);
    storageManagerProviderTest_->NotifyDiskCreated(disk);
    ret = storageManagerProviderTest_->NotifyVolumeDamaged(volId, fsTypeStr, uuid, path, description);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_001 end";
}
 
/**
 * @tc.name: StorageManagerProviderTest_TryToFix_001
 * @tc.desc: Verify the TryToFix function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_TryToFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string volId = "vol-8-1";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-1";
    std::string path = "/";
    std::string description = "My Disk";

    auto ret = storageManagerProviderTest_->TryToFix(volId);
    EXPECT_EQ(ret, E_OK);
    
    int32_t fsType = 1;
    std::string diskId = "disk-1-6";
    VolumeCore vc(volId, fsType, diskId);
    storageManagerProviderTest_->NotifyVolumeCreated(vc);
    ret = storageManagerProviderTest_->TryToFix(volId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_TryToFix_001 end";
}
}
}
