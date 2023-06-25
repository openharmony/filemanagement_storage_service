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

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#ifdef USE_LIBRESTORECON
#include "policycoreutils.h"
#endif
namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t ALL_PERMS = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
const int BUF_LEN = 1024;

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

int32_t UMount2(const std::string &path, int flag)
{
    return TEMP_FAILURE_RETRY(umount2(path.c_str(), flag));
}

bool IsDir(const std::string &path)
{
    // check whether the path exists
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret) {
        return false;
    }

    return S_ISDIR(st.st_mode);
}

bool MkDirRecurse(const std::string& path, mode_t mode)
{
    std::string::size_type index = 0;
    do {
        std::string subPath = path;
        index = path.find('/', index + 1);
        if (index != std::string::npos) {
            subPath = path.substr(0, index);
        }

        if (TEMP_FAILURE_RETRY(access(subPath.c_str(), F_OK)) != 0) {
            if (MkDir(subPath, mode) != 0 && errno != EEXIST) {
                return false;
            }
        }
    } while (index != std::string::npos);

    return TEMP_FAILURE_RETRY(access(path.c_str(), F_OK)) == 0;
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

    mode_t mask = umask(0);
    if (MkDir(path, mode)) {
        LOGE("failed to mkdir, errno %{public}d", errno);
        umask(mask);
        return false;
    }
    umask(mask);

    if (ChMod(path, mode)) {
        LOGE("failed to chmod, errno %{public}d", errno);
        return false;
    }

    if (ChOwn(path, uid, gid)) {
        LOGE("failed to chown, errno %{public}d", errno);
        return false;
    }

#ifdef USE_LIBRESTORECON
    int err = Restorecon(path.c_str());
    if (err) {
        LOGE("failed to restorecon, err:%{public}d", err);
        return false;
    }
#endif

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
                (void)closedir(dir);
                return false;
            }
        } else {
            if (unlink((path + "/" + ent->d_name).c_str())) {
                LOGE("failed to unlink file %{public}s, errno %{public}d", ent->d_name, errno);
                (void)closedir(dir);
                return false;
            }
        }
    }

    (void)closedir(dir);
    if (rmdir(path.c_str())) {
        LOGE("failed to rm dir %{public}s, errno %{public}d", path.c_str(), errno);
        return false;
    }

    return true;
}

void TravelChmod(std::string path, mode_t mode)
{
    struct stat st;
    DIR *d = NULL;
    struct dirent *dp = NULL;
    const char *skip1 = ".";
    const char *skip2 = "..";

    if (stat(path.c_str(), &st) < 0 || !S_ISDIR(st.st_mode)) {
        LOGE("invalid path");
        return;
    }

    (void)ChMod(path, mode);
    if (!(d = opendir(path.c_str()))) {
        LOGE("opendir failed");
        return;
    }

    while ((dp = readdir(d)) != NULL) {
        if ((!strncmp(dp->d_name, skip1, strlen(skip1))) || (!strncmp(dp->d_name, skip2, strlen(skip2))))
            continue;

        std::string subpath = path + "/" + dp->d_name;
        stat(subpath.c_str(), &st);
        (void)ChMod(subpath, mode);
        if (S_ISDIR(st.st_mode))
            TravelChmod(subpath, mode);
    }
    (void)closedir(d);
}

bool StringToUint32(const std::string &str, uint32_t &num)
{
    if (str.empty()) {
        return false;
    }
    if (!IsNumericStr(str)) {
        LOGE("Not numeric entry");
        return false;
    }

    int value;
    if (!StrToInt(str, value)) {
        LOGE("String to int convert failed");
        return false;
    }
    num = static_cast<uint32_t>(value);

    return true;
}

