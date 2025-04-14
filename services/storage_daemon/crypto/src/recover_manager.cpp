/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "recover_manager.h"

#include <openssl/sha.h>
#include <unistd.h>
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
static const std::string CRYPTO_NAME_PREFIXES[] = {"ext4", "f2fs", "fscrypt"};
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
constexpr uint32_t ELX_TYPE_ARR[] = { TYPE_GLOBAL_EL1, USERID_GLOBAL_EL1, TYPE_EL1, TYPE_EL2, TYPE_EL3, TYPE_EL4 };
constexpr static uint32_t TEE_PARAM_INDEX_0 = 0;
constexpr static uint32_t TEE_PARAM_INDEX_1 = 1;
constexpr static uint32_t TEE_PARAM_INDEX_2 = 2;
constexpr static int SESSION_START_DEFAULT = 1;
constexpr int MAX_RETRY_COUNT = 3;
constexpr int RETRY_INTERVAL = 100 * 1000; // 100ms
#endif

RecoveryManager::RecoveryManager()
{
    LOGI("enter");
    isSessionOpened = false;
}

RecoveryManager::~RecoveryManager()
{
    LOGI("enter");
}

int RecoveryManager::CreateRecoverKey(uint32_t userId,
                                      uint32_t userType,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &secret,
                                      const std::vector<KeyBlob> &originIv)
{
    LOGI("enter");
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    TEEC_Context createKeyContext = {};
    TEEC_Session createKeySession = {};
    if (!OpenSession(createKeyContext, createKeySession)) {
        LOGE("Open session failed !");
        return E_RECOVERY_KEY_OPEN_SESSION_ERR;
    }
    uint32_t createKeyOrigin = 0;
    TEEC_Operation operation = { 0 };
    operation.started = SESSION_START_DEFAULT;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    CreateRecoverKeyStr recoverKeyStr = { .userType = userType, .userId = userId,
                                          .authTokenLen = static_cast<uint32_t>(token.size()) };
    if (!token.empty()) {
        auto err = memcpy_s(recoverKeyStr.authToken, AUTH_TOKEN_LEN, token.data(), token.size());
        if (err != EOK) {
            CloseSession(createKeyContext, createKeySession);
            return E_MEMORY_OPERATION_ERR;
        }
    }
    for (size_t i = 0; i < originIv.size(); ++i) {
        auto err = memcpy_s(recoverKeyStr.rndToTee[i], RND_AND_KEY2_LEN, originIv[i].data.get(), originIv[i].size);
        if (err != EOK) {
            CloseSession(createKeyContext, createKeySession);
            return E_MEMORY_OPERATION_ERR;
        }
    }
    operation.params[TEE_PARAM_INDEX_0].tmpref.buffer = static_cast<void *>(&recoverKeyStr);
    operation.params[TEE_PARAM_INDEX_0].tmpref.size = sizeof(recoverKeyStr);
    TEEC_Result ret = TEEC_InvokeCommand(&createKeySession, TaCmdId::RK_CMD_ID_GEN_RECOVERY_KEY,
                                         &operation, &createKeyOrigin);
    LOGW("InvokeCmd ret: %{public}d, origin: %{public}d, token size: %{public}zu", ret, createKeyOrigin, token.size());
    if (ret != TEEC_SUCCESS) {
        LOGE("InvokeCmd failed, ret: %{public}d, origin: %{public}d", ret, createKeyOrigin);
        CloseSession(createKeyContext, createKeySession);
        std::string extraData = "cmd=RK_CMD_ID_GEN_RECOVERY_KEY,ret=" + std::to_string(ret) +
            ",origin=" + std::to_string(createKeyOrigin);
        StorageRadar::ReportTEEClientResult("TEEC_InvokeCommand", E_TEEC_GEN_RECOVERY_KEY_ERR, userId, extraData);
        return E_TEEC_GEN_RECOVERY_KEY_ERR;
    }
    CloseSession(createKeyContext, createKeySession);
#endif
    LOGI("success");
    return 0;
}

