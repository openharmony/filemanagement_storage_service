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

#include "dfx_report/storage_dfx_reporter.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>
#include "statistic_info.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_radar.h"
#include "storage_service_constant.h"
#include "storage/storage_total_status_service.h"
#include "storage/storage_status_manager.h"
#include "storage_stats.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "storage/bundle_manager_connector.h"
#include "os_account_manager.h"

namespace OHOS {
namespace StorageManager {
constexpr double DIVISOR = 1000.0 * 1000.0;
constexpr double BASE_NUMBER = 10.0;
constexpr int32_t ACCURACY_NUM = 2;
constexpr int32_t TOP_COUNT = 20; // Top N directories to report
constexpr int64_t TIME_INTERVAL_HOURS = 24;

// HMFS metadata constants
constexpr const char *HMFS_PATH = "/sys/fs/hmfs/userdata";
constexpr const char *MAIN_BLKADDR = "/main_blkaddr";
constexpr const char *OVP_CHUNKS = "/ovp_chunks";
constexpr uint64_t FOUR_K = 4096;
constexpr uint64_t BLOCK_COUNT = 512;
constexpr int32_t SEVEN_DAYS_IN_HOURS = 7 * 24;

constexpr int32_t ROOT_UID = 0;
constexpr int32_t SYSTEM_UID = 1000;
constexpr int32_t FOUNDATION_UID = 5523;
static std::vector<int32_t> SYS_UIDS = {0, 1000, 5523};

StorageDfxReporter::~StorageDfxReporter()
{
    LOGI("StorageDfxReporter destructor called.");
    if (hapAndSaThread_.joinable()) {
        LOGI("Waiting for hapAndSaThread to finish...");
        hapAndSaThread_.join();
    }
    LOGI("StorageDfxReporter destructor completed.");
}

void StorageDfxReporter::StartReportHapAndSaStorageStatus()
{
    LOGI("StorageDfxReporter StartReportHapAndSaStorageStatus start.");
    if (isHapAndSaRunning_.load()) {
        LOGI("Hap and Sa statistics task is already running, ignoring this request.");
        return;
    }
    if (hapAndSaThread_.joinable()) {
        LOGI("Previous Hap and Sa thread is still joinable, joining it.");
        hapAndSaThread_.join();
    }
    isHapAndSaRunning_.store(true);
    int32_t userId = StorageService::DEFAULT_USERID;
    hapAndSaThread_ = std::thread([this, userId]() {
        pthread_setname_np(pthread_self(), "hap_sa_stats_task");
        ExecuteHapAndSaStatistics(userId);
    });
    LOGI("StartReportHapAndSaStorageStatus launched async task successfully.");
}

bool StorageDfxReporter::CheckTimeIntervalTriggered(const std::chrono::system_clock::time_point &lastTime,
    int64_t timeIntervalHours, int64_t &hoursDiff)
{
    if (lastTime.time_since_epoch().count() == 0) {
        hoursDiff = 0;
        return false;
    }
    auto currentTime = std::chrono::system_clock::now();
    hoursDiff = std::chrono::duration_cast<std::chrono::hours>(
        currentTime - lastTime).count();
    bool triggered = hoursDiff >= timeIntervalHours;
    if (triggered) {
        LOGI("Time interval >= %{public}lld hours, hours: %{public}lld",
             static_cast<long long>(timeIntervalHours), static_cast<long long>(hoursDiff));
    }
    return triggered;
}

bool StorageDfxReporter::CheckValueChangeTriggered(int64_t currentValue, int64_t lastValue, int64_t threshold,
    int64_t &valueDiff)
{
    valueDiff = std::abs(currentValue - lastValue);
    bool triggered = valueDiff >= threshold;
    if (triggered) {
        LOGI("Free size diff >= %{public}lld bytes, diff: %{public}lld bytes",
             static_cast<long long>(threshold), static_cast<long long>(valueDiff));
    }

    return triggered;
}

void StorageDfxReporter::CheckAndTriggerHapAndSaStatistics()
{
    LOGI("CheckAndTriggerHapAndSaStatistics start.");

    int64_t lastFreeSize = 0;
    std::chrono::system_clock::time_point lastTime;
    {
        std::lock_guard<std::mutex> lock(hapAndSaStateMutex_);
        lastFreeSize = lastHapAndSaFreeSize_;
        lastTime = lastHapAndSaTime_;
    }
    int64_t currentFreeSize = 0;
    if (StorageTotalStatusService::GetInstance().GetFreeSize(currentFreeSize) != E_OK || currentFreeSize < 0) {
        LOGE("Get current free size failed");
        return;
    }
    int64_t hoursDiff = 0;
    int64_t sizeDiff = 0;
    bool timeTriggered = CheckTimeIntervalTriggered(lastTime, SEVEN_DAYS_IN_HOURS, hoursDiff);
    bool sizeTriggered = CheckValueChangeTriggered(currentFreeSize, lastFreeSize, StorageService::TWO_G_BYTE,
        sizeDiff);
    if (timeTriggered || sizeTriggered) {
        LOGI("Trigger statistic - reason: %{public}s, current: %{public}lld, last: %{public}lld",
             timeTriggered ? "time >= 7d" : "size >= 2GB",
             static_cast<long long>(currentFreeSize), static_cast<long long>(lastFreeSize));
        StartReportHapAndSaStorageStatus();
    } else {
        LOGI("Skip statistic - time: %{public}lld hrs, size: %{public}lld bytes",
             static_cast<long long>(hoursDiff), static_cast<long long>(sizeDiff));
    }
}

void StorageDfxReporter::GetCurrentTime(std::ostringstream &extraData)
{
    auto now = std::chrono::system_clock::now();
    auto timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    struct tm timeInfo;
    localtime_r(&timeT, &timeInfo);
    std::ostringstream timeStr;
    timeStr << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    extraData << "{timeStamp:" << timeStamp
              << ",time:" << timeStr.str() << "}" << std::endl;
}

void StorageDfxReporter::ExecuteHapAndSaStatistics(int32_t userId)
{
    LOGI("Hap and Sa statistics thread started.");
    std::ostringstream extraData;
    GetCurrentTime(extraData);
    int32_t ret = CollectStorageStats(userId, extraData);
    if (ret != E_OK) {
        isHapAndSaRunning_.store(false);
        return;
    }
    CollectMetadataAndAnco(extraData);
    ret = CollectBundleStatistics(userId, extraData);
    StorageService::StorageRadar::ReportSpaceRadar("StartReportHapAndSaStorageStatus",
        E_STORAGE_STATUS, extraData.str());
    LOGI("StorageDfxReporter StartReportHapAndSaStorageStatus end.");
    UpdateHapAndSaState();
    isHapAndSaRunning_.store(false);
    LOGI("Hap and Sa statistics thread completed.");
}

int32_t StorageDfxReporter::CollectStorageStats(int32_t userId, std::ostringstream &extraData)
{
    StorageStats storageStatsInfo;
    int32_t ret = GetStorageStatsInfo(userId, storageStatsInfo);
    if (ret != E_OK) {
        LOGE("GetStorageStatsInfo failed, ret=%{public}d", ret);
        return ret;
    }
    int64_t freeSize = 0;
    int64_t systemSize = 0;
    StorageTotalStatusService::GetInstance().GetFreeSize(freeSize);
    StorageTotalStatusService::GetInstance().GetSystemSize(systemSize);

    extraData << "{userId is:" << userId << "}\n";
    extraData << "{app size is:" << ConvertBytesToMB(storageStatsInfo.app_, ACCURACY_NUM) << "MB";
    extraData << ",audio size is:" << ConvertBytesToMB(storageStatsInfo.audio_, ACCURACY_NUM) << "MB";
    extraData << ",image size is:" << ConvertBytesToMB(storageStatsInfo.image_, ACCURACY_NUM) << "MB";
    extraData << ",video size is:" << ConvertBytesToMB(storageStatsInfo.video_, ACCURACY_NUM) << "MB";
    extraData << ",file size is:" << ConvertBytesToMB(storageStatsInfo.file_, ACCURACY_NUM) << "MB";
    extraData << ",total size is:" << ConvertBytesToMB(storageStatsInfo.total_, ACCURACY_NUM) << "MB";
    extraData << ",sys size is:" << ConvertBytesToMB(systemSize, ACCURACY_NUM) << "MB";
    extraData << ",free size is:" << ConvertBytesToMB(freeSize, ACCURACY_NUM) << "MB}" << std::endl;
    return E_OK;
}

void StorageDfxReporter::CollectMetadataAndAnco(std::ostringstream &extraData)
{
    GetMetaDataSize(extraData);
    GetAncoDataSize(extraData);
}

int32_t StorageDfxReporter::CollectBundleStatistics(int32_t userId, std::ostringstream &extraData)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication =
        DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("Get StorageDaemonCommunication instance failed.");
        return E_ERR;
    }
    std::map<int32_t, std::string> bundleNameAndUid;
    int32_t ret = StorageStatusManager::GetInstance().GetBundleNameAndUid(userId, bundleNameAndUid);
    if (ret != E_OK) {
        LOGE("GetBundleNameAndUid failed, ret=%{public}d", ret);
    }
    std::string storageStatusRes = "";
    ret = sdCommunication->QueryOccupiedSpaceForSa(storageStatusRes, bundleNameAndUid);
    if (ret == E_OK) {
        extraData << storageStatusRes;
    }
    extraData << "{bundleCount:" << bundleNameAndUid.size() << "}" << std::endl;
    return E_OK;
}

