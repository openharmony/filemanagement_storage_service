/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "client/storage_manager_client.h"

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iservice_registry.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "system_ability_definition.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace StorageManager {
sptr<IStorageManager> StorageManagerClient::GetStorageManagerProxy(void)
{
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("samgr empty error");
        return nullptr;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::STORAGE_MANAGER_MANAGER_ID);
    if (object == nullptr) {
        LOGE("storage manager client samgr ablity empty error");
        return nullptr;
    }

    return iface_cast<IStorageManager>(object);
}

int32_t StorageManagerClient::PrepareAddUser(uint32_t userId, uint32_t flags)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->PrepareAddUser(userId, flags);
}

int32_t StorageManagerClient::RemoveUser(uint32_t userId, uint32_t flags)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->RemoveUser(userId, flags);
}

int32_t StorageManagerClient::EraseAllUserEncryptedKeys()
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->EraseAllUserEncryptedKeys();
}

int32_t StorageManagerClient::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &oldSecret,
                                             const std::vector<uint8_t> &newSecret)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
}

int32_t StorageManagerClient::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                           const std::vector<uint8_t> &newSecret,
                                                           uint64_t secureUid,
                                                           uint32_t userId,
                                                           const std::vector<std::vector<uint8_t>> &plainText)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
}

int32_t StorageManagerClient::ActiveUserKey(uint32_t userId,
                                            const std::vector<uint8_t> &token,
                                            const std::vector<uint8_t> &secret)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->ActiveUserKey(userId, token, secret);
}

int32_t StorageManagerClient::InactiveUserKey(uint32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->InactiveUserKey(userId);
}

int32_t StorageManagerClient::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UpdateKeyContext(userId, needRemoveTmpKey);
}

int32_t StorageManagerClient::LockUserScreen(uint32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->LockUserScreen(userId);
}

int32_t StorageManagerClient::UnlockUserScreen(uint32_t userId,
                                               const std::vector<uint8_t> &token,
                                               const std::vector<uint8_t> &secret)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UnlockUserScreen(userId, token, secret);
}

int32_t StorageManagerClient::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
}

int32_t StorageManagerClient::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->GetUserNeedActiveStatus(userId, needActive);
}

int32_t StorageManagerClient::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->GetLockScreenStatus(userId, lockScreenStatus);
}

int32_t StorageManagerClient::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageManagerClient::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }

    return client->UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageManagerClient::RegisterUeceActivationCallback(
    const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }
    
    return client->RegisterUeceActivationCallback(ueceCallback);
}

int32_t StorageManagerClient::UnregisterUeceActivationCallback()
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    sptr<IStorageManager> client = GetStorageManagerProxy();
    if (client == nullptr) {
        LOGE("get storage manager service failed");
        return E_SA_IS_NULLPTR;
    }
    return client->UnregisterUeceActivationCallback();
}
}
}
