/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "utils/volume_op_diag.h"

#include <cstdint>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "file_ex.h"
#include "parameters.h"
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/storage_radar.h"
#include "utils/hi_audit.h"
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
constexpr int BUF_LEN = 20480;
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
const std::string DROP_ENCRYPT_DENTRY_PATH = "/proc/sys/vm/drop_encrypt_dentry";
const std::string DROP_ENCRYPT_DENTRY_VALUE = "1";
constexpr uint32_t OVER_LOOP_FIRST_ALERT_THRESHOLD = 1000;
constexpr uint32_t OVER_LOOP_SECOND_ALERT_THRESHOLD = 2000;
constexpr uint32_t OVER_LOOP_COUNT_GROW_CAP = 3000;
constexpr uint32_t OVER_LOOP_ALERT_HALF_MAX_MULTIPLE = 2;
constexpr uint32_t OVER_LOOP_COUNT_GROW_CAP_MULTIPLE = 3;
constexpr mode_t DEFAULT_OUTPUT_FILE_MODE = 0644;

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
    if (MkDir(path, mode)) {
        if (errno == EEXIST) {
            LOGE("[L8:FileUtils] PrepareDirSimple: path already exists, path=%{public}s", path.c_str());
            return E_CREATE_USER_DIR_EXIST;
        }
        LOGE("[L8:FileUtils] PrepareDirSimple: <<< EXIT FAILED <<< mkdir failed, errno=%{public}d", errno);
        return E_MKDIR_ERROR;
    }
    if (ChMod(path, mode)) {
        LOGE("[L8:FileUtils] PrepareDirSimple: <<< EXIT FAILED <<< chmod failed, errno=%{public}d", errno);
        return E_CHMOD_ERROR;
    }

    if (ChOwn(path, uid, gid)) {
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
        return true;
    }
    mode_t mask = umask(0);
    if (MkDir(path, mode)) {
        LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< mkdir failed, errno=%{public}d", errno);
        umask(mask);
        return false;
    }
    umask(mask);
    if (ChMod(path, mode)) {
        LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< chmod failed, errno=%{public}d", errno);
        return false;
    }
    if (ChOwn(path, uid, gid)) {
        LOGE("[L8:FileUtils] PrepareDir: <<< EXIT FAILED <<< chown failed, errno=%{public}d", errno);
        return false;
    }
    return RestoreconDir(path);
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
        LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< open dir %{public}s failed, errno=%{public}d",
            path.c_str(), errno);
        return false;
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            if (!RmDirRecurse(path + "/" + ent->d_name)) {
                LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< RmDirRecurse %{public}s failed,"
                    "errno=%{public}d", path.c_str(), errno);
                (void)closedir(dir);
                return false;
            }
        } else {
            if (unlink((path + "/" + ent->d_name).c_str())) {
                LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< unlink file %{public}s failed, errno=%{public}d",
                    ent->d_name, errno);
                (void)closedir(dir);
                return false;
            }
        }
    }

    (void)closedir(dir);
    if (rmdir(path.c_str())) {
        LOGE("[L8:FileUtils] RmDirRecurse: <<< EXIT FAILED <<< rmdir dir %{public}s failed, errno=%{public}d",
            path.c_str(), errno);
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

    return true;
}

