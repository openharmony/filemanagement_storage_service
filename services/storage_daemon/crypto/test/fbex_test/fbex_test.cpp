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

#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "directory_ex.h"
#include "fbex.h"
#include "file_ex.h"
#include "key_blob.h"
#include "storage_service_errno.h"

using namespace testing::ext;
using namespace testing;

namespace {
const uint32_t PARAMS_SIZE_1 = 1;
const uint32_t PARAMS_1 = 1;
const uint32_t PARAMS_2 = 2;
constexpr size_t GCM_MAC_BYTES = 16;
constexpr size_t GCM_NONCE_BYTES = 12;
constexpr int AES_256_HASH_RANDOM_SIZE = 32;
constexpr uint32_t UNLOCK_STATUS = 0x2;
const uint32_t VALID_SIZE = GCM_NONCE_BYTES + AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES;
constexpr const char *FBEX_UFS_INLINE_BASE_ADDR = "/proc/bootdevice/name";
constexpr const char *FBEX_CMD_PATH = "/dev/fbex_cmd";
constexpr const char *FBEX_UECE_PATH = "/dev/fbex_uece";
}
namespace OHOS::StorageDaemon {
class FbexTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FbexTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FbexTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FbexTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FbexTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: fbex_InstallEL5KeyToKernel
 * @tc.desc: Verify the fbex InstallEL5KeyToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, InstallEL5KeyToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_InstallEL5KeyToKernel start";
    FBEX fbex;
    uint32_t userIdSingle = PARAMS_1;
    uint32_t userIdDouble = PARAMS_2;
    uint8_t flag = 1;
    bool isSupport = true;
    bool isNeedEncryptClassE = true;
    OHOS::RemoveFile(FBEX_UECE_PATH);
    int ret = fbex.InstallEL5KeyToKernel(userIdSingle, userIdDouble, flag, isSupport, isNeedEncryptClassE);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(isSupport, false);
    EXPECT_EQ(isNeedEncryptClassE, true);

    isSupport = true;
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    OHOS::ForceCreateDirectory(FBEX_UECE_PATH);
    ret = fbex.InstallEL5KeyToKernel(userIdSingle, userIdDouble, flag, isSupport, isNeedEncryptClassE);
    EXPECT_EQ(ret, -EISDIR);
    EXPECT_EQ(isSupport, true);

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.InstallEL5KeyToKernel(userIdSingle, userIdDouble, flag, isSupport, isNeedEncryptClassE);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    GTEST_LOG_(INFO) << "fbex_InstallEL5KeyToKernel end";
}

