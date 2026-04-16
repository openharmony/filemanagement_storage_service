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
#include <regex>
#include <string>
#include <sstream>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parameters.h"
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/storage_radar.h"
#ifdef USE_LIBRESTORECON
#include "policycoreutils.h"
#endif
#ifdef EXTERNAL_STORAGE_QOS_TRANS
#include "concurrent_task_client.h"
#endif

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t ALL_PERMS = (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
#ifdef EXTERNAL_STORAGE_QOS_TRANS
constexpr int SET_SCHED_LOAD_TRANS_TYPE = 10001;
#endif
constexpr int BUF_LEN = 1024;
constexpr int PIPE_FD_LEN = 2;
constexpr int UUID_LENGTH = 36;
constexpr int UUID_PREFIX_LENGTH = 4;
constexpr int UUID_PREFIX_SUFFIX_LENGTH = 8;
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
constexpr const char *PATH_INVALID_FLAG1 = "../";
constexpr const char *PATH_INVALID_FLAG2 = "/..";
constexpr int32_t PATH_INVALID_FLAG_LEN = 3;
constexpr char FILE_SEPARATOR_CHAR = '/';

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

std::string MaskSensitiveInfo(const std::string &input)
{
    if (input.length() < UUID_LENGTH) {
        return input;
    }
    static const std::regex uuidStr("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");
    std::string output;
    std::sregex_iterator it(input.begin(), input.end(), uuidStr);
    std::sregex_iterator end;
    size_t lastPos = 0;

    for (; it != end; ++it) {
        const std::smatch& match = *it;
        output += input.substr(lastPos, match.position() - lastPos);
        std::string fullUuid = match.str(0);
        output += fullUuid.substr(0, UUID_PREFIX_LENGTH) +
                  std::string(fullUuid.length() - UUID_PREFIX_SUFFIX_LENGTH, '*') +
                  fullUuid.substr(fullUuid.length() - UUID_PREFIX_LENGTH);
        
        lastPos = match.position() + match.length();
    }
    output += input.substr(lastPos);
    return output;
}

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
        LOGE("[L8:FileUtils] RedirectStdToPipe: <<< EXIT FAILED <<< param is invalid");
        return E_ERR;
    }
    int ret = E_OK;
    (void)close(logpipe[0]);
    if (dup2(logpipe[1], STDOUT_FILENO) == -1) {
        LOGE("[L8:FileUtils] RedirectStdToPipe: <<< EXIT FAILED <<< dup2 stdout failed, errno=%{public}d", errno);
        ret = E_ERR;
    }
    if (dup2(logpipe[1], STDERR_FILENO) == -1) {
        LOGE("[L8:FileUtils] RedirectStdToPipe: <<< EXIT FAILED <<< dup2 stderr failed, errno=%{public}d", errno);
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
    LOGI("[L8:FileUtils] PrepareDirSimple: >>> ENTER <<< path=%{public}s", path.c_str());
    if (MkDir(path, mode) != 0) {
        if (errno == EEXIST) {
            LOGE("[L8:FileUtils] PrepareDirSimple: path already exists, path=%{public}s", path.c_str());
            return E_CREATE_USER_DIR_EXIST;
        }
        LOGE("[L8:FileUtils] PrepareDirSimple: <<< EXIT FAILED <<< mkdir failed, errno=%{public}d", errno);
        return E_MKDIR_ERROR;
    }
    if (ChMod(path, mode) != 0) {
        LOGE("[L8:FileUtils] PrepareDirSimple: <<< EXIT FAILED <<< chmod failed, errno=%{public}d", errno);
        return E_CHMOD_ERROR;
    }

    if (ChOwn(path, uid, gid) != 0) {
        LOGE("[L8:FileUtils] PrepareDirSimple: <<< EXIT FAILED <<< chown failed, errno=%{public}d", errno);
        return E_CHOWN_ERROR;
    }

#ifdef USE_LIBRESTORECON
    auto ret = Restorecon(path.c_str());
    if (ret != E_OK) {
        LOGE("[L8:FileUtils] PrepareDirSimple: RestoreconDir failed, errno=%{public}d", errno);
    }
    return ret;
#endif
    LOGI("[L8:FileUtils] PrepareDirSimple: <<< EXIT SUCCESS <<< path=%{public}s", path.c_str());
    return E_OK;
}

