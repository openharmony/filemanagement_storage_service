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

/**
 * @tc.name: KeyManagerExt_Init_001
 * @tc.desc: Verify the Init function with re-initialization check.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_Init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_Init_001 start";

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));
    KeyManagerExt::GetInstance().Init();

    GTEST_LOG_(INFO) << "KeyManagerExt_Init_001 end";
}

/**
 * @tc.name: KeyManagerExt_Init_002
 * @tc.desc: Verify the Init function with handler_ already initialized.
 * @tc.type: FUNC
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_Init_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_Init_002 start";

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));
    KeyManagerExt::GetInstance().Init();

    GTEST_LOG_(INFO) << "KeyManagerExt_Init_002 end";
}


/**
 * @tc.name: KeyManagerExt_SetRecoverKey_004
 * @tc.desc: Verify the SetRecoverKey function with invalid ivBlob.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_004 start";

    uint32_t userId = 300;
    uint32_t keyType = 1;
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillRepeatedly(Return(true));

    KeyBlob ivBlob;
    int ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob);
    EXPECT_EQ(ret, E_PARAMS);

    KeyBlob ivBlob2(0);
    ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob2);
    EXPECT_EQ(ret, E_PARAMS);

    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_004 end";
}

/**
 * @tc.name: KeyManagerExt_SetRecoverKey_005
 * @tc.desc: Verify the SetRecoverKey function with valid ivBlob.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_005 start";

    uint32_t userId = 300;
    uint32_t keyType = 1;
    KeyBlob ivBlob(16);
    for (uint32_t i = 0; i < ivBlob.size; i++) {
        ivBlob.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillRepeatedly(Return(true));

    EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillOnce(Return(E_OK));
    int ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob);
    EXPECT_TRUE(ret == E_OK || ret == E_KEY_EMPTY_ERROR);

    EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillOnce(Return(E_PARAMS));
    ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_005 end";
}

/**
 * @tc.name: KeyManagerExt_SetRecoverKey_006
 * @tc.desc: Verify the SetRecoverKey function with various keyTypes.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_006 start";

    uint32_t userId = 300;
    KeyBlob ivBlob(16);
    for (uint32_t i = 0; i < ivBlob.size; i++) {
        ivBlob.data.get()[i] = static_cast<uint8_t>(i);
    }

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillRepeatedly(Return(true));
    EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillRepeatedly(Return(E_OK));

    for (uint32_t keyType = 0; keyType <= 3; keyType++) {
        int ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob);
        EXPECT_TRUE(ret == E_OK || ret == E_KEY_EMPTY_ERROR);
    }

    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_006 end";
}

/**
 * @tc.name: KeyManagerExt_SetRecoverKey_007
 * @tc.desc: Verify the SetRecoverKey function with boundary ivBlob sizes.
 * @tc.type: FUNC
 * @tc.require: DTS2025080504160
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_SetRecoverKey_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_007 start";

    uint32_t userId = 300;
    uint32_t keyType = 1;

    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillRepeatedly(Return(true));

    {
        KeyBlob ivBlob(1);
        ivBlob.data.get()[0] = 0xFF;
        EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillOnce(Return(E_OK));
        int ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob);
        EXPECT_TRUE(ret == E_OK || ret == E_KEY_EMPTY_ERROR);
    }

    {
        KeyBlob ivBlob(32);
        for (uint32_t i = 0; i < ivBlob.size; i++) {
            ivBlob.data.get()[i] = static_cast<uint8_t>(i);
        }
        EXPECT_CALL(*userkeyExtMocMock_, SetRecoverKey(_, _)).WillOnce(Return(E_OK));
        int ret = KeyManagerExt::GetInstance().SetRecoverKey(userId, keyType, ivBlob);
        EXPECT_TRUE(ret == E_OK || ret == E_KEY_EMPTY_ERROR);
    }

    GTEST_LOG_(INFO) << "KeyManagerExt_SetRecoverKey_007 end";
}

/**
 * @tc.name: KeyManagerExt_Init_003
 * @tc.desc: Verify the Init function with handler_ already set.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_Init_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_Init_003 start";

    KeyManagerExt::GetInstance().handler_ = reinterpret_cast<void*>(0x1234);
    KeyManagerExt::GetInstance().service_ = userkeyExtMocMock_.get();
    KeyManagerExt::GetInstance().Init();

    EXPECT_NE(KeyManagerExt::GetInstance().handler_, nullptr);
    EXPECT_NE(KeyManagerExt::GetInstance().service_, nullptr);

    KeyManagerExt::GetInstance().handler_ = nullptr;
    KeyManagerExt::GetInstance().service_ = nullptr;

    KeyManagerExt::GetInstance().handler_ = reinterpret_cast<void*>(0x5678);
    KeyManagerExt::GetInstance().service_ = nullptr;
    KeyManagerExt::GetInstance().Init();

    EXPECT_NE(KeyManagerExt::GetInstance().handler_, nullptr);
    EXPECT_EQ(KeyManagerExt::GetInstance().service_, nullptr);

    KeyManagerExt::GetInstance().handler_ = nullptr;

    GTEST_LOG_(INFO) << "KeyManagerExt_Init_003 end";
}

/**
 * @tc.name: KeyManagerExt_Init_004
 * @tc.desc: Verify the Init function when dlopen returns nullptr.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_Init_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_Init_004 start";

    KeyManagerExt::GetInstance().handler_ = nullptr;
    KeyManagerExt::GetInstance().service_ = nullptr;

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(true));

    KeyManagerExt::GetInstance().Init();

    EXPECT_EQ(KeyManagerExt::GetInstance().service_, nullptr);

    GTEST_LOG_(INFO) << "KeyManagerExt_Init_004 end";
}

/**
 * @tc.name: KeyManagerExt_Init_005
 * @tc.desc: Verify the Init function when KeyCtrlHasFscryptSyspara returns false.
 * @tc.type: FUNC
 * @tc.require: AR20250418146433
 */
HWTEST_F(KeyManagerExtTest, KeyManagerExt_Init_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KeyManagerExt_Init_005 start";

    KeyManagerExt::GetInstance().handler_ = nullptr;
    KeyManagerExt::GetInstance().service_ = nullptr;

    EXPECT_CALL(*fscryptControlMock_, KeyCtrlHasFscryptSyspara()).WillOnce(Return(false));

    KeyManagerExt::GetInstance().Init();

    EXPECT_EQ(KeyManagerExt::GetInstance().handler_, nullptr);
    EXPECT_EQ(KeyManagerExt::GetInstance().service_, nullptr);

    GTEST_LOG_(INFO) << "KeyManagerExt_Init_005 end";
}
}
