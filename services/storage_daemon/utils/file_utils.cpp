/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License,2.0 (the "License");
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

#include <cstdint>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parameters.h"
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#ifdef USE_LIBRESTORECON
#include "policycoreutils.h"
#endif
#ifdef EXTERNAL_STORAGE_QOS_TRANS
#include "concurrent_task_client.h"
#endif

using namespace std;
namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t ALL_PERMS = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
#ifdef EXTERNAL_STORAGE_QOS_TRANS
constexpr int SET_SCHED_LOAD_TRANS_TYPE = 10001;
#endif
constexpr int BUF_LEN = 1024;
constexpr int PIPE_FD_LEN = 2;
constexpr uint8_t KILL_RETRY_TIME = 5;
constexpr uint32_t KILL_RETRY_INTERVAL_MS = 100 * 1000;
constexpr int32_t MAX_STATISTICS_FILES_NUMBER = 5120000;
constexpr const char *MOUNT_POINT_INFO = "/proc/mounts";
#define RGM_MANAGER_PATH_DEF  "/data/service/el1/public/rgm_manager/data"
#define RGM_STATE_PRE_DEF "virt_service.rgm_state."
const std::string CONTAINER_HMOS = "rgm_hmos";
const std::string CONTAINER_LINUX = "rgm_linux";
const std::string VM_LINUX = "rgm_openEuler";
const std::string EL_RGM_MANAGER_PATH = "/data/service/el1/public/vm_manager";
const std::string RGM_MANAGER_PATH = RGM_MANAGER_PATH_DEF;

struct RgmPathConfig {
    bool isImg = false;
    std::string stateParam = "";
    std::string mgrPath = "";
    std::string imgDir = "";
    std::string imgPath = "";
    std::string configZipPath = "";
    std::string configVerifyDir = "";
    std::string configDir = "";
    std::string configBakDir = "";
    std::string businessPath = "";
    std::string rootfsPath = "";
};

const static std::map<std::string, RgmPathConfig> rgmConfigs = {
    {
        CONTAINER_HMOS, {
            true,
            RGM_STATE_PRE_DEF    "rgm_hmos",
            RGM_MANAGER_PATH_DEF "/rgm_hmos",
            RGM_MANAGER_PATH_DEF "/rgm_hmos/image",
            RGM_MANAGER_PATH_DEF "/rgm_hmos/image/agi.img",
            RGM_MANAGER_PATH_DEF "/rgm_hmos/image/config.zip",
            RGM_MANAGER_PATH_DEF "/rgm_hmos/config",
            RGM_MANAGER_PATH_DEF "/rgm_hmos/config/",
            RGM_MANAGER_PATH_DEF "/rgm_hmos/config.old",
            "/data/virt_service/rgm_hmos",
            "/data/virt_service/rgm_hmos/anco_hmos",
        }
    }, {
        CONTAINER_LINUX, {
            false,
            RGM_STATE_PRE_DEF    "rgm_linux",
            RGM_MANAGER_PATH_DEF "/rgm_linux",
            RGM_MANAGER_PATH_DEF "/rgm_linux/image",
            RGM_MANAGER_PATH_DEF "/rgm_linux/image/a.tgz",
            RGM_MANAGER_PATH_DEF "/rgm_linux/image/config.zip",
            RGM_MANAGER_PATH_DEF "/rgm_linux/config",
            RGM_MANAGER_PATH_DEF "/rgm_linux/config/",
            RGM_MANAGER_PATH_DEF "/rgm_linux/config.old",
            "/data/virt_service/rgm_linux",
            "/data/virt_service/rgm_linux/rootfs",
        }
    }, {
        VM_LINUX, {
            true,
            RGM_STATE_PRE_DEF    "rgm_openEuler",
            RGM_MANAGER_PATH_DEF "/vm_linux",
            RGM_MANAGER_PATH_DEF "/vm_linux/image",
            RGM_MANAGER_PATH_DEF "/vm_linux/image/a.tgz",
            RGM_MANAGER_PATH_DEF "/vm_linux/image/config.zip",
            RGM_MANAGER_PATH_DEF "/vm_linux/config",
            RGM_MANAGER_PATH_DEF "/vm_linux/config/",
            RGM_MANAGER_PATH_DEF "/vm_linux/config.old",
            "/data/virt_service/vm_linux",
            "",
        }
    }
};

