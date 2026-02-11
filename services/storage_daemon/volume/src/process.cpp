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

#include "volume/process.h"

#include <csignal>
#include <dirent.h>
#include <unistd.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"

using namespace std;

namespace OHOS {
namespace StorageDaemon {
Process::Process(std::string path)
{
    path_ = path;
}

std::unordered_set<pid_t> Process::GetPids()
{
    return pids_;
}

std::string Process::GetPath()
{
    return path_;
}

std::string Process::Readlink(std::string path)
{
    int len = 0;
    int size = 0;
    int growlen = 64;
    std::string buf;

    do {
        size += growlen;
        buf.assign(std::string(size, '\0'));
        len = readlink(path.c_str(), buf.data(), size);
        if (len == -1) {
            return "";
        }
    } while (size <= len);

    return buf;
}

bool Process::CheckSubDir(std::string subdir)
{
    const char *p = path_.c_str();
    const char *q = subdir.c_str();

    while (*p != '\0' && *q != '\0') {
        if (*p != *q) {
            return false;
        }
        p++;
        q++;
    }

    if (*p == '\0' && *q == '\0') {
        return true;
    }

    if (*p == '\0' && *q == '/') {
        return true;
    }

    return false;
}

bool Process::CheckMaps(std::string pidPath)
{
    char *buf = nullptr;
    size_t lineLen = 0;
    std::string line;
    auto path = StringPrintf("%s/maps", pidPath.c_str());
    FILE *file = fopen(path.c_str(), "r");
    if (file == nullptr) {
        return false;
    }

    while (getline(&buf, &lineLen, file) > 0) {
        line = buf;
        std::string::size_type pos = line.find('/');
        if (pos != line.npos) {
            line = line.substr(pos);
            if (CheckSubDir(line)) {
                LOGI("Found map in %{public}s", pidPath.c_str());
                (void)fclose(file);
                free(buf);
                buf = nullptr;
                return true;
            }
        }
    }

    (void)fclose(file);
    free(buf);
    buf = nullptr;
    return false;
}

bool Process::CheckSymlink(std::string path)
{
    std::string link = Readlink(path);
    if (!link.empty() && CheckSubDir(link)) {
        return true;
    }
    return false;
}

bool Process::CheckFds(std::string pidPath)
{
    struct dirent *dirEntry;
    auto path = StringPrintf("%s/fd", pidPath.c_str());
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return E_ERR;
    }

    while ((dirEntry = readdir(dir)) != nullptr) {
        if (dirEntry->d_type != DT_LNK) continue;
        if (CheckSymlink(path + "/" + dirEntry->d_name)) {
            (void)closedir(dir);
            return true;
        }
    }

    (void)closedir(dir);
    return false;
}

int32_t Process::UpdatePidAndKill(int signal)
{
    if (signal == 0) {
        return E_OK;
    }

    struct dirent *dirEntry;
    DIR *dir = opendir("/proc");
    if (dir == nullptr) {
        return E_ERR;
    }

    while ((dirEntry = readdir(dir)) != nullptr) {
        if (dirEntry->d_type != DT_DIR) continue;
        pid_t pid = atoi(dirEntry->d_name);
        if (pid > 0 && pid != getprocpid()) {
            std::string pidPath = StringPrintf("/proc/%d", pid);
            if (CheckMaps(pidPath)
                || CheckSymlink(pidPath + "/cwd")
                || CheckSymlink(pidPath + "/root")
                || CheckSymlink(pidPath + "/exe")) {
                LOGI("KILL PID %{public}d immediately", pid);
                kill(pid, signal);
            }
        }
    }
    (void)closedir(dir);
    return E_OK;
}
} // StorageDaemon
} // OHOS
