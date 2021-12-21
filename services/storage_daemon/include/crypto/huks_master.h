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
#ifndef STORAGE_DAEMON_CRYPTO_HUKS_MASTER_H
#define STORAGE_DAEMON_CRYPTO_HUKS_MASTER_H

#include "key_utils.h"

namespace OHOS {
namespace StorageDaemon {
class HuksMaster {
public:
    static bool Init();
    static bool GenerateRandomKey(KeyBlob &rawKey);
    static bool GenerateKey(const KeyBlob &keyAlias);
    static bool DeleteKey(const KeyBlob &keyAlias);

    static bool EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key);
    static bool DecryptKey(const KeyContext &ctx, const UserAuth &auth, KeyInfo &key);
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_HUKS_MASTER_H