void StorageDfxReporter::UpdateHapAndSaState()
{
    std::lock_guard<std::mutex> lock(hapAndSaStateMutex_);
    int64_t currentFreeSize = 0;
    int32_t err = StorageTotalStatusService::GetInstance().GetFreeSize(currentFreeSize);
    if (err == E_OK && currentFreeSize >= 0) {
        lastHapAndSaFreeSize_ = currentFreeSize;
        lastHapAndSaTime_ = std::chrono::system_clock::now();
        LOGI("Updated Hap and Sa state: freeSize=%{public}lld", static_cast<long long>(lastHapAndSaFreeSize_));
    }
}

double StorageDfxReporter::ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces)
{
    if (bytes < 0) {
        return 0.0;
    }
    double mb = static_cast<double>(bytes) / DIVISOR;

    if (decimalPlaces < 0) {
        decimalPlaces = 0;
    }
    double factor = std::pow(BASE_NUMBER, decimalPlaces);
    if (factor == 0) {
        return 0.0;
    }
    return std::round(mb * factor) / factor;
}

int32_t StorageDfxReporter::GetStorageStatsInfo(int32_t userId, StorageStats &storageStats)
{
    LOGI("GetStorageStatsInfo start, userId=%{public}d", userId);

    int32_t ret = StorageStatusManager::GetInstance().GetUserStorageStats(userId, storageStats, true);
    if (ret != E_OK) {
        LOGE("GetUserStorageStats failed, userId=%{public}d, ret=%{public}d", userId, ret);
        return ret;
    }

    LOGI("GetStorageStatsInfo success, userId=%{public}d", userId);
    return E_OK;
}