void GetSubDirs(const std::string &path, std::vector<std::string> &dirList)
{
    LOGD("[L8:FileUtils] GetSubDirs: >>> ENTER <<< path=%{public}s", path.c_str());
    dirList.clear();

    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret != 0 || ((st.st_mode & S_IFDIR) != S_IFDIR)) {
        LOGE("[L8:FileUtils] GetSubDirs: <<< EXIT FAILED <<< path is not dir, path=%{public}s", path.c_str());
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("[L8:FileUtils] GetSubDirs: <<< EXIT FAILED <<< open dir failed, errno=%{public}d,"
            "path=%{public}s", errno, path.c_str());
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
        LOGE("[L8:FileUtils] ReadDigitDir: <<< EXIT FAILED <<< path is not dir, path=%{public}s", path.c_str());
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("[L8:FileUtils] ReadDigitDir: <<< EXIT FAILED <<< open dir failed, errno=%{public}d"
            ", path=%{public}s", errno, path.c_str());
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
        LOGI("[L8:FileUtils] OpenSubFile: path is not dir, path=%{public}s", path.c_str());
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGI("[L8:FileUtils] OpenSubFile: open dir failed, errno=%{public}d,"
            "path=%{public}s", errno, path.c_str());
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
        LOGE("[L8:FileUtils] ReadFile: <<< EXIT FAILED <<< realpath failed, "
            "path=%{public}s", path.c_str());
        return false;
    }

    infile.open(rpath.c_str());
    if (!infile) {
        LOGE("[L8:FileUtils] ReadFile: <<< EXIT FAILED <<< Cannot open file,"
            "path=%{public}s", path.c_str());
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

std::string ReadFileContent(const std::string &path)
{
    LOGD("[L8:FileUtils] ReadFileContent: >>> ENTER <<< path=%{public}s", path.c_str());
    
    std::ifstream infile;
    std::string result;

    char realPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), realPath) == nullptr) {
        LOGE("[L8:FileUtils] ReadFileContent: <<< EXIT FAILED <<< realpath failed, errno: %{public}d.", errno);
        return "";
    }

    infile.open(realPath);
    if (!infile) {
        LOGE("[L8:FileUtils] ReadFileContent: <<< EXIT FAILED <<< Cannot open file, errno: %{public}d.", errno);
        return "";
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(infile, line)) {
        if (!firstLine) {
            result += "\n";
        }
        result += line;
        firstLine = false;
    }

    infile.close();
    
    if (result.empty()) {
        LOGE("[L8:FileUtils] ReadFileContent: <<< EXIT FAILED <<< file is empty");
    } else {
        LOGD("[L8:FileUtils] ReadFileContent: <<< EXIT SUCCESS <<<");
    }
    
    return result;
}

