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
#ifndef OHOS_STORAGE_DAEMON_FILE_SHARING_H
#define OHOS_STORAGE_DAEMON_FILE_SHARING_H

#include <string>

#include <sys/xattr.h>

#include "file_sharing/acl.h"
#include "parameter.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
const std::string FSCRYPT_EL1_PUBLIC = "/data/service/el1/public";
const std::string STORAGE_DAEMON_EL1_DIR = FSCRYPT_EL1_PUBLIC + "/storage_daemon";
const std::string FILE_SHARING_DIR = STORAGE_DAEMON_EL1_DIR + "/share";
const std::string PUBLIC_DIR = FILE_SHARING_DIR + "/public";
const std::string SHARE_TOB_DIR = STORAGE_DAEMON_EL1_DIR + "/share_tob";
const std::string TOB_SCENE = "2b_share";
const std::string TOC_SCENE = "2c_share";
const std::string TOD_SCENE = "2c2b_share";
static const char *SHARE_DIR_ENABLE_PARAMETER = "const.file_transfer.fs_share_dir_define";
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

int SetupFileSharingDir();
int PrepareFileSharingDir(const std::string &fsShareParam);
int SetupDirAcl(const std::string &fsShareParam);
std::string GetFileShareDefineParameter();
} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_FILE_SHARING_H
