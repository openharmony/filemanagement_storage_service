/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "scan/storage_manager_scan.h"

#include <cmath>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "cJSON.h"

#include "storage_radar.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

using namespace OHOS::StorageService;

namespace OHOS {
namespace StorageManager {
constexpr const char* HYPERHOLD_PATH = "/data/service/el1/0/hyperhold";
constexpr const char* RGM_MANAGER_PATH = "/data/service/el1/public/rgm_manager";
constexpr const char* SCAN_RESULT_DIR = "/data/service/el1/public/storage_manager/database";
constexpr const char* SCAN_RESULT_FILE = "scan_result.json";

constexpr double DIVISOR = 1000.0 * 1000.0;
constexpr double BASE_NUMBER = 10.0;
constexpr int32_t ACCURACY_NUM = 2;
constexpr int32_t SCAN_TIMEOUT_MS = 30000;       // The scanning times out after 30 seconds.
constexpr int64_t TIME_INTERVAL_MS = 24 * 60 * 60 * 1000;  // Number of milliseconds in 24 hours

int64_t StorageManagerScan::GetRootSize()
{
    std::lock_guard<std::mutex> lock(calSizeMutex_);
    return StorageManagerScan::rootSize_;
}

int64_t StorageManagerScan::GetSystemSize()
{
    std::lock_guard<std::mutex> lock(calSizeMutex_);
    return StorageManagerScan::systemSize_;
}

int64_t StorageManagerScan::GetMemmgrSize()
{
    std::lock_guard<std::mutex> lock(calSizeMutex_);
    return StorageManagerScan::memmgrSize_;
}

int32_t StorageManagerScan::Init()
{
    LOGI("StorageManagerScan::Init start.");

    int32_t ret = LoadScanResultFromFile();
    if (ret == E_OK) {
        LOGI("StorageManagerScan::Init LoadScanResultFromFile success, root=%{public}lld,"
            " system=%{public}lld, memmgr=%{public}lld", static_cast<long long>(rootSize_),
            static_cast<long long>(systemSize_), static_cast<long long>(memmgrSize_));
        return E_OK;
    }

    LOGI("StorageManagerScan::Init LoadScanResultFromFile failed, ret=%{public}d, try quota query", ret);
    std::vector<int32_t> uidsList = {ROOT_UID, SYSTEM_UID, MEMMGR_UID};
    std::map<int32_t, int64_t> uidSizeMap;
    ret = GetQuotaSizeByUid(uidsList, uidSizeMap);
    if (ret != E_OK) {
        LOGE("StorageManagerScan::Init GetQuotaSizeByUid failed, ret=%{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(calSizeMutex_);
        rootSize_ = uidSizeMap[ROOT_UID];
        systemSize_ = uidSizeMap[SYSTEM_UID];
        memmgrSize_ = uidSizeMap[MEMMGR_UID];
    }
    ret = SaveScanResultToFile();
    if (ret != E_OK) {
        LOGE("StorageManagerScan::Init SaveScanResultToFile failed, ret=%{public}d", ret);
        return ret;
    }
    LOGI("StorageManagerScan::Init success, root=%{public}lld, system=%{public}lld, memmgr=%{public}lld",
        static_cast<long long>(rootSize_), static_cast<long long>(systemSize_),
        static_cast<long long>(memmgrSize_));
    return E_OK;
}

void StorageManagerScan::StartScan()
{
    LOGI("StorageManagerScan::StartScan called");
    std::lock_guard<std::mutex> lock(scanMutex_);
    if (isScanRunning_.load()) {
        LOGI("StorageManagerScan::StartScan scan is already running");
        return;
    }
    if (!CheckScanPreconditions()) {
        LOGI("StorageManagerScan::StartScan preconditions not met");
        return;
    }
    LaunchScanWorker();
}

void StorageManagerScan::StopScan()
{
    LOGI("StorageManagerScan::StopScan called");

    if (!isScanRunning_.load()) {
        return;
    }
    LOGI("StorageManagerScan::StopScan terminate the scanning thread");
    isScanRunning_.store(false);
    stopScanFlag_.store(true);
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("sdCommunication is nullptr");
        return;
    }
    int32_t ret = sdCommunication->SetStopScanFlag(true);
    if (ret != E_OK) {
        LOGE("Failed to reset stop scan flag, ret=%{public}d", ret);
    } else {
        LOGI("Successfully reset stop scan flag.");
    }
}

bool StorageManagerScan::CheckScanPreconditions()
{
    if (isFirstScan_) {
        LOGI("CheckScanPreconditions: first scan, allow to scan");
        return true;
    }

    auto currentTime = std::chrono::system_clock::now();
    auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime.time_since_epoch()).count();
    int64_t timeDiff = currentTimeMs - lastScanTime_;

    if (timeDiff >= TIME_INTERVAL_MS) {
        LOGI("CheckScanPreconditions: timeDiff=%{public}lldms >= 24h, allow to scan",
             static_cast<long long>(timeDiff));
        return true;
    }

    LOGI("CheckScanPreconditions: timeDiff=%{public}lldms < 24h, skip scan",
         static_cast<long long>(timeDiff));
    return false;
}

void StorageManagerScan::LaunchScanWorker()
{
    LOGI("LaunchScanWorker start");
    bool expected = false;
    if (!isScanRunning_.compare_exchange_strong(expected, true)) {
        LOGI("StorageManagerScan::LaunchScanWorker scan is already running");
        return;
    }
    stopScanFlag_.store(false);
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
        pthread_setname_np(pthread_self(), "storage_mgr_scan");
        LOGI("Scan worker thread started");
        int32_t ret = ExecuteScan();
        if (ret != E_OK) {
            LOGE("Scan worker ExecuteScan failed, ret=%{public}d", ret);
        }
        isScanRunning_.store(false);
        isFirstScan_ = false;
        LOGI("Scan worker thread completed");
    }).detach();

    std::thread([this]() {
        pthread_setname_np(pthread_self(), "scan_timeout_mon");
        ScanTimeoutMonitor();
    }).detach();

    LOGI("LaunchScanWorker end");
}

int32_t StorageManagerScan::ExecuteScan()
{
    LOGI("ExecuteScan start");
    scanStartTime_ = std::chrono::system_clock::now();
    auto startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        scanStartTime_.time_since_epoch()).count();

    std::map<int32_t, int64_t> uidSizeMap;
    int32_t ret = GetQuotaSizeByUid({MEMMGR_UID}, uidSizeMap);
    if (ret != E_OK) {
        LOGE("ExecuteScan GetMemmgrQuotaSize failed, ret=%{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(calSizeMutex_);
        memmgrSize_ = uidSizeMap[MEMMGR_UID];
    }
    std::vector<int32_t> scanUids = {ROOT_UID, SYSTEM_UID};
    std::vector<std::string> whiteList = GetDirWhiteList();
    int64_t dirScanRootSize = 0;
    int64_t dirScanSystemSize = 0;
    ret = ScanDirectories(whiteList, scanUids, dirScanRootSize, dirScanSystemSize);
    if (ret != E_OK) {
        LOGE("ExecuteScan ScanDirectories failed, ret=%{public}d", ret);
        return ret;
    }

    int64_t hyperholdRootSize = 0;
    ret = ScanSinglePath(HYPERHOLD_PATH, ROOT_UID, hyperholdRootSize);
    if (ret != E_OK) {
        LOGE("ExecuteScan ScanSinglePath hyperhold failed, ret=%{public}d", ret);
        hyperholdRootSize = 0;
    }

    int64_t rgmManagerRootSize = 0;
    ret = ScanSinglePath(RGM_MANAGER_PATH, ROOT_UID, rgmManagerRootSize);
    if (ret != E_OK) {
        LOGE("ExecuteScan ScanSinglePath rgm_manager failed, ret=%{public}d", ret);
        rgmManagerRootSize = 0;
    }
    CalculateFinalSizes(startTimeMs, dirScanRootSize, hyperholdRootSize, rgmManagerRootSize, dirScanSystemSize);
    ret = SaveScanResultToFile();
    if (ret != E_OK) {
        LOGE("ExecuteScan SaveScanResultToFile failed, ret=%{public}d", ret);
    }
    ReportScanResult();
    LOGI("ExecuteScan success, duration=%{public}lldms, root=%{public}lld, system=%{public}lld, memmgr=%{public}lld",
        static_cast<long long>(scanDurationMs_), static_cast<long long>(rootSize_),
        static_cast<long long>(systemSize_), static_cast<long long>(memmgrSize_));
    return E_OK;
}

