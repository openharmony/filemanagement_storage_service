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
#include <cerrno>
#include <string>

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/xattr.h>

#include "file_sharing/acl.h"
#include "securec.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
int AclEntryParseTag(const std::string &tagTxt, AclXattrEntry &entry)
{
    switch (tagTxt[0]) {
        case 'u':
            entry.tag = ACL_TAG::USER;
            break;
        case 'g':
            entry.tag = ACL_TAG::GROUP;
            break;
        case 'm':
            entry.tag = ACL_TAG::MASK;
            break;
        case 'o':
            entry.tag = ACL_TAG::OTHER;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    return 0;
}

bool ParseNumericId(const std::string &idTxt, unsigned int &outId)
{
    char *p;
    long converted = strtol(idTxt.c_str(), &p, 10);
    if (*p == '\0' && converted >= 0 && converted <= UINT_MAX) {
        outId = static_cast<unsigned int>(converted);
        return true;
    }
    return false;
}

int AclEntryParseId(const std::string &idTxt, AclXattrEntry &entry)
{
    struct passwd *pwd = nullptr;
    struct group *grp = nullptr;

    switch (entry.tag) {
        case ACL_TAG::USER:
            if (idTxt.empty()) {
                entry.tag = ACL_TAG::USER_OBJ;
                return 0;
            }
            if (ParseNumericId(idTxt, entry.id)) {
                break;
            }
            if ((pwd = getpwnam(idTxt.c_str())) == nullptr) {
                return -1;
            }
            entry.id = pwd->pw_uid;
            (void)memset_s(pwd, sizeof(struct passwd), 0, sizeof(struct passwd));
            break;
        case ACL_TAG::GROUP:
            if (idTxt.empty()) {
                entry.tag = ACL_TAG::GROUP_OBJ;
                return 0;
            }
            if (ParseNumericId(idTxt, entry.id)) {
                break;
            }
            if ((grp = getgrnam(idTxt.c_str())) == nullptr) {
                return -1;
            }
            entry.id = grp->gr_gid;
            (void)memset_s(grp, sizeof(struct group), 0, sizeof(struct group));
            break;
        default:
            if (!idTxt.empty()) {
                errno = EINVAL;
                return -1;
            }
            break;
    }
    return 0;
}

int AclEntryParsePerm(const std::string &permTxt, AclXattrEntry &entry)
{
    if (permTxt.empty()) {
        errno = EINVAL;
        return -1;
    }
    for (const char &c : permTxt) {
        switch (c) {
            case 'r':
                entry.perm.SetR();
                break;
            case 'w':
                entry.perm.SetW();
                break;
            case 'x':
                entry.perm.SetE();
                break;
            case '-':
                break;
            default:
                errno = EINVAL;
                return -1;
        }
    }
    return 0;
}

AclXattrEntry AclEntryParseText(const std::string &entryTxt)
{
    AclXattrEntry entry = {};
    std::string::size_type last = 0;
    std::string::size_type pos;

    if ((pos = entryTxt.find(":", last)) == std::string::npos) {
        LOGE("Invalid ACL entry format");
        return {};
    }
    const std::string tagTxt = entryTxt.substr(last, pos-last);
    if (AclEntryParseTag(tagTxt, entry) == -1) {
        LOGE("Unknown tag: %{public}s", tagTxt.c_str());
        return {};
    }
    last = pos + 1;

    if ((pos = entryTxt.find(":", last)) == std::string::npos) {
        LOGE("Invalid ACL entry format");
        return {};
    }
    const std::string idTxt = entryTxt.substr(last, pos-last);
    if (AclEntryParseId(idTxt, entry) == -1) {
        switch (entry.tag) {
            case ACL_TAG::USER:
            case ACL_TAG::GROUP:
                LOGE("Error in processing qualifier: \"%{public}s\": %{public}s",
                     idTxt.c_str(),
                     errno == 0 ? "user/group not found" : std::strerror(errno));
                break;
            default:
                LOGE("Qualifier only allowed for USER & GROUP");
                break;
        }
        return {};
    }
    last = pos + 1;

    const std::string permTxt = entryTxt.substr(last); // take substr till the end
    if (AclEntryParsePerm(permTxt, entry) == -1) {
        LOGE("Wrong permission: %{public}s", permTxt.c_str());
        return {};
    }

    return entry;
}

Acl AclFromMode(const std::string &file)
{
    Acl acl;
    struct stat st;

    if (stat(file.c_str(), &st) == -1) {
        return acl;
    }

    acl.InsertEntry(
        { .tag = ACL_TAG::USER_OBJ,
          .perm = (st.st_mode & S_IRWXU) >> 6,
          .id = ACL_UNDEFINED_ID, }
    );
    acl.InsertEntry(
        { .tag = ACL_TAG::GROUP_OBJ,
          .perm = (st.st_mode & S_IRWXG) >> 3,
          .id = ACL_UNDEFINED_ID, }
    );
    acl.InsertEntry(
        { .tag = ACL_TAG::OTHER,
          .perm = (st.st_mode & S_IRWXO),
          .id = ACL_UNDEFINED_ID, }
    );

    return acl;
}
} // anonymous namespace

int AclSetDefault(const std::string &targetFile, const std::string &entryTxt)
{
    if (!IsDir(targetFile)) {
        LOGE("Failed to confirm %{private}s is a directory: %{public}s",
             targetFile.c_str(),
             errno == 0 ? "file exists but isn't a directory" : std::strerror(errno));
        return -1;
    }

    /* parse text */
    AclXattrEntry entry = AclEntryParseText(entryTxt);
    if (!entry.IsValid()) {
        LOGE("Failed to parse entry text: %{public}s", std::strerror(errno));
        return -1;
    }

    /* init acl from file's mode */
    Acl acl = AclFromMode(targetFile);
    if (acl.IsEmpty()) {
        LOGE("Failed to generate ACL from file's mode: %{public}s", std::strerror(errno));
        return -1;
    }

    /* add new entry into set */
    if (acl.InsertEntry(entry) == -1) {
        LOGE("Failed to insert new entry into ACL: %{public}s", std::strerror(errno));
        return -1;
    }

    /* transform to binary and write to file */
    size_t bufSize;
    char *buf = acl.Serialize(bufSize);
    if (buf == nullptr) {
        LOGE("Failed to serialize ACL into binary: %{public}s", std::strerror(errno));
        return -1;
    }
    if (setxattr(targetFile.c_str(), ACL_XATTR_DEFAULT, buf, bufSize, 0) == -1) {
        LOGE("Failed to write into file's xattr: %{public}s", std::strerror(errno));
        return -1;
    }

    return 0;
}
} // namespace StorageDaemon
} // namespace OHOS
