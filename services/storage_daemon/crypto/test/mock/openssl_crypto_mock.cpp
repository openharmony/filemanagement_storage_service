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

#include "openssl_crypto_mock.h"

#include "openssl_crypto.h"
#include "storage_service_errno.h"

using namespace std;
using namespace OHOS::StorageDaemon;

int32_t OpensslCrypto::AESDecrypt(const KeyBlob &preKey, KeyContext &keyContext_, KeyBlob &plainText)
{
    if (IOpensslCrypto::opensslCryptoMock == nullptr) {
        return E_OK;
    }
    return IOpensslCrypto::opensslCryptoMock->AESDecrypt(preKey, keyContext_, plainText);
}

int32_t OpensslCrypto::AESEncrypt(const KeyBlob &preKey, const KeyBlob &plainText, KeyContext &keyContext_)
{
    if (IOpensslCrypto::opensslCryptoMock == nullptr) {
        return E_OK;
    }
    return IOpensslCrypto::opensslCryptoMock->AESEncrypt(preKey, plainText, keyContext_);
}

KeyBlob OpensslCrypto::HashWithPrefix(const KeyBlob &prefix, const KeyBlob &payload, uint32_t length)
{
    if (IOpensslCrypto::opensslCryptoMock == nullptr) {
        return {};
    }
    return IOpensslCrypto::opensslCryptoMock->HashWithPrefix(prefix, payload, length);    
}
