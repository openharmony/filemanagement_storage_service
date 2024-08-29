
/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "key_control_mock.h"

using namespace std;
using namespace OHOS::StorageDaemon;

uint8_t KeyCtrlGetFscryptVersion(const char *mnt)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return FSCRYPT_V2;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlGetFscryptVersion(mnt);
}

long KeyCtrlSearch(key_serial_t ringId, const char *type, const char *description,
    key_serial_t destRingId)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return 0;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlSearch(ringId, type, description, destRingId);
}

key_serial_t KeyCtrlAddKey(const char *type, const char *description,
    const key_serial_t ringId)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return 0;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlAddKey(type, description, ringId);
}

key_serial_t KeyCtrlAddKeyEx(const char *type, const char *description,
    struct fscrypt_key *fsKey, const key_serial_t ringId)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return 0;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlAddKeyEx(type, description, fsKey, ringId);
}

key_serial_t KeyCtrlAddKeySdp(const char *type, const char *description,
                              struct EncryptionKeySdp *fsKey, const key_serial_t ringId)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return 0;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlAddKeySdp(type, description, fsKey, ringId);
}

long KeyCtrlUnlink(key_serial_t key, key_serial_t keyring)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return 0;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlUnlink(key, keyring);
}

key_serial_t KeyCtrlAddAppAsdpKey(const char *type,
                                  const char *description,
                                  struct EncryptAsdpKey *fsKey,
                                  const key_serial_t ringId)
{
    if (IKeyControlMoc::keyControlMoc == nullptr) {
        return 0;
    }

    return IKeyControlMoc::keyControlMoc->KeyCtrlAddAppAsdpKey(type, description, fsKey, ringId);
}
