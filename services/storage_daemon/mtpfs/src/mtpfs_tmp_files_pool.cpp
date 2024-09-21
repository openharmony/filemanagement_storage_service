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

#include "mtpfs_tmp_files_pool.h"

#include <sstream>

#include "mtpfs_sha.h"
#include "mtpfs_util.h"

MtpFsTmpFilesPool::MtpFsTmpFilesPool() : tmpDir_(SmtpfsGetTmpDir()), pool_() {}

MtpFsTmpFilesPool::~MtpFsTmpFilesPool() {}

void MtpFsTmpFilesPool::RemoveFile(const std::string &path)
{
    auto it = std::find(pool_.begin(), pool_.end(), path);
    if (it == pool_.end()) {
        return;
    }
    pool_.erase(it);
}

const MtpFsTypeTmpFile *MtpFsTmpFilesPool::GetFile(const std::string &path) const
{
    auto it = std::find(pool_.begin(), pool_.end(), path);
    if (it == pool_.end()) {
        return nullptr;
    }
    return static_cast<const MtpFsTypeTmpFile *>(&*it);
}

std::string MtpFsTmpFilesPool::MakeTmpPath(const std::string &pathDevice) const
{
    static int cnt = 0;
    std::stringstream ss;
    ss << pathDevice << ++cnt;
    return tmpDir_ + std::string("/") + MtpFsSha::SumString(ss.str());
}

bool MtpFsTmpFilesPool::CreateTmpDir()
{
    if (RemoveTmpDir()) {
        return SmtpfsCreateDir(tmpDir_);
    }
    return false;
}

bool MtpFsTmpFilesPool::RemoveTmpDir()
{
    if (!tmpDir_.empty()) {
        return SmtpfsRemoveDir(tmpDir_);
    }
    return false;
}
