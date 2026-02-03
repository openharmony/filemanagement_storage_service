/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include <atomic>
#include <cinttypes>
#include <singleton.h>
#include <thread>
#include "event_handler.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageManager {
const int SMART_EVENT_INTERVAL = 24; // 24h
const int SMART_EVENT_INTERVAL_HIGH_FREQ = 5; // 5m
const int32_t CLEAN_LOW_LEVEL_TIME = 300; // 5m
const int32_t CLEAN_LOW_TIME_ONE_HOUR = 3600; // 1h

struct SizeInfo {
    int64_t freeSize;
    int64_t totalSize;
    int64_t freeInode;
    int64_t totalInode;
};

enum class CleanType : int8_t {
    CACHE_SPACE = 0,
    CACHE_INODE = 1,
};

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
    void CleanBundleCache(int64_t lowThreshold, int64_t lowInodeThreshold, bool isCleanSpace,
                          const std::string &cleanLevel);
    void CheckAndCleanCache(int64_t freeSize, int64_t totalSize, int64_t freeInode, int64_t totalInode);
    void CheckAndEventNotify(int64_t freeSize, int64_t totalSize, int64_t freeInode, int64_t totalInode);
    void SendSmartNotificationEvent(const std::string &faultDesc, const std::string &faultSuggest, bool isHighFreq);
    void ReportRadarStorageUsage(enum StorageService::BizStage stage, const std::string &extraData);
    void RefreshAllNotificationTimeStamp();
    void EventNotifyFreqHandlerForLow(bool isCleanSpace, int64_t freeInode, int64_t totalInode);
    void EventNotifyFreqHandlerForMedium(bool isCleanSpace, int64_t freeInode, int64_t totalInode);
    void EventNotifyFreqHandlerForHigh(bool isCleanSpace, int64_t freeInode, int64_t totalInode);
    void ParseStorageParameters(int64_t totalSize, int64_t totalInode);
    void ParseStorageSizeParameters(int64_t totalSize, const std::string storageParams);
    void ParseStorageInodeParameters(int64_t totalInode, const std::string storageInodeParams);
    void UpdateBaseLineByUid();
    std::string GetStorageAlertCleanupParams();
    std::string GetStorageAlertInodeCleanupParams();
    std::string GetJsonString(const std::string &faultDesc, const std::string &faultSuggest, bool isHighFreq);
    std::string GetJsonStringForInode(const std::string &faultDesc, const std::string &faultSuggest,
                                      int64_t freeInode, int64_t totalInode);

    // stats
    void HapAndSaStatisticsThd();

    void PublishCleanCacheEvent(const std::string &cleanLevel, bool isCleanSpace, struct SizeInfo &sizeInfo);
    int32_t SendCommonEventToCleanCache(const std::string &cleanLevel, struct SizeInfo &sizeInfo, bool isCleanSpace);
    void SendSmartNotificationInodeEvent(const std::string &faultDesc, const std::string &faultSuggest,
                                         int64_t freeInode, int64_t totalInode);
    void HandleEventAndClean(const std::string &cleanLevel, struct SizeInfo &sizeInfo);
    bool isNumberString(const std::string &str);
    int32_t GetLastNotifyTime(const std::string &cleanLevel, int64_t &lastNotifyTime);
    int32_t SetLastNotifyTime(const std::string &cleanLevel, int64_t curTime);
    std::mutex notifyCleanMtx_;

    bool hasNotifiedStorageEvent_ = true;
    std::atomic<int32_t> cleanLowLevelTime_{CLEAN_LOW_LEVEL_TIME};
    std::mutex eventMutex_;
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::map<std::string, int64_t> thresholds;
    std::map<std::string, int64_t> inodeThresholds_;
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