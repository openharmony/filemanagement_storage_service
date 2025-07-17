/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

KeyManagerExt::KeyManagerExt()
{
    Init();
    LOGI("Instance created.");
}

KeyManagerExt::~KeyManagerExt()
{
    UnInit();
    LOGI("Instance destroyed.");
}

void KeyManagerExt::Init()
{
    if (!KeyCtrlHasFscryptSyspara()) {
        LOGE("FscryptSyspara has not enabled");
        return;
    }
    handler_ = dlopen("/system/lib64/libspace_mgr_ext.z.so", RTLD_LAZY);
    if (handler_ == nullptr) {
        LOGE("Policy not exist, just start service.");
        return;
    }

    GetExtInstance fnInstance = reinterpret_cast<GetExtInstance>(dlsym(handler_, "GetUserKeyExtInstance"));
    if (fnInstance == nullptr) {
        LOGE("GetExtInstance failed.");
        return;
    }

    service_ = static_cast<UserkeyExtInterface*>(fnInstance());
    if (service_ == nullptr) {
        LOGE("User key Ext instance is null.");
    }
}

void KeyManagerExt::UnInit()
{
    LOGI("UnInit start");
    if (handler_) {
        dlclose(handler_);
        handler_ = nullptr;
    }
    service_ = nullptr;
}

int KeyManagerExt::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    if ((flags & IStorageDaemonEnum::CRYPTO_FLAG_EL2) != IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
        LOGI("Not specify el2 flags");
        return E_OK;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = GenerateAndInstallUserKey(userId);
    if (ret != E_OK) {
        return ret;
    }

    LOGI("Create user key success");
    return ret;
}

int KeyManagerExt::DeleteUserKeys(uint32_t userId)
{
    LOGI("start, user:%{public}d", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoDeleteUserKeys(userId);
    if (ret != E_OK) {
        return ret;
    }
    LOGI("delete user key success");
    return ret;
}

int KeyManagerExt::ActiveUserKey(uint32_t userId,
                                 const std::vector<uint8_t>& token,
                                 const std::vector<uint8_t>& secret)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoActiveUserKey(userId, token, secret);
    if (ret != E_OK) {
        return ret;
    }

    LOGI("Active user %{public}u key success", userId);
    return ret;
}

int KeyManagerExt::InActiveUserKey(uint32_t userId)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsServiceExtSoLoaded()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoInactiveUserKey(userId);
    if (ret != E_OK) {
        return ret;
    }
    LOGI("Inactive user %{public}u key success", userId);
    return E_OK;
}

int KeyManagerExt::GetHashKey(uint32_t userId, KeyType type, KeyBlob& hashKey)
{
    if (!KeyManager::GetInstance().HasElkey(userId, type)) {
        LOGE("user el%{public}u key is not existed", type);
        return E_KEY_EMPTY_ERROR;
    }
    std::shared_ptr<BaseKey> elKey = KeyManager::GetInstance().GetUserElKey(userId, type);
    if (elKey == nullptr) {
        LOGE("Have not found user %{public}u, type el%{public}u", userId, type);
        return E_KEY_EMPTY_ERROR;
    }

    if (!elKey->GetHashKey(hashKey) || hashKey.IsEmpty()) {
        LOGE("get origin hash key failed !");
        return E_KEY_EMPTY_ERROR;
    }
    return E_OK;
}

int KeyManagerExt::GenerateAndInstallUserKey(uint32_t userId)
{
    KeyBlob hashKey;
    int ret = GetHashKey(userId, EL2_KEY, hashKey);
    if (ret != E_OK) {
        LOGE("Generate user key error, ret: %{public}d", ret);
        return ret;
    }

    LOGI("Generate user key for %{public}d.", userId);
    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    ret = service_->GenerateUserKey(userId, std::move(keyVec));
    if (ret != E_OK) {
        LOGE("Generate user key error, ret: %{public}d", ret);
        return ret;
    }
    hashKey.Clear();

    ret = service_->SetFilePathPolicy(userId);
    if (ret != E_OK) {
        LOGE("Set directory policy error, ret: %{public}d", ret);
    }
    return ret;
}

int KeyManagerExt::DoInactiveUserKey(uint32_t userId)
{
    int ret = service_->InactiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("Inactive user key error, ret: %{public}d", ret);
        return ret;
    }
    return ret;
}

int KeyManagerExt::DoActiveUserKey(uint32_t userId,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &secret)
{
    KeyBlob hashKey;
    int ret = GetHashKey(userId, EL2_KEY, hashKey);
    if (ret != E_OK) {
        LOGE("Generate user key error, ret: %{public}d", ret);
        return ret;
    }

    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    ret = service_->ActiveUserKey(userId, std::move(keyVec), token);
    if (ret != E_OK) {
        LOGE("Active user key with token error, ret: %{public}d", ret);
    }
    hashKey.Clear();
    return ret;
}

int KeyManagerExt::DoDeleteUserKeys(uint32_t userId)
{
    int ret = service_->DeleteUserKey(userId);
    if (ret != E_OK) {
        LOGE("del user key error, ret: %{public}d", ret);
    }
    return ret;
}

} // namespace StorageDaemon
} // namespace OHOS
