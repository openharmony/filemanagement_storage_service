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
#include <cerrno>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/keyctl.h>

#include "storage_service_log.h"
#include "securec.h"
#include "file_ex.h"
#include "string_ex.h"
#include "parameter.h"

namespace {
constexpr uint32_t INDEX_FSCRYPT_VERSION = 0;
constexpr uint32_t INDEX_FSCRYPT_FILENAME = 1;
constexpr uint32_t INDEX_FSCRYPT_CONTENT = 2;
}

namespace OHOS {
namespace StorageDaemon {
struct EncryptPolicy g_policyOption = DEFAULT_POLICY;
bool g_fscryptEnable = false;
const std::string FSCRYPT_POLICY_KEY = "fscrypt.policy.config";
constexpr uint32_t FSCRYPT_POLICY_BUFFER_SIZE = 100;

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

std::string CheckRealPath(const std::string &path)
{
    std::string realp(PATH_MAX + 1, '\0');
    if (path.length() > PATH_MAX || realpath(path.c_str(), realp.data()) == nullptr) {
        LOGE("realpath failed, path: %{public}s, errno: %{public}d", path.c_str(), errno);
        return "";
    }
    return realp;
}

bool FsIoctl(const std::string &mnt, unsigned long cmd, void *arg)
{
    std::string realp = CheckRealPath(mnt);
    int fd = open(realp.c_str(), O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno:%{public}d", realp.c_str(), errno);
        return false;
    }
    if (ioctl(fd, cmd, arg) != 0) {
        LOGE("ioctl to %{public}s failed, errno:%{public}d", realp.c_str(), errno);
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

bool KeyCtrl::GetPolicy(const std::string &path, fscrypt_policy &policy)
{
    LOGD("enter");
    return FsIoctl(path, FS_IOC_GET_ENCRYPTION_POLICY, reinterpret_cast<void *>(&policy));
}

bool KeyCtrl::GetPolicy(const std::string &path, fscrypt_get_policy_ex_arg &policy)
{
    LOGD("enter");
    return FsIoctl(path, FS_IOC_GET_ENCRYPTION_POLICY_EX, reinterpret_cast<void *>(&policy));
}

static bool ParseOption(const std::map<std::string, uint8_t> &policy, const std::string &option, uint8_t &value)
{
    if (policy.find(option) != policy.end()) {
        LOGI("use option: %{public}s", option.c_str());
        value = policy.at(option);
        return true;
    }

    LOGE("bad option: %{public}s, value not filled!", option.c_str());
    return false;
}

static bool SetPolicyLegacy(const std::string &keyDescPath, const std::string &toEncrypt, FscryptPolicy &arg)
{
    std::string keyDesc;
    if (OHOS::LoadStringFromFile(keyDescPath, keyDesc) == false || keyDesc.length() != FSCRYPT_KEY_DESCRIPTOR_SIZE) {
        LOGE("bad key_desc file content, length=%{public}u", static_cast<uint32_t>(keyDesc.length()));
        return false;
    }

    arg.v1.version = FSCRYPT_POLICY_V1;
    auto ret = memcpy_s(arg.v1.master_key_descriptor, FSCRYPT_KEY_DESCRIPTOR_SIZE, keyDesc.data(),
        keyDesc.length());
    if (ret) {
        LOGE("memcpy_s failed ret %{public}d", ret);
        return false;
    }
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

bool KeyCtrl::LoadAndSetPolicy(const std::string &keyPath, const std::string &toEncrypt)
{
    LOGD("enter");

    FscryptPolicy arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    // the modes and flags shares the same offset in the struct
    ParseOption(FILENAME_MODES, g_policyOption.fileName, arg.v1.filenames_encryption_mode);
    ParseOption(CONTENTS_MODES, g_policyOption.content, arg.v1.contents_encryption_mode);
    ParseOption(POLICY_FLAGS, g_policyOption.flags, arg.v1.flags);

    auto ver = LoadVersion(keyPath);
    if (ver == FSCRYPT_V1) {
        return SetPolicyLegacy(keyPath + PATH_KEYDESC, toEncrypt, arg);
    } else if (ver == FSCRYPT_V2) {
        return SetPolicyV2(keyPath + PATH_KEYID, toEncrypt, arg);
    }
    LOGE("SetPolicy fail, unknown version");
    return false;
}

uint8_t KeyCtrl::LoadVersion(const std::string &keyPath)
{
    std::string buf;
    int ver = 0;
    auto path = keyPath + PATH_FSCRYPT_VER;
    if (!OHOS::LoadStringFromFile(path, buf)) {
        LOGE("load version from %{public}s failed", path.c_str());
        return FSCRYPT_INVALID;
    }
    if (IsNumericStr(buf) && StrToInt(buf, ver) && (ver == FSCRYPT_V1 || ver == FSCRYPT_V2)) {
        LOGD("version %{public}d loaded", ver);
        return ver;
    }

    LOGE("bad version content: %{public}s", buf.c_str());
    return FSCRYPT_INVALID;
}

static uint8_t CheckKernelFscrypt(const std::string &mnt)
{
    std::string realp = CheckRealPath(mnt);
    int fd = open(realp.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno: %{public}d", realp.c_str(), errno);
        return FSCRYPT_INVALID;
    }

    errno = 0;
    (void)ioctl(fd, FS_IOC_ADD_ENCRYPTION_KEY, nullptr);
    close(fd);
    if (errno == EOPNOTSUPP) {
        LOGE("Kernel doesn't support fscrypt v1 or v2.");
        return FSCRYPT_INVALID;
    } else if (errno == ENOTTY) {
        LOGE("Kernel doesn't support fscrypt v2, pls use v1.");
        return FSCRYPT_V1;
    } else if (errno == EFAULT) {
        LOGI("Kernel is support fscrypt v2.");
        return FSCRYPT_V2;
    }
    LOGW("Unexpected errno: %{public}d", errno);
    return FSCRYPT_INVALID;
}

uint8_t KeyCtrl::GetFscryptVersion(const std::string &mnt)
{
    static auto version = CheckKernelFscrypt(mnt);
    return version;
}

uint8_t KeyCtrl::GetEncryptedVersion(const std::string &dir)
{
    std::string realp = CheckRealPath(dir);
    int fd = open(realp.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno:%{public}d", realp.c_str(), errno);
        return FSCRYPT_INVALID;
    }

    fscrypt_policy_v1 policy;
    if (ioctl(fd, FS_IOC_GET_ENCRYPTION_POLICY, &policy) == 0) {
        LOGI("%{public}s is encrypted with v1 policy", realp.c_str());
        close(fd);
        return FSCRYPT_V1;
    }
    close(fd);

    if (errno == EINVAL) {
        LOGI("%{public}s is encrypted with v2 policy", realp.c_str());
        return FSCRYPT_V2;
    } else if (errno == ENODATA) {
        LOGI("%{public}s is not encrypted", realp.c_str());
    } else {
        LOGE("%{public}s unexpected errno: %{public}d", realp.c_str(), errno);
    }
    return FSCRYPT_INVALID;
}

int32_t KeyCtrl::InitFscryptPolicy()
{
    LOGD("enter");
    char tmp[FSCRYPT_POLICY_BUFFER_SIZE] = { 0 };
    int ret = GetParameter(FSCRYPT_POLICY_KEY.c_str(), "", tmp, FSCRYPT_POLICY_BUFFER_SIZE);
    if (ret < 0) {
        LOGI("fscrypt config parameter not set, not enable fscrypt");
        g_fscryptEnable = false;
        return -ENOTSUP;
    }

    std::string config(tmp);
    std::vector<std::string> strs;
    std::string sep = ":";
    LOGI("enable fscrypt policy from config: %{public}s", config.c_str());
    SplitStr(config, sep, strs, false, false);
    if (strs.size() != FSCRYPT_OPTIONS_TABLE.size()) {
        LOGI("bad policy size, just use the default policy");
        g_fscryptEnable = true;
        g_policyOption = DEFAULT_POLICY;
        return 0;
    }

    for (size_t index = 0; index < strs.size(); index++) {
        auto item = FSCRYPT_OPTIONS_TABLE[index];
        if (item.find(strs[index]) == item.end()) {
            LOGI("bad policy option %{public}s, just use the default policy", strs[index].c_str());
            g_fscryptEnable = true;
            g_policyOption = DEFAULT_POLICY;
            return 0;
        }
    }
    g_policyOption.version = strs[INDEX_FSCRYPT_VERSION];
    g_policyOption.fileName = strs[INDEX_FSCRYPT_FILENAME];
    g_policyOption.content = strs[INDEX_FSCRYPT_CONTENT];
    g_fscryptEnable = true;
    LOGI("fscrypt policy init success");

    return 0;
}

int32_t KeyCtrl::SetFscryptSyspara(const std::string &config)
{
    LOGD("start set parameter: %{public}s", config.c_str());
    std::vector<std::string> strs;
    std::string sep = ":";
    SplitStr(config, sep, strs, false, false);
    if (strs.size() != FSCRYPT_OPTIONS_TABLE.size()) {
        LOGE("bad policy size, not to set the parameter");
        return -EINVAL;
    }

    for (size_t index = 0; index < strs.size(); index++) {
        auto item = FSCRYPT_OPTIONS_TABLE[index];
        if (item.find(strs[index]) == item.end()) {
            LOGE("bad policy option %{public}s, not to set the parameter", strs[index].c_str());
            return -EINVAL;
        }
    }

    int ret = SetParameter(FSCRYPT_POLICY_KEY.c_str(), config.c_str());
    if (ret < 0) {
        LOGE("SetParameter failed ret = %{public}d", ret);
        return -ENOTSUP;
    }
    LOGI("fscrypt config parameter set success");

    return 0;
}

bool KeyCtrl::HasFscryptSyspara()
{
    LOGD("enter");
    char tmp[FSCRYPT_POLICY_BUFFER_SIZE] = { 0 };
    int ret = GetParameter(FSCRYPT_POLICY_KEY.c_str(), "", tmp, FSCRYPT_POLICY_BUFFER_SIZE);
    if (ret <= 0) {
        LOGD("fscrypt config parameter not set, not enable fscrypt");
        return false;
    }

    return true;
}
} // namespace StorageDaemon
} // namespace OHOS
