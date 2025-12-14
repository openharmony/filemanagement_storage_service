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

#include "file_utils_mock.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
int32_t ChMod(const std::string &path, mode_t mode)
{
    return IFileUtilMoc::fileUtilMoc->ChMod(path, mode);
}

int32_t MkDir(const std::string &path, mode_t mode)
{
    return IFileUtilMoc::fileUtilMoc->MkDir(path, mode);
}

int32_t Mount(const std::string &source, const std::string &target, const char *type,
              unsigned long flags, const void *data)
{
    return IFileUtilMoc::fileUtilMoc->Mount(source, target, type, flags, data);
}

int32_t UMount(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->UMount(path);
}

int32_t UMount2(const std::string &path, int flag)
{
    return IFileUtilMoc::fileUtilMoc->UMount2(path, flag);
}

bool IsDir(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->IsDir(path);
}

bool IsFile(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->IsFile(path);
}

bool MkDirRecurse(const std::string& path, mode_t mode)
{
    return IFileUtilMoc::fileUtilMoc->MkDirRecurse(path, mode);
}

bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    return IFileUtilMoc::fileUtilMoc->PrepareDir(path, mode, uid, gid);
}

bool RmDirRecurse(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->RmDirRecurse(path);
}

void TravelChmod(const std::string &path, mode_t mode)
{
    return;
}

bool StringToUint32(const std::string &str, uint32_t &num)
{
    return IFileUtilMoc::fileUtilMoc->StringToUint32(str, num);
}

void GetSubDirs(const std::string &path, std::vector<std::string> &dirList)
{
    return IFileUtilMoc::fileUtilMoc->GetSubDirs(path, dirList);
}

void ReadDigitDir(const std::string &path, std::vector<FileList> &dirInfo)
{
    return;
}

void OpenSubFile(const std::string &path, std::vector<std::string>  &file)
{
    return;
}

bool ReadFile(const std::string &path, std::string *str)
{
    return IFileUtilMoc::fileUtilMoc->ReadFile(path, str);
}

int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output, int *exitStatus)
{
    return IFileUtilMoc::fileUtilMoc->ForkExec(cmd, output, exitStatus);
}

void TraverseDirUevent(const std::string &path, bool flag)
{
    return;
}

int IsSameGidUid(const std::string &dir, uid_t uid, gid_t gid)
{
    return IFileUtilMoc::fileUtilMoc->IsSameGidUid(dir, uid, gid);
}

void MoveFileManagerData(const std::string &filesPath)
{
    return;
}

void ChownRecursion(const std::string &dir, uid_t uid, gid_t gid)
{
    return;
}

std::vector<std::string> Split(std::string str, const std::string &pattern)
{
    return IFileUtilMoc::fileUtilMoc->Split(str, pattern);
}

bool DeleteFile(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->DeleteFile(path);
}

bool IsTempFolder(const std::string &path, const std::string &sub)
{
    return IFileUtilMoc::fileUtilMoc->IsTempFolder(path, sub);
}

void DelTemp(const std::string &path)
{
    return;
}

bool IsPathMounted(std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->IsPathMounted(path);
}

bool CreateFolder(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->CreateFolder(path);
}

bool DelFolder(const std::string &path)
{
    return IFileUtilMoc::fileUtilMoc->DelFolder(path);
}

std::string ProcessToString(std::vector<ProcessInfo> &processList)
{
    return IFileUtilMoc::fileUtilMoc->ProcessToString(processList);
}

void KillProcess(const std::vector<ProcessInfo> &processList, std::vector<ProcessInfo> &killFailList)
{
    return;
}
}
}