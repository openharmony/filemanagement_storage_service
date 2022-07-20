/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "crypto/filesystem_crypto.h"

#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
FileSystemCrypto::FileSystemCrypto()
{
    LOGI("DEBUG FileSystemCrypto constructer");
}

FileSystemCrypto::~FileSystemCrypto()
{
    LOGI("DEBUG ~FileSystemCrypto destructer ~");
}

int32_t FileSystemCrypto::CheckUserIdRange(int32_t userId)
{
    if (userId < StorageService::START_USER_ID || userId > StorageService::MAX_USER_ID) {
        LOGE("FileSystemCrypto: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

int32_t FileSystemCrypto::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    LOGI("UserId: %{public}u, flags:  %{public}u", userId, flags);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GenerateUserKeys(userId, flags);
    return err;
}

int32_t FileSystemCrypto::DeleteUserKeys(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->DeleteUserKeys(userId);
    return err;
}

int32_t FileSystemCrypto::UpdateUserAuth(uint32_t userId,
                                         const std::vector<uint8_t> &token,
                                         const std::vector<uint8_t> &oldSecret,
                                         const std::vector<uint8_t> &newSecret)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateUserAuth(userId, token, oldSecret, newSecret);
    return err;
}

int32_t FileSystemCrypto::ActiveUserKey(uint32_t userId,
                                        const std::vector<uint8_t> &token,
                                        const std::vector<uint8_t> &secret)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->ActiveUserKey(userId, token, secret);
    return err;
}

int32_t FileSystemCrypto::InactiveUserKey(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->InactiveUserKey(userId);
    return err;
}

int32_t FileSystemCrypto::UpdateKeyContext(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateKeyContext(userId);
    return err;
}
}
}
