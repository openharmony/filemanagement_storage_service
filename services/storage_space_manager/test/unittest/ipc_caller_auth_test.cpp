/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ipc_caller_auth.h"
#include "ipc_caller_auth_mock.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace testing;
using namespace testing::ext;

class IpcCallerAuthTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        g_mockCallingTokenId = 12345;
        g_mockCallingFullTokenId = 0;
        g_mockCallingUid = 1000;
        g_mockVerifyAccessTokenResult = 0;
        g_mockTokenTypeFlag = 1;
        g_mockIsSystemApp = true;
        g_mockGetNativeTokenInfoResult = 0;
        g_mockNativeProcessName = "mock_process";
        g_mockGetHapTokenInfoResult = 0;
        g_mockHapBundleName = "mock_bundle";
    }
    void TearDown() {}
};

/* ---------- VerifyCallerPermission ---------- */

HWTEST_F(IpcCallerAuthTest, VerifyCallerPermission_Granted, TestSize.Level1)
{
    g_mockVerifyAccessTokenResult = 0;
    EXPECT_TRUE(IpcCallerAuth::VerifyCallerPermission("ohos.permission.STORAGE_MANAGER"));
}

HWTEST_F(IpcCallerAuthTest, VerifyCallerPermission_Denied, TestSize.Level1)
{
    g_mockVerifyAccessTokenResult = -1;
    EXPECT_FALSE(IpcCallerAuth::VerifyCallerPermission("ohos.permission.STORAGE_MANAGER"));
}

/* ---------- VerifyCallerPermissionUnlessTrusted ---------- */

HWTEST_F(IpcCallerAuthTest, VerifyCallerPermissionUnlessTrusted_Trusted, TestSize.Level1)
{
    bool trusted = IpcCallerAuth::VerifyCallerPermissionUnlessTrusted(
        "ohos.permission.STORAGE_MANAGER", []() { return true; });
    EXPECT_TRUE(trusted);
}

HWTEST_F(IpcCallerAuthTest, VerifyCallerPermissionUnlessTrusted_NotTrusted_Granted, TestSize.Level1)
{
    g_mockVerifyAccessTokenResult = 0;
    bool trusted = IpcCallerAuth::VerifyCallerPermissionUnlessTrusted(
        "ohos.permission.STORAGE_MANAGER", []() { return false; });
    EXPECT_TRUE(trusted);
}

HWTEST_F(IpcCallerAuthTest, VerifyCallerPermissionUnlessTrusted_NotTrusted_Denied, TestSize.Level1)
{
    g_mockVerifyAccessTokenResult = -1;
    bool trusted = IpcCallerAuth::VerifyCallerPermissionUnlessTrusted(
        "ohos.permission.STORAGE_MANAGER", []() { return false; });
    EXPECT_FALSE(trusted);
}

HWTEST_F(IpcCallerAuthTest, VerifyCallerPermissionUnlessTrusted_NullCallback, TestSize.Level1)
{
    g_mockVerifyAccessTokenResult = 0;
    bool trusted = IpcCallerAuth::VerifyCallerPermissionUnlessTrusted(
        "ohos.permission.STORAGE_MANAGER", nullptr);
    EXPECT_TRUE(trusted);
}

/* ---------- IsCallingSystemApp ---------- */

HWTEST_F(IpcCallerAuthTest, IsCallingSystemApp_HapSystemApp, TestSize.Level1)
{
    g_mockTokenTypeFlag = 0;
    g_mockIsSystemApp = true;
    EXPECT_TRUE(IpcCallerAuth::IsCallingSystemApp());
}

HWTEST_F(IpcCallerAuthTest, IsCallingSystemApp_HapNotSystemApp, TestSize.Level1)
{
    g_mockTokenTypeFlag = 0;
    g_mockIsSystemApp = false;
    EXPECT_FALSE(IpcCallerAuth::IsCallingSystemApp());
}

HWTEST_F(IpcCallerAuthTest, IsCallingSystemApp_NativeToken, TestSize.Level1)
{
    g_mockTokenTypeFlag = 1;
    EXPECT_TRUE(IpcCallerAuth::IsCallingSystemApp());
}

HWTEST_F(IpcCallerAuthTest, IsCallingSystemApp_UnknownToken, TestSize.Level1)
{
    g_mockTokenTypeFlag = 2;
    EXPECT_TRUE(IpcCallerAuth::IsCallingSystemApp());
}

/* ---------- VerifyNativeCallerMatches ---------- */

HWTEST_F(IpcCallerAuthTest, VerifyNativeCallerMatches_Success, TestSize.Level1)
{
    g_mockGetNativeTokenInfoResult = 0;
    g_mockNativeProcessName = "expected_process";
    g_mockCallingUid = 2000;
    EXPECT_TRUE(IpcCallerAuth::VerifyNativeCallerMatches("expected_process", 2000));
}

