/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "key_blob.h"

#include "huks_hdi.h"

// for new HUKS IDL
#include <mutex>
#ifdef HUKS_IDL_ENVIRONMENT
#include "v1_1/ihuks.h"
#include "v1_1/ihuks_types.h"
#endif

namespace OHOS {
namespace StorageDaemon {
class HuksMaster {
public:
    static HuksMaster &GetInstance()
    {
        static HuksMaster instance;
        return instance;
    }

    /* key operations */
    static KeyBlob GenerateRandomKey(uint32_t keyLen);
    int32_t GenerateKey(const UserAuth &auth, KeyBlob &keyOut);
    int32_t EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key, bool isNeedNewNonce);
    int32_t EncryptKeyEx(const UserAuth &auth, const KeyBlob &rnd, KeyContext &ctx);
    int32_t DecryptKey(KeyContext &ctx, const UserAuth &auth, KeyInfo &key, bool isNeedNewNonce);
    int32_t DecryptKeyEx(KeyContext &ctx, const UserAuth &auth, KeyBlob &rnd);
    bool UpgradeKey(KeyContext &ctx);

private:
    HuksMaster();
    ~HuksMaster();
    HuksMaster(const HuksMaster &) = delete;
    HuksMaster &operator=(const HuksMaster &) = delete;

#ifdef HUKS_IDL_ENVIRONMENT
    int32_t InitHdiProxyInstance();
    void ReleaseHdiProxyInstance();
    int32_t HdiModuleInit();
    int32_t HdiModuleDestroy();
    int HdiGenerateKey(const HuksBlob &keyAlias, const HksParamSet *paramSetIn, HuksBlob &keyOut);
    int HdiAccessInit(const HuksBlob &key, const HksParamSet *paramSet, HuksBlob &handle, HuksBlob &token);
    int HdiAccessFinish(const HuksBlob &handle, const HksParamSet *paramSet, const HuksBlob &inData, HuksBlob &outData);
    int HdiAccessUpgradeKey(const HuksBlob &oldKey, const HksParamSet *paramSet, struct HuksBlob &newKey);
    int HuksHalTripleStage(HksParamSet *paramSet1, const HksParamSet *paramSet2,
                           const KeyBlob &keyIn, KeyBlob &keyOut);

    std::mutex hdiProxyMutex_;
    struct IHuks *hksHdiProxyInstance_ = nullptr;
#endif
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_HUKS_MASTER_H
