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

#include "storage/storage_monitor_service.h"

#include <string>
#include <sstream>

#include "cJSON.h"
#include "common_event_manager.h"
#include "init_param.h"
#include "parameter.h"
#include "parameters.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_total_status_service.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_constant.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
constexpr int32_t ONE_MB = 1024 * 1024;
constexpr int32_t ONE_GB = 1024 * 1024 * 1024;
constexpr int32_t CONST_NUM_TWO = 2;
constexpr int32_t CONST_NUM_ONE_HUNDRED = 100;
constexpr int32_t CLEAN_CACHE_WEEK = 7 * 24; // week
constexpr int32_t WAIT_THREAD_TIMEOUT_MS = 5;
constexpr int32_t DEFAULT_CHECK_INTERVAL = 60 * 1000; // 60s
constexpr int32_t SEND_EVENT_INTERVAL = 24; // day
constexpr int32_t SEND_EVENT_INTERVAL_HIGH_FREQ = 5; // 5m
constexpr int32_t STORAGE_PARAMS_PATH_LEN = 128;
constexpr const char *STORAGE_ALERT_CLEANUP_PARAMETER = "const.storage_service.storage_alert_policy";
constexpr const char *DEFAULT_PARAMS = "notify_l:500M/notify_m:2G/notify_h:10%/clean_l:750M/clean_m:5%/clean_h:10%";
const std::string PUBLISH_SYSTEM_COMMON_EVENT = "ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT";
const std::string SMART_ACTION = "hicare.event.SMART_NOTIFICATION";
const std::string TIMESTAMP_DAY = "persist.storage_manager.timestamp.day";
const std::string TIMESTAMP_WEEK = "persist.storage_manager.timestamp.week";
const std::string FAULT_ID_ONE = "845010021";
const std::string FAULT_ID_TWO = "845010022";
const std::string FAULT_ID_THREE = "845010023";
const std::string FAULT_SUGGEST_THREE = "545010023";
constexpr int RETRY_MAX_TIMES = 3;

StorageMonitorService::StorageMonitorService() {}

StorageMonitorService::~StorageMonitorService()
{
    LOGI("StorageMonitorService Destructor.");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if ((eventHandler_ != nullptr) && (eventHandler_->GetEventRunner() != nullptr)) {
        eventHandler_->RemoveAllEvents();
        eventHandler_->GetEventRunner()->Stop();
    }
    if (eventThread_.joinable()) {
        eventThread_.join();
    }
    eventHandler_ = nullptr;
}

void StorageMonitorService::StartStorageMonitorTask()
{
    LOGI("StorageMonitorService, start deicve storage monitor task.");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if (eventHandler_ == nullptr) {
        eventThread_ = std::thread(&StorageMonitorService::StartEventHandler, this);
        eventCon_.wait_for(lock, std::chrono::seconds(WAIT_THREAD_TIMEOUT_MS), [this] {
            return eventHandler_ != nullptr;
        });
    }

    auto executeFunc = [this] { Execute(); };
    if (eventHandler_ == nullptr) {
        LOGE("event handler is nullptr in StartStorageMonitorTask.");
    }
    eventHandler_->PostTask(executeFunc, DEFAULT_CHECK_INTERVAL);
}

