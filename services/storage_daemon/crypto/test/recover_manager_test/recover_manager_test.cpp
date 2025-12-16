/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "recover_manager.h"

#include <gtest/gtest.h>
#include <memory>

#include "key_control_mock.h"
#include "storage_service_errno.h"
#include "securec.h"

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
#include "tee_client_api.h"
namespace {
    TEEC_Result g_initializeContext;
    TEEC_Result g_invokeCommand;
    int g_memcpyRet;
}

int memcpy_s(void *dest, size_t destMax, const void *src, size_t count)
{
    return g_memcpyRet;
}

TEEC_Result TEEC_InitializeContext(const char *name, TEEC_Context *context)
{
    return g_initializeContext;
}

TEEC_Result TEEC_InvokeCommand(TEEC_Session *session, uint32_t commandID, TEEC_Operation *operation,
                               uint32_t *returnOrigin)
{
    return g_invokeCommand;
}
#endif

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS::StorageDaemon {
class RecoveryManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
};

void RecoveryManagerTest::SetUp(void)
{
    // input testcase setup step, setup invoked before each testcases
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
}

void RecoveryManagerTest::TearDown(void)
{
    // input testcase teardown step, teardown invoked after each testcases
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
}

/**
 * @tc.name: RecoveryManager_IsEncryptionEnabled_001
 * @tc.desc: Verify the IsEncryptionEnabled function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_IsEncryptionEnabled_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_IsEncryptionEnabled_001 start";
    g_initializeContext = static_cast<TEEC_ReturnCode>(1);
    EXPECT_TRUE(RecoveryManager::GetInstance().IsEncryptionEnabled());

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = static_cast<TEEC_ReturnCode>(1);
    EXPECT_TRUE(RecoveryManager::GetInstance().IsEncryptionEnabled());

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = TEEC_SUCCESS;
    EXPECT_TRUE(RecoveryManager::GetInstance().IsEncryptionEnabled());
    GTEST_LOG_(INFO) << "RecoveryManager_IsEncryptionEnabled_001 end";
}

/**
 * @tc.name: RecoveryManager_ResetSecretWithRecoveryKeyToTee_001
 * @tc.desc: Verify the ResetSecretWithRecoveryKeyToTee function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_ResetSecretWithRecoveryKeyToTee_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_InRecoveryManager_ResetSecretWithRecoveryKeyToTee_001stallDeCe_001 start";

    g_initializeContext = static_cast<TEEC_ReturnCode>(1);
    SetRecoverKeyStr setRecoverKeyStr;
    auto ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKeyToTee(1, 1, {}, setRecoverKeyStr);
    EXPECT_EQ(ret, E_RECOVERY_KEY_OPEN_SESSION_ERR);

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = static_cast<TEEC_ReturnCode>(1);
    ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKeyToTee(1, 1, {}, setRecoverKeyStr);
    EXPECT_EQ(ret, E_TEEC_SET_RK_FOR_PLUGGED_IN_SSD_ERR);

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = TEEC_SUCCESS;
    ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKeyToTee(1, 1, {}, setRecoverKeyStr);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "RecoveryManager_ResetSecretWithRecoveryKeyToTee_001 end";
}

/**
 * @tc.name: RecoveryManager_ResetSecretWithRecoveryKey_001
 * @tc.desc: Verify the ResetSecretWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_ResetSecretWithRecoveryKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_ResetSecretWithRecoveryKey_001 start";
    std::vector<KeyBlob> originIvs;

    g_initializeContext = static_cast<TEEC_ReturnCode>(1);
    auto ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKey(1, 1, {}, originIvs);
    EXPECT_EQ(ret, E_RECOVERY_KEY_OPEN_SESSION_ERR);

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = TEEC_SUCCESS;

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).Times(6).WillRepeatedly(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeyEx(_, _, _, _)).Times(12).WillRepeatedly(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeySdp(_, _, _, _)).Times(6).WillRepeatedly(Return(0));
    ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKey(1, 1, {}, originIvs);
    EXPECT_EQ(ret, E_OK);

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = TEEC_SUCCESS;

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(-1));

    ret = RecoveryManager::GetInstance().ResetSecretWithRecoveryKey(1, 1, {}, originIvs);
    EXPECT_EQ(ret, E_ADD_SESSION_KEYRING_ERROR);

    GTEST_LOG_(INFO) << "RecoveryManager_ResetSecretWithRecoveryKey_001 end";
}

/**
 * @tc.name: RecoveryManager_InstallDeCe_001
 * @tc.desc: Verify the InstallDeCe function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_InstallDeCe_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_InstallDeCe_001 start";
    std::vector<uint8_t> keyDescVct(3, 2);
    KeyBlob key2BlobErr;
    KeyBlob keyDesc(keyDescVct);
    g_memcpyRet = -1;
    int ret = RecoveryManager::GetInstance().InstallDeCe(key2BlobErr, keyDesc);
    EXPECT_NE(0, ret);
    g_memcpyRet = 0;

    std::vector<uint8_t> key2BlobVct(5, 1);
    KeyBlob key2Blob(key2BlobVct);
    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(-1));
    ret = RecoveryManager::GetInstance().InstallDeCe(key2BlobErr, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeyEx(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallDeCe(key2Blob, keyDesc);
    EXPECT_EQ(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeyEx(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallDeCe(key2Blob, keyDesc);
    EXPECT_EQ(0, ret);
    GTEST_LOG_(INFO) << "RecoveryManager_InstallDeCe_001 end";
}

/**
 * @tc.name: RecoveryManager_InstallEceSece_001
 * @tc.desc: Verify the InstallEceSece function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_InstallEceSece_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_InstallEceSece_001 start";
    uint32_t sdpClass = 0;
    std::vector<uint8_t> keyDescVct(3, 2);
    KeyBlob key2BlobErr;
    KeyBlob keyDesc(keyDescVct);
    int ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2BlobErr, keyDesc);
    EXPECT_NE(0, ret);
    
    EncryptionKeySdp fskey;
    KeyBlob key2Blob;
    key2Blob.Alloc(sizeof(fskey.raw));
    ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2BlobErr, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeySdp(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2Blob, keyDesc);
    EXPECT_EQ(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeySdp(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2Blob, keyDesc);
    EXPECT_EQ(0, ret);
    GTEST_LOG_(INFO) << "RecoveryManager_InstallEceSece_001 end";
}

/**
 * @tc.name: RecoveryManager_InstallKeyDescToKeyring_001
 * @tc.desc: Verify the InstallKeyDescToKeyring function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_InstallKeyDescToKeyring_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_InstallKeyDescToKeyring_001 start";
    EncryptionKeySdp fskey;
    KeyBlob key2Blob;
    key2Blob.Alloc(sizeof(fskey.raw));
    std::vector<uint8_t> keyDescVct(3, 2);
    KeyBlob keyDesc(keyDescVct);
    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(-1));
    int ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL3, key2Blob, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(-1));
    ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL3, key2Blob, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeySdp(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL3, key2Blob, keyDesc);
    EXPECT_EQ(0, ret);
    GTEST_LOG_(INFO) << "RecoveryManager_InstallKeyDescToKeyring_001 end";
}

/**
 * @tc.name: RecoveryManager_InstallKeyDescToKeyring_002
 * @tc.desc: Verify the InstallKeyDescToKeyring function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_InstallKeyDescToKeyring_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_InstallKeyDescToKeyring_002 start";
    std::vector<uint8_t> keyDescVct(3, 2);
    KeyBlob keyDesc(keyDescVct);
    std::vector<uint8_t> key2BlobVct(5, 1);
    KeyBlob key2Blob(key2BlobVct);
    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(-1));
    int ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL2, key2Blob, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeyEx(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL2, key2Blob, keyDesc);
    EXPECT_EQ(0, ret);
    GTEST_LOG_(INFO) << "RecoveryManager_InstallKeyDescToKeyring_002 end";
}

/**
 * @tc.name: RecoveryManager_GenerateKeyDesc_001
 * @tc.desc: Verify the GenerateKeyDesc function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_GenerateKeyDesc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_GenerateKeyDesc_001 start";
    KeyBlob ivBlobErr;
    KeyBlob keyDesc;
    int ret = RecoveryManager::GetInstance().GenerateKeyDesc(ivBlobErr, keyDesc);
    EXPECT_NE(0, ret);

    std::vector<uint8_t> ivBlobVct(5, 1);
    KeyBlob ivBlob(ivBlobVct);
    ret = RecoveryManager::GetInstance().GenerateKeyDesc(ivBlob, keyDesc);
    EXPECT_EQ(0, ret);
    GTEST_LOG_(INFO) << "RecoveryManager_GenerateKeyDesc_001 end";
}

/**
 * @tc.name: RecoveryManager_CreateRecoverKey_001
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_CreateRecoverKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_CreateRecoverKey_001 start";

    g_initializeContext = static_cast<TEEC_ReturnCode>(1);
    auto ret = RecoveryManager::GetInstance().CreateRecoverKey(10, 1, {}, {}, {});
    EXPECT_EQ(ret, E_RECOVERY_KEY_OPEN_SESSION_ERR);

    RecoveryManager::GetInstance().isSessionOpened = true;
    g_memcpyRet = -1;
    std::vector<KeyBlob> originIvs(5, {'1'});
    std::vector<uint8_t> token(2, 0);
    ret = RecoveryManager::GetInstance().CreateRecoverKey(10, 1, token, {}, originIvs);
    EXPECT_EQ(ret, E_MEMORY_OPERATION_ERR);

    g_memcpyRet = 0;
    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = static_cast<TEEC_ReturnCode>(1);
    ret = RecoveryManager::GetInstance().CreateRecoverKey(10, 1, {}, {}, originIvs);
    EXPECT_EQ(ret, E_TEEC_GEN_RECOVERY_KEY_ERR);

    g_memcpyRet = -1;
    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = static_cast<TEEC_ReturnCode>(1);
    ret = RecoveryManager::GetInstance().CreateRecoverKey(10, 1, {}, {}, originIvs);
    EXPECT_EQ(ret, E_MEMORY_OPERATION_ERR);

    g_memcpyRet = 0;
    RecoveryManager::GetInstance().isSessionOpened = true;
    g_invokeCommand = TEEC_SUCCESS;
    ret = RecoveryManager::GetInstance().CreateRecoverKey(10, 1, {'!'}, {'!'}, originIvs);
    EXPECT_EQ(ret, TEEC_SUCCESS);

    GTEST_LOG_(INFO) << "RecoveryManager_CreateRecoverKey_001 end";
}

/**
 * @tc.name: RecoveryManager_SetRecoverKeyToTee_001
 * @tc.desc: Verify the SetRecoverKeyToTee function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(RecoveryManagerTest, RecoveryManager_SetRecoverKeyToTee_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecoveryManager_SetRecoverKeyToTee_001 start";
    std::vector<uint8_t> key(5, {'1'});
    SetRecoverKeyStr keyStr;
    g_initializeContext = static_cast<TEEC_ReturnCode>(1);
    auto ret = RecoveryManager::GetInstance().SetRecoverKeyToTee(key, keyStr);
    EXPECT_EQ(ret, E_RECOVERY_KEY_OPEN_SESSION_ERR);

    g_invokeCommand = static_cast<TEEC_ReturnCode>(1);
    RecoveryManager::GetInstance().isSessionOpened = true;
    ret = RecoveryManager::GetInstance().SetRecoverKeyToTee(key, keyStr);
    EXPECT_EQ(ret, E_TEEC_DECRYPT_CLASS_KEY_ERR);
    
    g_invokeCommand = TEEC_SUCCESS;
    RecoveryManager::GetInstance().isSessionOpened = true;
    ret = RecoveryManager::GetInstance().SetRecoverKeyToTee(key, keyStr);
    EXPECT_EQ(ret, TEEC_SUCCESS);

    GTEST_LOG_(INFO) << "RecoveryManager_SetRecoverKeyToTee_001 end";
}
} // OHOS::StorageDaemon
