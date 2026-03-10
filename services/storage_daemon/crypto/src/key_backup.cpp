/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "key_backup.h"

#include <dirent.h>
#include <fcntl.h>
#include <thread>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "unique_fd.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t INVALID_LOOP_NUM = 0xFFFFFFFF;
constexpr uint8_t BACK_MAX_RETRY_TIME = 3;
constexpr uint16_t BACK_RETRY_INTERVAL_MS = 50 * 1000;

constexpr const char *BACKUP_NAME = "_bak";
struct FileNode {
    std::string baseName;
    std::string origFile;
    std::string backFile;
    bool isSame;
};

void KeyBackup::CreateBackup(const std::string &from, const std::string &to, bool removeOld)
{
    LOGI("[L4:KeyBackup] CreateBackup: >>> ENTER <<< from=%{public}s, to=%{public}s, removeOld=%{public}d",
        from.c_str(), to.c_str(), removeOld ? 1 : 0);
    if (access(from.c_str(), 0) != 0) {
        LOGE("[L4:KeyBackup] CreateBackup: <<< EXIT FAILED <<< from path not exist, path=%{public}s", from.c_str());
        return;
    }

    if (access(to.c_str(), 0) == 0) {
        if (removeOld) {
            if (RemoveNode(to) != 0) {
                LOGE("[L4:KeyBackup] CreateBackup: <<< EXIT FAILED <<< failed to remove to=%{public}s", to.c_str());
            }
        }
    } else {
        int32_t ret = MkdirParentWithRetry(to, DEFAULT_DIR_PERM);
        if (ret != 0) {
            LOGE("[L4:KeyBackup] CreateBackup: <<< EXIT FAILED <<< path=%{public}s", to.c_str());
            return;
        }
    }
    CheckAndCopyFiles(from, to);
    LOGI("[L4:KeyBackup] CreateBackup: <<< EXIT SUCCESS <<<");
}

int32_t KeyBackup::RemoveNode(const std::string &pathName)
{
    LOGD("[L4:KeyBackup] RemoveNode: >>> ENTER <<< pathName=%{public}s", pathName.c_str());
    struct stat st;
    if (lstat(pathName.c_str(), &st) < 0) {
        int32_t ret = (errno == ENOENT) ? 0 : -1;
        LOGI("[L4:KeyBackup] RemoveNode: <<< EXIT %s <<< pathName=%{public}s",
             ret == 0 ? "SUCCESS" : "FAILED", pathName.c_str());
        return ret;
    }

    if (!S_ISDIR(st.st_mode)) {
        CleanFile(pathName);
        int32_t ret = remove(pathName.c_str());
        LOGI("[L4:KeyBackup] RemoveNode: <<< EXIT %s <<< pathName=%{public}s",
             ret == 0 ? "SUCCESS" : "FAILED", pathName.c_str());
        return ret;
    }

    DIR *dir = opendir(pathName.c_str());
    if (dir == nullptr) {
        LOGE("[L4:KeyBackup] RemoveNode: <<< EXIT FAILED <<< opendir failed, pathName=%{public}s", pathName.c_str());
        return -1;
    }

    struct dirent *de = nullptr;
    bool rmSubNodeFail = false;
    errno = 0;
    while ((de = readdir(dir)) != nullptr) {
        if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, ".")) {
            continue;
        }
        std::string dn = pathName + "/" + std::string(de->d_name);
        if (RemoveNode(dn) < 0) {
            rmSubNodeFail = true;
            break;
        }
        errno = 0;
    }

    if (errno < 0 || rmSubNodeFail) {
        closedir(dir);
        LOGE("[L4:KeyBackup] RemoveNode: <<< EXIT FAILED <<< remove subnode failed, pathName=%{public}s",
             pathName.c_str());
        return -1;
    }

    if (closedir(dir) < 0) {
        LOGE("[L4:KeyBackup] RemoveNode: <<< EXIT FAILED <<< closedir failed, pathName=%{public}s", pathName.c_str());
        return -1;
    }
    int32_t ret = rmdir(pathName.c_str());
    LOGD("[L4:KeyBackup] RemoveNode: <<< EXIT %s <<< pathName=%{public}s",
         ret == 0 ? "SUCCESS" : "FAILED", pathName.c_str());
    return ret;
}