void GetSubDirs(const std::string &path, std::vector<std::string> &dirList)
{
    dirList.clear();

    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGE("path is not dir");
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("failed to open dir %{public}s, errno %{public}d", path.c_str(), errno);
        return;
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if ((ent->d_type != DT_DIR) ||
            (strcmp(ent->d_name, ".") == 0) ||
            (strcmp(ent->d_name, "..") == 0)) {
            continue;
        }
        dirList.push_back(ent->d_name);
    }

    (void)closedir(dir);
}

void ReadDigitDir(const std::string &path, std::vector<FileList> &dirInfo)
{
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGE("path is not dir");
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("failed to open dir %{public}s, errno %{public}d", path.c_str(), errno);
        return;
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if ((ent->d_type != DT_DIR) ||
            (strcmp(ent->d_name, ".") == 0) ||
            (strcmp(ent->d_name, "..") == 0)) {
            continue;
        }

        uint32_t userId;
        std::string name(ent->d_name);
        if (!StringToUint32(name, userId)) {
            continue;
        }
        FileList entry = {
            .userId = userId,
            .path = path + "/" + name
        };
        dirInfo.push_back(entry);
    }

    (void)closedir(dir);
}

bool ReadFile(std::string path, std::string *str)
{
    std::ifstream infile;
    int cnt = 0;

    std::string rpath(PATH_MAX + 1, '\0');
    if ((path.length() > PATH_MAX) || (realpath(path.c_str(), rpath.data()) == nullptr)) {
        LOGE("realpath failed");
        return false;
    }

    infile.open(rpath.c_str());
    if (!infile) {
        LOGE("Cannot open file");
        return false;
    }

    while (1) {
        std::string subStr;
        infile >> subStr;
        if (subStr == "") {
            break;
        }
        cnt++;
        *str = *str + subStr + '\n';
    }

    infile.close();
    return cnt == 0 ? false : true;
}

static std::vector<char*> FromatCmd(std::vector<std::string> &cmd)
{
    std::vector<char*>res;
    res.reserve(cmd.size() + 1);

    for (auto& line : cmd) {
        LOGI("cmd %{public}s", line.c_str());
        res.emplace_back(const_cast<char*>(line.c_str()));
    }
    res.emplace_back(nullptr);

    return res;
}

int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output)
{
    int pipe_fd[2];
    pid_t pid;
    int status;
    auto args = FromatCmd(cmd);

    if (pipe(pipe_fd) < 0) {
        LOGE("creat pipe failed");
        return E_ERR;
    }

    pid = fork();
    if (pid == -1) {
        LOGE("fork failed");
        return E_ERR;
    } else if (pid == 0) {
        (void)close(pipe_fd[0]);
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            LOGE("dup2 failed");
            exit(1);
        }
        (void)close(pipe_fd[1]);
        execvp(args[0], const_cast<char **>(args.data()));
        exit(0);
    } else {
        (void)close(pipe_fd[1]);
        if (output) {
            char buf[BUF_LEN] = { 0 };
            (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
            output->clear();
            while (read(pipe_fd[0], buf, BUF_LEN - 1) > 0) {
                LOGI("get result %{public}s", buf);
                output->push_back(buf);
            }
            return E_OK;
        }

        waitpid(pid, &status, 0);
        if (errno == ECHILD) {
            return E_NO_CHILD;
        }
        if (!WIFEXITED(status)) {
            LOGE("Process exits abnormally");
            return E_ERR;
        }
    }
    return E_OK;
}

void TraverseDirUevent(const std::string &path, bool flag)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    int dirFd = dirfd(dir);
    int fd = openat(dirFd, "uevent", O_WRONLY | O_CLOEXEC);
    if (fd >= 0) {
        std::string writeStr = "add\n";
        write(fd, writeStr.c_str(), writeStr.length());
        (void)close(fd);
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        if (ent->d_type != DT_DIR && !flag) {
            continue;
        }

        TraverseDirUevent(path + "/" + ent->d_name, false);
    }

    (void)closedir(dir);
}
} // STORAGE_DAEMON
} // OHOS
