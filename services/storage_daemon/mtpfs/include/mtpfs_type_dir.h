/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef MTPFS_TYPE_DIR_H
#define MTPFS_TYPE_DIR_H

#include <mutex>
#include <set>
#include <string>

#include "mtpfs_type_basic.h"
#include "mtpfs_type_file.h"

class MtpFsTypeDir : public MtpFsTypeBasic {
public:
    MtpFsTypeDir();
    MtpFsTypeDir(uint32_t id, uint32_t parentId, uint32_t storageId, const std::string &name);
    MtpFsTypeDir(LIBMTP_file_t *file);
    MtpFsTypeDir(const MtpFsTypeDir &copy);

    void Clear()
    {
        dirList_.clear();
        fileList_.clear();
    }
    void SetFetched(bool f)
    {
        fetched_ = f;
    }
    bool IsFetched() const
    {
        return fetched_;
    }
    void AddDir(const MtpFsTypeDir &dir);
    void AddFile(const MtpFsTypeFile &file);
    bool RemoveDir(const MtpFsTypeDir &dir);
    bool RemoveFile(const MtpFsTypeFile &file);
    bool ReplaceFile(const MtpFsTypeFile &oldFile, const MtpFsTypeFile &newFile);

    std::set<MtpFsTypeDir>::size_type DirCount() const
    {
        return dirList_.size();
    }
    std::set<MtpFsTypeFile>::size_type FileCount() const
    {
        return fileList_.size();
    }
    const MtpFsTypeDir *Dir(const std::string &name) const;
    const MtpFsTypeFile *File(const std::string &name) const;
    std::set<MtpFsTypeDir> Dirs() const
    {
        return dirList_;
    }
    std::set<MtpFsTypeFile> Files() const
    {
        return fileList_;
    }
    bool IsEmpty() const
    {
        return dirList_.empty() && fileList_.empty();
    }

    time_t ModificationDate() const
    {
        return modifyDate_;
    }
    void SetModificationDate(time_t modifyDate)
    {
        modifyDate_ = modifyDate;
    }

    LIBMTP_folder_t *ToLIBMTPFolder() const;
    MtpFsTypeDir &operator = (const MtpFsTypeDir &rhs);
    bool operator == (const std::string &rhs) const
    {
        return MtpFsTypeBasic::operator == (rhs);
    }
    bool operator == (const MtpFsTypeDir &rhs) const
    {
        return MtpFsTypeBasic::operator == (rhs);
    }
    bool operator < (const std::string &rhs) const
    {
        return MtpFsTypeBasic::operator < (rhs);
    }
    bool operator < (const MtpFsTypeDir &rhs) const
    {
        return MtpFsTypeBasic::operator < (rhs);
    }

private:
    mutable std::mutex mutex_;
    bool fetched_ = false;
    time_t modifyDate_;

public:
    LIBMTP_object_handles_t *objHandles = nullptr;
    std::set<MtpFsTypeDir> dirList_;
    std::set<MtpFsTypeFile> fileList_;
};

#endif // MTPFS_TYPE_DIR_H
