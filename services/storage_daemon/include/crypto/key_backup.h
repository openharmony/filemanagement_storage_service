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

#ifndef STORAGE_DAEMON_KEY_BACKUP_H
#define STORAGE_DAEMON_KEY_BACKUP_H

#include <sys/stat.h>

#include "base_key.h"

namespace OHOS {
namespace StorageDaemon {
struct FileAttr {
    uid_t uid;
    gid_t gid;
    mode_t mode;
};

class KeyBackup {
public:
    static KeyBackup &GetInstance()
    {
        static KeyBackup instance;
        return instance;
    }

    void CreateBackup(const std::string &from, const std::string &to, bool removeOld = true);
    int32_t RemoveNode(const std::string &pathName);
    int32_t TryRestoreKey(const std::shared_ptr<BaseKey> &baseKey, const UserAuth &auth);
    int32_t TryRestoreUeceKey(const std::shared_ptr<BaseKey> &baseKey,
                              const UserAuth &auth,
                              KeyBlob &planKey,
                              KeyBlob &decryptedKey);
    int32_t GetBackupDir(std::string &origDir, std::string &backupDir);
    void ListAndCheckDir(std::string &origDir);

private:
    KeyBackup() {};
    ~KeyBackup() {};
    KeyBackup(const KeyBackup &) = delete;
    KeyBackup &operator=(const KeyBackup &) = delete;

    void FsyncFile(const std::string &dirName);
    int32_t MkdirParent(const std::string &pathName, mode_t mode);
    int32_t MkdirParentWithRetry(const std::string &pathName, mode_t mode);
    void CleanFile(const std::string &path);
    void CheckAndCopyFiles(const std::string &from, const std::string &to);
    int32_t CheckAndCopyOneFile(const std::string &from, const std::string &to);
    bool ReadFileToString(const std::string &filePath, std::string &content);
    bool GetRealPath(const std::string &path, std::string &realPath);
    bool WriteStringToFd(int fd, const std::string &content);
    bool WriteStringToFile(const std::string &payload, const std::string &fileName);
    int32_t CompareFile(const std::string &fileA, const std::string fileB);
    int32_t GetAttr(const std::string &path, struct FileAttr &attr);
    int32_t SetAttr(const std::string &path, struct FileAttr &attr);
    int32_t HandleCopyDir(const std::string &from, const std::string &to);
    void CheckAndFixFiles(const std::string &from, const std::string &to);
    int32_t GetFileList(const std::string &origDir, const std::string &backDir,
        std::vector<struct FileNode> &fileListm, uint32_t diffNum);
    void AddOrigFileToList(const std::string &fileName, const std::string &origDir,
        std::vector<struct FileNode> &fileList);
    void AddBackupFileToList(const std::string &fileName, const std::string &backDir,
        std::vector<struct FileNode> &fileList);
    uint32_t GetDiffFilesNum(const std::vector<struct FileNode> &fileList);
    int32_t CopySameFilesToTempDir(const std::string &backupDir, std::string &tempDir,
        std::vector<struct FileNode> &fileList);
    int32_t CreateTempDirForMixFiles(const std::string &backupDir, std::string &tempDir);
    uint32_t GetLoopMaxNum(uint32_t diffNum);
    int32_t CopyMixFilesToTempDir(uint32_t diffNum, uint32_t num, const std::string &tempDir,
        const std::vector<struct FileNode> &fileList);
    bool IsRegFile(const std::string &filePath);
    int32_t DoResotreKeyMix(std::shared_ptr<BaseKey> &baseKey, const UserAuth &auth, const std::string &keyDir,
        const std::string &backupDir);

private:
    constexpr static mode_t DEFAULT_DIR_PERM = 0700;
    constexpr static mode_t DEFAULT_WRITE_FILE_PERM = 0644;
    constexpr static uint32_t MAX_FILE_NUM = 5;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_KEY_BACKUP_H