std::string ReadFileInParentDirs(const std::string &startPath, const std::string &fileName)
{
    LOGD("[L3:DiskUtils] FindFileInParentDirs: >>> ENTER <<< startPath=%{public}s, fileName=%{public}s",
         startPath.c_str(), fileName.c_str());
    std::string currentPath = startPath;
    while (!currentPath.empty() && currentPath != "/") {
        std::string targetPath = currentPath + "/" + fileName;
        std::string content = ReadFileContent(targetPath);
        if (!content.empty()) {
            LOGD("[L3:DiskUtils] FindFileInParentDirs: Found %{public}s at %{public}s",
                fileName.c_str(), targetPath.c_str());
            return content;
        }
        size_t lastSlash = currentPath.find_last_of('/');
        if (lastSlash == 0) {
            break;
        }
        if (lastSlash != std::string::npos) {
            currentPath = currentPath.substr(0, lastSlash);
        } else {
            break;
        }
    }
    LOGD("[L3:DiskUtils] FindFileInParentDirs: <<< EXIT SUCCESS <<< %{public}s not found", fileName.c_str());
    return "";
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

static void ReadPipeOutputForExec(int pipeFdRead, std::vector<std::string> *output, const std::string &cmdName)
{
    if (!output) {
        return;
    }
    char buf[BUF_LEN] = { 0 };
    output->clear();
    ssize_t bytesRead = 0;
    while ((bytesRead = read(pipeFdRead, buf, BUF_LEN - 1)) > 0) {
        buf[bytesRead] = '\0';
        LOGE("[L8:FileUtils] ReadPipeOutputForExec: cmd=%{public}s", cmdName.c_str());
        output->emplace_back(buf, bytesRead);
        (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
    }
}

static int CheckChildProcessExitStatus(pid_t pid, int &status, int *exitStatus)
{
    status = 0;
    pid_t waitRet = waitpid(pid, &status, 0);
    if (waitRet == -1) {
        if (errno == ECHILD) {
            LOGE("[L8:FileUtils] CheckChildProcessExitStatus: ECHILD");
            return E_NO_CHILD;
        }
        LOGE("[L8:FileUtils] CheckChildProcessExitStatus: waitpid failed, errno=%{public}d", errno);
        return E_SYS_KERNEL_ERR;
    }
    if (!WIFEXITED(status)) {
        LOGE("[L8:FileUtils] CheckChildProcessExitStatus: Process exits abnormally, status=%{public}d", status);
        return E_WIFEXITED;
    }
    int tempExitStatus = WEXITSTATUS(status);
    GetExitStatus(exitStatus, tempExitStatus);
    if (tempExitStatus != 0) {
        LOGE("[L8:FileUtils] CheckChildProcessExitStatus: Process exited with error, status=%{public}d", status);
        return E_WEXITSTATUS;
    }
    return E_OK;
}

static int32_t ResolveForkExecExitCode(int32_t ret, int status, int *exitStatus)
{
    if (ret == E_NO_CHILD) {
        return errno;
    }
    if (ret == E_WIFEXITED) {
        return -1;
    }
    if (exitStatus != nullptr) {
        return *exitStatus;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

static void ReportForkExecDiagIfNeeded(const std::vector<std::string> &cmd, int32_t ret, int32_t exitCode,
                                       const std::vector<std::string> *output)
{
    VolumeOpDiagReportToolFailure(cmd, ret, exitCode, output);
}

static void RedirectChildStd(int pipeFd[PIPE_FD_LEN], bool captureAll)
{
    if (captureAll) {
        if (RedirectStdToPipe(pipeFd, PIPE_FD_LEN) != E_OK) {
            _exit(1);
        }
        return;
    }
    (void)close(pipeFd[0]);
    if (dup2(pipeFd[1], STDOUT_FILENO) == -1) {
        LOGE("[L8:FileUtils] RedirectChildStd: <<< EXIT FAILED <<< dup2 failed");
        _exit(1);
    }
    (void)close(pipeFd[1]);
}

/*
 * ForkExec - 通过 fork+exec 执行外部命令，并通过管道捕获子进程的输出
 *
 * 【主要功能】
 * 在子进程中执行指定的命令行程序，父进程通过管道（pipe）读取子进程的
 * stdout 和 stderr 输出，并将输出内容存入 output 参数指向的 vector 中。
 *
 * 【实现流程】
 * 1. 参数校验：检查 cmd 是否为空，为空则直接返回 E_PARAMS_INVALID。
 * 2. 创建管道：调用 pipe() 创建一对管道文件描述符 pipeFd[0]（读端）和
 *    pipeFd[1]（写端），用于父子进程间通信。
 * 3. 格式化命令：调用 FormatCmd() 将 std::vector<std::string> 转换为
 *    execvp 所需的 char* 数组格式（末尾以 nullptr 哨兵结尾）。
 * 4. fork 子进程：
 *    - fork 失败：关闭管道，上报诊断，返回 E_FORK。
 *    - 子进程（pid == 0）：
 *      a. 调用 RedirectStdToPipe() 将 stdout 和 stderr 都重定向到管道写端
 *         pipeFd[1]，这样子进程的所有标准输出和错误输出都会写入管道。
 *      b. 调用 execvp() 执行目标命令。若 execvp 成功，当前进程映像被替换，
 *         不会返回；若失败则记录日志后 _exit(1) 退出。
 *    - 父进程（pid > 0）：
 *      a. 关闭管道写端 pipeFd[1]（父进程只读）。
 *      b. 调用 ReadPipeOutputForExec() 从管道读端 pipeFd[0] 循环读取子进程
 *         输出，将内容追加到 output 指向的 vector 中（若 output 为 nullptr 则跳过）。
 *      c. 关闭管道读端 pipeFd[0]。
 *      d. 调用 CheckChildProcessExitStatus() 通过 waitpid() 等待子进程结束并
 *         检查退出状态，可通过 exitStatus 参数获取子进程退出码。
 * 5. 错误上报：若子进程异常退出，调用 ReportForkExecDiagIfNeeded() 上报诊断信息。
 *
 * 【与 ForkExecToFile 的区别】
 * - ForkExec 将子进程 stdout/stderr 通过管道实时捕获到内存
 *   （std::vector<std::string>），适用于需要读取和解析命令输出内容的场景。
 * - ForkExecToFile 将子进程 stdout 重定向到指定文件（stderr 不重定向，
 *   走 hilog），适用于只需将输出持久化到文件的场景，且不会上报诊断信息。
 */
int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output, int *exitStatus)
{
    if (cmd.empty()) {
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< cmd is empty");
        return E_PARAMS_INVALID;
    }
    int pipeFd[PIPE_FD_LEN];
    pid_t pid;
    int status = 0;
    auto args = FormatCmd(cmd);
    if (pipe(pipeFd) < 0) {
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< create pipe failed,"
            "errno=%{public}d, cmd=%{public}s", errno, cmd.empty() ? "" : cmd[0].c_str());
        ClosePipe(pipeFd, PIPE_FD_LEN);
        ReportForkExecDiagIfNeeded(cmd, E_CREATE_PIPE, errno, output);
        return E_CREATE_PIPE;
    }
    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< fork failed, errno=%{public}d", errno);
        ClosePipe(pipeFd, PIPE_FD_LEN);
        ReportForkExecDiagIfNeeded(cmd, E_FORK, errno, output);
        return E_FORK;
    } else if (pid == 0) {
        if (RedirectStdToPipe(pipeFd, PIPE_FD_LEN)) {
            _exit(1);
        }
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ForkExec: <<< EXIT FAILED <<< execvp failed, errno=%{public}d,"
            "cmd=%{public}s", errno, cmd.empty() ? "" : cmd[0].c_str());
        _exit(1);
    } else {
        (void)close(pipeFd[1]);
        ReadPipeOutputForExec(pipeFd[0], output, cmd.empty() ? "" : cmd[0]);
        (void)close(pipeFd[0]);
        int ret = CheckChildProcessExitStatus(pid, status, exitStatus);
        if (ret != E_OK) {
            ReportForkExecDiagIfNeeded(cmd, ret, ResolveForkExecExitCode(ret, status, exitStatus), output);
            return ret;
        }
    }
    return E_OK;
}

