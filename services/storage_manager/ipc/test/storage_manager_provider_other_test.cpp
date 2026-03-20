/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "mock/storage_daemon_communication_mock.h"
#include "bundle_mgr_client.h"
#include "volume_core.h"
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <cstdint>

namespace {
int32_t g_getTokenTypeFlag = 0;
int32_t g_verifyAccessToken = 0;
int32_t g_returnBundleName = 0;
pid_t g_testCallingUid = 1009;
pid_t g_testUpdateServiceUid = 6666;
bool g_returnUpdateService = false;
}

void SetCallingUid(int32_t uid)
{
    g_testCallingUid = uid;
}

void MockGetTokenTypeFlag(int32_t typeFlag)
{
    g_getTokenTypeFlag = typeFlag;
}

void MockVerifyAccessToken(int32_t token)
{
    g_verifyAccessToken = token;
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

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

pid_t OHOS::IPCSkeleton::GetCallingUid()
{
    if (g_returnUpdateService) {
        return g_testUpdateServiceUid;
    }
    return g_testCallingUid;
}

uint32_t OHOS::IPCSkeleton::GetCallingTokenID()
{
    uint32_t callingTokenID = 100;
    return callingTokenID;
}
}

namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID mockRet)
{
    return static_cast<OHOS::Security::AccessToken::ATokenTypeEnum>(g_getTokenTypeFlag);
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return g_verifyAccessToken;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    nativeTokenInfoRes.processName = "foundation";
    return 0;
}
}

