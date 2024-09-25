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

#ifndef MTPFS_TMP_FILES_POOL_H
#define MTPFS_TMP_FILES_POOL_H

#include "mtpfs_type_tmp_file.h"

class MtpFsTmpFilesPool {
public:
    MtpFsTmpFilesPool();
    ~MtpFsTmpFilesPool();

    void SetTmpDir(const std::string &tmpDir)
    {
        tmpDir_ = tmpDir;
    }

    void AddFile(const MtpFsTypeTmpFile &tmp)
    {
        pool_.insert(tmp);
    }
    void RemoveFile(const std::string &path);
    bool Empty() const
    {
        return pool_.size();
    }

    const MtpFsTypeTmpFile *GetFile(const std::string &path) const;

    std::string MakeTmpPath(const std::string &pathDevice) const;
    bool CreateTmpDir();
    bool RemoveTmpDir();

private:
    std::string tmpDir_;
    std::set<MtpFsTypeTmpFile> pool_;
};

#endif // MTPFS_TMP_FILES_POOL_H