/**
 * @tc.name: fbex_InstallKeyToKernel
 * @tc.desc: Verify the fbex InstallKeyToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, InstallKeyToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_InstallKeyToKernel start";
    FBEX fbex;
    uint32_t userId = PARAMS_1;
    uint32_t type = PARAMS_1;
    KeyBlob iv;
    uint8_t flag = 1;
    int ret = fbex.InstallKeyToKernel(userId, type, iv, flag, {});
    EXPECT_EQ(ret, -EINVAL);

    KeyBlob iv2(FBEX_IV_SIZE);
    KeyBlob authToken(FBEX_IV_SIZE);
    ret = fbex.InstallKeyToKernel(userId, type, iv2, flag, authToken);
    EXPECT_NE(ret, 0);

    OHOS::ForceRemoveDirectory(FBEX_CMD_PATH);
    std::ofstream file(FBEX_CMD_PATH);
    ret = fbex.InstallKeyToKernel(userId, type, iv2, flag, authToken);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_CMD_PATH));
    GTEST_LOG_(INFO) << "fbex_InstallKeyToKernel end";
}

/**
 * @tc.name: fbex_UninstallOrLockUserKeyToKernel
 * @tc.desc: Verify the fbex UninstallOrLockUserKeyToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, UninstallOrLockUserKeyToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_UninstallOrLockUserKeyToKernel start";
    FBEX fbex;
    uint32_t userId = PARAMS_1;
    uint32_t type = PARAMS_1;
    uint8_t destroy = 1;
    int ret = fbex.UninstallOrLockUserKeyToKernel(userId, type, nullptr, FBEX_IV_SIZE, destroy);
    EXPECT_EQ(ret, -EINVAL);

    uint8_t *iv = new uint8_t[FBEX_IV_SIZE];
    std::ofstream file(FBEX_CMD_PATH);
    ret = fbex.UninstallOrLockUserKeyToKernel(userId, type, iv, FBEX_IV_SIZE, destroy);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_CMD_PATH));
    delete[] iv;
    iv = nullptr;
    GTEST_LOG_(INFO) << "fbex_UninstallOrLockUserKeyToKernel end";
}

/**
 * @tc.name: fbex_DeleteClassEPinCode
 * @tc.desc: Verify the fbex DeleteClassEPinCode.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, DeleteClassEPinCode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_DeleteClassEPinCode start";
    FBEX fbex;
    uint32_t userIdSingle = PARAMS_1;
    uint32_t userIdDouble = PARAMS_2;
    int ret = fbex.DeleteClassEPinCode(userIdSingle, userIdDouble);
    EXPECT_EQ(ret, 0);

    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.DeleteClassEPinCode(userIdSingle, userIdDouble);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    GTEST_LOG_(INFO) << "fbex_DeleteClassEPinCode end";
}

/**
 * @tc.name: fbex_ChangePinCodeClassE
 * @tc.desc: Verify the fbex ChangePinCodeClassE.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, ChangePinCodeClassE, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_ChangePinCodeClassE start";
    FBEX fbex;
    uint32_t userIdSingle = PARAMS_1;
    uint32_t userIdDouble = PARAMS_2;
    bool isFbeSupport = true;
    int ret = fbex.ChangePinCodeClassE(userIdSingle, userIdDouble, isFbeSupport);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(isFbeSupport, false);

    isFbeSupport = true;
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    OHOS::ForceCreateDirectory(FBEX_UECE_PATH);
    ret = fbex.ChangePinCodeClassE(userIdSingle, userIdDouble, isFbeSupport);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(isFbeSupport, true);

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.ChangePinCodeClassE(userIdSingle, userIdDouble, isFbeSupport);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(isFbeSupport, true);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    GTEST_LOG_(INFO) << "fbex_ChangePinCodeClassE end";
}

/**
 * @tc.name: fbex_LockScreenToKernel
 * @tc.desc: Verify the fbex LockScreenToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, LockScreenToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_LockScreenToKernel start";
    FBEX fbex;
    uint32_t userId = PARAMS_1;
    OHOS::ForceRemoveDirectory(FBEX_CMD_PATH);
    int ret = fbex.LockScreenToKernel(userId);
    EXPECT_NE(ret, 0);

    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.LockScreenToKernel(userId);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    GTEST_LOG_(INFO) << "fbex_LockScreenToKernel end";
}

/**
 * @tc.name: fbex_GenerateAppkey
 * @tc.desc: Verify the fbex GenerateAppkey.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, GenerateAppkey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_GenerateAppkey start";
    FBEX fbex;
    UserIdToFbeStr userIdToFbe;
    uint32_t appUid = PARAMS_1;
    auto keyId = std::make_unique<uint8_t[]>(PARAMS_SIZE_1);
    uint32_t size = PARAMS_1;

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    OHOS::ForceCreateDirectory(FBEX_UECE_PATH);
    int ret = fbex.GenerateAppkey(userIdToFbe, appUid, keyId, size);
    EXPECT_NE(ret, 0);
    ASSERT_TRUE(keyId);

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.GenerateAppkey(userIdToFbe, appUid, keyId, size);
    EXPECT_NE(ret, 0);
    ASSERT_TRUE(keyId);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    ret = fbex.GenerateAppkey(userIdToFbe, appUid, keyId, size);
    EXPECT_EQ(ret, 0);
    ASSERT_FALSE(keyId);
    GTEST_LOG_(INFO) << "fbex_GenerateAppkey end";
}

/**
 * @tc.name: fbex_LockUece
 * @tc.desc: Verify the fbex LockUece.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, LockUece, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_LockUece start";
    FBEX fbex;
    uint32_t userIdSingle = PARAMS_1;
    uint32_t userIdDouble = PARAMS_2;
    bool isFbeSupport = true;
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    int ret = fbex.LockUece(userIdSingle, userIdDouble, isFbeSupport);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(isFbeSupport, false);

    isFbeSupport = true;
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    OHOS::ForceCreateDirectory(FBEX_UECE_PATH);
    ret = fbex.LockUece(userIdSingle, userIdDouble, isFbeSupport);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(isFbeSupport, true);

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.LockUece(userIdSingle, userIdDouble, isFbeSupport);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    GTEST_LOG_(INFO) << "fbex_LockUece end";
}

/**
 * @tc.name: fbex_UnlockScreenToKernel
 * @tc.desc: Verify the fbex UnlockScreenToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, UnlockScreenToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_UnlockScreenToKernel start";
    FBEX fbex;
    uint32_t userId = PARAMS_1;
    uint32_t type = PARAMS_1;
    uint32_t size = FBEX_IV_SIZE;

    int ret = fbex.UnlockScreenToKernel(userId, type, nullptr, size, {});
    EXPECT_EQ(ret, -EINVAL);

    uint8_t *iv = new uint8_t[FBEX_IV_SIZE];
    OHOS::ForceRemoveDirectory(FBEX_CMD_PATH);
    KeyBlob authToken(FBEX_IV_SIZE);
    ret = fbex.UnlockScreenToKernel(userId, type, iv, size, authToken);
    EXPECT_NE(ret, 0);

    std::ofstream file(FBEX_CMD_PATH);
    ret = fbex.UnlockScreenToKernel(userId, type, iv, size, authToken);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_CMD_PATH));
    delete[] iv;
    iv = nullptr;
    GTEST_LOG_(INFO) << "fbex_UnlockScreenToKernel end";
}

/**
 * @tc.name: fbex_ReadESecretToKernel
 * @tc.desc: Verify the fbex ReadESecretToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, ReadESecretToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_ReadESecretToKernel start";
    FBEX fbex;
    UserIdToFbeStr userIdToFbe;

    uint32_t status = PARAMS_1;
    bool isFbeSupport = true;
    EXPECT_EQ(fbex.ReadESecretToKernel(userIdToFbe, status, {}, {}, isFbeSupport), -EINVAL);

    status = UNLOCK_STATUS;
    int ret = fbex.ReadESecretToKernel(userIdToFbe, status, {}, {}, isFbeSupport);
    EXPECT_EQ(ret, -EINVAL);

    KeyBlob eBuffer(VALID_SIZE);
    isFbeSupport = true;
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    OHOS::ForceCreateDirectory(FBEX_UECE_PATH);
    KeyBlob authToken(FBEX_IV_SIZE);
    ret = fbex.ReadESecretToKernel(userIdToFbe, status, eBuffer, authToken, isFbeSupport);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(isFbeSupport, true);

    isFbeSupport = true;
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    ret = fbex.ReadESecretToKernel(userIdToFbe, status, eBuffer, authToken, isFbeSupport);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(isFbeSupport, false);

    isFbeSupport = true;
    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.ReadESecretToKernel(userIdToFbe, status, eBuffer, authToken, isFbeSupport);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(isFbeSupport, true);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    eBuffer.Clear();
    GTEST_LOG_(INFO) << "fbex_ReadESecretToKernel end";
}

/**
 * @tc.name: fbex_WriteESecretToKernel
 * @tc.desc: Verify the fbex WriteESecretToKernel.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, WriteESecretToKernel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_WriteESecretToKernel start";
    FBEX fbex;
    UserIdToFbeStr userIdToFbe;

    uint32_t status = PARAMS_1;
    EXPECT_EQ(fbex.WriteESecretToKernel(userIdToFbe, status, nullptr, VALID_SIZE), -EINVAL);

    status = UNLOCK_STATUS;
    int ret = fbex.WriteESecretToKernel(userIdToFbe, status, nullptr, VALID_SIZE);
    EXPECT_EQ(ret, -EINVAL);

    uint8_t *eBuffer = new uint8_t[AES_256_HASH_RANDOM_SIZE];
    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    OHOS::ForceCreateDirectory(FBEX_UECE_PATH);
    ret = fbex.WriteESecretToKernel(userIdToFbe, status, eBuffer, AES_256_HASH_RANDOM_SIZE);
    EXPECT_NE(ret, 0);

    OHOS::ForceRemoveDirectory(FBEX_UECE_PATH);
    ret = fbex.WriteESecretToKernel(userIdToFbe, status, eBuffer, AES_256_HASH_RANDOM_SIZE);
    EXPECT_EQ(ret, 0);

    std::ofstream file(FBEX_UECE_PATH);
    ret = fbex.WriteESecretToKernel(userIdToFbe, status, eBuffer, AES_256_HASH_RANDOM_SIZE);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_UECE_PATH));
    delete[] eBuffer;
    eBuffer = nullptr;
    GTEST_LOG_(INFO) << "fbex_WriteESecretToKernel end";
}

/**
 * @tc.name: fbex_IsMspReady
 * @tc.desc: Verify the fbex IsMspReady.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, IsMspReady, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_IsMspReady start";
    FBEX fbex;
    EXPECT_EQ(fbex.IsMspReady(), false);
    GTEST_LOG_(INFO) << "fbex_IsMspReady end";
}

/**
 * @tc.name: fbex_GetStatus
 * @tc.desc: Verify the fbex GetStatus.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, GetStatus, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_GetStatus start";
    FBEX fbex;
    EXPECT_NE(fbex.GetStatus(), 0);

    std::ofstream file(FBEX_CMD_PATH);
    EXPECT_NE(fbex.GetStatus(), 0);
    EXPECT_TRUE(OHOS::RemoveFile(FBEX_CMD_PATH));
    GTEST_LOG_(INFO) << "fbex_GetStatus end";
}

/**
 * @tc.name: fbex_IsFBEXSupported
 * @tc.desc: Verify the fbex IsFBEXSupported.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, IsFBEXSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "fbex_IsFBEXSupported start";
    FBEX fbex;
    OHOS::ForceRemoveDirectory(FBEX_UFS_INLINE_BASE_ADDR);
    OHOS::RemoveFile(FBEX_UFS_INLINE_BASE_ADDR);
    EXPECT_FALSE(fbex.IsFBEXSupported());
    GTEST_LOG_(INFO) << "fbex_IsFBEXSupported end";
}
} // OHOS::StorageDaemon