namespace OHOS {
namespace StorageManager {
using namespace testing;
using namespace testing::ext;
constexpr pid_t DFS_UID = 1009;
constexpr pid_t ACCOUNT_UID = 3058;
constexpr pid_t AOCO_UID = 7558;
constexpr pid_t FOUNDATION_UID = 5523;
constexpr int32_t USER_ID = 100;
constexpr int32_t MINUSERID = -1;
constexpr int32_t MAXUSERID = 10739;

class StorageManagerProviderTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
    std::shared_ptr<StorageDaemonCommunicationMock> sdCommMock_;
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

class MockBundleMgr : public AppExecFwk::IBundleMgr {
public:
    bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        if (g_returnBundleName == 0) {
            bundleName = "com.huawei.hmos.filemanager";
        } else if (g_returnBundleName == 1) {
            bundleName = "com.ohos.sceneboard";
        } else if (g_returnBundleName == 2) {
            bundleName = "com.ohos.medialibrary.medialibrarydata";
        }
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

/**
 * @tc.name: StorageManagerProviderMockTest_MountDisShareFile_001
 * @tc.desc: Verify the MountDisShareFile function returns E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderMockTest_MountDisShareFile_001, TestSize.Level1) {
    GTEST_LOG_(INFO) << "StorageManagerProviderMockTest_MountDisShareFile_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::map<std::string, std::string> shareFiles = {
        {"file1", "path1"},
        {"file2", "path2"}
    };

    int32_t result = storageManagerProviderTest_->MountDisShareFile(USER_ID, shareFiles);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderMockTest_MountDisShareFile_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountDisShareFile_001
 * @tc.desc: Verify the UMountDisShareFile function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountDisShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDisShareFile_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string networkId = "sharefile1";
    auto ret = storageManagerProviderTest_->UMountDisShareFile(USER_ID, networkId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDisShareFile_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_001
 * @tc.desc: Verify the PrepareStartUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockGetTokenTypeFlag(1);

    auto ret = storageManagerProviderTest_->PrepareStartUser(100);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_StopUser_001
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto ret = storageManagerProviderTest_->StopUser(USER_ID);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_001
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_CompleteAddUser_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto ret = storageManagerProviderTest_->CompleteAddUser(USER_ID);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ActiveUserKey_001
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ActiveUserKey_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::vector<uint8_t> token = {1, 2, 3, 4};
    std::vector<uint8_t> secret = {5, 6, 7, 8};
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    auto ret = storageManagerProviderTest_->ActiveUserKey(USER_ID, token, secret);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ActiveUserKey_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_InactiveUserKey_001
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_InactiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_InactiveUserKey_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto ret = storageManagerProviderTest_->InactiveUserKey(USER_ID);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_InactiveUserKey_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserAuth_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint64_t secureUid = 123456789;
    std::vector<uint8_t> token = {1, 2, 3, 4};
    std::vector<uint8_t> oldSecret = {5, 6, 7, 8};
    std::vector<uint8_t> newSecret = {9, 10, 11, 12};
    auto ret = storageManagerProviderTest_->UpdateUserAuth(USER_ID, secureUid, token, oldSecret, newSecret);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateUserAuth_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateKeyContext_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    bool needRemoveTmpKey = true;
    auto ret = storageManagerProviderTest_->UpdateKeyContext(USER_ID, needRemoveTmpKey);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UpdateKeyContext_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GenerateAppkey_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    uint32_t hashId = 12345;
    std::string keyId;
    bool needReSet = false;
    auto ret = storageManagerProviderTest_->GenerateAppkey(hashId, USER_ID, keyId, needReSet);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GenerateAppkey_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_DeleteAppkey_001
 * @tc.desc: Verify the DeleteAppkey function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_DeleteAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteAppkey_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string keyId = "testKeyId";
    auto ret = storageManagerProviderTest_->DeleteAppkey(keyId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteAppkey_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetLockScreenStatus_001
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetLockScreenStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetLockScreenStatus_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    bool needActive = false;
    auto ret = storageManagerProviderTest_->GetLockScreenStatus(USER_ID, needActive);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetLockScreenStatus_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFileEncryptStatus_001
 * @tc.desc: Verify the GetFileEncryptStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFileEncryptStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFileEncryptStatus_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    bool isEncrypted = false;
    bool needCheckDirMount = true;
    auto ret = storageManagerProviderTest_->GetFileEncryptStatus(USER_ID, isEncrypted, needCheckDirMount);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFileEncryptStatus_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserNeedActiveStatus_001
 * @tc.desc: Verify the GetUserNeedActiveStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserNeedActiveStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserNeedActiveStatus_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    bool needActive = false;
    auto ret = storageManagerProviderTest_->GetUserNeedActiveStatus(USER_ID, needActive);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserNeedActiveStatus_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_EraseAllUserEncryptedKeys_001
 * @tc.desc: Verify the EraseAllUserEncryptedKeys function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_EraseAllUserEncryptedKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    g_returnUpdateService = true;
    auto ret = storageManagerProviderTest_->EraseAllUserEncryptedKeys();
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_EraseAllUserEncryptedKeys_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CreateUserDir_001
 * @tc.desc: Verify the CreateUserDir function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CreateUserDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateUserDir_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    g_returnUpdateService = false;
    SetCallingUid(AOCO_UID);
    auto ret = storageManagerProviderTest_->CreateUserDir("", 0, 0, 0);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateUserDir_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    std::string dirPath = "/test";
    uint32_t type = 2;
    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(USER_ID, dirPath, type);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountDfsDocs_001
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDfsDocs_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    std::string relativePath = "/mnt/dfs/network123/device123/relative/path";
    std::string networkId = "network123";
    std::string deviceId = "device123";
    auto ret = storageManagerProviderTest_->MountDfsDocs(USER_ID, relativePath, networkId, deviceId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountDfsDocs_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountDfsDocs_001
 * @tc.desc: Verify the UMountDfsDocs function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDfsDocs_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string relativePath = "/mnt/dfs/network123/device123/relative/path";
    std::string networkId = "network123";
    std::string deviceId = "device123";
    auto ret = storageManagerProviderTest_->UMountDfsDocs(USER_ID, relativePath, networkId, deviceId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountDfsDocs_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountMediaFuse_001
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountMediaFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountMediaFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    g_returnBundleName = 2;
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    int32_t devFd = -1;
    auto ret = storageManagerProviderTest_->MountMediaFuse(USER_ID, devFd);
    EXPECT_NE(ret, E_OK);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountMediaFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountMediaFuse_001
 * @tc.desc: Verify the UMountMediaFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountMediaFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountMediaFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    auto ret = storageManagerProviderTest_->UMountMediaFuse(USER_ID);
    EXPECT_NE(ret, E_OK);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountMediaFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CreateShareFile_001
 * @tc.desc: Verify the CreateShareFile function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CreateShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateShareFile_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(FOUNDATION_UID);
    std::string uriStr = "file1";
    std::vector<std::string> uriStrVec = {uriStr};
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 12345;
    uint32_t flag = 1;
    std::vector<int32_t> funcResult;
    auto ret = storageManagerProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CreateShareFile_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_DeleteShareFile_001
 * @tc.desc: Verify the DeleteShareFile function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_DeleteShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteShareFile_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string uriStr = "file1";
    std::vector<std::string> uriStrVec = {uriStr};
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 12345;
    auto ret = storageManagerProviderTest_->DeleteShareFile(tokenId, fileRawData);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_DeleteShareFile_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_MountFileMgrFuse_001
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_MountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    std::string path = "/mnt/data/100/userExternal/1702";
    int32_t fuseFd = -1;
    g_returnBundleName = 0;
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();

    auto ret = storageManagerProviderTest_->MountFileMgrFuse(USER_ID, path, fuseFd);
    EXPECT_NE(ret, E_OK);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_MountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UMountFileMgrFuse_001
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UMountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    std::string path = "/mnt/data/100/userExternal/1702";

    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();

    auto ret = storageManagerProviderTest_->UMountFileMgrFuse(USER_ID, path);
    EXPECT_NE(ret, E_OK);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UMountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RegisterUeceActivationCallback_001
 * @tc.desc: Verify the RegisterUeceActivationCallback function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RegisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RegisterUeceActivationCallback_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    sptr<IUeceActivationCallback> ueceCallback(new UeceActivationCallbackMock());
    EXPECT_NE(ueceCallback, nullptr);
    auto ret = storageManagerProviderTest_->RegisterUeceActivationCallback(ueceCallback);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RegisterUeceActivationCallback_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UnregisterUeceActivationCallback_001
 * @tc.desc: Verify the UnregisterUeceActivationCallback function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UnregisterUeceActivationCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnRegisterUeceActivationCallback_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    auto ret = storageManagerProviderTest_->UnregisterUeceActivationCallback();
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnRegisterUeceActivationCallback_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_LockUserScreen_001
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_LockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_LockUserScreen_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    g_returnBundleName = 1;
    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();

    auto ret = storageManagerProviderTest_->LockUserScreen(USER_ID);
    EXPECT_NE(ret, E_OK);
    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_LockUserScreen_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_UnlockUserScreen_001
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_UnlockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnlockUserScreen_001 start";
    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    std::vector<uint8_t> token = {0x01, 0x02, 0x03};
    std::vector<uint8_t> secret = {0x04, 0x05, 0x06};
    auto ret = storageManagerProviderTest_->UnlockUserScreen(USER_ID, token, secret);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_UnlockUserScreen_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareAddUser_001
 * @tc.desc: Verify the PrepareAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareAddUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    uint32_t flags = 100;

    auto ret = storageManagerProviderTest_->PrepareAddUser(USER_ID, flags);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareAddUser_002
 * @tc.desc: Verify the PrepareAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareAddUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    uint32_t flags = 3;

    auto ret = storageManagerProviderTest_->PrepareAddUser(MINUSERID, flags);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareAddUser_003
 * @tc.desc: Verify the PrepareAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareAddUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    uint32_t flags = 1;

    auto ret = storageManagerProviderTest_->PrepareAddUser(MAXUSERID, flags);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareAddUser_004
 * @tc.desc: Verify the PrepareAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareAddUser_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    uint32_t flags = 1;

    auto ret = storageManagerProviderTest_->PrepareAddUser(USER_ID, flags);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareAddUser_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RemoveUser_001
 * @tc.desc: Verify the RemoveUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RemoveUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    const uint32_t flag = 0;

    auto ret = storageManagerProviderTest_->RemoveUser(USER_ID, flag);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RemoveUser_002
 * @tc.desc: Verify the RemoveUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RemoveUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    const uint32_t flag = 3;

    auto ret = storageManagerProviderTest_->RemoveUser(MINUSERID, flag);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RemoveUser_003
 * @tc.desc: Verify the RemoveUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RemoveUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    const uint32_t flag = 10;

    auto ret = storageManagerProviderTest_->RemoveUser(MAXUSERID, flag);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_RemoveUser_004
 * @tc.desc: Verify the RemoveUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_RemoveUser_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    const uint32_t flag = 1;

    auto ret = storageManagerProviderTest_->RemoveUser(USER_ID, flag);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_RemoveUser_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_002
 * @tc.desc: Verify the PrepareStartUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);

    auto ret = storageManagerProviderTest_->PrepareStartUser(USER_ID);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_003
 * @tc.desc: Verify the PrepareStartUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);

    auto ret = storageManagerProviderTest_->PrepareStartUser(MINUSERID);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_004
 * @tc.desc: Verify the PrepareStartUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);

    auto ret = storageManagerProviderTest_->PrepareStartUser(MAXUSERID);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_PrepareStartUser_005
 * @tc.desc: Verify the PrepareStartUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_PrepareStartUser_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_005 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);

    auto ret = storageManagerProviderTest_->PrepareStartUser(USER_ID);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_PrepareStartUser_005 end";
}

/**
 * @tc.name: StorageManagerProviderTest_StopUser_002
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    MockVerifyAccessToken(-1);
    SetCallingUid(DFS_UID);

    auto ret = storageManagerProviderTest_->StopUser(USER_ID);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_StopUser_003
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);

    auto ret = storageManagerProviderTest_->StopUser(MINUSERID);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_StopUser_004
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);

    auto ret = storageManagerProviderTest_->StopUser(MAXUSERID);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_StopUser_005
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_StopUser_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_005 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);

    auto ret = storageManagerProviderTest_->StopUser(USER_ID);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_StopUser_005 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_002
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_CompleteAddUser_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    MockVerifyAccessToken(-1);
    SetCallingUid(DFS_UID);

    auto ret = storageManagerProviderTest_->CompleteAddUser(USER_ID);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_003
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_CompleteAddUser_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);

    auto ret = storageManagerProviderTest_->CompleteAddUser(MINUSERID);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_004
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_CompleteAddUser_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);

    auto ret = storageManagerProviderTest_->CompleteAddUser(MAXUSERID);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_CompleteAddUser_005
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_CompleteAddUser_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_CompleteAddUser_005 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);

    auto ret = storageManagerProviderTest_->CompleteAddUser(USER_ID);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_CompleteAddUser_005 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFreeSizeOfVolume_001
 * @tc.desc: Verify the GetFreeSizeOfVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFreeSizeOfVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_GetFreeSizeOfVolume_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    MockVerifyAccessToken(-1);
    SetCallingUid(DFS_UID);
    std::string volumeUuid = "volume-1702";
    int64_t freeSize = 0;

    auto ret = storageManagerProviderTest_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetFreeSizeOfVolume_002
 * @tc.desc: Verify the GetFreeSizeOfVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetFreeSizeOfVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storageManagerProviderTest_GetFreeSizeOfVolume_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    MockVerifyAccessToken(0);
    SetCallingUid(ACCOUNT_UID);
    std::string volumeUuid = "volume-1702";
    int64_t freeSize = 0;

    auto ret = storageManagerProviderTest_->GetFreeSizeOfVolume(volumeUuid, freeSize);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetFreeSizeOfVolume_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetTotalSizeOfVolume_001
 * @tc.desc: Verify the GetTotalSizeOfVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetTotalSizeOfVolume_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    const std::string volumeUuid = "volume-1702";
    int64_t totalSize = 0;

    auto ret = storageManagerProviderTest_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetTotalSizeOfVolume_002
 * @tc.desc: Verify the GetTotalSizeOfVolume function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetTotalSizeOfVolume_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    const std::string volumeUuid = "volume-1702";
    int64_t totalSize = 0;

    auto ret = storageManagerProviderTest_->GetTotalSizeOfVolume(volumeUuid, totalSize);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetTotalSizeOfVolume_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetBundleStats_001
 * @tc.desc: Verify the GetBundleStats function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetBundleStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string pkgName = "com.example.bundle";
    BundleStats bundleStats;
    int32_t appIndex = 1;
    uint32_t statFlag = 0x02;

    auto ret = storageManagerProviderTest_->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetBundleStats_002
 * @tc.desc: Verify the GetBundleStats function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetBundleStats_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    std::string pkgName = "com.example.bundle";
    BundleStats bundleStats;
    int32_t appIndex = 2;
    uint32_t statFlag = 0x03;

    auto ret = storageManagerProviderTest_->GetBundleStats(pkgName, bundleStats, appIndex, statFlag);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetBundleStats_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ListUserdataDirInfo_001
 * @tc.desc: Verify the ListUserdataDirInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ListUserdataDirInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ListUserdataDirInfo_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    std::vector<UserdataDirInfo> scanDirs;

    auto ret = storageManagerProviderTest_->ListUserdataDirInfo(scanDirs);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ListUserdataDirInfo_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_ListUserdataDirInfo_002
 * @tc.desc: Verify the ListUserdataDirInfo function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_ListUserdataDirInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ListUserdataDirInfo_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    std::vector<UserdataDirInfo> scanDirs;

    auto ret = storageManagerProviderTest_->ListUserdataDirInfo(scanDirs);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_ListUserdataDirInfo_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_002
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string dirPath = "/test/1702";
    uint32_t type = 1;

    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(USER_ID, dirPath, type);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_003
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string dirPath = "../test/1702";
    uint32_t type = 1;

    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(USER_ID, dirPath, type);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_004
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string dirPath = "/test/1702/1403";
    uint32_t type = 1;

    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(MINUSERID, dirPath, type);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_005
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_005 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string dirPath = "/test/mtp/external";
    uint32_t type = 1;

    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(MAXUSERID, dirPath, type);
    EXPECT_EQ(ret, E_USERID_RANGE);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_005 end";
}

/**
 * @tc.name: StorageManagerProviderTest_SetDirEncryptionPolicy_006
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_SetDirEncryptionPolicy_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_006 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string dirPath = "/test/mtp/external";
    uint32_t type = 1;

    auto ret = storageManagerProviderTest_->SetDirEncryptionPolicy(USER_ID, dirPath, type);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_SetDirEncryptionPolicy_006 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemSize_001
 * @tc.desc: Verify the GetSystemSize function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    int64_t systemSize = 0;

    auto ret = storageManagerProviderTest_->GetSystemSize(systemSize);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemSize_002
 * @tc.desc: Verify the GetSystemSize function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    int64_t systemSize = 0;

    auto ret = storageManagerProviderTest_->GetSystemSize(systemSize);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemSize_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemDataSize_001
 * @tc.desc: Verify the GetSystemDataSize function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemDataSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemDataSize_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    int64_t systemDataSize = 200;

    auto ret = storageManagerProviderTest_->GetSystemDataSize(systemDataSize);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemDataSize_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetSystemDataSize_002
 * @tc.desc: Verify the GetSystemDataSize function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetSystemDataSize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemDataSize_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    int64_t systemDataSize = 100;

    auto ret = storageManagerProviderTest_->GetSystemDataSize(systemDataSize);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetSystemDataSize_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStats_001
 * @tc.desc: Verify the GetUserStorageStats function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStats_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    StorageStats storageStats;

    auto ret = storageManagerProviderTest_->GetUserStorageStats(storageStats);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStats_002
 * @tc.desc: Verify the GetUserStorageStats function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStats_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    StorageStats storageStats;

    auto ret = storageManagerProviderTest_->GetUserStorageStats(storageStats);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStats_003
 * @tc.desc: Verify the GetUserStorageStats function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStats_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    StorageStats storageStats;

    auto ret = storageManagerProviderTest_->GetUserStorageStats(USER_ID, storageStats);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_GetUserStorageStats_004
 * @tc.desc: Verify the GetUserStorageStats function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_GetUserStorageStats_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    StorageStats storageStats;

    auto ret = storageManagerProviderTest_->GetUserStorageStats(USER_ID, storageStats);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_GetUserStorageStats_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeCreated_001
 * @tc.desc: Verify the NotifyVolumeCreated function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeCreated_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    VolumeCore volumeCore;

    auto ret = storageManagerProviderTest_->NotifyVolumeCreated(volumeCore);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeCreated_002
 * @tc.desc: Verify the NotifyVolumeCreated function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeCreated_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    VolumeCore volumeCore;

    auto ret = storageManagerProviderTest_->NotifyVolumeCreated(volumeCore);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeCreated_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeMounted_001
 * @tc.desc: Verify the NotifyVolumeMounted function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeMounted_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volumeId = "volumeId";
    std::string fsTypeStr = "fsTypeStr";
    std::string fsUuid = "fsUuid";
    std::string path = "/mnt/mtp/external";
    std::string description = "description";

    auto ret = storageManagerProviderTest_->NotifyVolumeMounted(
        VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, false});
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeMounted_002
 * @tc.desc: Verify the NotifyVolumeMounted function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeMounted_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volumeId = "volumeId";
    std::string fsTypeStr = "fsTypeStr";
    std::string fsUuid = "fsUuid";
    std::string path = "/mnt/mtp/external";
    std::string description = "description";

    auto ret = storageManagerProviderTest_->NotifyVolumeMounted(
        VolumeInfoStr{volumeId, fsTypeStr, fsUuid, path, description, false});
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeMounted_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_001
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volId = "vol-1-2";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-123456";
    std::string path = "/TEST/mtp";
    std::string description = "description";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_002
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volId = "vol-1-2-3";
    std::string fsTypeStr = "ntfs";
    std::string uuid = "uuid-11111";
    std::string path = "/external/mtp";
    std::string description = "description123";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_003
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_003 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volId = "vol-8-1-9-2-3";
    std::string fsTypeStr = "exfat";
    std::string uuid = "uuid-234";
    std::string path = "/exfat";
    std::string description = "exfatdescription";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_003 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_004
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_004 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volId = "vol-8-1-9-2-3";
    std::string fsTypeStr = "exfat";
    std::string uuid = "uuid-234";
    std::string path = "/exfat";
    std::string description = "exfatdescription";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_004 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_005
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_005 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volId = "vol-1-2";
    std::string fsTypeStr = "vfat";
    std::string uuid = "uuid";
    std::string path = "/vfat";
    std::string description = "vfatdescription";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_005 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeDamaged_006
 * @tc.desc: Verify the NotifyVolumeDamaged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeDamaged_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_006 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volId = "vol-1-2-3-4";
    std::string fsTypeStr = "vfat";
    std::string uuid = "uuid-vfat";
    std::string path = "/vfat/";
    std::string description = "vfatdescription";

    auto ret = storageManagerProviderTest_->NotifyVolumeDamaged(
        VolumeInfoStr{volId, fsTypeStr, uuid, path, description, true});
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeDamaged_006 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeStateChanged_001
 * @tc.desc: Verify the NotifyVolumeStateChanged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeStateChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volumeId = "volumeId";

    uint32_t state = MOUNTED;
    auto ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    state = ENCRYPTING;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    state = ENCRYPTED_AND_LOCKED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    state = ENCRYPTED_AND_UNLOCKED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    state = DECRYPTING;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_NotifyVolumeStateChanged_002
 * @tc.desc: Verify the NotifyVolumeStateChanged function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_NotifyVolumeStateChanged_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volumeId = "volumeId-123";

    uint32_t state = MOUNTED;
    auto ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);

    state = ENCRYPTING;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);

    state = ENCRYPTED_AND_LOCKED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);

    state = ENCRYPTED_AND_UNLOCKED;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);

    state = DECRYPTING;
    ret = storageManagerProviderTest_->NotifyVolumeStateChanged(volumeId, state);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_NotifyVolumeStateChanged_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Mount_001
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volumeId = "volumeId";

    auto ret = storageManagerProviderTest_->Mount(volumeId);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Mount_002
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Mount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volumeId = "volumeId";

    auto ret = storageManagerProviderTest_->Mount(volumeId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Mount_002 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Unmount_001
 * @tc.desc: Verify the Unmount function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Unmount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_001 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(DFS_UID);
    MockVerifyAccessToken(-1);
    std::string volumeId = "volumeId";

    auto ret = storageManagerProviderTest_->Unmount(volumeId);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_001 end";
}

/**
 * @tc.name: StorageManagerProviderTest_Unmount_002
 * @tc.desc: Verify the Unmount function.
 * @tc.type: FUNC
 */
HWTEST_F(StorageManagerProviderTest, StorageManagerProviderTest_Unmount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_002 start";

    ASSERT_TRUE(storageManagerProviderTest_ != nullptr);
    SetCallingUid(ACCOUNT_UID);
    MockVerifyAccessToken(0);
    std::string volumeId = "volumeId";

    auto ret = storageManagerProviderTest_->Unmount(volumeId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageManagerProviderTest_Unmount_002 end";
}
} // namespace StorageManager
} // namespace OHOS