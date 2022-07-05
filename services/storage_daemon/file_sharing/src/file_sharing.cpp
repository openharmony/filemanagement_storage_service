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
#include <string>

#include <sys/xattr.h>

#include "file_sharing/acl.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
const std::string FSCRYPT_EL1_PUBLIC = "/data/service/el1/public";
const std::string STORAGE_DAEMON_DIR = FSCRYPT_EL1_PUBLIC + "/storage_daemon";
const std::string FILE_SHARING_DIR = STORAGE_DAEMON_DIR + "/share";
const std::string PUBLIC_DIR = FILE_SHARING_DIR + "/public";
constexpr mode_t STORAGE_DAEMON_DIR_MODE = 0711;
constexpr mode_t FILE_SHARING_DIR_MODE = 0750;
constexpr mode_t PUBLIC_DIR_MODE = 0770;
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr uid_t ROOT_UID = 0;
constexpr gid_t ROOT_GID = 0;
}

int SetupFileSharingDir()
{
    if (!IsDir(FSCRYPT_EL1_PUBLIC)) {
        LOGE("No directory for filesystem encryption EL1");
        return -1;
    }

    bool success = PrepareDir(STORAGE_DAEMON_DIR, STORAGE_DAEMON_DIR_MODE,
                              ROOT_UID, ROOT_GID);
    if (!success) {
        LOGE("Failed to properly set up directory of storage daemon");
        return -1;
    }

    success = PrepareDir(FILE_SHARING_DIR, FILE_SHARING_DIR_MODE,
                         FILE_MANAGER_UID, FILE_MANAGER_GID);
    if (!success) {
        LOGE("Failed to properly set up directory of file sharing");
        return -1;
    }

    success = PrepareDir(PUBLIC_DIR, PUBLIC_DIR_MODE,
                         FILE_MANAGER_UID, FILE_MANAGER_GID);
    if (!success) {
        LOGE("Failed to properly set up directory of public file sharing");
        return -1;
    }

    /* Skip setting default ACL if it's been done */
    if (getxattr(PUBLIC_DIR.c_str(), ACL_XATTR_DEFAULT, nullptr, 0) > 0) {
        return 0;
    }

    /*
     * We have to use numeric id (1006) instead of character string ("file_manager")
     * here due to the name-id mismatch in the in-house system.
     */
    return AclSetDefault(PUBLIC_DIR, "g:1006:rwx");
}
} // namespace StorageDaemon
} // namespace OHOS
