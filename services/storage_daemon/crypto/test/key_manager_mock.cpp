/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
int32_t KeyManager::UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret)
{
    return E_OK;
}

int32_t KeyManager::ActiveUserKey(unsigned int user, const std::vector<uint8_t> &token,
    const std::vector<uint8_t> &secret)
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

int32_t KeyManager::UnlockUserScreen(uint32_t user)
{
    return E_OK;
}

int32_t KeyManager::GetLockScreenStatus(uint32_t user, bool &lockScreenStatus)
{
    return E_OK;
}

int32_t KeyManager::GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId)
{
    return E_OK;
}

int32_t KeyManager::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    return E_OK;
}

int32_t KeyManager::SetDirectoryElPolicy(unsigned int user, KeyType type,
    const std::vector<FileList> &vec)
{
    return E_OK;
}

int32_t KeyManager::UpdateKeyContext(uint32_t userId)
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
} // namespace StorageDaemon
} // namespace OHOS
