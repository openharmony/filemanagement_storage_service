/*
 * Copyright (C) 2023-2025 Huawei Device Co., Ltd.
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

#include "crypto/key_manager.h"

#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {

int32_t flag_num_sec = 2;
int32_t flag_num_thir = 3;
int32_t main_user = 100;
int32_t KeyManager::InitGlobalDeviceKey(void)
{
    return E_OK;
}

int32_t KeyManager::InitGlobalUserKeys(void)
{
    return E_OK;
}

int32_t KeyManager::GenerateUserKeys(unsigned int user, uint32_t flags)
{
    return E_OK;
}

int32_t KeyManager::DeleteUserKeys(unsigned int user)
{
    return E_OK;
}

int32_t KeyManager::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
    const std::vector<uint8_t> &newSecret, uint64_t secureUid, uint32_t userId,
    const std::vector<std::vector<uint8_t>> &plainText)
{
    return E_OK;
}


#ifdef USER_CRYPTO_MIGRATE_KEY

int KeyManager::RestoreUserKey(uint32_t userId, KeyType type)
{
    return E_OK;
}

int KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
    bool needGenerateShield)
{
    return E_OK;
}
#else
int KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret)
{
    return E_OK;
}
#endif

std::string KeyManager::GetNatoNeedRestorePath(uint32_t userId, KeyType type)
{
    return "";
}

std::string KeyManager::GetKeyDirByUserAndType(unsigned int user, KeyType type)
{
    return "";
}
std::string KeyManager::GetKeyDirByType(KeyType type)
{
    return "";
}

int KeyManager::TryToFixUserCeEceSeceKey(unsigned int userId, KeyType type,
                                         const std::vector<uint8_t> &token,
                                         const std::vector<uint8_t> &secret)
{
    return E_OK;
}
int KeyManager::TryToFixUeceKey(unsigned int userId,
                                const std::vector<uint8_t> &token,
                                const std::vector<uint8_t> &secret)
{
    return E_OK;
}
int KeyManager::ActiveElxUserKey4Nato(unsigned int user, KeyType type, const KeyBlob &authToken)
{
    return E_OK;
}

#ifdef USER_CRYPTO_MIGRATE_KEY
int KeyManager::UpdateCeEceSeceUserAuth(unsigned int user,
                                        struct UserTokenSecret &userTokenSecret,
                                        KeyType type, bool needGenerateShield)
#else
int KeyManager::UpdateCeEceSeceUserAuth(unsigned int user,
                                        struct UserTokenSecret &userTokenSecret,
                                        KeyType type)
#endif
{
    return E_OK;
}

int KeyManager::UpdateCeEceSeceKeyContext(uint32_t userId, KeyType type)
{
    return E_OK;
}

int32_t KeyManager::InActiveUserKey(unsigned int user)
{
    return E_OK;
}

int32_t KeyManager::LockUserScreen(uint32_t user)
{
    return E_OK;
}

int32_t KeyManager::UnlockUserScreen(uint32_t user, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int32_t KeyManager::GetLockScreenStatus(uint32_t user, bool &lockScreenStatus)
{
    return E_OK;
}

int32_t KeyManager::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    return E_OK;
}

int32_t KeyManager::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    return E_OK;
}

int32_t KeyManager::UnlockUserAppKeys(uint32_t userId, bool needGetAllAppKey)
{
    return E_OK;
}

int32_t KeyManager::CreateRecoverKey(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int32_t KeyManager::SetRecoverKey(const std::vector<uint8_t> &key)
{
    return E_OK;
}

int32_t KeyManager::ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key)
{
    return E_OK;
}

int32_t KeyManager::SetDirectoryElPolicy(unsigned int user, KeyType type,
    const std::vector<FileList> &vec)
{
    for (auto temp : vec) {
        if (type == flag_num_sec && IsDir("/data/c")) {
            return -flag_num_thir;
        }
        if (IsDir("/data/a") && user == 0) {
            return -1;
        }
        if (IsDir("/data/b") && user == main_user) {
            return -flag_num_sec;
        }
    }
    return E_OK;
}

int32_t KeyManager::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath,
    StorageService::EncryptionLevel level)
{
    return E_OK;
}

int32_t KeyManager::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    return E_OK;
}

int KeyManager::GenerateUserKeyByType(unsigned int user, KeyType type,
                                      const std::vector<uint8_t> &token,
                                      const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int KeyManager::ActiveCeSceSeceUserKey(unsigned int user, KeyType type,
                                       const std::vector<uint8_t> &token,
                                       const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int KeyManager::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    return E_OK;
}

#ifdef EL5_FILEKEY_MANAGER
int KeyManager::RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    return E_OK;
}

int KeyManager::UnregisterUeceActivationCallback()
{
    return E_OK;
}
#endif

int KeyManager::NotifyUeceActivation(uint32_t userId, int32_t resultCode, bool needGetAllAppKey)
{
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
