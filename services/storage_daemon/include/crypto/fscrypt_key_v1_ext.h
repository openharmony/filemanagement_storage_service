/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "key_blob.h"

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
    int32_t ActiveKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken);
    int32_t ActiveDoubleKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken);
    int32_t InactiveKeyExt(uint32_t flag);
    int32_t LockUserScreenExt(uint32_t flag, uint32_t &elType);
    int32_t UnlockUserScreenExt(uint32_t flag, uint8_t *iv, uint32_t size, const KeyBlob &authToken);
    int32_t AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status);
    int32_t DeleteClassEPinCode(uint32_t userId);
    int32_t ChangePinCodeClassE(uint32_t userId, bool &isFbeSupport);
    int32_t ReadClassE(uint32_t status, KeyBlob &classEBuffer, const KeyBlob &authToken, bool &isFbeSupport);
    int32_t WriteClassE(uint32_t status, uint8_t *classEBuffer, uint32_t length);
    int32_t GenerateAppkey(uint32_t userId, uint32_t appUid, std::unique_ptr<uint8_t[]> &keyId, uint32_t size);
    int32_t LockUeceExt(bool &isFbeSupport);

private:
    uint32_t GetUserIdFromDir();
    uint32_t GetTypeFromDir();
    uint32_t GetMappedUserId(uint32_t userId, uint32_t type);
    uint32_t GetMappedDeUserId(uint32_t userId);

    std::string dir_;
    uint32_t userId_ = 0;
    uint32_t type_ = 0;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_KEY_V1_EXT_H