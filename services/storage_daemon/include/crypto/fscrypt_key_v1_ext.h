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

#ifndef STORAGE_DAEMON_CRYPTO_FSCRYPT_KEY_V1_EXT_H
#define STORAGE_DAEMON_CRYPTO_FSCRYPT_KEY_V1_EXT_H

#include <string>

namespace OHOS {
namespace StorageDaemon {
class FscryptKeyV1Ext {
public:
    FscryptKeyV1Ext() = default;
    ~FscryptKeyV1Ext() = default;
    void SetDir(const std::string &dir)
    {
        dir_ = dir;
        userId_ = GetUserIdFromDir();
        type_ = GetTypeFromDir();
    }
    bool ActiveKeyExt(uint32_t flag, uint8_t *iv, uint32_t size);
    bool InactiveKeyExt(uint32_t flag);

private:
    uint32_t GetUserIdFromDir();
    uint32_t GetTypeFromDir();

    std::string dir_;
    uint32_t userId_ = 0;
    uint32_t type_ = 0;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_KEY_V1_EXT_H