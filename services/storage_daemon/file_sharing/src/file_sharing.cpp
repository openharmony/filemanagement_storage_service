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

#include "storage_service_log.h"
#include "file_sharing/acl.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
const std::string FSCRYPT_USER_EL1_PUBLIC = "/data/service/el1/public";
const std::string SERVICE_STORAGE_DAEMON_DIR = FSCRYPT_USER_EL1_PUBLIC + "/storage_daemon";
const std::string FILE_SHARING_DIR = SERVICE_STORAGE_DAEMON_DIR + "/share";
const std::string PUBLIC_DIR = FILE_SHARING_DIR + "/public";
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr uid_t ROOT_UID = 0;
constexpr gid_t ROOT_GID = 0;
}

int SetupFileSharingDir()
{
    if (!IsDir(FSCRYPT_USER_EL1_PUBLIC)) {
        LOGE("No directory for filesystem encryption EL1");
        return -1;
    }

    bool success = PrepareDir(SERVICE_STORAGE_DAEMON_DIR, 0711, ROOT_UID, ROOT_GID);
    if (!success) {
        LOGE("Failed to properly set up directory of storage daemon");
        return -1;
    }

    success = PrepareDir(FILE_SHARING_DIR, 0750, FILE_MANAGER_UID, FILE_MANAGER_GID);
    if (!success) {
        LOGE("Failed to properly set up directory of file sharing");
        return -1;
    }

    success = PrepareDir(PUBLIC_DIR, 0770, FILE_MANAGER_UID, FILE_MANAGER_GID);
    if (!success) {
        LOGE("Failed to properly set up directory of public file sharing");
        return -1;
    }

    /* Skip setting default ACL if it's been done */
    if (getxattr(PUBLIC_DIR.c_str(), ACL_XATTR_DEFAULT, nullptr, 0) > 0) {
        return 0;
    }

    int rc = AclSetDefault(PUBLIC_DIR, "g:file_manager:rwx");
    if (rc == -1) {
        LOGE("Failed to set default ACL for the public file sharing directory");
        return -1;
    }

    return 0;
}
} // namespace StorageDaemon
} // namespace OHOS