int32_t KeyBackup::TryRestoreKey(const std::shared_ptr<BaseKey> &baseKey, const UserAuth &auth)
{
    LOGI("[L4:KeyBackup] TryRestoreKey: >>> ENTER <<<");
    if (baseKey == nullptr) {
        LOGE("[L4:KeyBackup] TryRestoreKey: <<< EXIT FAILED <<< basekey is nullptr");
        return -1;
    }
    std::string keyDir = baseKey->GetDir();
    std::string backupDir;
    GetBackupDir(keyDir, backupDir);
    if (baseKey->DoRestoreKey(auth, keyDir + PATH_LATEST) == E_OK) {
        std::thread fixFileThread([this, keyDir, backupDir]() { CheckAndFixFiles(keyDir, backupDir); });
        fixFileThread.detach();
        LOGI("[L4:KeyBackup] TryRestoreKey: <<< EXIT SUCCESS <<< Restore by main key success");
        return 0;
    }
    LOGE("[L4:KeyBackup] TryRestoreKey: origKey failed, try backupKey");
    if (baseKey->DoRestoreKey(auth, backupDir + PATH_LATEST) == E_OK) {
        CheckAndFixFiles(backupDir, keyDir);
        LOGI("[L4:KeyBackup] TryRestoreKey: <<< EXIT SUCCESS <<< Restore by back key success");
        return 0;
    }

    LOGE("[L4:KeyBackup] TryRestoreKey: <<< EXIT FAILED <<< origKey failed, backupKey failed, so mix key");
    return -1;
}

int32_t KeyBackup::TryRestoreUeceKey(const std::shared_ptr<BaseKey> &baseKey,
                                     const UserAuth &auth,
                                     KeyBlob &planKey,
                                     KeyBlob &decryptedKey)
{
    LOGI("[L4:KeyBackup] TryRestoreUeceKey: >>> ENTER <<<");
    if (baseKey == nullptr) {
        LOGE("[L4:KeyBackup] TryRestoreUeceKey: <<< EXIT FAILED <<< basekey is nullptr");
        return -1;
    }
    std::string keyDir = baseKey->GetDir();
    std::string backupDir;
    GetBackupDir(keyDir, backupDir);
    auto ret = baseKey->DecryptKeyBlob(auth, keyDir + PATH_LATEST, planKey, decryptedKey);
    if (ret == E_OK) {
        CheckAndFixFiles(keyDir, backupDir);
        LOGI("[L4:KeyBackup] TryRestoreUeceKey: <<< EXIT SUCCESS <<< Restore uece by main key success");
        return 0;
    }
    LOGE("[L4:KeyBackup] TryRestoreUeceKey: origKey failed, try backupKey");
    ret = baseKey->DecryptKeyBlob(auth, backupDir + PATH_LATEST, planKey, decryptedKey);
    if (ret == E_OK) {
        CheckAndFixFiles(backupDir, keyDir);
        LOGI("[L4:KeyBackup] TryRestoreUeceKey: <<< EXIT SUCCESS <<< Restore uece by back key success");
        return 0;
    }

    LOGE("[L4:KeyBackup] TryRestoreUeceKey: <<< EXIT FAILED <<< origKey failed, backupKey failed, so mix key");
    return -1;
}

int32_t KeyBackup::GetBackupDir(std::string &origDir, std::string &backupDir)
{
    LOGI("[L4:KeyBackup] GetBackupDir: >>> ENTER <<< origDir=%{public}s", origDir.c_str());
    if (origDir == DEVICE_EL1_DIR) {
        backupDir = std::string(DEVICE_EL1_DIR) + BACKUP_NAME;
        LOGI("[L4:KeyBackup] GetBackupDir: <<< EXIT SUCCESS <<< backupDir=%{public}s", backupDir.c_str());
        return 0;
    }

    auto slashIndex = origDir.rfind("/");
    if (slashIndex == std::string::npos || slashIndex == 0) {
        LOGE("[L4:KeyBackup] GetBackupDir: <<< EXIT FAILED <<< invalid origDir=%{public}s", origDir.c_str());
        return -1;
    }
    std::string prefixStr = origDir.substr(0, slashIndex);
    std::string endStr = origDir.substr(slashIndex);
    backupDir = prefixStr + BACKUP_NAME + endStr;
    LOGI("[L4:KeyBackup] GetBackupDir: <<< EXIT SUCCESS <<< backupDir=%{public}s", backupDir.c_str());
    return 0;
}