void StorageDfxReporter::GetMetaDataSize(std::ostringstream &extraData)
{
    LOGI("begin get MetaData info.");

    std::string blkPath = std::string(HMFS_PATH) + std::string(MAIN_BLKADDR);
    std::string chunkPath = std::string(HMFS_PATH) + std::string(OVP_CHUNKS);

    int64_t blkSize = -1;
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("Get StorageDaemonCommunication instance failed.");
        return;
    }

    auto ret = sdCommunication->GetDataSizeByPath(blkPath, blkSize);
    if (ret == E_OK && blkSize != -1) {
        extraData << "{main_blkaddr data is:" << ConvertBytesToMB(blkSize * FOUR_K, ACCURACY_NUM)
        << "MB}" << std::endl;
    } else {
        extraData << "{get main_blkaddr wrong, ret:" << ret << ",blkSize is:" << blkSize << "}" << std::endl;
    }

    int64_t chunkSize = -1;
    ret = sdCommunication->GetDataSizeByPath(chunkPath, chunkSize);
    if (ret == E_OK && chunkSize != -1) {
        extraData << "{ovp_chunks data is:" << ConvertBytesToMB(chunkSize * FOUR_K * BLOCK_COUNT, ACCURACY_NUM)
        << "MB}" << std::endl;
    } else {
        extraData << "{get ovp_chunks wrong, ret:" << ret << ",chunkSize is:" << chunkSize << "}" << std::endl;
    }

    if (blkSize != -1 && chunkSize != -1) {
        extraData << "{metaData is:"
                  << ConvertBytesToMB(blkSize * FOUR_K + chunkSize * FOUR_K * BLOCK_COUNT, ACCURACY_NUM)
                  << "MB}" << std::endl;
    }

    LOGI("end get MetaData info.");
}

