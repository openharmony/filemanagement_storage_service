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

const inline std::set<std::string> SANDBOX_EXCLUDE_PATH = {
    "chipset",
    "system",
    "com.ohos.render"
};
const inline std::vector<std::string> CRYPTO_SANDBOX_PATH = {
    "/data/storage/el2/base/",
    "/data/storage/el2/database/",
    "/data/storage/el2/share/",
    "/data/storage/el2/log/",
    "/data/storage/el2/distributedfiles/",
    "/data/storage/el2/cloud/",
    "/data/storage/el3/base/",
    "/data/storage/el3/database/",
    "/data/storage/el4/base/",
    "/data/storage/el4/database/",
    "/data/storage/el5/base/",
    "/data/storage/el5/database/"
};
const inline std::vector<std::string> CRYPTO_SRC_PATH = {
    "/data/app/el2/<currentUserId>/base/<bundleName>/",
    "/data/app/el2/<currentUserId>/database/<bundleName>/",
    "/mnt/share/<currentUserId>/<bundleName>/",
    "/data/app/el2/<currentUserId>/log/<bundleName>/",
    "/mnt/hmdfs/<currentUserId>/account/merge_view/data/<bundleName>/",
    "/mnt/hmdfs/<currentUserId>/cloud/data/<bundleName>/",
    "/data/app/el3/<currentUserId>/base/<bundleName>/",
    "/data/app/el3/<currentUserId>/database/<bundleName>/",
    "/data/app/el4/<currentUserId>/base/<bundleName>/",
    "/data/app/el4/<currentUserId>/database/<bundleName>/",
    "/data/app/el5/<currentUserId>/base/<bundleName>/",
    "/data/app/el5/<currentUserId>/database/<bundleName>/"
};

const inline std::vector<std::string> APPDATA_DST_PATH = {
    "/mnt/user/<currentUserId>/nosharefs/appdata/el1/base/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el2/base/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el2/cloud/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el2/distributedfiles/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el5/base/"
};

const inline std::vector<std::string> APPDATA_SRC_PATH = {
    "/data/app/el1/<currentUserId>/base/",
    "/data/app/el2/<currentUserId>/base/",
    "/mnt/hmdfs/<currentUserId>/cloud/data/",
    "/mnt/hmdfs/<currentUserId>/account/merge_view/data/",
    "/data/app/el5/<currentUserId>/base/"
};

const inline std::vector<std::string> FD_PATH = {
    "/data/service/el2/<currentUserId>",
    "/data/service/el3/<currentUserId>",
    "/data/service/el4/<currentUserId>",
    "/data/service/el5/<currentUserId>",
    "/storage/media/<currentUserId>"
};

const inline std::vector<std::string> SYS_PATH = {
    "/mnt/hmdfs/<currentUserId>/account",
    "/mnt/hmdfs/<currentUserId>/non_account",
    "/mnt/hmdfs/<currentUserId>/cloud"
};

const inline std::vector<std::string> HMDFS_SUFFIX = {"account", "non_account", "cloud"};

constexpr const int32_t HMDFS_VAL_LEN = 6;
constexpr const int32_t HMDFS_TRUE_LEN = 5;

constexpr int MODE_0711 = 0711;
constexpr int MODE_0771 = 0771;
constexpr int MODE_02771 = 02771;
constexpr int32_t REMOUNT_VALUE_LEN = 10;

constexpr uid_t OID_ROOT = 0;
constexpr uid_t OID_SYSTEM = 1000;
constexpr uid_t OID_FILE_MANAGER = 1006;
constexpr uid_t OID_USER_DATA_RW = 1008;
constexpr uid_t OID_DFS = 1009;
constexpr uid_t OID_MEDIA = 1013;
constexpr uid_t OID_BACKUP = 1089;
constexpr uid_t OID_DFS_SHARE = 3822;
constexpr uid_t OID_TEE = 6668;
constexpr uid_t OID_DEVICE_AUTH = 3333;
constexpr uid_t OID_HUKS = 3510;
constexpr uid_t OID_ASSET = 6226;
constexpr uid_t OID_DDMS = 3012;
constexpr uid_t OID_HWID = 7008;
constexpr uid_t OID_HEALTH_SPORT = 7259;
constexpr uid_t OID_DLP_CREDENTIAL = 3553;
constexpr uid_t OID_RSS = 1096;
constexpr uid_t OID_HIVIEW = 1201;
constexpr uid_t OID_PARENT_CONTROL = 7007;
constexpr uid_t OID_ACCOUNT = 3058;
constexpr uid_t OID_COLLABORATION_FWK = 5520;
constexpr uid_t OID_CLOUD_BACK = 5206;
constexpr uid_t OID_AV_SESSION = 6700;
constexpr uid_t USER_ID_BASE = 200000;
constexpr uid_t OID_CLOUD_DEVELOP_PROXY = 7996;
constexpr uid_t OID_MEDIA_ENHANCE_SERVICE = 7998;
constexpr uid_t OID_PUSH = 7023;
constexpr uid_t OID_GAMESERVICE_SERVER = 7011;
constexpr uid_t OID_HWF_SERVICE = 7700;
constexpr uid_t OID_FOUNDATION = 5523;
constexpr uid_t OID_PASTEBOARD = 3816;
constexpr uid_t OID_PRINT = 3823;
constexpr uid_t OID_FINDNETWORK = 7518;
// Proprietary service, not for open-source use.
constexpr uid_t OID_GLASSES_COLLABORATION_SERVICE = 7140;
constexpr uid_t OID_TRUSTED_RING = 65936;
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_MOUNT_CONSTANT_H

