/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "utils/file_utils.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "utils/errno.h"
#include "utils/log.h"

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t ALL_PERMS = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);

int32_t ChMod(const std::string &path, mode_t mode)
{
    return TEMP_FAILURE_RETRY(chmod(path.c_str(), mode));
}

int32_t ChOwn(const std::string &path, uid_t uid, gid_t gid)
{
    return TEMP_FAILURE_RETRY(chown(path.c_str(), uid, gid));
}

int32_t MkDir(const std::string &path, mode_t mode)
{
    return TEMP_FAILURE_RETRY(mkdir(path.c_str(), mode));
}

int32_t RmDir(const std::string &path)
{
    return TEMP_FAILURE_RETRY(rmdir(path.c_str()));
}

int32_t Mount(const std::string &source, const std::string &target, const char *type,
              unsigned long flags, const void *data)
{
    return TEMP_FAILURE_RETRY(mount(source.c_str(), target.c_str(), type, flags, data));
}

int32_t UMount(const std::string &path)
{
    return TEMP_FAILURE_RETRY(umount(path.c_str()));
}

// On success, true is returned.  On error, false is returned, and errno is set appropriately.
bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    LOGI("prepare for %{public}s", path.c_str());

    // check whether the path exists
    struct stat st;
    if (TEMP_FAILURE_RETRY(lstat(path.c_str(), &st)) == E_ERR) {
        if (errno != ENOENT) {
            LOGE("failed to lstat, errno %{public}d", errno);
            return false;
        }
    } else {
        if (!S_ISDIR(st.st_mode)) {
            LOGE("%{public}s exists and is not a directory", path.c_str());
            return false;
        }

        if (((st.st_mode & ALL_PERMS) != mode) && ChMod(path, mode)) {
            LOGE("dir exists and failed to chmod, errno %{public}d", errno);
            return false;
        }

        if (((st.st_uid != uid) || (st.st_gid != gid)) && ChOwn(path, uid, gid)) {
            LOGE("dir exists and failed to chown, errno %{public}d", errno);
            return false;
        }

        return true;
    }

    if (MkDir(path, mode)) {
        LOGE("failed to mkdir, errno %{public}d", errno);
        return false;
    }

    if (ChOwn(path, uid, gid)) {
        LOGE("failed to chown, errno %{public}d", errno);
        return false;
    }

    return true;
}

bool RmDirRecurse(const std::string &path)
{
    LOGI("rm dir %{public}s", path.c_str());

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        if (errno == ENOENT) {
            return true;
        }

        LOGE("failed to open dir %{public}s, errno %{public}d", path.c_str(), errno);
        return false;
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            if (!RmDirRecurse(path + "/" + ent->d_name)) {
                closedir(dir);
                return false;
            }
        } else {
            if (unlink((path + "/" + ent->d_name).c_str())) {
                LOGE("failed to unlink file %{public}s, errno %{public}d", ent->d_name, errno);
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);
    if (rmdir(path.c_str())) {
        LOGE("failed to rm dir %{public}s, errno %{public}d", path.c_str(), errno);
        return false;
    }

    return true;
}
} // STORAGE_DAEMON
} // OHOS