void KeyBackup::ListAndCheckDir(std::string &origDir)
{
    LOGD("[L4:KeyBackup] ListAndCheckDir: >>> ENTER <<< origDir=%{public}s", origDir.c_str());
    if (access(origDir.c_str(), F_OK) == 0) {
        LOGI("[L4:KeyBackup] ListAndCheckDir: <<< EXIT SUCCESS <<< dir exists");
        return;
    }
    LOGW("[L4:KeyBackup] ListAndCheckDir: origDir is not exist %{public}s", origDir.c_str());
    std::string backupDir;
    int32_t ret = GetBackupDir(origDir, backupDir);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] ListAndCheckDir: <<< EXIT FAILED <<< origDir=%{public}s", origDir.c_str());
        return;
    }
    if (access(backupDir.c_str(), F_OK) == 0) {
        LOGW("[L4:KeyBackup] ListAndCheckDir: origDir=%{public}s backupDir=%{public}s",
             origDir.c_str(), backupDir.c_str());
        ret = MkdirParent(origDir, DEFAULT_DIR_PERM);
        if (ret != 0) {
            LOGE("[L4:KeyBackup] ListAndCheckDir: <<< EXIT FAILED <<< MkdirParent failed");
            return;
        }
        CheckAndCopyFiles(backupDir, origDir);
    }
    LOGD("[L4:KeyBackup] ListAndCheckDir: <<< EXIT SUCCESS <<<");
    return;
}

int32_t KeyBackup::DoResotreKeyMix(std::shared_ptr<BaseKey> &baseKey, const UserAuth &auth, const std::string &keyDir,
    const std::string &backupDir)
{
    LOGI("[L4:KeyBackup] DoResotreKeyMix: >>> ENTER <<<");
    std::string origKeyDir = keyDir + PATH_LATEST;
    std::string backupKeyDir = backupDir + PATH_LATEST;
    std::vector<struct FileNode> fileList;
    uint32_t diffNum = 0;
    int32_t ret = GetFileList(origKeyDir, backupKeyDir, fileList, diffNum);
    if (ret != 0 || diffNum <= 1) {
        LOGE("[L4:KeyBackup] DoResotreKeyMix: <<< EXIT FAILED <<< get file list failed or diffNum too least,"
             "ret=%{public}d, diffNum=%{public}d", ret, diffNum);
        return E_ERR;
    }

    std::string tempKeyDir;
    ret = CopySameFilesToTempDir(backupKeyDir, tempKeyDir, fileList);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] DoResotreKeyMix: <<< EXIT FAILED <<< CopySameFilesToTempDir failed");
        return E_ERR;
    }

    diffNum = fileList.size();
    uint32_t loopNum = GetLoopMaxNum(diffNum);
    if (loopNum == INVALID_LOOP_NUM) {
        RemoveNode(tempKeyDir);
        LOGE("[L4:KeyBackup] DoResotreKeyMix: <<< EXIT FAILED <<< loopNum is invalid");
        return E_ERR;
    }
    for (uint32_t i = 0; i <= loopNum; i++) {
        LOGI("[L4:KeyBackup] DoResotreKeyMix: try mix key files to decrypt i=%{public}u loopNum=%{public}u",
             i, loopNum);
        ret = CopyMixFilesToTempDir(diffNum, i, tempKeyDir, fileList);
        if (ret != 0) {
            LOGE("[L4:KeyBackup] DoResotreKeyMix: copy mix files to temp dir failed, i=%{public}u", i);
            continue;
        }
        if (baseKey == nullptr) {
            LOGE("[L4:KeyBackup] DoResotreKeyMix: <<< EXIT FAILED <<< basekey is nullptr");
            RemoveNode(tempKeyDir);
            return E_ERR;
        }
        if (baseKey->DoRestoreKey(auth, tempKeyDir) == E_OK) {
            LOGI("[L4:KeyBackup] DoResotreKeyMix: mix key files decrypt succ, fix orig and backup");
            CheckAndFixFiles(tempKeyDir, origKeyDir);
            CheckAndFixFiles(tempKeyDir, backupKeyDir);
            RemoveNode(tempKeyDir);
            LOGI("[L4:KeyBackup] DoResotreKeyMix: <<< EXIT SUCCESS <<<");
            return E_OK;
        }
    }
    RemoveNode(tempKeyDir);
    LOGE("[L4:KeyBackup] DoResotreKeyMix: <<< EXIT FAILED <<< all attempts failed");
    return E_ERR;
}

