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
 * @brief IPC 侧调用方鉴权工具：封装 AccessTokenKit、IPCSkeleton 等通用能力。
 *
 * 不包含具体业务白名单（特定 UID、进程名、免检条件等），由调用方在业务层自行组合。
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
 * @brief 基于当前 IPC 线程上下文（IPCSkeleton + AccessTokenKit）的通用鉴权与调用方信息读取。
 */
class IpcCallerAuth {
public:
    /** 校验当前 IPC 客户端是否持有指定声明权限（AccessTokenKit::VerifyAccessToken）。 */
    static bool VerifyCallerPermission(const std::string &permissionName);

    /**
     * 先按业务回调判定是否为可信调用方；否时再走 VerifyCallerPermission。
     * @param permissionName 待校验权限名。
     * @param trustWithoutPermission 返回 true 则跳过令牌权限校验。
     */
    static bool VerifyCallerPermissionUnlessTrusted(const std::string &permissionName,
                                                    const std::function<bool()> &trustWithoutPermission);

    /** 判断是否为系统应用：HAP 需系统应用，非 HAP（如原生 SA）视为 true。 */
    static bool IsCallingSystemApp();

    /** 比对原生调用方进程名与 UID，均与期望值一致返回 true（依赖 GetNativeTokenInfo）。 */
    static bool VerifyNativeCallerMatches(const std::string &expectedProcessName, int32_t expectedUid);

    /** 返回当前 IPC 调用方 UID（IPCSkeleton::GetCallingUid）。 */
    static int32_t GetCallingUid();

    /** 判断当前 IPC 调用方 UID 是否等于 expectedUid。 */
    static bool IsCallingUid(int32_t expectedUid);

    /** 判断当前线程 IPC 令牌类型是否为 TOKEN_NATIVE。 */
    static bool IsCallingNativeToken();

    /** 判断当前线程 IPC 令牌类型是否为 TOKEN_HAP。 */
    static bool IsCallingHapToken();

    /** 按令牌类型读取调用方标识：原生为进程名，HAP 为 bundleName；其它或未取到为空串。 */
    static std::string GetCallingBundleOrNativeProcessName();

    /** 工具类禁止构造。 */
    IpcCallerAuth() = delete;

    /** 工具类禁止析构。 */
    ~IpcCallerAuth() = delete;
};

/** IpcCallerAuth 别名，语义为 IPC 侧鉴权相关静态方法集合。 */
using PermissionChecker = IpcCallerAuth;

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_IPC_CALLER_AUTH_H