void StorageManagerScan::ScanTimeoutMonitor()
{
    LOGI("ScanTimeoutMonitor start, will sleep %{public}dms", SCAN_TIMEOUT_MS);

    std::this_thread::sleep_for(std::chrono::milliseconds(SCAN_TIMEOUT_MS));
    if (isScanRunning_.load()) {
        LOGI("ScanTimeoutMonitor: scan still running after %{public}dms, stopping scan", SCAN_TIMEOUT_MS);
        StopScan();
    } else {
        LOGI("ScanTimeoutMonitor: scan already completed, no need to stop");
    }

    LOGI("ScanTimeoutMonitor end");
}

void StorageManagerScan::ReportScanResult()
{
    LOGI("ReportScanResult start");
    std::ostringstream extraData;
    GetCurrentTime(extraData);
    extraData << "{scanDurationMs:" << scanDurationMs_ << "}" << std::endl;
    extraData << "{rootSize:" << ConvertBytesToMB(rootSize_, ACCURACY_NUM) << "MB}" << std::endl;
    extraData << "{systemSize:" << ConvertBytesToMB(systemSize_, ACCURACY_NUM) << "MB}" << std::endl;
    extraData << "{memmgrSize:" << ConvertBytesToMB(memmgrSize_, ACCURACY_NUM) << "MB}" << std::endl;
    StorageService::StorageRadar::ReportSpaceRadar("StorageManagerScan", E_SCAN_RESULT, extraData.str());
    LOGI("ReportScanResult end");
}

void StorageManagerScan::CalculateFinalSizes(int64_t startTimeMs, int64_t dirScanRootSize, int64_t hyperholdRootSize,
    int64_t rgmManagerRootSize, int64_t dirScanSystemSize)
{
    std::lock_guard<std::mutex> lock(calSizeMutex_);
    scanEndTime_ = std::chrono::system_clock::now();
    auto endTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
               scanEndTime_.time_since_epoch()).count();
    scanDurationMs_ = endTimeMs - startTimeMs;
    lastScanTime_ = endTimeMs;

    rootSize_ = dirScanRootSize - hyperholdRootSize - rgmManagerRootSize;
    systemSize_ = dirScanSystemSize;
    memmgrSize_ = memmgrSize_ + hyperholdRootSize;
}

int32_t StorageManagerScan::GetQuotaSizeByUid(const std::vector<int32_t>& uids, std::map<int32_t, int64_t>& uidSizeMap)
{
    if (stopScanFlag_.load()) {
        LOGI("StorageManagerScan::GetQuotaSizeByUid ExecuteScan stopped by flag");
        return E_ERR;
    }
    LOGI("GetQuotaSizeByUid start, uids size=%{public}zu", uids.size());
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("GetQuotaSizeByUid GetInstance StorageDaemonCommunication failed");
        return E_SERVICE_IS_NULLPTR;
    }

    std::vector<NextDqBlk> dqBlks;
    int32_t ret = sdCommunication->GetDqBlkSpacesByUids(uids, dqBlks);
    if (ret != E_OK) {
        LOGE("GetQuotaSizeByUid GetDqBlkSpacesByUids failed, ret=%{public}d", ret);
        return ret;
    }

    for (const auto& dqBlk : dqBlks) {
        uidSizeMap[dqBlk.dqbId] = dqBlk.dqbCurSpace;
    }
    LOGI("GetQuotaSizeByUid end");
    return E_OK;
}

