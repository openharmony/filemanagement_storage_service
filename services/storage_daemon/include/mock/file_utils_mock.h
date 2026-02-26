/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_UTILS_FILE_UTILS_MOCK_H
#define STORAGE_DAEMON_UTILS_FILE_UTILS_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
class IFileUtilMoc {
public:
    virtual ~IFileUtilMoc() = default;
public:
    virtual int32_t ChMod(const std::string &path, mode_t mode) = 0;
    virtual int32_t MkDir(const std::string &path, mode_t mode) = 0;
    virtual bool IsDir(const std::string &path) = 0;
    virtual bool IsFile(const std::string &path) = 0;
    virtual bool PrepareDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid) = 0;
    virtual bool MkDirRecurse(const std::string& path, mode_t mode) = 0;
    virtual bool RmDirRecurse(const std::string &path);
    virtual int32_t Mount(const std::string &source, const std::string &target, const char *type,
        unsigned long flags, const void *data) = 0;
    virtual int32_t UMount(const std::string &path) = 0;
    virtual int32_t UMount2(const std::string &path, int flag) = 0;
    virtual bool StringToUint32(const std::string &str, uint32_t &num) = 0;
    virtual bool ReadFile(const std::string &path, std::string *str) = 0;
    virtual int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output = nullptr,
        int *exitStatus = nullptr) = 0;
    virtual int IsSameGidUid(const std::string &dir, uid_t uid, gid_t gid) = 0;
    virtual bool IsTempFolder(const std::string &path, const std::string &sub) = 0;
    virtual void DeleteFile(const std::string &path) = 0;
    virtual std::vector<std::string> Split(std::string str, const std::string &pattern) = 0;
    virtual bool IsPathMounted(std::string &path) = 0;
    virtual bool DelFolder(const std::string &path) = 0;
    virtual std::string ProcessToString(std::vector<ProcessInfo> &processList) = 0;
    virtual void GetSubDirs(const std::string &path, std::vector<std::string> &dirList) = 0;
public:
    static inline std::shared_ptr<IFileUtilMoc> fileUtilMoc = nullptr;
};

class FileUtilMoc : public IFileUtilMoc {
public:
    MOCK_METHOD2(ChMod, int32_t(const std::string &path, mode_t mode));
    MOCK_METHOD2(MkDir, int32_t(const std::string &path, mode_t mode));
    MOCK_METHOD1(IsDir, bool(const std::string &path));
    MOCK_METHOD1(IsFile, bool(const std::string &path));
    MOCK_METHOD4(PrepareDir, bool(const std::string &path, mode_t mode, uid_t uid, gid_t gid));
    MOCK_METHOD2(MkDirRecurse, bool(const std::string& path, mode_t mode));
    MOCK_METHOD1(RmDirRecurse, bool(const std::string &path));
    MOCK_METHOD5(Mount, int32_t(const std::string &source, const std::string &target, const char *type,
        unsigned long flags, const void *data));
    MOCK_METHOD1(UMount, int32_t(const std::string &path));
    MOCK_METHOD2(UMount2, int32_t(const std::string &path, int flag));
    MOCK_METHOD2(StringToUint32, bool(const std::string &str, uint32_t &num));
    MOCK_METHOD2(ReadFile, bool(const std::string &path, std::string *str));
    MOCK_METHOD3(ForkExec, int(std::vector<std::string> &cmd, std::vector<std::string> *output, int *exitStatus));
    MOCK_METHOD3(IsSameGidUid, int(const std::string &dir, uid_t uid, gid_t gid));
    MOCK_METHOD2(IsTempFolder, bool(const std::string &path, const std::string &sub));
    MOCK_METHOD1(DeleteFile, void(const std::string &path));
    MOCK_METHOD2(Split, std::vector<std::string>(std::string str, const std::string &pattern));
    MOCK_METHOD1(IsPathMounted, bool(std::string &path));
    MOCK_METHOD0(IsUsbFuse, bool());
    MOCK_METHOD1(DelFolder, bool(const std::string &path));
    MOCK_METHOD1(ProcessToString, std::string(std::vector<ProcessInfo> &processList));
    MOCK_METHOD2(GetSubDirs, void(const std::string &path, std::vector<std::string> &dirList));
};
}
}
#endif