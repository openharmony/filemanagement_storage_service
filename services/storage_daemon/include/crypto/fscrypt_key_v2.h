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
#ifndef STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV2_H
#define STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV2_H

#include "base_key.h"
#include "libfscrypt/key_control.h"

namespace OHOS {
namespace StorageDaemon {
// for openharmony kernel 5.10+, prefer using the the FSCRYPT_V2.
class FscryptKeyV2 final : public BaseKey {
public:
    FscryptKeyV2() = delete;
    FscryptKeyV2(const std::string &dir, uint8_t keyLen = CRYPTO_AES_256_XTS_KEY_SIZE) : BaseKey(dir, keyLen)
    {
        keyInfo_.version = FSCRYPT_V2;
    }
    ~FscryptKeyV2() = default;

    bool ActiveKey(uint32_t flag = 0, const std::string &mnt = MNT_DATA);
    bool InactiveKey(uint32_t flag = 0, const std::string &mnt = MNT_DATA);
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_KEYV2_H