// On success, true is returned.  On error, false is returned, and errno is set appropriately.
bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    LOGI("[L8:FileUtils] PrepareDir: >>> ENTER <<< path=%{public}s", path.c_str());
    struct stat st;
    if (TEMP_FAILURE_RETRY(lstat(path.c_str(), &st)) == E_ERR) {
        if (errno != ENOENT) {
            LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< lstat failed, errno=%{public}d", errno);
            return false;
        }
    } else {
        if (!S_ISDIR(st.st_mode)) {
            LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< path exists and is not directory, path=%{public}s",
                path.c_str());
            return false;
        }
        if (((st.st_mode & ALL_PERMS) != mode) && ChMod(path, mode)) {
                LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< chmod failed, errno=%{public}d, uid=%{public}d,"
                    "gid=%{public}d", errno, st.st_uid, st.st_gid);
            std::string extraData = "path=" + path + ",uid=" + to_string(st.st_uid) +
              ",gid=" + to_string(st.st_gid) + ",mode=" + to_string(st.st_mode) + ",errno=" + to_string(errno);
            StorageRadar::ReportUserManager("PrepareDir", DEFAULT_USERID, E_PREPARE_DIR, extraData);
            LOGE("dir exists and failed to chmod, %{public}s", extraData.c_str());
            if (TEMP_FAILURE_RETRY(lstat(path.c_str(), &st)) == E_ERR) {
                LOGE("[L8:FileUtils] PrepareDir: lstat for chmod failed, errno=%{public}d", errno);
            }
            return false;
        }
        if (((st.st_uid != uid) || (st.st_gid != gid)) && ChOwn(path, uid, gid)) {
            LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< chown failed, errno=%{public}d, uid=%{public}d,"
                "gid=%{public}d", errno, st.st_uid, st.st_gid);
            std::string extraData = "path=" + path + ",uid=" + to_string(st.st_uid) +
              ",gid=" + to_string(st.st_gid) + ",mode=" + to_string(st.st_mode) + ",errno=" + to_string(errno);
            StorageRadar::ReportUserManager("PrepareDir", DEFAULT_USERID, E_PREPARE_DIR, extraData);
            LOGE("dir exists and failed to chown, %{public}s", extraData.c_str());
            if (TEMP_FAILURE_RETRY(lstat(path.c_str(), &st)) == E_ERR) {
                LOGE("[L8:FileUtils] PrepareDir: lstat for chown failed, errno=%{public}d", errno);
            }
            return false;
        }
        LOGI("[L8:FileUtils] PrepareDir: <<< EXIT SUCCESS <<< dir already exists, path=%{public}s", path.c_str());
        return true;
    }
    mode_t mask = umask(0);
    if (MkDir(path, mode) != 0) {
        LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< mkdir failed, errno=%{public}d", errno);
        umask(mask);
        return false;
    }
    umask(mask);
    if (ChMod(path, mode) != 0) {
        LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< chmod failed, errno=%{public}d", errno);
        return false;
    }
    if (ChOwn(path, uid, gid) != 0) {
        LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< chown failed, errno=%{public}d", errno);
        return false;
    }
    bool ret = RestoreconDir(path);
    if (ret) {
        LOGI("[L8:FileUtils] PrepareDir: <<< EXIT SUCCESS <<< path=%{public}s", path.c_str());
    }
    return ret;
}

bool RmDirRecurse(const std::string &path)
{
    LOGD("[L8:FileUtils] RmDirRecurse: >>> ENTER <<< path=%{public}s", path.c_str());
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        if (errno == ENOENT) {
            LOGD("[L8:FileUtils] RmDirRecurse: <<< EXIT SUCCESS <<< path not exist");
            return true;
        }
        LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< open dir failed, errno=%{public}d", errno);
        return false;
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            if (!RmDirRecurse(path + "/" + ent->d_name)) {
                LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< RmDirRecurse failed, errno=%{public}d", errno);
                (void)closedir(dir);
                return false;
            }
        } else {
            if (unlink((path + "/" + ent->d_name).c_str())) {
                LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< unlink failed, errno=%{public}d", errno);
                (void)closedir(dir);
                return false;
            }
        }
    }

    (void)closedir(dir);
    if (rmdir(path.c_str())) {
        LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< rmdir failed, errno=%{public}d", errno);
        return false;
    }
    LOGD("[L8:FileUtils] RmDirRecurse: <<< EXIT SUCCESS <<<");
    return true;
}