int RecoveryManager::SetRecoverKey(const std::vector<uint8_t> &key)
{
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    SetRecoverKeyStr setRecoverKeyStr;
    int ret = SetRecoverKeyToTee(key, setRecoverKeyStr);
    if (ret != 0) {
        LOGE("Set recover key to tee failed !");
        return ret;
    }

    if (sizeof(setRecoverKeyStr.key2FromTee) != sizeof(setRecoverKeyStr.rndFromTee)) {
        LOGE("key2 size dose not match iv size !");
        return E_PARAMS_INVALID;
    }
    int rndNum = sizeof(setRecoverKeyStr.rndFromTee) / RND_AND_KEY2_LEN;
    int key2Num = sizeof(setRecoverKeyStr.key2FromTee) / RND_AND_KEY2_LEN;
    if (rndNum != RND_AND_KEY2_NUMS || key2Num != RND_AND_KEY2_NUMS) {
        LOGE("rnd and key2 num is not match ! rndNum: %{public}d, key2Num: %{public}d", rndNum, key2Num);
        return E_PARAMS_INVALID;
    }

    for (int i = 0; i < rndNum; ++i) {
        uint8_t *key2 = setRecoverKeyStr.key2FromTee[i];
        uint8_t *originIv = setRecoverKeyStr.rndFromTee[i];
        std::vector<uint8_t> key2Data(key2, key2 + RND_AND_KEY2_LEN);
        std::vector<uint8_t> ivData(originIv, originIv + RND_AND_KEY2_LEN);
        KeyBlob ivBlob(ivData);
        KeyBlob key2Blob(key2Data);
        KeyBlob keyDesc;
        auto errNo = GenerateKeyDesc(ivBlob, keyDesc);
        if (errNo != E_OK) {
            LOGE("Generate key desc failed !");
            return errNo;
        }
        ret = InstallKeyDescToKeyring(ELX_TYPE_ARR[i], key2Blob, keyDesc);
        if (ret != E_OK) {
            ivBlob.Clear();
            keyDesc.Clear();
            key2Blob.Clear();
            LOGE("install type %{public}d to keyring failed !", ELX_TYPE_ARR[i]);
            return ret;
        }
        ivBlob.Clear();
        keyDesc.Clear();
        key2Blob.Clear();
    }
#endif
    LOGI("succeed");
    return 0;
}

int RecoveryManager::SetRecoverKeyToTee(const std::vector<uint8_t> &key, SetRecoverKeyStr &setRecoverKeyStr)
{
    LOGI("enter");
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    TEEC_Context setKeyContext = {};
    TEEC_Session setKeySession = {};
    if (!OpenSession(setKeyContext, setKeySession)) {
        LOGE("Open session failed !");
        return E_RECOVERY_KEY_OPEN_SESSION_ERR;
    }
    TEEC_Operation operation = { 0 };
    operation.started = SESSION_START_DEFAULT;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);
    uint32_t setKeyOrigin;
    operation.params[TEE_PARAM_INDEX_0].tmpref.buffer = static_cast<void *>(const_cast<unsigned char *>(key.data()));
    operation.params[TEE_PARAM_INDEX_0].tmpref.size = key.size();
    operation.params[TEE_PARAM_INDEX_1].tmpref.buffer = static_cast<void *>(&setRecoverKeyStr);
    operation.params[TEE_PARAM_INDEX_1].tmpref.size = sizeof(setRecoverKeyStr);
    TEEC_Result ret = TEEC_InvokeCommand(&setKeySession, TaCmdId::RK_CMD_ID_DECRYPT_CLASS_KEY, &operation,
                                         &setKeyOrigin);
    LOGI("InvokeCmd ret: %{public}d, origin: %{public}d", ret, setKeyOrigin);
    if (ret != TEEC_SUCCESS) {
        LOGE("InvokeCmd failed, ret: %{public}d, origin: %{public}d", ret, setKeyOrigin);
        CloseSession(setKeyContext, setKeySession);
        std::string extraData = "cmd=RK_CMD_ID_DECRYPT_CLASS_KEY,ret=" + std::to_string(ret) +
            ",origin=" + std::to_string(setKeyOrigin);
        StorageRadar::ReportTEEClientResult("SetRecoverKeyToTee::TEEC_InvokeCommand", E_TEEC_DECRYPT_CLASS_KEY_ERR,
            DEFAULT_USERID, extraData);
        return E_TEEC_DECRYPT_CLASS_KEY_ERR;
    }
    CloseSession(setKeyContext, setKeySession);
#endif
    return 0;
}

int32_t RecoveryManager::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType,
    const std::vector<uint8_t> &key, std::vector<KeyBlob> &originIvs)
{
    LOGI("reset secret userId: %{public}d", userId);
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    SetRecoverKeyStr setRecoverKeyStr;
    int ret = ResetSecretWithRecoveryKeyToTee(userId, rkType, key, setRecoverKeyStr);
    if (ret != 0) {
        LOGE("Set recover key to tee failed !");
        return ret;
    }

    if (sizeof(setRecoverKeyStr.key2FromTee) != sizeof(setRecoverKeyStr.rndFromTee)) {
        LOGE("key2 size dose not match iv size !");
        return E_PARAMS_INVALID;
    }
    int rndNum = sizeof(setRecoverKeyStr.rndFromTee) / RND_AND_KEY2_LEN;
    int key2Num = sizeof(setRecoverKeyStr.key2FromTee) / RND_AND_KEY2_LEN;
    if (rndNum != RND_AND_KEY2_NUMS || key2Num != RND_AND_KEY2_NUMS) {
        LOGE("rnd and key2 num is not match ! rndNum: %{public}d, key2Num: %{public}d", rndNum, key2Num);
        return E_PARAMS_INVALID;
    }

    for (int i = 0; i < rndNum; ++i) {
        uint8_t *key2 = setRecoverKeyStr.key2FromTee[i];
        uint8_t *originIv = setRecoverKeyStr.rndFromTee[i];
        std::vector<uint8_t> key2Data(key2, key2 + RND_AND_KEY2_LEN);
        std::vector<uint8_t> ivData(originIv, originIv + RND_AND_KEY2_LEN);
        KeyBlob ivBlob(ivData);
        KeyBlob key2Blob(key2Data);
        KeyBlob keyDesc;
        auto errNo = GenerateKeyDesc(ivBlob, keyDesc);
        if (errNo != E_OK) {
            LOGE("Generate key desc failed !");
            return errNo;
        }
        ret = InstallKeyDescToKeyring(ELX_TYPE_ARR[i], key2Blob, keyDesc);
        if (ret != E_OK) {
            LOGE("install type %{public}d to keyring failed !", ELX_TYPE_ARR[i]);
            return ret;
        }
        originIvs.emplace_back(ivBlob);
    }
#endif
    return E_OK;
}

int32_t RecoveryManager::ResetSecretWithRecoveryKeyToTee(uint32_t userId, uint32_t rkType,
    const std::vector<uint8_t> &key, SetRecoverKeyStr &setRecoverKeyStr)
{
    LOGI("reset secret with recovery key");
#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    TEEC_Context setKeyContext = {};
    TEEC_Session setKeySession = {};
    if (!OpenSession(setKeyContext, setKeySession)) {
        LOGE("Open session failed !");
        return E_RECOVERY_KEY_OPEN_SESSION_ERR;
    }

    TEEC_Operation operation = { 0 };
    operation.started = SESSION_START_DEFAULT;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
        TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE);
    uint32_t setKeyOrigin;
    operation.params[TEE_PARAM_INDEX_0].tmpref.buffer = static_cast<void *>(const_cast<unsigned char *>(key.data()));
    operation.params[TEE_PARAM_INDEX_0].tmpref.size = key.size();
    operation.params[TEE_PARAM_INDEX_1].value.a = userId;
    operation.params[TEE_PARAM_INDEX_1].value.b = rkType;
    operation.params[TEE_PARAM_INDEX_2].tmpref.buffer = static_cast<void *>(&setRecoverKeyStr);
    operation.params[TEE_PARAM_INDEX_2].tmpref.size = sizeof(setRecoverKeyStr);
    TEEC_Result ret = TEEC_InvokeCommand(&setKeySession, TaCmdId::RK_CMD_ID_SET_RK_FOR_PLUGGED_IN_SSD, &operation,
                                         &setKeyOrigin);
    LOGI("InvokeCmd ret: %{public}d, origin: %{public}d", ret, setKeyOrigin);
    if (ret != TEEC_SUCCESS) {
        LOGE("InvokeCmd failed, ret: %{public}d, origin: %{public}d", ret, setKeyOrigin);
        CloseSession(setKeyContext, setKeySession);
        std::string extraData = "cmd=RK_CMD_ID_SET_RK_FOR_PLUGGED_IN_SSD,ret=" + std::to_string(ret) +
            ",origin=" + std::to_string(setKeyOrigin);
        StorageRadar::ReportTEEClientResult("ResetSecretWithRecoveryKeyToTee::TEEC_InvokeCommand",
            E_TEEC_SET_RK_FOR_PLUGGED_IN_SSD_ERR, userId, extraData);
        return E_TEEC_SET_RK_FOR_PLUGGED_IN_SSD_ERR;
    }
    CloseSession(setKeyContext, setKeySession);
