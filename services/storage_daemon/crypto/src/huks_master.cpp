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

#include "huks_master.h"

#include <iomanip>
#include <sstream>

#include <unistd.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include "key_crypto_utils.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr uint8_t MAX_RETRY_TIME = 3;
constexpr uint16_t RETRY_INTERVAL_MS = 50 * 1000;
constexpr uint32_t CRYPTO_KEY_ALIAS_SIZE = 16;
constexpr uint32_t CRYPTO_AES_AAD_LEN = 16;
constexpr uint32_t CRYPTO_AES_NONCE_LEN = 64;
constexpr uint32_t CRYPTO_HKS_NONCE_LEN = 12;
constexpr uint32_t CRYPTO_KEY_SHIELD_MAX_SIZE = 2048;
constexpr uint32_t CRYPTO_AES_256_KEY_ENCRYPTED_SIZE = 80;
constexpr uint32_t CRYPTO_TOKEN_SIZE = TOKEN_CHALLENGE_LEN; // 32
constexpr uint32_t DEFAULT_PREFIX_SIZE_FOR_KEY_BLOB = 32;

HuksMaster::HuksMaster()
{
    LOGI("enter");
    InitHdiProxyInstance();
    HdiModuleInit();
    LOGI("finish");
}

HuksMaster::~HuksMaster()
{
    LOGI("enter");
    HdiModuleDestroy();
    ReleaseHdiProxyInstance();
    LOGI("finish");
}


int32_t HuksMaster::InitHdiProxyInstance()
{
    LOGI("enter");
    if (hksHdiProxyInstance_ != nullptr) {
        return HKS_SUCCESS;
    }
    std::lock_guard<std::mutex> lock(hdiProxyMutex_);
    if (hksHdiProxyInstance_ != nullptr) {
        return HKS_SUCCESS;
    }
    hksHdiProxyInstance_ = IHuksGetInstance("hdi_service", true);
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("IHuksGet hdi huks service failed");
        return HKS_ERROR_NULL_POINTER;
    }
    LOGI("success");
    return HKS_SUCCESS;
}

void HuksMaster::ReleaseHdiProxyInstance()
{
    LOGI("enter");
    if (hksHdiProxyInstance_ != nullptr) {
        IHuksReleaseInstance("hdi_service", hksHdiProxyInstance_, true);
    }
    hksHdiProxyInstance_ = nullptr;
    LOGI("finish");
}

