/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "file_sharing/file_sharing.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *FSCRYPT_EL1_PUBLIC = "/data/service/el1/public";
constexpr const char *STORAGE_DAEMON_EL1_DIR = "/data/service/el1/public/storage_daemon";
constexpr const char *FILE_SHARING_DIR = "/data/service/el1/public/storage_daemon/share";
constexpr const char *PUBLIC_DIR = "/data/service/el1/public/storage_daemon/share/public";
constexpr const char *SHARE_TOB_DIR = "/data/service/el1/public/storage_daemon/share_tob";
constexpr const char *TOB_SCENE = "2b_share";
constexpr const char *TOC_SCENE = "2c_share";
constexpr const char *TOD_SCENE = "2c2b_share";
constexpr const char *SHARE_DIR_ENABLE_PARAMETER = "const.file_transfer.fs_share_dir_define";

constexpr int32_t MAX_FS_DEFINE_VAL_LEN = 16;
constexpr mode_t STORAGE_DAEMON_DIR_MODE = 0711;
constexpr mode_t FILE_SHARING_DIR_MODE = 0750;
constexpr mode_t PUBLIC_DIR_MODE = 02770;
constexpr mode_t SHARE_TOB_DIR_MODE = 0770;
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr uid_t ROOT_UID = 0;
constexpr gid_t ROOT_GID = 0;
constexpr uid_t SHARE_TOB_UID = 7017;
constexpr gid_t SHARE_TOB_GID = 7017;

int SetupFileSharingDir()
{
    if (!IsDir(FSCRYPT_EL1_PUBLIC)) {
        LOGE("No directory for filesystem encryption EL1");
        return -1;
    }

    std::string fsShareParam = GetFileShareDefineParameter();
    if (PrepareFileSharingDir(fsShareParam) != 0) {
        LOGE("Failed to prepare file sharing directory of storage daemon");
        return -1;
    }

    return SetupDirAcl(fsShareParam);
}

int PrepareFileSharingDir(const std::string &fsShareParam)
{
    bool success = PrepareDir(STORAGE_DAEMON_EL1_DIR, STORAGE_DAEMON_DIR_MODE, ROOT_UID, ROOT_GID);
    if (!success) {
        LOGE("Prepare directory of storage daemon failed, path = %{public}s", STORAGE_DAEMON_EL1_DIR);
        return -1;
    }

    if (fsShareParam == TOB_SCENE || fsShareParam == TOD_SCENE) {
        success = PrepareDir(SHARE_TOB_DIR, SHARE_TOB_DIR_MODE, SHARE_TOB_UID, SHARE_TOB_GID);
        if (!success) {
            LOGE("Prepare directory for path = %{public}s failed, fsShareParam = %{public}s",
                 SHARE_TOB_DIR, fsShareParam.c_str());
            return -1;
        }
    }

    if (fsShareParam == TOC_SCENE || fsShareParam == TOD_SCENE) {
        success = PrepareDir(FILE_SHARING_DIR, FILE_SHARING_DIR_MODE, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!success) {
            LOGE("Prepare directory for path = %{public}s failed, fsShareParam = %{public}s",
                 FILE_SHARING_DIR, fsShareParam.c_str());
            return -1;
        }

        success = PrepareDir(PUBLIC_DIR, PUBLIC_DIR_MODE, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!success) {
            LOGE("Prepare directory for path = %{public}s failed, fsShareParam = %{public}s",
                 PUBLIC_DIR, fsShareParam.c_str());
            return -1;
        }
    }
    LOGI("Prepare file sharing directory success");
    return 0;
}

int SetupDirAcl(const std::string &fsShareParam)
{
    if (fsShareParam == TOB_SCENE || fsShareParam == TOD_SCENE) {
        if (getxattr(SHARE_TOB_DIR, ACL_XATTR_DEFAULT, nullptr, 0) <= 0) {
            int rc = AclSetDefault(SHARE_TOB_DIR, "g:7017:rwx");
            if (rc != 0) {
                LOGE("Set acl for dir = %{public}s failed, fsShareParam = %{public}s",
                     SHARE_TOB_DIR, fsShareParam.c_str());
                return -1;
            }
        }
    }

    if (fsShareParam == TOC_SCENE || fsShareParam == TOD_SCENE) {
        if (getxattr(PUBLIC_DIR, ACL_XATTR_DEFAULT, nullptr, 0) <= 0) {
            int rc = AclSetDefault(PUBLIC_DIR, "g:1006:rwx");
            if (rc != 0) {
                LOGE("Set acl for dir = %{public}s failed, fsShareParam = %{public}s",
                     PUBLIC_DIR, fsShareParam.c_str());
                return -1;
            }
        }
    }
    LOGI("Set acl success");
    return 0;
}

std::string GetFileShareDefineParameter()
{
    char fsShareParam[] = "2c_share";
    int ret = GetParameter(SHARE_DIR_ENABLE_PARAMETER, "", fsShareParam, MAX_FS_DEFINE_VAL_LEN);
    if (ret <= 0) {
        LOGE("GetParameter name = %{public}s error, ret = %{public}d, return default value",
             SHARE_DIR_ENABLE_PARAMETER, ret);
        return TOC_SCENE;
    }

    if ((strlen(fsShareParam) == 0) || (strlen(fsShareParam) > MAX_FS_DEFINE_VAL_LEN)) {
        LOGE("GetParameter success, but fsShareParam = %{public}s is invalid, return default value",
             fsShareParam);
        return TOC_SCENE;
    }

    if ((fsShareParam != TOB_SCENE) && (fsShareParam != TOC_SCENE) && (fsShareParam != TOD_SCENE)) {
        LOGE("GetParameter success, but fsShareParam = %{public}s is not expected, return default value",
             fsShareParam);
        return TOC_SCENE;
    }
    LOGI("GetParameter success, fsShareParam = %{public}s", fsShareParam);
    return fsShareParam;
}
} // namespace StorageDaemon
} // namespace OHOS

