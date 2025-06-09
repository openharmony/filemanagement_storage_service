/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_KEY_CONTROL_MOCK_H
#define STORAGE_DAEMON_KEY_CONTROL_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "key_control.h"
#include "fbex.h"

namespace OHOS {
namespace StorageDaemon {
class IKeyControlMoc {
public:
    virtual ~IKeyControlMoc() = default;
public:
    virtual uint8_t KeyCtrlGetFscryptVersion(const char*) = 0;
    virtual long KeyCtrlSearch(key_serial_t, const char*, const char*, key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddKey(const char*, const char*, const key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddKeyEx(const char*, const char*, struct fscrypt_key*, const key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddKeySdp(const char*, const char*, struct EncryptionKeySdp*, const key_serial_t) = 0;
    virtual long KeyCtrlUnlink(key_serial_t, key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddAppAsdpKey(const char*, const char*, struct EncryptAsdpKey*, const key_serial_t) = 0;
#ifdef SUPPORT_FSCRYPT_V2
    virtual bool KeyCtrlInstallKey(const char*, struct fscrypt_add_key_arg*) = 0;
    virtual bool KeyCtrlRemoveKey(const char*, struct fscrypt_remove_key_arg*) = 0;
    virtual bool KeyCtrlGetKeyStatus(const char*, struct fscrypt_get_key_status_arg*) = 0;
    virtual bool KeyCtrlGetPolicyEx(const char*, struct fscrypt_get_policy_ex_arg*) = 0;
#endif
    virtual bool KeyCtrlSetPolicy(const char*, union FscryptPolicy*) = 0;
    virtual bool KeyCtrlGetPolicy(const char*, struct fscrypt_policy*) = 0;
    virtual uint8_t KeyCtrlLoadVersion(const char*) = 0;
public:
    static inline std::shared_ptr<IKeyControlMoc> keyControlMoc = nullptr;
};

class KeyControlMoc : public IKeyControlMoc {
public:
    MOCK_METHOD(uint8_t, KeyCtrlGetFscryptVersion, (const char*));
    MOCK_METHOD(long, KeyCtrlSearch, (key_serial_t, const char*, const char*, key_serial_t));
    MOCK_METHOD(key_serial_t, KeyCtrlAddKey, (const char*, const char*, const key_serial_t));
    MOCK_METHOD(key_serial_t, KeyCtrlAddKeyEx, (const char*, const char*, struct fscrypt_key*, const key_serial_t));
    MOCK_METHOD(key_serial_t, KeyCtrlAddKeySdp, (const char*, const char*, struct EncryptionKeySdp*,
        const key_serial_t));
    MOCK_METHOD(long, KeyCtrlUnlink, (key_serial_t, key_serial_t));
    MOCK_METHOD(key_serial_t, KeyCtrlAddAppAsdpKey, (const char*, const char*, struct EncryptAsdpKey*,
        const key_serial_t));
#ifdef SUPPORT_FSCRYPT_V2
    MOCK_METHOD(bool, KeyCtrlInstallKey, (const char*, struct fscrypt_add_key_arg*));
    MOCK_METHOD(bool, KeyCtrlRemoveKey, (const char*, struct fscrypt_remove_key_arg*));
    MOCK_METHOD(bool, KeyCtrlGetKeyStatus, (const char*, struct fscrypt_get_key_status_arg*));
    MOCK_METHOD(bool, KeyCtrlGetPolicyEx, (const char*, struct fscrypt_get_policy_ex_arg*));
#endif
    MOCK_METHOD(bool, KeyCtrlSetPolicy, (const char*, union FscryptPolicy*));
    MOCK_METHOD(bool, KeyCtrlGetPolicy, (const char*, struct fscrypt_policy*));
    MOCK_METHOD(uint8_t, KeyCtrlLoadVersion, (const char*));
};
}
}
#endif