/*
 * ForkExecToFile - 通过 fork+exec 执行外部命令，将子进程 stdout 输出重定向到指定文件
 *
 * 【主要功能】
 * 在子进程中执行指定的命令行程序，将子进程的 stdout 输出重定向到
 * outputFilePath 指定的文件中（覆盖写入）。stderr 不做重定向，保留
 * 继承自父进程的文件描述符，错误信息走 hilog 输出。
 *
 * 【实现流程】
 * 1. 格式化命令：调用 FormatCmd() 将命令向量转为 execvp 所需的 char* 数组。
 * 2. fork 子进程：
 *    - fork 失败：返回 E_FORK。
 *    - 子进程（pid == 0）：
 *      a. 以 O_WRONLY | O_CREAT | O_TRUNC 模式打开 outputFilePath（文件不存在
 *         则创建，已存在则截断清空）。
 *      b. dup2(fd, STDOUT_FILENO)：将标准输出重定向到文件，命令的正常输出
 *         将写入文件。
 *      c. 不重定向 stderr：当前调用方（iso9660_operator、udf_operator）均使用
 *         isoinfo -x 提取 ISO 内文件，stdout 是提取的文件二进制内容，
 *         stderr 是错误文本信息。若将 stderr 也重定向到同一文件，错误文本
 *         会混入二进制流损坏输出文件。因此 stderr 保留继承自父进程的 fd，
 *         错误信息通过 hilog 输出，便于问题定位且不污染输出文件。
 *      d. close(fd)：fd 已被 dup2 复制到 STDOUT，原始 fd 不再需要。
 *      e. execvp() 执行目标命令。成功时不返回；失败则 _exit(1)。
 *    - 父进程（pid > 0）：
 *      调用 CheckChildProcessExitStatus() 等待子进程结束。
 * 3. 此函数不使用管道，不调用 ReportForkExecDiagIfNeeded 上报诊断信息。
 *    output 参数保留用于接口兼容，但不会填充数据（子进程输出直接写文件）。
 *
 * 【与 ForkExec 的区别】
 * - 输出目标不同：ForkExec 将子进程 stdout/stderr 通过管道捕获到内存
 *   （output vector）；ForkExecToFile 将子进程 stdout 重定向到指定文件
 *   （通过 dup2 到文件描述符），不使用管道传输数据。
 * - stderr 处理不同：ForkExec 将 stderr 通过管道一并捕获到内存；
 *   ForkExecToFile 不重定向 stderr，错误信息走 hilog，避免错误文本
 *   混入输出文件损坏数据（尤其对 isoinfo -x 等二进制输出场景）。
 * - 诊断上报：ForkExec 在子进程异常退出时上报诊断信息
 *   （ReportForkExecDiagIfNeeded），ForkExecToFile 不上报。
 * - 退出码获取：ForkExec 可通过 exitStatus 参数获取子进程退出码，
 *   ForkExecToFile 不获取退出码。
 * - 参数校验：ForkExec 会检查 cmd 是否为空，ForkExecToFile 未做此检查。
 * - 适用场景：ForkExec 适用于需要读取命令输出内容进行解析的场景
 *   （如读取 mount 命令的输出）；ForkExecToFile 适用于命令输出需
 *   直接持久化到文件的场景（如 isoinfo 提取 ISO 内文件到本地）。
 */
