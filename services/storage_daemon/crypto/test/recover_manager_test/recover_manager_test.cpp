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

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS::StorageDaemon {
class RecoveryManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
};

void RecoveryManagerTest::SetUpTestCase(void)
{
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
}

void RecoveryManagerTest::TearDownTestCase(void)
{
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
}

void RecoveryManagerTest::SetUp(void)
{
    // input testcase setup step, setup invoked before each testcases
}

void RecoveryManagerTest::TearDown(void)
{
    // input testcase teardown step, teardown invoked after each testcases
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
    int ret = RecoveryManager::GetInstance().InstallDeCe(key2BlobErr, keyDesc);
    EXPECT_NE(0, ret);

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
    ret = RecoveryManager::GetInstance().InstallDeCe(key2BlobErr, keyDesc);
    EXPECT_EQ(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeyEx(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallDeCe(key2BlobErr, keyDesc);
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
    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(-1));
    ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2BlobErr, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKey(_, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeySdp(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2BlobErr, keyDesc);
    EXPECT_EQ(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeySdp(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallEceSece(sdpClass, key2BlobErr, keyDesc);
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
    int ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL3, key2Blob, keyDesc);
    EXPECT_NE(0, ret);

    EXPECT_CALL(*keyControlMock_, KeyCtrlSearch(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*keyControlMock_, KeyCtrlAddKeyEx(_, _, _, _)).WillOnce(Return(0))
        .WillOnce(Return(-1)).WillOnce(Return(0));
    ret = RecoveryManager::GetInstance().InstallKeyDescToKeyring(TYPE_EL3, key2Blob, keyDesc);
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
} // OHOS::StorageDaemon