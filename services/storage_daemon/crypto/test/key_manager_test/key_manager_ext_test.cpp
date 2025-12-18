/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "key_manager_ext.h"

#include <fcntl.h>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "base_key_mock.h"
#include "directory_ex.h"
#include "fscrypt_control_mock.h"
#include "fscrypt_key_v2_mock.h"
#include "fscrypt_key_v2.h"
#include "key_control_mock.h"
#include "key_manager.h"
#include "storage_service_errno.h"
#include "user_key_ext_mock.h"
#include "utils/file_utils.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS::StorageDaemon {
class KeyManagerExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FscryptControlMoc> fscryptControlMock_ = nullptr;
    static inline shared_ptr<KeyControlMoc> keyControlMock_ = nullptr;
    static inline shared_ptr<BaseKeyMoc> baseKeyMock_ = nullptr;
    static inline shared_ptr<FscryptKeyV2Moc> fscryptKeyMock_ = nullptr;
    static inline shared_ptr<UserkeyExtMoc> userkeyExtMocMock_ = nullptr;
};

void KeyManagerExtTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void KeyManagerExtTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void KeyManagerExtTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    fscryptControlMock_ = make_shared<FscryptControlMoc>();
    FscryptControlMoc::fscryptControlMoc = fscryptControlMock_;
    keyControlMock_ = make_shared<KeyControlMoc>();
    KeyControlMoc::keyControlMoc = keyControlMock_;
    baseKeyMock_ = make_shared<BaseKeyMoc>();
    BaseKeyMoc::baseKeyMoc = baseKeyMock_;
    fscryptKeyMock_ = make_shared<FscryptKeyV2Moc>();
    FscryptKeyV2Moc::fscryptKeyV2Moc = fscryptKeyMock_;
    userkeyExtMocMock_ = make_shared<UserkeyExtMoc>();
}

void KeyManagerExtTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    FscryptControlMoc::fscryptControlMoc = nullptr;
    fscryptControlMock_ = nullptr;
    KeyControlMoc::keyControlMoc = nullptr;
    keyControlMock_ = nullptr;
    BaseKeyMoc::baseKeyMoc = nullptr;
    baseKeyMock_ = nullptr;
    FscryptKeyV2Moc::fscryptKeyV2Moc = nullptr;
    fscryptKeyMock_ = nullptr;
    userkeyExtMocMock_ = nullptr;
    KeyManagerExt::GetInstance().service_ = nullptr;
}

