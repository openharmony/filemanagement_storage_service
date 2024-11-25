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

#include "huks_master_mock.h"

using namespace std;
using namespace OHOS::StorageDaemon;

HuksMaster::HuksMaster()
{
}

HuksMaster::~HuksMaster()
{
}

bool HuksMaster::DecryptKey(KeyContext &ctx, const UserAuth &auth, KeyInfo &key, bool isNeedNewNonce)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->DecryptKey(ctx, auth, key, isNeedNewNonce);
}

bool HuksMaster::EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key, bool isNeedNewNonce)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->EncryptKey(ctx, auth, key, isNeedNewNonce);
}

bool HuksMaster::DecryptKeyEx(KeyContext &ctx, const UserAuth &auth, KeyBlob &rnd)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->DecryptKeyEx(ctx, auth, rnd);
}

bool HuksMaster::EncryptKeyEx(const UserAuth &auth, const KeyBlob &rnd, KeyContext &ctx)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->EncryptKeyEx(auth, rnd, ctx);
}

bool HuksMaster::UpgradeKey(KeyContext &ctx)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->UpgradeKey(ctx);
}

bool HuksMaster::GenerateKey(const UserAuth &auth, KeyBlob &keyOut)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->GenerateKey(auth, keyOut);
}

KeyBlob HuksMaster::GenerateRandomKey(uint32_t keyLen)
{
    if (IHuksMaster::huksMasterMock == nullptr) {
        return true;
    }
    return IHuksMaster::huksMasterMock->GenerateRandomKey(keyLen);
}