void StorageMonitorService::StartEventHandler()
{
    pthread_setname_np(pthread_self(), "storage_monitor_task_event");
    auto runner = AppExecFwk::EventRunner::Create(false);
    if (runner == nullptr) {
        LOGE("event runner is nullptr.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
}

void StorageMonitorService::Execute()
{
    if (eventHandler_ == nullptr) {
        LOGE("event handler is nullptr.");
        return;
    }
    MonitorAndManageStorage();
    auto executeFunc = [this] { Execute(); };
    eventHandler_->PostTask(executeFunc, DEFAULT_CHECK_INTERVAL);
}

void StorageMonitorService::MonitorAndManageStorage()
{
    int64_t totalSize;
    int32_t err = StorageTotalStatusService::GetInstance().GetTotalSize(totalSize);
    if ((err != E_OK) || (totalSize <= 0)) {
        LOGE("Get device total size failed.");
        return;
    }

    int64_t freeSize;
    err = StorageTotalStatusService::GetInstance().GetFreeSize(freeSize);
    if ((err != E_OK) || (freeSize < 0)) {
        LOGE("Get device free size failed.");
        return;
    }
    ParseStorageParameters(totalSize);

    LOGI("clean_l, size=%{public}lld, clean_m, size=%{public}lld, clean_h, size=%{public}lld",
         static_cast<long long>(thresholds["clean_l"]), static_cast<long long>(thresholds["clean_m"]),
         static_cast<long long>(thresholds["clean_h"]));
    if (freeSize < thresholds["clean_h"]) {
        CheckAndCleanCache(freeSize, totalSize);
    }
 
    LOGI("notify_l, size=%{public}lld, notify_m, size=%{public}lld, notify_h, size=%{public}lld",
        static_cast<long long>(thresholds["notify_l"]), static_cast<long long>(thresholds["notify_m"]),
        static_cast<long long>(thresholds["notify_h"]));
    if (freeSize < thresholds["notify_h"]) {
        CheckAndEventNotify(freeSize, totalSize);
        hasNotifiedStorageEvent_ = true;
    } else if (hasNotifiedStorageEvent_) {
        RefreshAllNotificationTimeStamp();
        hasNotifiedStorageEvent_ = false;
    }
    if (!IsCurTimeNeedStatistic()) {
        return;
    }
    StatisticSysDirSpace(freeSize);
}

void StorageMonitorService::StatisticSysDirSpace(int64_t &freeSize)
{
    if (freesizeCache > 0 && std::abs(freeSize - freesizeCache) < StorageService::TWO_G_BYTE) {
        return;
    }
    freesizeCache = freeSize;
    LOGI("storage monitor statistic start.");
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t ret = sdCommunication->StatisticSysDirSpace();
    LOGI("storage monitor statistic end, ret is %{public}d.", ret);
}

bool StorageMonitorService::IsCurTimeNeedStatistic()
{
    std::time_t now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);
    return localTime->tm_hour == 1 && (localTime->tm_min == 0 || localTime->tm_min == 1);
}

std::string StorageMonitorService::GetStorageAlertCleanupParams()
{
    std::string storageParams;
    char tmpBuffer[STORAGE_PARAMS_PATH_LEN] = {0};
    
    int ret = GetParameter(STORAGE_ALERT_CLEANUP_PARAMETER, DEFAULT_PARAMS, tmpBuffer, STORAGE_PARAMS_PATH_LEN);
    if (ret <= 0) {
        LOGE("GetParameter name = %{public}s error, ret = %{public}d, return default value",
             STORAGE_ALERT_CLEANUP_PARAMETER, ret);
        return DEFAULT_PARAMS;
    }
    
    return tmpBuffer;
}
 
void StorageMonitorService::ParseStorageParameters(int64_t totalSize)
{
    std::string storageParams = GetStorageAlertCleanupParams();
    std::istringstream storageParamsStream(storageParams);
    std::string item;
    while (std::getline(storageParamsStream, item, '/')) {
        std::string key;
        std::string value;
 
        size_t pos = item.find(':');
        if (pos != std::string::npos) {
            key = item.substr(0, pos);
            value = item.substr(pos + 1);
        } else {
            LOGE("Invalid parameter format");
            continue;
        }
 
        if (value.length() > 1 && value.substr(value.length() - 1) == "%") {
            int percentage = std::atoi(value.substr(0, value.length() - 1).c_str());
            if (percentage < 0 || percentage > CONST_NUM_ONE_HUNDRED) {
                LOGE("Invalid percentage value");
            } else {
                int64_t size = (totalSize * percentage) / CONST_NUM_ONE_HUNDRED;
                thresholds[key] = static_cast<int64_t>(size);
            }
        } else if (value.length() > 1 && value.substr(value.length() - 1) == "G") {
            int gigabytes = std::atoi(value.substr(0, value.length() - 1).c_str());
            if (gigabytes < 0) {
                LOGE("Storage value cannot be negative: %{public}d G", gigabytes);
            } else {
                int64_t size = static_cast<int64_t>(gigabytes) * ONE_GB;
                thresholds[key] = static_cast<int64_t>(size);
            }
        } else if (value.length() > 1 && value.substr(value.length() - 1) == "M") {
            int megabytes = std::atoi(value.substr(0, value.length() - 1).c_str());
            if (megabytes < 0) {
                LOGE("Storage value cannot be negative: %{public}d M", megabytes);
            } else {
                int64_t size = static_cast<int64_t>(megabytes) * ONE_MB;
                thresholds[key] = static_cast<int64_t>(size);
            }
        } else {
            LOGE("parameter value format is invalid");
        }
    }
}

void StorageMonitorService::CheckAndCleanCache(int64_t freeSize, int64_t totalSize)
{
    int64_t lowThreshold = thresholds["clean_l"];
    if (lowThreshold <= 0) {
        LOGE("Lower threshold value is invalid.");
        return;
    }

    LOGI("Device storage freeSize=%{public}lld, threshold=%{public}lld", static_cast<long long>(freeSize),
         static_cast<long long>(lowThreshold));

    std::string freeSizeStr = std::to_string(freeSize);
    std::string totalSizeStr = std::to_string(totalSize);
    std::string lowThresholdStr = std::to_string(lowThreshold);
    std::string storageUsage = "storage usage not enough:freeSize = " + freeSizeStr + ", totalSize = " + totalSizeStr +
                               ", lowThreshold = " + lowThresholdStr;
    if (freeSize < lowThreshold) {
        CleanBundleCache(lowThreshold);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_LOW, storageUsage);
        LOGI("Device running out of storage");
        return;
    }

    if (freeSize > thresholds["clean_m"]) {
        CleanBundleCacheByInterval(TIMESTAMP_WEEK, lowThreshold, CLEAN_CACHE_WEEK);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_HIGH, storageUsage);
    } else {
        CleanBundleCacheByInterval(TIMESTAMP_DAY, lowThreshold, SEND_EVENT_INTERVAL);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_MEDIUM, storageUsage);
    }
}

