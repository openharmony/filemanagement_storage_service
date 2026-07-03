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

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "ipc_caller_auth_mock.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    (void)tokenID;
    (void)permissionName;
    return g_mockVerifyAccessTokenResult;
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    (void)tokenID;
    return static_cast<ATokenTypeEnum>(g_mockTokenTypeFlag);
}

bool AccessTokenKit::IsSystemAppByFullTokenID(uint64_t fullTokenId)
{
    (void)fullTokenId;
    return g_mockIsSystemApp;
}

int32_t AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
{
    (void)tokenID;
    nativeTokenInfoRes.processName = g_mockNativeProcessName;
    return g_mockGetNativeTokenInfoResult;
}

int32_t AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    (void)tokenID;
    hapInfo.bundleName = g_mockHapBundleName;
    return g_mockGetHapTokenInfoResult;
}

} // namespace AccessToken
} // namespace Security

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return g_mockCallingTokenId;
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    return g_mockCallingFullTokenId;
}

int32_t IPCSkeleton::GetCallingUid()
{
    return g_mockCallingUid;
}

} // namespace OHOS

// Global mock control variables
uint32_t g_mockCallingTokenId = 0;
uint64_t g_mockCallingFullTokenId = 0;
int32_t g_mockCallingUid = 0;
int32_t g_mockVerifyAccessTokenResult = 0;
uint32_t g_mockTokenTypeFlag = 1;  // TOKEN_NATIVE
bool g_mockIsSystemApp = true;
int32_t g_mockGetNativeTokenInfoResult = 0;
std::string g_mockNativeProcessName = "mock_process";
int32_t g_mockGetHapTokenInfoResult = 0;
std::string g_mockHapBundleName = "mock_bundle";