int32_t RedirectStdToPipe(int logpipe[PIPE_FD_LEN], size_t len)
{
    if (logpipe == nullptr || len < PIPE_FD_LEN) {
        LOGE("std to pipe param is invalid.");
        return E_ERR;
    }
    int ret = E_OK;
    (void)close(logpipe[0]);
    if (dup2(logpipe[1], STDOUT_FILENO) == -1) {
        LOGE("dup2 stdout failed, errno is %{public}d.", errno);
        ret = E_ERR;
    }
    if (dup2(logpipe[1], STDERR_FILENO) == -1) {
        LOGE("dup2 stderr failed, errno is %{public}d.", errno);
        ret = E_ERR;
    }
    (void)close(logpipe[1]);
    return ret;
}

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
    const char *sourcePtr = nullptr;
    if (!source.empty()) {
        sourcePtr = source.c_str();
    }
    return TEMP_FAILURE_RETRY(mount(sourcePtr, target.c_str(), type, flags, data));
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

bool IsFile(const std::string &path)
{
    // check whether the path exists
    struct stat buf = {};
    if (stat(path.c_str(), &buf) != 0) {
        return false;
    }
    return S_ISREG(buf.st_mode);
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

int32_t PrepareDirSimple(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    LOGI("prepare for %{public}s", path.c_str());
    if (MkDir(path, mode) != 0) {
        if (errno == EEXIST) {
            LOGE("The path: %{public}s already exists.", path.c_str());
            return E_CREATE_USER_DIR_EXIST;
        }
        LOGE("failed to mkdir, errno %{public}d", errno);
        return E_MKDIR_ERROR;
    }
    if (ChMod(path, mode) != 0) {
        LOGE("failed to chmod, errno %{public}d", errno);
        return E_CHMOD_ERROR;
    }

    if (ChOwn(path, uid, gid) != 0) {
        LOGE("failed to chown, errno %{public}d", errno);
        return E_CHOWN_ERROR;
    }

#ifdef USE_LIBRESTORECON
    auto ret = Restorecon(path.c_str());
    if (ret != E_OK) {
        LOGE("failed to RestoreconDir, errno %{public}d", errno);
    }
    return ret;
#endif
    return E_OK;
}

// On success, true is returned.  On error, false is returned, and errno is set appropriately.
bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    LOGI("prepare for %{public}s", path.c_str());
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
            LOGE("dir exists and failed to chmod, errno %{public}d, uid %{public}d, gid %{public}d, mode %{public}d",
                 errno, st.st_uid, st.st_gid, st.st_mode);
            if (TEMP_FAILURE_RETRY(lstat(path.c_str(), &st)) == E_ERR) {
                LOGE("failed to lstat for chmod, errno %{public}d", errno);
            }
            return false;
        }
        if (((st.st_uid != uid) || (st.st_gid != gid)) && ChOwn(path, uid, gid)) {
            LOGE("dir exists and failed to chown, errno %{public}d, uid %{public}d, gid %{public}d, mode %{public}d",
                 errno, st.st_uid, st.st_gid, st.st_mode);
            if (TEMP_FAILURE_RETRY(lstat(path.c_str(), &st)) == E_ERR) {
                LOGE("failed to lstat for chown, errno %{public}d", errno);
            }
            return false;
        }
        return true;
    }
    mode_t mask = umask(0);
    if (MkDir(path, mode) != 0) {
        LOGE("failed to mkdir, errno %{public}d", errno);
        umask(mask);
        return false;
    }
    umask(mask);
    if (ChMod(path, mode) != 0) {
        LOGE("failed to chmod, errno %{public}d", errno);
        return false;
    }
    if (ChOwn(path, uid, gid) != 0) {
        LOGE("failed to chown, errno %{public}d", errno);
        return false;
    }
    return RestoreconDir(path);
}

