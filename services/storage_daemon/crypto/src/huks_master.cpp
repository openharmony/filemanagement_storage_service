/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifdef HUKS_IDL_ENVIRONMENT
constexpr uint8_t MAX_RETRY_TIME = 3;
constexpr uint16_t RETRY_INTERVAL_MS = 50 * 1000;
constexpr uint32_t CRYPTO_KEY_ALIAS_SIZE = 16;
constexpr uint32_t CRYPTO_AES_AAD_LEN = 16;
constexpr uint32_t CRYPTO_AES_NONCE_LEN = 64;
constexpr uint32_t CRYPTO_HKS_NONCE_LEN = 12;
constexpr uint32_t CRYPTO_KEY_SHIELD_MAX_SIZE = 2048;
constexpr uint32_t CRYPTO_AES_256_KEY_ENCRYPTED_SIZE = 80;
constexpr uint32_t CRYPTO_TOKEN_SIZE = TOKEN_CHALLENGE_LEN; // 32
#endif

HuksMaster::HuksMaster()
{
    LOGI("[L8:HuksMaster] HuksMaster: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    InitHdiProxyInstance();
    HdiModuleInit();
#endif
    LOGI("[L8:HuksMaster] HuksMaster: <<< EXIT SUCCESS <<<");
}

HuksMaster::~HuksMaster()
{
    LOGI("[L8:HuksMaster] ~HuksMaster: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    HdiModuleDestroy();
    ReleaseHdiProxyInstance();
#endif
    LOGI("[L8:HuksMaster] ~HuksMaster: <<< EXIT SUCCESS <<<");
}

#ifdef HUKS_IDL_ENVIRONMENT
int32_t HuksMaster::InitHdiProxyInstance()
{
    LOGI("[L8:HuksMaster] InitHdiProxyInstance: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(hdiProxyMutex_);
    if (hksHdiProxyInstance_ != nullptr) {
        LOGI("[L8:HuksMaster] InitHdiProxyInstance: <<< EXIT SUCCESS <<< hdiProxyInstance already exists");
        return HKS_SUCCESS;
    }
    hksHdiProxyInstance_ = IHuksGetInstance("hdi_service", true);
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] InitHdiProxyInstance: <<< EXIT FAILED <<< IHuksGet hdi huks service failed");
        StorageRadar::ReportHuksResult("InitHdiProxyInstance", HKS_ERROR_NULL_POINTER);
        return HKS_ERROR_NULL_POINTER;
    }
    LOGI("[L8:HuksMaster] InitHdiProxyInstance: <<< EXIT SUCCESS <<<");
    return HKS_SUCCESS;
}

void HuksMaster::ReleaseHdiProxyInstance()
{
    LOGI("[L8:HuksMaster] ReleaseHdiProxyInstance: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(hdiProxyMutex_);
    if (hksHdiProxyInstance_ != nullptr) {
        IHuksReleaseInstance("hdi_service", hksHdiProxyInstance_, true);
    }
    hksHdiProxyInstance_ = nullptr;
    LOGI("[L8:HuksMaster] ReleaseHdiProxyInstance: <<< EXIT SUCCESS <<<");
}

int32_t HuksMaster::HdiModuleInit()
{
    LOGI("[L8:HuksMaster] HdiModuleInit: >>> ENTER <<<");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] HdiModuleInit: <<< EXIT FAILED <<< hksHdiProxyInstance_ is nullptr");
        StorageRadar::ReportHuksResult("HdiModuleInit.hksHdiProxyInstance_", HKS_ERROR_NULL_POINTER);
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->ModuleInit == nullptr) {
        LOGE("[L8:HuksMaster] HdiModuleInit: <<< EXIT FAILED <<< HuksHdiModuleInit is nullptr");
        StorageRadar::ReportHuksResult("HdiModuleInit hksHdiProxyInstance_->ModuleInit", HKS_ERROR_NULL_POINTER);
        return HKS_ERROR_NULL_POINTER;
    }
    auto ret = hksHdiProxyInstance_->ModuleInit(hksHdiProxyInstance_);
    if (ret == HKS_SUCCESS) {
        LOGI("[L8:HuksMaster] HdiModuleInit: <<< EXIT SUCCESS <<< HuksHdiModuleInit success, ret %{public}d", ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("[L8:HuksMaster] HdiModuleInit: <<< EXIT FAILED <<< HuksHdiModuleInit failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdiModuleInit", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->ModuleInit(hksHdiProxyInstance_);
        LOGE("[L8:HuksMaster] HdiModuleInit: HuksHdiModuleInit has retry %{public}d times, retryRet %{public}d",
             i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("[L8:HuksMaster] HdiModuleInit: HuksHdiModuleInit end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiModuleInit_Retry", retryRet);
        LOGI("[L8:HuksMaster] HdiModuleInit: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L8:HuksMaster] HdiModuleInit: <<< EXIT SUCCESS <<<");
    }
    return retryRet;
}

int32_t HuksMaster::HdiModuleDestroy()
{
    LOGI("[L8:HuksMaster] HdiModuleDestroy: >>> ENTER <<<");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] HdiModuleDestroy: <<< EXIT FAILED <<< hksHdiProxyInstance_ is nullptr");
        StorageRadar::ReportHuksResult("HdiModuleDestroy.hksHdiProxyInstance_", HKS_ERROR_NULL_POINTER);
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->ModuleDestroy == nullptr) {
        LOGE("[L8:HuksMaster] HdiModuleDestroy: <<< EXIT FAILED <<< HuksHdiModuleDestroy is nullptr");
        StorageRadar::ReportHuksResult("HdiModuleDestroy hksHdiProxyInstance_->ModuleDestroy", HKS_ERROR_NULL_POINTER);
        return HKS_ERROR_NULL_POINTER;
    }
    auto ret = hksHdiProxyInstance_->ModuleDestroy(hksHdiProxyInstance_);
    if (ret == HKS_SUCCESS) {
        LOGI("[L8:HuksMaster] HdiModuleDestroy: <<< EXIT SUCCESS <<< HuksHdiModuleDestroy success, ret %{public}d",
             ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("[L8:HuksMaster] HdiModuleDestroy: <<< EXIT FAILED <<< HuksHdiModuleDestroy failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdiModuleDestroy", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->ModuleDestroy(hksHdiProxyInstance_);
        LOGE("[L8:HuksMaster] HdiModuleDestroy: HuksHdiModuleDestroy has retry %{public}d times, retryRet %{public}d",
             i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("[L8:HuksMaster] HdiModuleDestroy: HuksHdiModuleDestroy end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiModuleDestroy_Retry", retryRet);
        LOGE("[L8:HuksMaster] HdiModuleDestroy: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L8:HuksMaster] HdiModuleDestroy: <<< EXIT SUCCESS <<<");
    }
    return retryRet;
}

int32_t HuksMaster::HdiGenerateKey(const HuksBlob &keyAlias, const HksParamSet *paramSetIn, HuksBlob &keyOut)
{
    LOGI("[L8:HuksMaster] HdiGenerateKey: >>> ENTER <<<");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] HdiGenerateKey: <<< EXIT FAILED <<< hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->GenerateKey == nullptr) {
        LOGE("[L8:HuksMaster] HdiGenerateKey: <<< EXIT FAILED <<< HuksHdiGenerateKey is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    uint8_t keyData = 0;
    struct HuksBlob keyIn = {&keyData, 1};
    struct HuksParamSet hksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSetIn, hksParamSet);
    auto ret = hksHdiProxyInstance_->GenerateKey(hksHdiProxyInstance_, &keyAlias, &hksParamSet, &keyIn, &keyOut);
    if (ret == HKS_SUCCESS) {
        LOGI("[L8:HuksMaster] HdiGenerateKey: <<< EXIT SUCCESS <<< HuksHdiGenerateKey success, ret %{public}d", ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("[L8:HuksMaster] HdiGenerateKey: <<< EXIT FAILED <<< HuksHdiGenerateKey failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdi GenerateKey", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->GenerateKey(hksHdiProxyInstance_, &keyAlias, &hksParamSet, &keyIn, &keyOut);
        LOGE("[L8:HuksMaster] HdiGenerateKey: HuksHdiGenerateKey has retry %{public}d times, retryRet %{public}d",
             i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("[L8:HuksMaster] HdiGenerateKey: HuksHdiGenerateKey end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdi GenerateKey_Retry", retryRet);
        LOGI("[L8:HuksMaster] HdiGenerateKey: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L8:HuksMaster] HdiGenerateKey: <<< EXIT SUCCESS <<<");
    }
    return retryRet;
}

int32_t HuksMaster::HdiAccessInit(const HuksBlob &key, const HksParamSet *paramSet, HuksBlob &handle, HuksBlob &token)
{
    LOGD("[L8:HuksMaster] HdiAccessInit: >>> ENTER <<<");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] HdiAccessInit: <<< EXIT FAILED <<< hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->Init == nullptr) {
        LOGE("[L8:HuksMaster] HdiAccessInit: <<< EXIT FAILED <<< HuksHdiInit is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HuksParamSet huksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSet, huksParamSet);
    auto ret = hksHdiProxyInstance_->Init(hksHdiProxyInstance_, &key, &huksParamSet, &handle, &token);
    if (ret == HKS_SUCCESS) {
        LOGD("[L8:HuksMaster] HdiAccessInit: <<< EXIT SUCCESS <<< HuksHdiInit success, ret %{public}d", ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("[L8:HuksMaster] HdiAccessInit: <<< EXIT FAILED <<< HuksHdiInit failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HdiAccessInit hksHdiProxyInstance_->Init", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->Init(hksHdiProxyInstance_, &key, &huksParamSet, &handle, &token);
        LOGE("[L8:HuksMaster] HdiAccessInit: HuksHdiInit has retry %{public}d times, retryRet %{public}d", i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("[L8:HuksMaster] HdiAccessInit: HuksHdiInit end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiInit_Retry", retryRet);
        LOGI("[L8:HuksMaster] HdiAccessInit: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L8:HuksMaster] HdiAccessInit: <<< EXIT SUCCESS <<<");
    }
    return retryRet;
}

int32_t HuksMaster::HdiAccessFinish(const HuksBlob &handle, const HksParamSet *paramSet,
                                    const HuksBlob &inData, HuksBlob &outData)
{
    LOGD("[L8:HuksMaster] HdiAccessFinish: >>> ENTER <<<");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] HdiAccessFinish: <<< EXIT FAILED <<< hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->Finish == nullptr) {
        LOGE("[L8:HuksMaster] HdiAccessFinish: <<< EXIT FAILED <<< HuksHdiFinish is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HuksParamSet huksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSet, huksParamSet);
    auto ret = hksHdiProxyInstance_->Finish(hksHdiProxyInstance_, &handle, &huksParamSet, &inData, &outData);
    if (ret == HKS_SUCCESS) {
        LOGD("[L8:HuksMaster] HdiAccessFinish: <<< EXIT SUCCESS <<< HuksHdiFinish success, ret %{public}d", ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("[L8:HuksMaster] HdiAccessFinish: <<< EXIT FAILED <<< HuksHdiFinish failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HdiAccessFinish Finish", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->Finish(hksHdiProxyInstance_, &handle, &huksParamSet, &inData, &outData);
        LOGE("[L8:HuksMaster] HdiAccessFinish: HuksHdiFinish has retry %{public}d times, retryRet %{public}d",
             i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("[L8:HuksMaster] HdiAccessFinish: HuksHdiFinish end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdiFinish_Retry", retryRet);
        LOGI("[L8:HuksMaster] HdiAccessFinish: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L8:HuksMaster] HdiAccessFinish: <<< EXIT SUCCESS <<<");
    }
    return retryRet;
}

int32_t HuksMaster::HdiAccessUpgradeKey(const HuksBlob &oldKey, const HksParamSet *paramSet, struct HuksBlob &newKey)
{
    LOGI("[L8:HuksMaster] HdiAccessUpgradeKey: >>> ENTER <<<");
    if (hksHdiProxyInstance_ == nullptr) {
        LOGE("[L8:HuksMaster] HdiAccessUpgradeKey: <<< EXIT FAILED <<< hksHdiProxyInstance_ is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }
    if (hksHdiProxyInstance_->UpgradeKey == nullptr) {
        LOGE("[L8:HuksMaster] HdiAccessUpgradeKey: <<< EXIT FAILED <<< HuksHdiUpgradeKey is nullptr");
        return HKS_ERROR_NULL_POINTER;
    }

    struct HuksParamSet huksParamSet;
    HDI_CONVERTER_PARAM_IN_PARAMSET(paramSet, huksParamSet);
    auto ret = hksHdiProxyInstance_->UpgradeKey(hksHdiProxyInstance_, &oldKey, &huksParamSet, &newKey);
    if (ret == HKS_SUCCESS) {
        LOGI("[L8:HuksMaster] HdiAccessUpgradeKey: <<< EXIT SUCCESS <<< HuksHdiUpgradeKey success, ret %{public}d",
             ret);
        return ret;
    }

    if (ret != HKS_ERROR_RETRYABLE_ERROR) {
        LOGE("[L8:HuksMaster] HdiAccessUpgradeKey: <<< EXIT FAILED <<< HuksHdiUpgradeKey failed, ret %{public}d", ret);
        StorageRadar::ReportHuksResult("HuksHdiUpgradeKey", ret);
        return ret;
    }
    int retryRet = 0;
    for (int i = 0; i < MAX_RETRY_TIME; ++i) {
        usleep(RETRY_INTERVAL_MS);
        retryRet = hksHdiProxyInstance_->UpgradeKey(hksHdiProxyInstance_, &oldKey, &huksParamSet, &newKey);
        LOGE("[L8:HuksMaster] HdiAccessUpgradeKey: HuksHdiUpgradeKey has retry %{public}d times, retryRet %{public}d",
             i, retryRet);
        if (retryRet == HKS_SUCCESS) {
            break;
        }
    }
    LOGE("[L8:HuksMaster] HdiAccessUpgradeKey: HuksHdiUpgradeKey end, retryRet %{public}d", retryRet);
    if (retryRet != HKS_SUCCESS) {
        StorageRadar::ReportHuksResult("HuksHdi UpgradeKey_Retry", retryRet);
        LOGI("[L8:HuksMaster] HdiAccessUpgradeKey: <<< EXIT FAILED <<<");
    } else {
        LOGI("[L8:HuksMaster] HdiAccessUpgradeKey: <<< EXIT SUCCESS <<<");
    }
    return retryRet;
}

static bool CheckNeedUpgrade(KeyBlob &inData)
{
    constexpr uint32_t HKS_KEY_VERSION = 3;
    HksParamSet *keyBlobParamSet = nullptr;
    int ret = HksGetParamSet(reinterpret_cast<HksParamSet *>(inData.data.get()), inData.size, &keyBlobParamSet);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] CheckNeedUpgrade: HksGetParamSet failed %{public}d", ret);
        StorageRadar::ReportHuksResult("CheckNeedUpgrade HksGetParamSet", ret);
        return false;
    }

    struct HksParam *keyVersion = nullptr;
    ret = HksGetParam(keyBlobParamSet, HKS_TAG_KEY_VERSION, &keyVersion);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] CheckNeedUpgrade: version get key param failed!");
        StorageRadar::ReportHuksResult("CheckNeedUpgrade HksGetParam", ret);
        HksFreeParamSet(&keyBlobParamSet);
        return false;
    }

    if (keyVersion->uint32Param >= HKS_KEY_VERSION) {
        StorageRadar::ReportUserKeyResult("CheckNeedUpgrade", 0, 0, "",
            "keyVersion.uint32Param=" + std::to_string(keyVersion->uint32Param));
        HksFreeParamSet(&keyBlobParamSet);
        return false;
    }
    HksFreeParamSet(&keyBlobParamSet);
    return true;
}

static int AppendSecureAccessParams(const UserAuth &auth, HksParamSet *paramSet)
{
    if (auth.secret.IsEmpty() || auth.token.IsEmpty()) {
        LOGI("[L8:HuksMaster] AppendSecureAccessParams: auth is empty, not to enable secure access for the key");
        return HKS_SUCCESS;
    }

    LOGI("[L8:HuksMaster] AppendSecureAccessParams: append the secure access params when generate key");

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
        LOGE("[L8:HuksMaster] AppendAeTag: cipherText size %{public}d is too small", cipherText.size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }
    if (cipherText.data.get() == nullptr) {
        LOGE("[L8:HuksMaster] AppendAeTag: cipherText data pointer is null");
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
        LOGI("[L8:HuksMaster] AppendNonceAadTokenEx: auth is empty, not to use secure access tag");
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
    LOGI("[L8:HuksMaster] AppendNewNonceAadToken: append the secure access params when encrypt/decrypt");
    if (isEncrypt) {
        ctx.nonce = HuksMaster::GenerateRandomKey(CRYPTO_HKS_NONCE_LEN);
        LOGI("[L8:HuksMaster] AppendNewNonceAadToken: Encrypt generate new nonce size: %{public}d", ctx.nonce.size);
    }
    ctx.aad = HashWithPrefix("AAD SHA512 prefix", ctx.secDiscard, CRYPTO_AES_AAD_LEN);
    LOGI("[L8:HuksMaster] AppendNewNonceAadToken: secret/token is empty : %{public}d / %{public}d",
         auth.secret.IsEmpty(), auth.token.IsEmpty());
    if (auth.secret.IsEmpty() && auth.token.IsEmpty()) {
        LOGI("[L8:HuksMaster] AppendNewNonceAadToken: token & secret is empty, Only append nonce & aad!");
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
        LOGI("[L8:HuksMaster] AppendNonceAadToken: auth is empty, not to use secure access tag");
        return AppendNonceAad(ctx, paramSet);
    }

    LOGI("[L8:HuksMaster] AppendNonceAadToken: append the secure access params when encrypt/decrypt");
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
        LOGE("[L8:HuksMaster] GenHuksOptionParamEx: HksInitParamSet failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParamEx HksInitParamSet", ret);
        return nullptr;
    }
    ret = HksAddParams(paramSet, encryptParam, HKS_ARRAY_SIZE(encryptParam));
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] GenHuksOptionParamEx: HksAddParams failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParamEx HksAddParams", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    if (!isEncrypt) {
        ret = AppendAeTag(ctx.rndEnc, paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenHuksOptionParamEx: AppendAeTag failed ret %{public}d", ret);
            HksFreeParamSet(&paramSet);
            StorageRadar::ReportHuksResult("GenHuksOptionParamEx AppendAeTag", ret);
            return nullptr;
        }
    }

    ret = AppendNonceAadTokenEx(ctx, auth, paramSet, isEncrypt);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] GenHuksOptionParamEx: AppendNonceAad failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParamEx AppendNonceAadToken", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] GenHuksOptionParamEx: HksBuildParamSet failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParamEx HksBuildParamSet", ret);
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
        LOGE("[L8:HuksMaster] GenHuksOptionParam: HksInitParamSet failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParam HksInitParamSet", ret);
        return nullptr;
    }
    ret = HksAddParams(paramSet, encryptParam, HKS_ARRAY_SIZE(encryptParam));
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] GenHuksOptionParam: HksAddParams failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParam HksAddParams", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    if (!isEncrypt) {
        ret = AppendAeTag(ctx.rndEnc, paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenHuksOptionParam: AppendAeTag failed ret %{public}d", ret);
            HksFreeParamSet(&paramSet);
            StorageRadar::ReportHuksResult("GenHuksOptionParam AppendAeTag", ret);
            return nullptr;
        }
    }

    ret = isNeedNewNonce ? AppendNonceAadToken(ctx, auth, paramSet)
                         : AppendNewNonceAadToken(ctx, auth, paramSet, isEncrypt);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] GenHuksOptionParam: AppendNonceAad failed ret %{public}d", ret);
        HksFreeParamSet(&paramSet);
        StorageRadar::ReportHuksResult("GenHuksOptionParam AppendNonceAadToken", ret);
        return nullptr;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] GenHuksOptionParam: HksBuildParamSet failed ret %{public}d", ret);
        StorageRadar::ReportHuksResult("GenHuksOptionParam HksBuildParamSet", ret);
        HksFreeParamSet(&paramSet);
        return nullptr;
    }

    return paramSet;
}