void StorageMonitorService::CleanBundleCacheByInterval(const std::string &timestamp,
                                                       int64_t lowThreshold, int32_t checkInterval)
{
    auto currentTime = std::chrono::system_clock::now();
    auto curTimePoint =
            std::chrono::time_point_cast<std::chrono::hours>(currentTime).time_since_epoch().count();
    std::string param = system::GetParameter(timestamp, "");
    if (param.empty()) {
        LOGI("Not found timestamp from system parameter");
        return;
    }
    uint64_t lastCleanCacheTime = static_cast<uint64_t>(std::atoll(param.c_str()));
    auto duration = std::chrono::duration_cast<std::chrono::hours>(currentTime -
            std::chrono::system_clock::time_point(std::chrono::hours(lastCleanCacheTime))).count();
    LOGI("CleanBundleCache timestamp is %{public}s, duration is %{public}ld", timestamp.c_str(), duration);
    if (duration >= checkInterval) {
        CleanBundleCache(lowThreshold);
        system::SetParameter(timestamp, std::to_string(curTimePoint));
    }
}

void StorageMonitorService::ReportRadarStorageUsage(enum StorageService::BizStage stage, const std::string &extraData)
{
    LOGI("storage monitor service report radar storage usage start");
    StorageService::StorageRadar::ReportStorageUsage(stage, extraData);
}

void StorageMonitorService::CleanBundleCache(int64_t lowThreshold)
{
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return;
    }
    LOGI("Device storage free size not enough, start clean bundle cache files automatic.");
    int32_t ret = E_OK;
    int retryCount = 0;
    do {
        ret = bundleMgr->CleanBundleCacheFilesAutomatic(lowThreshold * CONST_NUM_TWO);
        if (ret == ERR_OK) {
            LOGI("Invoke bundleMgr interface to clean bundle cache files automatic success.");
            return;
        }
        retryCount ++;
        LOGE("Invoke bundleMgr interface to clean bundle cache files automatic failed. Retry.");
    } while (retryCount < RETRY_MAX_TIMES);
    StorageRadar::ReportBundleMgrResult("CleanBundleCacheFilesAutomatic", ret, DEFAULT_USERID, "");
}

void StorageMonitorService::CheckAndEventNotify(int64_t freeSize, int64_t totalSize)
{
    LOGI("StorageMonitorService, start CheckAndEventNotify.");
    std::string freeSizeStr = std::to_string(freeSize);
    std::string totalSizeStr = std::to_string(totalSize);
    std::string storageUsage = "storage usage not enough event notify freeSize = " + freeSizeStr + ", totalSize = " +
                               totalSizeStr;
    if (freeSize < thresholds["notify_l"]) {
        EventNotifyFreqHandlerForLow();
        storageUsage += ", freeSize < " + std::to_string(thresholds["notify_l"]) + ", success notify event";
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_LOW, storageUsage);
        return;
    }
    if (freeSize < thresholds["notify_m"]) {
        EventNotifyFreqHandlerForMedium();
        storageUsage += ", freeSize < " + std::to_string(thresholds["notify_m"]) + ", success notify event";
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_MEDIUM, storageUsage);
        return;
    }
    EventNotifyFreqHandlerForHigh();
}

