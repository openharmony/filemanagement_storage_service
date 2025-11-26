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
constexpr int32_t TOP_COUNT = 20; // Top N directories to report

// HMFS metadata constants
constexpr int32_t ROOT_UID = 0;
constexpr int32_t SYSTEM_UID = 1000;
constexpr int32_t FOUNDATION_UID = 5523;
static std::vector<int32_t> SYS_UIDS = {0, 1000, 5523};

int32_t StorageDfxReporter::StartReportDirStatus()
{
    LOGI("StorageDfxReporter StartReportDirStatus start.");

    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("Get StorageDaemonCommunication instance failed.");
        return E_ERR;
    }
    // 获取系统UID配额信息
    std::vector<NextDqBlk> dqBlks;
    int32_t ret = sdCommunication->GetDqBlkSpacesByUids(SYS_UIDS, dqBlks);
    if (ret != E_OK) {
        LOGE("GetDqBlkSpacesByUids failed, ret=%{public}d.", ret);
        return ret;
    }
    // 检查系统UID大小
    int64_t totalSize = 0;
    int64_t rootSize = 0;
    int64_t systemSize = 0;
    int64_t foundationSize = 0;
    ret = CheckSystemUidSize(dqBlks, totalSize, rootSize, systemSize, foundationSize);
    if (ret != E_OK) {
        return ret;
    }
    // 准备打点数据
    std::ostringstream extraData;
    GetCurrentTime(extraData);
    // 收集目录统计信息
    CollectDirStatistics(rootSize, systemSize, foundationSize, extraData);
    // 上报打点数据
    StorageService::StorageRadar::ReportSpaceRadar("StartReportDirStatus",
        E_SYS_DIR_SPACE_STATUS, extraData.str());
    LOGI("StorageDfxReporter StartReportDirStatus end.");
    // 更新扫描状态
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
        int32_t uid = static_cast<int32_t>(dqBlks[i].uid);
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
    int64_t sizeIncrease = totalSize - lastTotalSize_;
    if (lastTotalSize_ > 0 && sizeIncrease < StorageService::ONE_G_BYTE) {
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
    // 获取 root 目录详细信息
    std::vector<DirSpaceInfo> rootDirs = GetRootDirList();
    std::vector<DirSpaceInfo> rootOutDirs;
    int32_t ret = sdCommunicationInstance->GetDirListSpace(rootDirs, rootOutDirs);
    if (ret == E_OK) {
        extraData << "{root size:" << ConvertBytesToMB(rootSize, ACCURACY_NUM) << "MB}" << std::endl;
        extraData << "{root directories:}" << std::endl;
        AppendDirInfo(rootOutDirs, extraData);
    }
    // 获取 system 目录详细信息
    std::vector<DirSpaceInfo> systemDirs = GetSystemDirList();
    std::vector<DirSpaceInfo> systemOutDirs;
    ret = sdCommunicationInstance->GetDirListSpace(systemDirs, systemOutDirs);
    if (ret == E_OK) {
        extraData << "{system size:" << ConvertBytesToMB(systemSize, ACCURACY_NUM) << "MB}" << std::endl;
        extraData << "{system directories:}" << std::endl;
        AppendDirInfo(systemOutDirs, extraData);
    }
    // 获取 foundation 目录详细信息
    std::vector<DirSpaceInfo> foundationDirs = GetFoundationDirList();
    std::vector<DirSpaceInfo> foundationOutDirs;
    ret = sdCommunicationInstance->GetDirListSpace(foundationDirs, foundationOutDirs);
    if (ret == E_OK) {
        extraData << "{foundation size:" << ConvertBytesToMB(foundationSize, ACCURACY_NUM) << "MB}" << std::endl;
        extraData << "{foundation directories:}" << std::endl;
        AppendDirInfo(foundationOutDirs, extraData);
    }
    // 获取 Anco 大小信息
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

int64_t StorageDfxReporter::GetLastScanFreeSize()
{
    std::lock_guard<std::mutex> lock(scanStateMutex_);
    return lastScanFreeSize_;
}

std::chrono::system_clock::time_point StorageDfxReporter::GetLastScanTime()
{
    std::lock_guard<std::mutex> lock(scanStateMutex_);
    return lastScanTime_;
}
} // namespace StorageManager
} // namespace OHOS