bool RmDirRecurse(const std::string &path)
{
    LOGD("rm dir %{public}s", path.c_str());
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
                LOGE("failed to RmDirRecurse %{public}s, errno %{public}d", path.c_str(), errno);
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

void TravelChmod(const std::string &path, mode_t mode)
{
    struct stat st;
    DIR *d = nullptr;
    struct dirent *dp = nullptr;
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

    while ((dp = readdir(d)) != nullptr) {
        if ((!strncmp(dp->d_name, skip1, strlen(skip1))) || (!strncmp(dp->d_name, skip2, strlen(skip2)))) {
            continue;
        }
        std::string subpath = path + "/" + dp->d_name;
        stat(subpath.c_str(), &st);
        (void)ChMod(subpath, mode);
        if (S_ISDIR(st.st_mode)) {
            TravelChmod(subpath, mode);
        }
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
    if (value < 0 || value >= INT32_MAX) {
        return false;
    }
    num = static_cast<uint32_t>(value);
    return true;
}

bool StringToBool(const std::string &str, bool &result)
{
    if (str.empty()) {
        LOGE("String is empty.");
        return false;
    }

    if (str == "true") {
        result = true;
    } else if (str == "false") {
        result = false;
    } else {
        LOGE("Invalid boolean string: %{public}s", str.c_str());
        return false;
    }

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

void OpenSubFile(const std::string &path, std::vector<std::string>  &file)
{
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGI("path is not dir");
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGI("failed to open dir %{public}s, errno %{public}d", path.c_str(), errno);
        return;
    }
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if ((ent->d_type != DT_DIR)) {
            std::string name(ent->d_name);
            std::string filePath = path + "/" + name;
            LOGI("filePath is %{public}s", filePath.c_str());
            file.push_back(filePath);
            continue;
        } else {
            if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
                continue;
            }
            std::string name(ent->d_name);
            std::string filePath = path + "/" + name;
            OpenSubFile(filePath, file);
        }
    }
    (void)closedir(dir);
}

bool ReadFile(const std::string &path, std::string *str)
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
        LOGE("cmd %{public}s", line.c_str());
        res.emplace_back(const_cast<char*>(line.c_str()));
    }
    res.emplace_back(nullptr);

    return res;
}

static void ClosePipe(int pipedes[PIPE_FD_LEN], size_t len)
{
    if (pipedes == nullptr || len < PIPE_FD_LEN) {
        LOGE("close pipe param is invalid.");
        return;
    }
    (void)close(pipedes[0]);
    (void)close(pipedes[1]);
}

void GetExitStatus(int *exitStatus, int inputExitStatus)
{
    if (exitStatus != nullptr) {
        *exitStatus = inputExitStatus;
    }
}

int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output, int *exitStatus)
{
    int pipe_fd[PIPE_FD_LEN];
    pid_t pid;
    int status;
    auto args = FromatCmd(cmd);
    if (pipe(pipe_fd) < 0) {
        LOGE("creat pipe failed, errno is %{public}d.", errno);
        return E_CREATE_PIPE;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("fork failed, errno is %{public}d.", errno);
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        return E_FORK;
    } else if (pid == 0) {
        if (RedirectStdToPipe(pipe_fd, PIPE_FD_LEN)) {
            _exit(1);
        }
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("execvp failed errno: %{public}d", errno);
        _exit(1);
    } else {
        (void)close(pipe_fd[1]);
        if (output) {
            char buf[BUF_LEN] = { 0 };
            (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
            output->clear();
            while (read(pipe_fd[0], buf, BUF_LEN - 1) > 0) {
                LOGE("get result %{public}s", buf);
                output->push_back(buf);
            }
        }
        (void)close(pipe_fd[0]);
        waitpid(pid, &status, 0);
        if (errno == ECHILD) {
            return E_NO_CHILD;
        }
        if (!WIFEXITED(status)) {
            LOGE("Process exits abnormally, errno is %{public}d, status is %{public}d", errno, status);
            return E_WIFEXITED;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("Process exited with an error, errno is %{public}d, status is %{public}d", errno, status);
            return E_WEXITSTATUS;
        }
    }
    return E_OK;
}

int ForkExecWithExit(std::vector<std::string> &cmd, int *exitStatus)
{
    int pipe_fd[2];
    pid_t pid;
    int status;
    auto args = FromatCmd(cmd);
    if (pipe(pipe_fd) < 0) {
        LOGE("creat pipe failed");
        return E_CREATE_PIPE;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("fork failed");
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        return E_FORK;
    } else if (pid == 0) {
        (void)close(pipe_fd[0]);
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            LOGE("dup2 failed");
            _exit(1);
        }
        (void)close(pipe_fd[1]);
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("execvp failed errno: %{public}d", errno);
        _exit(1);
    } else {
        (void)close(pipe_fd[1]);
        (void)close(pipe_fd[0]);
        waitpid(pid, &status, 0);
        LOGE("Process exits %{public}d", errno);
        if (!WIFEXITED(status)) {
            LOGE("Process exits abnormally, status: %{public}d", status);
            return E_WIFEXITED;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("Process exited with an error, status: %{public}d", status);
            return E_WEXITSTATUS;
        }
    }
    return E_OK;
}

