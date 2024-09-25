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

#ifndef MTPFS_TYPE_FILE_H
#define MTPFS_TYPE_FILE_H

#include <libmtp.h>
#include "mtpfs_type_basic.h"

class MtpFsTypeFile : public MtpFsTypeBasic {
public:
    MtpFsTypeFile();
    MtpFsTypeFile(uint32_t id, uint32_t parentId, uint32_t storageId, const std::string &name, uint64_t size,
        time_t modifyDate);
    MtpFsTypeFile(LIBMTP_file_t *file);
    MtpFsTypeFile(const MtpFsTypeFile &copy);

    uint64_t Size() const
    {
        return size_;
    }
    time_t ModificationDate() const
    {
        return modifyDate_;
    }

    void SetSize(uint64_t size)
    {
        size_ = size;
    }
    void SetModificationDate(time_t modifyDate)
    {
        modifyDate_ = modifyDate;
    }

    LIBMTP_file_t *ToLIBMTPFile() const;
    MtpFsTypeFile &operator = (const MtpFsTypeFile &rhs);

    bool operator == (const std::string &rhs) const
    {
        return MtpFsTypeBasic::operator == (rhs);
    }
    bool operator == (const MtpFsTypeFile &rhs) const
    {
        return MtpFsTypeBasic::operator == (rhs);
    }
    bool operator < (const std::string &rhs) const
    {
        return MtpFsTypeBasic::operator < (rhs);
    }
    bool operator < (const MtpFsTypeFile &rhs) const
    {
        return MtpFsTypeBasic::operator < (rhs);
    }

private:
    uint64_t size_;
    time_t modifyDate_;
};

#endif // MTPFS_TYPE_FILE_H