int ForkExecToFile(std::vector<std::string> &cmd, const std::string &outputFilePath,
                   std::vector<std::string> *output)
{
    pid_t pid;
    int status;
    auto args = FormatCmd(cmd);
    std::string cmdName = cmd.empty() ? "" : cmd[0];
    if (output != nullptr) {
        output->clear();
    }
    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ForkExecToFile: <<< EXIT FAILED <<< fork failed,"
            "errno=%{public}d, cmd=%{public}s", errno, cmdName.c_str());
        return E_FORK;
    } else if (pid == 0) {
        // 子进程：将 stdout 重定向到输出文件，然后执行命令
        int fd = open(outputFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_OUTPUT_FILE_MODE);
        if (fd < 0) {
            LOGE("[L8:FileUtils] ForkExecToFile: open output file failed, errno=%{public}d", errno);
            _exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            LOGE("[L8:FileUtils] ForkExecToFile: dup2 stdout failed, errno=%{public}d", errno);
            close(fd);
            _exit(1);
        }
        close(fd);
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ForkExecToFile: <<< EXIT FAILED <<< execvp failed, errno=%{public}d,"
            "cmd=%{public}s", errno, cmdName.c_str());
        _exit(1);
    } else {
        int ret = CheckChildProcessExitStatus(pid, status, nullptr);
        if (ret != E_OK) {
            ReportForkExecDiagIfNeeded(cmd, ret, ResolveForkExecExitCode(ret, status, nullptr), output);
            LOGE("[L8:FileUtils] ForkExecToFile: <<< EXIT FAILED <<< ret=%{public}d, status=%{public}d",
                ret, status);
            return ret;
        }
    }
    return E_OK;
}

int ForkExecWithExit(std::vector<std::string> &cmd, int *exitStatus, std::vector<std::string> *output)
{
    int pipe_fd[PIPE_FD_LEN];
    LOGD("[L8:FileUtils] ForkExecWithExit: >>> ENTER <<< cmd=%{public}s", cmd.empty() ? "" : cmd[0].c_str());
    pid_t pid;
    int status = 0;
    auto args = FormatCmd(cmd);

    if (pipe(pipe_fd) < 0) {
        LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< create pipe failed");
        ReportForkExecDiagIfNeeded(cmd, E_CREATE_PIPE, errno, output);
        return E_CREATE_PIPE;
    }

    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< fork failed");
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        ReportForkExecDiagIfNeeded(cmd, E_FORK, errno, output);
        return E_FORK;
    } else if (pid == 0) {
        RedirectChildStd(pipe_fd, output != nullptr);
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< execvp failed, errno=%{public}d", errno);
        _exit(1);
    } else {
        (void)close(pipe_fd[1]);
        if (output != nullptr) {
            ReadPipeOutputForExec(pipe_fd[0], output, cmd.empty() ? "" : cmd[0]);
        }
        (void)close(pipe_fd[0]);

        pid_t waitRet = waitpid(pid, &status, 0);
        if (waitRet == -1) {
            if (errno == ECHILD) {
                LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< ECHILD");
                return E_NO_CHILD;
            }
            LOGE("[L8:FileUtils] ForkExecWithExit: <<< EXIT FAILED <<< waitpid failed, errno=%{public}d", errno);
            return E_SYS_KERNEL_ERR;
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
    LOGD("[L8:FileUtils] ForkExecWithExit: <<< EXIT SUCCESS <<<");
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
        LOGE("[L8:FileUtils] WritePidToPipe: pipe param is invalid.");
        return;
    }
    (void)close(pipe_fd[0]);
    int send_pid = (int)getpid();
    if (write(pipe_fd[1], &send_pid, sizeof(int)) == -1) {
        LOGE("[L8:FileUtils] WritePidToPipe: <<< EXIT FAILED <<< write pipe failed, errno=%{public}d", errno);
        _exit(1);
    }
    (void)close(pipe_fd[1]);
}