void StorageDfxReporter::GetAncoDataSize(std::ostringstream &extraData)
{
    LOGI("begin get Anco info.");

    uint64_t imageSize = 0;
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("Get StorageDaemonCommunication instance failed.");
        return;
    }
    sdCommunication->GetRmgResourceSize("rgm_hmos", imageSize);
    extraData << "{anco image size:" << ConvertBytesToMB(imageSize, ACCURACY_NUM) << "MB}" << std::endl;
    LOGI("end get Anco info.");
}

int32_t StorageDfxReporter::StartReportDirStatus()
{
    LOGI("StorageDfxReporter StartReportDirStatus start.");

    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("Get StorageDaemonCommunication instance failed.");
        return E_ERR;
    }
    std::vector<NextDqBlk> dqBlks;
    int32_t ret = sdCommunication->GetDqBlkSpacesByUids(SYS_UIDS, dqBlks);
    if (ret != E_OK) {
        LOGE("GetDqBlkSpacesByUids failed, ret=%{public}d.", ret);
        return ret;
    }
    int64_t totalSize = 0;
    int64_t rootSize = 0;
    int64_t systemSize = 0;
    int64_t foundationSize = 0;
    ret = CheckSystemUidSize(dqBlks, totalSize, rootSize, systemSize, foundationSize);
    if (ret != E_OK) {
        return ret;
    }
    std::ostringstream extraData;
    GetCurrentTime(extraData);
    CollectDirStatistics(rootSize, systemSize, foundationSize, extraData);
    StorageService::StorageRadar::ReportSpaceRadar("StartReportDirStatus",
        E_SYS_DIR_SPACE_STATUS, extraData.str());
    LOGI("StorageDfxReporter StartReportDirStatus end.");
    return UpdateScanState(totalSize);
}

int32_t StorageDfxReporter::CheckSystemUidSize(const std::vector<NextDqBlk> &dqBlks, int64_t &totalSize,
    int64_t &rootSize, int64_t &systemSize, int64_t &foundationSize)
{
    LOGI("GetDqBlkSpacesByUids success, got %{public}zu quota blocks.", dqBlks.size());
    totalSize = 0;
    rootSize = 0;
    systemSize = 0;
    foundationSize = 0;
    for (size_t i = 0; i < dqBlks.size() && i < SYS_UIDS.size(); i++) {
        int64_t uidSize = static_cast<int64_t>(dqBlks[i].dqbCurSpace);
        int32_t uid = static_cast<int32_t>(dqBlks[i].dqbId);
        if (uidSize < StorageService::TWO_G_BYTE) {
            LOGE("uid=%{public}d size %{public}lld bytes less than 2GB, break.",
                 uid, static_cast<long long>(uidSize));
            return E_ERR;
        }
        totalSize += uidSize;
        if (uid == ROOT_UID) {
            rootSize = uidSize;
        } else if (uid == SYSTEM_UID) {
            systemSize = uidSize;
        } else if (uid == FOUNDATION_UID) {
            foundationSize = uidSize;
        }
        LOGI("uid=%{public}d, curSpace=%{public}lld bytes", SYS_UIDS[i], static_cast<long long>(uidSize));
    }
    int64_t sizeIncrease = abs(totalSize - lastTotalSize_);
    if (sizeIncrease < StorageService::ONE_G_BYTE) {
        LOGE("Total size increase %{public}lld bytes (from %{public}lld to %{public}lld) is less than 1GB, "
             "skip dir statistics.",
             static_cast<long long>(sizeIncrease), static_cast<long long>(lastTotalSize_),
             static_cast<long long>(totalSize));
        return E_ERR;
    }
    return E_OK;
}

