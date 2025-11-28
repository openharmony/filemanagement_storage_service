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

#include "os_account_manager.h"
#include "crypto/filesystem_crypto.h"

#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageDaemon;
using namespace OHOS::AccountSA;
using namespace OHOS::StorageService;
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

int32_t FileSystemCrypto::EraseAllUserEncryptedKeys()
{
    LOGI("Enter EraseAllUserEncryptedKeys");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->EraseAllUserEncryptedKeys();
    return err;
}

int32_t FileSystemCrypto::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
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
    err = sdCommunication->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    return err;
}

int32_t FileSystemCrypto::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                       const std::vector<uint8_t> &newSecret,
                                                       uint64_t secureUid,
                                                       uint32_t userId,
                                                       const std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
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

int32_t FileSystemCrypto::LockUserScreen(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->LockUserScreen(userId);
}

int32_t FileSystemCrypto::UnlockUserScreen(uint32_t userId,
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
    return sdCommunication->UnlockUserScreen(userId, token, secret);
}

int32_t FileSystemCrypto::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
}

int32_t FileSystemCrypto::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->GetUserNeedActiveStatus(userId, needActive);
}

int32_t FileSystemCrypto::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->GetLockScreenStatus(userId, lockScreenStatus);
}

int32_t FileSystemCrypto::GenerateAppkey(uint32_t hashId, uint32_t userId, std::string &keyId, bool needReSet)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->GenerateAppkey(userId, hashId, keyId, needReSet);
}

int32_t FileSystemCrypto::DeleteAppkey(const std::string keyId)
{
    std::vector<int32_t> ids;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("Query active userid failed, ret = %{public}u", ret);
        StorageRadar::ReportOsAccountResult("DeleteAppkey::QueryActiveOsAccountIds", ret, DEFAULT_USERID);
        return ret;
    }
    int32_t userId = ids[0];
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->DeleteAppkey(userId, keyId);
}

int32_t FileSystemCrypto::CreateRecoverKey(uint32_t userId,
                                           uint32_t userType,
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
    return sdCommunication->CreateRecoverKey(userId, userType, token, secret);
}

int32_t FileSystemCrypto::SetRecoverKey(const std::vector<uint8_t> &key)
{
    std::vector<int32_t> ids;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != 0 || ids.empty()) {
        LOGE("Query active userid failed, ret = %{public}u", ret);
        StorageRadar::ReportOsAccountResult("SetRecoverKey::QueryActiveOsAccountIds", ret, DEFAULT_USERID);
        return ret;
    }
    int32_t userId = ids[0];
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->SetRecoverKey(key);
}

int32_t FileSystemCrypto::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->ResetSecretWithRecoveryKey(userId, rkType, key);
    return err;
}

int32_t FileSystemCrypto::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateKeyContext(userId, needRemoveTmpKey);
    return err;
}

int32_t FileSystemCrypto::InactiveUserPublicDirKey(uint32_t userId)
{
    LOGI("UserId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->InactiveUserPublicDirKey(userId);
    return err;
}

int32_t FileSystemCrypto::UpdateUserPublicDirPolicy(uint32_t userId)
{
    LOGI("Update policy userId: %{public}u", userId);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->UpdateUserPublicDirPolicy(userId);
    return err;
}

int32_t FileSystemCrypto::RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback)
{
    LOGI("Enter RegisterUeceActivationCallback");
    if (ueceCallback == nullptr) {
        LOGE("callback is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->RegisterUeceActivationCallback(ueceCallback);
}
 
int32_t FileSystemCrypto::UnregisterUeceActivationCallback()
{
    LOGI("Enter UnregisterUeceActivationCallback");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->UnregisterUeceActivationCallback();
}

int32_t FileSystemCrypto::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    LOGI("Enter SetDirEncryptionPolicy.");
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("User ID out of range");
        return err;
    }
    
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    return sdCommunication->SetDirEncryptionPolicy(userId, dirPath, level);
}
}
}