HWTEST_F(IpcCallerAuthTest, VerifyNativeCallerMatches_GetInfoFailed, TestSize.Level1)
{
    g_mockGetNativeTokenInfoResult = -1;
    EXPECT_FALSE(IpcCallerAuth::VerifyNativeCallerMatches("mock_process", 1000));
}

HWTEST_F(IpcCallerAuthTest, VerifyNativeCallerMatches_ProcessMismatch, TestSize.Level1)
{
    g_mockGetNativeTokenInfoResult = 0;
    g_mockNativeProcessName = "actual_process";
    g_mockCallingUid = 1000;
    EXPECT_FALSE(IpcCallerAuth::VerifyNativeCallerMatches("expected_process", 1000));
}

HWTEST_F(IpcCallerAuthTest, VerifyNativeCallerMatches_UidMismatch, TestSize.Level1)
{
    g_mockGetNativeTokenInfoResult = 0;
    g_mockNativeProcessName = "mock_process";
    g_mockCallingUid = 9999;
    EXPECT_FALSE(IpcCallerAuth::VerifyNativeCallerMatches("mock_process", 1000));
}

/* ---------- GetCallingUid / IsCallingUid ---------- */

HWTEST_F(IpcCallerAuthTest, GetCallingUid_Basic, TestSize.Level1)
{
    g_mockCallingUid = 3000;
    EXPECT_EQ(IpcCallerAuth::GetCallingUid(), 3000);
}

HWTEST_F(IpcCallerAuthTest, IsCallingUid_Match, TestSize.Level1)
{
    g_mockCallingUid = 4000;
    EXPECT_TRUE(IpcCallerAuth::IsCallingUid(4000));
}

HWTEST_F(IpcCallerAuthTest, IsCallingUid_Mismatch, TestSize.Level1)
{
    g_mockCallingUid = 4000;
    EXPECT_FALSE(IpcCallerAuth::IsCallingUid(5000));
}

/* ---------- IsCallingNativeToken ---------- */

HWTEST_F(IpcCallerAuthTest, IsCallingNativeToken_Native, TestSize.Level1)
{
    g_mockTokenTypeFlag = 1;
    EXPECT_TRUE(IpcCallerAuth::IsCallingNativeToken());
}

HWTEST_F(IpcCallerAuthTest, IsCallingNativeToken_NotNative, TestSize.Level1)
{
    g_mockTokenTypeFlag = 2;
    EXPECT_FALSE(IpcCallerAuth::IsCallingNativeToken());
}

/* ---------- IsCallingHapToken ---------- */

HWTEST_F(IpcCallerAuthTest, IsCallingHapToken_Hap, TestSize.Level1)
{
    g_mockTokenTypeFlag = 0;
    EXPECT_TRUE(IpcCallerAuth::IsCallingHapToken());
}

HWTEST_F(IpcCallerAuthTest, IsCallingHapToken_NotHap, TestSize.Level1)
{
    g_mockTokenTypeFlag = 1;
    EXPECT_FALSE(IpcCallerAuth::IsCallingHapToken());
}

/* ---------- GetCallingBundleOrNativeProcessName ---------- */

HWTEST_F(IpcCallerAuthTest, GetCallingName_NativeSuccess, TestSize.Level1)
{
    g_mockTokenTypeFlag = 1;
    g_mockNativeProcessName = "foundation";
    g_mockGetNativeTokenInfoResult = 0;
    EXPECT_EQ(IpcCallerAuth::GetCallingBundleOrNativeProcessName(), "foundation");
}

HWTEST_F(IpcCallerAuthTest, GetCallingName_NativeFailed, TestSize.Level1)
{
    g_mockTokenTypeFlag = 1;
    g_mockGetNativeTokenInfoResult = -1;
    EXPECT_TRUE(IpcCallerAuth::GetCallingBundleOrNativeProcessName().empty());
}

HWTEST_F(IpcCallerAuthTest, GetCallingName_HapSuccess, TestSize.Level1)
{
    g_mockTokenTypeFlag = 0;
    g_mockHapBundleName = "com.example.app";
    g_mockGetHapTokenInfoResult = 0;
    EXPECT_EQ(IpcCallerAuth::GetCallingBundleOrNativeProcessName(), "com.example.app");
}

HWTEST_F(IpcCallerAuthTest, GetCallingName_HapFailed, TestSize.Level1)
{
    g_mockTokenTypeFlag = 0;
    g_mockGetHapTokenInfoResult = -1;
    EXPECT_TRUE(IpcCallerAuth::GetCallingBundleOrNativeProcessName().empty());
}

HWTEST_F(IpcCallerAuthTest, GetCallingName_UnknownToken, TestSize.Level1)
{
    g_mockTokenTypeFlag = 2;
    EXPECT_TRUE(IpcCallerAuth::GetCallingBundleOrNativeProcessName().empty());
}

} // namespace StorageSpaceManager
} // namespace OHOS
