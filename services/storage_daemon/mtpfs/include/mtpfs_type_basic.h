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

#ifndef MTPFS_TYPE_BASIC_H
#define MTPFS_TYPE_BASIC_H

#include <string>
#include <cstdint>

class MtpFsTypeBasic {
public:
    MtpFsTypeBasic() : id_(0), parentId_(0), storageId_(0), name_() {}

    MtpFsTypeBasic(uint32_t id, uint32_t parentId, uint32_t storageId, const std::string &name)
        : id_(id), parentId_(parentId), storageId_(storageId), name_(name)
    {}

    MtpFsTypeBasic(const MtpFsTypeBasic &copy)
        : id_(copy.id_), parentId_(copy.parentId_), storageId_(copy.storageId_), name_(copy.name_)
    {}

    uint32_t Id() const
    {
        return id_;
    }
    uint32_t ParentId() const
    {
        return parentId_;
    }
    uint32_t StorageId() const
    {
        return storageId_;
    }
    std::string Name() const
    {
        return name_;
    }

    void SetId(uint32_t id)
    {
        id_ = id;
    }
    void SetParent(uint32_t parentId)
    {
        parentId_ = parentId;
    }
    void SetStorage(uint32_t storageId)
    {
        storageId_ = storageId;
    }
    void SetName(const std::string &name)
    {
        name_ = name;
    }

    MtpFsTypeBasic &operator = (const MtpFsTypeBasic &rhs)
    {
        id_ = rhs.id_;
        parentId_ = rhs.parentId_;
        storageId_ = rhs.storageId_;
        name_ = rhs.name_;
        return *this;
    }

    bool operator == (const std::string &rhs) const
    {
        return name_ == rhs;
    }
    bool operator == (const MtpFsTypeBasic &rhs) const
    {
        return name_ == rhs.name_;
    }
    bool operator < (const std::string &rhs) const
    {
        return name_ < rhs;
    }
    bool operator < (const MtpFsTypeBasic &rhs) const
    {
        return name_ < rhs.name_;
    }

protected:
    uint32_t id_;
    uint32_t parentId_;
    uint32_t storageId_;
    std::string name_;
};

#endif // MTPFS_TYPE_BASIC_H
