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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_DFX_REPORTER_H
#define OHOS_STORAGE_MANAGER_STORAGE_DFX_REPORTER_H

#include <vector>
#include <sstream>
#include <chrono>
#include <mutex>
#include <map>
#include <thread>
#include <atomic>
#include "statistic_info.h"

namespace OHOS {
namespace StorageManager {
class StorageDfxReporter {
public:
    static StorageDfxReporter& GetInstance()
    {
        static StorageDfxReporter instance;
        return instance;
    }
    int32_t StartReportDirStatus();
    // Scan control methods
    void StartScan();
    void StopScan();

private:
    StorageDfxReporter() = default;
    ~StorageDfxReporter();
    StorageDfxReporter(const StorageDfxReporter&) = delete;
    StorageDfxReporter& operator=(const StorageDfxReporter&) = delete;

    void GetCurrentTime(std::ostringstream &extraData);
    int32_t CheckSystemUidSize(const std::vector<NextDqBlk> &dqBlks, int64_t &totalSize,
                               int64_t &rootSize, int64_t &systemSize, int64_t &foundationSize);
    void CollectDirStatistics(int64_t rootSize, int64_t systemSize, int64_t foundationSize,
                              std::ostringstream &extraData);
    int32_t UpdateScanState(int64_t totalSize);
    double ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces);
    void AppendDirInfo(const std::vector<DirSpaceInfo> &dirs, std::ostringstream &extraData);
    std::vector<DirSpaceInfo> GetRootDirList();
    std::vector<DirSpaceInfo> GetSystemDirList();
    std::vector<DirSpaceInfo> GetFoundationDirList();

    // Scan control helper methods
    bool CheckScanPreconditions();
    void LaunchScanWorker();
    std::mutex scanStateMutex_;
    int64_t lastScanFreeSize_ = 0;
    int64_t lastTotalSize_ = 0;
    std::chrono::system_clock::time_point lastScanTime_;

    // Scan control variables
    std::atomic<bool> isScanRunning_{false};
    std::mutex scanMutex_;
    std::thread scanThread_;
};
} // namespace StorageManager
} // namespace OHOS
#endif // OHOS_STORAGE_MANAGER_STORAGE_DFX_REPORTER_H
