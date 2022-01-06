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

#include "huks_master.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <openssl/sha.h>

#include "hks_api.h"
#include "hks_param.h"
#include "utils/log.h"

namespace OHOS {
namespace StorageDaemon {
bool HuksMaster::Init()
{
    return HksInitialize();
}

static bool GenerateRandomKeyLoop(KeyBlob &rawKey)
{
    LOGD("enter");
    uint32_t offset = 0;
    while (offset < rawKey.size) {
        LOGD("off %{public}d", offset);
        HksBlob hksKey = {
            .size = std::min(rawKey.size - offset, static_cast<uint32_t>(HKS_MAX_RANDOM_LEN)),
            .data = rawKey.data.get() + offset,
        };
        auto ret = HksGenerateRandom(nullptr, &hksKey);
        if (ret != HKS_SUCCESS) {
            LOGE("HksGenerateRandom failed ret %{public}d", ret);
            return false;
        }
        offset += HKS_MAX_RANDOM_LEN;
    }
    return true;
}

bool HuksMaster::GenerateRandomKey(KeyBlob &rawKey)
{
    LOGD("enter");
    if (rawKey.IsEmpty()) {
        LOGE("bad key size %{public}d", rawKey.size);
        return false;
    }
    if (rawKey.size > HKS_MAX_RANDOM_LEN) {
        return GenerateRandomKeyLoop(rawKey);
    }

    HksBlob hksKey = {
        .size = rawKey.size,
        .data = rawKey.data.get(),
    };
    auto ret = HksGenerateRandom(nullptr, &hksKey);
    if (ret != HKS_SUCCESS) {
        LOGE("HksGenerateRandom failed ret %{public}d", ret);
        return false;
    }
    LOGD("success");
    return true;
}

bool HuksMaster::GenerateKey(const KeyBlob &keyAlias)
{
    LOGD("enter");
    if (keyAlias.IsEmpty()) {
        LOGE("bad keyAlias input, size %{public}d", keyAlias.size);
        return false;
    }

    HksParamSet *paramSet = nullptr;
    struct HksParam keyParam[] = {
        { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE }
    };
    auto ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksInitParamSet failed ret %{public}d", ret);
        return false;
    }
    ret = HksAddParams(paramSet, keyParam, HKS_ARRAY_SIZE(keyParam));
    if (ret != HKS_SUCCESS) {
        LOGE("HksAddParams failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return false;
    }
    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksBuildParamSet failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return false;
    }

