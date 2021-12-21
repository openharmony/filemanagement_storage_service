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
#ifndef STORAGE_DAEMON_CRYPTO_KEYCTL_H_
#define STORAGE_DAEMON_CRYPTO_KEYCTL_H_
#include <unistd.h>
#include <string>
#include <linux/fs.h>
#include <linux/keyctl.h>

namespace OHOS {
namespace StorageDaemon {
using key_serial_t = int;
class KeyCtrl {
public:
    static key_serial_t AddKey(const std::string &type, const std::string &description, const key_serial_t ringId);

    static key_serial_t AddKey(const std::string &type, const std::string &description, fscrypt_key &fsKey,
        const key_serial_t ringId);

    static key_serial_t GetKeyring(key_serial_t id, int create);

    static long Revoke(key_serial_t id);

    static long Search(key_serial_t ringId, const std::string &type, const std::string &description,
        key_serial_t destRingId);

    static long SetPermission(key_serial_t id, int permissions);

    static long Unlink(key_serial_t key, key_serial_t keyring);

    static long RestrictKeyring(key_serial_t keyring, const std::string &type, const std::string &restriction);

    static long GetSecurity(key_serial_t key, std::string &buffer);
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYCTL_H_
