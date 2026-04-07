/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "key_manager_ext.h"

#include "directory_ex.h"
#include "file_ex.h"
#include "key_manager.h"
#include "iam_client.h"
#include "libfscrypt/fscrypt_control.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {

namespace {
typedef UserkeyExtInterface* (*GetExtInstance)(void);
}

constexpr uint32_t TYPE_EL2 = 1;
const std::vector<uint8_t> DEFAULT_KEY = { 'D', 'o', 'c', 's' };

KeyManagerExt::KeyManagerExt()
{
    LOGI("[L3:KeyManagerExt] KeyManagerExt: >>> ENTER <<< Constructor");
    Init();
    LOGI("[L3:KeyManagerExt] KeyManagerExt: <<< EXIT SUCCESS <<< Instance created");
}

KeyManagerExt::~KeyManagerExt()
{
    LOGI("[L3:KeyManagerExt] ~KeyManagerExt: >>> ENTER <<< Destructor");
    UnInit();
    LOGI("[L3:KeyManagerExt] ~KeyManagerExt: <<< EXIT SUCCESS <<< Instance destroyed");
}

void KeyManagerExt::Init()
{
    if (handler_ != nullptr || service_ != nullptr) {
        LOGE("[L3:KeyManagerExt] Init: <<< EXIT FAILED <<< FscryptSyspara has already initialized.");
        return;
    }

    if (!KeyCtrlHasFscryptSyspara()) {
        LOGE("[L3:KeyManagerExt] Init: <<< EXIT FAILED <<< FscryptSyspara has not enabled");
        return;
    }
    handler_ = dlopen("/system/lib64/libspace_mgr_ext.z.so", RTLD_LAZY);
    if (handler_ == nullptr) {
        LOGE("[L3:KeyManagerExt] Init: Policy not exist, just start service");
        return;
    }

    GetExtInstance fnInstance = reinterpret_cast<GetExtInstance>(dlsym(handler_, "GetUserKeyExtInstance"));
    if (fnInstance == nullptr) {
        LOGE("[L3:KeyManagerExt] Init: <<< EXIT FAILED <<< GetExtInstance failed");
        dlclose(handler_);
        handler_ = nullptr;
        return;
    }

    service_ = static_cast<UserkeyExtInterface*>(fnInstance());
    if (service_ == nullptr) {
        LOGE("[L3:KeyManagerExt] Init: <<< EXIT FAILED <<< User key Ext instance is null");
        dlclose(handler_);
        handler_ = nullptr;
        return;
    }
}

void KeyManagerExt::UnInit()
{
    LOGI("[L3:KeyManagerExt] UnInit: >>> ENTER <<<");
    if (handler_) {
        dlclose(handler_);
        handler_ = nullptr;
    }
    service_ = nullptr;
    LOGI("[L3:KeyManagerExt] UnInit: <<< EXIT SUCCESS <<<");
}

