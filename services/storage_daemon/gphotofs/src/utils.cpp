/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "utils.h"
#include <gphoto2/gphoto2.h>
#include "storage_service_log.h"
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <securec.h>
#include <sstream>
#include <libgen.h>
#include "gphotofs_fuse.h"
#include "file_utils.h"
using namespace std;

static constexpr char *INVALID_PREFIX_PATH = "../";
static constexpr char *INVALID_SUFFIX_PATH = "/..";
static const char FILE_SEPARATOR_CHAR = '/';

constexpr size_t INVALID_DIR_PATH_LEN = 16;
constexpr size_t INVALID_PREFIX_PATH_LEN = 3;
constexpr size_t INVALID_SUFFIX_PATH_LEN = 3;


int Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec;
}

off_t SizeToBlocks(off_t size)
{
    return size / SIZE_TO_BLOCK + (size % SIZE_TO_BLOCK ? 1 : 0);
}

static bool IsIOError(int result)
{
    return result == GP_ERROR_IO ||
           result == GP_ERROR_IO_SUPPORTED_SERIAL ||
           result == GP_ERROR_IO_SUPPORTED_USB ||
           result == GP_ERROR_IO_INIT ||
           result == GP_ERROR_IO_READ ||
           result == GP_ERROR_IO_WRITE ||
           result == GP_ERROR_IO_UPDATE ||
           result == GP_ERROR_IO_SERIAL_SPEED ||
           result == GP_ERROR_IO_USB_CLEAR_HALT ||
           result == GP_ERROR_IO_USB_FIND ||
           result == GP_ERROR_IO_USB_CLAIM ||
           result == GP_ERROR_IO_LOCK;
}

static bool IsFileDirError(int result)
{
    return result == GP_ERROR_FILE_NOT_FOUND ||
           result == GP_ERROR_DIRECTORY_NOT_FOUND ||
           result == GP_ERROR_FILE_EXISTS ||
           result == GP_ERROR_DIRECTORY_EXISTS ||
           result == GP_ERROR_PATH_NOT_ABSOLUTE ||
           result == GP_ERROR_CORRUPTED_DATA;
}

static int HandleFileDirError(int result)
{
    static const std::unordered_map<int, int> fileDirErrorMap = {
        {GP_ERROR_FILE_NOT_FOUND, -ENOENT},
        {GP_ERROR_DIRECTORY_NOT_FOUND, -ENOENT},
        {GP_ERROR_FILE_EXISTS, -EEXIST},
        {GP_ERROR_DIRECTORY_EXISTS, -EEXIST},
        {GP_ERROR_PATH_NOT_ABSOLUTE, -ENOTDIR},
        {GP_ERROR_CORRUPTED_DATA, -EIO}
    };
    
    auto it = fileDirErrorMap.find(result);
    return (it != fileDirErrorMap.end()) ? it->second : -EINVAL;
}

int GpresultToErrno(int result)
{
    LOGE("gphoto result error: %{public}d", result);
    
    if (IsIOError(result)) {
        return -EIO;
    }
    
    if (IsFileDirError(result)) {
        return HandleFileDirError(result);
    }
    
    static const std::unordered_map<int, int> errorMap = {
        {GP_ERROR, -EPROTO},
        {GP_ERROR_BAD_PARAMETERS, -EINVAL},
        {GP_ERROR_NO_MEMORY, -ENOMEM},
        {GP_ERROR_LIBRARY, -ENOSYS},
        {GP_ERROR_UNKNOWN_PORT, -ENXIO},
        {GP_ERROR_NOT_SUPPORTED, -EPROTONOSUPPORT},
        {GP_ERROR_TIMEOUT, -ETIMEDOUT},
        {GP_ERROR_CAMERA_BUSY, -EBUSY},
        {GP_ERROR_CANCEL, -ECANCELED},
        {GP_ERROR_MODEL_NOT_FOUND, -EPROTO},
        {GP_ERROR_CAMERA_ERROR, -EPERM},
        {GP_ERROR_OS_FAILURE, -EPIPE}
    };
    
    auto it = errorMap.find(result);
    if (it != errorMap.end()) {
        return it->second;
    }
    
    return -EINVAL;
}

std::string SmtpfsDirName(const std::string &path)
{
    if (path.empty() || path.length() > PATH_MAX) {
        return "";
    }
    char *str = strdup(path.c_str());
    if (!str) {
        return "";
    }
    std::string result(dirname(str));
    free(static_cast<void *>(str));
    str = nullptr;
    return result;
}

std::string SmtpfsBaseName(const std::string &path)
{
    if (path.empty() || path.length() > PATH_MAX) {
        return "";
    }
    char *str = strdup(path.c_str());
    if (!str) {
        return "";
    }
    std::string result(basename(str));
    free(static_cast<void *>(str));
    str = nullptr;
    return result;
}

std::string SmtpfsRealPath(const std::string &path)
{
    if (path.empty() || path.length() > PATH_MAX) {
        return "";
    }
    char realPath[PATH_MAX] = {0x00};
    if (realpath(path.c_str(), realPath) == nullptr) {
        LOGE("path not valid");
        return "";
    }
    
    return std::string(realPath);
}

