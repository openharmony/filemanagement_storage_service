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
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_radar.h"
#include "storage_service_constant.h"
#include "storage/storage_total_status_service.h"
#include "storage/storage_status_service.h"
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

// HMFS metadata constants
constexpr const char *HMFS_PATH = "/sys/fs/hmfs/userdata";
constexpr const char *MAIN_BLKADDR = "/main_blkaddr";
constexpr const char *OVP_CHUNKS = "/ovp_chunks";
constexpr uint64_t FOUR_K = 4096;
constexpr uint64_t BLOCK_COUNT = 512;

void StorageDfxReporter::StartReportHapAndSaStorageStatus()
{
    LOGI("StorageDfxReporter StartReportHapAndSaStorageStatus start.");
    // 1. 检查是否有任务正在执行
    if (isHapAndSaRunning_.load()) {
        LOGI("Hap and Sa statistics task is already running, ignoring this request.");
        return;
    }
    // 2. 启动异步线程执行所有耗时统计任务
    std::lock_guard<std::mutex> lock(hapAndSaStateMutex_);
    // 如果之前的线程还在运行,先join
    if (hapAndSaThread_.joinable()) {
        LOGI("Previous Hap and Sa thread is still joinable, joining it.");
        hapAndSaThread_.join();
    }
    // 设置运行标志
    isHapAndSaRunning_.store(true);
    // 创建新线程执行统计任务
    int32_t userId = StorageService::DEFAULT_USERID;
    hapAndSaThread_ = std::thread([this, userId]() {
        pthread_setname_np(pthread_self(), "hap_sa_stats_task");
        ExecuteHapAndSaStatistics(userId);
    });
    LOGI("StartReportHapAndSaStorageStatus launched async task successfully.");
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
    // 收集存储统计信息
    int32_t ret = CollectStorageStats(userId, extraData);
    if (ret != E_OK) {
        isHapAndSaRunning_.store(false);
        return;
    }
    // 收集元数据和Anco信息
    CollectMetadataAndAnco(extraData);
    // 收集Bundle统计信息
    ret = CollectBundleStatistics(userId, extraData);
    // 上报打点数据
    StorageService::StorageRadar::ReportSpaceRadar("StartReportHapAndSaStorageStatus",
        E_STORAGE_STATUS, extraData.str());
    LOGI("StorageDfxReporter StartReportHapAndSaStorageStatus end.");
    // 更新状态
    UpdateHapAndSaState();
    // 重置运行标志
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
    extraData << ",total size is:" << storageStatsInfo.total_ << "B";
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
    int32_t ret = StorageStatusService::GetInstance().GetBundleNameAndUid(userId, bundleNameAndUid);
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

// 获取上次Hap和Sa统计时的剩余空间大小
int64_t StorageDfxReporter::GetLastHapAndSaFreeSize()
{
    std::lock_guard<std::mutex> lock(hapAndSaStateMutex_);
    return lastHapAndSaFreeSize_;
}

// 获取上次Hap和Sa统计的时间点
std::chrono::system_clock::time_point StorageDfxReporter::GetLastHapAndSaTime()
{
    std::lock_guard<std::mutex> lock(hapAndSaStateMutex_);
    return lastHapAndSaTime_;
}

int32_t StorageDfxReporter::GetStorageStatsInfo(int32_t userId, StorageStats &storageStats)
{
    LOGI("GetStorageStatsInfo start, userId=%{public}d", userId);

    // 调用 storage_status_service 的 GetUserStorageStats 方法获取存储统计信息
    int32_t ret = StorageStatusService::GetInstance().GetUserStorageStats(userId, storageStats, true);
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
} // namespace StorageManager
} // namespace OHOS
