/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "utils/storage_radar.h"
#include "utils/storage_xcollie.h"
#include <cinttypes>
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_manager.h"
#include "volume/volume_manager.h"
#endif
#include "file_ex.h"
#include "user/mount_manager.h"
#include "utils/string_utils.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <thread>
#ifdef USER_CRYPTO_MANAGER
#include "crypto/app_clone_key_manager.h"
#include "crypto/iam_client.h"
#include "crypto/key_crypto_utils.h"
#include "crypto/key_manager.h"
#endif
#include "file_sharing/file_sharing.h"
#include "quota/quota_manager.h"
#include "user/user_manager.h"
#include "user/system_mount_manager.h"
namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace StorageService;
constexpr unsigned int LOCAL_TIME_OUT_SECONDS = 10;
constexpr unsigned int INACTIVE_USER_KEY_OUT_SECONDS = 25;
constexpr unsigned int UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS = 300;
constexpr unsigned int RADAR_REPORT_STATISTIC_INTERVAL_MINUTES = 1440;
constexpr unsigned int USER0ID = 0;
constexpr unsigned int USER100ID = 100;
constexpr unsigned int RADAR_STATISTIC_THREAD_WAIT_SECONDS = 60;
constexpr unsigned int MAX_URI_COUNT = 200000;
constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024; // 128M
constexpr const char *PATH_INVALID_FLAG1 = "../";
constexpr const char *PATH_INVALID_FLAG2 = "/..";
constexpr int32_t PATH_INVALID_FLAG_LEN = 3;

std::map<uint32_t, RadarStatisticInfo>::iterator StorageDaemonProvider::GetUserStatistics(const uint32_t userId)
{
    auto it = opStatistics_.find(userId);
    if (it != opStatistics_.end()) {
        return it;
    }
    RadarStatisticInfo radarInfo = {0};
    return opStatistics_.insert(make_pair(userId, radarInfo)).first;
}

int32_t StorageDaemonProvider::CheckUserIdRange(int32_t userId)
{
    if (userId < StorageService::START_USER_ID || userId > StorageService::MAX_USER_ID) {
        LOGE("StorageDaemonProvider: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

bool StorageDaemonProvider::IsFilePathInvalid(const std::string &filePath)
{
    size_t pos = filePath.find(PATH_INVALID_FLAG1);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("Relative path is not allowed, path contain ../");
            return true;
        }
        pos = filePath.find(PATH_INVALID_FLAG1, pos + PATH_INVALID_FLAG_LEN);
    }
    pos = filePath.rfind(PATH_INVALID_FLAG2);
    if ((pos != std::string::npos) && (filePath.size() - pos == PATH_INVALID_FLAG_LEN)) {
        LOGE("Relative path is not allowed, path tail is /..");
        return true;
    }
    return false;
}

void StorageDaemonProvider::SetUserStatistics(uint32_t userId, RadarStatisticInfoType type)
{
    std::lock_guard<std::mutex> lock(mutexStats_);
    auto it = GetUserStatistics(userId);
    if (it == opStatistics_.end()) {
        LOGE("GetUserStatistics is nullptr, type: %{public}d", static_cast<int32_t>(type));
        return;
    }
    switch (type) {
        case RadarStatisticInfoType::KEY_LOAD_SUCCESS:
            it->second.keyLoadSuccCount++;
            break;
        case RadarStatisticInfoType::KEY_LOAD_FAIL:
            it->second.keyLoadFailCount++;
            break;
        case RadarStatisticInfoType::KEY_UNLOAD_SUCCESS:
            it->second.keyUnloadSuccCount++;
            break;
        case RadarStatisticInfoType::KEY_UNLOAD_FAIL:
            it->second.keyUnloadFailCount++;
            break;
        case RadarStatisticInfoType::USER_ADD_SUCCESS:
            it->second.userAddSuccCount++;
            break;
        case RadarStatisticInfoType::USER_ADD_FAIL:
            it->second.userAddFailCount++;
            break;
        case RadarStatisticInfoType::USER_REMOVE_SUCCESS:
            it->second.userRemoveSuccCount++;
            break;
        case RadarStatisticInfoType::USER_REMOVE_FAIL:
            it->second.userRemoveFailCount++;
            break;
        case RadarStatisticInfoType::USER_START_SUCCESS:
            it->second.userStartSuccCount++;
            break;
        case RadarStatisticInfoType::USER_START_FAIL:
            it->second.userStartFailCount++;
            break;
        case RadarStatisticInfoType::USER_STOP_SUCCESS:
            it->second.userStopSuccCount++;
            break;
        case RadarStatisticInfoType::USER_STOP_FAIL:
            it->second.userStopFailCount++;
            break;
        default:
            break;
    }
}

void StorageDaemonProvider::GetTempStatistics(std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    std::lock_guard<std::mutex> lock(mutexStats_);
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
            LOGD("Storage statistic not need update.");
            continue;
        }
        LOGI("Storage statistic update, intervalMinutes:%{public}" PRId64, intervalMinutes);
        isNeedUpdateRadarFile_ = false;
        StorageStatisticRadar::GetInstance().UpdateStatisticFile(opStatistics_);
    }
}

StorageDaemonProvider::StorageDaemonProvider()
{
    callRadarStatisticReportThread_ = std::thread([this]() { StorageRadarThd(); });
    std::thread thread([this]() {
        MountManager::GetInstance().InitSecondMountBundleName();
    });
    thread.detach();
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
}

int32_t StorageDaemonProvider::Shutdown()
{
    return E_OK;
}

int32_t StorageDaemonProvider::Mount(const std::string &volId, uint32_t flags)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Mount");
    int32_t ret = VolumeManager::Instance().Mount(volId, flags);
    if (ret != E_OK) {
        LOGW("Mount failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::Mount", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::UMount(const std::string &volId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle UMount");
    int32_t ret = VolumeManager::Instance().UMount(volId);
    if (ret != E_OK) {
        LOGW("UMount failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::UMount", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::TryToFix(const std::string &volId, uint32_t flags)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle TryToFix");
    int32_t ret = VolumeManager::Instance().TryToFix(volId, flags);
    if (ret != E_OK) {
        LOGW("TryToFix failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::TryToFix", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    int32_t ret = VolumeManager::Instance().MountUsbFuse(volumeId, fsUuid, fuseFd);
    if (ret != E_OK) {
        LOGW("MountUsbFuse failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::MountUsbFuse", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::Check(const std::string &volId)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Check");
    int32_t ret = VolumeManager::Instance().Check(volId);
    if (ret != E_OK) {
        LOGW("Check failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::Check", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::Format(const std::string &volId, const std::string &fsType)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Format");
    int32_t ret = VolumeManager::Instance().Format(volId, fsType);
    if (ret != E_OK) {
        LOGW("Format failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::Format", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::Partition(const std::string &diskId, int32_t type)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle Partition");
    int32_t ret = DiskManager::Instance().HandlePartition(diskId);
    if (ret != E_OK) {
        LOGW("HandlePartition failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::Partition", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::SetVolumeDescription(const std::string &volId, const std::string &description)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    LOGI("Handle SetVolumeDescription");
    int32_t ret = VolumeManager::Instance().SetVolumeDescription(volId, description);
    if (ret != E_OK) {
        LOGW("SetVolumeDescription failed, please check");
        StorageService::StorageRadar::ReportVolumeOperation("VolumeManager::SetVolumeDescription", ret);
    }
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    if (IsFilePathInvalid(diskPath)) {
        return E_PARAMS_INVALID;
    }
    LOGI("StorageDaemon::QueryUsbIsInUse diskPath: %{public}s", diskPath.c_str());
    isInUse = true;
    return VolumeManager::Instance().QueryUsbIsInUse(diskPath, isInUse);
#else
    return E_NOT_SUPPORT;
#endif
}

int32_t StorageDaemonProvider::StartUser(int32_t userId)
{
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    isNeedUpdateRadarFile_ = true;
    (void)StorageDaemon::GetInstance().SetPriority();  // set tid priority to 40
    int32_t ret = UserManager::GetInstance().StartUser(userId);
    if (ret != E_OK && ret != E_KEY_NOT_ACTIVED) {
        LOGE("StartUser failed, please check");
        StorageService::StorageRadar::ReportUserManager("StartUser", userId, ret, "");
    }
    SetUserStatistics(userId, ret != E_OK ? USER_START_FAIL : USER_START_SUCCESS);

    auto delay = StorageService::StorageRadar::ReportDuration("START USER",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: START USER: delay time = %{public}s", delay.c_str());
    return ret;
}

int32_t StorageDaemonProvider::StopUser(int32_t userId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::PrepareUserDirs userId %{public}d out of range", userId);
        return err;
    }
    isNeedUpdateRadarFile_ = true;
    int32_t ret = UserManager::GetInstance().StopUser(userId);
    LOGE("StopUser end, ret is %{public}d.", ret);
    StorageService::StorageRadar::ReportUserManager("StopUser", userId, ret, "");
    SetUserStatistics(userId, ret != E_OK ? USER_STOP_FAIL : USER_STOP_SUCCESS);
    return ret;
}

int32_t StorageDaemonProvider::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    int32_t error = CheckUserIdRange(userId);
    if (error != E_OK) {
        LOGE("StorageDaemon::PrepareUserDirs userId %{public}d out of range", userId);
        return error;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int32_t err = StorageDaemon::GetInstance().PrepareUserDirs(userId, flags);
    SetUserStatistics(userId, err != E_OK ? USER_ADD_FAIL : USER_ADD_SUCCESS);
    return err;
}

int32_t StorageDaemonProvider::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    int32_t error = CheckUserIdRange(userId);
    if (error != E_OK) {
        LOGE("StorageDaemon::DestroyUserDirs userId %{public}d out of range", userId);
        return error;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int err = StorageDaemon::GetInstance().DestroyUserDirs(userId, flags);
    SetUserStatistics(userId, err != E_OK ? USER_REMOVE_FAIL : USER_REMOVE_SUCCESS);
    return err;
}

int32_t StorageDaemonProvider::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level)
{
    LOGI("SetDirEncryptionPolicy begin.");
    if (IsFilePathInvalid(dirPath)) {
        return E_PARAMS_INVALID;
    }
    int timerId = StorageXCollie::SetTimer("storage:SetDirEncryptionPolicy", LOCAL_TIME_OUT_SECONDS);
    int32_t ret = StorageDaemon::GetInstance().SetDirEncryptionPolicy(userId, dirPath, level);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::CompleteAddUser(int32_t userId)
{
    int32_t err = StorageDaemon::GetInstance().CompleteAddUser(userId);
    return err;
}

int32_t StorageDaemonProvider::InitGlobalKey()
{
    StorageRadar::ReportFucBehavior("InitGlobalKey", DEFAULT_USERID, "InitGlobalKey Begin", E_OK);
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int err = StorageDaemon::GetInstance().InitGlobalKey();
    SetUserStatistics(USER0ID, err != E_OK ? KEY_LOAD_FAIL : KEY_LOAD_SUCCESS);
    StorageRadar::ReportFucBehavior("InitGlobalKey", DEFAULT_USERID, "InitGlobalKey End", err);
    return err;
}

int32_t StorageDaemonProvider::InitGlobalUserKeys()
{
    LOGI("StorageDaemonProvider_InitGlobalUserKeys start.");
    StorageRadar::ReportFucBehavior("InitGlobalUserKeys", DEFAULT_USERID, "InitGlobalUserKeys Begin", E_OK);
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int32_t err = StorageDaemon::GetInstance().InitGlobalUserKeys();
    SetUserStatistics(USER100ID, err != E_OK ? KEY_LOAD_FAIL : KEY_LOAD_SUCCESS);
    StorageRadar::ReportFucBehavior("InitGlobalUserKeys", DEFAULT_USERID, "InitGlobalUserKeys End", err);
    return err;
}

int32_t StorageDaemonProvider::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    return StorageDaemon::GetInstance().EraseAllUserEncryptedKeys(localIdList);
}

int32_t StorageDaemonProvider::UpdateUserAuth(uint32_t userId,
                                              uint64_t secureUid,
                                              const std::vector<uint8_t> &token,
                                              const std::vector<uint8_t> &oldSecret,
                                              const std::vector<uint8_t> &newSecret)
{
    int timerId = StorageXCollie::SetTimer("storage:UpdateUserAuth", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int err =  StorageDaemon::GetInstance().UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    StorageXCollie::CancelTimer(timerId);
    return err;
}

int32_t StorageDaemonProvider::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                            const std::vector<uint8_t> &newSecret,
                                                            uint64_t secureUid,
                                                            uint32_t userId,
                                                            const std::vector<std::vector<uint8_t>> &plainText)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UpdateUseAuthWithRecoveryKey userId %{public}d out of range", userId);
        return err;
    }
    return StorageDaemon::GetInstance().UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId,
                                                                     plainText);
}

int32_t StorageDaemonProvider::ActiveUserKey(uint32_t userId,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret)
{
    int32_t error = CheckUserIdRange(userId);
    if (error != E_OK) {
        LOGE("StorageDaemon::ActiveUserKey userId %{public}d out of range", userId);
        return error;
    }
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int timerId = StorageXCollie::SetTimer("storage:ActiveUserKey", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int32_t err = StorageDaemon::GetInstance().ActiveUserKey(userId, token, secret);
    StorageXCollie::CancelTimer(timerId);
    if ((err == E_OK) || ((err == E_ACTIVE_EL2_FAILED) && token.empty() && secret.empty())) {
        SetUserStatistics(userId, KEY_LOAD_SUCCESS);
    } else {
        SetUserStatistics(userId, KEY_LOAD_FAIL);
    }
    auto delay = StorageService::StorageRadar::ReportDuration("ACTIVE USER KEY",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: ACTIVE USER KEY: delay time = %{public}s", delay.c_str());
    return err;
}

int32_t StorageDaemonProvider::InactiveUserKey(uint32_t userId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::InactiveUserKey userId %{public}d out of range", userId);
        return err;
    }
    int timerId = StorageXCollie::SetTimer("storage:InactiveUserKey", INACTIVE_USER_KEY_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int32_t ret = StorageDaemon::GetInstance().InactiveUserKey(userId);
    StorageXCollie::CancelTimer(timerId);
    SetUserStatistics(userId, ret != E_OK ? KEY_UNLOAD_FAIL : KEY_UNLOAD_SUCCESS);
    return ret;
}

int32_t StorageDaemonProvider::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UpdateKeyContext userId %{public}d out of range", userId);
        return E_USERID_RANGE;
    }
    int timerId = StorageXCollie::SetTimer("storage:UpdateKeyContext", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = StorageDaemon::GetInstance().UpdateKeyContext(userId, needRemoveTmpKey);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::MountCryptoPathAgain(uint32_t userId)
{
    LOGI("begin to MountCryptoPathAgain");
    StorageRadar::ReportFucBehavior("MountCryptoPathAgain", userId, "MountCryptoPathAgain Begin", E_OK);
#ifdef USER_CRYPTO_MANAGER
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    auto ret = MountManager::GetInstance().MountCryptoPathAgain(userId);
    if (ret != E_OK) {
        StorageService::StorageRadar::ReportUserManager("MountCryptoPathAgain::MountManager::MountCryptoPathAgain",
                                                        userId, ret, "");
    }
    auto delay = StorageService::StorageRadar::ReportDuration("MountCryptoPathAgain",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: MountCryptoPathAgain: delay time = %{public}s", delay.c_str());
    StorageRadar::ReportFucBehavior("MountCryptoPathAgain", userId, "MountCryptoPathAgain End", ret);
    return ret;
#else
    return E_OK;
#endif
}

int32_t StorageDaemonProvider::LockUserScreen(uint32_t userId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::LockUserScreen userId %{public}d out of range", userId);
        return err;
    }
    int timerId = StorageXCollie::SetTimer("storage:LockUserScreen", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int ret = StorageDaemon::GetInstance().LockUserScreen(userId);
    StorageXCollie::CancelTimer(timerId);
    SetUserStatistics(userId, ret != E_OK ? KEY_UNLOAD_FAIL : KEY_UNLOAD_SUCCESS);
    return ret;
}

int32_t StorageDaemonProvider::UnlockUserScreen(uint32_t userId,
                                                const std::vector<uint8_t> &token,
                                                const std::vector<uint8_t> &secret)
{
    int timerId = StorageXCollie::SetTimer("storage:UnlockUserScreen", LOCAL_TIME_OUT_SECONDS);
    std::unique_lock<std::mutex> lock(mutex_);
    isNeedUpdateRadarFile_ = true;
    int ret = StorageDaemon::GetInstance().UnlockUserScreen(userId, token, secret);
    StorageXCollie::CancelTimer(timerId);
    SetUserStatistics(userId, ret != E_OK ? KEY_LOAD_FAIL : KEY_LOAD_SUCCESS);
    lock.unlock();

#ifdef USER_CRYPTO_MANAGER
    int cbRet = KeyManager::GetInstance().NotifyUeceActivation(userId, ret, false);
    if (ret != E_OK) { //unlock EL3-5 failed
        return ret;
    }
    if (cbRet != E_OK) { // unlock userAppkeys failed
        LOGE("failed to delete appkey2, cbRet=%{public}d", cbRet);
        return cbRet;
    }
#endif
    return ret;
}

int32_t StorageDaemonProvider::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::GetLockScreenStatus userId %{public}d out of range", userId);
        return err;
    }
    lockScreenStatus = false;
    int timerId = StorageXCollie::SetTimer("storage:GetLockScreenStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = StorageDaemon::GetInstance().GetLockScreenStatus(userId, lockScreenStatus);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::GenerateAppkey userId %{public}d out of range", userId);
        return err;
    }
    int timerId = StorageXCollie::SetTimer("storage:GenerateAppkey", LOCAL_TIME_OUT_SECONDS);
    int32_t ret = StorageDaemon::GetInstance().GenerateAppkey(userId, hashId, keyId, needReSet);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::DeleteAppkey userId %{public}d out of range", userId);
        return err;
    }
    int timerId = StorageXCollie::SetTimer("storage:DeleteAppkey", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = StorageDaemon::GetInstance().DeleteAppkey(userId, keyId);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::CreateRecoverKey(uint32_t userId,
                                                uint32_t userType,
                                                const std::vector<uint8_t> &token,
                                                const std::vector<uint8_t> &secret)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::CreateRecoverKey userId %{public}d out of range", userId);
        return err;
    }
    return StorageDaemon::GetInstance().CreateRecoverKey(userId, userType, token, secret);
}

int32_t StorageDaemonProvider::SetRecoverKey(const std::vector<uint8_t> &key)
{
    return StorageDaemon::GetInstance().SetRecoverKey(key);
}

int32_t StorageDaemonProvider::RawDataToStringVec(const StorageFileRawData &rawData,
    std::vector<std::string> &stringVec)
{
    if (rawData.data == nullptr) {
        LOGE("rawData is null");
        return ERR_DEAD_OBJECT;
    }
    if (rawData.size == 0 || rawData.size > MAX_IPC_RAW_DATA_SIZE) {
        LOGE("size invalid: %{public}u", rawData.size);
        return ERR_DEAD_OBJECT;
    }
    std::stringstream ss;
    ss.write(reinterpret_cast<const char *>(rawData.data), rawData.size);
    uint32_t stringVecSize = 0;
    ss.read(reinterpret_cast<char *>(&stringVecSize), sizeof(stringVecSize));
    uint32_t ssLength = static_cast<uint32_t>(ss.str().length());
    if (stringVecSize > MAX_URI_COUNT) {
        LOGE("out of range: %{public}u", stringVecSize);
        return ERR_DEAD_OBJECT;
    }
    for (uint32_t i = 0; i < stringVecSize; i++) {
        uint32_t strLen = 0;
        ss.read(reinterpret_cast<char *>(&strLen), sizeof(strLen));
        if (ss.tellg() == -1 || ssLength < static_cast<uint32_t>(ss.tellg())) {
            LOGE("ssLength length:%{public}u is invalid", ssLength);
            return ERR_DEAD_OBJECT;
        }
        if (strLen > ssLength - static_cast<uint32_t>(ss.tellg())) {
            LOGE("string length:%{public}u is invalid", strLen);
            return ERR_DEAD_OBJECT;
        }
        std::string str;
        str.resize(strLen);
        ss.read(&str[0], strLen);
        stringVec.emplace_back(str);
    }
    return ERR_OK;
}

int32_t StorageDaemonProvider::CreateShareFile(const StorageFileRawData &rawData,
                                               uint32_t tokenId,
                                               uint32_t flag,
                                               std::vector<int32_t> &funcResult)
{
    LOGI("Create Share file list len is %{public}u", rawData.size);
    funcResult.clear();
    std::vector<std::string> uriList;
    int32_t ret = RawDataToStringVec(rawData, uriList);
    if (ret != E_OK) {
        LOGE("RawDataToStringVec failed, ret is %{public}d", ret);
        return ret;
    }
    LOGI("StorageDaemonProvider::CreateShareFile start. file size is %{public}zu", uriList.size());

    void *dlhandler = dlopen("libfileshare_native.z.so", RTLD_LAZY);
    if (dlhandler == NULL) {
        LOGE("CreateShareFile cannot open so, errno = %{public}s", dlerror());
        return E_DLOPEN_ERROR;
    }

    typedef int32_t (*CreateShareFileFunc)(const std::vector<std::string>&, uint32_t, uint32_t, std::vector<int32_t>&);
    CreateShareFileFunc createShareFile = nullptr;
    createShareFile = reinterpret_cast<CreateShareFileFunc>(dlsym(dlhandler, "CreateShareFile"));
    if (createShareFile == nullptr) {
        LOGE("CreateShareFile dlsym failed, errno = %{public}s", dlerror());
        dlclose(dlhandler);
        return E_DLSYM_ERROR;
    }
    createShareFile(uriList, tokenId, flag, funcResult);
    dlclose(dlhandler);

    LOGI("StorageDaemonProvider::CreateShareFile end. result size is %{public}zu", funcResult.size());
    return E_OK;
}

int32_t StorageDaemonProvider::DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData)
{
    std::vector<std::string> uriList;
    int32_t ret = RawDataToStringVec(rawData, uriList);
    if (ret != E_OK) {
        LOGE("RawDataToStringVec failed, ret is %{public}d", ret);
        return ret;
    }

    void *dlhandler = dlopen("libfileshare_native.z.so", RTLD_LAZY);
    if (dlhandler == NULL) {
        LOGE("DeleteShareFile cannot open so, errno = %{public}s", dlerror());
        return E_DLOPEN_ERROR;
    }

    typedef int32_t (*DeleteShareFileFunc)(uint32_t, const std::vector<std::string> &);
    DeleteShareFileFunc deleteShareFile = nullptr;
    deleteShareFile = reinterpret_cast<DeleteShareFileFunc>(dlsym(dlhandler, "DeleteShareFile"));
    if (deleteShareFile == nullptr) {
        LOGE("DeleteShareFile dlsym failed, errno = %{public}s", dlerror());
        dlclose(dlhandler);
        return E_DLSYM_ERROR;
    }

    ret = deleteShareFile(tokenId, uriList);
    dlclose(dlhandler);
    return ret;
}

int32_t StorageDaemonProvider::SetBundleQuota(int32_t uid,
                                              const std::string &bundleDataDirPath,
                                              int32_t limitSizeMb)
{
    return QuotaManager::GetInstance().SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
}

int32_t StorageDaemonProvider::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    return QuotaManager::GetInstance().ListUserdataDirInfo(scanDirs);
}

int32_t StorageDaemonProvider::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    size = 0;
    return QuotaManager::GetInstance().GetOccupiedSpace(idType, id, size);
}

int32_t StorageDaemonProvider::MountDfsDocs(int32_t userId,
                                            const std::string &relativePath,
                                            const std::string &networkId,
                                            const std::string &deviceId)
{
    LOGI("StorageDaemon::MountDfsDocs start.");
    if (IsFilePathInvalid(relativePath)) {
        return E_PARAMS_INVALID;
    }
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemonProvider::MountDfsDocs userId %{public}d out of range", userId);
        return err;
    }
    return MountManager::GetInstance().MountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonProvider::UMountDfsDocs(int32_t userId,
                                             const std::string &relativePath,
                                             const std::string &networkId,
                                             const std::string &deviceId)
{
    LOGI("StorageDaemon::UMountDfsDocs start.");
    if (IsFilePathInvalid(relativePath)) {
        return E_PARAMS_INVALID;
    }
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemonProvider::UMountDfsDocs userId %{public}d out of range", userId);
        return err;
    }
    return MountManager::GetInstance().UMountDfsDocs(userId, relativePath, networkId, deviceId);
}

int32_t StorageDaemonProvider::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    isEncrypted = true;
    int timerId = StorageXCollie::SetTimer("storage:GetFileEncryptStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = StorageDaemon::GetInstance().GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::GetUserNeedActiveStatus userId %{public}d out of range", userId);
        return err;
    }
    needActive = false;
    int timerId = StorageXCollie::SetTimer("storage:GetUserNeedActiveStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = StorageDaemon::GetInstance().GetUserNeedActiveStatus(userId, needActive);
    StorageXCollie::CancelTimer(timerId);
    return ret;
}

int32_t StorageDaemonProvider::MountMediaFuse(int32_t userId, int32_t &devFd)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::MountMediaFuse userId %{public}d out of range", userId);
        return err;
    }
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageDaemonProvider::MountMediaFuse start.");
    devFd = -1;
    return MountManager::GetInstance().MountMediaFuse(userId, devFd);
#endif
    return E_OK;
}

int32_t StorageDaemonProvider::UMountMediaFuse(int32_t userId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UMountMediaFuse userId %{public}d out of range", userId);
        return err;
    }
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageDaemonProvider::UMountMediaFuse start.");
    return MountManager::GetInstance().UMountMediaFuse(userId);
#endif
    return E_OK;
}

int32_t StorageDaemonProvider::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }

    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::MountFileMgrFuse userId %{public}d out of range", userId);
        return err;
    }
    LOGI("StorageDaemonProvider::MountFileMgrFuse, userId:%{public}d.", userId);
    fuseFd = -1;
    return MountManager::GetInstance().MountFileMgrFuse(userId, path, fuseFd);
}

int32_t StorageDaemonProvider::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }

    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::UMountFileMgrFuse userId %{public}d out of range", userId);
        return err;
    }
    LOGI("StorageDaemonProvider::UMountFileMgrFuse, userId:%{public}d.", userId);
    return MountManager::GetInstance().UMountFileMgrFuse(userId, path);
}

int32_t StorageDaemonProvider::IsFileOccupied(const std::string &path,
                                              const std::vector<std::string> &inputList,
                                              std::vector<std::string> &outputList,
                                              bool &isOccupy)
{
    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    isOccupy = false;
    return StorageDaemon::GetInstance().IsFileOccupied(path, inputList, outputList, isOccupy);
}

int32_t StorageDaemonProvider::ResetSecretWithRecoveryKey(uint32_t userId,
                                                          uint32_t rkType,
                                                          const std::vector<uint8_t> &key)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::ResetSecretWithRecoveryKey userId %{public}d out of range", userId);
        return err;
    }
    return StorageDaemon::GetInstance().ResetSecretWithRecoveryKey(userId, rkType, key);
}

void StorageDaemonProvider::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
                                                                                  const std::string &deviceId)
{
    LOGI("SystemAbilityId:%{public}d", systemAbilityId);
#ifdef EXTERNAL_STORAGE_MANAGER
    if (systemAbilityId == ACCESS_TOKEN_MANAGER_SERVICE_ID) {
        DiskManager::Instance().ReplayUevent();
    }
#endif
    if (systemAbilityId == FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID) {
        SystemMountManager::GetInstance().SetCloudState(true);
    }
}

void StorageDaemonProvider::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(int32_t systemAbilityId,
                                                                                     const std::string &deviceId)
{
    LOGI("SystemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId == FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID) {
        SystemMountManager::GetInstance().SetCloudState(false);
    }
}

int32_t StorageDaemonProvider::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    if (userId <= 0) {
        LOGE("mount share file, userId %{public}d is invalid.", userId);
        return E_PARAMS_INVALID;
    }
    for (const auto &item : shareFiles) {
        if (item.first.find("..") != std::string::npos || item.second.find("..") != std::string::npos) {
            LOGE("mount share file, shareFiles is invalid.");
            return E_PARAMS_INVALID;
        }
    }
    return MountManager::GetInstance().MountDisShareFile(userId, shareFiles);
}

int32_t StorageDaemonProvider::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    if (userId <= 0) {
        LOGE("umount share file, userId %{public}d is invalid.", userId);
        return E_PARAMS_INVALID;
    }
    if (networkId.find("..") != std::string::npos) {
        LOGE("umount share file, networkId is invalid.");
        return E_PARAMS_INVALID;
    }
    return MountManager::GetInstance().UMountDisShareFile(userId, networkId);
}

int32_t StorageDaemonProvider::InactiveUserPublicDirKey(uint32_t userId)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("StorageDaemon::InactiveUserPublicDirKey userId %{public}d out of range", userId);
        return err;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return StorageDaemon::GetInstance().InactiveUserPublicDirKey(userId);
}

int32_t StorageDaemonProvider::UpdateUserPublicDirPolicy(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return StorageDaemon::GetInstance().UpdateUserPublicDirPolicy(userId);
}

int32_t StorageDaemonProvider::QueryOccupiedSpaceForSa(std::string &storageStatus,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    QuotaManager::GetInstance().GetUidStorageStats(storageStatus, bundleNameAndUid);
    return E_OK;
}

int32_t StorageDaemonProvider::RegisterUeceActivationCallback(
    const sptr<StorageManager::IUeceActivationCallback> &ueceCallback)
{
    if (ueceCallback == nullptr) {
        LOGE("callback is nullptr");
        return E_PARAMS_NULLPTR_ERR;
    }
    return StorageDaemon::GetInstance().RegisterUeceActivationCallback(ueceCallback);
}

int32_t StorageDaemonProvider::UnregisterUeceActivationCallback()
{
    return StorageDaemon::GetInstance().UnregisterUeceActivationCallback();
}

int32_t StorageDaemonProvider::CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    return UserManager::GetInstance().CreateUserDir(path, mode, uid, gid);
}

int32_t StorageDaemonProvider::DeleteUserDir(const std::string &path)
{
    if (IsFilePathInvalid(path)) {
        return E_PARAMS_INVALID;
    }
    return UserManager::GetInstance().DeleteUserDir(path);
}

int32_t StorageDaemonProvider::GetDqBlkSpacesByUids(const std::vector<int32_t> &uids,
    std::vector<NextDqBlk> &dqBlks)
{
    return QuotaManager::GetInstance().GetDqBlkSpacesByUids(uids, dqBlks);
}

int32_t StorageDaemonProvider::GetDirListSpace(const std::vector<DirSpaceInfo> &inDirs,
    std::vector<DirSpaceInfo> &outDirs)
{
    outDirs = inDirs;
    return QuotaManager::GetInstance().GetDirListSpace(outDirs);
}

int32_t StorageDaemonProvider::SetStopScanFlag(bool stop)
{
    QuotaManager::GetInstance().SetStopScanFlag(stop);
    return E_OK;
}

int32_t StorageDaemonProvider::GetAncoSizeData(std::string &outExtraData)
{
    QuotaManager::GetInstance().GetAncoSizeData(outExtraData);
    return E_OK;
}

int32_t StorageDaemonProvider::GetDataSizeByPath(const std::string &path, int64_t &size)
{
    return QuotaManager::GetInstance().GetFileData(path, size);
}

int32_t StorageDaemonProvider::GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    return OHOS::StorageDaemon::GetRmgResourceSize(rgmName, totalSize);
}

int32_t StorageDaemonProvider::ClearSecondMountPoint(uint32_t userId, const std::string &bundleName)
{
    return MountManager::GetInstance().ClearSecondMountPoint(userId, bundleName);
}
} // namespace StorageDaemon
} // namespace OHOS
