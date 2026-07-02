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

#include "ipc_caller_auth.h"

#include "storage_space_manager_hilog.h"

#include "accesstoken_kit.h"
#include "errors.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace StorageSpaceManager {

namespace {
using Security::AccessToken::AccessTokenID;
using Security::AccessToken::AccessTokenKit;
constexpr int32_t GRANTED = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
} // namespace

bool IpcCallerAuth::VerifyCallerPermission(const std::string &permissionName)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    int32_t res = AccessTokenKit::VerifyAccessToken(tokenCaller, permissionName);
    if (res == GRANTED) {
        LOGD("IpcCallerAuth: permission granted for %{public}s", permissionName.c_str());
        return true;
    }
    LOGE("IpcCallerAuth: permission denied, need %{public}s", permissionName.c_str());
    return false;
}

bool IpcCallerAuth::VerifyCallerPermissionUnlessTrusted(const std::string &permissionName,
                                                        const std::function<bool()> &trustWithoutPermission)
{
    if (trustWithoutPermission && trustWithoutPermission()) {
        LOGD("IpcCallerAuth: trusted caller, skip permission %{public}s", permissionName.c_str());
        return true;
    }
    return VerifyCallerPermission(permissionName);
}

bool IpcCallerAuth::IsCallingSystemApp()
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenType == Security::AccessToken::TOKEN_HAP) {
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        return AccessTokenKit::IsSystemAppByFullTokenID(fullTokenId);
    }
    return true;
}

bool IpcCallerAuth::VerifyNativeCallerMatches(const std::string &expectedProcessName, int32_t expectedUid)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    int32_t ret = AccessTokenKit::GetNativeTokenInfo(tokenCaller, nativeInfo);
    if (ret != ERR_OK) {
        LOGE("IpcCallerAuth: GetNativeTokenInfo failed ret=%{public}d", ret);
        return false;
    }

    int32_t uid = IPCSkeleton::GetCallingUid();
    if (nativeInfo.processName != expectedProcessName || uid != expectedUid) {
        LOGE("IpcCallerAuth: native caller mismatch, process=%{public}s uid=%{public}d",
             nativeInfo.processName.c_str(), uid);
        return false;
    }
    return true;
}

int32_t IpcCallerAuth::GetCallingUid()
{
    return IPCSkeleton::GetCallingUid();
}

bool IpcCallerAuth::IsCallingUid(int32_t expectedUid)
{
    return IPCSkeleton::GetCallingUid() == expectedUid;
}

bool IpcCallerAuth::IsCallingNativeToken()
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    return AccessTokenKit::GetTokenTypeFlag(tokenCaller) == Security::AccessToken::TOKEN_NATIVE;
}

bool IpcCallerAuth::IsCallingHapToken()
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    return AccessTokenKit::GetTokenTypeFlag(tokenCaller) == Security::AccessToken::TOKEN_HAP;
}

std::string IpcCallerAuth::GetCallingBundleOrNativeProcessName()
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenType == Security::AccessToken::TOKEN_NATIVE) {
        Security::AccessToken::NativeTokenInfo tokenInfo;
        if (AccessTokenKit::GetNativeTokenInfo(tokenCaller, tokenInfo) != ERR_OK) {
            LOGE("IpcCallerAuth: GetNativeTokenInfo failed");
            return "";
        }
        return tokenInfo.processName;
    }
    if (tokenType == Security::AccessToken::TOKEN_HAP) {
        Security::AccessToken::HapTokenInfo hapInfo;
        if (AccessTokenKit::GetHapTokenInfo(tokenCaller, hapInfo) != ERR_OK) {
            LOGE("IpcCallerAuth: GetHapTokenInfo failed");
            return "";
        }
        return hapInfo.bundleName;
    }
    return "";
}

} // namespace StorageSpaceManager
} // namespace OHOS