void StorageMonitorService::SendSmartNotificationEvent(const std::string &faultDesc,
                                                       const std::string &faultSuggest,
                                                       bool isHighFreq)
{
    LOGI("StorageMonitorService, start SendSmartNotificationEvent.");
    EventFwk::CommonEventPublishInfo publishInfo;
    const std::string permission = PUBLISH_SYSTEM_COMMON_EVENT;
    std::vector<std::string> permissions;
    permissions.emplace_back(permission);
    publishInfo.SetSubscriberPermissions(permissions);
    publishInfo.SetOrdered(false);
    publishInfo.SetSticky(false);

    AAFwk::Want want;
    want.SetAction(SMART_ACTION);
    EventFwk::CommonEventData eventData;
    eventData.SetWant(want);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "faultDescription", faultDesc.c_str());
    cJSON_AddStringToObject(root, "faultSuggestion", faultSuggest.c_str());
    if (isHighFreq) {
        cJSON *faultSuggestionParam = cJSON_CreateString("500M");
        cJSON *faultSuggestionArray = cJSON_CreateArray();
        cJSON_AddItemToArray(faultSuggestionArray, faultSuggestionParam);
        cJSON_AddItemToObject(root, "faultSuggestionParams", faultSuggestionArray);
    }
    char *json_string = cJSON_Print(root);
    std::string eventDataStr(json_string);
    eventDataStr.erase(remove(eventDataStr.begin(), eventDataStr.end(), '\n'), eventDataStr.end());
    eventDataStr.erase(remove(eventDataStr.begin(), eventDataStr.end(), '\t'), eventDataStr.end());

    LOGI("send message is %{public}s", eventDataStr.c_str());
    eventData.SetData(eventDataStr);
    free(json_string);
    cJSON_Delete(root);
    EventFwk::CommonEventManager::PublishCommonEvent(eventData, publishInfo, nullptr);
}

void StorageMonitorService::EventNotifyFreqHandlerForLow()
{
    auto currentTime = std::chrono::system_clock::now();
    int32_t duration = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::minutes>
            (currentTime - lastNotificationTimeHighFreq_).count());
    LOGW("StorageMonitorService left Storage Size < Low, duration is %{public}d", duration);
    if (duration >= SEND_EVENT_INTERVAL_HIGH_FREQ) {
        lastNotificationTimeHighFreq_ = currentTime;
        lastNotificationTime_ =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        std::chrono::system_clock::now()) - std::chrono::hours(SEND_EVENT_INTERVAL);
        lastNotificationTimeMedium_ =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        std::chrono::system_clock::now()) - std::chrono::hours(SEND_EVENT_INTERVAL);
    }
}

void StorageMonitorService::EventNotifyFreqHandlerForMedium()
{
    auto currentTime = std::chrono::system_clock::now();
    int32_t duration = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::hours>
            (currentTime - lastNotificationTimeMedium_).count());
    LOGW("StorageMonitorService left Storage Size < Medium, duration is %{public}d", duration);
    if (duration >= SEND_EVENT_INTERVAL) {
        lastNotificationTimeMedium_ = currentTime;
        lastNotificationTime_ =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        std::chrono::system_clock::now()) - std::chrono::hours(SEND_EVENT_INTERVAL);
        lastNotificationTimeHighFreq_ =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        std::chrono::system_clock::now()) - std::chrono::minutes(SMART_EVENT_INTERVAL_HIGH_FREQ);
    }
}

void StorageMonitorService::EventNotifyFreqHandlerForHigh()
{
    auto currentTime = std::chrono::system_clock::now();
    int32_t duration = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::hours>
            (currentTime - lastNotificationTime_).count());
    LOGW("StorageMonitorService left Storage Size < High, duration is %{public}d", duration);
    if (duration >= SEND_EVENT_INTERVAL) {
        lastNotificationTime_ = currentTime;
        lastNotificationTimeMedium_ =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        std::chrono::system_clock::now()) - std::chrono::hours(SEND_EVENT_INTERVAL);
        lastNotificationTimeHighFreq_ =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        std::chrono::system_clock::now()) - std::chrono::minutes(SMART_EVENT_INTERVAL_HIGH_FREQ);
    }
}

void StorageMonitorService::RefreshAllNotificationTimeStamp()
{
    lastNotificationTime_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::hours(SEND_EVENT_INTERVAL);

    lastNotificationTimeMedium_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::hours(SEND_EVENT_INTERVAL);

    lastNotificationTimeHighFreq_ =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    std::chrono::system_clock::now()) - std::chrono::minutes(SMART_EVENT_INTERVAL_HIGH_FREQ);
}
} // StorageManager
} // OHOS