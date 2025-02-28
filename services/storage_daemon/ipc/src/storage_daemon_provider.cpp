/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ipc/storage_daemon_provider.h"

#include <cinttypes>
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/storage_radar.h"
#include "utils/storage_xcollie.h"
#include "system_ability_definition.h"
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager.h"
#include "volume/volume_manager.h"
#endif
#include "user/mount_manager.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;
constexpr unsigned int LOCAL_TIME_OUT_SECONDS = 10;
constexpr unsigned int INACTIVE_USER_KEY_OUT_SECONDS = 25;
constexpr unsigned int UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS = 300;
constexpr unsigned int RADAR_REPORT_STATISTIC_INTERVAL_MINUTES = 1440;
constexpr unsigned int USER0ID = 0;
constexpr unsigned int USER100ID = 100;
constexpr unsigned int RADAR_STATISTIC_THREAD_WAIT_SECONDS = 60;

std::map<uint32_t, RadarStatisticInfo>::iterator StorageDaemonProvider::GetUserStatistics(const uint32_t userId)
{
    auto it = opStatistics_.find(userId);
    if (it != opStatistics_.end()) {
        return it;
    }
    RadarStatisticInfo radarInfo = {0};
    return opStatistics_.insert(make_pair(userId, radarInfo)).first;
}

void StorageDaemonProvider::GetTempStatistics(std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    std::lock_guard<std::mutex> lock(mutex_);
    statistics.insert(opStatistics_.begin(), opStatistics_.end());
    opStatistics_.clear();
}

void StorageDaemonProvider::StorageRadarThd(void)
{
    // report radar statistics when restart
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    if (execRadarReportCon_.wait_for(lock, std::chrono::seconds(RADAR_STATISTIC_THREAD_WAIT_SECONDS),
                                     [this] { return this->stopRadarReport_.load(); })) {
        LOGI("Storage statistic radar exit.");
        return;
    }
    lock.unlock();
    LOGI("Storage statistic thread start.");
    StorageStatisticRadar::GetInstance().CreateStatisticFile();
    std::map<uint32_t, RadarStatisticInfo> opStatisticsTemp;
    StorageStatisticRadar::GetInstance().ReadStatisticFile(opStatisticsTemp);
    if (!opStatisticsTemp.empty()) {
        for (auto ele : opStatisticsTemp) {
            StorageService::StorageRadar::ReportStatistics(ele.first, ele.second);
        }
        StorageStatisticRadar::GetInstance().CleanStatisticFile();
    }
    lastRadarReportTime_ = std::chrono::system_clock::now();
    while (!stopRadarReport_.load()) {
        std::unique_lock<std::mutex> lock(onRadarReportLock_);
        if (execRadarReportCon_.wait_for(lock, std::chrono::seconds(UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS),
                                         [this] { return this->stopRadarReport_.load(); })) {
            LOGI("Storage statistic radar exit.");
            return;
        }
        std::chrono::time_point<std::chrono::system_clock> nowTime = std::chrono::system_clock::now();
        int64_t intervalMinutes =
            std::chrono::duration_cast<std::chrono::minutes>(nowTime - lastRadarReportTime_).count();
        if ((intervalMinutes > RADAR_REPORT_STATISTIC_INTERVAL_MINUTES) && !opStatistics_.empty()) {
            LOGI("Storage statistic report, intervalMinutes:%{public}" PRId64, intervalMinutes);
            opStatisticsTemp.clear();
            GetTempStatistics(opStatisticsTemp);
            for (auto ele : opStatisticsTemp) {
                StorageService::StorageRadar::ReportStatistics(ele.first, ele.second);
            }
            lastRadarReportTime_ = std::chrono::system_clock::now();
            StorageStatisticRadar::GetInstance().CleanStatisticFile();
            continue;
        }
        if (!isNeedUpdateRadarFile_) {
            LOGI("Storage statistic not need update.");
            continue;
        }
        LOGI("Storage statistic update, intervalMinutes:%{public}" PRId64, intervalMinutes);
        isNeedUpdateRadarFile_ = false;
        StorageStatisticRadar::GetInstance().UpdateStatisticFile(opStatistics_);
    }
}

StorageDaemonProvider::StorageDaemonProvider() : storageDaemon_()
{
    if (storageDaemon_ == nullptr) {
        storageDaemon_ = std::make_shared<OHOS::StorageDaemon::StorageDaemon>();
    }
    callRadarStatisticReportThread_ = std::thread([this]() { StorageRadarThd(); });
}

StorageDaemonProvider::~StorageDaemonProvider()
{
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    stopRadarReport_ = true;
    execRadarReportCon_.notify_one();
    lock.unlock();
    if (callRadarStatisticReportThread_.joinable()) {
        callRadarStatisticReportThread_.join();
    }
    storageDaemon_ = nullptr;
}

int32_t StorageDaemonProvider::Shutdown()
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->Shutdown();
}