    HksBlob hksAlias = {
        .size = keyAlias.size,
        .data = keyAlias.data.get(),
    };
    ret = HksGenerateKey(&hksAlias, paramSet, nullptr);
    if (ret != HKS_SUCCESS) {
        LOGE("HksGenerateKey failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return false;
    }
    HksFreeParamSet(&paramSet);
    LOGD("success");
    return true;
}

bool HuksMaster::DeleteKey(const KeyBlob &keyAlias)
{
    LOGD("enter");
    if (keyAlias.IsEmpty()) {
        LOGE("bad keyAlias input, size %{public}d", keyAlias.size);
        return false;
    }
    HksBlob hksAlias = {
        .size = keyAlias.size,
        .data = keyAlias.data.get(),
    };
    auto ret = HksDeleteKey(&hksAlias, nullptr);
    if (ret != HKS_SUCCESS) {
        LOGE("HksDeleteKey failed ret %{public}d", ret);
        return false;
    }
    LOGD("success");
    return true;
}

static KeyBlob HashAndClip(const std::string &prefix, const KeyBlob &payload, const uint32_t length)
{
    KeyBlob res;
    res.Alloc(SHA512_DIGEST_LENGTH);
    std::string header = prefix;
    if (header.empty()) {
        header = "dummy SHA512 header";
    }

    SHA512_CTX c;
    SHA512_Init(&c);
    SHA512_Update(&c, header.data(), header.size());
    if (!payload.IsEmpty()) {
        SHA512_Update(&c, payload.data.get(), payload.size);
    }
    SHA512_Final(res.data.get(), &c);

    res.size = length;
    return res;
}

static HksParamSet *GenHksParam(KeyContext &ctx, const UserAuth &auth, const bool isEncrypt)
{
    HksParamSet *paramSet = nullptr;
    struct HksParam encryptParam[] = {
        { .tag = HKS_TAG_KEY_STORAGE_FLAG, .uint32Param = HKS_STORAGE_PERSISTENT },
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_IS_KEY_ALIAS, .boolParam = true },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = isEncrypt ? HKS_KEY_PURPOSE_ENCRYPT : HKS_KEY_PURPOSE_DECRYPT},

    };
    auto ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksInitParamSet failed ret %{public}d", ret);
        return nullptr;
    }
    ret = HksAddParams(paramSet, encryptParam, HKS_ARRAY_SIZE(encryptParam));
    if (ret != HKS_SUCCESS) {
        LOGE("HksAddParams failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    ctx.nonce = HashAndClip("NONCE SHA512 prefix", ctx.secDiscard, HKS_AE_NONCE_LEN);
    if (ctx.nonce.IsEmpty()) {
        HksFreeParamSet(&paramSet);
        return nullptr;
    }
    ctx.aad = HashAndClip("AAD SHA512 prefix", ctx.secDiscard, CRYPTO_AES_AAD_LEN);
    if (ctx.aad.IsEmpty()) {
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    // pass the token here
    struct HksParam addParam[] = {
        { .tag = HKS_TAG_NONCE,
          .blob =
            { ctx.nonce.size, ctx.nonce.data.get() }
        },
        { .tag = HKS_TAG_ASSOCIATED_DATA,
          .blob =
            { ctx.aad.size, ctx.aad.data.get() }
        }
    };
    ret = HksAddParams(paramSet, addParam, HKS_ARRAY_SIZE(addParam));
    if (ret != HKS_SUCCESS) {
        LOGE("HksAddParams failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }
    return paramSet;
}

bool HuksMaster::EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key)
{
    LOGD("enter");
    if (key.keyDesc.IsEmpty()) {
        LOGE("bad keyAlias input, size %{public}d", key.keyDesc.size);
        return false;
    }
    if (key.key.IsEmpty()) {
        LOGE("bad rawKey input, size %{public}d", key.key.size);
        return false;
    }

    HksParamSet *paramSet = GenHksParam(ctx, auth, true);
    if (paramSet == nullptr) {
        return false;
    }

    HksBlob hksAlias = {
        .size = key.keyDesc.size,
        .data = key.keyDesc.data.get(),
    };
    HksBlob hksRawKey = {
        .size = key.key.size,
        .data = key.key.data.get(),
    };
    ctx.encrypted.Alloc(CRYPTO_AES_256_LEN);
    HksBlob hksEncrypted = {
        .size = ctx.encrypted.size,
        .data = ctx.encrypted.data.get(),
    };
    LOGI("alias len:%{public}d, data(hex):%{public}s", key.keyDesc.size, key.keyDesc.ToString().c_str());
    auto ret = HksEncrypt(&hksAlias, paramSet, &hksRawKey, &hksEncrypted);
    if (ret != HKS_SUCCESS) {
        LOGE("HksEncrypt failed ret %{public}d", ret);
        ctx.encrypted.Clear();
        HksFreeParamSet(&paramSet);
        return false;
    }
    // save the encrypted text real length
    ctx.encrypted.size = hksEncrypted.size;
    HksFreeParamSet(&paramSet);
    LOGD("success");
    return true;
}

bool HuksMaster::DecryptKey(KeyContext &ctx, const UserAuth &auth, KeyInfo &key)
{
    LOGD("enter");
    if (key.keyDesc.IsEmpty()) {
        LOGE("bad keyAlias input, size %{public}d", key.keyDesc.size);
        return false;
    }
    if (ctx.encrypted.IsEmpty()) {
        LOGE("bad encrypted input, size %{public}d", ctx.encrypted.size);
        return false;
    }

    HksParamSet *paramSet = GenHksParam(ctx, auth, false);
    if (paramSet == nullptr) {
        return false;
    }

    HksBlob hksAlias = {
        .size = key.keyDesc.size,
        .data = key.keyDesc.data.get(),
    };
    HksBlob hksEncrypted = {
        .size = ctx.encrypted.size,
        .data = ctx.encrypted.data.get(),
    };
    key.key.Alloc(CRYPTO_AES_256_LEN);
    HksBlob hksRawKey = {
        .size = key.key.size,
        .data = key.key.data.get(),
    };
    LOGI("alias len:%{public}d, data(hex):%{public}s", key.keyDesc.size, key.keyDesc.ToString().c_str());
    auto ret = HksDecrypt(&hksAlias, paramSet, &hksEncrypted, &hksRawKey);
    if (ret != HKS_SUCCESS) {
        LOGE("HksDecrypt failed ret %{public}d", ret);
        key.key.Clear();
        HksFreeParamSet(&paramSet);
        return false;
    }
    // restore the plaintext real length
    key.key.size = hksRawKey.size;
    HksFreeParamSet(&paramSet);
    LOGD("success");
    return true;
}
} // namespace StorageDaemon
} // namespace OHOS