int32_t StorageManagerScan::ScanDirectories(const std::vector<std::string>& dirWhiteList,
    const std::vector<int32_t>& uids, int64_t& rootSize, int64_t& systemSize)
{
    if (stopScanFlag_.load()) {
        LOGI("StorageManagerScan::ScanDirectories ExecuteScan stop");
        return E_ERR;
    }
    LOGI("ScanDirectories start, whiteList size=%{public}zu, uids size=%{public}zu",
         dirWhiteList.size(), uids.size());

    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("ScanDirectories GetInstance StorageDaemonCommunication failed");
        return E_SERVICE_IS_NULLPTR;
    }

    std::vector<DirSpaceInfo> resultDirs;
    int32_t ret = sdCommunication->GetDirListSpaceByPaths(dirWhiteList, uids, resultDirs);
    if (ret != E_OK) {
        LOGE("ScanDirectories GetDirListSpaceByPaths failed, ret=%{public}d", ret);
        return ret;
    }

    for (const auto& dirInfo : resultDirs) {
        if (dirInfo.uid == static_cast<uint32_t>(ROOT_UID)) {
            rootSize += dirInfo.size;
        } else if (dirInfo.uid == static_cast<uint32_t>(SYSTEM_UID)) {
            systemSize += dirInfo.size;
        }
    }
    LOGI("ScanDirectories end, rootSize=%{public}lld, systemSize=%{public}lld",
         static_cast<long long>(rootSize), static_cast<long long>(systemSize));
    return E_OK;
}

int32_t StorageManagerScan::ScanSinglePath(const std::string& path, int32_t uid, int64_t& size)
{
    if (stopScanFlag_.load()) {
        LOGI("StorageManagerScan::ScanSinglePath ExecuteScan stopped by flag");
        return E_ERR;
    }
    LOGI("ScanSinglePath start, path=%{public}s, uid=%{public}d", path.c_str(), uid);
    size = 0;

    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("ScanSinglePath GetInstance StorageDaemonCommunication failed");
        return E_SERVICE_IS_NULLPTR;
    }

    std::vector<DirSpaceInfo> dirInfos = {{path, uid, 0}};
    std::vector<DirSpaceInfo> resultDirs;

    int32_t ret = sdCommunication->GetDirListSpace(dirInfos, resultDirs);
    if (ret != E_OK) {
        LOGE("ScanSinglePath GetDirListSpace failed, ret=%{public}d", ret);
        return ret;
    }

    if (!resultDirs.empty()) {
        size = resultDirs[0].size;
    }

    LOGI("ScanSinglePath end, path=%{public}s, size=%{public}lld",
         path.c_str(), static_cast<long long>(size));
    return E_OK;
}

void StorageManagerScan::GetCurrentTime(std::ostringstream& extraData)
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

double StorageManagerScan::ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces)
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

std::vector<std::string> StorageManagerScan::GetDirWhiteList()
{
    return {
        "/data/service/el0",
        "/data/service/el1",
        "/data/chipset/el1",
        "/data/log",
        "/data/local",
        "/data/vendor",
        "/data/hisi_logs"
    };
}

int32_t StorageManagerScan::CheckScanResultDirExists()
{
    if (access(SCAN_RESULT_DIR, F_OK) != 0) {
        LOGE("CheckScanResultDirExists: don't exists, path=%{public}s, errno is %{public}d", SCAN_RESULT_DIR, errno);
        return E_ERR;
    }
    LOGI("CheckScanResultDirExists: dir already exists, path=%{public}s", SCAN_RESULT_DIR);
    return E_OK;
}