/**
 * @tc.name: KeyManagerExt_GenerateUserKeys_001
 * @tc.desc: Verify the KeyManagerExt GenerateUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_GenerateUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_GenerateUserKeys_001 start";
    uint32_t userId = 124;
    uint32_t flags = 1;
    KeyManagerExt::GetInstance().service_ = nullptr;
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_OK);

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_OK);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_OK);

    flags = 2;
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_KEY_EMPTY_ERROR);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(userId, EL1_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_KEY_EMPTY_ERROR);

    KeyManager::GetInstance().SaveUserElKey(userId, EL2_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetHashKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_KEY_EMPTY_ERROR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    auto setKeyBlob = [](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    };
    EXPECT_CALL(*baseKeyMock_, GetHashKey(_)).WillOnce(DoAll(WithArgs<0>(setKeyBlob), Return(true)));
    EXPECT_CALL(*userkeyExtMocMock_, GenerateUserKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userkeyExtMocMock_, SetFilePathPolicy(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_OK);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetHashKey(_)).WillOnce(DoAll(WithArgs<0>(setKeyBlob), Return(true)));
    EXPECT_CALL(*userkeyExtMocMock_, GenerateUserKey(_, _)).WillOnce(Return(E_KEY_EMPTY_ERROR));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_KEY_EMPTY_ERROR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetHashKey(_)).WillOnce(DoAll(WithArgs<0>(setKeyBlob), Return(true)));
    EXPECT_CALL(*userkeyExtMocMock_, GenerateUserKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userkeyExtMocMock_, SetFilePathPolicy(_)).WillOnce(Return(E_KEY_EMPTY_ERROR));
    EXPECT_EQ(KeyManagerExt::GetInstance().GenerateUserKeys(userId, flags), E_KEY_EMPTY_ERROR);
    GTEST_LOG_(INFO) << "KeyManagerExt_GenerateUserKeys_001 end";
}

/**
 * @tc.name: KeyManagerExt_DeleteUserKeys_001
 * @tc.desc: Verify the KeyManagerExt DeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_DeleteUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_DeleteUserKeys_001 start";
    uint32_t userId = 124;
    KeyManagerExt::GetInstance().service_ = nullptr;
    EXPECT_EQ(KeyManagerExt::GetInstance().DeleteUserKeys(userId), E_OK);

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().DeleteUserKeys(userId), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_DeleteUserKeys_001 end";
}

/**
 * @tc.name: KeyManagerExt_DeleteUserKeys_002
 * @tc.desc: Verify the KeyManagerExt DeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_DeleteUserKeys_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_DeleteUserKeys_002 start";
    uint32_t userId = 124;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, DeleteUserKey(_)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(KeyManagerExt::GetInstance().DeleteUserKeys(userId), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "KeyManagerExt_DeleteUserKeys_002 end";
}

/**
 * @tc.name: KeyManagerExt_DeleteUserKeys_003
 * @tc.desc: Verify the KeyManagerExt DeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_DeleteUserKeys_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_DeleteUserKeys_003 start";
    uint32_t userId = 124;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, DeleteUserKey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManagerExt::GetInstance().DeleteUserKeys(userId), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_DeleteUserKeys_003 end";
}

/**
 * @tc.name: KeyManagerExt_ActiveUserKey_001
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_ActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_ActiveUserKey_001 Start";
    unsigned int user = 888;
    const std::vector<uint8_t> token = {};
    const std::vector<uint8_t> secret = {};
    KeyManagerExt::GetInstance().service_ = nullptr;
    EXPECT_EQ(KeyManagerExt::GetInstance().ActiveUserKey(user, token, secret), E_OK);

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().ActiveUserKey(user, token, secret), E_OK);

    std::shared_ptr<BaseKey> tmpKey = std::dynamic_pointer_cast<BaseKey>(std::make_shared<FscryptKeyV2>("test"));
    KeyManager::GetInstance().SaveUserElKey(user, EL1_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManagerExt::GetInstance().ActiveUserKey(user, token, secret), E_KEY_EMPTY_ERROR);

    KeyManager::GetInstance().SaveUserElKey(user, EL2_KEY, tmpKey);
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetHashKey(_)).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().ActiveUserKey(user, token, secret), E_KEY_EMPTY_ERROR);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*baseKeyMock_, GetHashKey(_)).WillOnce(DoAll(WithArgs<0>(Invoke([](KeyBlob &value) {
        std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
        value.Alloc(vecIn.size());
        std::copy(vecIn.begin(), vecIn.end(), value.data.get());
    })), Return(true)));
    EXPECT_CALL(*userkeyExtMocMock_, ActiveUserKey(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManagerExt::GetInstance().ActiveUserKey(user, token, secret), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_ActiveUserKey_001 end";
}

/**
 * @tc.name: KeyManagerExt_InActiveUserKey_001
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_InActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_InactiveUserElKey_001 Start";
    unsigned int user = 800;
    KeyManagerExt::GetInstance().service_ = nullptr;
    EXPECT_EQ(KeyManagerExt::GetInstance().InActiveUserKey(user), E_OK);

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().InActiveUserKey(user), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_InactiveUserElKey_001 end";
}

/**
 * @tc.name: KeyManagerExt_InActiveUserKey_002
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_InActiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_InActiveUserKey_002 Start";
    unsigned int user = 800;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, InactiveUserKey(_)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(KeyManagerExt::GetInstance().InActiveUserKey(user), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "KeyManagerExt_InActiveUserKey_002 end";
}

/**
 * @tc.name: KeyManagerExt_InActiveUserKey_003
 * @tc.desc: Verify the InActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_InActiveUserKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_InActiveUserKey_003 Start";
    unsigned int user = 800;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, InactiveUserKey(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManagerExt::GetInstance().InActiveUserKey(user), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_InActiveUserKey_003 end";
}

/**
 * @tc.name: KeyManagerExt_SetRecoverKey_001
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_001 Start";
    uint32_t userId = 300;
    uint32_t keyType = 0;
    std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
    KeyManagerExt::GetInstance().service_ = nullptr;
    EXPECT_EQ(KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, vecIn), E_OK);

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, vecIn), E_OK);

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_EQ(KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, vecIn), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_001 end";
}

/**
 * @tc.name: KeyManagerExt_SetRecoverKey_002
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_002 Start";
    uint32_t userId = 300;
    uint32_t keyType = 1;
    std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, vecIn), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_002 end";
}

/**
 * @tc.name: KeyManagerExt_SetRecoverKey_003
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_003 Start";
    uint32_t userId = 300;
    uint32_t keyType = 1;
    std::vector<uint8_t> vecIn{1, 2, 3, 4, 5};
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, vecIn), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_003 end";
}

/**
 * @tc.name: KeyManagerExt_UpdateUserPublicDirPolicy_001
 * @tc.desc: Verify the UpdateUserPublicDirPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_UpdateUserPublicDirPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_UpdateUserPublicDirPolicy_001 Start";
    unsigned int user = 800;
    KeyManagerExt::GetInstance().service_ = nullptr;
    EXPECT_EQ(KeyManagerExt::GetInstance().UpdateUserPublicDirPolicy(user), E_OK);

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    EXPECT_EQ(KeyManagerExt::GetInstance().UpdateUserPublicDirPolicy(user), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_UpdateUserPublicDirPolicy_001 end";
}

/**
 * @tc.name: KeyManagerExt_UpdateUserPublicDirPolicy_002
 * @tc.desc: Verify the UpdateUserPublicDirPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_UpdateUserPublicDirPolicy_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_UpdateUserPublicDirPolicy_002 Start";
    unsigned int user = 800;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, UpdateUserPublicDirPolicy(_)).WillOnce(Return(E_PARAMS_INVALID));
    EXPECT_EQ(KeyManagerExt::GetInstance().UpdateUserPublicDirPolicy(user), E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "KeyManagerExt_UpdateUserPublicDirPolicy_002 end";
}

/**
 * @tc.name: KeyManagerExt_UpdateUserPublicDirPolicy_003
 * @tc.desc: Verify the UpdateUserPublicDirPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR20250722463628
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_UpdateUserPublicDirPolicy_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_UpdateUserPublicDirPolicy_003 Start";
    unsigned int user = 800;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, UpdateUserPublicDirPolicy(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(KeyManagerExt::GetInstance().UpdateUserPublicDirPolicy(user), E_OK);
    GTEST_LOG_(INFO) << "KeyManagerExt_UpdateUserPublicDirPolicy_003 end";
}
}