int HuksMaster::HdiModuleInit()
{
    LOGI("enter");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->ModuleInit == nullptr) {
        LOGE("HuksHdiModuleInit is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    auto ret = hksHdiProxyInstance_->ModuleInit(hksHdiProxyInstance_);
    if (ret == HKS_SUCCESS) {
        LOGI("HuksHdiModuleInit success, ret %{public}d", ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("HuksHdiModuleInit failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdiModuleInit", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->ModuleInit(hksHdiProxyInstance_);
        LOGE("HuksHdiModuleInit has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("HuksHdiModuleInit end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiModuleInit_Retry", retryRet);
    }
    return retryRet;
}

int HuksMaster::HdiModuleDestroy()
{
    LOGI("enter");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->ModuleDestroy == nullptr) {
        LOGE("HuksHdiModuleDestroy is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    auto ret = hksHdiProxyInstance_->ModuleDestroy(hksHdiProxyInstance_);
    if (ret == HKS_SUCCESS) {
        LOGI("HuksHdiModuleDestroy success, ret %{public}d", ret);
        return ret;
    }
    
    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("HuksHdiModuleDestroy failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdiModuleDestroy", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->ModuleDestroy(hksHdiProxyInstance_);
        LOGE("HuksHdiModuleDestroy has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("HuksHdiModuleDestroy end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiModuleDestroy_Retry", retryRet);
    }
    return retryRet;
}

int HuksMaster::HdiGenerateKey(const HuksBlob &keyAlias, const HksParamSet *paramSetIn, HuksBlob &keyOut)
{
    LOGI("enter");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->GenerateKey == nullptr) {
        LOGE("HuksHdiGenerateKey is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    uint8_t keyData = 0;
    struct HuksBlob keyIn = {&keyData, 1};
    struct HuksParamSet hksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSetIn, hksParamSet);
    auto ret = hksHdiProxyInstance_->GenerateKey(hksHdiProxyInstance_, &keyAlias, &hksParamSet, &keyIn, &keyOut);
    if (ret == HKS_SUCCESS) {
        LOGI("HuksHdiGenerateKey success, ret %{public}d", ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("HuksHdiGenerateKey failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdi GenerateKey", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->GenerateKey(hksHdiProxyInstance_, &keyAlias, &hksParamSet, &keyIn, &keyOut);
        LOGE("HuksHdiGenerateKey has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("HuksHdiGenerateKey end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdi GenerateKey_Retry", retryRet);
    }
    return retryRet;
}

int HuksMaster::HdiAccessInit(const HuksBlob &key, const HksParamSet *paramSet, HuksBlob &handle, HuksBlob &token)
{
    LOGI("enter");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->Init == nullptr) {
        LOGE("HuksHdiInit is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HuksParamSet huksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSet, huksParamSet);
    auto ret = hksHdiProxyInstance_->Init(hksHdiProxyInstance_, &key, &huksParamSet, &handle, &token);
    if (ret == HKS_SUCCESS) {
        LOGI("HuksHdiInit success, ret %{public}d", ret);
        return ret;
    }
    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("HuksHdiInit failed, ret %{public}d", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->Init(hksHdiProxyInstance_, &key, &huksParamSet, &handle, &token);
        LOGE("HuksHdiInit has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("HuksHdiInit end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiInit_Retry", retryRet);
    }
    return retryRet;
}

int HuksMaster::HdiAccessFinish(const HuksBlob &handle, const HksParamSet *paramSet,
                                const HuksBlob &inData, HuksBlob &outData)
{
    LOGI("enter");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->Finish == nullptr) {
        LOGE("HuksHdiFinish is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HuksParamSet huksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSet, huksParamSet);
    auto ret = hksHdiProxyInstance_->Finish(hksHdiProxyInstance_, &handle, &huksParamSet, &inData, &outData);
    if (ret == HKS_SUCCESS) {
        LOGI("HuksHdiFinish success, ret %{public}d", ret);
        return ret;
    }
    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("HuksHdiFinish failed, ret %{public}d", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->Finish(hksHdiProxyInstance_, &handle, &huksParamSet, &inData, &outData);
        LOGE("HuksHdiFinish has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("HuksHdiFinish end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiFinish_Retry", retryRet);
    }
    return retryRet;
}

int HuksMaster::HdiAccessUpgradeKey(const HuksBlob &oldKey, const HksParamSet *paramSet, struct HuksBlob &newKey)
{
    LOGI("enter");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->UpgradeKey == nullptr) {
        LOGE("HuksHdiUpgradeKey is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HuksParamSet huksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSet, huksParamSet);
    auto ret = hksHdiProxyInstance_->UpgradeKey(hksHdiProxyInstance_, &oldKey, &huksParamSet, &newKey);
    if (ret == HKS_SUCCESS) {
        LOGI("HuksHdiUpgradeKey success, ret %{public}d", ret);
        return ret;
    }
    
    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("HuksHdiUpgradeKey failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdiUpgradeKey", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->UpgradeKey(hksHdiProxyInstance_, &oldKey, &huksParamSet, &newKey);
        LOGE("HuksHdiUpgradeKey has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("HuksHdiUpgradeKey end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdi UpgradeKey_Retry", retryRet);
    }
    return retryRet;
}

KeyBlob HuksMaster::GenerateRandomKey(uint32_t keyLen)
{
    LOGI("enter, size %{public}d", keyLen);
    KeyBlob out(keyLen);
    if (out.IsEmpty()) {
        return out;
    }

    auto ret = RAND_bytes(out.data.get(), out.size);
    if (ret <= 0) {
        LOGE("RAND_bytes failed return %{public}d, errno %{public}lu", ret, ERR_get_error());
        out.Clear();
    }
    return out;
}

static int AppendSecureAccessParams(const UserAuth &auth, HksParamSet *paramSet)
{
    if (auth.secret.IsEmpty() || auth.token.IsEmpty()) {
        LOGI("auth is empty, not to enable secure access for the key");
        return HKS_SUCCESS;
    }

    LOGI("append the secure access params when generate key");

    HksParam param[] = {
        { .tag = HKS_TAG_USER_AUTH_TYPE,
            .uint32Param = HKS_USER_AUTH_TYPE_PIN | HKS_USER_AUTH_TYPE_FACE | HKS_USER_AUTH_TYPE_FINGERPRINT },
        { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
        { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NONE },
        { .tag = HKS_TAG_USER_AUTH_SECURE_UID, .blob = { sizeof(auth.secureUid), (uint8_t *)&auth.secureUid } },
        { .tag = HKS_TAG_AUTH_TIMEOUT, .uint32Param = 30 } // token timeout is 30 seconds when no challenge
    };
    return HksAddParams(paramSet, param, HKS_ARRAY_SIZE(param));
}

static uint8_t g_processName[sizeof(uint32_t)] = {0};
static const HksParam g_generateKeyParam[] = {
    { .tag = HKS_TAG_KEY_GENERATE_TYPE, .uint32Param = HKS_KEY_GENERATE_TYPE_DEFAULT },
    { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
    { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
    { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
    { .tag = HKS_TAG_DIGEST, .uint32Param = HKS_DIGEST_SHA256 },
    { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
    { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
    { .tag = HKS_TAG_PROCESS_NAME,
      .blob =
        { sizeof(g_processName), g_processName }
    },
};

int32_t HuksMaster::GenerateKey(const UserAuth &auth, KeyBlob &keyOut)
{
    LOGI("enter");

    HksParamSet *paramSet = nullptr;
    int ret = HKS_SUCCESS;
    do {
        ret = HksInitParamSet(&paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("HksInitParamSet failed ret %{public}d", ret);
            break;
        }
        ret = HksAddParams(paramSet, g_generateKeyParam, HKS_ARRAY_SIZE(g_generateKeyParam));
        if (ret != HKS_SUCCESS) {
            LOGE("HksAddParams failed ret %{public}d", ret);
            break;
        }
        ret = AppendSecureAccessParams(auth, paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("AppendSecureAccessParams failed ret %{public}d", ret);
            break;
        }
        ret = HksBuildParamSet(&paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("HksBuildParamSet failed ret %{public}d", ret);
            break;
        }
        KeyBlob alias = GenerateRandomKey(CRYPTO_KEY_ALIAS_SIZE);
        HuksBlob hksAlias = alias.ToHuksBlob();
        keyOut.Alloc(CRYPTO_KEY_SHIELD_MAX_SIZE);
        HuksBlob hksKeyOut = keyOut.ToHuksBlob();
        ret = HdiGenerateKey(hksAlias, paramSet, hksKeyOut);
        if (ret != HKS_SUCCESS) {
            LOGE("HdiGenerateKey failed ret %{public}d", ret);
            break;
        }
        keyOut.size = hksKeyOut.dataLen;
        LOGI("HdiGenerateKey success, out size %{public}d", keyOut.size);
    } while (0);

    HksFreeParamSet(&paramSet);
    return ret;
}

static KeyBlob HashWithPrefix(const std::string &prefix, const KeyBlob &payload, uint32_t length)
{
    KeyBlob res(SHA512_DIGEST_LENGTH);
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

static HksParamSet *GenHuksKeyBlobParam(KeyContext &ctx)
{
    return reinterpret_cast<HksParamSet *>(ctx.shield.data.get());
}

static int AppendAeTag(KeyBlob &cipherText, HksParamSet *paramSet)
{
    if (cipherText.size <= HKS_AE_TAG_LEN) {
        LOGE("cipherText size %{public}d is too small", cipherText.size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }
    if (cipherText.data.get() == nullptr) {
        LOGE("cipherText data pointer is null");
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    HksParam param[] = {
        { .tag = HKS_TAG_AE_TAG,
          .blob =
            { HKS_AE_TAG_LEN, cipherText.data.get() + cipherText.size - HKS_AE_TAG_LEN }
        },
    };
    cipherText.size -= HKS_AE_TAG_LEN;
    return HksAddParams(paramSet, param, HKS_ARRAY_SIZE(param));
}

static int AppendNonceAad(KeyContext &ctx, HksParamSet *paramSet)
{
    ctx.nonce = HashWithPrefix("NONCE SHA512 prefix", ctx.secDiscard, CRYPTO_AES_NONCE_LEN);
    ctx.aad = HashWithPrefix("AAD SHA512 prefix", ctx.secDiscard, CRYPTO_AES_AAD_LEN);
    HksParam addParam[] = {
        { .tag = HKS_TAG_NONCE, .blob = { ctx.nonce.size, ctx.nonce.data.get() } },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = { ctx.aad.size, ctx.aad.data.get() } }
    };
    return HksAddParams(paramSet, addParam, HKS_ARRAY_SIZE(addParam));
}

static int AppendNonceAadTokenEx(KeyContext &ctx, const UserAuth &auth, HksParamSet *paramSet, bool isEncrypt)
{
    if (auth.secret.IsEmpty() || auth.token.IsEmpty()) {
        LOGI("auth is empty, not to use secure access tag");
        HksParam addParam[] = {
            { .tag = HKS_TAG_NONCE, .blob = { ctx.nonce.size, ctx.nonce.data.get() } },
            { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = { ctx.aad.size, ctx.aad.data.get() } }
        };
        return HksAddParams(paramSet, addParam, HKS_ARRAY_SIZE(addParam));
    }

    HksParam addParam[] = {
        { .tag = HKS_TAG_NONCE, .blob = { ctx.nonce.size, ctx.nonce.data.get() } },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = { ctx.aad.size, ctx.aad.data.get() } },
        { .tag = HKS_TAG_AUTH_TOKEN, .blob = { auth.token.size, auth.token.data.get() } }
    };
    return HksAddParams(paramSet, addParam, HKS_ARRAY_SIZE(addParam));
}

static int AppendNewNonceAadToken(KeyContext &ctx, const UserAuth &auth, HksParamSet *paramSet, const bool isEncrypt)
{
    LOGI("append the secure access params when encrypt/decrypt");
    if (isEncrypt) {
        ctx.nonce = HuksMaster::GenerateRandomKey(CRYPTO_HKS_NONCE_LEN);
        LOGI("Encrypt generate new nonce size: %{public}d", ctx.nonce.size);
    }
    ctx.aad = HashWithPrefix("AAD SHA512 prefix", ctx.secDiscard, CRYPTO_AES_AAD_LEN);
    LOGI("secret/token is empty : %{public}d / %{public}d", auth.secret.IsEmpty(), auth.token.IsEmpty());
    if (auth.secret.IsEmpty() && auth.token.IsEmpty()) {
        LOGI("token & secret is empty, Only append nonce & aad!");
        return AppendNonceAad(ctx, paramSet);
    }
    HksParam addParam[] = {
        { .tag = HKS_TAG_NONCE, .blob = { ctx.nonce.size, ctx.nonce.data.get() } },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = {ctx.aad.size, ctx.aad.data.get() } },
        { .tag = HKS_TAG_AUTH_TOKEN, .blob = {auth.token.size, auth.token.data.get() } }
    };
    return HksAddParams(paramSet, addParam, HKS_ARRAY_SIZE(addParam));
}

static int AppendNonceAadToken(KeyContext &ctx, const UserAuth &auth, HksParamSet *paramSet)
{
    if (auth.secret.IsEmpty() || auth.token.IsEmpty()) {
        LOGI("auth is empty, not to use secure access tag");
        return AppendNonceAad(ctx, paramSet);
    }

    LOGI("append the secure access params when encrypt/decrypt");
    ctx.nonce = HashWithPrefix("NONCE SHA512 prefix", auth.secret, CRYPTO_AES_NONCE_LEN);
    ctx.aad = HashWithPrefix("AAD SHA512 prefix", ctx.secDiscard, CRYPTO_AES_AAD_LEN);
    HksParam addParam[] = {
        { .tag = HKS_TAG_USER_AUTH_TYPE, .uint32Param = HKS_USER_AUTH_TYPE_PIN },
        { .tag = HKS_TAG_KEY_AUTH_ACCESS_TYPE, .uint32Param = HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD },
        { .tag = HKS_TAG_NONCE,
          .blob =
            { ctx.nonce.size, ctx.nonce.data.get() }
        },
        { .tag = HKS_TAG_ASSOCIATED_DATA,
          .blob =
            { ctx.aad.size, ctx.aad.data.get() }
        },
        { .tag = HKS_TAG_AUTH_TOKEN,
          .blob =
            { auth.token.size, auth.token.data.get() }
        }
    };
    return HksAddParams(paramSet, addParam, HKS_ARRAY_SIZE(addParam));
}

static HksParamSet *GenHuksOptionParamEx(KeyContext &ctx, const UserAuth &auth, const bool isEncrypt)
{
    HksParam encryptParam[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_IS_KEY_ALIAS, .boolParam = false },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = isEncrypt ? HKS_KEY_PURPOSE_ENCRYPT : HKS_KEY_PURPOSE_DECRYPT},
        { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NONE },
        { .tag = HKS_TAG_PROCESS_NAME, .blob = { sizeof(g_processName), g_processName } }
    };

    HksParamSet *paramSet = nullptr;
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

    if (!isEncrypt) {
        ret = AppendAeTag(ctx.rndEnc, paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("AppendAeTag failed ret %{public}d", ret);
            HksFreeParamSet(&paramSet);
            return nullptr;
        }
    }

    ret = AppendNonceAadTokenEx(ctx, auth, paramSet, isEncrypt);
    if (ret != HKS_SUCCESS) {
        LOGE("AppendNonceAad failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksBuildParamSet failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    return paramSet;
}

static HksParamSet *GenHuksOptionParam(KeyContext &ctx,
                                       const UserAuth &auth,
                                       const bool isEncrypt,
                                       const bool isNeedNewNonce)
{
    HksParam encryptParam[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_IS_KEY_ALIAS, .boolParam = false },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = isEncrypt ? HKS_KEY_PURPOSE_ENCRYPT : HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_CHALLENGE_TYPE, .uint32Param = HKS_CHALLENGE_TYPE_NONE },
        { .tag = HKS_TAG_PROCESS_NAME, .blob = { sizeof(g_processName), g_processName } }
    };

    HksParamSet *paramSet = nullptr;
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

    if (!isEncrypt) {
        ret = AppendAeTag(ctx.rndEnc, paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("AppendAeTag failed ret %{public}d", ret);
            HksFreeParamSet(&paramSet);
            return nullptr;
        }
    }

    ret = isNeedNewNonce ? AppendNonceAadToken(ctx, auth, paramSet)
                        : AppendNewNonceAadToken(ctx, auth, paramSet, isEncrypt);
    if (ret != HKS_SUCCESS) {
        LOGE("AppendNonceAad failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksBuildParamSet failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    return paramSet;
}

int HuksMaster::HuksHalTripleStage(HksParamSet *paramSet1, const HksParamSet *paramSet2,
                                   const KeyBlob &keyIn, KeyBlob &keyOut)
{
    LOGI("enter");
    HuksBlob hksKey = { reinterpret_cast<uint8_t *>(paramSet1), paramSet1->paramSetSize };
    HuksBlob hksIn = keyIn.ToHuksBlob();
    HuksBlob hksOut = keyOut.ToHuksBlob();
    uint8_t h[sizeof(uint64_t)] = {0};
    HuksBlob hksHandle = { h, sizeof(uint64_t) };
    uint8_t t[CRYPTO_TOKEN_SIZE] = {0};
    HuksBlob hksToken = { t, sizeof(t) };  // would not use the challenge here
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = HdiAccessInit(hksKey, paramSet2, hksHandle, hksToken);
    if (ret != HKS_SUCCESS) {
        LOGE("HdiAccessInit failed ret %{public}d", ret);
        return ret;
    }
    LOGI("SD_DURATION: HUKS: INIT: delay time = %{public}s",
        StorageService::StorageRadar::RecordDuration(startTime).c_str());
    startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = HdiAccessFinish(hksHandle, paramSet2, hksIn, hksOut);
    if (ret != HKS_SUCCESS) {
        if (ret == HKS_ERROR_KEY_AUTH_TIME_OUT) {
            StorageService::KeyCryptoUtils::ForceLockUserScreen();
            LOGE("HdiAccessFinish failed because authToken timeout, force lock user screen.");
        }
        LOGE("HdiAccessFinish failed ret %{public}d", ret);
        return ret;
    }
    LOGI("SD_DURATION: HUKS: FINISH: delay time = %{public}s",
        StorageService::StorageRadar::RecordDuration(startTime).c_str());
    keyOut.size = hksOut.dataLen;
    LOGI("finish");
    return E_OK;
}

int32_t HuksMaster::EncryptKeyEx(const UserAuth &auth, const KeyBlob &rnd, KeyContext &ctx)
{
    LOGI("enter");
    if (ctx.shield.IsEmpty()) {
        LOGE("bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (rnd.IsEmpty()) {
        LOGE("bad rawKey input, size %{public}d", rnd.size);
        return E_KEY_BLOB_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParamEx(ctx, auth, true);
    if (paramSet2 == nullptr) {
        LOGE("GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    ctx.rndEnc.Alloc(rnd.size + CRYPTO_AES_AAD_LEN);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, rnd, ctx.rndEnc);
    if (ret != E_OK) {
        LOGE("HuksHalTripleStage failed");
    }
    HksFreeParamSet(&paramSet2);
    LOGI("finish");
    return ret;
}

int32_t HuksMaster::EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key, bool isNeedNewNonce)
{
    LOGI("enter");
    if (ctx.shield.IsEmpty()) {
        LOGE("bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (key.key.IsEmpty()) {
        LOGE("bad rawKey input, size %{public}d", key.key.size);
        return E_KEY_EMPTY_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParam(ctx, auth, true, isNeedNewNonce);
    if (paramSet2 == nullptr) {
        LOGE("GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    ctx.rndEnc.Alloc(CRYPTO_AES_256_KEY_ENCRYPTED_SIZE);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, key.key, ctx.rndEnc);
    if (ret != E_OK) {
        LOGE("HuksHalTripleStage failed");
    }
    HksFreeParamSet(&paramSet2);
    LOGI("finish");
    return ret;
}

int32_t HuksMaster::DecryptKey(KeyContext &ctx, const UserAuth &auth, KeyInfo &key, bool isNeedNewNonce)
{
    LOGI("enter");
    if (ctx.shield.IsEmpty()) {
        LOGE("bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (ctx.rndEnc.IsEmpty()) {
        LOGE("bad encrypted input, size %{public}d", ctx.rndEnc.size);
        return E_KEY_CTX_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParam(ctx, auth, false, isNeedNewNonce);
    if (paramSet2 == nullptr) {
        LOGE("GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    key.key.Alloc(ctx.rndEnc.size);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, ctx.rndEnc, key.key);
    if (ret != E_OK) {
        LOGE("HuksHalTripleStage failed");
    }
    HksFreeParamSet(&paramSet2);
    LOGI("finish");
    return ret;
}

int32_t HuksMaster::DecryptKeyEx(KeyContext &ctx, const UserAuth &auth, KeyBlob &rnd)
{
    LOGI("enter");
    if (ctx.shield.IsEmpty()) {
        LOGE("bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (ctx.rndEnc.IsEmpty()) {
        LOGE("bad encrypted input, size %{public}d", ctx.rndEnc.size);
        return E_KEY_CTX_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParamEx(ctx, auth, false);
    if (paramSet2 == nullptr) {
        LOGE("GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    rnd.Alloc(ctx.rndEnc.size);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, ctx.rndEnc, rnd);
    if (ret != E_OK) {
        LOGE("HuksHalTripleStage failed");
    }
    HksFreeParamSet(&paramSet2);
    LOGI("finish");
    return ret;
}

static bool CheckNeedUpgrade(KeyBlob &inData)
{
    constexpr uint32_t HKS_KEY_VERSION = 3;
    HksParamSet *keyBlobParamSet = nullptr;
    int ret = HksGetParamSet(reinterpret_cast<HksParamSet *>(inData.data.get()), inData.size, &keyBlobParamSet);
    if (ret != HKS_SUCCESS) {
        LOGE("HksGetParamSet failed %{public}d", ret);
        return false;
    }

    struct HksParam *keyVersion = nullptr;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_KEY_VERSION, &keyVersion);
    if (ret != HKS_SUCCESS) {
        LOGE("version get key param failed!");
        HksFreeParamSet(&keyBlobParamSet);
        return false;
    }

    if (keyVersion->uint32Param >= HKS_KEY_VERSION) {
        HksFreeParamSet(&keyBlobParamSet);
        return false;
    }
    HksFreeParamSet(&keyBlobParamSet);
    return true;
}

bool HuksMaster::UpgradeKey(KeyContext &ctx)
{
    struct HksParamSet *paramSet = nullptr;
    bool ret = false;

    if (!CheckNeedUpgrade(ctx.shield)) {
        LOGI("no need to upgrade");
        return false;
    }

    LOGI("Do upgradekey");
    do {
        int err = HksInitParamSet(&paramSet);
        if (err != HKS_SUCCESS) {
            LOGE("HksInitParamSet failed ret %{public}d", err);
            break;
        }
        err = HksAddParams(paramSet, g_generateKeyParam, HKS_ARRAY_SIZE(g_generateKeyParam));
        if (err != HKS_SUCCESS) {
            LOGE("HksAddParams failed ret %{public}d", err);
            break;
        }
        err = HksBuildParamSet(&paramSet);
        if (err != HKS_SUCCESS) {
            LOGE("HksBuildParamSet failed ret %{public}d", err);
            break;
        }

        KeyBlob keyOut(CRYPTO_KEY_SHIELD_MAX_SIZE);
        HuksBlob hksIn = ctx.shield.ToHuksBlob();
        HuksBlob hksOut = keyOut.ToHuksBlob();
        err = HdiAccessUpgradeKey(hksIn, paramSet, hksOut);
        if (err == HKS_SUCCESS) {
            LOGI("Shield upgraded successfully");
            keyOut.size = hksOut.dataLen;
            ctx.shield.Clear();
            ctx.shield = std::move(keyOut);
            ret = true;
        }
    } while (0);
    HksFreeParamSet(&paramSet);
    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS