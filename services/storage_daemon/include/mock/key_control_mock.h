/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
    virtual uint8_t KeyCtrlGetFscryptVersion(const char *mnt) = 0;
    virtual long KeyCtrlSearch(key_serial_t, const char *, const char *, key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddKey(const char *, const char *, const key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddKeyEx(const char *, const char *,
        struct fscrypt_key *, const key_serial_t) = 0;
    virtual key_serial_t KeyCtrlAddKeySdp(const char *, const char *,
        struct EncryptionKeySdp *, const key_serial_t) = 0;
    virtual long KeyCtrlUnlink(key_serial_t key, key_serial_t keyring) = 0;
    virtual key_serial_t KeyCtrlAddAppAsdpKey(const char *type,
                                              const char *description,
                                              struct EncryptAsdpKey *fsKey,
                                              const key_serial_t ringId) = 0;
public:
    static inline std::shared_ptr<IKeyControlMoc> keyControlMoc = nullptr;
};

class KeyControlMoc : public IKeyControlMoc {
public:
    MOCK_METHOD1(KeyCtrlGetFscryptVersion, uint8_t(const char *mnt));
    MOCK_METHOD4(KeyCtrlSearch, long(key_serial_t, const char *, const char *, key_serial_t));
    MOCK_METHOD3(KeyCtrlAddKey, key_serial_t(const char *, const char *, const key_serial_t));
    MOCK_METHOD4(KeyCtrlAddKeyEx, key_serial_t(const char *, const char *,
        struct fscrypt_key *, const key_serial_t));
    MOCK_METHOD4(KeyCtrlAddKeySdp, key_serial_t(const char *, const char *,
        struct EncryptionKeySdp *, const key_serial_t));
    MOCK_METHOD2(KeyCtrlUnlink, long(key_serial_t key, key_serial_t keyring));
    MOCK_METHOD4(KeyCtrlAddAppAsdpKey, key_serial_t(const char *type, const char *description,
        struct EncryptAsdpKey *fsKey, const key_serial_t ringId));
};
}
}
#endif