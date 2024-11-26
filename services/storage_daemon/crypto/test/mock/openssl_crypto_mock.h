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
#ifndef STORAGE_DAEMON_OPENSSL_CRYPTO_MOCK_H
#define STORAGE_DAEMON_OPENSSL_CRYPTO_MOCK_H

#include <gmock/gmock.h>

#include "key_blob.h"

namespace OHOS {
namespace StorageDaemon {
class IOpensslCrypto {
public:
    virtual ~IOpensslCrypto() = default;
    virtual bool AESEncrypt(const KeyBlob &, const KeyBlob &, KeyContext &) = 0;
    virtual bool AESDecrypt(const KeyBlob &, KeyContext &, KeyBlob &);
public:
    static inline std::shared_ptr<IOpensslCrypto> opensslCryptoMock = nullptr;
};

class OpensslCryptoMock : public IOpensslCrypto {
public:
    MOCK_METHOD3(AESEncrypt, bool(const KeyBlob &, const KeyBlob &, KeyContext &));
    MOCK_METHOD3(AESDecrypt, bool(const KeyBlob &, KeyContext &, KeyBlob &));
};
}
}
#endif