void StorageDfxReporter::CollectDirStatistics(int64_t rootSize, int64_t systemSize,
    int64_t foundationSize, std::ostringstream &extraData)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunicationInstance;
    sdCommunicationInstance = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunicationInstance == nullptr) {
        LOGE("Get StorageDaemonCommunication instance failed.");
        return;
    }
    std::vector<DirSpaceInfo> rootDirs = GetRootDirList();
    std::vector<DirSpaceInfo> rootOutDirs;
    int32_t ret = sdCommunicationInstance->GetDirListSpace(rootDirs, rootOutDirs);
    if (ret == E_OK) {
        extraData << "{root size:" << ConvertBytesToMB(rootSize, ACCURACY_NUM) << "MB}" << std::endl;
        extraData << "{root directories:}" << std::endl;
        AppendDirInfo(rootOutDirs, extraData);
    }
    std::vector<DirSpaceInfo> systemDirs = GetSystemDirList();
    std::vector<DirSpaceInfo> systemOutDirs;
    ret = sdCommunicationInstance->GetDirListSpace(systemDirs, systemOutDirs);
    if (ret == E_OK) {
        extraData << "{system size:" << ConvertBytesToMB(systemSize, ACCURACY_NUM) << "MB}" << std::endl;
        extraData << "{system directories:}" << std::endl;
        AppendDirInfo(systemOutDirs, extraData);
    }
    std::vector<DirSpaceInfo> foundationDirs = GetFoundationDirList();
    std::vector<DirSpaceInfo> foundationOutDirs;
    ret = sdCommunicationInstance->GetDirListSpace(foundationDirs, foundationOutDirs);
    if (ret == E_OK) {
        extraData << "{foundation size:" << ConvertBytesToMB(foundationSize, ACCURACY_NUM) << "MB}" << std::endl;
        extraData << "{foundation directories:}" << std::endl;
        AppendDirInfo(foundationOutDirs, extraData);
    }
    std::string ancoData;
    ret = sdCommunicationInstance->GetAncoSizeData(ancoData);
    if (ret == E_OK) {
        extraData << ancoData;
    }
}

int32_t StorageDfxReporter::UpdateScanState(int64_t totalSize)
{
    std::lock_guard<std::mutex> lock(scanStateMutex_);
    int64_t currentFreeSize = 0;
    int32_t err = StorageTotalStatusService::GetInstance().GetFreeSize(currentFreeSize);
    if (err != E_OK || currentFreeSize < 0) {
        LOGE("Failed to get free size, err=%{public}d", err);
        return E_ERR;
    }
    lastScanFreeSize_ = currentFreeSize;
    lastTotalSize_ = totalSize;
    lastScanTime_ = std::chrono::system_clock::now();
    LOGI("Updated scan state: freeSize=%{public}lld", static_cast<long long>(lastScanFreeSize_));
    return E_OK;
}

void StorageDfxReporter::AppendDirInfo(const std::vector<DirSpaceInfo> &dirs,
    std::ostringstream &extraData)
{
    int32_t count = 0;
    for (const auto& info : dirs) {
        if (count >= TOP_COUNT) {
            break;
        }
        extraData << "  {path:" << info.path
                  << ",uid:" << info.uid
                  << ",size:" << ConvertBytesToMB(info.size, ACCURACY_NUM)
                  << "MB}" << std::endl;
        count++;
    }
}