int32_t KeyBackup::GetFileList(const std::string &origDir, const std::string &backDir,
    std::vector<struct FileNode> &fileList, uint32_t diffNum)
{
    LOGI("[L4:KeyBackup] GetFileList: >>> ENTER <<< origDir=%{public}s, backDir=%{public}s",
         origDir.c_str(), backDir.c_str());
    DIR *dir = opendir(origDir.c_str());
    if (dir == nullptr) {
        LOGE("[L4:KeyBackup] GetFileList: <<< EXIT FAILED <<< fail to open origDir=%{public}s", origDir.c_str());
        return -1;
    }
    struct dirent *de = nullptr;
    while ((de = readdir(dir)) != nullptr) {
        AddOrigFileToList(std::string(de->d_name), origDir, fileList);
    }
    closedir(dir);

    dir = opendir(backDir.c_str());
    if (dir == nullptr) {
        LOGE("[L4:KeyBackup] GetFileList: <<< EXIT FAILED <<< fail to open backDir=%{public}s", backDir.c_str());
        return -1;
    }
    while ((de = readdir(dir)) != nullptr) {
        AddBackupFileToList(std::string(de->d_name), backDir, fileList);
    }
    closedir(dir);
    dir = nullptr;

    diffNum = GetDiffFilesNum(fileList);
    LOGI("[L4:KeyBackup] GetFileList: <<< EXIT SUCCESS <<< diffNum=%{public}d", diffNum);
    return 0;
}

bool KeyBackup::IsRegFile(const std::string &filePath)
{
    struct stat st;
    if (lstat(filePath.c_str(), &st) < 0) {
        LOGE("[L4:KeyBackup] IsRegFile: lstat failed %{public}s", filePath.c_str());
        return false;
    }

    if (!S_ISREG(st.st_mode)) {
        LOGE("[L4:KeyBackup] IsRegFile: filePath is not reg file %{public}s", filePath.c_str());
        return false;
    }
    return true;
}

void KeyBackup::AddOrigFileToList(const std::string &fileName, const std::string &origDir,
    std::vector<struct FileNode> &fileList)
{
    if (fileName.compare("..") == 0 || fileName.compare(".") == 0) {
        return;
    }

    std::string filePath = origDir + "/" + fileName;
    if (!IsRegFile(filePath)) {
        return;
    }

    struct FileNode fl;
    fl.baseName = fileName;
    fl.origFile = filePath;
    fl.backFile = "";
    fl.isSame = false;
    fileList.push_back(fl);
}

void KeyBackup::AddBackupFileToList(const std::string &fileName, const std::string &backDir,
    std::vector<struct FileNode> &fileList)
{
    if (fileName.compare("..") == 0 || fileName.compare(".") == 0) {
        return;
    }

    std::string filePath = backDir + "/" + fileName;
    if (!IsRegFile(filePath)) {
        return;
    }

    for (auto iter = fileList.begin(); iter != fileList.end(); ++iter) {
        if (iter->baseName.compare(fileName) == 0) {
            iter->backFile = backDir + "/" + fileName;
            if (CompareFile(iter->origFile, iter->backFile) == 0) {
                iter->isSame = true;
            }
            return;
        }
    }

    struct FileNode fl;
    fl.baseName = fileName;
    fl.origFile = "";
    fl.backFile = filePath;
    fl.isSame = false;
    fileList.push_back(fl);
}

uint32_t KeyBackup::GetDiffFilesNum(const std::vector<struct FileNode> &fileList)
{
    uint32_t diffNum = 0;
    for (auto iter = fileList.begin(); iter != fileList.end(); ++iter) {
        LOGI("[L4:KeyBackup] GetDiffFilesNum: origFile=%{public}s, backupFile=%{public}s, isSame=%{public}d,"
             "fileName=%{public}s",
            iter->origFile.c_str(), iter->backFile.c_str(), iter->isSame ? 0 : 1, iter->baseName.c_str());
        if (!iter->isSame) {
            diffNum++;
        }
    }
    LOGI("[L4:KeyBackup] GetDiffFilesNum: diff files num=%{public}d", diffNum);
    return diffNum;
}

int32_t KeyBackup::CopySameFilesToTempDir(const std::string &backupDir, std::string &tempDir,
    std::vector<struct FileNode> &fileList)
{
    LOGI("[L4:KeyBackup] CopySameFilesToTempDir: >>> ENTER <<< backupDir=%{public}s", backupDir.c_str());
    int32_t ret = CreateTempDirForMixFiles(backupDir, tempDir);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] CopySameFilesToTempDir: <<< EXIT FAILED <<< CreateTempDirForMixFiles failed");
        return -1;
    }

    for (auto iter = fileList.begin(); iter != fileList.end();) {
        if (iter->isSame || iter->backFile.empty()) {
            ret = CheckAndCopyOneFile(iter->origFile, tempDir + "/" + iter->baseName);
            if (ret != 0) {
                RemoveNode(tempDir);
                LOGE("[L4:KeyBackup] CopySameFilesToTempDir: <<< EXIT FAILED <<< CheckAndCopyOneFile failed");
                return -1;
            }
            iter = fileList.erase(iter);
        } else if (iter->origFile.empty()) {
            ret = CheckAndCopyOneFile(iter->backFile, tempDir + "/" + iter->baseName);
            if (ret != 0) {
                RemoveNode(tempDir);
                LOGE("[L4:KeyBackup] CopySameFilesToTempDir: <<< EXIT FAILED <<< CheckAndCopyOneFile failed");
                return -1;
            }
            iter = fileList.erase(iter);
        } else {
            ++iter;
        }
    }
    LOGI("[L4:KeyBackup] CopySameFilesToTempDir: <<< EXIT SUCCESS <<<");
    return 0;
}

int32_t KeyBackup::CreateTempDirForMixFiles(const std::string &backupDir, std::string &tempDir)
{
    auto pos = backupDir.rfind("/");
    std::string parentDir = backupDir.substr(0, pos);
    tempDir = parentDir + "/temp";

    RemoveNode(tempDir);
    int32_t ret = HandleCopyDir(backupDir, tempDir);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] CreateTempDirForMixFiles: <<< EXIT FAILED <<< ret=%{public}d, tempDir=%{public}s",
             ret, tempDir.c_str());
    } else {
        LOGI("[L4:KeyBackup] CreateTempDirForMixFiles: success, tempDir=%{public}s", tempDir.c_str());
    }
    return ret;
}

uint32_t KeyBackup::GetLoopMaxNum(uint32_t diffNum)
{
    if (diffNum > MAX_FILE_NUM) {
        LOGE("[L4:KeyBackup] GetLoopMaxNum: <<< EXIT FAILED <<< there are too many different files,"
             "diffNum=%{public}d", diffNum);
        return INVALID_LOOP_NUM;
    }

    const double fileNum = 2;
    uint32_t result = static_cast<uint32_t>(pow(fileNum, diffNum) - 1);
    LOGI("[L4:KeyBackup] GetLoopMaxNum: <<< EXIT SUCCESS <<< loopNum=%{public}u", result);
    return result;
}

int32_t KeyBackup::CopyMixFilesToTempDir(uint32_t diffNum, uint32_t num, const std::string &tempDir,
    const std::vector<struct FileNode> &fileList)
{
    for (uint32_t i = 0; i < diffNum; i++) {
        std::string from;
        if (num & (1UL << i)) {
            from = fileList[i].backFile;
        } else {
            from = fileList[i].origFile;
        }
        std::string to = tempDir + "/" + fileList[i].baseName;
        if (CheckAndCopyOneFile(from, to) != 0) {
            return -1;
        }
    }
    return 0;
}

void KeyBackup::CheckAndFixFiles(const std::string &from, const std::string &to)
{
    LOGD("[L4:KeyBackup] CheckAndFixFiles: >>> ENTER <<< from=%{public}s, to=%{public}s", from.c_str(), to.c_str());
    CreateBackup(from, to, false);
    LOGI("[L4:KeyBackup] CheckAndFixFiles: <<< EXIT SUCCESS <<<");
}

void KeyBackup::FsyncFile(const std::string &dirName)
{
    LOGI("[L4:KeyBackup] FsyncFile: >>> ENTER <<< dirName=%{public}s", dirName.c_str());
    std::string realPath;
    if (!GetRealPath(dirName, realPath)) {
        LOGI("[L4:KeyBackup] FsyncFile: <<< EXIT FAILED <<< GetRealPath failed");
        return;
    }

    UniqueFd fd(open(realPath.c_str(), O_RDONLY | O_CLOEXEC));
    if (fd < 0) {
        LOGE("[L4:KeyBackup] FsyncFile: <<< EXIT FAILED <<< failed to open %{public}s", realPath.c_str());
        return;
    }

    auto ret = fsync(fd);
    if (ret == -1) {
        if (errno == EROFS || errno == EINVAL) {
            LOGE("[L4:KeyBackup] FsyncFile: file system does not support sync, dirName=%{public}s", realPath.c_str());
        } else {
            StorageService::StorageRadar::ReportUserManager("KeyBackup::FsyncFile", 0, ret, "dirName=" + dirName);
            LOGE("[L4:KeyBackup] FsyncFile: <<< EXIT FAILED <<< sync failed, dirName=%{public}s", realPath.c_str());
        }
    }
    LOGI("[L4:KeyBackup] FsyncFile: <<< EXIT SUCCESS <<<");
}

