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
#ifndef STORAGE_DAEMON_HUKS_MASTER_MOCK_H
#define STORAGE_DAEMON_HUKS_MASTER_MOCK_H

#include <gmock/gmock.h>

#include "huks_master.h"

namespace OHOS {
namespace StorageDaemon {
class IHuksMaster {
public:
    virtual ~IHuksMaster() = default;
    virtual bool DecryptKey(KeyContext &ctx, const UserAuth &auth, KeyInfo &key, bool isNeedNewNonce) = 0;
    virtual bool EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key, bool isNeedNewNonce) = 0;
    virtual bool DecryptKeyEx(KeyContext &ctx, const UserAuth &auth, KeyBlob &rnd) = 0;
    virtual bool EncryptKeyEx(const UserAuth &auth, const KeyBlob &rnd, KeyContext &ctx);
    virtual bool UpgradeKey(KeyContext &ctx);
    virtual bool GenerateKey(const UserAuth &auth, KeyBlob &keyOut) = 0;
    virtual KeyBlob GenerateRandomKey(uint32_t keyLen) = 0;
public:
    static inline std::shared_ptr<IHuksMaster> huksMasterMock = nullptr;
};

class HuksMasterMock : public IHuksMaster {
public:
    MOCK_METHOD4(DecryptKey, bool(KeyContext &ctx, const UserAuth &auth, KeyInfo &key, bool isNeedNewNonce));
    MOCK_METHOD4(EncryptKey, bool(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key, bool isNeedNewNonce));
    MOCK_METHOD3(DecryptKeyEx, bool(KeyContext &ctx, const UserAuth &auth, KeyBlob &rnd));
    MOCK_METHOD3(EncryptKeyEx, bool(const UserAuth &auth, const KeyBlob &rnd, KeyContext &ctx));
    MOCK_METHOD1(UpgradeKey, bool(KeyContext &ctx));
    MOCK_METHOD2(GenerateKey, bool(const UserAuth &auth, KeyBlob &keyOut));
    MOCK_METHOD1(GenerateRandomKey, KeyBlob(uint32_t keyLen));
};
}
}
#endif