int32_t HuksMaster::HuksHalTripleStage(HksParamSet *paramSet1, const HksParamSet *paramSet2,
                                       const KeyBlob &keyIn, KeyBlob &keyOut)
{
    LOGD("[L8:HuksMaster] HuksHalTripleStage: >>> ENTER <<<");
    HuksBlob hksKey = { reinterpret_cast<uint8_t *>(paramSet1), paramSet1->paramSetSize };
    HuksBlob hksIn = keyIn.ToHuksBlob();
    HuksBlob hksOut = keyOut.ToHuksBlob();
    uint8_t h[sizeof(uint64_t)] = {0};
    HuksBlob hksHandle = { h, sizeof(uint64_t) };
    uint8_t t[CRYPTO_TOKEN_SIZE] = {0};
    HuksBlob hksToken = { t, sizeof(t) };  // would not use the challenge here

    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int32_t ret = HdiAccessInit(hksKey, paramSet2, hksHandle, hksToken);
    if (ret != HKS_SUCCESS) {
        LOGE("[L8:HuksMaster] HuksHalTripleStage: <<< EXIT FAILED <<< HdiAccessInit failed ret %{public}d", ret);
        return ret;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("HUKS: INIT", startTime);
    LOGI("SD_DURATION: HUKS: INIT: delay time = %{public}s", delay.c_str());
    startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = HdiAccessFinish(hksHandle, paramSet2, hksIn, hksOut);
    if (ret != HKS_SUCCESS) {
        if (ret == HKS_ERROR_KEY_AUTH_TIME_OUT) {
            StorageService::KeyCryptoUtils::ForceLockUserScreen();
            LOGE("[L8:HuksMaster] HuksHalTripleStage: HdiAccessFinish failed because authToken timeout, force lock"
                 "user screen.");
        }
        LOGE("[L8:HuksMaster] HuksHalTripleStage: <<< EXIT FAILED <<< HdiAccessFinish failed ret %{public}d", ret);
        return ret;
    }
    delay = StorageService::StorageRadar::ReportDuration("HUKS: FINISH", startTime);
    LOGI("SD_DURATION: HUKS: FINISH: delay time = %{public}s", delay.c_str());

    keyOut.size = hksOut.dataLen;
    LOGD("[L8:HuksMaster] HuksHalTripleStage: <<< EXIT SUCCESS <<<");
    return E_OK;
}
#endif

KeyBlob HuksMaster::GenerateRandomKey(uint32_t keyLen)
{
    LOGI("[L8:HuksMaster] GenerateRandomKey: >>> ENTER <<< size %{public}d", keyLen);
    KeyBlob out(keyLen);
    if (out.IsEmpty()) {
        LOGI("[L8:HuksMaster] GenerateRandomKey: <<< EXIT FAILED <<< out is empty");
        StorageRadar::ReportUserKeyResult("GenerateRandomKey", 0, E_KEY_BLOB_ERROR, "",
            "KeyBlob alloc failed, keyLen=" + std::to_string(keyLen));
        return out;
    }

    auto ret = RAND_bytes(out.data.get(), out.size);
    if (ret <= 0) {
        LOGE("[L8:HuksMaster] GenerateRandomKey: <<< EXIT FAILED <<< RAND_bytes failed return %{public}d, errno"
             "%{public}lu", ret, ERR_get_error());
        StorageRadar::ReportUserKeyResult("GenerateRandomKey", 0, E_KEY_BLOB_ERROR, "",
            "RAND_bytes failed, ret=" + std::to_string(ret) + ", errno=" + std::to_string(ERR_get_error()) +
            ", keyLen=" + std::to_string(keyLen));
        out.Clear();
    } else {
        LOGI("[L8:HuksMaster] GenerateRandomKey: <<< EXIT SUCCESS <<<");
    }
    return out;
}

int32_t HuksMaster::GenerateKey(const UserAuth &auth, KeyBlob &keyOut)
{
    LOGI("[L8:HuksMaster] GenerateKey: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    HksParamSet *paramSet = nullptr;
    int ret = HKS_SUCCESS;
    do {
        ret = HksInitParamSet(&paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenerateKey: <<< EXIT FAILED <<< HksInitParamSet failed ret %{public}d", ret);
            StorageRadar::ReportUserKeyResult("GenerateKey", auth.secureUid, ret, "", "HksInitParamSet failed");
            break;
        }
        ret = HksAddParams(paramSet, g_generateKeyParam, HKS_ARRAY_SIZE(g_generateKeyParam));
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenerateKey: <<< EXIT FAILED <<< HksAddParams failed ret %{public}d", ret);
            StorageRadar::ReportUserKeyResult("GenerateKey", auth.secureUid, ret, "", "HksAddParams failed");
            break;
        }
        ret = AppendSecureAccessParams(auth, paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenerateKey: <<< EXIT FAILED <<< AppendSecureAccessParams failed ret %{public}d",
                 ret);
            StorageRadar::ReportUserKeyResult("GenerateKey", auth.secureUid, ret, "", "AppendSecureAccessParams failed");
            break;
        }
        ret = HksBuildParamSet(&paramSet);
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenerateKey: <<< EXIT FAILED <<< HksBuildParamSet failed ret %{public}d", ret);
            StorageRadar::ReportUserKeyResult("GenerateKey", auth.secureUid, ret, "", "HksBuildParamSet failed");
            break;
        }
        KeyBlob alias = GenerateRandomKey(CRYPTO_KEY_ALIAS_SIZE);
        HuksBlob hksAlias = alias.ToHuksBlob();
        keyOut.Alloc(CRYPTO_KEY_SHIELD_MAX_SIZE);
        HuksBlob hksKeyOut = keyOut.ToHuksBlob();
        ret = HdiGenerateKey(hksAlias, paramSet, hksKeyOut);
        if (ret != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] GenerateKey: <<< EXIT FAILED <<< HdiGenerateKey failed ret %{public}d", ret);
            StorageRadar::ReportUserKeyResult("GenerateKey", auth.secureUid, ret, "", "HdiGenerateKey failed");
            break;
        }
        keyOut.size = hksKeyOut.dataLen;
        LOGI("[L8:HuksMaster] GenerateKey: <<< EXIT SUCCESS <<< HdiGenerateKey success, out size %{public}d",
             keyOut.size);
    } while (0);

    HksFreeParamSet(&paramSet);
    return ret;
#endif
    LOGI("[L8:HuksMaster] GenerateKey: <<< EXIT SUCCESS <<<");
    return HKS_SUCCESS;
}

int32_t HuksMaster::EncryptKeyEx(const UserAuth &auth, const KeyBlob &rnd, KeyContext &ctx)
{
    LOGI("[L8:HuksMaster] EncryptKeyEx: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    if (ctx.shield.IsEmpty()) {
        LOGE("[L8:HuksMaster] EncryptKeyEx: <<< EXIT FAILED <<< bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (rnd.IsEmpty()) {
        LOGE("[L8:HuksMaster] EncryptKeyEx: <<< EXIT FAILED <<< bad rawKey input, size %{public}d", rnd.size);
        return E_KEY_BLOB_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("[L8:HuksMaster] EncryptKeyEx: <<< EXIT FAILED <<< GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParamEx(ctx, auth, true);
    if (paramSet2 == nullptr) {
        LOGE("[L8:HuksMaster] EncryptKeyEx: <<< EXIT FAILED <<< GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    ctx.rndEnc.Alloc(rnd.size + CRYPTO_AES_AAD_LEN);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, rnd, ctx.rndEnc);
    if (ret != E_OK) {
        LOGE("[L8:HuksMaster] EncryptKeyEx: <<< EXIT FAILED <<< HuksHalTripleStage failed");
    } else {
        LOGI("[L8:HuksMaster] EncryptKeyEx: <<< EXIT SUCCESS <<<");
    }

    HksFreeParamSet(&paramSet2);
    return ret;
#endif
    LOGI("[L8:HuksMaster] EncryptKeyEx: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t HuksMaster::EncryptKey(KeyContext &ctx, const UserAuth &auth, const KeyInfo &key, bool isNeedNewNonce)
{
    LOGI("[L8:HuksMaster] EncryptKey: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    if (ctx.shield.IsEmpty()) {
        LOGE("[L8:HuksMaster] EncryptKey: <<< EXIT FAILED <<< bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (key.key.IsEmpty()) {
        LOGE("[L8:HuksMaster] EncryptKey: <<< EXIT FAILED <<< bad rawKey input, size %{public}d", key.key.size);
        return E_KEY_EMPTY_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("[L8:HuksMaster] EncryptKey: <<< EXIT FAILED <<< GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParam(ctx, auth, true, isNeedNewNonce);
    if (paramSet2 == nullptr) {
        LOGE("[L8:HuksMaster] EncryptKey: <<< EXIT FAILED <<< GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    ctx.rndEnc.Alloc(CRYPTO_AES_256_KEY_ENCRYPTED_SIZE);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, key.key, ctx.rndEnc);
    if (ret != E_OK) {
        LOGE("[L8:HuksMaster] EncryptKey: <<< EXIT FAILED <<< HuksHalTripleStage failed");
    } else {
        LOGI("[L8:HuksMaster] EncryptKey: <<< EXIT SUCCESS <<<");
    }

    HksFreeParamSet(&paramSet2);
    return ret;
#endif
    LOGI("[L8:HuksMaster] EncryptKey: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t HuksMaster::DecryptKey(KeyContext &ctx, const UserAuth &auth, KeyInfo &key, bool isNeedNewNonce)
{
    LOGI("[L8:HuksMaster] DecryptKey: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    if (ctx.shield.IsEmpty()) {
        LOGE("[L8:HuksMaster] DecryptKey: <<< EXIT FAILED <<< bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (ctx.rndEnc.IsEmpty()) {
        LOGE("[L8:HuksMaster] DecryptKey: <<< EXIT FAILED <<< bad encrypted input, size %{public}d", ctx.rndEnc.size);
        return E_KEY_CTX_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("[L8:HuksMaster] DecryptKey: <<< EXIT FAILED <<< GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParam(ctx, auth, false, isNeedNewNonce);
    if (paramSet2 == nullptr) {
        LOGE("[L8:HuksMaster] DecryptKey: <<< EXIT FAILED <<< GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    key.key.Alloc(ctx.rndEnc.size);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, ctx.rndEnc, key.key);
    if (ret != E_OK) {
        LOGE("[L8:HuksMaster] DecryptKey: <<< EXIT FAILED <<< HuksHalTripleStage failed");
    } else {
        LOGI("[L8:HuksMaster] DecryptKey: <<< EXIT SUCCESS <<<");
    }

    HksFreeParamSet(&paramSet2);
    return ret;
#endif
    LOGI("[L8:HuksMaster] DecryptKey: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t HuksMaster::DecryptKeyEx(KeyContext &ctx, const UserAuth &auth, KeyBlob &rnd)
{
    LOGD("[L8:HuksMaster] DecryptKeyEx: >>> ENTER <<<");
#ifdef HUKS_IDL_ENVIRONMENT
    if (ctx.shield.IsEmpty()) {
        LOGE("[L8:HuksMaster] DecryptKeyEx: <<< EXIT FAILED <<< bad shield input, size %{public}d", ctx.shield.size);
        return E_KEY_CTX_ERROR;
    }
    if (ctx.rndEnc.IsEmpty()) {
        LOGE("[L8:HuksMaster] DecryptKeyEx: <<< EXIT FAILED <<< bad encrypted input, size %{public}d", ctx.rndEnc.size);
        return E_KEY_CTX_ERROR;
    }

    HksParamSet *paramSet1 = GenHuksKeyBlobParam(ctx);
    if (paramSet1 == nullptr) {
        LOGE("[L8:HuksMaster] DecryptKeyEx: <<< EXIT FAILED <<< GenHuksKeyBlobParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }
    HksParamSet *paramSet2 = GenHuksOptionParamEx(ctx, auth, false);
    if (paramSet2 == nullptr) {
        LOGE("[L8:HuksMaster] DecryptKeyEx: <<< EXIT FAILED <<< GenHuksOptionParam failed");
        return E_GEN_HUKS_PARAM_ERROR;
    }

    rnd.Alloc(ctx.rndEnc.size);
    auto ret = HuksHalTripleStage(paramSet1, paramSet2, ctx.rndEnc, rnd);
    if (ret != E_OK) {
        LOGE("[L8:HuksMaster] DecryptKeyEx: <<< EXIT FAILED <<< HuksHalTripleStage failed");
    } else {
        LOGI("[L8:HuksMaster] DecryptKeyEx: <<< EXIT SUCCESS <<<");
    }

    HksFreeParamSet(&paramSet2);
    return ret;
#endif
    LOGI("[L8:HuksMaster] DecryptKeyEx: <<< EXIT SUCCESS <<<");
    return E_OK;
}

bool HuksMaster::UpgradeKey(KeyContext &ctx)
{
#ifdef HUKS_IDL_ENVIRONMENT
    struct HksParamSet *paramSet = nullptr;
    bool ret = false;

    if (!CheckNeedUpgrade(ctx.shield)) {
        LOGI("[L8:HuksMaster] UpgradeKey: no need to upgrade");
        return false;
    }

    LOGI("[L8:HuksMaster] UpgradeKey: Do upgradekey");
    do {
        int err = HksInitParamSet(&paramSet);
        if (err != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] UpgradeKey: HksInitParamSet failed ret %{public}d", err);
            StorageRadar::ReportUserKeyResult("UpgradeKey", 0, err, "", "HksInitParamSet failed");
            break;
        }
        err = HksAddParams(paramSet, g_generateKeyParam, HKS_ARRAY_SIZE(g_generateKeyParam));
        if (err != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] UpgradeKey: HksAddParams failed ret %{public}d", err);
            StorageRadar::ReportUserKeyResult("UpgradeKey", 0, err, "", "HksAddParams failed");
            break;
        }
        err = HksBuildParamSet(&paramSet);
        if (err != HKS_SUCCESS) {
            LOGE("[L8:HuksMaster] UpgradeKey: HksBuildParamSet failed ret %{public}d", err);
            StorageRadar::ReportUserKeyResult("UpgradeKey", 0, err, "", "HksBuildParamSet failed");
            break;
        }

        KeyBlob keyOut(CRYPTO_KEY_SHIELD_MAX_SIZE);
        HuksBlob hksIn = ctx.shield.ToHuksBlob();
        HuksBlob hksOut = keyOut.ToHuksBlob();

        err = HdiAccessUpgradeKey(hksIn, paramSet, hksOut);
        if (err == HKS_SUCCESS) {
            LOGI("[L8:HuksMaster] UpgradeKey: Shield upgraded successfully");
            keyOut.size = hksOut.dataLen;
            ctx.shield.Clear();
            ctx.shield = std::move(keyOut);
            ret = true;
        } else {
            LOGE("[L8:HuksMaster] UpgradeKey: Shield upgrade failed ret %{public}d", err);
            StorageRadar::ReportUserKeyResult("UpgradeKey", 0, err, "", "HdiAccessUpgradeKey failed");
        }
    } while (0);
    HksFreeParamSet(&paramSet);
    return ret;
#endif
    return false;
}

} // namespace StorageDaemon
} // namespace OHOS

