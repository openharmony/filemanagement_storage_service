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
#include "bundle_manager_connector.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "disk.h"
#include "ext_bundle_stats.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "storage_manager_provider.h"
#include "storage_service_errno.h"
#include "test/common/help_utils.h"
#include "mock/uece_activation_callback_mock.h"
#include "volume_core.h"
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <cstdint>
int32_t g_accessTokenType = 1;
namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    if (g_accessTokenType == -1) {
        return Security::AccessToken::TOKEN_INVALID;
    }
    if (g_accessTokenType == 0) {
        return Security::AccessToken::TOKEN_HAP;
    }
    if (g_accessTokenType == 1) {
        return Security::AccessToken::TOKEN_NATIVE;
    }
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

pid_t g_testCallingUid = 5523;
pid_t g_testUpdateServiceUid = 6666;
bool g_returnUpdateService = false;
namespace OHOS {
pid_t IPCSkeleton::GetCallingUid()
{
    if (g_returnUpdateService) {
        return g_testUpdateServiceUid;
    }
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
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp();
    void TearDown();
    StorageManagerProvider *storageManagerProviderTest_;
};

class MockBundleMgr : public AppExecFwk::IBundleMgr {
public:
    bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        bundleName = "com.example.fake";
        return true;
    }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

BundleMgrConnector::BundleMgrConnector() {}
BundleMgrConnector::~BundleMgrConnector() {}
sptr<AppExecFwk::IBundleMgr> g_testBundleMgrProxy = nullptr;
#ifdef STORAGE_MANAGER_UNIT_TEST
sptr<AppExecFwk::IBundleMgr> BundleMgrConnector::GetBundleMgrProxy()
{
    return g_testBundleMgrProxy;
}
#endif

void StorageManagerProviderTest::SetUp(void)
{
    g_returnUpdateService = false;
    g_accessTokenType = 1;
    storageManagerProviderTest_ = new StorageManagerProvider(STORAGE_MANAGER_MANAGER_ID);
}

void StorageManagerProviderTest::TearDown(void)
{
    if (storageManagerProviderTest_ != nullptr) {
        delete storageManagerProviderTest_;
        storageManagerProviderTest_ = nullptr;
    }
}

void StringVecToRawData(const std::vector<std::string> &stringVec, StorageFileRawData &rawData)
{
    std::stringstream ss;
    uint32_t stringCount = stringVec.size();
    ss.write(reinterpret_cast<const char*>(&stringCount), sizeof(stringCount));
    for (uint32_t i = 0; i < stringCount; ++i) {
        uint32_t strLen = stringVec[i].length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(stringVec[i].c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

class ScopedTestUid {
public:
    explicit ScopedTestUid(pid_t newUid) : oldUid(g_testCallingUid) { g_testCallingUid = newUid; }
    ~ScopedTestUid() { g_testCallingUid = oldUid; }
private:
    pid_t oldUid;
};

/**
 * @tc.name: StorageManagerProviderTest_PrepareAddUser_003
 * @tc.desc: Verify the PrepareAddUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareAddUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_003  start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    int32_t userId = 121;
    uint32_t flag = 3;
    auto ret = storageManagerProviderTest_->PrepareAddUser(userId, flag);
    EXPECT_EQ(ret, E_OK);
    storageManagerProviderTest_->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_003 end";
}
/**
 * @tc.number: SUB_STORAGE_StorageManagerProviderTest_RemoveUser_003
 * @tc.name: StorageManagerProviderTest_RemoveUser_003
 * @tc.desc: Test function of RemoveUser interface for removing a prepared user normally.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RemoveUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-begin StorageManagerProviderTest_RemoveUser_003";
    int32_t userId = 103;
    uint32_t flag = 3;
    int32_t result;
    storageManagerProviderTest_->PrepareAddUser(userId, flag);
    result = storageManagerProviderTest_->RemoveUser(userId, flag);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-end StorageManagerProviderTest_RemoveUser_003";
}

/**
 * @tc.number: SUB_STORAGE_StorageManagerProviderTest_PrepareStartUser_003
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_003
 * @tc.desc: Test function of PrepareStartUser interface for starting a prepared user normally.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-begin StorageManagerProviderTest_PrepareStartUser_003";
    int32_t userId = 105;
    uint32_t flag = 3;
    int32_t result;
    storageManagerProviderTest_->PrepareAddUser(userId, flag);
    result = storageManagerProviderTest_->PrepareStartUser(userId);
    EXPECT_EQ(result, E_OK);
    storageManagerProviderTest_->StopUser(userId);
    storageManagerProviderTest_->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-end StorageManagerProviderTest_PrepareStartUser_003";
}

/**
 * @tc.number: SUB_STORAGE_StorageManagerProviderTest_StopUser_003
 * @tc.name: StorageManagerProviderTest_StopUser_003
 * @tc.desc: Test function of StopUser interface for stopping a user normally.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-begin StorageManagerProviderTest_StopUser_0031";
    int32_t userId = 109;
    int32_t result;
    result = storageManagerProviderTest_->StopUser(userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-end StorageManagerProviderTest_StopUser_0031";
}

/**
 * @tc.number: SUB_STORAGE_StorageManagerProviderTest_CompleteAddUser_003
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_003
 * @tc.desc: Test function of CompleteAddUser interface for a specific user ID.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level: Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-begin StorageManagerProviderTest_CompleteAddUser_003";
    int32_t userId = 100;
    int32_t result;
    result = storageManagerProviderTest_->CompleteAddUser(userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest-end StorageManagerProviderTest_CompleteAddUser_003";
}

/**
 * @tc.name: StorageManagerProviderTest_EraseAllUserEncryptedKeys_003
 * @tc.desc: Verify the EraseAllUserEncryptedKeys when service is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_EraseAllUserEncryptedKeys_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_003 start";
    g_returnUpdateService = true;
    auto ret = storageManagerProviderTest_->EraseAllUserEncryptedKeys();
    EXPECT_EQ(ret,  E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateUserAuth_003
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateUserAuth_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserAuth_003 start";
    uint32_t userId = 110;
    auto ret = storageManagerProviderTest_->UpdateUserAuth(userId, 0, {}, {}, {});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserAuth_003 E_USERID_RANGE end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_003
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_003 start";
    std::vector<uint8_t> authToken = {1, 2, 3, 4};
    std::vector<uint8_t> newSecret = {5, 6, 7, 8};
    std::vector<std::vector<uint8_t>> plainText = {{9, 10}, {11, 12}};
    uint64_t secureUid = 123456789;
    uint32_t userId = 800;
    auto ret = storageManagerProviderTest_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid,
        userId, plainText);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUseAuthWithRecoveryKey_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ActiveUserKey_003
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ActiveUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ActiveUserKey_003 start";
    uint32_t userId = 111;
    auto ret = storageManagerProviderTest_->ActiveUserKey(userId, {}, {});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ActiveUserKey_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_InactiveUserKey_003
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_InactiveUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_InactiveUserKey_003 start";
    uint32_t userId = 112;
    auto ret = storageManagerProviderTest_->InactiveUserKey(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_InactiveUserKey_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFileEncryptStatus_003
 * @tc.desc: Verify the GetFileEncryptStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFileEncryptStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFileEncryptStatus_003 start";
    uint32_t userId = 800;
    bool isEncrypted = false;
    auto ret = storageManagerProviderTest_->GetFileEncryptStatus(userId, isEncrypted);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "torageManagerProviderTest_GetFileEncryptStatus_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserNeedActiveStatus_003
 * @tc.desc: Verify the GetUserNeedActiveStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserNeedActiveStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserNeedActiveStatus_003 start";
    uint32_t userId = 800;
    bool needActive = false;
    auto ret = storageManagerProviderTest_->GetUserNeedActiveStatus(userId, needActive);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserNeedActiveStatus_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UnlockUserScreen_003
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UnlockUserScreen_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnlockUserScreen_003 start";
    uint32_t userId = 100;
    auto ret = storageManagerProviderTest_->UnlockUserScreen(userId, {}, {});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnlockUserScreen_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetLockScreenStatus_003
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetLockScreenStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetLockScreenStatus_003 start";
    uint32_t userId = 100;
    bool lockScreenStatus = false;
    auto ret = storageManagerProviderTest_->GetLockScreenStatus(userId, lockScreenStatus);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetLockScreenStatus_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_storageManagerProviderTest__003
 * @tc.desc: Verify the storageManagerProviderTest_ function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GenerateAppkey__003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GenerateAppkey__003 start";
    uint32_t userId = 108;
    uint32_t hashId = 12345;
    std::string keyId = "keys";
    auto ret = storageManagerProviderTest_->GenerateAppkey(hashId, userId, keyId);
    EXPECT_EQ(ret, E_OK);
    storageManagerProviderTest_->DeleteAppkey(keyId);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GenerateAppkey__003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CreateRecoverKey_003
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CreateRecoverKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateRecoverKey_003 start";
    uint32_t userId = 100;
    uint32_t userType = 10;
    auto ret = storageManagerProviderTest_->CreateRecoverKey(userId, userType, {}, {});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateRecoverKey_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetRecoverKey_003
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetRecoverKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetRecoverKey_003 start";
    auto ret = storageManagerProviderTest_->SetRecoverKey({});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetRecoverKey_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateKeyContext_003
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateKeyContext_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateKeyContext_003 start";
    uint32_t userId = 113;
    auto ret = storageManagerProviderTest_->UpdateKeyContext(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateKeyContext_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ResetSecretWithRecoveryKey_003
 * @tc.desc: Verify the ResetSecretWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ResetSecretWithRecoveryKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ResetSecretWithRecoveryKey_003 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto  ret = storageManagerProviderTest_->ResetSecretWithRecoveryKey(100, 1, {});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ResetSecretWithRecoveryKey_003 end";
}
}
}