static void ReadPidFromPipe(std::vector<std::string> &cmd, int pipe_fd[2])
{
    (void)close(pipe_fd[1]);
    int recv_pid = 0;
    while (read(pipe_fd[0], &recv_pid, sizeof(int)) > 0) {
        LOGI("[L8:FileUtils] ReadPidFromPipe: read child pid=%{public}d", recv_pid);
    }
    (void)close(pipe_fd[0]);
    ReportExecutorPidEvent(cmd, recv_pid);
}

static void ReadLogFromPipe(int logpipe[PIPE_FD_LEN], size_t len, std::vector<std::string> *output)
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
            if (output != nullptr) {
                output->emplace_back(line);
            }
        }
        fclose(fp);
        return;
    }
    LOGE("[L8:FileUtils] ReadLogFromPipe: <<< EXIT FAILED <<< open pipe file failed, errno=%{public}d", errno);
    (void)close(logpipe[0]);
}

int ExtStorageMountForkExec(std::vector<std::string> &cmd, int *exitStatus)
{
    if (cmd.empty()) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< cmd is empty");
        return E_PARAMS_INVALID;
    }
    int pipe_fd[PIPE_FD_LEN];
    int pipe_log_fd[PIPE_FD_LEN]; /* for mount.exfat log*/
    pid_t pid;
    int status = 0;
    auto args = FormatCmd(cmd);

    if (pipe(pipe_fd) < 0) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< create pipe failed, errno=%{public}d", errno);
        ReportForkExecDiagIfNeeded(cmd, E_ERR, errno, nullptr);
        return E_ERR;
    }

    if (pipe(pipe_log_fd) < 0) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< create pipe for log failed,"
            "errno=%{public}d", errno);
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        ReportForkExecDiagIfNeeded(cmd, E_ERR, errno, nullptr);
        return E_ERR;
    }

    pid = fork();
    if (pid == -1) {
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< fork failed, errno=%{public}d", errno);
        ClosePipe(pipe_fd, PIPE_FD_LEN);
        ClosePipe(pipe_log_fd, PIPE_FD_LEN);
        ReportForkExecDiagIfNeeded(cmd, E_ERR, errno, nullptr);
        return E_ERR;
    } else if (pid == 0) {
        WritePidToPipe(pipe_fd, PIPE_FD_LEN);
        if (RedirectStdToPipe(pipe_log_fd, PIPE_FD_LEN)) {
            _exit(1);
        }
        execvp(args[0], const_cast<char **>(args.data()));
        LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< execvp failed, errno=%{public}d", errno);
        _exit(1);
    } else {
        ReadPidFromPipe(cmd, pipe_fd);
        std::vector<std::string> mountLog;
        ReadLogFromPipe(pipe_log_fd, PIPE_FD_LEN, &mountLog);

        pid_t waitRet = waitpid(pid, &status, 0);
        if (waitRet == -1) {
            if (errno == ECHILD) {
                LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< ECHILD");
                ReportForkExecDiagIfNeeded(cmd, E_NO_CHILD, errno, &mountLog);
                return E_NO_CHILD;
            }
            LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< "
                "waitpid failed, errno=%{public}d", errno);
            return E_SYS_KERNEL_ERR;
        }
        if (!WIFEXITED(status)) {
            LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< "
                "Process exits abnormally, status=%{public}d", status);
            ReportForkExecDiagIfNeeded(cmd, E_ERR, -1, &mountLog);
            return E_ERR;
        }
        int tempExitStatus = WEXITSTATUS(status);
        GetExitStatus(exitStatus, tempExitStatus);
        if (tempExitStatus != 0) {
            LOGE("[L8:FileUtils] ExtStorageMountForkExec: <<< EXIT FAILED <<< "
                "Process exited with error, status=%{public}d", status);
            ReportForkExecDiagIfNeeded(cmd, E_ERR, tempExitStatus, &mountLog);
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

void KillProcess(const std::vector<ProcessInfo> &processList, std::vector<ProcessInfo> &killFailList)
{
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
    return true;
}

int32_t GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    if (!IsValidRgmName(rgmName)) {
        LOGE("[L8:FileUtils] GetRmgResourceSize: <<< EXIT FAILED <<< rgm name %{public}s invalid", rgmName.c_str());
        return E_CONTAINERPLUGIN_UTILS_RGM_NAME_INVALID;
    }
    std::vector<std::string> ignorePaths;
    ignorePaths.clear();
    return StatisticsFilesTotalSize(rgmConfigs.at(rgmName).mgrPath, ignorePaths, totalSize);
}