std::string SmtpfsGetTmpDir()
{
    DelTemp(TMP_FULL_PATH);

    std::string tmpDir = SmtpfsRealPath(TMP_FULL_PATH) + "/simple-gphotofs-XXXXXX";
    if (tmpDir.length() > PATH_MAX) {
        LOGE("SmtpfsGetTmpDir: path too long");
        return "";
    }
    char *cTempDir = ::strdup(tmpDir.c_str());
    if (cTempDir == nullptr) {
        return "";
    }
    char *cTmpDir = ::mkdtemp(cTempDir);
    if (cTmpDir == nullptr) {
        ::free(static_cast<void *>(cTempDir));
        cTempDir = nullptr;
        return "";
    }
    tmpDir.assign(cTmpDir);
    ::free(static_cast<void *>(cTempDir));
    cTempDir = nullptr;
    return tmpDir;
}

void DelTemp(const std::string &path)
{
    DIR *dir;
    if (!OHOS::StorageDaemon::IsDir(path)) {
        return;
    }
    if ((dir = opendir(path.c_str())) != nullptr) {
        struct dirent *dirinfo = ::readdir(dir);
        while (dirinfo != nullptr) {
            if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) {
                dirinfo = ::readdir(dir);
                continue;
            }
            std::string filePath;
            filePath.append(path).append("/").append(dirinfo->d_name);
            if (OHOS::StorageDaemon::IsTempFolder(filePath, "simple-mtpfs")) {
                OHOS::StorageDaemon::DeleteFile(filePath.c_str());
                rmdir(filePath.c_str());
            }
            dirinfo = ::readdir(dir);
        }
        closedir(dir);
    }
}

bool SmtpfsCheckDir(const std::string &path)
{
    struct stat buf;
    if (::stat(path.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode)) {
        return true;
    }
    return false;
}

static bool IsEntryDirectory(const std::string &path, struct dirent *entry)
{
    bool isDir = false;
#ifdef DT_DIR
    if (entry->d_type == DT_DIR) {
        isDir = true;
    } else if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK) {
        struct stat st;
        if (::lstat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            isDir = true;
        }
    }
#else
    struct stat st;
    if (::lstat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        isDir = true;
    }
#endif
    return isDir;
}

static bool RemoveSingleFile(const std::string &path)
{
    if (::unlink(path.c_str()) != 0) {
        int err = errno;
        LOGE("gphoto unlink failed errno=%{public}d", err);
        return false;
    }
    return true;
}

static bool ProcessDirEntry(const std::string &dirName, struct dirent *entry)
{
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        return true;
    }
    std::string path = dirName + "/" + entry->d_name;
    bool isDir = IsEntryDirectory(path, entry);
    if (isDir) {
        return SmtpfsRemoveDir(path);
    } else {
        return RemoveSingleFile(path);
    }
}

bool SmtpfsRemoveDir(const std::string &dirName)
{
    DIR *dir = ::opendir(dirName.c_str());
    if (dir == nullptr) {
        LOGE("gphoto opendir failed: %{public}d", errno);
        return false;
    }

    struct dirent *entry;
    bool ok = true;

    int entryCount = 0;
    const int maxEntryCount = 10000;
    while ((entry = ::readdir(dir)) != nullptr) {
        entryCount++;
        if (entryCount > maxEntryCount) {
            LOGE("SmtpfsRemoveDir: too many entries, possible loop");
            ok = false;
            break;
        }
        if (!ProcessDirEntry(dirName, entry)) {
            ok = false;
        }
    }
    ::closedir(dir);
    dir = nullptr;
    if (::rmdir(dirName.c_str()) != 0) {
        int err = errno;
        LOGE("gphoto rmdir failed: %{public}d", err);
        ok = false;
    }
    return ok;
}

bool SmtpfsCreateDir(const std::string &dirName)
{
    return ::mkdir(dirName.c_str(), S_IRWXU) == 0;
}

bool CreateTmpDir()
{
    if (RemoveTmpDir()) {
        return SmtpfsCreateDir(TMP_FULL_PATH);
    }
    return false;
}

bool RemoveTmpDir()
{
    if (!SmtpfsCheckDir(TMP_FULL_PATH)) {
        return true;
    }
    return SmtpfsRemoveDir(TMP_FULL_PATH);
}

bool IsFilePathValid(const std::string &filePath)
{
    if (filePath.empty()) {
        LOGE("IsFilePathValid: file path is empty");
        return false;
    }

    size_t pos = filePath.find(INVALID_PREFIX_PATH);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("IsFilePathValid: Relative path is not allowed");
            return false;
        }
        pos = filePath.find(INVALID_PREFIX_PATH, pos + INVALID_PREFIX_PATH_LEN);
    }

    pos = filePath.rfind(INVALID_SUFFIX_PATH);
    if ((pos != std::string::npos) && (filePath.size() - pos == INVALID_SUFFIX_PATH_LEN)) {
        LOGE("IsFilePathValid: Relative path is not allowed");
        return false;
    }

    return true;
}