int32_t StorageManagerScan::LoadScanResultFromFile()
{
    LOGI("LoadScanResultFromFile start");
    std::lock_guard<std::mutex> lock(fileMutex_);
    std::string filePath = std::string(SCAN_RESULT_DIR) + "/" + SCAN_RESULT_FILE;
    if (access(filePath.c_str(), F_OK) != 0) {
        LOGE("LoadScanResultFromFile: file not exist, path=%{public}s, errno is %{public}d", filePath.c_str(), errno);
        return E_ERR;
    }
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        LOGE("LoadScanResultFromFile: open file failed, path=%{public}s", filePath.c_str());
        return E_ERR;
    }
    std::string jsonString((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    cJSON* root = cJSON_Parse(jsonString.c_str());
    if (root == NULL) {
        LOGE("LoadScanResultFromFile: cJSON_Parse failed");
        return E_ERR;
    }
    cJSON* rootSizeItem = cJSON_GetObjectItem(root, "rootSize");
    cJSON* systemSizeItem = cJSON_GetObjectItem(root, "systemSize");
    cJSON* memmgrSizeItem = cJSON_GetObjectItem(root, "memmgrSize");
    if (rootSizeItem == NULL || systemSizeItem == NULL || memmgrSizeItem == NULL) {
        LOGE("LoadScanResultFromFile: missing required fields");
        cJSON_Delete(root);
        return E_ERR;
    }
    if (!cJSON_IsNumber(rootSizeItem) || !cJSON_IsNumber(systemSizeItem) ||
        !cJSON_IsNumber(memmgrSizeItem)) {
        LOGE("LoadScanResultFromFile: invalid data type");
        cJSON_Delete(root);
        return E_ERR;
    }
    {
        std::lock_guard<std::mutex> lock(calSizeMutex_);
        rootSize_ = static_cast<int64_t>(rootSizeItem->valuedouble);
        systemSize_ = static_cast<int64_t>(systemSizeItem->valuedouble);
        memmgrSize_ = static_cast<int64_t>(memmgrSizeItem->valuedouble);
    }
    cJSON_Delete(root);
    LOGI("LoadScanResultFromFile success, root=%{public}lld, system=%{public}lld, memmgr=%{public}lld",
        static_cast<long long>(rootSize_), static_cast<long long>(systemSize_),
        static_cast<long long>(memmgrSize_));
    return E_OK;
}

int32_t StorageManagerScan::SaveScanResultToFile()
{
    LOGI("SaveScanResultToFile start");
    std::lock_guard<std::mutex> lock(fileMutex_);
    int32_t ret = CheckScanResultDirExists();
    if (ret != E_OK) {
        LOGE("SaveScanResultToFile: CheckScanResultDirExists failed, ret=%{public}d", ret);
        return ret;
    }
    cJSON* root = cJSON_CreateObject();
    if (root == NULL) {
        LOGE("SaveScanResultToFile: cJSON_CreateObject failed");
        return E_ERR;
    }
    {
        std::lock_guard<std::mutex> lock(calSizeMutex_);
        cJSON_AddNumberToObject(root, "rootSize", static_cast<double>(rootSize_));
        cJSON_AddNumberToObject(root, "systemSize", static_cast<double>(systemSize_));
        cJSON_AddNumberToObject(root, "memmgrSize", static_cast<double>(memmgrSize_));
    }
    char* jsonChar = cJSON_Print(root);
    cJSON_Delete(root);
    if (jsonChar == NULL) {
        LOGE("SaveScanResultToFile: cJSON_Print failed");
        return E_ERR;
    }
    std::string jsonString(jsonChar);
    cJSON_free(jsonChar);
    std::string filePath = std::string(SCAN_RESULT_DIR) + "/" + SCAN_RESULT_FILE;
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        LOGE("SaveScanResultToFile: open file failed, path=%{public}s", filePath.c_str());
        return E_ERR;
    }
    outFile << jsonString;
    if (outFile.fail()) {
        LOGE("Failed to write to file, path=%{public}s", filePath.c_str());
    }
    outFile.close();
    LOGI("SaveScanResultToFile success, path=%{public}s, root=%{public}lld, system=%{public}lld, memmgr=%{public}lld",
        filePath.c_str(), static_cast<long long>(rootSize_), static_cast<long long>(systemSize_),
        static_cast<long long>(memmgrSize_));

    return E_OK;
}

} // namespace StorageManager
} // namespace OHOS
