/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_MOUNT_CONSTANT_H
#define OHOS_STORAGE_DAEMON_MOUNT_CONSTANT_H

#include <string>
#include <set>
#include <vector>

namespace OHOS {
namespace StorageDaemon {
constexpr const char *SANDBOX_ROOT_PATH = "/mnt/sandbox/";
constexpr const char *CURRENT_USER_ID_FLAG = "<currentUserId>";
constexpr const char *PACKAGE_NAME_FLAG = "<bundleName>";
constexpr const char *MOUNT_POINT_INFO = "/proc/mounts";
constexpr const char *MOUNT_POINT_TYPE_HMDFS = "hmdfs";
constexpr const char *MOUNT_POINT_TYPE_HMFS = "hmfs";
constexpr const char *MOUNT_POINT_TYPE_F2FS = "f2fs";
constexpr const char *MOUNT_POINT_TYPE_SHAREFS = "sharefs";
constexpr const char *EL2_BASE = "/data/storage/el2/base/";
constexpr const char *MOUNT_SUFFIX = "_locked";
constexpr const char *APP_EL1_PATH = "/data/app/el1";
constexpr const char *FILE_MGR_ROOT_PATH = "/storage/Users/currentUser/";
constexpr const char *HMDFS_SYS_CAP = "const.distributed_file_property.enabled";
constexpr const char *SHARE_PATH = "/data/service/el1/public/storage_daemon/share/public";
constexpr const char *UN_REACHABLE = "(unreachable)";
constexpr const char *PID_CWD = "cwd";
constexpr const char *PID_EXE = "exe";
constexpr const char *PID_ROOT = "root";
constexpr const char *PID_FD = "fd";
constexpr const char *PID_PROC = "/proc";

const std::set<std::string> SANDBOX_EXCLUDE_PATH = {
    "chipset",
    "system",
    "com.ohos.render"
};

const std::vector<std::string> FD_PATH = {
    "/data/service/el2/<currentUserId>",
    "/data/service/el3/<currentUserId>",
    "/data/service/el4/<currentUserId>",
    "/data/service/el5/<currentUserId>",
    "/storage/media/<currentUserId>"
};

const std::vector<std::string> SYS_PATH = {
    "/mnt/hmdfs/<currentUserId>/account",
    "/mnt/hmdfs/<currentUserId>/non_account",
    "/mnt/hmdfs/<currentUserId>/cloud"
};

const std::vector<std::string> HMDFS_SUFFIX = {"account", "non_account", "cloud"};

constexpr const int32_t HMDFS_VAL_LEN = 6;
constexpr const int32_t HMDFS_TRUE_LEN = 5;

constexpr int MODE_0711 = 0711;
constexpr uid_t OID_SYSTEM = 1000;
constexpr uid_t OID_FILE_MANAGER = 1006;
constexpr uid_t OID_DFS = 1009;
constexpr uid_t USER_ID_BASE = 200000;
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_MOUNT_CONSTANT_H

