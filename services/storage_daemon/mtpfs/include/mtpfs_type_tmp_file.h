/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MTPFS_TYPE_TMP_FILE_H
#define MTPFS_TYPE_TMP_FILE_H

#include <set>
#include <string>
#include "mtpfs_type_file.h"

class MtpFsTypeTmpFile {
public:
    MtpFsTypeTmpFile();
    MtpFsTypeTmpFile(const MtpFsTypeTmpFile &copy);
    MtpFsTypeTmpFile(const std::string &pathDevice, const std::string &pathTmp, int fileDesc, bool modified = false);

    std::string PathDevice() const
    {
        return pathDevice_;
    }
    std::string PathTmp() const
    {
        return pathTmp_;
    }

    bool IsModified() const
    {
        return modified_;
    }
    void SetModified(bool modified = true)
    {
        modified_ = modified;
    }

    std::set<int> FileDescriptors() const
    {
        return fileDescriptors_;
    }
    void AddFileDescriptor(int fd)
    {
        fileDescriptors_.insert(fd);
    }
    bool HasFileDescriptor(int fd);
    void RemoveFileDescriptor(int fd);
    int RefCnt() const
    {
        return fileDescriptors_.size();
    }

    MtpFsTypeTmpFile &operator = (const MtpFsTypeTmpFile &rhs);

    bool operator == (const MtpFsTypeTmpFile &rhs) const
    {
        return pathDevice_ == rhs.pathDevice_;
    }
    bool operator == (const std::string &path) const
    {
        return pathDevice_ == path;
    }
    bool operator < (const MtpFsTypeTmpFile &rhs) const
    {
        return pathDevice_ < rhs.pathDevice_;
    }
    bool operator < (const std::string &path) const
    {
        return pathDevice_ < path;
    }

private:
    std::string pathDevice_;
    std::string pathTmp_;
    std::set<int> fileDescriptors_;
    bool modified_;
};

#endif // MTPFS_TYPE_TMP_FILE_H