std::vector<DirSpaceInfo> StorageDfxReporter::GetRootDirList()
{
    return {{"/data/app/el1/0/shader_cache", 0, 0},
            {"/data/log/bbox", 0, 0},
            {"/data/log/sysres", 0, 0},
            {"/data/log/last_tee", 0, 0},
            {"/data/log/security_guard", 0, 0},
            {"/data/log/hiaudit/storageservice", 0, 0},
            {"/data/app/el1/bundle/public", 0, 0},
            {"/data/service/el1/public/udev", 0, 0},
            {"/data/service/el1/public/i18n", 0, 0},
            {"/data/service/el1/public/hosts_user", 0, 0},
            {"/data/service/el1/public/usb/mode", 0, 0},
            {"/data/service/el0/startup", 0, 0},
            {"/data/service/el0/access_token", 0, 0},
            {"/data/service/el0/storage_daemon", 0, 0},
            {"/data/service/el0/public/for-all-app", 0, 0},
            {"/data/service/el1/startup", 0, 0},
            {"/data/service/el1/public/for-all-app", 0, 0},
            {"/data/service/el1/public/startup", 0, 0},
            {"/data/service/el1/public/storage_daemon", 0, 0},
            {"/data/service/el1/public/hdc", 0, 0},
            {"/data/service/el1/public/usb/mode", 0, 0},
            {"/data/service/el1/public/udev", 0, 0},
            {"/data/service/el1/public/i18n", 0, 0},
            {"/data/service/el1/public/rgm_engine", 0, 0},
            {"/data/service/el1/network", 0, 0},
            {"/data/service/el1/0/hyperhold", 0, 0},
            {"/data/local/shader_cache", 0, 0},
            {"/data/local/tmp", 0, 0},
            {"/data/app/el1/0/aot_compiler", 0, 0},
            {"/data/app/el1/0/base", 0, 0},
            {"/data/app/el1/0/system_optimize", 0, 0},
            {"/data/app/el1/public/aot_compiler", 0, 0},
            {"/data/app/el1/public/shader_cache", 0, 0},
            {"/data/app/el1/%d/aot_compiler", 0, 0},
            {"/data/app/el1/%d/shader_cache", 0, 0},
            {"/data/app/el1/%d/system_optimize", 0, 0},
            {"/data/apr_dumplogs", 0, 0},
            {"/data/misc/shared_relro", 0, 0},
            {"/data/vendor/hyperhold", 0, 0},
            {"/data/vendor/log/thplog", 0, 0},
            {"/data/vendor/log/dfx_logs", 0, 0},
            {"/data/vendor/log/wifi", 0, 0},
            {"/data/vendor/log/hi110x", 0, 0},
            {"/data/hisi_logs", 0, 0},
            {"/data/uwb", 0, 0},
            {"/data/log/tcpdump", 0, 0},
            {"/data/virt_service", 0, 0},
            {"/data/updater/log", 0, 0}};
}

std::vector<DirSpaceInfo> StorageDfxReporter::GetSystemDirList()
{
    return {{"/data/hmos4", 1000, 0},
            {"/data/service/el1/public/for-all-app/framework_ark_cache", 1000, 0},
            {"/data/service/el1/public/shader_cache", 1000, 0},
            {"/data/service/el2/100/virt_service", 1000, 0},
            {"/data/log/faultlog/temp", 1000, 0},
            {"/data/log/hiview", 1000, 0},
            {"/data/system/RTShaderCache", 1000, 0},
            {"/data/local/shader_cache/local/system", 1000, 0},
            {"/data/local/mtp_tmp", 1000, 0},
            {"/data/virt_service", 1000, 0},
            {"/data/misc", 1000, 0},
            {"/data/vendor/log", 1000, 0},
            {"/data/data", 1000, 0}};
}

std::vector<DirSpaceInfo> StorageDfxReporter::GetFoundationDirList()
{
    return {{"/data/service/el1/public/AbilityManagerService", 5523, 0},
            {"/data/service/el1/public/database/bundle_manager_service", 5523, 0},
            {"/data/service/el1/public/database/notification_service", 5523, 0},
            {"/data/service/el1/public/database/form_storage", 5523, 0},
            {"/data/service/el1/public/database/common_event_service", 5523, 0},
            {"/data/service/el1/public/database/auto_startup_service", 5523, 0},
            {"/data/service/el1/public/database/app_config_data", 5523, 0},
            {"/data/service/el1/public/database/app_exit_reason", 5523, 0},
            {"/data/service/el1/public/database/ability_manager_service", 5523, 0},
            {"/data/service/el1/public/database/keep_alive_service", 5523, 0},
            {"/data/service/el1/public/database/insight_intent", 5523, 0},
            {"/data/service/el1/public/notification", 5523, 0},
            {"/data/service/el1/public/window", 5523, 0},
            {"/data/service/el1/public/ecological_rule_mgr_service", 5523, 0},
            {"/data/service/el1/public/app_domain_verify_mgr_service", 5523, 0},
            {"/data/service/el1/public/screenlock", 5523, 0},
            {"/data/service/el1/public/bms/bundle_manager_service", 5523, 0},
            {"/data/service/el1/public/bms/bundle_resources", 5523, 0},
            {"/data/service/el1/0/utdtypes", 5523, 0},
            {"/data/service/el1/%d/utdtypes", 5523, 0},
            {"/data/log/eventlog/freeze", 5523, 0}};
}