int32_t GetRmgDataSize(const std::string &rgmName, const std::string &path,
    const std::vector<std::string> &ignorePaths, uint64_t &totalSize)
{
    LOGI("[L8:FileUtils] GetRmgDataSize: >>> ENTER <<< rgmName=%{public}s, path=%{public}s",
         rgmName.c_str(), path.c_str());
    if (!IsValidRgmName(rgmName)) {
        LOGE("[L8:FileUtils] GetRmgDataSize: <<< EXIT FAILED <<< rgm name invalid, rgmName=%{public}s, path=%{public}s",
            rgmName.c_str(), path.c_str());
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
    return StatisticsFilesTotalSize(realPath, innerIgnorePaths, totalSize);
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
    if (!IsFileExist(dirPath)) {
        LOGE("[L8:FileUtils] StatisticsFilesTotalSize: <<< EXIT SUCCESS <<< path not exist");
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
    constexpr const char *PATH_INVALID_FLAG1 = "../";
    constexpr const char *PATH_INVALID_FLAG2 = "/..";
    constexpr int32_t PATH_INVALID_FLAG_LEN = 3;
    constexpr char FILE_SEPARATOR_CHAR = '/';
    size_t pos = filePath.find(PATH_INVALID_FLAG1);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("Relative path is not allowed, path contain ../");
            return true;
        }
        pos = filePath.find(PATH_INVALID_FLAG1, pos + PATH_INVALID_FLAG_LEN);
    }
    pos = filePath.rfind(PATH_INVALID_FLAG2);
    if ((pos != std::string::npos) && (filePath.size() - pos == PATH_INVALID_FLAG_LEN)) {
        LOGE("Relative path is not allowed, path tail is /..");
        return true;
    }
    return false;
}

bool IsShellMetacharPresent(const std::string& str)
{
    static const std::string shellChars = "\"$`\\;|&!(){}<>*?[ ]^~\n";
    return str.find_first_of(shellChars) != std::string::npos;
}

bool GetRealPath(const std::string &path, std::string &realPath)
{
    char resolvedPath[PATH_MAX] = { 0 };
    if (path.size() >= PATH_MAX || !realpath(path.c_str(), resolvedPath)) {
        LOGE("[L8:FileUtils] GetRealPath: %{public}s realpath failed", path.c_str());
        return false;
    }
    realPath = std::string(resolvedPath);
    return true;
}

bool CleanOrphanNode()
{
    LOGI("Clean orphan node start");
    if (!SaveStringToFile(DROP_ENCRYPT_DENTRY_PATH, DROP_ENCRYPT_DENTRY_VALUE)) {
        LOGE("Failed to clean orphan node, errno=%{public}d", errno);
        return false;
    }
    LOGI("Clean orphan node success");
    return true;
}

void CheckAndReportOverLoop(const std::string &funcName, uint32_t &loopCount)
{
    if (loopCount <= OVER_LOOP_COUNT_GROW_CAP) {
        loopCount++;
    }
    if (loopCount == OVER_LOOP_FIRST_ALERT_THRESHOLD || loopCount == OVER_LOOP_SECOND_ALERT_THRESHOLD) {
        StorageRadar::ReportUserKeyResult("ReportOverLoopCount for function: " + funcName,
            DEFAULT_USERID, E_OK, "ELx", "");
    }
}

void CheckAndReportOverLoop(const std::string &funcName, uint32_t &loopCount, uint32_t maxCount)
{
    if (loopCount <= OVER_LOOP_COUNT_GROW_CAP_MULTIPLE * maxCount) {
        loopCount++;
    }
    if (loopCount == maxCount || loopCount == OVER_LOOP_ALERT_HALF_MAX_MULTIPLE * maxCount) {
        StorageRadar::ReportUserKeyResult("ReportOverLoopCount for function: " + funcName,
            DEFAULT_USERID, E_OK, "ELx", "");
    }
}
} // namespace StorageDaemon
} // namespace OHOS
