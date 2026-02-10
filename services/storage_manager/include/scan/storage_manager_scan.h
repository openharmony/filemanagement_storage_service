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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MANAGER_SCAN_H
#define OHOS_STORAGE_MANAGER_STORAGE_MANAGER_SCAN_H

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "storage_daemon_communication/storage_daemon_communication.h"
#include "statistic_info.h"

namespace OHOS {
namespace StorageManager {

class StorageManagerScan {
public:
    static StorageManagerScan& GetInstance()
    {
        static StorageManagerScan instance;
        return instance;
    }
    int32_t Init();
    void StartScan();
    void StopScan();
    int64_t GetRootSize();
    int64_t GetSystemSize();
    int64_t GetMemmgrSize();

private:
    StorageManagerScan() = default;
    ~StorageManagerScan() = default;
    StorageManagerScan(const StorageManagerScan&) = delete;
    StorageManagerScan& operator=(const StorageManagerScan&) = delete;

    bool CheckScanPreconditions();
    void LaunchScanWorker();
    int32_t ExecuteScan();
    void ScanTimeoutMonitor();
    void ReportScanResult();
    void CalculateFinalSizes(int64_t startTimeMs, int64_t dirScanRootSize, int64_t hyperholdRootSize,
    int64_t rgmManagerRootSize, int64_t dirScanSystemSize);
    int32_t GetQuotaSizeByUid(const std::vector<int32_t>& uids, std::map<int32_t, int64_t>& uidSizeMap);
    int32_t ScanDirectories(const std::vector<std::string>& dirWhiteList, const std::vector<int32_t>& uids,
        int64_t& rootSize, int64_t& systemSize);
    int32_t ScanSinglePath(const std::string& path, int32_t uid, int64_t& size);
    void GetCurrentTime(std::ostringstream& extraData);
    double ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces);
    std::vector<std::string> GetDirWhiteList();
    int32_t LoadScanResultFromFile();
    int32_t SaveScanResultToFile();
    int32_t CheckScanResultDirExists();

    std::atomic<bool> isScanRunning_{false};       // Scan running flag
    std::atomic<bool> stopScanFlag_{false};        // Scanning stop flag
    std::mutex scanMutex_;                          // Scanning for mutexes
    std::mutex fileMutex_;                          // File operation mutex lock
    std::mutex calSizeMutex_;

    std::chrono::system_clock::time_point scanStartTime_;  // Scan start time
    std::chrono::system_clock::time_point scanEndTime_;    // Scan End Time

    bool isFirstScan_{true};    // First scan flag
    int64_t rootSize_ = 0;           // Space occupied by the root user (byte)
    int64_t systemSize_ = 0;         // Space occupied by the system user (byte)
    int64_t memmgrSize_ = 0;         // Size of the space occupied by the memmgr user (byte)
    int64_t scanDurationMs_ = 0;     // Scanning duration (ms)
    int64_t lastScanTime_ = 0;       // Last scan timestamp (ms)
};

} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MANAGER_SCAN_H
