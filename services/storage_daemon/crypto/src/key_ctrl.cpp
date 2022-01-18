/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "key_ctrl.h"

#include <vector>
#include <map>
#include <sys/syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/keyctl.h>

#include "utils/log.h"
#include "securec.h"
#include "file_ex.h"
#include "string_ex.h"

namespace OHOS {
namespace StorageDaemon {
key_serial_t KeyCtrl::AddKey(const std::string &type, const std::string &description, const key_serial_t ringId)
{
    return syscall(__NR_add_key, type.c_str(), description.c_str(), nullptr, 0, ringId);
}
key_serial_t KeyCtrl::AddKey(const std::string &type, const std::string &description, fscrypt_key &fsKey,
    const key_serial_t ringId)
{
    return syscall(__NR_add_key, type.c_str(), description.c_str(), static_cast<void *>(&fsKey), sizeof(fsKey),
        ringId);
}

key_serial_t KeyCtrl::GetKeyring(key_serial_t id, int create)
{
    return syscall(__NR_keyctl, KEYCTL_GET_KEYRING_ID, id, create);
}

long KeyCtrl::Revoke(key_serial_t id)
{
    return syscall(__NR_keyctl, KEYCTL_REVOKE, id);
}

long KeyCtrl::Search(key_serial_t ringId, const std::string &type, const std::string &description,
    key_serial_t destRingId)
{
    return syscall(__NR_keyctl, KEYCTL_SEARCH, ringId, type.data(), description.data(), destRingId);
}

long KeyCtrl::SetPermission(key_serial_t id, int permissions)
{
    return syscall(__NR_keyctl, KEYCTL_SETPERM, id, permissions);
}

long KeyCtrl::Unlink(key_serial_t key, key_serial_t keyring)
{
    return syscall(__NR_keyctl, KEYCTL_UNLINK, key, keyring);
}

long KeyCtrl::RestrictKeyring(key_serial_t keyring, const std::string &type, const std::string &restriction)
{
    return syscall(__NR_keyctl, KEYCTL_RESTRICT_KEYRING, keyring, type.data(), restriction.data());
}

long KeyCtrl::GetSecurity(key_serial_t id, std::string &buffer)
{
    return syscall(__NR_keyctl, KEYCTL_GET_SECURITY, id, buffer.data(), buffer.length());
}

bool FsIoctl(const std::string &mnt, unsigned long cmd, void *arg)
{
    int fd = open(mnt.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno:%{public}d", mnt.c_str(), errno);
        return false;
    }
    if (ioctl(fd, cmd, arg) != 0) {
        LOGE("ioctl to %{public}s failed, errno:%{public}d", mnt.c_str(), errno);
        close(fd);
        return false;
    }
    close(fd);
    LOGD("success");
    return true;
}

bool KeyCtrl::InstallKey(const std::string &mnt, fscrypt_add_key_arg &arg)
{
    LOGD("enter");
    return FsIoctl(mnt, FS_IOC_ADD_ENCRYPTION_KEY, reinterpret_cast<void *>(&arg));
}

bool KeyCtrl::RemoveKey(const std::string &mnt, fscrypt_remove_key_arg &arg)
{
    LOGD("enter");
    return FsIoctl(mnt, FS_IOC_REMOVE_ENCRYPTION_KEY, reinterpret_cast<void *>(&arg));
}

bool KeyCtrl::GetKeyStatus(const std::string &mnt, fscrypt_get_key_status_arg &arg)
{
    LOGD("enter");
    return FsIoctl(mnt, FS_IOC_GET_ENCRYPTION_KEY_STATUS, reinterpret_cast<void *>(&arg));
}

bool KeyCtrl::SetPolicy(const std::string &path, fscrypt_policy_v1 &policy)
{
    LOGD("enter");
    return FsIoctl(path, FS_IOC_SET_ENCRYPTION_POLICY, reinterpret_cast<void *>(&policy));
}

bool KeyCtrl::SetPolicy(const std::string &path, fscrypt_policy_v2 &policy)
{
    LOGD("enter");
    return FsIoctl(path, FS_IOC_SET_ENCRYPTION_POLICY, reinterpret_cast<void *>(&policy));
}

bool KeyCtrl::GetPolicy(const std::string &path, fscrypt_get_policy_ex_arg &policy)
{
    LOGD("enter");
    return FsIoctl(path, FS_IOC_GET_ENCRYPTION_POLICY_EX, reinterpret_cast<void *>(&policy));
}

static bool ParseOption(const std::map<std::string, uint8_t> &policy, const std::string &defaultStr, uint8_t &value)
{
    if (policy.find(defaultStr) != policy.end()) {
        LOGI("use default: %{public}s", defaultStr.c_str());
        value = policy.at(defaultStr);
        return true;
    }

    LOGE("bad default policy: %{public}s, value not filled!", defaultStr.c_str());
    return false;
}

bool KeyCtrl::LoadAndSetPolicy(const std::string &keyIdPath, const std::string &policyFile, const std::string &toEncrypt)
{
    LOGD("enter");
    std::string keyId;
    if (OHOS::LoadStringFromFile(keyIdPath, keyId) == false || keyId.length() != FSCRYPT_KEY_IDENTIFIER_SIZE) {
        LOGE("bad kid file content, length=%{public}u", static_cast<uint32_t>(keyId.length()));
        return false;
    }

    // Add parsing options from the policy file, now using default.
    (void)policyFile;

    struct fscrypt_policy_v2 arg = {.version = FSCRYPT_POLICY_V2};
    (void)memcpy_s(arg.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, keyId.data(), keyId.length());
    if (!ParseOption(FILENAME_MODES, DEFAULT_POLICY.fileName, arg.filenames_encryption_mode) ||
        !ParseOption(CONTENTS_MODES, DEFAULT_POLICY.content, arg.contents_encryption_mode) ||
        !ParseOption(POLICY_FLAGS, DEFAULT_POLICY.flags, arg.flags)) {
        LOGE("parse option failed");
        return false;
    }
    return KeyCtrl::SetPolicy(toEncrypt, arg);
}
} // namespace StorageDaemon
} // namespace OHOS
