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
#include "storage_stats.h"

namespace OHOS {
namespace StorageManager {
class StorageDfxReporter {
public:
    static StorageDfxReporter& GetInstance()
    {
        static StorageDfxReporter instance;
        return instance;
    }

    void StartReportHapAndSaStorageStatus();

    // 获取上次Hap和Sa查询的时间和空间大小(线程安全)
    int64_t GetLastHapAndSaFreeSize();
    std::chrono::system_clock::time_point GetLastHapAndSaTime();

private:
    StorageDfxReporter() = default;
    ~StorageDfxReporter() = default;
    StorageDfxReporter(const StorageDfxReporter&) = delete;
    StorageDfxReporter& operator=(const StorageDfxReporter&) = delete;

    void GetCurrentTime(std::ostringstream &extraData);
    void ExecuteHapAndSaStatistics(int32_t userId);
    int32_t CollectStorageStats(int32_t userId, std::ostringstream &extraData);
    void CollectMetadataAndAnco(std::ostringstream &extraData);
    int32_t CollectBundleStatistics(int32_t userId, std::ostringstream &extraData);
    void UpdateHapAndSaState();
    int32_t UpdateScanState(int64_t totalSize);
    double ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces);

    int32_t GetStorageStatsInfo(int32_t userId, StorageStats &storageStats);
    void GetMetaDataSize(std::ostringstream &extraData);
    void GetAncoDataSize(std::ostringstream &extraData);

    // Hap和Sa统计状态变量(需要线程安全保护)
    std::mutex hapAndSaStateMutex_;
    std::thread hapAndSaThread_;
    std::thread scanThread_;
    int64_t lastHapAndSaFreeSize_ = 0;
    std::chrono::system_clock::time_point lastHapAndSaTime_;
    std::atomic<bool> isHapAndSaRunning_{false};
};
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_DFX_REPORTER_H