void TravelChmod(const std::string &path, mode_t mode)
{
    LOGD("[L8:FileUtils] TravelChmod: >>> ENTER <<< path=%{public}s", path.c_str());
    struct stat st;
    DIR *d = nullptr;
    struct dirent *dp = nullptr;
    const char *skip1 = ".";
    const char *skip2 = "..";

    if (stat(path.c_str(), &st) < 0 || !S_ISDIR(st.st_mode)) {
        LOGE("[L8:FileUtils] TravelChmod: <<< EXIT FAILED <<< invalid path");
        return;
    }

    (void)ChMod(path, mode);
    if (!(d = opendir(path.c_str()))) {
        LOGE("[L8:FileUtils] TravelChmod: <<< EXIT FAILED <<< opendir failed");
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
    LOGD("[L8:FileUtils] TravelChmod: <<< EXIT SUCCESS <<<");
}

bool StringToUint32(const std::string &str, uint32_t &num)
{
    if (str.empty()) {
        return false;
    }
    if (!IsNumericStr(str)) {
        LOGE("[L8:FileUtils] StringToUint32: <<< EXIT FAILED <<< Not numeric entry");
        return false;
    }

    int value;
    if (!StrToInt(str, value)) {
        LOGE("[L8:FileUtils] StringToUint32: <<< EXIT FAILED <<< String to int convert failed");
        return false;
    }
    if (value < 0 || value >= INT32_MAX) {
        LOGE("[L8:FileUtils] StringToUint32: <<< EXIT FAILED <<< value out of range");
        return false;
    }
    num = static_cast<uint32_t>(value);
    LOGD("[L8:FileUtils] StringToUint32: <<< EXIT SUCCESS <<<");
    return true;
}

bool StringToBool(const std::string &str, bool &result)
{
    if (str.empty()) {
        LOGE("[L8:FileUtils] StringToBool: <<< EXIT FAILED <<< String is empty");
        return false;
    }

    if (str == "true") {
        result = true;
    } else if (str == "false") {
        result = false;
    } else {
        LOGE("[L8:FileUtils] StringToBool: <<< EXIT FAILED <<< Invalid boolean string=%{public}s", str.c_str());
        return false;
    }

    LOGD("[L8:FileUtils] StringToBool: <<< EXIT SUCCESS <<< result=%{public}d", result);
    return true;
}

void GetSubDirs(const std::string &path, std::vector<std::string> &dirList)
{
    LOGD("[L8:FileUtils] GetSubDirs: >>> ENTER <<< path=%{public}s", path.c_str());
    dirList.clear();

    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGE("[L8:FileUtils] GetSubDirs: <<< EXIT FAILED <<< path is not dir");
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("[L8:FileUtils] GetSubDirs: <<< EXIT FAILED <<< open dir failed, errno=%{public}d", errno);
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
    LOGD("[L8:FileUtils] GetSubDirs: <<< EXIT SUCCESS <<< dirCount=%{public}zu", dirList.size());
}

void ReadDigitDir(const std::string &path, std::vector<FileList> &dirInfo)
{
    LOGD("[L8:FileUtils] ReadDigitDir: >>> ENTER <<< path=%{public}s", path.c_str());
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGE("[L8:FileUtils] ReadDigitDir: <<< EXIT FAILED <<< path is not dir");
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("[L8:FileUtils] ReadDigitDir: <<< EXIT FAILED <<< open dir failed, errno=%{public}d", errno);
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
    LOGD("[L8:FileUtils] ReadDigitDir: <<< EXIT SUCCESS <<< dirCount=%{public}zu", dirInfo.size());
}

void OpenSubFile(const std::string &path, std::vector<std::string>  &file)
{
    LOGD("[L8:FileUtils] OpenSubFile: >>> ENTER <<< path=%{public}s", path.c_str());
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGI("[L8:FileUtils] OpenSubFile: path is not dir");
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGI("[L8:FileUtils] OpenSubFile: open dir failed, errno=%{public}d", errno);
        return;
    }
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if ((ent->d_type != DT_DIR)) {
            std::string name(ent->d_name);
            std::string filePath = path + "/" + name;
            LOGI("[L8:FileUtils] OpenSubFile: filePath=%{public}s", filePath.c_str());
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
    LOGD("[L8:FileUtils] OpenSubFile: <<< EXIT SUCCESS <<< fileCount=%{public}zu", file.size());
}

bool ReadFile(const std::string &path, std::string *str)
{
    LOGD("[L8:FileUtils] ReadFile: >>> ENTER <<< path=%{public}s", path.c_str());
    std::ifstream infile;
    int cnt = 0;

    std::string rpath(PATH_MAX + 1, '\0');
    if ((path.length() > PATH_MAX) || (realpath(path.c_str(), rpath.data()) == nullptr)) {
        LOGE("[L8:FileUtils] ReadFile: <<< EXIT FAILED <<< realpath failed");
        return false;
    }

    infile.open(rpath.c_str());
    if (!infile) {
        LOGE("[L8:FileUtils] ReadFile: <<< EXIT FAILED <<< Cannot open file");
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
    bool ret = cnt == 0 ? false : true;
    if (ret) {
        LOGD("[L8:FileUtils] ReadFile: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L8:FileUtils] ReadFile: <<< EXIT FAILED <<< file is empty");
    }
    return ret;
}

static std::vector<char*> FormatCmd(std::vector<std::string> &cmd)
{
    std::vector<char*>res;
    res.reserve(cmd.size() + 1);

    for (auto& line : cmd) {
        LOGE("[L8:FileUtils] FromatCmd: cmd=%{public}s", line.c_str());
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
    LOGD("[L8:FileUtils] ForkExec: >>> ENTER <<< cmd=%{public}s", cmd.empty() ? "" : cmd[0].c_str());
    int pipeFd[PIPE_FD_LEN];
    pid_t pid;
    int status;
    auto args = FormatCmd(cmd);
    if (pipe(pipeFd) < 0) {
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< create pipe failed, errno=%{public}d", errno);
        return E_CREATE_PIPE;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< fork failed, errno=%{public}d", errno);
        ClosePipe(pipeFd, PIPE_FD_LEN);
        return E_FORK;
    } else if (pid == 0) {
        if (RedirectStdToPipe(pipeFd, PIPE_FD_LEN)) {
            _exit(1);
        }
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< execvp failed, errno=%{public}d", errno);
        _exit(1);
    } else {
        (void)close(pipeFd[1]);
        if (output) {
            char buf[BUF_LEN] = { 0 };
            (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
            output->clear();
            while (read(pipeFd[0], buf, BUF_LEN - 1) > 0) {
                LOGE("[L8:FileUtils] ForkExec: read output chunk errno=%{public}d", errno);
                output->push_back(buf);
            }
        }
        (void)close(pipeFd[0]);
        waitpid(pid, &status, 0);
        if (errno == ECHILD) {
            LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< ECHILD");
            return E_NO_CHILD;
        }
        if (!WIFEXITED(status)) {
            LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< Process exits abnormally, errno=%{public}d,"
                "status=%{public}d", errno, status);
            return E_WIFEXITED;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< Process exited with error, errno=%{public}d,"
                "status=%{public}d", errno, status);
            return E_WEXITSTATUS;
        }
    }
    LOGD("[L8:FileUtils] ForkExec: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int ForkExecWithExit(std::vector<std::string> &cmd, int *exitStatus)
{
    LOGD("[L8:FileUtils] ForkExecWithExit: >>> ENTER <<< cmd=%{public}s", cmd.empty() ? "" : cmd[0].c_str());
    int pipeFd[2];
    pid_t pid;
    int status;
    auto args = FormatCmd(cmd);
    if (pipe(pipeFd) < 0) {
        LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< create pipe failed");
        return E_CREATE_PIPE;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< fork failed");
        ClosePipe(pipeFd, PIPE_FD_LEN);
        return E_FORK;
    } else if (pid == 0) {
        (void)close(pipeFd[0]);
        if (dup2(pipeFd[1], STDOUT_FILENO) == -1) {
            LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< dup2 failed");
            _exit(1);
        }
        (void)close(pipeFd[1]);
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< execvp failed, errno=%{public}d", errno);
        _exit(1);
    } else {
        (void)close(pipeFd[1]);
        (void)close(pipeFd[0]);
        waitpid(pid, &status, 0);
        if (errno == ECHILD) {
            LOGE("[L8:FileUtils] ForkExecWithExit: Process exits %{public}d", errno);
            return E_NO_CHILD;
        }
        if (!WIFEXITED(status)) {
            LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< Process exits abnormally, status=%{public}d",
                status);
            return E_WIFEXITED;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< Process exited with error, status=%{public}d",
                status);
            return E_WEXITSTATUS;
        }
    }
    return E_OK;
}

static void WriteInputToPipe(int pipeFd, std::vector<std::string> &input)
{
    for (const auto& line : input) {
        write(pipeFd, line.c_str(), line.size());
        write(pipeFd, "\n", 1);
    }
}

static void ReadPipeOutput(int pipeFd, std::vector<std::string> &output)
{
    LOGI("[L8:FileUtils] ReadPipeOutput: start!!!");
    char buf[BUF_LEN] = { 0 };
    (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
    output.clear();
    while (read(pipeFd, buf, BUF_LEN - 1) > 0) {
        if (strstr(buf, "[2k") != nullptr) {
            break;
        }
        LOGI("[L8:FileUtils] ReadPipeOutput: get result %{public}s", buf);
        output.push_back(buf);
    }
    LOGI("[L8:FileUtils] ReadPipeOutput: ReadPipeOutput end!!!");
}

int ForkExecInteractive(std::vector<std::string> &cmd, std::vector<std::string> *output,
                        std::vector<std::string> *input)
{
    int inPipe[PIPE_FD_LEN];
    int outPipe[PIPE_FD_LEN];
    pid_t pid;
    auto args = FormatCmd(cmd);
    if (pipe(inPipe) < 0) {
        LOGE("[L8:FileUtils] ForkExecInteractive: creat pipe failed, errno is %{public}d.", errno);
        return E_CREATE_PIPE;
    }
    if (pipe(outPipe) < 0) {
        LOGE("[L8:FileUtils] ForkExecInteractive: creat pipe failed, errno is %{public}d.", errno);
        ClosePipe(inPipe, PIPE_FD_LEN);
        return E_CREATE_PIPE;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ForkExecInteractive: fork failed, errno is %{public}d.", errno);
        return E_FORK;
    } else if (pid == 0) {
        (void)close(inPipe[1]);
        (void)close(outPipe[0]);
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(outPipe[1], STDERR_FILENO);
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ForkExecInteractive: execvp failed errno: %{public}d", errno);
        _exit(1);
    } else {
        (void)close(inPipe[0]);
        (void)close(outPipe[1]);
        if (input) {
            WriteInputToPipe(inPipe[1], *input);
        }
        (void)close(inPipe[1]);

        if (output) {
            ReadPipeOutput(outPipe[0], *output);
        }
        (void)close(outPipe[0]);
    }
    LOGI("[L8:FileUtils] ForkExecInteractive: ForkExecInteractive end.");
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

static void WritePidToPipe(int pipeFd[PIPE_FD_LEN], size_t len)
{
    if (pipeFd == nullptr || len < PIPE_FD_LEN) {
        LOGE("write pipe param is invalid.");
        return;
    }
    (void)close(pipeFd[0]);
    int send_pid = (int)getpid();
    if (write(pipeFd[1], &send_pid, sizeof(int)) == -1) {
        LOGE("[L8:FileUtils] WritePidToPipe: <<< EXIT FAILED <<< write pipe failed, errno=%{public}d", errno);
        _exit(1);
    }
    (void)close(pipeFd[1]);
}

static void ReadPidFromPipe(std::vector<std::string> &cmd, int pipeFd[2])
{
    (void)close(pipeFd[1]);
    int recv_pid;
    while (read(pipeFd[0], &recv_pid, sizeof(int)) > 0) {
        LOGI("[L8:FileUtils] ReadPidFromPipe: read child pid=%{public}d", recv_pid);
    }
    (void)close(pipeFd[0]);
    ReportExecutorPidEvent(cmd, recv_pid);
}

static void ReadLogFromPipe(int logpipe[PIPE_FD_LEN], size_t len)
{
    if (logpipe == nullptr || len < PIPE_FD_LEN) {
        LOGE("[L8:FileUtils] ReadLogFromPipe: <<< EXIT FAILED <<< param is invalid");
        return;
    }
    (void)close(logpipe[1]);
    FILE* fp = fdopen(logpipe[0], "r");
    if (fp) {
        char line[BUF_LEN];
        while (fgets(line, sizeof(line), fp)) {
            LOGE("[L8:FileUtils] ReadLogFromPipe: exec mount log lerrno=%{public}d", errno);
        }
        fclose(fp);
        return;
    }
    LOGE("[L8:FileUtils] ReadLogFromPipe: <<< EXIT FAILED <<< open pipe file failed, errno=%{public}d", errno);
    (void)close(logpipe[0]);
}

int ExtStorageMountForkExec(std::vector<std::string> &cmd, int *exitStatus)
{
    LOGD("[L8:FileUtils] ExtStorageMountForkExec: >>> ENTER <<< cmd=%{public}s", cmd.empty() ? "" : cmd[0].c_str());
    int pipeFd[PIPE_FD_LEN];
    int pipe_log_fd[PIPE_FD_LEN]; /* for mount.exfat log*/
    pid_t pid;
    int status;
    auto args = FormatCmd(cmd);

    if (pipe(pipeFd) < 0) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< create pipe failed, errno=%{public}d", errno);
        return E_ERR;
    }

    if (pipe(pipe_log_fd) < 0) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< create pipe for log failed,"
            "errno=%{public}d", errno);
        ClosePipe(pipeFd, PIPE_FD_LEN);
        return E_ERR;
    }

    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< fork failed, errno=%{public}d", errno);
        ClosePipe(pipeFd, PIPE_FD_LEN);
        ClosePipe(pipe_log_fd, PIPE_FD_LEN);
        return E_ERR;
    } else if (pid == 0) {
        WritePidToPipe(pipeFd, PIPE_FD_LEN);
        if (RedirectStdToPipe(pipe_log_fd, PIPE_FD_LEN)) {
            _exit(1);
        }
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< execvp failed, errno=%{public}d", errno);
        _exit(1);
    } else {
        ReadPidFromPipe(cmd, pipeFd);
        ReadLogFromPipe(pipe_log_fd, PIPE_FD_LEN);

        waitpid(pid, &status, 0);
        if (errno == ECHILD) {
            LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< ECHILD");
            return E_NO_CHILD;
        }
        if (!WIFEXITED(status)) {
            LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< Process exits abnormally");
            return E_ERR;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< Process exited with error");
            return E_ERR;
        }
    }
    LOGD("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT SUCCESS <<<");
    return E_OK;
}
#endif

