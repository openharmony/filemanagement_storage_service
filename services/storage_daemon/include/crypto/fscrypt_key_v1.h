/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV1_H
#define STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV1_H

#include "base_key.h"
#include "fscrypt_key_v1_ext.h"
#include "libfscrypt/key_control.h"

namespace OHOS {
namespace StorageDaemon {
class FscryptKeyV1 final : public BaseKey {
public:
    FscryptKeyV1() = delete;
    FscryptKeyV1(const std::string &dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE) : BaseKey(dir, keyLen)
    {
        keyInfo_.version = FSCRYPT_V1;
        fscryptV1Ext.SetDir(dir);
    }
    ~FscryptKeyV1() = default;

    bool ActiveKey(uint32_t flag = 0, const std::string &mnt = MNT_DATA);
    bool InactiveKey(uint32_t flag = 0, const std::string &mnt = MNT_DATA);

private:
    bool GenerateKeyDesc();
    bool InstallKeyToKeyring();
    bool UninstallKeyToKeyring();
    FscryptKeyV1Ext fscryptV1Ext;
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV1_H
