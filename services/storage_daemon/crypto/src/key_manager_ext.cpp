/*
 * Copyright (C) 2025-2025 Huawei Device Co., Ltd.
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
// typedef UserkeyExtInterface* (*GetExtInstance)(void);
typedef void* (*GetExtInstance)(void);
}

UserkeyExt::UserkeyExt()
{
    Init();

    LOGI("Instance created.");
}

UserkeyExt::~UserkeyExt()
{
    UnInit();

    if (handler_) {
        dlclose(handler_);
        handler_ = nullptr;
    }
    LOGI("Instance destroyed.");
}

int32_t UserkeyExt::Init()
{
    LOGI("Ready to init.");

    handler_ = dlopen("/system/lib64/libspace_mgr_ext.z.so", RTLD_LAZY);
    if (handler_ == nullptr) {
        LOGE("Policy not exist, just start service.");
        return E_SYS_KERNEL_ERR;
    }

    GetExtInstance fnInstance = reinterpret_cast<GetExtInstance>(dlsym(handler_, "GetUserKeyExtInstance"));
    if (fnInstance == nullptr) {
        LOGE("GetExtInstance failed.");
        return E_SYS_KERNEL_ERR;
    }

    service_ = static_cast<UserkeyExtInterface*>(fnInstance());
    if (service_ == nullptr) {
        LOGE("User key Ext instance is null.");
        return E_SYS_KERNEL_ERR;
    }

    return E_OK;
}

void UserkeyExt::UnInit()
{
    LOGI("UnInit start");
    if (service_) {
        service_->UnInit();
    }
}

int32_t UserkeyExt::GenerateUserKey(int32_t userId, const std::vector<uint8_t>& keyInfo)
{
    if (service_ == nullptr) {
        LOGE("Failed to get policy.");
        return E_OK;
    }

    LOGI("Generate user key for %{public}d.", userId);
    return service_->GenerateUserKey(userId, keyInfo);
}

int32_t UserkeyExt::ActiveUserKey(uint32_t userId, const std::vector<uint8_t>& keyInfo,
        const std::vector<uint8_t>& token)
{
    if (service_ == nullptr) {
        LOGE("Failed to get policy.");
        return E_OK;
    }

    LOGI("active %{public}d's user key.", userId);
    return service_->ActiveUserKey(userId, keyInfo, token);
}

int32_t UserkeyExt::GenerateUserKeyWithToken(int32_t userId,
    const std::vector<uint8_t>& keyInfo, const std::vector<uint8_t>& token)
{
    if (service_ == nullptr) {
        LOGE("Failed to get policy.");
        return E_OK;
    }

    LOGI("Generate user key for %{public}d with token.", userId);
    return service_->ActiveUserKey(userId, keyInfo, token);
}

int32_t UserkeyExt::InactiveUserKey(int32_t userId)
{
    if (service_ == nullptr) {
        LOGE("Failed to get policy.");
        return E_OK;
    }

    LOGI("Inactive %{public}d's user key.", userId);
    return service_->InactiveUserKey(userId);
}

int32_t UserkeyExt::DeleteUserKey(int32_t userId)
{
    if (service_ == nullptr) {
        LOGE("Failed to get policy.");
        return E_OK;
    }

    LOGI("Delete %{public}d's user key.", userId);
    return service_->DeleteUserKey(userId);
}

int32_t UserkeyExt::SetFilePathPolicy(uint32_t userId)
{
    if (service_ == nullptr) {
        LOGE("Failed to get policy.");
        return E_OK;
    }

    return service_->SetFilePathPolicy(userId);
}

std::shared_ptr<BaseKey> KeyManagerExt::GetBaseKey(uint32_t userId, KeyType type)
{
    if (!KeyManager::GetInstance()->HashElxActived(userId, type)) {
        LOGE("user el%{public}u key is not existed", type);
        return nullptr;
    }
    std::shared_ptr<BaseKey> elKey = KeyManager::GetInstance()->GetUserElKey(userId, type);
    if (elKey == nullptr) {
        LOGE("Have not found user %{public}u, type el%{public}u", userId, type);
        return nullptr;
    }
    return elKey;
}

int KeyManagerExt::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsEnabled()) {
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

    ret = LoadAndSetPolicy(userId);
    if (ret != E_OK) {
        return ret;
    }

    LOGI("Create user key success");
    return ret;
}

int KeyManagerExt::DeleteUserKeys(uint32_t userId)
{
    LOGI("start, user:%{public}d", userId);
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = DoDeleteUserKeys(userId);
    LOGI("delete user key end, ret is %{public}d", ret);
    return ret;
}

int KeyManagerExt::ActiveUserKey(uint32_t userId,
                                 const std::vector<uint8_t>& token,
                                 const std::vector<uint8_t>& secret)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::shared_ptr<BaseKey> elKey = GetBaseKey(userId, EL2_KEY);
    if (elKey == nullptr) {
        LOGE("elKey failed");
        return E_PARAMS_INVALID;
    }

    if (ActiveUserElKey(userId, token, secret, elKey) != 0) {
        LOGE("Active user El Key failed");
        return E_ELX_KEY_ACTIVE_ERROR;
    }

    LOGI("Active user %{public}u key success", userId);
    return E_OK;
}

int KeyManagerExt::InActiveUserKey(uint32_t userId)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = InactiveUserElKey(userId);
    if (ret != E_OK) {
        LOGE("Inactive user El key failed, ret: %{public}d", ret);
        return ret;
    }
    LOGI("Inactive user %{public}u key success", userId);
    return E_OK;
}

int KeyManagerExt::LockUserScreen(uint32_t userId)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = InactiveUserElKey(userId);
    if (ret != E_OK) {
        LOGE("lock user screen error, ret: %{public}d", ret);
        return ret;
    }

    LOGI("lock user %{public}u screen success", userId);
    return E_OK;
}

int KeyManagerExt::UnlockUserScreen(uint32_t userId, 
    const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(keyMutex_);
    std::shared_ptr<BaseKey> elKey = GetBaseKey(userId, EL2_KEY);
    if (elKey == nullptr) {
        LOGE("elKey failed");
        return E_PARAMS_INVALID;
    }

    if (ActiveUserElKey(userId, token, secret, elKey) != 0) {
        LOGE("Active user El Key failed");
        return E_ELX_KEY_ACTIVE_ERROR;
    }
    LOGI("unlock user %{public}u screen success", userId);
    return E_OK;
}

int KeyManagerExt::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("start");
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }
    return E_OK;
}

int KeyManagerExt::InactiveUserPublicDirKey(uint32_t userId)
{
    LOGI("start, user:%{public}u", userId);
    if (!IsEnabled()) {
        LOGI("user key ext policy is disabled");
        return E_OK;
    }
    if (!KeyCtrlHasFscryptSyspara()) {
        return E_OK;
    }

    std::lock_guard<std::mutex> lock(keyMutex_);
    int ret = userKeyExt_.InactiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("Inactive user key error, ret: %{public}d", ret);
    }

    LOGI("Inactive user %{public}u elX success", userId);
    return E_OK;
}

// private
int KeyManagerExt::GenerateAndInstallUserKey(uint32_t userId)
{
    std::shared_ptr<BaseKey> elKey = GetBaseKey(userId, EL2_KEY);
    if (elKey == nullptr) {
        LOGE("elKey failed");
        return E_PARAMS_INVALID;
    }

    KeyBlob hashKey;
    if (!elKey->GetHashKey(hashKey)) {
        LOGE("get origin hash key failed !");
        return E_PARAMS_INVALID;
    }

    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    int ret = userKeyExt_.GenerateUserKey(userId, std::move(keyVec));
    if (ret != E_OK) {
        LOGE("Generate user key error, ret: %{public}d", ret);
    }
    hashKey.Clear();
    return ret;
}

int KeyManagerExt::InactiveUserElKey(uint32_t userId)
{
    int ret = userKeyExt_.InactiveUserKey(userId);
    if (ret != E_OK) {
        LOGE("Inactive user key error, ret: %{public}d", ret);
        return ret;
    }

    LOGI("Inactive user %{public}u key success", userId);
    return ret;
}

int KeyManagerExt::ActiveUserElKey(uint32_t userId, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey)
{
    KeyBlob hashKey;
    if (!elKey->GetHashKey(hashKey)) {
        LOGE("get origin hash key failed !");
        return E_PARAMS_INVALID;
    }

    std::vector<uint8_t> keyVec(hashKey.data.get(), hashKey.data.get() + hashKey.size);
    int ret = userKeyExt_.ActiveUserKey(userId, std::move(keyVec), token);
    if (ret != E_OK) {
        LOGE("Active user key with token error, ret: %{public}d", ret);
    }
    hashKey.Clear();
    return ret;
}

int KeyManagerExt::DoDeleteUserKeys(uint32_t userId)
{
    int ret = userKeyExt_.DeleteUserKey(userId);
    if (ret != E_OK) {
        LOGE("del user key error, ret: %{public}d", ret);
    }
    return ret;
}

int KeyManagerExt::LoadAndSetPolicy(uint32_t userId)
{
    int ret = userKeyExt_.SetFilePathPolicy(userId);
    if (ret != E_OK) {
        LOGE("Set directory policy error, ret: %{public}d", ret);
    }
    return ret;
}

} // namespace StorageDaemon
} // namespace OHOS