int32_t KeyBackup::MkdirParentWithRetry(const std::string &pathName, mode_t mode)
{
    LOGI("[L4:KeyBackup] MkdirParentWithRetry: >>> ENTER <<< pathName=%{public}s", pathName.c_str());
    int32_t ret = MkdirParent(pathName, DEFAULT_DIR_PERM);
    if (ret == 0) {
        LOGI("[L4:KeyBackup] MkdirParentWithRetry: <<< EXIT SUCCESS <<<");
        return 0;
    }
    LOGE("[L4:KeyBackup] MkdirParentWithRetry: CreateBackup failed start retry, path=%{public}s", pathName.c_str());
    for (int i = 0; i < BACK_MAX_RETRY_TIME; ++i) {
        usleep(BACK_RETRY_INTERVAL_MS);
        ret = MkdirParent(pathName, DEFAULT_DIR_PERM);
        LOGE("[L4:KeyBackup] MkdirParentWithRetry: has retry %{public}d times, retryRet=%{public}d", i, ret);
        if (ret == 0) {
            LOGI("[L4:KeyBackup] MkdirParentWithRetry: <<< EXIT SUCCESS <<<");
            break;
        }
    }
    if (ret != 0) {
        LOGE("[L4:KeyBackup] MkdirParentWithRetry: <<< EXIT FAILED <<< pathName=%{public}s", pathName.c_str());
    }
    return ret;
}

int32_t KeyBackup::MkdirParent(const std::string &pathName, mode_t mode)
{
    LOGI("[L4:KeyBackup] MkdirParent: >>> ENTER <<< pathName=%{public}s", pathName.c_str());
    std::string::size_type pos = 0;
    pos = pathName.find("/", pos + 1);
    while (pos != std::string::npos) {
        std::string dirName = pathName.substr(0, pos);
        if (access(dirName.c_str(), F_OK) != 0) {
            if (mkdir(dirName.c_str(), mode) < 0) {
                LOGE("[L4:KeyBackup] MkdirParent: <<< EXIT FAILED <<< mkdir dir failed, dirName=%{public}s",
                     dirName.c_str());
                return -1;
            }
        }
        pos = pathName.find("/", pos + 1);
    }

    LOGI("[L4:KeyBackup] MkdirParent: <<< EXIT SUCCESS <<<");
    return 0;
}

void KeyBackup::CleanFile(const std::string &path)
{
    LOGD("[L4:KeyBackup] CleanFile: >>> ENTER <<< path=%{public}s", path.c_str());
    std::string realPath;
    if (!GetRealPath(path, realPath)) {
        LOGI("[L4:KeyBackup] CleanFile: <<< EXIT FAILED <<< GetRealPath failed");
        return;
    }
    FILE *f = fopen(realPath.c_str(), "w");
    if (f == nullptr) {
        LOGE("[L4:KeyBackup] CleanFile: <<< EXIT FAILED <<< open failed, path=%{public}s", realPath.c_str());
        return;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("[L4:KeyBackup] CleanFile: <<< EXIT FAILED <<< fd failed, path=%{public}s", realPath.c_str());
        (void)fclose(f);
        return;
    }

    int len = lseek(fd, 0, SEEK_END);
    std::string data(len, '\0');

    lseek(fd, 0, SEEK_SET);
    if (write(fd, data.c_str(), data.size()) < 0) {
        LOGE("[L4:KeyBackup] CleanFile: <<< EXIT FAILED <<< failed to write file, path=%{public}s", realPath.c_str());
    }
    if (fsync(fd) == -1) {
        LOGE("[L4:KeyBackup] CleanFile: <<< EXIT FAILED <<< failed to sync file, path=%{public}s", realPath.c_str());
    }
    (void)fclose(f);
    LOGI("[L4:KeyBackup] CleanFile: <<< EXIT SUCCESS <<<");
}

