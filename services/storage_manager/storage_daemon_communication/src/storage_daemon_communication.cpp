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

#include "storage_daemon_communication/storage_daemon_communication.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "os_account_manager.h"
#ifdef ENABLE_SCREENLOCK_MANAGER
#include "screenlock_manager.h"
#endif
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
StorageDaemonCommunication::StorageDaemonCommunication()
{
    LOGI("DEBUG StorageDaemonCommunication constructer");
    storageDaemon_ = nullptr;
}

StorageDaemonCommunication::~StorageDaemonCommunication()
{
    LOGI("DEBUG ~StorageDaemonCommunication destructer ~");
}

int32_t StorageDaemonCommunication::Connect()
{
    LOGD("StorageDaemonCommunication::Connect start");
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageDaemon_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("StorageDaemonCommunication::Connect samgr nullptr");
            return E_SA_IS_NULLPTR;
        }
        auto object = sam->GetSystemAbility(STORAGE_MANAGER_DAEMON_ID);
        if (object == nullptr) {
            LOGE("StorageDaemonCommunication::Connect object nullptr");
            return E_REMOTE_IS_NULLPTR;
        }
        storageDaemon_ = iface_cast<OHOS::StorageDaemon::IStorageDaemon>(object);
        if (storageDaemon_ == nullptr) {
            LOGE("StorageDaemonCommunication::Connect service nullptr");
            return E_SERVICE_IS_NULLPTR;
        }
        deathRecipient_ = new (std::nothrow) SdDeathRecipient();
        if (!deathRecipient_) {
            LOGE("StorageDaemonCommunication::Connect failed to create death recipient");
            return E_DEATHRECIPIENT_IS_NULLPTR;
        }

        storageDaemon_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    LOGD("StorageDaemonCommunication::Connect end");
    return E_OK;
}

int32_t StorageDaemonCommunication::PrepareAddUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageDaemonCommunication::PrepareAddUser start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::PrepareAddUser connect failed");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("PrepareAddUser::Connect", userId, err, extraData);
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("StorageDaemonCommunication::PrepareAddUser",
            userId, E_SERVICE_IS_NULLPTR, extraData);
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->PrepareUserDirs(userId, flags);
}

int32_t StorageDaemonCommunication::RemoveUser(int32_t userId, uint32_t flags)
{
    LOGI("StorageDaemonCommunication::RemoveUser start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::RemoveUser connect failed");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("RemoveUser::Connect", userId, err, extraData);
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        std::string extraData = "flags=" + std::to_string(flags);
        StorageRadar::ReportUserManager("StorageDaemonCommunication::RemoveUser",
            userId, E_SERVICE_IS_NULLPTR, extraData);
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->DestroyUserDirs(userId, flags);
}

int32_t StorageDaemonCommunication::PrepareStartUser(int32_t userId)
{
    LOGD("StorageDaemonCommunication::PrepareStartUser start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::PrepareStartUser connect failed");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::PrepareStartUser::Connect",
            userId, err, "");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::PrepareStartUser",
            userId, E_SERVICE_IS_NULLPTR, "");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->StartUser(userId);
}

int32_t StorageDaemonCommunication::StopUser(int32_t userId)
{
    LOGI("StorageDaemonCommunication::StopUser start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::StopUser connect failed");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::StopUser::Connect", userId, err, "");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::StopUser", userId, E_SERVICE_IS_NULLPTR, "");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->StopUser(userId);
}

int32_t StorageDaemonCommunication::CompleteAddUser(int32_t userId)
{
    LOGI("StorageDaemonCommunication::CompleteAddUser start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::CompleteAddUser connect failed");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::CompleteAddUser::Connect", userId, err, "");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::CompleteAddUser service nullptr");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::CompleteAddUser",
            userId, E_SERVICE_IS_NULLPTR, "");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->CompleteAddUser(userId);
}

int32_t StorageDaemonCommunication::Mount(std::string volumeId, int32_t flag)
{
    LOGI("StorageDaemonCommunication::mount start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::mount connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->Mount(volumeId, flag);
}

int32_t StorageDaemonCommunication::Unmount(std::string volumeId)
{
    LOGI("StorageDaemonCommunication::unmount start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::unmount connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UMount(volumeId);
}

int32_t StorageDaemonCommunication::TryToFix(std::string volumeId, int32_t flag)
{
    LOGI("StorageDaemonCommunication::TryToFix start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::TryToFix connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->TryToFix(volumeId, flag);
}

int32_t StorageDaemonCommunication::Check(std::string volumeId)
{
    LOGI("StorageDaemonCommunication::check start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::check connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->Check(volumeId);
}

int32_t StorageDaemonCommunication::Partition(std::string diskId, int32_t type)
{
    LOGI("StorageDaemonCommunication::Partition start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::Partition connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->Partition(diskId, type);
}

int32_t StorageDaemonCommunication::Format(std::string volumeId, std::string type)
{
    LOGI("StorageDaemonCommunication::Format start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::Format connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->Format(volumeId, type);
}

int32_t StorageDaemonCommunication::SetVolumeDescription(std::string volumeId, std::string description)
{
    LOGI("StorageDaemonCommunication::SetVolumeDescription start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::SetVolumeDescription connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->SetVolumeDescription(volumeId, description);
}

int32_t StorageDaemonCommunication::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
    LOGI("StorageDaemonCommunication::QueryUsbIsInUse start");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::QueryUsbIsInUse connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->QueryUsbIsInUse(diskPath, isInUse);
}

int32_t StorageDaemonCommunication::DeleteUserKeys(uint32_t userId)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->DeleteUserKeys(userId);
}

int32_t StorageDaemonCommunication::EraseAllUserEncryptedKeys()
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->EraseAllUserEncryptedKeys();
}

int32_t StorageDaemonCommunication::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                                   const std::vector<uint8_t> &token,
                                                   const std::vector<uint8_t> &oldSecret,
                                                   const std::vector<uint8_t> &newSecret)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
}

int32_t StorageDaemonCommunication::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                                 const std::vector<uint8_t> &newSecret,
                                                                 uint64_t secureUid,
                                                                 uint32_t userId,
                                                                 const std::vector<std::vector<uint8_t>> &plainText)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
}