int KeyManagerExt::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("[L3:KeyManagerExt] GenerateUserKeys: >>> ENTER <<< userId=%{public}u, flags=%{public}u", userId, flags);
    if (!IsServiceExtSoLoaded()) {
        LOGI("[L3:KeyManagerExt] GenerateUserKeys: <<< EXIT SUCCESS <<< user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    if ((flags & IStorageDaemonEnum::CRYPTO_FLAG_EL2) != IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        LOGI("[L3:KeyManagerExt] GenerateUserKeys: <<< EXIT SUCCESS <<< Not specify el2 flags");
        return E_OK;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = GenerateAndInstallUserKey(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] GenerateUserKeys: <<< EXIT FAILED <<< GenerateAndInstallUserKey failed,"
             "userId=%{public}u, ret=%{public}d", userId, ret);
        return ret;
    }

    LOGI("[L3:KeyManagerExt] GenerateUserKeys: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}

int KeyManagerExt::DeleteUserKeys(uint32_t userId)
{
    LOGI("[L3:KeyManagerExt] DeleteUserKeys: >>> ENTER <<< userId=%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("[L3:KeyManagerExt] DeleteUserKeys: <<< EXIT SUCCESS <<< user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGI("[L3:KeyManagerExt] DeleteUserKeys: <<< EXIT SUCCESS <<< FscryptSyspara not enabled");
        return E_OK;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoDeleteUserKeys(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] DeleteUserKeys: <<< EXIT FAILED <<< DoDeleteUserKeys failed, userId=%{public}u,"
             "ret=%{public}d", userId, ret);
        return ret;
    }
    LOGI("[L3:KeyManagerExt] DeleteUserKeys: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}

int KeyManagerExt::ActiveUserKey(uint32_t userId,
                                 const std::vector<uint8_t>& token,
                                 const std::vector<uint8_t>& secret)
{
    LOGI("[L3:KeyManagerExt] ActiveUserKey: >>> ENTER <<< userId=%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("[L3:KeyManagerExt] ActiveUserKey: <<< EXIT SUCCESS <<< user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGI("[L3:KeyManagerExt] ActiveUserKey: <<< EXIT SUCCESS <<< FscryptSyspara not enabled");
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoActiveUserKey(userId, token, secret);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] ActiveUserKey: <<< EXIT FAILED <<< DoActiveUserKey failed, userId=%{public}u,"
             "ret=%{public}d", userId, ret);
        return ret;
    }

    LOGI("[L3:KeyManagerExt] ActiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}

int KeyManagerExt::InActiveUserKey(uint32_t userId)
{
    LOGI("[L3:KeyManagerExt] InActiveUserKey: >>> ENTER <<< userId=%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("[L3:KeyManagerExt] InActiveUserKey: <<< EXIT SUCCESS <<< user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGI("[L3:KeyManagerExt] InActiveUserKey: <<< EXIT SUCCESS <<< FscryptSyspara not enabled");
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoInactiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] InActiveUserKey: <<< EXIT FAILED <<< DoInactiveUserKey failed, userId=%{public}u,"
            "ret=%{public}d", userId, ret);
        return ret;
    }
    LOGI("[L3:KeyManagerExt] InActiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int KeyManagerExt::SetRecoverKey(uint32_t userId, uint32_t keyType, const KeyBlob& ivBlob)
{
    LOGI("[L3:KeyManagerExt] SetRecoverKey: >>> ENTER <<< userId=%{public}u, keyType=%{public}u", userId, keyType);
    if (!IsServiceExtSoLoaded()) {
        LOGI("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT SUCCESS <<< user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGI("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT SUCCESS <<< FscryptSyspara not enabled");
        return E_OK;
    }
    if (keyType != TYPE_EL2) {
        LOGI("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT SUCCESS <<< keyType=%{public}u not EL2", keyType);
        return E_OK;
    }
    if (ivBlob.data == nullptr || ivBlob.size == 0) {
        LOGE("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT failed <<< Invalid ivBlob, size:%{public}u", ivBlob.size);
        return E_PARAMS;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    KeyBlob preKey(DEFAULT_KEY);
    KeyBlob hashKey = OpensslCrypto::HashWithPrefix(preKey, ivBlob, AES_256_HASH_RANDOM_SIZE);
    if (hashKey.IsEmpty()) {
        LOGE("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT failed <<< HashWithPrefix failed, hashKey is empty");
        hashKey.Clear();
        return E_KEY_EMPTY_ERROR;
    }
    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    int ret = service_->SetRecoverKey(userId, std::move(keyVec));
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT FAILED <<< set recover key error, userId=%{public}u,"
            "ret=%{public}d", userId, ret);
        hashKey.Clear();
        return ret;
    }

    hashKey.Clear();
    LOGI("[L3:KeyManagerExt] SetRecoverKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int KeyManagerExt::GetHashKey(uint32_t userId, KeyType type, KeyBlob& hashKey)
{
    LOGI("[L3:KeyManagerExt] GetHashKey: >>> ENTER <<< userId=%{public}u, type=%{public}u", userId, type);
    if (!KeyManager::GetInstance().HasElkey(userId, type)) {
        LOGE("[L3:KeyManagerExt] GetHashKey: <<< EXIT FAILED <<< user el%{public}u key is not existed,"
            "userId=%{public}u", type, userId);
        return E_KEY_EMPTY_ERROR;
    }
    std::shared_ptr<BaseKey> elKey = KeyManager::GetInstance().GetUserElKey(userId, type);
    if (elKey == nullptr) {
        LOGE("[L3:KeyManagerExt] GetHashKey: <<< EXIT FAILED <<< Have not found user %{public}u, type el%{public}u",
            userId, type);
        return E_KEY_EMPTY_ERROR;
    }

    if (!elKey->GetHashKey(hashKey) || hashKey.IsEmpty()) {
        LOGE("[L3:KeyManagerExt] GetHashKey: <<< EXIT FAILED <<< get origin hash key failed, userId=%{public}u,"
            "type=%{public}u", userId, type);
        return E_KEY_EMPTY_ERROR;
    }
    LOGI("[L3:KeyManagerExt] GetHashKey: <<< EXIT SUCCESS <<< userId=%{public}u, type=%{public}u", userId, type);
    return E_OK;
}

int KeyManagerExt::GenerateAndInstallUserKey(uint32_t userId)
{
    LOGI("[L3:KeyManagerExt] GenerateAndInstallUserKey: >>> ENTER <<< userId=%{public}u", userId);
    KeyBlob hashKey;
    int ret = GetHashKey(userId, EL2_KEY, hashKey);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] GenerateAndInstallUserKey: <<< EXIT FAILED <<< GetHashKey failed, userId=%{public}u,"
            "ret=%{public}d", userId, ret);
        return ret;
    }

    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    ret = service_->GenerateUserKey(userId, std::move(keyVec));
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] GenerateAndInstallUserKey: <<< EXIT FAILED <<< Generate user key error,"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        hashKey.Clear();
        return ret;
    }
    hashKey.Clear();

    ret = service_->SetFilePathPolicy(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] GenerateAndInstallUserKey: <<< EXIT FAILED <<< Set directory policy error,"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        return ret;
    }
    LOGI("[L3:KeyManagerExt] GenerateAndInstallUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}

int KeyManagerExt::DoInactiveUserKey(uint32_t userId)
{
    LOGI("[L3:KeyManagerExt] DoInactiveUserKey: >>> ENTER <<< userId=%{public}u", userId);
    int ret = service_->InactiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] DoInactiveUserKey: <<< EXIT FAILED <<< Inactive user key error, userId=%{public}u,"
            "ret=%{public}d", userId, ret);
        return ret;
    }
    LOGI("[L3:KeyManagerExt] DoInactiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}

int KeyManagerExt::DoActiveUserKey(uint32_t userId,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &secret)
{
    LOGI("[L3:KeyManagerExt] DoActiveUserKey: >>> ENTER <<< userId=%{public}u", userId);
    KeyBlob hashKey;
    int ret = GetHashKey(userId, EL2_KEY, hashKey);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] DoActiveUserKey: <<< EXIT FAILED <<< GetHashKey failed, userId=%{public}u,"
            "ret=%{public}d", userId, ret);
        return ret;
    }

    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    ret = service_->ActiveUserKey(userId, std::move(keyVec), token);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] DoActiveUserKey: <<< EXIT FAILED <<< Active user key with token error,"
            "userId=%{public}u, ret=%{public}d", userId, ret);
    }
    hashKey.Clear();
    if (ret == E_OK) {
        LOGI("[L3:KeyManagerExt] DoActiveUserKey: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    }
    return ret;
}

int KeyManagerExt::DoDeleteUserKeys(uint32_t userId)
{
    LOGI("[L3:KeyManagerExt] DoDeleteUserKeys: >>> ENTER <<< userId=%{public}u", userId);
    int ret = service_->DeleteUserKey(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] DoDeleteUserKeys: <<< EXIT FAILED <<< del user key error, userId=%{public}u,"
            "ret=%{public}d", userId, ret);
    } else {
        LOGI("[L3:KeyManagerExt] DoDeleteUserKeys: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    }
    return ret;
}

int KeyManagerExt::UpdateUserPublicDirPolicy(uint32_t userId)
{
    LOGI("[L3:KeyManagerExt] UpdateUserPublicDirPolicy: >>> ENTER <<< userId=%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGE("[L3:KeyManagerExt] UpdateUserPublicDirPolicy: <<< EXIT SUCCESS <<< user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGI("[L3:KeyManagerExt] UpdateUserPublicDirPolicy: <<< EXIT SUCCESS <<< FscryptSyspara has not or encryption"
            "not enabled");
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = service_->UpdateUserPublicDirPolicy(userId);
    if (ret != E_OK) {
        LOGE("[L3:KeyManagerExt] UpdateUserPublicDirPolicy: <<< EXIT FAILED <<< Update public dir policy failed,"
            "userId=%{public}u, ret=%{public}d", userId, ret);
        return ret;
    }
    LOGI("[L3:KeyManagerExt] UpdateUserPublicDirPolicy: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS
