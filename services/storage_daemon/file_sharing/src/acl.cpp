/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "file_sharing/acl.h"

#include <cerrno>
#include <new>
#include <type_traits>
#include "file_sharing/endian.h"
#include "securec.h"

namespace OHOS {
namespace StorageDaemon {
ACL_PERM Acl::ReCalcMaskPerm()
{
    ACL_PERM perm;
    for (const auto &e : entries) {
        if (e.tag == ACL_TAG::USER || e.tag == ACL_TAG::GROUP_OBJ || e.tag == ACL_TAG::GROUP) {
            perm.Merge(e.perm);
        }
    }
    return perm;
}

bool Acl::IsEmpty()
{
    return entries.empty();
}

bool Acl::IsValid()
{
    if (!entries.count(ACL_TAG::USER_OBJ) || !entries.count(ACL_TAG::GROUP_OBJ) ||
            !entries.count(ACL_TAG::OTHER)) {
        return false;
    }
    if (maskDemand && !entries.count(ACL_TAG::MASK)) {
        return false;
    }
    return true;
}

int Acl::InsertEntry(const AclXattrEntry &entry)
{
    if (entries.size() >= ENTRIES_MAX_NUM) {
        errno = EAGAIN;
        return -1;
    }

    switch (entry.tag) {
        case ACL_TAG::GROUP_OBJ:
        case ACL_TAG::USER_OBJ:
        case ACL_TAG::MASK:
        case ACL_TAG::OTHER:
            entries.insert(entry);
            break;
        case ACL_TAG::USER:
        case ACL_TAG::GROUP:
            entries.insert(entry); // must before ReCalcMaskPerm()
            maskDemand++;
            /*
             * In either case there's no or already one ACL_MASK entry in the set,
             * we need to re-calculate MASK's permission and *insert* it (to replace
             * the old one in latter case since we can't change std::set's element
             * in-place). So do the following unconditionally.
             *
             * Be warned: do _NOT_ combine the following into one line, otherwise
             * you can't pass the !!genius!! CI coding style check.
             */
            entries.insert(
                { ACL_TAG::MASK, ReCalcMaskPerm(), ACL_UNDEFINED_ID }
            );
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    return 0;
}

char *Acl::Serialize(size_t &bufSize)
{
    if (!IsValid()) {
        errno = EINVAL;
        return nullptr;
    }

    /* clear possible previous allocation */
    if (buf != nullptr) {
        delete[] buf;
        buf = nullptr;
    }

    bufSize = sizeof(AclXattrHeader) + sizeof(AclXattrEntry) * entries.size();
    if (bufSize > BUF_MAX_SIZE) {
        bufSize = 0;
        errno = EINVAL;
        return nullptr;
    }
    buf = new (std::nothrow) char[bufSize];
    if (buf == nullptr) {
        errno = ENOMEM;
        return nullptr;
    }
    auto err = memcpy_s(buf, bufSize, &header, sizeof(AclXattrHeader));
    if (err != EOK) {
        errno = err;
        delete[] buf;
        buf = nullptr;
        return nullptr;
    }

    size_t restSize = bufSize - sizeof(AclXattrHeader);
    AclXattrEntry *ptr = reinterpret_cast<AclXattrEntry *>(buf + sizeof(AclXattrHeader));
    for (const auto &e : entries) {
        auto err = memcpy_s(ptr++, restSize, &e, sizeof(AclXattrEntry));
        if (err != EOK) {
            errno = err;
            delete[] buf;
            buf = nullptr;
            return nullptr;
        }
        restSize -= sizeof(AclXattrEntry);
    }
    return buf;
}

int Acl::DeSerialize(const char *p, size_t size)
{
    if (size > BUF_MAX_SIZE || size < sizeof(AclXattrHeader)) {
        errno = EINVAL;
        return -1;
    }
    header = *reinterpret_cast<const AclXattrHeader *>(p);
    size -= sizeof(AclXattrHeader);
    p += sizeof(AclXattrHeader);

    /*
     * `e->tag != ACL_TAG::UNDEFINED` is unreliable outside the buffer, so check
     * it after checking the size of remaining buffer.
     */
    for (const AclXattrEntry *e = reinterpret_cast<const AclXattrEntry *>(p);
            size >= sizeof(AclXattrEntry) && LeToCpu(e->tag) != ACL_TAG::UNDEFINED;
            e++) {
        InsertEntry(*e);
        size -= sizeof(AclXattrEntry);
    }
    if (size != 0) {
        entries.clear();
        header = { 0 };
        errno = EINVAL;
        return -1;
    }

    return 0;
}

Acl::~Acl()
{
    if (buf != nullptr) {
        delete[] buf;
        buf = nullptr;
    }
}
} // STORAGE_DAEMON
} // OHOS