#ifdef EXTERNAL_STORAGE_QOS_TRANS
static void ReportExecutorPidEvent(std::vector<std::string> &cmd, int32_t pid)
{
    std::unordered_map<std::string, std::string> payloads;
    if (!cmd.empty() && (cmd[0] == "mount.ntfs" || cmd[0] == "mount.exfat")) {
        payloads["value"] = std::to_string(1);
        payloads["pid"] = std::to_string(pid);
        OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().ReportSceneInfo(
            SET_SCHED_LOAD_TRANS_TYPE, payloads);
    }
}

static void WritePidToPipe(int pipe_fd[PIPE_FD_LEN], size_t len)
{
    if (pipe_fd == nullptr || len < PIPE_FD_LEN) {
        LOGE("write pipe param is invalid.");
        return;
    }
    (void)close(pipe_fd[0]);
    int send_pid = (int)getpid();
    if (write(pipe_fd[1], &send_pid, sizeof(int)) == -1) {
        LOGE("write pipe failed, errno is %{public}d.", errno);
        _exit(1);
    }
    (void)close(pipe_fd[1]);
}

static void ReadPidFromPipe(std::vector<std::string> &cmd, int pipe_fd[2])
{
    (void)close(pipe_fd[1]);
    int recv_pid;
    while (read(pipe_fd[0], &recv_pid, sizeof(int)) > 0) {
        LOGI("read child pid: %{public}d", recv_pid);
    }
    (void)close(pipe_fd[0]);
    ReportExecutorPidEvent(cmd, recv_pid);
}

static void ReadLogFromPipe(int logpipe[PIPE_FD_LEN], size_t len)
{
    if (logpipe == nullptr || len < PIPE_FD_LEN) {
        LOGE("read pipe param is invalid.");
        return;
    }
    (void)close(logpipe[1]);
    FILE* fp = fdopen(logpipe[0], "r");
    if (fp) {
        char line[BUF_LEN];
        while (fgets(line, sizeof(line), fp)) {
            LOGE("exec mount log: %{public}s", line);
        }
        fclose(fp);
        return;
    }
    LOGE("open pipe file failed, errno is %{public}d.", errno);
    (void)close(logpipe[0]);
}

int ExtStorageMountForkExec(std::vector<std::string> &cmd, int *exitStatus)
{
    int pipe_fd[PIPE_FD_LEN];
    int pipe_log_fd[PIPE_FD_LEN]; /* for mount.exfat log*/
    pid_t pid;
    int status;
    auto args = FromatCmd(cmd);

    if (pipe(pipe_fd) < 0) {
        LOGE("creat pipe failed, errno is %{public}d.", errno);
        return E_ERR;
    }

    if (pipe(pipe_log_fd) < 0) {
        LOGE("creat pipe for log failed, errno is %{public}d.", errno);
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        return E_ERR;
    }

    pid = fork();
    if (pid == -1) {
        LOGE("fork failed, errno is %{public}d.", errno);
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        ClosePipe(pipe_log_fd, PIPE_FD_LEN);
        return E_ERR;
    } else if (pid == 0) {
        WritePidToPipe(pipe_fd, PIPE_FD_LEN);
        if (RedirectStdToPipe(pipe_log_fd, PIPE_FD_LEN)) {
            _exit(1);
        }
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("execvp failed errno: %{public}d", errno);
        _exit(1);
    } else {
        ReadPidFromPipe(cmd, pipe_fd);
        ReadLogFromPipe(pipe_log_fd, PIPE_FD_LEN);

        waitpid(pid, &status, 0);
        if (errno == ECHILD) {
            return E_NO_CHILD;
        }
        if (!WIFEXITED(status)) {
            LOGE("Process exits abnormally");
            return E_ERR;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("Process exited with an error");
            return E_ERR;
        }
    }
    return E_OK;
}
#endif

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