void KeyBackup::CheckAndCopyFiles(const std::string &from, const std::string &to)
{
    LOGD("[L4:KeyBackup] CheckAndCopyFiles: >>> ENTER <<< from=%{public}s, to=%{public}s", from.c_str(), to.c_str());
    struct stat st;
    if (lstat(from.c_str(), &st) < 0) {
        LOGE("[L4:KeyBackup] CheckAndCopyFiles: <<< EXIT FAILED <<< lstat file failed, from=%{public}s", from.c_str());
        return;
    }

    if (S_ISREG(st.st_mode)) {
        CheckAndCopyOneFile(from, to);
        LOGD("[L4:KeyBackup] CheckAndCopyFiles: <<< EXIT SUCCESS <<<");
        return;
    } else if (!S_ISDIR(st.st_mode)) {
        LOGE("[L4:KeyBackup] CheckAndCopyFiles: <<< EXIT FAILED <<< file is not reg file or dir, from=%{public}s",
             from.c_str());
        return;
    }

    int32_t ret = HandleCopyDir(from, to);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] CheckAndCopyFiles: failed start retry, to=%{public}s", to.c_str());
        for (int i = 0; i < BACK_MAX_RETRY_TIME; ++i) {
            usleep(BACK_RETRY_INTERVAL_MS);
            ret = HandleCopyDir(from, to);
            LOGE("[L4:KeyBackup] CheckAndCopyFiles: has retry %{public}d times, retryRet=%{public}d", i, ret);
            if (ret == 0) {
                break;
            }
        }
    }
    DIR *dir = opendir(from.c_str());
    if (dir == nullptr) {
        LOGE("[L4:KeyBackup] CheckAndCopyFiles: <<< EXIT FAILED <<< open dir failed, from=%{public}s", from.c_str());
        return;
    }

    struct dirent *de = nullptr;
    while ((de = readdir(dir)) != nullptr) {
        if (strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0) {
            continue;
        }
        std::string dfrom = from + "/" + de->d_name;
        std::string dto = to + "/" + de->d_name;
        CheckAndCopyFiles(dfrom, dto);
    }

    if (closedir(dir) < 0) {
        LOGE("[L4:KeyBackup] CheckAndCopyFiles: close dir failed, from=%{public}s", from.c_str());
    }
    LOGI("[L4:KeyBackup] CheckAndCopyFiles: <<< EXIT SUCCESS <<<");
}

int32_t KeyBackup::HandleCopyDir(const std::string &from, const std::string &to)
{
    struct FileAttr attr;
    int32_t ret = mkdir(to.c_str(), DEFAULT_DIR_PERM);
    if (ret && errno != EEXIST) {
        LOGE("[L4:KeyBackup] HandleCopyDir: <<< EXIT FAILED <<< mkdir dir failed, to=%{public}s", to.c_str());
        return -1;
    }

    if (GetAttr(from, attr) == 0) {
        SetAttr(to, attr);
    }
    return 0;
}

int32_t KeyBackup::CheckAndCopyOneFile(const std::string &srcFile, const std::string &dstFile)
{
    LOGI("[L4:KeyBackup] CheckAndCopyOneFile: >>> ENTER <<< srcFile=%{public}s, dstFile=%{public}s",
         srcFile.c_str(), dstFile.c_str());
    std::string srcData;
    if (!ReadFileToString(srcFile, srcData)) {
        LOGE("[L4:KeyBackup] CheckAndCopyOneFile: <<< EXIT FAILED <<< failed to read srcFile=%{public}s",
             srcFile.c_str());
        return -1;
    }

    std::string dstData;
    if (!ReadFileToString(dstFile, dstData)) {
        LOGE("[L4:KeyBackup] CheckAndCopyOneFile: failed to read dstFile=%{public}s", dstFile.c_str());
    }

    if (srcData.compare(dstData) == 0) {
        LOGI("[L4:KeyBackup] CheckAndCopyOneFile: <<< EXIT SUCCESS <<< files are same");
        return 0;
    }

    if (!WriteStringToFile(srcData, dstFile)) {
        LOGE("[L4:KeyBackup] CheckAndCopyOneFile: <<< EXIT FAILED <<< failed to write dstFile=%{public}s",
             dstFile.c_str());
        return -1;
    }

    struct FileAttr attr;
    if (GetAttr(srcFile, attr) == 0) {
        SetAttr(dstFile, attr);
    }
    FsyncFile(dstFile);
    LOGW("[L4:KeyBackup] CheckAndCopyOneFile: copy srcFile=%{public}s dstFile=%{public}s succ",
         srcFile.c_str(), dstFile.c_str());
    LOGI("[L4:KeyBackup] CheckAndCopyOneFile: <<< EXIT SUCCESS <<<");
    return 0;
}

bool KeyBackup::ReadFileToString(const std::string &filePath, std::string &content)
{
    std::string realPath;
    if (!GetRealPath(filePath, realPath)) {
        return false;
    }
    FILE *f = fopen(realPath.c_str(), "r");
    if (f == nullptr) {
        LOGE("[L4:KeyBackup] ReadFileToString: %{public}s realpath failed", realPath.c_str());
        return false;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("[L4:KeyBackup] ReadFileToString: %{public}s realpath failed", realPath.c_str());
        (void)fclose(f);
        return false;
    }
    struct stat sb {};
    if (fstat(fd, &sb) != -1 && sb.st_size > 0) {
        content.resize(sb.st_size);
    }

    ssize_t remaining = sb.st_size;
    bool readStatus = true;
    char* p = const_cast<char*>(content.data());

    while (remaining > 0) {
        ssize_t n = read(fd, p, remaining);
        if (n < 0) {
            readStatus = false;
            break;
        }
        p += n;
        remaining -= n;
    }
    (void)fclose(f);
    return readStatus;
}

