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

#include "mtpfs_type_file.h"

MtpFsTypeFile::MtpFsTypeFile() : MtpFsTypeBasic(), size_(0), modifyDate_(0) {}

MtpFsTypeFile::MtpFsTypeFile(uint32_t id, uint32_t parentId, uint32_t storageId, const std::string &name, uint64_t size,
    time_t modifyDate)
    : MtpFsTypeBasic(id, parentId, storageId, name), size_(size), modifyDate_(modifyDate)
{}

MtpFsTypeFile::MtpFsTypeFile(LIBMTP_file_t *file)
    : MtpFsTypeBasic(file->item_id, file->parent_id, file->storage_id, std::string(file->filename)),
      size_(file->filesize),
      modifyDate_(file->modificationdate)
{}

MtpFsTypeFile::MtpFsTypeFile(const MtpFsTypeFile &copy)
    : MtpFsTypeBasic(copy), size_(copy.size_), modifyDate_(copy.modifyDate_)
{}

LIBMTP_file_t *MtpFsTypeFile::ToLIBMTPFile() const
{
    LIBMTP_file_t *f = static_cast<LIBMTP_file_t *>(malloc(sizeof(LIBMTP_file_t)));
    if (f == nullptr) {
        return nullptr;
    }
    f->item_id = id_;
    f->parent_id = parentId_;
    f->storage_id = storageId_;
    f->filename = strdup(name_.c_str());
    if (f->filename == nullptr) {
        free(static_cast<void *>(f));
        return nullptr;
    }
    f->filesize = size_;
    f->modificationdate = modifyDate_;
    f->filetype = LIBMTP_FILETYPE_UNKNOWN;
    f->next = nullptr;
    return f;
}

MtpFsTypeFile &MtpFsTypeFile::operator = (const MtpFsTypeFile &rhs)
{
    MtpFsTypeBasic::operator = (rhs);
    size_ = rhs.size_;
    modifyDate_ = rhs.modifyDate_;
    return *this;
}
