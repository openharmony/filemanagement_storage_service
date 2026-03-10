/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/xattr.h>

#include "file_sharing/acl.h"
#include "securec.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

constexpr int BUF_SIZE = 400;

namespace OHOS {
namespace StorageDaemon {
namespace {
int AclEntryParseTag(const std::string &tagTxt, AclXattrEntry &entry)
{
    if (tagTxt.empty()) {
        errno = EINVAL;
        return -1;
    }
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
    char *p = nullptr;
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
                LOGE("[L3:FileSharing] AclEntryParseId: <<< EXIT FAILED <<< getpwnam failed, name=%{public}s",
                     idTxt.c_str());
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
                LOGE("[L3:FileSharing] AclEntryParseId: <<< EXIT FAILED <<< getgrnam failed, name=%{public}s",
                     idTxt.c_str());
                return -1;
            }
            entry.id = grp->gr_gid;
            (void)memset_s(grp, sizeof(struct group), 0, sizeof(struct group));
            break;
        default:
            if (!idTxt.empty()) {
                errno = EINVAL;
                LOGE("[L3:FileSharing] AclEntryParseId: <<< EXIT FAILED <<< invalid qualifier for non-USER/GROUP tag");
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
        LOGE("[L3:FileSharing] AclEntryParsePerm: <<< EXIT FAILED <<< permission text is empty");
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
                LOGE("[L3:FileSharing] AclEntryParsePerm: <<< EXIT FAILED <<< invalid permission char=%{public}c", c);
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
        LOGE("[L3:FileSharing] AclEntryParseText: <<< EXIT FAILED <<< invalid ACL entry format, entryTxt=%{public}s",
             entryTxt.c_str());
        return {};
    }
    const std::string tagTxt = entryTxt.substr(last, pos - last);
    if (AclEntryParseTag(tagTxt, entry) == -1) {
        LOGE("[L3:FileSharing] AclEntryParseText: <<< EXIT FAILED <<< unknown tag=%{public}s", tagTxt.c_str());
        return {};
    }
    last = pos + 1;

    if ((pos = entryTxt.find(":", last)) == std::string::npos) {
        LOGE("[L3:FileSharing] AclEntryParseText: <<< EXIT FAILED <<< invalid ACL entry format, missing second colon");
        return {};
    }
    const std::string idTxt = entryTxt.substr(last, pos - last);
    if (AclEntryParseId(idTxt, entry) == -1) {
        switch (entry.tag) {
            case ACL_TAG::USER:
            case ACL_TAG::GROUP:
                LOGE("[L3:FileSharing] AclEntryParseText: <<< EXIT FAILED <<< qualifier=%{public}s, error=%{public}s",
                     idTxt.c_str(),
                     errno == 0 ? "user/group not found" : std::strerror(errno));
                break;
            default:
                LOGE("[L3:FileSharing] AclEntryParseText: <<< EXIT FAILED <<< qualifier only allowed for USER & GROUP");
                break;
        }
        return {};
    }
    last = pos + 1;

    const std::string permTxt = entryTxt.substr(last); // take substr till the end
    if (AclEntryParsePerm(permTxt, entry) == -1) {
        LOGE("[L3:FileSharing] AclEntryParseText: <<< EXIT FAILED <<< permission=%{public}s, error=%{public}s",
             permTxt.c_str(), std::strerror(errno));
        return {};
    }