#endif
    return E_OK;
}

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
bool RecoveryManager::OpenSession(TEEC_Context &context, TEEC_Session &session)
{
    LOGI("enter");
    if (isSessionOpened) {
        LOGE("Tee session has Opened !");
        return true;
    }
    TEEC_Result ret = TEEC_InitializeContext(NULL, &context);
    if (ret != TEEC_SUCCESS) {
        LOGE("recovery tee ctx init failed !");
        TEEC_FinalizeContext(&context);
        isSessionOpened = false;
        StorageRadar::ReportTEEClientResult("OpenSession::TEEC_InitializeContext", ret, DEFAULT_USERID, "");
        return false;
    }
    TEEC_Operation operation;
    LOGI("Prepare session operation.");
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    if (memset_s(&operation, sizeof(TEEC_Operation), 0, sizeof(TEEC_Operation)) != EOK) {
        LOGE("[OpenSession] memset_s failed !");
        TEEC_FinalizeContext(&context);
        isSessionOpened = false;
        return false;
    }
    LOGI("Prepare open session.");
    operation.started = SESSION_START_DEFAULT;
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    uint32_t retErr = 0;
    uint32_t retryCount = 0;
    while (retryCount < MAX_RETRY_COUNT) {
        ret = TEEC_OpenSession(&context, &session, recoverUuid_, TEEC_LOGIN_IDENTIFY, nullptr, &operation, &retErr);
        if (ret == TEEC_SUCCESS) {
            LOGI("Open session success, has try %{public}u times.", retryCount);
            break;
        }
        retryCount++;
        LOGE("has retry %{public}d times, ret: %{public}d", retryCount, ret);
        usleep(RETRY_INTERVAL);
    }

    if (ret != TEEC_SUCCESS) {
        LOGE("open session failed !");
        CloseSession(context, session);
        StorageRadar::ReportTEEClientResult("OpenSession::TEEC_OpenSession", ret, DEFAULT_USERID, "");
        return false;
    }
    isSessionOpened = true;
    LOGI("open session success");
    return true;
}

void RecoveryManager::CloseSession(TEEC_Context &context, TEEC_Session &session)
{
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
    isSessionOpened = false;
    LOGI("close session success");
}
#endif

int32_t RecoveryManager::GenerateKeyDesc(const KeyBlob &ivBlob, KeyBlob &keyDesc)
{
    LOGI("enter");
    if (ivBlob.IsEmpty()) {
        LOGE("key is empty");
        return E_KEY_BLOB_ERROR;
    }
    SHA512_CTX c;

    SHA512_Init(&c);
    SHA512_Update(&c, ivBlob.data.get(), ivBlob.size);
    uint8_t keyRef1[SHA512_DIGEST_LENGTH] = { 0 };
    SHA512_Final(keyRef1, &c);

    SHA512_Init(&c);
    SHA512_Update(&c, keyRef1, SHA512_DIGEST_LENGTH);
    uint8_t keyRef2[SHA512_DIGEST_LENGTH] = { 0 };
    SHA512_Final(keyRef2, &c);

    static_assert(SHA512_DIGEST_LENGTH >= CRYPTO_KEY_DESC_SIZE, "Hash too short for descriptor");
    keyDesc.Alloc(CRYPTO_KEY_DESC_SIZE);
    auto err = memcpy_s(keyDesc.data.get(), keyDesc.size, keyRef2, CRYPTO_KEY_DESC_SIZE);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return err;
    }
    LOGI("succeed");
    return E_OK;
}

int32_t RecoveryManager::InstallKeyDescToKeyring(size_t keyType, const KeyBlob &key2Blob, const KeyBlob &keyDesc)
{
    int ret = E_OK;
    if (keyType == TYPE_EL3 || keyType == TYPE_EL4) {
        uint32_t sdpClass;
        if (keyType == TYPE_EL3) {
            sdpClass = FSCRYPT_SDP_SECE_CLASS;
        } else {
            sdpClass = FSCRYPT_SDP_ECE_CLASS;
        }
        ret = InstallEceSece(sdpClass, key2Blob, keyDesc);
        if (ret != E_OK) {
            LOGE("InstallKeyDescToKeyring failed");
            return ret;
        }
    } else {
        ret = InstallDeCe(key2Blob, keyDesc);
        if (ret != E_OK) {
            LOGE("InstallKeyToKeyring failed");
            return ret;
        }
    }
    return E_OK;
}


int32_t RecoveryManager::InstallDeCe(const KeyBlob &key2Blob, const KeyBlob &keyDesc)
{
    fscrypt_key fskey;
    fskey.mode = FS_ENCRYPTION_MODE_AES_256_XTS;
    fskey.size = key2Blob.size;
    auto err = memcpy_s(fskey.raw, FS_MAX_KEY_SIZE, key2Blob.data.get(), key2Blob.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return err;
    }

    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("failed to add session keyring");
            return E_ADD_SESSION_KEYRING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyDesc.ToString();
        LOGI("InstallDeCe: prefix: %{public}s", prefix.c_str());
        key_serial_t ks = KeyCtrlAddKeyEx("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey %{public}s to keyring, errno %{public}d", prefix.c_str(), errno);
        }
    }
    LOGI("success");
    return E_OK;
}

int32_t RecoveryManager::InstallEceSece(uint32_t sdpClass, const KeyBlob &key2Blob, const KeyBlob &keyDesc)
{
    EncryptionKeySdp fskey;
    if (key2Blob.size != sizeof(fskey.raw)) {
        LOGE("Wrong key size is %{public}d", key2Blob.size);
        return E_KEY_BLOB_ERROR;
    }
    fskey.mode = EXT4_ENCRYPTION_MODE_AES_256_XTS;
    auto err = memcpy_s(fskey.raw, sizeof(fskey.raw), key2Blob.data.get(), key2Blob.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return err;
    }
    fskey.size = EXT4_AES_256_XTS_KEY_SIZE_TO_KEYRING;
    fskey.sdpClass = sdpClass;
    fskey.version = 0;
    key_serial_t krid = KeyCtrlSearch(KEY_SPEC_SESSION_KEYRING, "keyring", "fscrypt", 0);
    if (krid == -1) {
        LOGI("no session keyring for fscrypt");
        krid = KeyCtrlAddKey("keyring", "fscrypt", KEY_SPEC_SESSION_KEYRING);
        if (krid == -1) {
            LOGE("failed to add session keyring");
            return E_ADD_SESSION_KEYRING_ERROR;
        }
    }
    for (auto prefix : CRYPTO_NAME_PREFIXES) {
        std::string keyref = prefix + ":" + keyDesc.ToString();
        key_serial_t ks = KeyCtrlAddKeySdp("logon", keyref.c_str(), &fskey, krid);
        if (ks == -1) {
            // Addkey failed, need to process the error
            LOGE("Failed to AddKey %{public}s into keyring, errno %{public}d", prefix.c_str(), errno);
        }
    }
    LOGI("success");
    return E_OK;
}
} // namespace StorageDaemon
} // namespace HOHS