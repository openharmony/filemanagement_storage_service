/*
 * Copyright (C) 2022-2025 Huawei Device Co., Ltd.
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
#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "directory_ex.h"
#include "fbex_mock.h"
#include "fscrypt_key_v1_ext.h"
#include "storage_service_errno.h"
#include "utils/string_utils.h"

using namespace std;
using namespace testing::ext;
using namespace testing;
 
namespace OHOS::StorageDaemon {
const std::string NEED_RESTORE_PATH = "/data/service/el0/storage_daemon/sd/latest/need_restore";
static const uint32_t DEFAULT_SINGLE_FIRST_USER_ID = 100;
static const uint32_t USER_ID_DIFF = 91;
class FscryptKeyV1ExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FbexMoc> fbexMock_ = nullptr;
    static inline bool fileExist = false;
};

void FscryptKeyV1ExtTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    std::error_code errCode;
    if (std::filesystem::exists(NEED_RESTORE_PATH, errCode)) {
        fileExist = true;
    }
}

void FscryptKeyV1ExtTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    if (!fileExist) {
        std::filesystem::remove_all(NEED_RESTORE_PATH);
    }
}

void FscryptKeyV1ExtTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp Start";
    fbexMock_ = make_shared<FbexMoc>();
    FbexMoc::fbexMoc = fbexMock_;
}

void FscryptKeyV1ExtTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown Start";
    FbexMoc::fbexMoc = nullptr;
    fbexMock_ = nullptr;
}

/**
 * @tc.name: FscryptKeyV1Ext_GetMappedUserId_001
 * @tc.desc: Verify the GetMappedUserId function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetMappedUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_001 Start";
    FscryptKeyV1Ext ext;
    if (!fileExist) {
        EXPECT_EQ(ext.GetMappedUserId(100, TYPE_EL2), 100);
        std::error_code errCode;
        std::filesystem::create_directory(NEED_RESTORE_PATH, errCode);
        ASSERT_TRUE(errCode.value() == 0);
    }

    EXPECT_EQ(ext.GetMappedUserId(101, TYPE_EL1), 101);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GetMappedUserId_002
 * @tc.desc: Verify the GetMappedUserId function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetMappedUserId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_002 Start";
    uint32_t userId = 100;
    FscryptKeyV1Ext ext;
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL2), 0);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL3), 0);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL4), 0);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL5), 0);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_002 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GetMappedUserId_003
 * @tc.desc: Verify the GetMappedUserId function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetMappedUserId_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_003 Start";
    uint32_t userId = 101;
    auto rlt = userId - USER_ID_DIFF;
    FscryptKeyV1Ext ext;
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL2), rlt);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL3), rlt);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL4), rlt);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL5), rlt);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_003 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GetMappedUserId_004
 * @tc.desc: Verify the GetMappedUserId function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetMappedUserId_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_004 end";
    uint32_t userId = 99;
    FscryptKeyV1Ext ext;
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL2), userId);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL3), userId);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL4), userId);
    EXPECT_EQ(ext.GetMappedUserId(userId, TYPE_EL5), userId);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedUserId_004 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_ActiveKeyExt_001
 * @tc.desc: Verify the ActiveKeyExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_ActiveKeyExt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ActiveKeyExt_001 end";
    KeyBlob iv(1);
    uint32_t elType = 0;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, elType, {}), 0);
    EXPECT_EQ(elType, 0);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(1)).WillOnce(Return(1));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, elType, {}), 1);
    EXPECT_EQ(elType, 0);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(1)).WillOnce(Return(0));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, elType, {}), 0);
    EXPECT_EQ(elType, TYPE_EL2);

    elType = 0;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.ActiveKeyExt(1, iv, elType, {}), 1);
    EXPECT_EQ(elType, TYPE_EL1);
    
    elType = 0;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, elType, {}), 0);
    EXPECT_EQ(elType, TYPE_EL2);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ActiveKeyExt_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_ActiveKeyExt_002
 * @tc.desc: Verify the ActiveKeyExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_ActiveKeyExt_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ActiveKeyExt_002 end";
    KeyBlob iv(1);
    uint32_t el3Type = TYPE_EL3;
    uint32_t el2Type = TYPE_EL2;
    FscryptKeyV1Ext ext;
    ext.userId_ = 219;
    ext.type_ = TYPE_EL2;
    int el3Err = 0xFBE30004;
    
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, el2Type, {}), 0);

    ext.userId_ = 219;
    ext.type_ = TYPE_EL3;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(el3Err));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, el3Type, {}), el3Err);


    std::string errMsg = "";
    std::string el4Path = "/data/service/el1/public/storage_daemon/sd/el4/219/latest";
    EXPECT_TRUE(OHOS::ForceCreateDirectory(el4Path));
    ASSERT_TRUE(SaveStringToFileSync(el4Path + "/need_restore", "1", errMsg));
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(el3Err)).WillOnce(Return(0));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, el3Type, {}), 0);

    
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallKeyToKernel(_, _, _, _, _)).WillOnce(Return(el3Err)).WillOnce(Return(-1));
    EXPECT_EQ(ext.ActiveKeyExt(0, iv, el3Type, {}), -1);

    EXPECT_TRUE(OHOS::ForceRemoveDirectory("/data/service/el1/public/storage_daemon/sd/el4/219"));
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ActiveKeyExt_002 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_UnlockUserScreenExt_001
 * @tc.desc: Verify the UnlockUserScreenExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_UnlockUserScreenExt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_UnlockUserScreenExt_001 end";
    uint8_t iv = 1;
    uint32_t size = 1;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.UnlockUserScreenExt(0, &iv, size, {}), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UnlockScreenToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.UnlockUserScreenExt(0, &iv, size, {}), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UnlockScreenToKernel(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_NE(ext.UnlockUserScreenExt(0, &iv, size, {}), E_OK);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_UnlockUserScreenExt_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GenerateAppkey_001 end";
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    std::unique_ptr<uint8_t []> appKey;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.GenerateAppkey(100, 100, appKey, 1), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.GenerateAppkey(100, 100, appKey, 1), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, GenerateAppkey(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.GenerateAppkey(100, 100, appKey, 1), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GenerateAppkey_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_AddClassE_001
 * @tc.desc: Verify the AddClassE function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_AddClassE_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_AddClassE_001 end";
    bool isNeedEncryptClassE;
    bool isSupport;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.AddClassE(isNeedEncryptClassE, isSupport, 0), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallEL5KeyToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.AddClassE(isNeedEncryptClassE, isSupport, 0), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, InstallEL5KeyToKernel(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.AddClassE(isNeedEncryptClassE, isSupport, 0), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_AddClassE_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_DeleteClassEPinCode_001
 * @tc.desc: Verify the DeleteClassEPinCode function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_DeleteClassEPinCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_DeleteClassEPinCode_001 end";
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.DeleteClassEPinCode(100), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, DeleteClassEPinCode(_, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.DeleteClassEPinCode(100), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, DeleteClassEPinCode(_, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.DeleteClassEPinCode(100), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_DeleteClassEPinCode_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_ChangePinCodeClassE_001
 * @tc.desc: Verify the ChangePinCodeClassE function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_ChangePinCodeClassE_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ChangePinCodeClassE_001 end";
    bool isFbeSupport;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.ChangePinCodeClassE(100, isFbeSupport), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, ChangePinCodeClassE(_, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.ChangePinCodeClassE(100, isFbeSupport), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, ChangePinCodeClassE(_, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.ChangePinCodeClassE(100, isFbeSupport), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ChangePinCodeClassE_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_UpdateClassEBackUp_001
 * @tc.desc: Verify the UpdateClassEBackUp function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_UpdateClassEBackUp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_UpdateClassEBackUp_001 end";
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.UpdateClassEBackUp(100), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UpdateClassEBackUp(_, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.UpdateClassEBackUp(100), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UpdateClassEBackUp(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(ext.UpdateClassEBackUp(100), -1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_UpdateClassEBackUp_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_ReadClassE_001
 * @tc.desc: Verify the ReadClassE function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_ReadClassE_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ReadClassE_001 end";
    bool isFbeSupport;
    KeyBlob classEBuffer;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.ReadClassE(0, classEBuffer, {}, isFbeSupport), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, ReadESecretToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.ReadClassE(0, classEBuffer, {}, isFbeSupport), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, ReadESecretToKernel(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.ReadClassE(0, classEBuffer, {}, isFbeSupport), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ReadClassE_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_WriteClassE_001
 * @tc.desc: Verify the WriteClassE function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_WriteClassE_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_WriteClassE_001 end";
    uint8_t classEBuffer;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.WriteClassE(0, &classEBuffer, 1), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, WriteESecretToKernel(_, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.WriteClassE(0, &classEBuffer, 1), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, WriteESecretToKernel(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.WriteClassE(0, &classEBuffer, 1), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_WriteClassE_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_InactiveKeyExt_001
 * @tc.desc: Verify the InactiveKeyExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_InactiveKeyExt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_InactiveKeyExt_001 end";
    uint32_t flag = 2;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.InactiveKeyExt(flag), 0);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UninstallOrLockUserKeyToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.InactiveKeyExt(flag), 0);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UninstallOrLockUserKeyToKernel(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.InactiveKeyExt(flag), 1);
    
    ext.type_ = TYPE_EL1;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_EQ(ext.InactiveKeyExt(0), 0);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UninstallOrLockUserKeyToKernel(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ext.InactiveKeyExt(flag), 0);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, UninstallOrLockUserKeyToKernel(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.InactiveKeyExt(flag), 1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_InactiveKeyExt_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_LockUserScreenExt_001
 * @tc.desc: Verify the LockUserScreenExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_LockUserScreenExt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_LockUserScreenExt_001 end";
    uint32_t flag = 2;
    uint32_t elType = TYPE_EL5;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.LockUserScreenExt(flag, elType), E_OK);
    EXPECT_EQ(elType, TYPE_EL5);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, LockScreenToKernel(_)).WillOnce(Return(1));
    EXPECT_NE(ext.LockUserScreenExt(flag, elType), E_OK);
    EXPECT_EQ(elType, TYPE_EL5);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, LockScreenToKernel(_)).WillOnce(Return(0));
    EXPECT_EQ(ext.LockUserScreenExt(flag, elType), E_OK);
    EXPECT_EQ(elType, ext.type_);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_LockUserScreenExt_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_LockUeceExt_001
 * @tc.desc: Verify the LockUeceExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_LockUeceExt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_LockUeceExt_001 end";
    bool isFbeSupport;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(false));
    EXPECT_EQ(ext.LockUeceExt(isFbeSupport), E_OK);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, LockUece(_, _, _)).WillOnce(Return(1));
    EXPECT_EQ(ext.LockUeceExt(isFbeSupport), 1);

    EXPECT_CALL(*fbexMock_, IsFBEXSupported()).WillOnce(Return(true));
    EXPECT_CALL(*fbexMock_, LockUece(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(ext.LockUeceExt(isFbeSupport), E_OK);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_LockUeceExt_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GetUserIdFromDir_001
 * @tc.desc: Verify the GetUserIdFromDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetUserIdFromDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetUserIdFromDir_001 end";
    FscryptKeyV1Ext ext;
    ext.dir_ = "test";
    EXPECT_EQ(ext.GetUserIdFromDir(), 0);
    
    ext.dir_ = "/data/foo/bar/el1/100";
    EXPECT_EQ(ext.GetUserIdFromDir(), 100);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetUserIdFromDir_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GetTypeFromDir_001
 * @tc.desc: Verify the GetTypeFromDir function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetTypeFromDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetTypeFromDir_001 end";
    FscryptKeyV1Ext ext;
    ext.dir_ = "test";
    EXPECT_EQ(ext.GetTypeFromDir(), TYPE_GLOBAL_EL1);
    
    ext.dir_ = "data/";
    EXPECT_EQ(ext.GetTypeFromDir(), TYPE_GLOBAL_EL1);

    ext.dir_ = "/data";
    EXPECT_EQ(ext.GetTypeFromDir(), TYPE_GLOBAL_EL1);

    ext.dir_ = "/data/foo/bar/el1/100";
    EXPECT_EQ(ext.GetTypeFromDir(), TYPE_EL1);

    ext.dir_ = "/data/foo/bar";
    EXPECT_EQ(ext.GetTypeFromDir(), TYPE_GLOBAL_EL1);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetTypeFromDir_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_GetMappedDeUserId_001
 * @tc.desc: Verify the GetMappedDeUserId function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_GetMappedDeUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedDeUserId_001 end";
    FscryptKeyV1Ext ext;
    EXPECT_EQ(ext.GetMappedDeUserId(DEFAULT_SINGLE_FIRST_USER_ID), 0);

    uint32_t userId = DEFAULT_SINGLE_FIRST_USER_ID + 5;
    EXPECT_EQ(ext.GetMappedDeUserId(userId), userId - USER_ID_DIFF);

    userId = DEFAULT_SINGLE_FIRST_USER_ID - 5;
    EXPECT_EQ(ext.GetMappedDeUserId(userId), userId);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_GetMappedDeUserId_001 end";
}

/**
 * @tc.name: FscryptKeyV1Ext_ActiveDoubleKeyExt_001
 * @tc.desc: Verify the ActiveDoubleKeyExt function.
 * @tc.type: FUNC
 * @tc.require: IAHHWW
 */
HWTEST_F(FscryptKeyV1ExtTest, FscryptKeyV1Ext_ActiveDoubleKeyExt_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ActiveDoubleKeyExt_001 end";
    KeyBlob iv(1);
    uint32_t elType = 0;
    FscryptKeyV1Ext ext;
    ext.userId_ = 100;
    ext.type_ = TYPE_EL2;
    EXPECT_CALL(*fbexMock_, InstallDoubleDeKeyToKernel(_, _, _, _)).WillOnce(Return(true));
    EXPECT_NE(ext.ActiveDoubleKeyExt(0, iv, elType, {}), 0);

    EXPECT_CALL(*fbexMock_, InstallDoubleDeKeyToKernel(_, _, _, _)).WillOnce(Return(false));
    EXPECT_EQ(ext.ActiveDoubleKeyExt(0, iv, elType, {}), 0);
    EXPECT_EQ(elType, TYPE_EL2);
    GTEST_LOG_(INFO) << "FscryptKeyV1Ext_ActiveDoubleKeyExt_001 end";
}
}