int32_t StorageDaemonCommunication::ActiveUserKey(uint32_t userId,
                                                  const std::vector<uint8_t> &token,
                                                  const std::vector<uint8_t> &secret)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->ActiveUserKey(userId, token, secret);
}

int32_t StorageDaemonCommunication::InactiveUserKey(uint32_t userId)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->InactiveUserKey(userId);
}

int32_t StorageDaemonCommunication::LockUserScreen(uint32_t userId)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->LockUserScreen(userId);
}

int32_t StorageDaemonCommunication::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
}

int32_t StorageDaemonCommunication::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GetUserNeedActiveStatus(userId, needActive);
}

int32_t StorageDaemonCommunication::UnlockUserScreen(uint32_t userId,
                                                     const std::vector<uint8_t> &token,
                                                     const std::vector<uint8_t> &secret)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UnlockUserScreen(userId, token, secret);
}

int32_t StorageDaemonCommunication::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GetLockScreenStatus(userId, lockScreenStatus);
}

int32_t StorageDaemonCommunication::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UpdateKeyContext(userId, needRemoveTmpKey);
}

int32_t StorageDaemonCommunication::ResetSdProxy()
{
    LOGI("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((storageDaemon_ != nullptr) && (storageDaemon_->AsObject() != nullptr)) {
        storageDaemon_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    storageDaemon_ = nullptr;

    return E_OK;
}

void StorageDaemonCommunication::ForceLockUserScreen()
{
    LOGI("StorageDaemonCommunication::ForceLockUserScreen, storage_daemon process maybe has died.");
#ifdef ENABLE_SCREENLOCK_MANAGER
    std::vector<int32_t> ids;
    int32_t ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        LOGE("Query active userid failed, ret = %{public}u", ret);
        StorageRadar::ReportOsAccountResult("ForceLockUserScreen::QueryActiveOsAccountIds", ret, DEFAULT_USERID);
        return;
    }
    int reasonFlag = static_cast<int>(ScreenLock::StrongAuthReasonFlags::ACTIVE_REQUEST);
    ret = ScreenLock::ScreenLockManager::GetInstance()->RequestStrongAuth(reasonFlag, ids[0]);
    if (ret != ScreenLock::E_SCREENLOCK_OK) {
        LOGE("Request strong auth by screen lock manager failed.");
        StorageRadar::ReportOsAccountResult("ForceLockUserScreen::RequestStrongAuth", ret, ids[0]);
        return;
    }
    ret = ScreenLock::ScreenLockManager::GetInstance()->Lock(ids[0]);
    if (ret != ScreenLock::E_SCREENLOCK_OK) {
        LOGE("Lock user screen by screen lock manager failed.");
        StorageRadar::ReportOsAccountResult("ForceLockUserScreen::Lock", ret, ids[0]);
        return;
    }
    LOGI("Force lock user screen and request strong auth success for userId = %{public}d.", ids[0]);
#endif
}

void SdDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LOGE("StorageDaemonCommunication::OnRemoteDied, storage_daemon process has died.");
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->ResetSdProxy();
    DelayedSingleton<StorageDaemonCommunication>::GetInstance()->ForceLockUserScreen();
}

std::vector<int32_t> StorageDaemonCommunication::CreateShareFile(const StorageFileRawData &rawData,
                                                                 uint32_t tokenId, uint32_t flag)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return std::vector<int32_t>{err};
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return std::vector<int32_t>{err};
    }
    std::vector<int32_t> funcResult;

    storageDaemon_->CreateShareFile(rawData, tokenId, flag, funcResult);
    LOGI("StorageDaemonCommunication::CreateShareFile end. result is %{public}zu", funcResult.size());
    return funcResult;
}

int32_t StorageDaemonCommunication::DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->DeleteShareFile(tokenId, rawData);
}

int32_t StorageDaemonCommunication::SetBundleQuota(const std::string &bundleName, int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageDaemonCommunication::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->ListUserdataDirInfo(scanDirs);
}