int32_t StorageDaemonProvider::Mount(const std::string &volId, uint32_t flags)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->Mount(volId, flags);
}

int32_t StorageDaemonProvider::UMount(const std::string &volId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->UMount(volId);
}

int32_t StorageDaemonProvider::Check(const std::string &volId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->Check(volId);
}

int32_t StorageDaemonProvider::Format(const std::string &volId, const std::string &fsType)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->Format(volId, fsType);
}

int32_t StorageDaemonProvider::Partition(const std::string &diskId, int32_t type)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->Partition(diskId, type);
}

int32_t StorageDaemonProvider::SetVolumeDescription(const std::string &volId, const std::string &description)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->SetVolumeDescription(volId, description);
}

int32_t StorageDaemonProvider::StartUser(int32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t err = storageDaemon_->StartUser(userId);
    if (err != E_OK) {
        it->second.userStartFailCount++;
    } else {
        it->second.userStartSuccCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::StopUser(int32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t err = storageDaemon_->StopUser(userId);
    if (err != E_OK) {
        it->second.userStopFailCount++;
    } else {
        it->second.userStopSuccCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t err = storageDaemon_->PrepareUserDirs(userId, flags);
    if (err != E_OK) {
        it->second.userAddFailCount++;
    } else {
        it->second.userAddSuccCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = storageDaemon_->DestroyUserDirs(userId, flags);
    if (err != E_OK) {
        it->second.userRemoveFailCount++;
    } else {
        it->second.userRemoveSuccCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::CompleteAddUser(int32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->CompleteAddUser(userId);
}

int32_t StorageDaemonProvider::InitGlobalKey()
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(USER0ID);
    isNeedUpdateRadarFile_ = true;
    int err = storageDaemon_->InitGlobalKey();
    if (err != E_OK) {
        it->second.keyLoadFailCount++;
    } else {
        it->second.keyLoadSuccCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::InitGlobalUserKeys()
{
    LOGI("StorageDaemonProvider_InitGlobalUserKeys start.");
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(USER100ID);
    isNeedUpdateRadarFile_ = true;
    int32_t err = storageDaemon_->InitGlobalUserKeys();
    if (err != E_OK) {
        it->second.keyLoadFailCount++;
    } else {
        it->second.keyLoadSuccCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::GenerateUserKeys(uint32_t userId, uint32_t flags)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:GenerateUserKeys", LOCAL_TIME_OUT_SECONDS);
    int err = storageDaemon_->GenerateUserKeys(userId, flags);
    StorageXCollie::CancelTimer(timerId);
    return err;
}

int32_t StorageDaemonProvider::DeleteUserKeys(uint32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:DeleteUserKeys", LOCAL_TIME_OUT_SECONDS);
    int err = storageDaemon_->DeleteUserKeys(userId);
    StorageXCollie::CancelTimer(timerId);
    return err;
}

int32_t StorageDaemonProvider::UpdateUserAuth(uint32_t userId,
                                              uint64_t secureUid,
                                              const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &oldSecret,
                                              const std::vector<uint8_t> &newSecret)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:UpdateUserAuth", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int err = storageDaemon_->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    StorageXCollie::CancelTimer(timerId);
    return err;
}

int32_t StorageDaemonProvider::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                            const std::vector<uint8_t> &newSecret,
                                                            uint64_t secureUid,
                                                            uint32_t userId,
                                                            const std::vector<std::vector<uint8_t>> &plainText)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
}

int32_t StorageDaemonProvider::ActiveUserKey(uint32_t userId,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:ActiveUserKey", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t err = storageDaemon_->ActiveUserKey(userId, token, secret);
    StorageXCollie::CancelTimer(timerId);
    if ((err == E_OK) || ((err == E_ACTIVE_EL2_FAILED) && token.empty() && secret.empty())) {
        it->second.keyLoadSuccCount++;
    } else {
        it->second.keyLoadFailCount++;
    }
    return err;
}

int32_t StorageDaemonProvider::InactiveUserKey(uint32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:InactiveUserKey", INACTIVE_USER_KEY_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t ret = storageDaemon_->InactiveUserKey(userId);
    StorageXCollie::CancelTimer(timerId);
    if (ret != E_OK) {
        it->second.keyUnloadFailCount++;
    } else {
        it->second.keyUnloadSuccCount++;
    }
    return ret;
}

int32_t StorageDaemonProvider::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:UpdateKeyContext", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = storageDaemon_->UpdateKeyContext(userId, needRemoveTmpKey);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::MountCryptoPathAgain(uint32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->MountCryptoPathAgain(userId);
}

int32_t StorageDaemonProvider::LockUserScreen(uint32_t userId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:LockUserScreen", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int ret = storageDaemon_->LockUserScreen(userId);
    StorageXCollie::CancelTimer(timerId);
    if (ret != E_OK) {
        it->second.keyUnloadFailCount++;
    } else {
        it->second.keyUnloadSuccCount++;
    }
    return ret;
}

int32_t StorageDaemonProvider::UnlockUserScreen(uint32_t userId,
                                                const std::vector<uint8_t> &token,
                                                const std::vector<uint8_t> &secret)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:UnlockUserScreen", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int ret = storageDaemon_->UnlockUserScreen(userId, token, secret);
    StorageXCollie::CancelTimer(timerId);
    if (ret != E_OK) {
        it->second.keyLoadFailCount++;
    } else {
        it->second.keyLoadSuccCount++;
    }
    return ret;
}

int32_t StorageDaemonProvider::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    lockScreenStatus = false;
    int timerId = StorageXCollie::SetTimer("storage:GetLockScreenStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = storageDaemon_->GetLockScreenStatus(userId, lockScreenStatus);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:GenerateAppkey", LOCAL_TIME_OUT_SECONDS);
    int32_t ret = storageDaemon_->GenerateAppkey(userId, hashId, keyId);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    int timerId = StorageXCollie::SetTimer("storage:DeleteAppkey", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = storageDaemon_->DeleteAppkey(userId, keyId);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::CreateRecoverKey(uint32_t userId,
                                                uint32_t userType,
                                                const std::vector<uint8_t> &token,
                                                const std::vector<uint8_t> &secret)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->CreateRecoverKey(userId, userType, token, secret);
}

int32_t StorageDaemonProvider::SetRecoverKey(const std::vector<uint8_t> &key)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->SetRecoverKey(key);
}

int32_t StorageDaemonProvider::CreateShareFile(const std::vector<std::string> &uriList,
                                               uint32_t tokenId,
                                               uint32_t flag,
                                               std::vector<int32_t> &funcResult)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->CreateShareFile(uriList, tokenId, flag, funcResult);
}

int32_t StorageDaemonProvider::DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &uriList)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->DeleteShareFile(tokenId, uriList);
}

int32_t StorageDaemonProvider::SetBundleQuota(const std::string &bundleName,
                                              int32_t uid,
                                              const std::string &bundleDataDirPath,
                                              int32_t limitSizeMb)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageDaemonProvider::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->GetOccupiedSpace(idType, id, size);
}

int32_t StorageDaemonProvider::UpdateMemoryPara(int32_t size, int32_t &oldSize)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->UpdateMemoryPara(size, oldSize);
}

int32_t StorageDaemonProvider::GetBundleStatsForIncrease(uint32_t userId,
                                                         const std::vector<std::string> &bundleNames,
                                                         const std::vector<int64_t> &incrementalBackTimes,
                                                         std::vector<int64_t> &pkgFileSizes,
                                                         std::vector<int64_t> &incPkgFileSizes)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes,
                                                     incPkgFileSizes);
}

int32_t StorageDaemonProvider::MountDfsDocs(int32_t userId,
                                            const std::string &relativePath,
                                            const std::string &networkId,
                                            const std::string &deviceId)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonProvider::UMountDfsDocs(int32_t userId,
                                             const std::string &relativePath,
                                             const std::string &networkId,
                                             const std::string &deviceId)
{
    LOGI("StorageDaemonProvider::UMountDfsDocs start.");
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonProvider::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    isEncrypted = true;
    int timerId = StorageXCollie::SetTimer("storage:GetFileEncryptStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = storageDaemon_->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    needActive = false;
    int timerId = StorageXCollie::SetTimer("storage:GetUserNeedActiveStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = storageDaemon_->GetUserNeedActiveStatus(userId, needActive);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::MountMediaFuse(int32_t userId, int32_t &devFd)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageDaemonProvider::MountMediaFuse start.");
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    devFd = -1;
    int32_t ret = storageDaemon_->MountMediaFuse(userId, devFd);
    return ret;
#endif
    return E_OK;
}

int32_t StorageDaemonProvider::UMountMediaFuse(int32_t userId)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageDaemonProvider::UMountMediaFuse start.");
    if (storageDaemon_ == nullptr) {
        return E_ERR;
    }
    return storageDaemon_->UMountMediaFuse(userId);
#endif
    return E_OK;
}

void StorageDaemonProvider::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
                                                                                  const std::string &deviceId)
{
    LOGI("SystemAbilityId:%{public}d", systemAbilityId);
#ifdef EXTERNAL_STORAGE_MANAGER
    if (systemAbilityId == ACCESS_TOKEN_MANAGER_SERVICE_ID) {
        DiskManager::Instance()->ReplayUevent();
    }
#endif
    if (systemAbilityId == FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID) {
        MountManager::GetInstance()->SetCloudState(true);
    }
}

void StorageDaemonProvider::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(int32_t systemAbilityId,
                                                                                     const std::string &deviceId)
{
    LOGI("SystemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId == FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID) {
        MountManager::GetInstance()->SetCloudState(false);
    }
}
} // namespace StorageDaemon
} // namespace OHOS
