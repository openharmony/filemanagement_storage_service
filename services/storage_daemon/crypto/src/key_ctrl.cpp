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

#include <sys/syscall.h>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/keyctl.h>

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
} // namespace StorageDaemon
} // namespace OHOS