bool StorageDfxReporter::CheckScanPreconditions()
{
    if (isScanRunning_.load()) {
        LOGI("Scan task is already running, ignoring this request.");
        return false;
    }

    int64_t currentFreeSize = 0;
    int32_t err = StorageTotalStatusService::GetInstance().GetFreeSize(currentFreeSize);
    if (err != E_OK || currentFreeSize < 0) {
        LOGE("Failed to get free size, err=%{public}d", err);
        return false;
    }
    {
        std::lock_guard<std::mutex> stateLock(scanStateMutex_);
        int64_t sizeDiff = std::abs(currentFreeSize - lastScanFreeSize_);
        if (sizeDiff < StorageService::TWO_G_BYTE) {
            LOGI("Free size diff %{public}lld < 2GB, skip scan.", static_cast<long long>(sizeDiff));
            return false;
        }
        auto currentTime = std::chrono::system_clock::now();
        int64_t duration = std::chrono::duration_cast<std::chrono::hours>(currentTime - lastScanTime_).count();
        if (duration < TIME_INTERVAL_HOURS && lastScanTime_.time_since_epoch().count() != 0) {
            LOGI("Last scan was %{public}lld hours ago, less than 24 hours, skip scan.",
                 static_cast<long long>(duration));
            return false;
        }
        LOGI("Starting scan task - free size diff: %{public}lld, hours since last scan: %{public}lld",
             static_cast<long long>(sizeDiff), static_cast<long long>(duration));
    }
    return true;
}

void StorageDfxReporter::LaunchScanWorker()
{
    isScanRunning_.store(true);
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication != nullptr) {
        int32_t ret = sdCommunication->SetStopScanFlag(false);
        if (ret != E_OK) {
            LOGE("Failed to reset stop scan flag, ret=%{public}d", ret);
        } else {
            LOGI("Successfully reset stop scan flag.");
        }
    }
    std::thread([this]() {
        pthread_setname_np(pthread_self(), "storage_scan_task");
        LOGI("Scan thread started.");

        int32_t ret = StartReportDirStatus();
        if (ret == E_OK) {
            LOGI("Scan completed successfully.");
        } else {
            LOGE("Scan failed with ret=%{public}d", ret);
        }

        isScanRunning_.store(false);
        LOGI("Scan thread completed.");
    }).detach();
    LOGI("StartScan completed, scan task launched as detached thread.");
}

void StorageDfxReporter::StartScan()
{
    LOGI("StartScan called.");
    std::lock_guard<std::mutex> lock(scanMutex_);
    if (!CheckScanPreconditions()) {
        return;
    }
    LaunchScanWorker();
}

void StorageDfxReporter::StopScan()
{
    LOGI("StopScan called.");
    if (!isScanRunning_.load()) {
        LOGI("No scan is running, skip StopScan.");
        return;
    }
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication != nullptr) {
        int32_t ret = sdCommunication->SetStopScanFlag(true);
        if (ret != E_OK) {
            LOGE("Failed to set stop scan flag, ret=%{public}d", ret);
        } else {
            LOGI("Successfully set stop scan flag to true.");
        }
    } else {
        LOGE("StorageDaemonCommunication instance is nullptr.");
    }
    if (isScanRunning_.load()) {
        isScanRunning_.store(false);
        LOGI("Force reset isScanRunning flag to false.");
    }
    LOGI("StopScan completed - all scan resources cleaned up.");
}
} // namespace StorageManager
} // namespace OHOS
