/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MONITOR_SERVICE_H
#define OHOS_STORAGE_MANAGER_STORAGE_MONITOR_SERVICE_H

#include <singleton.h>
#include <thread>
#include "event_handler.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageManager {
const int SMART_EVENT_INTERVAL = 24; // 24h
const int SMART_EVENT_INTERVAL_HIGH_FREQ = 5; // 5m
class StorageMonitorService : public NoCopyable  {
public:
    static StorageMonitorService &GetInstance()
    {
        static StorageMonitorService instance;
        return instance;
    }
    void StartStorageMonitorTask();

private:
    StorageMonitorService();
    ~StorageMonitorService();
    void StartEventHandler();
    void Execute();
    void MonitorAndManageStorage();
    void CleanBundleCache(int64_t lowThreshold);
    void CheckAndCleanCache(int64_t freeSize, int64_t totalSize);
    void CheckAndEventNotify(int64_t freeSize, int64_t totalSize);
    void SendSmartNotificationEvent(const std::string &faultDesc, const std::string &faultSuggest, bool isHighFreq);
    void CleanBundleCacheByInterval(const std::string &timestamp,
                                                           int64_t lowThreshold, int32_t checkInterval);
    void ReportRadarStorageUsage(enum StorageService::BizStage stage, const std::string &extraData);
    void RefreshAllNotificationTimeStamp();
    void EventNotifyFreqHandlerForLow();
    void EventNotifyFreqHandlerForMedium();
    void EventNotifyFreqHandlerForHigh();
    void ParseMemoryParameters(int64_t totalSize);
    std::string GetMemoryAlertCleanupParams();

    bool hasNotifiedStorageEvent_ = true;
    std::mutex eventMutex_;
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::map<std::string, int64_t> thresholds;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    std::chrono::system_clock::time_point lastNotificationTime_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::hours(SMART_EVENT_INTERVAL);
    std::chrono::system_clock::time_point lastNotificationTimeHighFreq_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::minutes(SMART_EVENT_INTERVAL_HIGH_FREQ);
    std::chrono::system_clock::time_point lastReportRadarTime_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::hours(SMART_EVENT_INTERVAL);
    std::chrono::system_clock::time_point lastNotificationTimeMedium_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::hours(SMART_EVENT_INTERVAL);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MONITOR_SERVICE_H