int32_t StorageDaemonCommunication::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    LOGD("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GetOccupiedSpace(idType, id, size);
}

int32_t StorageDaemonCommunication::MountCryptoPathAgain(int32_t userId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::MountCryptoPathAgain::Connect", userId, err, "");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        StorageRadar::ReportUserManager("StorageDaemonCommunication::MountCryptoPathAgain",
            userId, E_SERVICE_IS_NULLPTR, "");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->MountCryptoPathAgain(userId);
}

int32_t StorageDaemonCommunication::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GenerateAppkey(userId, hashId, keyId, needReSet);
}

int32_t StorageDaemonCommunication::DeleteAppkey(uint32_t userId, const std::string keyId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->DeleteAppkey(userId, keyId);
}

int32_t StorageDaemonCommunication::CreateRecoverKey(uint32_t userId,
                                                     uint32_t userType,
                                                     const std::vector<uint8_t> &token,
                                                     const std::vector<uint8_t> &secret)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->CreateRecoverKey(userId, userType, token, secret);
}

int32_t StorageDaemonCommunication::SetRecoverKey(const std::vector<uint8_t> &key)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->SetRecoverKey(key);
}

int32_t StorageDaemonCommunication::ResetSecretWithRecoveryKey(uint32_t userId,
    uint32_t rkType, const std::vector<uint8_t> &key)
{
    LOGI("enter");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->ResetSecretWithRecoveryKey(userId, rkType, key);
}

int32_t StorageDaemonCommunication::UpdateMemoryPara(int32_t size, int32_t &oldSize)
{
    LOGI("StorageDaemonCommunication::UpdateMemoryPara");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UpdateMemoryPara(size, oldSize);
}

int32_t StorageDaemonCommunication::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageDaemonCommunication::MountDfsDocs start.");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonCommunication::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("StorageDaemonCommunication::UMountDfsDocs start.");
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonCommunication::MountMediaFuse(int32_t userId, int32_t &devFd)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->MountMediaFuse(userId, devFd);
#else
    return E_OK;
#endif
}

int32_t StorageDaemonCommunication::UMountMediaFuse(int32_t userId)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UMountMediaFuse(userId);
#else
    return E_OK;
#endif
}

int32_t StorageDaemonCommunication::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->MountFileMgrFuse(userId, path, fuseFd);
}

int32_t StorageDaemonCommunication::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UMountFileMgrFuse(userId, path);
}

int32_t StorageDaemonCommunication::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->IsFileOccupied(path, inputList, outputList, isOccupy);
}

int32_t StorageDaemonCommunication::MountDisShareFile(int32_t userId,
    const std::map<std::string, std::string> &shareFiles)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->MountDisShareFile(userId, shareFiles);
}

int32_t StorageDaemonCommunication::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UMountDisShareFile(userId, networkId);
}

int32_t StorageDaemonCommunication::InactiveUserPublicDirKey(uint32_t userId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::InactiveUserPublicDirKey connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->InactiveUserPublicDirKey(userId);
}

int32_t StorageDaemonCommunication::UpdateUserPublicDirPolicy(uint32_t userId)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::UpdateUserPublicDirPolicy connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UpdateUserPublicDirPolicy(userId);
}

int32_t StorageDaemonCommunication::MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::MountUsbFuse connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->MountUsbFuse(volumeId, fsUuid, fuseFd);
}

int32_t StorageDaemonCommunication::QueryOccupiedSpaceForSa(std::string &storageStatus,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->QueryOccupiedSpaceForSa(storageStatus, bundleNameAndUid);
}

int32_t StorageDaemonCommunication::RegisterUeceActivationCallback(
    const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->RegisterUeceActivationCallback(ueceCallback);
}

int32_t StorageDaemonCommunication::UnregisterUeceActivationCallback()
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->UnregisterUeceActivationCallback();
}

int32_t StorageDaemonCommunication::CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->CreateUserDir(path, mode, uid, gid);
}

int32_t StorageDaemonCommunication::DeleteUserDir(const std::string &path)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->DeleteUserDir(path);
}

int32_t StorageDaemonCommunication::SetDirEncryptionPolicy(uint32_t userId,
    const std::string &dirPath, uint32_t level)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("StorageDaemonCommunication::SetDirEncryptionPolicy Connect failed, userId:%{public}u", userId);
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::SetDirEncryptionPolicy Connect service nullptr, userId:%{public}u", userId);
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->SetDirEncryptionPolicy(userId, dirPath, level);
}

int32_t StorageDaemonCommunication::StatisticSysDirSpace()
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::Connect service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->StatisticSysDirSpace();
}

int32_t StorageDaemonCommunication::GetDataSizeByPath(const std::string &path, int64_t &size)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::GetDataSizeByPath service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GetDataSizeByPath(path, size);
}

int32_t StorageDaemonCommunication::GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("Connect failed");
        return err;
    }
    if (storageDaemon_ == nullptr) {
        LOGE("StorageDaemonCommunication::GetRmgResourceSize service nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageDaemon_->GetRmgResourceSize(rgmName, totalSize);
}
} // namespace StorageManager
} // namespace OHOS
