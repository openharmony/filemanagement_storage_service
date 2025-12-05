/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_UTILS_FILE_UTILS_H
#define STORAGE_DAEMON_UTILS_FILE_UTILS_H

#include <sstream>
#include <sys/types.h>
#include <sys/mount.h>
#include <queue>
#include <map>

namespace OHOS {
namespace StorageDaemon {
struct FileList {
    uint32_t userId;
    std::string path;
};

struct ProcessInfo {
    int pid;
    std::string name;
};

int32_t ChMod(const std::string &path, mode_t mode);
int32_t MkDir(const std::string &path, mode_t mode);
bool IsDir(const std::string &path);
bool IsFile(const std::string &path);
bool IsUsbFuse();
int32_t PrepareDirSimple(const std::string &path, mode_t mode, uid_t uid, gid_t gid);
bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid);
int32_t DestroyDir(const std::string &path, bool &isPathEmpty);
bool MkDirRecurse(const std::string& path, mode_t mode);
bool RmDirRecurse(const std::string &path);
void TravelChmod(const std::string &path, mode_t mode);
int32_t Mount(const std::string &source, const std::string &target, const char *type,
              unsigned long flags, const void *data);
int32_t UMount(const std::string &path);
int32_t UMount2(const std::string &path, int flag);
void GetSubDirs(const std::string &path, std::vector<std::string> &dirList);
void ReadDigitDir(const std::string &path, std::vector<FileList> &dirInfo);
bool StringToUint32(const std::string &str, uint32_t &num);
bool StringToBool(const std::string &str, bool &result);
bool ReadFile(const std::string &path, std::string *str);
int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output = nullptr,
             int *exitStatus = nullptr);
int ForkExecWithExit(std::vector<std::string> &cmd, int *exitStatus = nullptr);
#ifdef EXTERNAL_STORAGE_QOS_TRANS
int ExtStorageMountForkExec(std::vector<std::string> &cmd, int *exitStatus = nullptr);
#endif
void TraverseDirUevent(const std::string &path, bool flag);
void ChownRecursion(const std::string &dir, uid_t uid, gid_t gid);
int IsSameGidUid(const std::string &dir, uid_t uid, gid_t gid);
void MoveFileManagerData(const std::string &filesPath);
void OpenSubFile(const std::string &path, std::vector<std::string>  &dirInfo);
void DelTemp(const std::string &path);
bool IsTempFolder(const std::string &path, const std::string &sub);
bool DeleteFile(const std::string &path);
std::vector<std::string> Split(std::string str, const std::string &pattern);
bool IsPathMounted(std::string &path);
bool CreateFolder(const std::string &path);
bool DelFolder(const std::string &path);
void KillProcess(const std::vector<ProcessInfo> &processList, std::vector<ProcessInfo> &killFailList);
bool IsProcessAlive(int pid);
std::string ProcessToString(std::vector<ProcessInfo> &processList);
bool RestoreconDir(const std::string &path);
int32_t RedirectStdToPipe(int logpipe[2], size_t len);
int32_t GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize);
int32_t GetRmgDataSize(const std::string &rgmName, const std::string &path,
    const std::vector<std::string> &ignorePaths, uint64_t &totalSize);
int32_t GetSubFilesSize(const std::string &folder, std::queue<std::string> &dirTraverseQue,
    const std::vector<std::string> &ignorePaths, uint64_t &totalSize, int &fileCount);
int32_t HandleStaticsDirError(int32_t oldErrno, int32_t newErrno);
bool IsValidRgmName(const std::string &rgmName);
bool IsValidPath(const std::string &path);
bool IsValidBusinessPath(const std::string &path, const std::string &userId = "");
int32_t StatisticsFilesTotalSize(const std::string &dirPath, const std::vector<std::string> &ignorePaths,
    uint64_t &totalSize);
bool IsBusinessPath(const std::string& path, const std::string &userId);
uint64_t GetFileSize(const std::string &filename);
bool IsFolder(const std::string &filename);
bool IsFileExist(const std::string &path);
}
}

#endif // STORAGE_DAEMON_UTILS_FILE_UTILS_H