int IsSameGidUid(const std::string &dir, uid_t uid, gid_t gid)
{
    struct stat st;
    if (TEMP_FAILURE_RETRY(lstat(dir.c_str(), &st)) == E_ERR) {
        LOGE("failed to lstat, errno %{public}d", errno);
        if (errno == ENOENT) {
            return E_NON_EXIST;
        }
        return E_SYS_KERNEL_ERR;
    }
    return (st.st_uid == uid) && (st.st_gid == gid) ? E_OK : E_DIFF_UID_GID;
}

bool MoveDataShell(const std::string &from, const std::string &to)
{
    LOGI("MoveDataShell start");
    if (TEMP_FAILURE_RETRY(access(from.c_str(), F_OK)) != 0) {
        return true;
    }
    std::vector<std::string> cmd = {
        "/system/bin/mv",
        from,
        to
    };
    std::vector<std::string> out;
    int32_t err = ForkExec(cmd, &out);
    if (err != 0) {
        LOGE("MoveDataShell failed err:%{public}d", err);
    }
    return true;
}

void MoveFileManagerData(const std::string &filesPath)
{
    std::string docsPath = filesPath + "Docs/";
    MoveDataShell(filesPath + "Download/", docsPath);
    MoveDataShell(filesPath + "Documents/", docsPath);
    MoveDataShell(filesPath + "Desktop/", docsPath);
    MoveDataShell(filesPath + ".Trash/", docsPath);
}

void ChownRecursion(const std::string &dir, uid_t uid, gid_t gid)
{
    std::vector<std::string> cmd = {
        "/system/bin/chown",
        "-R",
        std::to_string(uid) + ":" + std::to_string(gid),
        dir
    };
    std::vector<std::string> out;
    int32_t err = ForkExec(cmd, &out);
    if (err != 0) {
        LOGE("path: %{public}s chown failed err:%{public}d", cmd.back().c_str(), err);
    }
}

bool IsPathMounted(std::string &path)
{
    if (path.empty()) {
        return true;
    }
    if (path.back() == '/') {
        path.pop_back();
    }
    std::ifstream inputStream(MOUNT_POINT_INFO, std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open /proc/mounts, errno is %{public}d", errno);
        return true;
    }
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        std::stringstream ss(tmpLine);
        std::string dst;
        ss >> dst;
        ss >> dst;
        if (path == dst) {
            inputStream.close();
            return true;
        }
    }
    inputStream.close();
    return false;
}

std::vector<std::string> Split(std::string str, const std::string &pattern)
{
    std::vector<std::string> result;
    str += pattern;
    size_t size = str.size();
    for (size_t i = 0; i < size; i++) {
        size_t pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

void DeleteFile(const std::string &path)
{
    DIR *dir;
    struct dirent *dirinfo;
    struct stat statbuf;
    if (lstat(path.c_str(), &statbuf) != 0) {
        LOGE(" lstat failed, errno:%{public}d", errno);
        return;
    }

    if (S_ISREG(statbuf.st_mode)) {
        remove(path.c_str());
    } else if (S_ISDIR(statbuf.st_mode)) {
        if ((dir = opendir(path.c_str())) == NULL)
            return;
        while ((dirinfo = readdir(dir)) != NULL) {
            std::string filepath;
            filepath.append(path).append("/").append(dirinfo->d_name);
            if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) {
                continue;
            }
            DeleteFile(filepath);
            rmdir(filepath.c_str());
        }
        closedir(dir);
    }
    return;
}

bool IsTempFolder(const std::string &path, const std::string &sub)
{
    bool result = false;
    if (IsDir(path)) {
        std::vector<std::string> paths = Split(path, "/");
        std::string filePath = paths.back();
        if (filePath.find(sub) == 0) {
            result = true;
        }
    }
    return result;
}

void DelTemp(const std::string &path)
{
    DIR *dir;
    if (!IsDir(path)) {
        return;
    }
    if ((dir = opendir(path.c_str())) != NULL) {
        {
            struct dirent *dirinfo;
            while ((dirinfo = readdir(dir)) != NULL) {
                if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) {
                    continue;
                }
                std::string filePath;
                filePath.append(path).append("/").append(dirinfo->d_name);
                if (IsTempFolder(filePath, "simple-mtpfs")) {
                    DeleteFile(filePath.c_str());
                    rmdir(filePath.c_str());
                }
            }
            closedir(dir);
        }
    }
}

bool DelFolder(const std::string &path)
{
    if (rmdir(path.c_str()) == 0) {
        return true;
    }
    return false;
}

