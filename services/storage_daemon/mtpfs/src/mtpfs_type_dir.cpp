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

#include "mtpfs_type_dir.h"

MtpFsTypeDir::MtpFsTypeDir() : MtpFsTypeBasic(), dirs_(), files_(), accessMutex_(), fetched_(false), modifyDate_(0) {}

MtpFsTypeDir::MtpFsTypeDir(uint32_t id, uint32_t parentId, uint32_t storageId, const std::string &name)
    : MtpFsTypeBasic(id, parentId, storageId, name),
      dirs_(),
      files_(),
      accessMutex_(),
      fetched_(false),
      modifyDate_(0)
{}

MtpFsTypeDir::MtpFsTypeDir(LIBMTP_file_t *file)
    : MtpFsTypeBasic(file->item_id, file->parent_id, file->storage_id, std::string(file->filename)),
      dirs_(),
      files_(),
      accessMutex_(),
      fetched_(false),
      modifyDate_(file->modificationdate)
{}

MtpFsTypeDir::MtpFsTypeDir(const MtpFsTypeDir &copy)
    : MtpFsTypeBasic(copy),
      dirs_(copy.dirs_),
      files_(copy.files_),
      accessMutex_(),
      fetched_(copy.fetched_),
      modifyDate_(copy.modifyDate_)
{}

LIBMTP_folder_t *MtpFsTypeDir::ToLIBMTPFolder() const
{
    LIBMTP_folder_t *f = static_cast<LIBMTP_folder_t *>(malloc(sizeof(LIBMTP_folder_t)));
    if (f == nullptr) {
        return nullptr;
    }
    f->folder_id = id_;
    f->parent_id = parentId_;
    f->storage_id = storageId_;
    f->name = strdup(name_.c_str());
    f->sibling = nullptr;
    f->child = nullptr;
    return f;
}

void MtpFsTypeDir::AddDir(const MtpFsTypeDir &dir)
{
    EnterCritical();
    dirs_.insert(dir);
    LeaveCritical();
}

void MtpFsTypeDir::AddFile(const MtpFsTypeFile &file)
{
    EnterCritical();
    files_.insert(file);
    LeaveCritical();
}

bool MtpFsTypeDir::RemoveDir(const MtpFsTypeDir &dir)
{
    EnterCritical();
    auto it = std::find(dirs_.begin(), dirs_.end(), dir);
    if (it == dirs_.end()) {
        LeaveCritical();
        return false;
    }
    dirs_.erase(it);
    LeaveCritical();
    return true;
}

bool MtpFsTypeDir::RemoveFile(const MtpFsTypeFile &file)
{
    EnterCritical();
    auto it = std::find(files_.begin(), files_.end(), file);
    if (it == files_.end()) {
        LeaveCritical();
        return false;
    }
    files_.erase(it);
    LeaveCritical();
    return true;
}

bool MtpFsTypeDir::ReplaceFile(const MtpFsTypeFile &oldFile, const MtpFsTypeFile &newFile)
{
    EnterCritical();
    auto it = std::find(files_.begin(), files_.end(), oldFile);
    if (it == files_.end()) {
        LeaveCritical();
        return false;
    }
    files_.erase(it);
    files_.insert(newFile);
    LeaveCritical();
    return true;
}

MtpFsTypeDir &MtpFsTypeDir::operator = (const MtpFsTypeDir &rhs)
{
    MtpFsTypeBasic::operator = (rhs);
    dirs_ = rhs.dirs_;
    files_ = rhs.files_;
    fetched_ = rhs.fetched_;
    return *this;
}

const MtpFsTypeDir *MtpFsTypeDir::Dir(const std::string &name) const
{
    EnterCritical();
    auto it = std::find(dirs_.begin(), dirs_.end(), name);
    LeaveCritical();
    if (it == dirs_.end()) {
        return nullptr;
    }
    return &*it;
}

const MtpFsTypeFile *MtpFsTypeDir::File(const std::string &name) const
{
    EnterCritical();
    auto it = std::find(files_.begin(), files_.end(), name);
    LeaveCritical();
    if (it == files_.end()) {
        return nullptr;
    }
    return &*it;
}
