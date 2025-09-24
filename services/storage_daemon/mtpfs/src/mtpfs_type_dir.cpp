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

MtpFsTypeDir::MtpFsTypeDir() : MtpFsTypeBasic(), dirList_(), fileList_(), fetched_(false), modifyDate_(0) {}

MtpFsTypeDir::MtpFsTypeDir(uint32_t id, uint32_t parentId, uint32_t storageId, const std::string &name)
    : MtpFsTypeBasic(id, parentId, storageId, name),
      dirList_(),
      fileList_(),
      fetched_(false),
      modifyDate_(0)
{}

MtpFsTypeDir::MtpFsTypeDir(LIBMTP_file_t *file)
    : MtpFsTypeBasic(file->item_id, file->parent_id, file->storage_id, std::string(file->filename)),
      dirList_(),
      fileList_(),
      fetched_(false),
      modifyDate_(file->modificationdate)
{}

MtpFsTypeDir::MtpFsTypeDir(const MtpFsTypeDir &copy)
    : MtpFsTypeBasic(copy),
      dirList_(copy.dirList_),
      fileList_(copy.fileList_),
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
    if (f->name == nullptr) {
        free(static_cast<void *>(f));
        return nullptr;
    }
    f->sibling = nullptr;
    f->child = nullptr;
    return f;
}

void MtpFsTypeDir::Clear()
{
    std::unique_lock<std::mutex> lock(mutex_);
    dirList_.clear();
    fileList_.clear();
}

void MtpFsTypeDir::AddDir(const MtpFsTypeDir &dir)
{
    std::unique_lock<std::mutex> lock(mutex_);
    dirList_.insert(dir);
}

void MtpFsTypeDir::AddFile(const MtpFsTypeFile &file)
{
    std::unique_lock<std::mutex> lock(mutex_);
    fileList_.insert(file);
}

bool MtpFsTypeDir::RemoveDir(const MtpFsTypeDir &dir)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = std::find(dirList_.begin(), dirList_.end(), dir);
    if (it == dirList_.end()) {
        return false;
    }
    dirList_.erase(it);
    return true;
}

bool MtpFsTypeDir::RemoveFile(const MtpFsTypeFile &file)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = std::find(fileList_.begin(), fileList_.end(), file);
    if (it == fileList_.end()) {
        return false;
    }
    fileList_.erase(it);
    return true;
}

bool MtpFsTypeDir::ReplaceFile(const MtpFsTypeFile &oldFile, const MtpFsTypeFile &newFile)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = std::find(fileList_.begin(), fileList_.end(), oldFile);
    if (it == fileList_.end()) {
        return false;
    }
    fileList_.erase(it);
    fileList_.insert(newFile);
    return true;
}

MtpFsTypeDir &MtpFsTypeDir::operator = (const MtpFsTypeDir &rhs)
{
    MtpFsTypeBasic::operator = (rhs);
    dirList_ = rhs.dirList_;
    fileList_ = rhs.fileList_;
    fetched_ = rhs.fetched_;
    return *this;
}

const MtpFsTypeDir *MtpFsTypeDir::Dir(const std::string &name) const
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = std::find(dirList_.begin(), dirList_.end(), name);
    if (it == dirList_.end()) {
        return nullptr;
    }
    return &*it;
}

const MtpFsTypeFile *MtpFsTypeDir::File(const std::string &name) const
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = std::find(fileList_.begin(), fileList_.end(), name);
    if (it == fileList_.end()) {
        return nullptr;
    }
    return &*it;
}