    LOGI("[L3:FileSharing] AclEntryParseText: <<< EXIT SUCCESS <<< entryTxt=%{public}s", entryTxt.c_str());
    return entry;
}

Acl AclFromMode(const std::string &file)
{
    Acl acl;
    struct stat st;

    if (stat(file.c_str(), &st) == -1) {
        LOGE("[L3:FileSharing] AclFromMode: <<< EXIT FAILED <<< file=%{public}s, stat failed, errno=%{public}d",
             file.c_str(), errno);
        return acl;
    }

    acl.InsertEntry(
        { .tag = ACL_TAG::USER_OBJ,
          .perm = (st.st_mode & S_IRWXU) >> 6,
          .id = AclXattrHeader::ACL_UNDEFINED_ID, }
    );
    acl.InsertEntry(
        { .tag = ACL_TAG::GROUP_OBJ,
          .perm = (st.st_mode & S_IRWXG) >> 3,
          .id = AclXattrHeader::ACL_UNDEFINED_ID, }
    );
    acl.InsertEntry(
        { .tag = ACL_TAG::OTHER,
          .perm = (st.st_mode & S_IRWXO),
          .id = AclXattrHeader::ACL_UNDEFINED_ID, }
    );

    LOGI("[L3:FileSharing] AclFromMode: <<< EXIT SUCCESS <<< file=%{public}s", file.c_str());
    return acl;
}

Acl AclFromFile(const std::string &file)
{
    Acl acl;
    char buf[BUF_SIZE] = { 0 };
    ssize_t len = getxattr(file.c_str(), Acl::ACL_XATTR_ACCESS, buf, BUF_SIZE);
    if (len != -1) {
        acl.DeSerialize(buf, BUF_SIZE);
        LOGI("[L3:FileSharing] AclFromFile: <<< EXIT SUCCESS <<< file=%{public}s, len=%{public}zd",
             file.c_str(), len);
        return acl;
    }
    LOGI("[L3:FileSharing] AclFromFile: no xattr, use mode, file=%{public}s", file.c_str());
    return AclFromMode(file);
}

} // anonymous namespace

int AclSetAttribution(const std::string &targetFile, const std::string &entryTxt, const char *aclAttrName)
{
    LOGI("[L3:FileSharing] AclSetAttribution: >>> ENTER <<< file=%{public}s, attrName=%{public}s",
         targetFile.c_str(), aclAttrName);

    if (strcmp(aclAttrName, Acl::ACL_XATTR_ACCESS) && !IsDir(targetFile)) {
        LOGE("[L3:FileSharing] AclSetAttribution: <<< EXIT FAILED <<< file=%{public}s, error=%{public}s",
             targetFile.c_str(),
             errno == 0 ? "file exists but isn't a directory" : std::strerror(errno));
        return -1;
    }

    /* parse text */
    AclXattrEntry entry = AclEntryParseText(entryTxt);
    if (!entry.IsValid()) {
        LOGE("[L3:FileSharing] AclSetAttribution: <<< EXIT FAILED <<< file=%{public}s, parse failed,"
             "entryTxt=%{public}s",
             targetFile.c_str(), entryTxt.c_str());
        return -1;
    }

    /* init acl from file's mode */
    Acl acl;
    if (strcmp(aclAttrName, Acl::ACL_XATTR_ACCESS) == 0) {
        acl = AclFromFile(targetFile);
    } else {
        acl = AclFromMode(targetFile);
    }
    if (acl.IsEmpty()) {
        LOGE("[L3:FileSharing] AclSetAttribution: <<< EXIT FAILED <<< file=%{public}s, acl is empty",
             targetFile.c_str());
        return -1;
    }

    /* add new entry into set */
    if (acl.InsertEntry(entry) == -1) {
        LOGE("[L3:FileSharing] AclSetAttribution: <<< EXIT FAILED <<< file=%{public}s, insert failed, errno=%{public}d",
             targetFile.c_str(), errno);
        return -1;
    }

    /* transform to binary and write to file */
    size_t bufSize;
    char *buf = acl.Serialize(bufSize);
    if (buf == nullptr) {
        LOGE("[L3:FileSharing] AclSetAttribution: <<< EXIT FAILED <<< file=%{public}s, serialize failed,"
             "bufSize=%{public}zu",
             targetFile.c_str(), bufSize);
        return -1;
    }
    if (setxattr(targetFile.c_str(), aclAttrName, buf, bufSize, 0) == -1) {
        LOGE("[L3:FileSharing] AclSetAttribution: <<< EXIT FAILED <<< file=%{public}s, setxattr failed,"
             "errno=%{public}d",
             targetFile.c_str(), errno);
        return -1;
    }

    LOGI("[L3:FileSharing] AclSetAttribution: <<< EXIT SUCCESS <<< file=%{public}s, attrName=%{public}s",
         targetFile.c_str(), aclAttrName);
    return 0;
}

int AclSetDefault(const std::string &targetFile, const std::string &entryTxt)
{
    LOGI("[L3:FileSharing] AclSetDefault: >>> ENTER <<< file=%{public}s, entryTxt=%{public}s",
        targetFile.c_str(), entryTxt.c_str());
    int ret = AclSetAttribution(targetFile, entryTxt, Acl::ACL_XATTR_DEFAULT);
    if (ret == 0) {
        LOGI("[L3:FileSharing] AclSetDefault: <<< EXIT SUCCESS <<< file=%{public}s", targetFile.c_str());
    } else {
        LOGE("[L3:FileSharing] AclSetDefault: <<< EXIT FAILED <<< file=%{public}s, ret=%{public}d",
            targetFile.c_str(), ret);
    }
    return ret;
}

int AclSetAccess(const std::string &targetFile, const std::string &entryTxt)
{
    LOGI("[L3:FileSharing] AclSetAccess: >>> ENTER <<< file=%{public}s, entryTxt=%{public}s",
        targetFile.c_str(), entryTxt.c_str());
    int ret = AclSetAttribution(targetFile, entryTxt, Acl::ACL_XATTR_ACCESS);
    if (ret == 0) {
        LOGI("[L3:FileSharing] AclSetAccess: <<< EXIT SUCCESS <<< file=%{public}s", targetFile.c_str());
    } else {
        LOGE("[L3:FileSharing] AclSetAccess: <<< EXIT FAILED <<< file=%{public}s, ret=%{public}d",
            targetFile.c_str(), ret);
    }
    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS

