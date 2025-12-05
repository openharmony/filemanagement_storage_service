/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "parameters.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "user/multi_user_manager_service.h"

namespace {
using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace StorageManager;
const string FSCRYPT_POLICY_KEY = "fscrypt.policy.config";
bool g_fscryptEnable = false;
class StorageDaemonCommunicationTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::string res = system::GetParameter(FSCRYPT_POLICY_KEY, "");
        if (!res.empty()) {
            g_fscryptEnable = true;
        }
    }
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_Connect_0000
 * @tc.name: Daemon_communication_Connect_0000
 * @tc.desc: Test function of Connect interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Connect_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Connect_0000";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    if (sdCommunication != nullptr) {
        result = sdCommunication->Connect();
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Connect_0000";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_SetDirEncryptionPolicy_0000
 * @tc.name: Daemon_communication_SetDirEncryptionPolicy_0000
 * @tc.desc: Test function of SetDirEncryptionPolicy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_SetDirEncryptionPolicy_0000,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_SetDirEncryptionPolicy_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    std::string dirPath = "/data/service/test";
    uint32_t userId = 100;
    uint32_t type = 2;
    int32_t result = sdCommunication->SetDirEncryptionPolicy(userId, dirPath, type);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_SetDirEncryptionPolicy_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_PrepareAddUser_0000
 * @tc.name: Daemon_communication_PrepareAddUser_0000
 * @tc.desc: Test function of PrepareAddUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_PrepareAddUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_PrepareAddUser_0000";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t userId = 121;
    uint32_t flag = 3;
    int32_t result = -1;
    if (sdCommunication != nullptr) {
        result = sdCommunication->PrepareAddUser(userId, flag);
    }
    EXPECT_EQ(result, E_OK);
    sdCommunication->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_PrepareAddUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_RemoveUser_0000
 * @tc.name: Daemon_communication_RemoveUser_0000
 * @tc.desc: Test function of RemoveUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_RemoveUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_RemoveUser_0000";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t userId = 102;
    uint32_t flag = 3;
    int32_t result = -1;
    if (sdCommunication != nullptr) {
        sdCommunication->PrepareAddUser(userId, flag);
        result = sdCommunication->RemoveUser(userId, flag);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_RemoveUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_PrepareStartUser_0000
 * @tc.name: Daemon_communication_PrepareStartUser_0000
 * @tc.desc: Test function of PrepareStartUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_PrepareStartUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_PrepareStartUser_0000";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t userId = 104;
    uint32_t flag = 3;
    int32_t result = -1;
    if (sdCommunication != nullptr) {
        sdCommunication->PrepareAddUser(userId, flag);
        result = sdCommunication->PrepareStartUser(userId);
    }
    EXPECT_EQ(result, E_OK);
    sdCommunication->StopUser(userId);
    sdCommunication->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_PrepareStartUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_StopUser_0000
 * @tc.name: Daemon_communication_StopUser_0000
 * @tc.desc: Test function of StopUser interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_StopUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_StopUser_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t userId = 106;
    uint32_t flag = 3;
    int32_t result = -1;
    if (sdCommunication != nullptr) {
        sdCommunication->PrepareAddUser(userId, flag);
        sdCommunication->PrepareStartUser(userId);
        result = sdCommunication->StopUser(userId);
    }
    EXPECT_EQ(result, E_OK);
    sdCommunication->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_StopUser_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_MountDfsDocs_001
 * @tc.name: Daemon_communication_MountDfsDocs_001
 * @tc.desc: Test function of MountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_MountDfsDocs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_MountDfsDocs_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    int32_t result = sdCommunication->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_MountDfsDocs_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UMountDfsDocs_001
 * @tc.name: Daemon_communication_UMountDfsDocs_001
 * @tc.desc: Test function of UMountDfsDocs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UMountDfsDocs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UMountDfsDocs_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    std::string relativePath = "account";
    std::string networkId = "testnetworkid";
    std::string deviceId = "testdevid";
    int32_t result = sdCommunication->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UMountDfsDocs_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_MountMediaFuse_001
 * @tc.name: Daemon_communication_MountMediaFuse_001
 * @tc.desc: Test function of MountMediaFuse interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_MountMediaFuse_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_MountMediaFuse_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t userId = 130;
    int32_t devFd = 140;
    int32_t result = sdCommunication->MountMediaFuse(userId, devFd);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_MountMediaFuse_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UMountMediaFus_001
 * @tc.name: Daemon_communication_UMountMediaFuse_001
 * @tc.desc: Test function of UMountMediaFuse interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UMountMediaFuse_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UMountMediaFuse_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t userId = 130;
    int32_t result = sdCommunication->UMountMediaFuse(userId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UMountMediaFuse_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_ResetSdProxy_001
 * @tc.name: Daemon_communication_ResetSdProxy_001
 * @tc.desc: Test function of ResetSdProxy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_ResetSdProxy_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_ResetSdProxy_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t ret = sdCommunication->ResetSdProxy();
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_ResetSdProxy_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_CreateShareFile_001
 * @tc.name: Daemon_communication_CreateShareFile_001
 * @tc.desc: Test function of CreateShareFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_CreateShareFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_CreateShareFile_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    uint32_t tokenId = 100;
    uint32_t flag = 3;
    StorageFileRawData uriList;
    uriList.ownedData = "testcreatefile";
    uriList.size = uriList.ownedData.size();
    uriList.data = uriList.ownedData.data();
    std::vector<int32_t> result = sdCommunication->CreateShareFile(uriList, tokenId, flag);
    ASSERT_TRUE(!result.empty());

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_CreateShareFile_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_DeleteShareFile_001
 * @tc.name: Daemon_communication_DeleteShareFile_001
 * @tc.desc: Test function of DeleteShareFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_DeleteShareFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_DeleteShareFile_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    StorageFileRawData uriList;
    uriList.ownedData = "testcreatefile";
    uriList.size = uriList.ownedData.size();
    uriList.data = uriList.ownedData.data();
    uint32_t tokenId = 100;
    int32_t ret = sdCommunication->DeleteShareFile(tokenId, uriList);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_DeleteShareFile_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_SetBundleQuota_001
 * @tc.name: Daemon_communication_SetBundleQuota_001
 * @tc.desc: Test function of SetBundleQuota interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_SetBundleQuota_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_SetBundleQuota_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t limitSizeMb = 100;
    int32_t uid = 100;
    std::string bundleDataDirPath = "testbundleDataDirPath";
    int32_t ret = sdCommunication->SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_SetBundleQuota_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_MountCryptoPathAgain_001
 * @tc.name: Daemon_communication_MountCryptoPathAgain_001
 * @tc.desc: Test function of MountCryptoPathAgain interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_MountCryptoPathAgain_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_MountCryptoPathAgain_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t uid = 1990;
    int32_t ret = sdCommunication->MountCryptoPathAgain(uid);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_MountCryptoPathAgain_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_GenerateAppkey_001
 * @tc.name: Daemon_communication_GenerateAppkey_001
 * @tc.desc: Test function of GenerateAppkey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_GenerateAppkey_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_GenerateAppkey_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t userId = 1999;
    int32_t appUid = 100;
    std::string keyId = "testkeyId";
    int32_t ret = sdCommunication->GenerateAppkey(userId, appUid, keyId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_GenerateAppkey_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_DeleteAppkey_001
 * @tc.name: Daemon_communication_DeleteAppkey_001
 * @tc.desc: Test function of DeleteAppkey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_DeleteAppkey_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_DeleteAppkey_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t userId = 1999;
    std::string keyId = "testkeyId";
    int32_t ret = sdCommunication->DeleteAppkey(userId, keyId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_DeleteAppkey_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UpdateMemoryPara_001
 * @tc.name: Daemon_communication_UpdateMemoryPara_001
 * @tc.desc: Test function of UpdateMemoryPara interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UpdateMemoryPara_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UpdateMemoryPara_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t size = 100;
    int32_t oldsize = 100;
    int32_t ret = sdCommunication->UpdateMemoryPara(size, oldsize);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UpdateMemoryPara_001 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_CompleteAddUser_000
 * @tc.name: Daemon_communication_CompleteAddUser_000
 * @tc.desc: Test function of CompleteAddUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_CompleteAddUser_000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_CompleteAddUser_000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    int32_t userId = 100;
    int32_t ret = sdCommunication->CompleteAddUser(userId);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_CompleteAddUser_000 SUCCESS";
}
#ifdef EXTERNAL_STORAGE_MANAGER
/**
 * @tc.number: SUB_STORAGE_Daemon_communication_Mount_0000
 * @tc.name: Daemon_communication_Mount_0000
 * @tc.desc: Test function of Mount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Mount_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Mount_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    std::string volumeId = "vol-2-1";
    int32_t flag = 1;
    if (sdCommunication != nullptr) {
        result = sdCommunication->Mount(volumeId, flag);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Mount_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_Unmount_0000
 * @tc.name: Daemon_communication_Unmount_0000
 * @tc.desc: Test function of Unmount interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Unmount_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Unmount_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    std::string volumeId = "vol-2-2";
    if (sdCommunication != nullptr) {
        result = sdCommunication->Unmount(volumeId);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Unmount_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_TryToFix_0000
 * @tc.name: Daemon_communication_TryToFix_0000
 * @tc.desc: Test function of TryToFix interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_TryToFix_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_TryToFix_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    std::string volumeId = "vol-2-1";
    int32_t flag = 1;
    if (sdCommunication != nullptr) {
        result = sdCommunication->TryToFix(volumeId, flag);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_TryToFix_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_Check_0000
 * @tc.name: Daemon_communication_Check_0000
 * @tc.desc: Test function of Check interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUOT
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Check_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Check_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    std::string volumeId = "vol-2-3";
    if (sdCommunication != nullptr) {
        result = sdCommunication->Check(volumeId);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Check_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_Partition_0000
 * @tc.name: Daemon_communication_Partition_0000
 * @tc.desc: Test function of Partition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGOUT
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Partition_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Partition_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    std::string diskId = "disk-2-4";
    int32_t type = 1;
    if (sdCommunication != nullptr) {
        result = sdCommunication->Partition(diskId, type);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Partition_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_Format_0000
 * @tc.name: Daemon_communication_Format_0000
 * @tc.desc: Test function of Format interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Format_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Format_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    string volumeId = "vol-2-5";
    string fsType = "fsType-1";
    if (sdCommunication != nullptr) {
        result = sdCommunication->Format(volumeId, fsType);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Format_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_SetVolumeDescription_0000
 * @tc.name: Daemon_communication_SetVolumeDescription_0000
 * @tc.desc: Test function of SetVolumeDescription interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_SetVolumeDescription_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_SetVolumeDescription_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    string fsUuid = "uuid-1";
    string description = "description-1";
    if (sdCommunication != nullptr) {
        result = sdCommunication->SetVolumeDescription(fsUuid, description);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_SetVolumeDescription_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_QueryUsbIsInUse_0000
 * @tc.name: Daemon_communication_QueryUsbIsInUse_0000
 * @tc.desc: Test function of QueryUsbIsInUse interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20250226995120
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_QueryUsbIsInUse_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_QueryUsbIsInUse_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    std::string diskPath = "/mnt/data/external/F573-04E1";
    int32_t result = -1;
    bool isInUse = true;
    if (sdCommunication != nullptr) {
        result = sdCommunication->QueryUsbIsInUse(diskPath, isInUse);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_QueryUsbIsInUse_0000 SUCCESS";
}
#endif

#ifdef USER_CRYPTO_MANAGER
/**
 * @tc.number: SUB_STORAGE_Daemon_communication_DeleteUserKeys_0000
 * @tc.name: Daemon_communication_DeleteUserKeys_0000
 * @tc.desc: Test function of DeleteUserKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_DeleteUserKeys_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_DeleteUserKeys_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    int32_t userId = 107;
    if (sdCommunication != nullptr) {
        result = sdCommunication->DeleteUserKeys(userId);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_DeleteUserKeys_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_EraseAllUserEncryptedKeys_0000
 * @tc.name: Daemon_communication_EraseAllUserEncryptedKeys_0000
 * @tc.desc: Test function of EraseAllUserEncryptedKeys interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_EraseAllUserEncryptedKeys_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_EraseAllUserEncryptedKeys_0000";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication != nullptr) {
        EXPECT_EQ(sdCommunication->EraseAllUserEncryptedKeys(), E_OK);
    }
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_EraseAllUserEncryptedKeys_0000";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UpdateUserAuth_0000
 * @tc.name: Daemon_communication_UpdateUserAuth_0000
 * @tc.desc: Test function of UpdateUserAuth interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UpdateUserAuth_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UpdateUserAuth_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    uint32_t userId = 108;
    if (sdCommunication != nullptr) {
        int32_t flags = 3;
        sdCommunication->RemoveUser(userId, flags);
        result = sdCommunication->PrepareAddUser(userId, flags);
        EXPECT_EQ(result, E_OK);
        result = sdCommunication->UpdateUserAuth(userId, 0, {}, {}, {});
        EXPECT_EQ(result, E_OK);
        result = sdCommunication->RemoveUser(userId, flags);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UpdateUserAuth_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_ActiveUserKey_0000
 * @tc.name: Daemon_communication_ActiveUserKey_0000
 * @tc.desc: Test function of ActiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_ActiveUserKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_ActiveUserKey_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    uint32_t userId = 109;
    if (sdCommunication != nullptr) {
        int32_t flags = 3;
        result = sdCommunication->PrepareAddUser(userId, flags);
        EXPECT_EQ(result, E_OK);
        result = sdCommunication->ActiveUserKey(userId, {}, {});
        EXPECT_EQ(result, E_OK);
        sdCommunication->RemoveUser(userId, flags);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_ActiveUserKey_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UpdateKeyContext_0000
 * @tc.name: Daemon_communication_UpdateKeyContext_0000
 * @tc.desc: Test function of UpdateKeyContext interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UpdateKeyContext_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UpdateKeyContext_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    uint32_t userId = 110;
    if (sdCommunication != nullptr) {
        int32_t flags = 3;
        result = sdCommunication->PrepareAddUser(userId, flags);
        EXPECT_EQ(result, E_OK);
        result = sdCommunication->UpdateKeyContext(userId);
        EXPECT_EQ(result, E_OK);
        sdCommunication->RemoveUser(userId, flags);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UpdateKeyContext_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_InactiveUserKey_0000
 * @tc.name: Daemon_communication_InactiveUserKey_0000
 * @tc.desc: Test function of InactiveUserKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_InactiveUserKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_InactiveUserKey_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = -1;
    uint32_t userId = 109;
    if (sdCommunication != nullptr) {
        int32_t flags = 3;
        result = sdCommunication->PrepareAddUser(userId, flags);
        EXPECT_EQ(result, E_OK);
        result = sdCommunication->ActiveUserKey(userId, {}, {});
        EXPECT_EQ(result, E_OK);
        result = sdCommunication->InactiveUserKey(userId);
        EXPECT_EQ(result, E_OK);
        sdCommunication->RemoveUser(userId, flags);
    }
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_InactiveUserKey_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_LockUserScreen_0000
 * @tc.name: Daemon_communication_LockUserScreen_0000
 * @tc.desc: Test function of LockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_LockUserScreen_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_LockUserScreen_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    int32_t result = sdCommunication->LockUserScreen(userId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_LockUserScreen_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UnlockUserScreen_0000
 * @tc.name: Daemon_communication_UnlockUserScreen_0000
 * @tc.desc: Test function of UnlockUserScreen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UnlockUserScreen_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UnlockUserScreen_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    int32_t result = sdCommunication->UnlockUserScreen(userId, {}, {});
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UnlockUserScreen_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_CreateRecoverKey_0000
 * @tc.name: Daemon_communication_CreateRecoverKey_0000
 * @tc.desc: Test function of CreateRecoverKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_CreateRecoverKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_CreateRecoverKey_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    uint32_t userType = 10;
    int32_t result = sdCommunication->CreateRecoverKey(userId, userType, {}, {});
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_CreateRecoverKey_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_SetRecoverKey_0000
 * @tc.name: Daemon_communication_SetRecoverKey_0000
 * @tc.desc: Test function of SetRecoverKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_SetRecoverKey_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_SetRecoverKey_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t result = sdCommunication->SetRecoverKey({});
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_SetRecoverKey_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_ResetSecretWithRecoveryKey_0000
 * @tc.name: Daemon_communication_ResetSecretWithRecoveryKey_0000
 * @tc.desc: Test function of ResetSecretWithRecoveryKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_ResetSecretWithRecoveryKey_0000,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin "
                        "Daemon_communication_ResetSecretWithRecoveryKey_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    uint32_t rkType = 100;
    int32_t result = sdCommunication->ResetSecretWithRecoveryKey(userId, rkType, {});
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end "
                        "Daemon_communication_ResetSecretWithRecoveryKey_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_GetLockScreenStatus_0000
 * @tc.name: Daemon_communication_GetLockScreenStatus_0000
 * @tc.desc: Test function of GetLockScreenStatus interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_GetLockScreenStatus_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_GetLockScreenStatus_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    bool lockScreenStatus = false;
    int32_t result = sdCommunication->GetLockScreenStatus(userId, lockScreenStatus);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_GetLockScreenStatus_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_InactiveUserPublicDirKey_0000
 * @tc.name: Daemon_communication_InactiveUserPublicDirKey_0000
 * @tc.desc: Test function of InactiveUserPublicDirKey interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20250418146433
 */
HWTEST_F(StorageDaemonCommunicationTest,
         Daemon_communication_InactiveUserPublicDirKey_0000,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "StorageDaemonCommunicationTest-begin Daemon_communication_InactiveUserPublicDirKey_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    int32_t result = sdCommunication->InactiveUserPublicDirKey(userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_InactiveUserPublicDirKey_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UpdateUserPublicDirPolicy_0000
 * @tc.name: Daemon_communication_UpdateUserPublicDirPolicy_0000
 * @tc.desc: Test function of UpdateUserPublicDirPolicy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR20250722463628
 */
HWTEST_F(StorageDaemonCommunicationTest,
         Daemon_communication_UpdateUserPublicDirPolicy_0000,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "StorageDaemonCommunicationTest-begin Daemon_communication_UpdateUserPublicDirPolicy_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    int32_t result = sdCommunication->UpdateUserPublicDirPolicy(userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO)
        << "StorageDaemonCommunicationTest-end Daemon_communication_UpdateUserPublicDirPolicy_0000 SUCCESS";
}
#endif

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_MountDisShareFile_0000
 * @tc.name: Daemon_communication_MountDisShareFile_0000
 * @tc.desc: Test function of MountDisShareFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_MountDisShareFile_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_MountDisShareFile_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    std::map<std::string, std::string> shareFiles = {{"/data/sharefile1", "/data/sharefile2"}};
    int32_t result = sdCommunication->MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_MountDisShareFile_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_UMountDisShareFile_0000
 * @tc.name: Daemon_communication_UMountDisShareFile_0000
 * @tc.desc: Test function of UMountDisShareFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000H0FG3
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_UMountDisShareFile_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_UMountDisShareFile_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    uint32_t userId = 100;
    std::string networkId = "sharefile1";
    int32_t result = sdCommunication->UMountDisShareFile(userId, networkId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_UMountDisShareFile_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_MountUsbFuse_0000
 * @tc.name: Daemon_communication_MountUsbFuse_0000
 * @tc.desc: Test function of MountUsbFuse interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_MountUsbFuse_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_MountUsbFuse_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    std::string volumeId = "vol-usb-001";
    int fuseFd = -1;
    std::string fsUuid;
    int32_t result = sdCommunication->MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_MountUsbFuse_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_IsFileOccupied_0000
 * @tc.name: Daemon_communication_IsFileOccupied_0000
 * @tc.desc: Test function of IsFileOccupied interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_IsFileOccupied_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_IsFileOccupied_0000 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    const std::string path;
    const std::vector<std::string> inputList = {"unrelated_process_1", "unrelated_process_2"};
    std::vector<std::string> outputList;
    bool status = true;
    EXPECT_EQ(sdCommunication->IsFileOccupied(path, inputList, outputList, status), E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_IsFileOccupied_0000 SUCCESS";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_GetDataSizeByPath_0000
 * @tc.name: Daemon_communication_GetDataSizeByPath_0000
 * @tc.desc: Test function of GetDataSizeByPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_GetDataSizeByPath_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_GetDataSizeByPath_0000";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();

    std::string path = "/data/test/file.txt";
    int64_t size = 0;
    int32_t result = -1;

    if (sdCommunication != nullptr) {
        result = sdCommunication->GetDataSizeByPath(path, size);
    }
    EXPECT_EQ(result, E_OK);
    EXPECT_GE(size, 0);

    path = "/data/storage_service";
    size = 0;
    result = -1;

    if (sdCommunication != nullptr) {
        result = sdCommunication->GetDataSizeByPath(path, size);
    }
    EXPECT_EQ(result, E_OK);
    EXPECT_GE(size, 0);

    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_GetDataSizeByPath_0000";
}

/**
 * @tc.number: SUB_STORAGE_Daemon_communication_ListUserdataDirInfo_001
 * @tc.name: Daemon_communication_ListUserdataDirInfo_001
 * @tc.desc: Test function of ListUserdataDirInfo interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_ListUserdataDirInfo_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_ListUserdataDirInfo_001 SUCCESS";
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
            DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    ASSERT_TRUE(sdCommunication != nullptr);

    std::vector<UserdataDirInfo> scanDirs;
    int32_t ret = sdCommunication->ListUserdataDirInfo(scanDirs);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_ListUserdataDirInfo_001 SUCCESS";
}
} // namespace
