/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_BASEKEY_H
#define STORAGE_DAEMON_CRYPTO_BASEKEY_H

#include <string>

#include "key_utils.h"
#include "key_ctrl.h"

namespace OHOS {
namespace StorageDaemon {
class BaseKey {
public:
    BaseKey() = delete;
    BaseKey(std::string dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE);
    ~BaseKey() = default;

    bool InitKey();
    bool StoreKey(const UserAuth &auth);
    bool UpdateKey();
    bool RestoreKey(const UserAuth &auth);
    virtual bool ActiveKey(const std::string &mnt = MNT_DATA) = 0;
    virtual bool InactiveKey(const std::string &mnt = MNT_DATA) = 0;
    bool ClearKey(const std::string &mnt = MNT_DATA);

    KeyInfo keyInfo_;
    std::string GetDir() const
    {
        return dir_;
    }

protected:
    bool SaveKeyBlob(const KeyBlob &blob, const std::string &name);

private:
    bool DoStoreKey(const UserAuth &auth);
    bool GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &name, const uint32_t size);
    bool GenerateKeyBlob(KeyBlob &blob, const uint32_t size);
    bool LoadKeyBlob(KeyBlob &blob, const std::string &name, const uint32_t size);
    bool EncryptKey(const UserAuth &auth);
    bool DecryptKey(const UserAuth &auth);
    bool RemoveAlias(const std::string& dir);
    std::string GetCandidateDir() const;

    KeyContext keyContext_ {};
    std::string dir_ {};
    uint8_t keyLen_ {};
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_BASEKEY_H