void KillProcess(const std::vector<ProcessInfo> &processList, std::vector<ProcessInfo> &killFailList)
{
    if (processList.empty()) {
        return;
    }
    for (const auto &item: processList) {
        int pid = item.pid;
        LOGI("kill pid %{public}d", pid);
        kill(pid, SIGKILL);
        bool isAlive = true;
        for (int i = 0; i < KILL_RETRY_TIME; i++) {
            if (!IsProcessAlive(pid)) {
                LOGI("kill pid %{public}d success.", pid);
                isAlive = false;
                break;
            }
            usleep(KILL_RETRY_INTERVAL_MS);
        }
        if (isAlive) {
            LOGE("kill pid %{public}d failed.", pid);
            killFailList.push_back(item);
        }
    }
}

bool IsProcessAlive(int pid)
{
    std::stringstream procPath;
    procPath << "/proc/" << pid << "/stat";
    std::ifstream statFile(procPath.str());
    if (!statFile) {
        statFile.close();
        return false;
    }
    statFile.close();
    return true;
}

std::string ProcessToString(std::vector<ProcessInfo> &processList)
{
    if (processList.empty()) {
        return "";
    }
    std::string result;
    for (auto &iter : processList) {
        result += std::to_string(iter.pid) + "_" + iter.name + ",";
    }
    return result.empty() ? "" : result.substr(0, result.length() -1);
}

bool RestoreconDir(const std::string &path)
{
#ifdef USE_LIBRESTORECON
    int err = Restorecon(path.c_str());
    if (err) {
        LOGE("failed to restorecon, err:%{public}d", err);
        return false;
    }
#endif
    return true;
}

int32_t GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    if (!IsValidRgmName(rgmName)) {
        LOGE("rgm name %{public}s invaild", rgmName.c_str());
        return E_CONTAINERPLUGIN_UTILS_RGM_NAME_INVALID;
    }
    std::vector<std::string> ignorePaths;
    ignorePaths.clear();
    return StatisticsFilesTotalSize(rgmConfigs.at(rgmName).mgrPath, ignorePaths, totalSize);
}

int32_t GetRmgDataSize(const std::string &rgmName, const std::string &path,
    const std::vector<std::string> &ignorePaths, uint64_t &totalSize)
{
    if (!IsValidRgmName(rgmName)) {
        LOGE("rgm name %{public}s invalid", rgmName.c_str());
        return E_CONTAINERPLUGIN_UTILS_RGM_NAME_INVALID;
    }

    std::string statisticsPath = rgmConfigs.at(rgmName).businessPath;
    if (!path.empty()) {
        statisticsPath += "/" + path;
    }
    std::vector<std::string> innerIgnorePaths;
    int ignorePathSize = static_cast<int>(ignorePaths.size());
    for (int i = 0; i < ignorePathSize; i++) { // 必须使用引用类型
        innerIgnorePaths.push_back(rgmConfigs.at(rgmName).businessPath + "/" + ignorePaths[i]); // 直接修改元素
    }
    std::string realPath = statisticsPath;
    if (!IsValidPath(statisticsPath)) {
        return E_CONTAINERPLUGIN_UTILS_REMOVE_PATH_INVALID;
    }
    return StatisticsFilesTotalSize(realPath, innerIgnorePaths, totalSize);
}

bool IsValidRgmName(const std::string &rgmName)
{
    if (rgmConfigs.find(rgmName) == rgmConfigs.end()) {
        LOGI("check rgm name not in whitelist %{public}s", rgmName.c_str());
        return false;
    }
    return true;
}

bool IsValidPath(const string &path)
{
    char buf[PATH_MAX] = { 0 };
    if (realpath(path.c_str(), buf) == nullptr) {
        LOGE("Standardized path fail!");
        return false;
    }
    string standardizedPath = buf;
    if (path != standardizedPath) {
        LOGE("UnStandardized!");
        return false;
    }
    return true;
}

bool IsValidBusinessPath(const string &path, const string &userId)
{
    string tmp = path;
    // path is exist and real path same as input path and this path is belong my business
    if (!IsValidPath(tmp) || !IsBusinessPath(tmp, userId)) {
        return false;
    }
    return true;
}

int32_t StatisticsFilesTotalSize(const string &dirPath, const vector<string> &ignorePaths,
    uint64_t &totalSize)
{
    if (!IsFileExist(dirPath)) {
        return E_OK;
    }
    if (!IsValidBusinessPath(dirPath)) {
        LOGE("Statistics dir is illegal!");
        return E_CONTAINERPLUGIN_UTILS_FILE_PATH_ILLEGAL;
    }

    if (!IsFolder(dirPath)) {
        totalSize += GetFileSize(dirPath);
        return E_OK;
    }
    int32_t ret = E_OK;
    int fileCount = 1;
    queue<string> dirTraverseQue;
    dirTraverseQue.push(dirPath);
    while (!dirTraverseQue.empty()) {
        string folder = dirTraverseQue.front();
        int curErr = GetSubFilesSize(folder, dirTraverseQue, ignorePaths, totalSize, fileCount);
        if (curErr == E_CONTAINERPLUGIN_UTILS_FILE_STATISTICS_MAX) {
            LOGD("Statistics out off range!");
            return curErr;
        } else {
            ret = HandleStaticsDirError(ret, curErr);
        }
        dirTraverseQue.pop();
    }
    return ret;
}

bool IsFolder(const string &filename)
{
    struct stat st;
    if (lstat(filename.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

int32_t HandleStaticsDirError(int32_t oldErrno, int32_t newErrno)
{
    if (oldErrno == newErrno) {
        return newErrno;
    }
    if (oldErrno == E_OK) {
        return newErrno;
    }
    if (newErrno == E_OK) {
        return oldErrno;
    }
    return E_CONTAINERPLUGIN_UTILS_STATISTICS_OPEN_FILE_FAILED_AND_STATISTICS_FILE_FAILED;
}

int32_t GetSubFilesSize(const std::string &folder, queue<std::string> &dirTraverseQue,
    const vector<std::string> &ignorePaths, uint64_t &totalSize, int &fileCount)
{
    int32_t err = E_OK;
    std::string path = folder;
    auto it = std::find(ignorePaths.begin(), ignorePaths.end(), path);
    if (it != ignorePaths.end()) {
        LOGW("skip Statistics dir %{private}s", folder.c_str());
        return err;
    }

    unique_ptr<DIR, int (*)(DIR *)> dir(opendir(folder.c_str()), &closedir);
    if (!dir) {
        return E_NOT_DIR_PATH;
    }
    dirent *dt;
    while ((dt = readdir(dir.get())) != nullptr) {
        std::string name = dt->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        if (++fileCount > MAX_STATISTICS_FILES_NUMBER) {
            return E_CONTAINERPLUGIN_UTILS_FILE_STATISTICS_MAX;
        }
        std::string subPath = folder + "/" + name;
        if (IsFolder(subPath)) {
            dirTraverseQue.push(subPath);
        } else {
            totalSize += GetFileSize(subPath);
            continue;
        }
    }
    return err;
}

bool IsBusinessPath(const string &path, const string &userId)
{
    string prefix = "/system/opt/virt_service/";
    if (path.size() >= prefix.size() && prefix == path.substr(0, prefix.size())) {
        return true;
    }
    prefix = "/data/virt_service/rgm";
    if (path.size() >= prefix.size() && prefix == path.substr(0, prefix.size())) {
        return true;
    }

    if (path.size() >= RGM_MANAGER_PATH.size() && RGM_MANAGER_PATH == path.substr(0, RGM_MANAGER_PATH.size())) {
        return true;
    }

    if (path.size() >= EL_RGM_MANAGER_PATH.size() &&
        EL_RGM_MANAGER_PATH == path.substr(0, EL_RGM_MANAGER_PATH.size())) {
        return true;
    }

    if (!userId.empty()) {
        string el2VmImagePath = "/data/service/el2/" + userId + "/virt_service/vm_manager";
        if (path.size() >= el2VmImagePath.size() && el2VmImagePath == path.substr(0, el2VmImagePath.size())) {
            return true;
        }
    }
    return false;
}

bool IsFileExist(const string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        LOGD("IsFileExist path fail! %{public}d", errno);
        return false;
    }
    return true;
}

uint64_t GetFileSize(const string &filename)
{
    struct stat st;
    if (lstat(filename.c_str(), &st) != 0) {
        LOGD("Get %{private}s FileSize is Failed %{public}d", filename.c_str(), errno);
        return 0;
    }
    if (S_ISLNK(st.st_mode)) {
        return 0;
    }
    return st.st_size;
}
} // STORAGE_DAEMON
} // OHOS
