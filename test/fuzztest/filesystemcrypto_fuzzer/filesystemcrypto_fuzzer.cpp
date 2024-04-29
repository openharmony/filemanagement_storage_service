/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "filesystemcrypto_fuzzer.h"
#include "crypto/filesystem_crypto.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "vector"

namespace OHOS {
namespace StorageManager {

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

std::shared_ptr<FileSystemCrypto> fileSystem =
        DelayedSingleton<FileSystemCrypto>::GetInstance();

bool GenerateUserKeysFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t) + sizeof(uint32_t)) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t flags = TypeCast<uint32_t>(data + pos);

    int32_t result = fileSystem->GenerateUserKeys(userId, flags);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::GenerateUserKeysTest failed!");
        return false;
    }
    return true;
}

bool DeleteUserKeysFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t)) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    int32_t result = fileSystem->DeleteUserKeys(userId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::DeleteUserKeys failed!");
        return false;
    }
    return true;
}

bool UpdateUserAuthFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t) + sizeof(uint64_t)) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos);

    std::vector<uint8_t> token;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    token.push_back(*data);
    oldSecret.push_back(*data);
    newSecret.push_back(*data);

    int32_t result = fileSystem->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::UpdateUserAuth failed!");
        return false;
    }
    return true;
}

bool ActiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(uint32_t))) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    token.push_back(*data);
    secret.push_back(*data);

    int32_t result = fileSystem->ActiveUserKey(userId, token, secret);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::ActiveUserKey failed!");
        return false;
    }
    return true;
}

bool InactiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(uint32_t))) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    int32_t result = fileSystem->InactiveUserKey(userId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::InactiveUserKey failed!");
        return false;
    }
    return true;
}

bool UpdateKeyContextFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(uint32_t))) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    int32_t result = fileSystem->UpdateKeyContext(userId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::UpdateKeyContext failed!");
        return false;
    }
    return true;
}

bool LockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(uint32_t))) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    int32_t result = fileSystem->LockUserScreen(userId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::LockUserScreen failed!");
        return false;
    }
    return true;
}

bool UnlockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(uint32_t))) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);

    int32_t result = fileSystem->UnlockUserScreen(userId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::UnlockUserScreen failed!");
        return false;
    }
    return true;
}

bool GetLockScreenStatusFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t) + sizeof(bool)) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    bool lockScreenStatus = TypeCast<bool>(data + pos);

    int32_t result = fileSystem->GetLockScreenStatus(userId, lockScreenStatus);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::GetLockScreenStatus failed!");
        return false;
    }
    return true;
}

bool GenerateAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t)) {
        return false;
    }

    int pos = 0;
    uint32_t appUid = TypeCast<uint32_t>(data, &pos);
    std::string keyId;
    int32_t result = fileSystem->GenerateAppkey(appUid, keyId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::GenerateAppkey failed!");
        return false;
    }
    return true;
}

bool DeleteAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t)) {
        return false;
    }

    int pos = 0;
    const std::string keyId = TypeCast<std::string>(data, &pos);
    int32_t result = fileSystem->DeleteAppkey(keyId);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::DeleteAppkey failed!");
        return false;
    }
    return true;
}
} // namespace StorageManager
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StorageManager::GenerateUserKeysFuzzTest(data, size);
    OHOS::StorageManager::DeleteUserKeysFuzzTest(data, size);
    OHOS::StorageManager::UpdateUserAuthFuzzTest(data, size);
    OHOS::StorageManager::ActiveUserKeyFuzzTest(data, size);
    OHOS::StorageManager::InactiveUserKeyFuzzTest(data, size);
    OHOS::StorageManager::UpdateKeyContextFuzzTest(data, size);
    OHOS::StorageManager::LockUserScreenFuzzTest(data, size);
    OHOS::StorageManager::UnlockUserScreenFuzzTest(data, size);
    OHOS::StorageManager::GetLockScreenStatusFuzzTest(data, size);
    OHOS::StorageManager::GenerateAppkeyFuzzTest(data, size);
    OHOS::StorageManager::DeleteAppkeyFuzzTest(data, size);

    return 0;
}
