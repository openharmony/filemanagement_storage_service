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

#include "mtpfs_type_tmp_file.h"

MtpFsTypeTmpFile::MtpFsTypeTmpFile() : pathDevice_(), pathTmp_(), fileDescriptors_(), modified_(false) {}

MtpFsTypeTmpFile::MtpFsTypeTmpFile(const std::string &pathDevice, const std::string &pathTmp, int fileDesc,
    bool modified)
    : pathDevice_(pathDevice), pathTmp_(pathTmp), modified_(modified)
{
    fileDescriptors_.insert(fileDesc);
}

MtpFsTypeTmpFile::MtpFsTypeTmpFile(const MtpFsTypeTmpFile &copy)
    : pathDevice_(copy.pathDevice_),
      pathTmp_(copy.pathTmp_),
      fileDescriptors_(copy.fileDescriptors_),
      modified_(copy.modified_)
{}

bool MtpFsTypeTmpFile::HasFileDescriptor(int fd)
{
    auto it = std::find(fileDescriptors_.begin(), fileDescriptors_.end(), fd);
    return it != fileDescriptors_.end();
}

void MtpFsTypeTmpFile::RemoveFileDescriptor(int fd)
{
    auto it = std::find(fileDescriptors_.begin(), fileDescriptors_.end(), fd);
    if (it == fileDescriptors_.end()) {
        return;
    }
    fileDescriptors_.erase(it);
}

MtpFsTypeTmpFile &MtpFsTypeTmpFile::operator = (const MtpFsTypeTmpFile &rhs)
{
    pathDevice_ = rhs.pathDevice_;
    pathTmp_ = rhs.pathTmp_;
    fileDescriptors_ = rhs.fileDescriptors_;
    modified_ = rhs.modified_;
    return *this;
}