bool KeyBackup::GetRealPath(const std::string &path, std::string &realPath)
{
    char resolvedPath[PATH_MAX] = { 0 };
    if (path.size() >= PATH_MAX || !realpath(path.c_str(), resolvedPath)) {
        LOGE("[L4:KeyBackup] GetRealPath: %{public}s realpath failed", path.c_str());
        return false;
    }
    realPath = std::string(resolvedPath);
    return true;
}

bool KeyBackup::WriteStringToFd(int fd, const std::string &content)
{
    const char *p = content.data();
    size_t remaining = content.size();
    while (remaining > 0) {
        ssize_t n = write(fd, p, remaining);
        if (n == -1) {
            return false;
        }
        p += n;
        remaining -= n;
    }
    return true;
}

bool KeyBackup::WriteStringToFile(const std::string &payload, const std::string &fileName)
{
    UniqueFd fd(open(fileName.c_str(), O_WRONLY | O_CREAT | O_NOFOLLOW | O_TRUNC | O_CLOEXEC, DEFAULT_WRITE_FILE_PERM));
    if (fd < 0) {
        LOGE("[L4:KeyBackup] WriteStringToFile: <<< EXIT FAILED <<< open file failed, fileName=%{public}s",
             fileName.c_str());
        return false;
    }
    if (!WriteStringToFd(fd, payload)) {
        LOGE("[L4:KeyBackup] WriteStringToFile: <<< EXIT FAILED <<< failed to write file, fileName=%{public}s",
             fileName.c_str());
        unlink(fileName.c_str());
        return false;
    }

    auto ret = fsync(fd);
    if (ret == -1) {
        if (errno == EROFS || errno == EINVAL) {
            LOGE("[L4:KeyBackup] WriteStringToFile: file system does not support sync, fileName=%{public}s",
                 fileName.c_str());
        } else {
            std::string extraData = "fileName=" + fileName;
            StorageService::StorageRadar::ReportUserManager("KeyBackup::WriteStringToFile::fsync", 0, ret, extraData);
            LOGE("[L4:KeyBackup] WriteStringToFile: <<< EXIT FAILED <<< sync failed, fileName=%{public}s",
                 fileName.c_str());
            unlink(fileName.c_str());
            return false;
        }
    }
    LOGI("[L4:KeyBackup] WriteStringToFile: <<< EXIT SUCCESS <<< fileName=%{public}s", fileName.c_str());
    return true;
}

int32_t KeyBackup::CompareFile(const std::string &fileA, const std::string fileB)
{
    std::string dataA;
    if (!ReadFileToString(fileA, dataA)) {
        LOGE("[L4:KeyBackup] CompareFile: <<< EXIT FAILED <<< failed to read from fileA=%{public}s", fileA.c_str());
        return -1;
    }

    std::string dataB;
    if (!ReadFileToString(fileB, dataB)) {
        LOGE("[L4:KeyBackup] CompareFile: <<< EXIT FAILED <<< failed to read from fileB=%{public}s", fileB.c_str());
        return -1;
    }

    int ret = dataA.compare(dataB);
    LOGI("[L4:KeyBackup] CompareFile: <<< EXIT %s <<<", ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

int32_t KeyBackup::GetAttr(const std::string &path, struct FileAttr &attr)
{
    struct stat st;
    if (lstat(path.c_str(), &st) < 0) {
        return (errno == ENOENT) ? 0 : -1;
    }

    attr.uid = st.st_uid;
    attr.gid = st.st_gid;
    attr.mode = st.st_mode;
    return 0;
}

int32_t KeyBackup::SetAttr(const std::string &path, struct FileAttr &attr)
{
    int32_t ret = lchown(path.c_str(), attr.uid, attr.gid);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] SetAttr: lchown failed, path=%{public}s, uid=%{public}d, gid=%{public}d",
             path.c_str(), attr.uid, attr.gid);
        return ret;
    }

    ret = chmod(path.c_str(), attr.mode);
    if (ret != 0) {
        LOGE("[L4:KeyBackup] SetAttr: chmod failed, path=%{public}s, mode=%{public}d", path.c_str(), attr.mode);
        return ret;
    }

    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS