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

/**
 * @file ipc_caller_auth.h
 * @brief IPC caller authentication utility: wraps AccessTokenKit, IPCSkeleton, etc.
 *
 * Does not include business-specific whitelists (specific UIDs, process names,
 * exemption conditions, etc.); callers compose those at the business layer.
 */

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_IPC_CALLER_AUTH_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_IPC_CALLER_AUTH_H

#include <cstdint>
#include <functional>
#include <string>

namespace OHOS {
namespace StorageSpaceManager {

inline const std::string PERMISSION_STORAGE_MANAGER = "ohos.permission.STORAGE_MANAGER";
inline const std::string PERMISSION_MOUNT_MANAGER = "ohos.permission.MOUNT_UNMOUNT_MANAGER";
inline const std::string PERMISSION_FORMAT_MANAGER = "ohos.permission.MOUNT_FORMAT_MANAGER";

/**
 * @class IpcCallerAuth
 * @brief Generic authentication & caller info reader based on
 *     current IPC thread context (IPCSkeleton + AccessTokenKit).
 */
class IpcCallerAuth {
public:
    /** Verify the current IPC client holds the specified declared permission (AccessTokenKit::VerifyAccessToken). */
    static bool VerifyCallerPermission(const std::string &permissionName);

    /**
     * First checks via business callback whether the caller is trusted;
     * if not, falls back to VerifyCallerPermission.
     * @param permissionName Permission name to verify.
     * @param trustWithoutPermission If returns true, skips token permission check.
     */
    static bool VerifyCallerPermissionUnlessTrusted(const std::string &permissionName,
                                                    const std::function<bool()> &trustWithoutPermission);

    /** Returns true if the caller is a system app (HAP must be system, non-HAP like native SA returns true). */
    static bool IsCallingSystemApp();

    /** Compares native caller process name and UID, returns true only if both match (uses GetNativeTokenInfo). */
    static bool VerifyNativeCallerMatches(const std::string &expectedProcessName, int32_t expectedUid);

    /** Returns the current IPC caller UID (IPCSkeleton::GetCallingUid). */
    static int32_t GetCallingUid();

    /** Returns true if the current IPC caller UID equals expectedUid. */
    static bool IsCallingUid(int32_t expectedUid);

    /** Returns true if the current thread IPC token type is TOKEN_NATIVE. */
    static bool IsCallingNativeToken();

    /** Returns true if the current thread IPC token type is TOKEN_HAP. */
    static bool IsCallingHapToken();

    /** Reads caller identity by token type: process name for native, bundleName for HAP; empty string otherwise. */
    static std::string GetCallingBundleOrNativeProcessName();

    /** Utility class, construction prohibited. */
    IpcCallerAuth() = delete;

    /** Utility class, destruction prohibited. */
    ~IpcCallerAuth() = delete;
};

/** Alias for IpcCallerAuth, representing a collection of IPC-side authentication static methods. */
using PermissionChecker = IpcCallerAuth;

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_IPC_CALLER_AUTH_H
