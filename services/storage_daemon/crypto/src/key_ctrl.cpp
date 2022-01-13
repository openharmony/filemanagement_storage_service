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
    int fd = open(mnt.c_str(), O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
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

bool KeyCtrl::SetPolicy(const std::string &path, FscryptPolicy &policy)
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

static bool SetPolicyLegacy(const std::string &keyDescPath, const std::string &toEncrypt, FscryptPolicy &arg)
{
    std::string keyDesc;
    if (OHOS::LoadStringFromFile(keyDescPath, keyDesc) == false || keyDesc.length() != FSCRYPT_KEY_DESCRIPTOR_SIZE) {
        LOGE("bad key_desc file content, length=%{public}d", keyDesc.length());
        return false;
    }

    arg.v1.version = FSCRYPT_POLICY_V1;
    (void)memcpy_s(arg.v1.master_key_descriptor, FSCRYPT_KEY_DESCRIPTOR_SIZE, keyDesc.data(), keyDesc.length());
    return KeyCtrl::SetPolicy(toEncrypt, arg);
}

static bool SetPolicyV2(const std::string &keyIdPath, const std::string &toEncrypt, FscryptPolicy &arg)
{
    std::string keyId;
    if (OHOS::LoadStringFromFile(keyIdPath, keyId) == false || keyId.length() != FSCRYPT_KEY_IDENTIFIER_SIZE) {
        LOGE("bad key_id file content, length=%{public}u", static_cast<uint32_t>(keyId.length()));
        return false;
    }

    arg.v2.version = FSCRYPT_POLICY_V2;
    (void)memcpy_s(arg.v2.master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, keyId.data(), keyId.length());
    return KeyCtrl::SetPolicy(toEncrypt, arg);
}

bool KeyCtrl::LoadAndSetPolicy(const std::string &keyPath, const std::string &policyFile, const std::string &toEncrypt)
{
    LOGD("enter");

    FscryptPolicy arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    // the modes and flags shares the same offset in the struct
    if (!ParseOption(FILENAME_MODES, DEFAULT_POLICY.fileName, arg.v1.filenames_encryption_mode) ||
        !ParseOption(CONTENTS_MODES, DEFAULT_POLICY.content, arg.v1.contents_encryption_mode) ||
        !ParseOption(POLICY_FLAGS, DEFAULT_POLICY.flags, arg.v1.flags)) {
        return false;
    }

    // Add parsing options from the policy file, now using default.
    (void)policyFile;

    std::string buf{};
    if (OHOS::LoadStringFromFile(keyPath + "/version", buf)) {
        if (buf == "1") {
            return SetPolicyLegacy(keyPath + "/key_desc", toEncrypt, arg);
        } else if (buf == "2") {
            return SetPolicyV2(keyPath + "/key_id", toEncrypt, arg);
        } else {
            LOGE("bad version file : %{public}s", buf.c_str());
            return false;
        }
    } else {
        LOGE("fail to read the version file.");
        return false;
    }
}

static bool IsKernelSupportFscryptV2(const std::string &mnt)
{
    int fd = open(mnt.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno: %{public}d", mnt.c_str(), errno);
        return false;
    }

    errno = 0;
    (void)ioctl(fd, FS_IOC_ADD_ENCRYPTION_KEY, nullptr);
    close(fd);
    if (errno == ENOTTY) {
        LOGI("Kernel doesn't support FS_IOC_ADD_ENCRYPTION_KEY.");
        return false;
    }
    if (errno == EFAULT) {
        LOGI("Kernel is support FS_IOC_ADD_ENCRYPTION_KEY.");
        return true;
    }
    LOGW("Unexpected errno: %{public}d", errno);
    return false;
}

uint8_t KeyCtrl::GetFscryptVersion(const std::string &mnt)
{
    static uint8_t version = IsKernelSupportFscryptV2(mnt) ? FSCRYPT_V2 : FSCRYPT_V1;
    return version;
}

uint8_t KeyCtrl::GetEncryptedVersion(const std::string &dir)
{
    int fd = open(dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno:%{public}d", dir.c_str(), errno);
        return FSCRYPT_INVALID;
    }

    fscrypt_policy_v1 policy;
    if (ioctl(fd, FS_IOC_GET_ENCRYPTION_POLICY, &policy) == 0) {
        LOGI("%{public}s is encrypted with v1 policy", dir.c_str());
        close(fd);
        return FSCRYPT_V1;
    }
    close(fd);

    if (errno == EINVAL) {
        LOGI("%{public}s is encrypted with v2 policy", dir.c_str());
        return FSCRYPT_V2;
    } else if (errno == ENODATA) {
        LOGI("%{public}s is not encrypted", dir.c_str());
    } else {
        LOGE("%{public}s unexpected errno: %{public}d", dir.c_str(), errno);
    }
    return FSCRYPT_INVALID;
}

} // namespace StorageDaemon
} // namespace OHOS