void TraverseDirUevent(const std::string &path, bool flag)
{
    LOGD("[L8:FileUtils] TraverseDirUevent: >>> ENTER <<< path=%{public}s", path.c_str());
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
    LOGD("[L8:FileUtils] TraverseDirUevent: <<< EXIT SUCCESS <<<");
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
        LOGE("[L8:FileUtils] IsPathMounted: <<< EXIT FAILED <<< open /proc/mounts failed, errno=%{public}d", errno);
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
            LOGD("[L8:FileUtils] IsPathMounted: <<< EXIT SUCCESS <<< path is mounted");
            return true;
        }
    }
    inputStream.close();
    LOGD("[L8:FileUtils] IsPathMounted: <<< EXIT SUCCESS <<< path is not mounted");
    return false;
}

std::vector<std::string> Split(std::string str, const std::string &pattern)
{
    std::vector<std::string> result;
    str += pattern;
    size_t size = str.size();
    size_t i = 0;
    while (i < size) {
        size_t pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size();
        } else {
            break;
        }
    }
    return result;
}

void DeleteFile(const std::string &path)
{
    LOGD("[L8:FileUtils] DeleteFile: >>> ENTER <<< path=%{public}s", path.c_str());
    DIR *dir = nullptr;
    struct dirent *dirinfo = nullptr;
    struct stat statbuf;
    if (lstat(path.c_str(), &statbuf) != 0) {
        LOGE("[L8:FileUtils] DeleteFile: <<< EXIT FAILED <<< lstat failed, errno=%{public}d", errno);
        return;
    }

    if (S_ISREG(statbuf.st_mode)) {
        remove(path.c_str());
    } else if (S_ISDIR(statbuf.st_mode)) {
        if ((dir = opendir(path.c_str())) == nullptr) {
            LOGE("[L8:FileUtils] DeleteFile: opendir failed, errno:%{public}d", errno);
            return;
        }
        while ((dirinfo = readdir(dir)) != nullptr) {
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
    LOGD("[L8:FileUtils] DeleteFile: <<< EXIT SUCCESS <<<");
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

void KillProcess(const std::vector<ProcessInfo> &processList, std::vector<ProcessInfo> &killFailList)
{
    LOGI("[L8:FileUtils] KillProcess: >>> ENTER <<< processCount=%{public}zu", processList.size());
    if (processList.empty()) {
        return;
    }
    for (const auto &item: processList) {
        int pid = item.pid;
        LOGI("[L8:FileUtils] KillProcess: killing pid=%{public}d", pid);
        kill(pid, SIGKILL);
        bool isAlive = true;
        for (int i = 0; i < KILL_RETRY_TIME; i++) {
            if (!IsProcessAlive(pid)) {
                LOGI("[L8:FileUtils] KillProcess: kill pid=%{public}d success", pid);
                isAlive = false;
                break;
            }
            usleep(KILL_RETRY_INTERVAL_MS);
        }
        if (isAlive) {
            LOGE("[L8:FileUtils] KillProcess: <<< EXIT FAILED <<< kill pid=%{public}d failed", pid);
            killFailList.push_back(item);
        }
    }
    LOGI("[L8:FileUtils] KillProcess: <<< EXIT SUCCESS <<<");
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
        LOGE("[L8:FileUtils] RestoreconDir: <<< EXIT FAILED <<< err=%{public}d", err);
        return false;
    }
#endif
    LOGD("[L8:FileUtils] RestoreconDir: <<< EXIT SUCCESS <<< path=%{public}s", path.c_str());
    return true;
}

int32_t GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    LOGI("[L8:FileUtils] GetRmgResourceSize: >>> ENTER <<< rgmName=%{public}s", rgmName.c_str());
    if (!IsValidRgmName(rgmName)) {
        LOGE("[L8:FileUtils] GetRmgResourceSize: <<< EXIT FAILED <<< rgm name invalid");
        return E_CONTAINERPLUGIN_UTILS_RGM_NAME_INVALID;
    }
    std::vector<std::string> ignorePaths;
    ignorePaths.clear();
    int32_t ret = StatisticsFilesTotalSize(rgmConfigs.at(rgmName).mgrPath, ignorePaths, totalSize);
    if (ret == E_OK) {
        LOGI("[L8:FileUtils] GetRmgResourceSize: <<< EXIT SUCCESS <<< totalSize=%{public}llu",
            static_cast<unsigned long long>(totalSize));
    }
    return ret;
}

int32_t GetRmgDataSize(const std::string &rgmName, const std::string &path,
    const std::vector<std::string> &ignorePaths, uint64_t &totalSize)
{
    LOGI("[L8:FileUtils] GetRmgDataSize: >>> ENTER <<< rgmName=%{public}s, path=%{public}s",
         rgmName.c_str(), path.c_str());
    if (!IsValidRgmName(rgmName)) {
        LOGE("[L8:FileUtils] GetRmgDataSize: <<< EXIT FAILED <<< rgm name invalid");
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
        LOGE("[L8:FileUtils] GetRmgDataSize: <<< EXIT FAILED <<< path invalid");
        return E_CONTAINERPLUGIN_UTILS_REMOVE_PATH_INVALID;
    }
    int32_t ret = StatisticsFilesTotalSize(realPath, innerIgnorePaths, totalSize);
    if (ret == E_OK) {
        LOGI("[L8:FileUtils] GetRmgDataSize: <<< EXIT SUCCESS <<< totalSize=%{public}llu",
            static_cast<unsigned long long>(totalSize));
    }
    return ret;
}

bool IsValidRgmName(const std::string &rgmName)
{
    if (rgmConfigs.find(rgmName) == rgmConfigs.end()) {
        LOGI("[L8:FileUtils] IsValidRgmName: rgm name not in whitelist=%{public}s", rgmName.c_str());
        return false;
    }
    return true;
}

bool IsValidPath(const string &path)
{
    char buf[PATH_MAX] = { 0 };
    if (realpath(path.c_str(), buf) == nullptr) {
        LOGE("[L8:FileUtils] IsValidPath: <<< EXIT FAILED <<< Standardized path fail");
        return false;
    }
    string standardizedPath = buf;
    if (path != standardizedPath) {
        LOGE("[L8:FileUtils] IsValidPath: <<< EXIT FAILED <<< UnStandardized");
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
    LOGD("[L8:FileUtils] StatisticsFilesTotalSize: >>> ENTER <<< dirPath=%{private}s", dirPath.c_str());
    if (!IsFileExist(dirPath)) {
        LOGD("[L8:FileUtils] StatisticsFilesTotalSize: <<< EXIT SUCCESS <<< path not exist");
        return E_OK;
    }
    if (!IsValidBusinessPath(dirPath)) {
        LOGE("[L8:FileUtils] StatisticsFilesTotalSize: <<< EXIT FAILED <<< dir is illegal");
        return E_CONTAINERPLUGIN_UTILS_FILE_PATH_ILLEGAL;
    }

    if (!IsFolder(dirPath)) {
        totalSize += GetFileSize(dirPath);
        LOGD("[L8:FileUtils] StatisticsFilesTotalSize: <<< EXIT SUCCESS <<< is file");
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
            LOGD("[L8:FileUtils] StatisticsFilesTotalSize: <<< EXIT SUCCESS <<< out of range");
            return curErr;
        } else {
            ret = HandleStaticsDirError(ret, curErr);
        }
        dirTraverseQue.pop();
    }
    LOGD("[L8:FileUtils] StatisticsFilesTotalSize: <<< EXIT SUCCESS <<< totalSize=%{public}llu",
        static_cast<unsigned long long>(totalSize));
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
        LOGW("[L8:FileUtils] GetSubFilesSize: skip Statistics dir=%{private}s", folder.c_str());
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
        LOGD("[L8:FileUtils] IsFileExist: path fail, errno=%{public}d", errno);
        return false;
    }
    return true;
}

uint64_t GetFileSize(const string &filename)
{
    struct stat st;
    if (lstat(filename.c_str(), &st) != 0) {
        LOGD("[L8:FileUtils] GetFileSize: Failed, filename=%{private}s, errno=%{public}d", filename.c_str(), errno);
        return 0;
    }
    if (S_ISLNK(st.st_mode)) {
        return 0;
    }
    return st.st_size;
}

bool IsFilePathInvalid(const std::string &filePath)
{
    size_t pos = filePath.find(PATH_INVALID_FLAG1);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("[L8:FileUtils] IsFilePathInvalid: Relative path is not allowed, path contain ../");
            return true;
        }
        pos = filePath.find(PATH_INVALID_FLAG1, pos + PATH_INVALID_FLAG_LEN);
    }
    pos = filePath.rfind(PATH_INVALID_FLAG2);
    if ((pos != std::string::npos) && (filePath.size() - pos == PATH_INVALID_FLAG_LEN)) {
        LOGE("[L8:FileUtils] IsFilePathInvalid: Relative path is not allowed, path tail is /..");
        return true;
    }
    return false;
}
} // namespace StorageDaemon
} // namespace OHOS
