/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <unistd.h>

#include "securec.h"
#include "storage_service_log.h"
#include "unique_fd.h"

namespace OHOS {
namespace StorageDaemon {
const uint32_t INVALID_LOOP_NUM = 0xFFFFFFFF;

struct FileNode {
    std::string baseName;
    std::string origFile;
    std::string backFile;
    bool isSame;
};

void KeyBackup::CreateBackup(const std::string &from, const std::string &to, bool removeOld)
{
    LOGI("create backup from: %s to: %s removeOld: %d", from.c_str(), to.c_str(), removeOld ? 1 : 0);
    if (access(from.c_str(), 0) != 0) {
        LOGE("from path s not exist, path is %s", from.c_str());
        return;
    }

    if (access(to.c_str(), 0) == 0) {
        if (removeOld) {
            if (RemoveNode(to) != 0) {
                LOGE("faled to remove to: %s", to.c_str());
            }
        }
    } else {
        if (MkdirParent(to, DEFAULT_DIR_PERM) != 0) {
            return;
        }
    }

    CheckAndCopyFiles(from, to);
    FsyncDirectory(to);
}

int32_t KeyBackup::RemoveNode(const std::string &pathName)
{
    LOGI("remove node pathName %s", pathName.c_str());
    struct stat st;
    if (lstat(pathName.c_str(), &st) < 0) {
        return (errno == ENOENT) ? 0 : -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        CleanFile(pathName);
        return remove(pathName.c_str());
    }

    DIR *dir = opendir(pathName.c_str());
    if (dir == nullptr) {
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
        return -1;
    }

    if (closedir(dir) < 0) {
        return -1;
    }
    return rmdir(pathName.c_str());
}

int32_t KeyBackup::TryRestoreKey(const std::shared_ptr<BaseKey> &baseKey, const UserAuth &auth)
{
    if (baseKey == nullptr) {
        LOGE("basekey is nullptr");
        return -1;
    }
    std::string keyDir = baseKey->GetDir();
    std::string backupDir;
    GetBackupDir(keyDir, backupDir);
    if (baseKey->DoRestoreKeyEx(auth, keyDir + PATH_LATEST)) {
        CheckAndFixFiles(keyDir, backupDir);
        LOGI("Restore by main key success !");
        return 0;
    }
    LOGE("origKey failed, try backupKey");
    if (baseKey->DoRestoreKeyEx(auth, backupDir + PATH_LATEST)) {
        CheckAndFixFiles(backupDir, keyDir);
        LOGI("Restore by back key success !");
        return 0;
    }

    LOGE("origKey failed, backupKey failed, so mix key");
    return -1;
}

int32_t KeyBackup::TryRestoreUeceKey(const std::shared_ptr<BaseKey> &baseKey,
                                     const UserAuth &auth,
                                     KeyBlob &planKey,
                                     KeyBlob &decryptedKey)
{
    if (baseKey == nullptr) {
        LOGE("basekey is nullptr");
        return -1;
    }
    std::string keyDir = baseKey->GetDir();
    std::string backupDir;
    GetBackupDir(keyDir, backupDir);
    if (baseKey->DecryptKeyBlob(auth, keyDir + PATH_LATEST, planKey, decryptedKey)) {
        CheckAndFixFiles(keyDir, backupDir);
        LOGI("Restore uece by main key success !");
        return 0;
    }
    LOGE("origKey failed, try backupKey");
    if (baseKey->DecryptKeyBlob(auth, backupDir + PATH_LATEST, planKey, decryptedKey)) {
        CheckAndFixFiles(backupDir, keyDir);
        LOGI("Restore uece by back key success !");
        return 0;
    }

    LOGE("origKey failed, backupKey failed, so mix key");
    return -1;
}

int32_t KeyBackup::GetBackupDir(std::string &origDir, std::string &backupDir)
{
    LOGI("get backup dir origDir %s", origDir.c_str());
    if (origDir == DEVICE_EL1_DIR) {
        backupDir = DEVICE_EL1_DIR + BACKUP_NAME;
        LOGI("backup dir is: %s", backupDir.c_str());
        return 0;
    }

    auto slashIndex = origDir.rfind("/");
    if (slashIndex == std::string::npos || slashIndex == 0) {
        return -1;
    }
    std::string prefixStr = origDir.substr(0, slashIndex);
    std::string endStr = origDir.substr(slashIndex);
    backupDir = prefixStr + BACKUP_NAME + endStr;
    LOGI("backup dir is: %s", backupDir.c_str());
    return 0;
}

void KeyBackup::ListAndCheckDir(std::string &origDir)
{
    LOGI("list and check dir %s", origDir.c_str());
    if (access(origDir.c_str(), F_OK) == 0) {
        return;
    }
    LOGW("list and check dir origDir is not exist %s", origDir.c_str());
    std::string backupDir;
    int32_t ret = GetBackupDir(origDir, backupDir);
    if (ret != 0) {
        LOGE("list and check dir failed %s", origDir.c_str());
        return;
    }
    if (access(backupDir.c_str(), F_OK) == 0) {
        LOGW("list and check dir origDir: %s backupDir: %s", origDir.c_str(), backupDir.c_str());
        ret = MkdirParent(origDir, DEFAULT_DIR_PERM);
        if (ret != 0) {
            return;
        }
        CheckAndCopyFiles(backupDir, origDir);
    }
    return;
}

int32_t KeyBackup::DoResotreKeyMix(std::shared_ptr<BaseKey> &baseKey, const UserAuth &auth, const std::string &keyDir,
    const std::string &backupDir)
{
    std::string origKeyDir = keyDir + PATH_LATEST;
    std::string backupKeyDir = backupDir + PATH_LATEST;
    std::vector<struct FileNode> fileList;
    uint32_t diffNum = 0;
    int32_t ret = GetFileList(origKeyDir, backupKeyDir, fileList, diffNum);
    if (ret != 0 || diffNum <= 1) {
        LOGE("get file list failed or diffNum too least, ret: %d, diffNum: %d", ret, diffNum);
        return -1;
    }

    std::string tempKeyDir;
    ret = CopySameFilesToTempDir(backupKeyDir, tempKeyDir, fileList);
    if (ret != 0) {
        return false;
    }

    diffNum = fileList.size();
    uint32_t loopNum = GetLoopMaxNum(diffNum);
    if (loopNum == INVALID_LOOP_NUM) {
        RemoveNode(tempKeyDir);
        return -1;
    }
    for (uint32_t i = 0; i <= loopNum; i++) {
        LOGI("try mix key files to decrypt i: %d loopNum: %d", i, loopNum);
        ret = CopyMixFilesToTempDir(diffNum, i, tempKeyDir, fileList);
        if (ret != 0) {
            LOGE("copy mix files to temp dir failed");
            continue;
        }
        if (baseKey->DoRestoreKeyEx(auth, tempKeyDir)) {
            LOGI("mix key files descrpt succ, fix orig and backup");
            CheckAndFixFiles(tempKeyDir, origKeyDir);
            CheckAndFixFiles(tempKeyDir, backupKeyDir);
            RemoveNode(tempKeyDir);
            return true;
        }
    }
    RemoveNode(tempKeyDir);
    return -1;
}

int32_t KeyBackup::GetFileList(const std::string &origDir, const std::string &backDir,
    std::vector<struct FileNode> &fileList, uint32_t diffNum)
{
    LOGI("get file list origDir: %s backDir: %s", origDir.c_str(), backDir.c_str());
    DIR *dir = opendir(origDir.c_str());
    if (dir == nullptr) {
        LOGE("fail to open %s", origDir.c_str());
        return -1;
    }
    struct dirent *de = nullptr;
    while ((de = readdir(dir)) != nullptr) {
        AddOrigFileToList(std::string(de->d_name), origDir, fileList);
    }
    closedir(dir);
    dir = nullptr;

    dir = opendir(backDir.c_str());
    if (dir == nullptr) {
        LOGE("fail to open %s", backDir.c_str());
        return -1;
    }
    while ((de = readdir(dir)) != nullptr) {
        AddBackupFileToList(std::string(de->d_name), backDir, fileList);
    }
    closedir(dir);
    dir = nullptr;

    diffNum = GetDiffFilesNum(fileList);
    LOGI("get file list origDir: %s backDir: %s diffNum: %d", origDir.c_str(), backDir.c_str(), diffNum);
    return 0;
}

bool KeyBackup::IsRegFile(const std::string &filePath)
{
    struct stat st;
    if (lstat(filePath.c_str(), &st) < 0) {
        LOGE("lstat failed %s", filePath.c_str());
        return false;
    }

    if (!S_ISREG(st.st_mode)) {
        LOGE("filePath is not reg file %s", filePath.c_str());
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
    return;
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
    return;
}

uint32_t KeyBackup::GetDiffFilesNum(const std::vector<struct FileNode> &fileList)
{
    uint32_t diffNum = 0;
    for (auto iter = fileList.begin(); iter != fileList.end(); ++iter) {
        LOGI("fileList contain origFile: %s, backupFile: %s, isSame: %d, fileName: %s", iter->origFile.c_str(),
            iter->backFile.c_str(), iter->isSame ? 0 : 1, iter->baseName.c_str());
        if (!iter->isSame) {
            diffNum++;
        }
    }
    LOGI("diff files num %d", diffNum);
    return diffNum;
}

int32_t KeyBackup::CopySameFilesToTempDir(const std::string &backupDir, std::string &tempDir,
    std::vector<struct FileNode> &fileList)
{
    LOGI("copy same files to temp dir, backupDir: %s tempDir: %s", backupDir.c_str(), tempDir.c_str());
    int32_t ret = CreateTempDirForMixFiles(backupDir, tempDir);
    if (ret != 0) {
        return -1;
    }

    for (auto iter = fileList.begin(); iter != fileList.end();) {
        if (iter->isSame || iter->backFile.empty()) {
            ret = CheckAndCopyOneFile(iter->origFile, tempDir + "/" + iter->baseName);
            if (ret != 0) {
                RemoveNode(tempDir);
                return -1;
            }
            iter = fileList.erase(iter);
        } else if (iter->origFile.empty()) {
            ret = CheckAndCopyOneFile(iter->backFile, tempDir + "/" + iter->baseName);
            if (ret != 0) {
                RemoveNode(tempDir);
                return -1;
            }
            iter = fileList.erase(iter);
        } else {
            ++iter;
        }
    }
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
        LOGE("create temp dir for mix files failed, ret: %d, tempDir: %s", ret, tempDir.c_str());
    } else {
        LOGI("create temp dir for mix files success, tempDir: %s", tempDir.c_str());
    }
    return ret;
}

uint32_t KeyBackup::GetLoopMaxNum(uint32_t diffNum)
{
    if (diffNum > MAX_FILE_NUM) {
        LOGE("there are too many different files, diffNum: %d", diffNum);
        return INVALID_LOOP_NUM;
    }

    const double fileNum = 2;
    return static_cast<uint32_t>(pow(fileNum, diffNum) - 1);
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
    LOGI("check fix files from: %s to: %s", from.c_str(), to.c_str());
    CreateBackup(from, to, false);
    FsyncDirectory(to);
}

void KeyBackup::FsyncDirectory(const std::string &dirName)
{
    LOGI("sync directory dirName %s", dirName.c_str());
    std::string realPath;
    if (!GetRealPath(dirName, realPath)) {
        return;
    }

    UniqueFd fd(open(realPath.c_str(), O_RDONLY | O_CLOEXEC));
    if (fd < 0) {
        LOGE("failed to open %s", realPath.c_str());
        return;
    }
    if (fsync(fd) == -1) {
        if (errno == EROFS || errno == EINVAL) {
            LOGE("file system does not support sync, dirName: %s", realPath.c_str());
        } else {
            LOGE("sync failed: %s", realPath.c_str());
        }
    }
    return;
}
    
int32_t KeyBackup::MkdirParent(const std::string &pathName, mode_t mode)
{
    LOGI("mkdir parent dirName %s", pathName.c_str());
    std::string::size_type pos = 0;
    pos = pathName.find("/", pos + 1);
    while (pos != std::string::npos) {
        std::string dirName = pathName.substr(0, pos);
        if (access(dirName.c_str(), F_OK) != 0) {
            if (mkdir(dirName.c_str(), mode) < 0) {
                LOGE("mkdr dir %s failed", dirName.c_str());
                return -1;
            }
        }
        pos = pathName.find("/", pos + 1);
    }

    return 0;
}

void KeyBackup::CleanFile(const std::string &path)
{
    LOGI("clean file path %s", path.c_str());
    std::string realPath;
    if (!GetRealPath(path, realPath)) {
        return;
    }
    int fd = open(realPath.c_str(), O_WRONLY);
    if (fd < 0) {
        LOGE("open %s failed", realPath.c_str());
        return;
    }

    int len = lseek(fd, 0, SEEK_END);
    std::string data(len, '\0');

    lseek(fd, 0, SEEK_SET);
    if (write(fd, data.c_str(), data.size()) < 0) {
        LOGE("failed to write file %s", realPath.c_str());
    }
    if (fsync(fd) == -1) {
        LOGE("failed to sync file %s", realPath.c_str());
    }
    close(fd);
    return;
}

void KeyBackup::CheckAndCopyFiles(const std::string &from, const std::string &to)
{
    LOGI("check and copy files from: %s to: %s", from.c_str(), to.c_str());
    struct stat st;
    if (lstat(from.c_str(), &st) < 0) {
        LOGE("lstat file failed, %s", from.c_str());
        return;
    }

    if (S_ISREG(st.st_mode)) {
        CheckAndCopyOneFile(from, to);
        return;
    } else if (!S_ISDIR(st.st_mode)) {
        LOGE("file: %s is not reg file or dir, skip it", from.c_str());
        return;
    }

    HandleCopyDir(from, to);
    DIR *dir = opendir(from.c_str());
    if (dir == nullptr) {
        LOGE("open dr failed, %s", from.c_str());
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
        LOGE("close dir failed, %s", from.c_str());
    }
}

int32_t KeyBackup::HandleCopyDir(const std::string &from, const std::string &to)
{
    struct FileAttr attr;
    int32_t ret = mkdir(to.c_str(), DEFAULT_DIR_PERM);
    if (ret && errno != EEXIST) {
        LOGE("mkdir dir %s failed", to.c_str());
        return -1;
    }

    if (GetAttr(from, attr) == 0) {
        SetAttr(to, attr);
    }
    return 0;
}

int32_t KeyBackup::CheckAndCopyOneFile(const std::string &from, const std::string &to)
{
    LOGI("check and copy one file from: %s to: %s", from.c_str(), to.c_str());
    if (CompareFile(from, to) == 0) {
        return 0;
    }

    if (CopyRegfileData(from, to) != 0) {
        LOGE("copy from: %s to file: %s failed", from.c_str(), to.c_str());
        return -1;
    }

    struct FileAttr attr;
    if (GetAttr(from, attr) == 0) {
        SetAttr(to, attr);
    }
    LOGI("copy from: %s to file %s succ", from.c_str(), to.c_str());
    return 0;
}

bool KeyBackup::ReadFileToString(const std::string &filePath, std::string &content)
{
    std::string realPath;
    if (!GetRealPath(filePath, realPath)) {
        return false;
    }
    int fd = open(realPath.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("%s realpath failed", realPath.c_str());
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
    close(fd);
    return readStatus;
}

bool KeyBackup::GetRealPath(const std::string &path, std::string &realPath)
{
    char resolvedPath[PATH_MAX] = { 0 };
    if (path.size() > PATH_MAX || !realpath(path.c_str(), resolvedPath)) {
        LOGE("%s realpath failed", path.c_str());
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
        LOGE("open file failed, %s", fileName.c_str());
        return false;
    }
    if (!WriteStringToFd(fd, payload)) {
        LOGE("failed to write file, %s", fileName.c_str());
        unlink(fileName.c_str());
        return false;
    }

    if (fsync(fd) == -1) {
        if (errno == EROFS || errno == EINVAL) {
            LOGE("file system does not support sync, fileName: %s", fileName.c_str());
        } else {
            LOGE("sync failed: %s", fileName.c_str());
            unlink(fileName.c_str());
            return false;
        }
    }
    return true;
}

int32_t KeyBackup::CompareFile(const std::string &fileA, const std::string fileB)
{
    std::string dataA;
    if (!ReadFileToString(fileA, dataA)) {
        LOGE("failed to read from %s", fileA.c_str());
        return -1;
    }

    std::string dataB;
    if (!ReadFileToString(fileB, dataB)) {
        LOGE("failed to read from %s", fileB.c_str());
        return -1;
    }

    return dataA.compare(dataB);
}

int32_t KeyBackup::CopyRegfileData(const std::string &from, const std::string &to)
{
    std::string data;
    if (!ReadFileToString(from, data)) {
        LOGE("failed to read from: %s", from.c_str());
        return -1;
    }

    if (!WriteStringToFile(data, to)) {
        LOGE("failed to write file: %s", to.c_str());
        return -1;
    }
    return 0;
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
        LOGE("lchown failed path: %s, uid: %d, gid: %d", path.c_str(), attr.uid, attr.gid);
        return ret;
    }

    ret = chmod(path.c_str(), attr.mode);
    if (ret != 0) {
        LOGE("chmod failed, path: %s, mode: %d", path.c_str(), attr.mode);
        return ret;
    }

    return ret;
}
} // namespace StorageDaemon
} // namespace HOHS