/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <sstream>
#include <string>

#include "cJSON.h"
#include "common_event_manager.h"
#include "dfx_report/storage_dfx_reporter.h"
#include "file_cache_adapter.h"
#include "init_param.h"
#include "parameter.h"
#include "parameters.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_quota_controller.h"
#include "storage/storage_status_manager.h"
#include "storage/storage_total_status_service.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_stats.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
constexpr int32_t ONE_MB = 1024 * 1024;
constexpr int32_t ONE_GB = 1024 * 1024 * 1024;
constexpr int32_t CONST_NUM_TWO = 2;
constexpr int32_t CONST_NUM_ONE_HUNDRED = 100;
constexpr int32_t WAIT_THREAD_TIMEOUT_MS = 5;
constexpr int32_t DEFAULT_CHECK_INTERVAL = 60 * 1000; // 60s
constexpr int32_t SEND_EVENT_INTERVAL = 24; // day
constexpr int32_t SEND_EVENT_INTERVAL_HIGH_FREQ = 5; // 5m
constexpr int32_t STORAGE_PARAMS_PATH_LEN = 128;
constexpr int32_t STORAGE_STATIC_BEGIN_INTERVAL = 10 * 60 * 1000; //10min
constexpr int32_t STORAGE_FIRST_STATIC_HOUR = 0;
constexpr int32_t STORAGE_SECOND_STATIC_HOUR = 8;
constexpr int32_t STORAGE_THIRD_STATIC_HOUR = 16;
constexpr int32_t STORAGE_FIRST_STATIC_MINUTE = 0;
constexpr int32_t STORAGE_SECOND_STATIC_MINUTE = 1;
constexpr const char *STORAGE_ALERT_CLEANUP_PARAMETER = "const.storage_service.storage_alert_policy";
constexpr const char *DEFAULT_PARAMS = "notify_l:500M/notify_m:2G/notify_h:10%/clean_l:750M/clean_m:5%/clean_h:12%";
constexpr const char *STORAGE_ALERT_INODE_CLEANUP_PARAMETER = "const.storage_service.inode_alert_policy";
constexpr const char *INODE_DEFAULT_PARAMS =
    "notify_l:25000/notify_m:100000/notify_h:10%/clean_l:37500/clean_m:5%/clean_h:12%";
const std::string PUBLISH_SYSTEM_COMMON_EVENT = "ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT";
const std::string SMART_ACTION = "hicare.event.SMART_NOTIFICATION";
const std::string TIMESTAMP_DAY = "persist.storage_manager.timestamp.day";
const std::string TIMESTAMP_WEEK = "persist.storage_manager.timestamp.week";
const std::string FAULT_ID_ONE = "845010021";
const std::string FAULT_ID_TWO = "845010022";
const std::string FAULT_ID_THREE = "845010023";
const std::string FAULT_SUGGEST_THREE = "545010023";
const std::string FAULT_ID_INODE_HIGH = "845010008";
const std::string FAULT_ID_INODE_MEDIUM = "845010009";
const std::string FAULT_ID_INODE_LOW = "845010010";
const std::string FAULT_INODE_SUGGEST_HIGH = "545010008";
const std::string FAULT_INODE_SUGGEST_MEDIUM = "545010009";
const std::string FAULT_INODE_SUGGEST_LOW = "545010010";
const std::string CLEAN_TYPE = "type";
const std::string CLEAN_FREE = "free";
const std::string CLEAN_TOTAL = "total";

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

    auto executeUpdateBaseLineByUid = [this] { UpdateBaseLineByUid(); };
    eventHandler_->PostTask(executeUpdateBaseLineByUid, STORAGE_STATIC_BEGIN_INTERVAL);
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
    HapAndSaStatisticsThd();
    auto executeFunc = [this] { Execute(); };
    eventHandler_->PostTask(executeFunc, DEFAULT_CHECK_INTERVAL);
}

void StorageMonitorService::UpdateBaseLineByUid()
{
    LOGI("begin update base line by uid task.");
    StorageQuotaController::GetInstance().UpdateBaseLineByUid();
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

    int64_t totalInode;
    err = StorageTotalStatusService::GetInstance().GetTotalInodes(totalInode);
    if ((err != E_OK) || (totalInode <= 0)) {
        LOGE("Get device total inode fail");
        return;
    }

    int64_t freeInode;
    err = StorageTotalStatusService::GetInstance().GetFreeInodes(freeInode);
    if ((err != E_OK) || (freeInode < 0)) {
        LOGE("Get device free inode fail");
        return;
    }

    ParseStorageParameters(totalSize, totalInode);
    struct SizeInfo sizeInfo = {freeSize, totalSize, 0, 0};
    LOGI("space clean_l, size=%{public}lld, clean_m, size=%{public}lld, clean_h, size=%{public}lld",
         static_cast<long long>(thresholds["clean_l"]), static_cast<long long>(thresholds["clean_m"]),
         static_cast<long long>(thresholds["clean_h"]));
    LOGI("Inode clean_l, size=%{public}" PRId64 ", clean_m, size=%{public}" PRId64 ", clean_h, size=%{public}" PRId64,
         inodeThresholds_["clean_l"], inodeThresholds_["clean_m"], inodeThresholds_["clean_h"]);
    if (freeSize < thresholds["clean_h"] || freeInode < inodeThresholds_["clean_h"]) {
        CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    } else {
        SendCommonEventToCleanCache(CLEAN_LEVEL_RICH, sizeInfo, true);
    }

    LOGI("space notify_l, size=%{public}lld, notify_m, size=%{public}lld, notify_h, size=%{public}lld",
        static_cast<long long>(thresholds["notify_l"]), static_cast<long long>(thresholds["notify_m"]),
        static_cast<long long>(thresholds["notify_h"]));
    LOGI("Inode notify_l, size=%{public}" PRId64 ", notify_m, size=%{public}" PRId64 ", notify_h,size=%{public}" PRId64,
         inodeThresholds_["notify_l"], inodeThresholds_["notify_m"], inodeThresholds_["notify_h"]);
    if (freeSize < thresholds["notify_h"] || freeInode < inodeThresholds_["notify_h"]) {
        CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode);
        hasNotifiedStorageEvent_ = true;
    } else if (hasNotifiedStorageEvent_) {
        RefreshAllNotificationTimeStamp();
        hasNotifiedStorageEvent_ = false;
    }
}

std::string StorageMonitorService::GetStorageAlertCleanupParams()
{
    std::string storageParams;
    char tmpBuffer[STORAGE_PARAMS_PATH_LEN] = {0};

    int ret = GetParameter(STORAGE_ALERT_CLEANUP_PARAMETER, DEFAULT_PARAMS, tmpBuffer, STORAGE_PARAMS_PATH_LEN);
    if (ret <= 0) {
        LOGE("GetParameter name = %{public}s error, ret = %{public}d, return default value",
             STORAGE_ALERT_CLEANUP_PARAMETER, ret);
        std::string extraData = "Get param for storage_alert_policy, ret = " + std::to_string(ret);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_GET_CCM_PARA, extraData);
        return DEFAULT_PARAMS;
    }

    return tmpBuffer;
}

std::string StorageMonitorService::GetStorageAlertInodeCleanupParams()
{
    char tmpBuffer[STORAGE_PARAMS_PATH_LEN] = {0};
    int ret = GetParameter(STORAGE_ALERT_INODE_CLEANUP_PARAMETER, INODE_DEFAULT_PARAMS, tmpBuffer,
                           STORAGE_PARAMS_PATH_LEN);
    if (ret <= 0) {
        LOGE("GetParameter name = %{public}s error, ret = %{public}d, return default value",
             STORAGE_ALERT_INODE_CLEANUP_PARAMETER, ret);
        std::string extraData = "Get param for inode_alert_policy, ret = " + std::to_string(ret);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_GET_CCM_PARA, extraData);
        return INODE_DEFAULT_PARAMS;
    }
    return tmpBuffer;
}

bool StorageMonitorService::isNumberString(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isdigit(c); });
}

void StorageMonitorService::ParseStorageInodeParameters(int64_t totalInode, const std::string storageInodeParams)
{
    std::istringstream inodeParamsStream(storageInodeParams);
    std::string item;
    while (std::getline(inodeParamsStream, item, '/')) {
        std::string key;
        std::string value;

        size_t pos = item.find(':');
        if (pos != std::string::npos) {
            key = item.substr(0, pos);
            value = item.substr(pos + 1);
        } else {
            LOGE("Invalid parameter format, item=%{public}s", item.c_str());
            continue;
        }

        if (value.length() > 1 && value.substr(value.length() - 1) == "%") {
            int32_t percentage = std::atoi(value.substr(0, value.length() - 1).c_str());
            if (percentage < 0 || percentage > CONST_NUM_ONE_HUNDRED) {
                LOGE("Invalid percentage value, percentage=%{public}d", percentage);
            } else {
                int64_t inodeSize = (totalInode * percentage) / CONST_NUM_ONE_HUNDRED;
                inodeThresholds_[key] = static_cast<int64_t>(inodeSize);
            }
        } else if (value.length() > 0 && isNumberString(value)) {
            int64_t inodeSize = std::atoi(value.c_str());
            inodeThresholds_[key] = inodeSize;
        } else {
            LOGE("parameter value format is invalid, value=%{public}s", value.c_str());
        }
    }
}

void StorageMonitorService::ParseStorageSizeParameters(int64_t totalSize, const std::string storageParams)
{
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

void StorageMonitorService::ParseStorageParameters(int64_t totalSize, int64_t totalInode)
{
    std::string storageParams = GetStorageAlertCleanupParams();
    std::string storageInodeParams = GetStorageAlertInodeCleanupParams();
    ParseStorageSizeParameters(totalSize, storageParams);
    ParseStorageInodeParameters(totalInode, storageInodeParams);
}

void StorageMonitorService::HandleEventAndClean(const std::string &cleanLevel, struct SizeInfo &sizeInfo)
{
    int64_t lowThreshold = thresholds["clean_l"];
    int64_t lowInodeThreshold = inodeThresholds_["clean_l"];
    bool isCleanSpace = true;
    if (cleanLevel == CLEAN_LEVEL_LOW) {
        isCleanSpace = sizeInfo.freeSize < lowThreshold ? true : false;
    } else if (cleanLevel == CLEAN_LEVEL_MEDIUM) {
        isCleanSpace = sizeInfo.freeSize < thresholds["clean_m"] ? true : false;
    } else {
        isCleanSpace = sizeInfo.freeSize < thresholds["clean_h"] ? true : false;
    }

    int32_t ret = SendCommonEventToCleanCache(cleanLevel, sizeInfo, isCleanSpace);
    if (ret == E_CLEAN_TIME_INTERVAL_NOT_ENOUGH) {
        return;
    }
    CleanBundleCache(lowThreshold, lowInodeThreshold, isCleanSpace, cleanLevel);
}

void StorageMonitorService::CheckAndCleanCache(int64_t freeSize, int64_t totalSize, int64_t freeInode,
                                               int64_t totalInode)
{
    int64_t lowThreshold = thresholds["clean_l"];
    int64_t lowInodeThreshold = inodeThresholds_["clean_l"];
    if (lowThreshold <= 0 || lowInodeThreshold <= 0) {
        LOGE("Lower threshold value is invalid.");
        return;
    }
    LOGI("Device storage or inode not enough, freeSize=%{public}lld, freeInode=%{public}lld",
         static_cast<long long>(freeSize), static_cast<long long>(freeInode));

    std::string freeSizeStr = std::to_string(freeSize);
    std::string totalSizeStr = std::to_string(totalSize);
    std::string lowThresholdStr = std::to_string(lowThreshold);
    std::string freeInodeStr = std::to_string(freeInode);
    std::string totalInodeStr = std::to_string(totalInode);
    std::string lowInodeThresholdStr = std::to_string(lowInodeThreshold);
    std::string storageUsage = "storage usage not enough:freeSize = " + freeSizeStr + ", totalSize = " + totalSizeStr +
                               ", lowThreshold = " + lowThresholdStr + ", freeInode = " + freeInodeStr +
                               ", totalInode = " + totalInodeStr + ", lowInodeThreshold = " + lowInodeThresholdStr;

    struct SizeInfo sizeInfo;
    sizeInfo.freeSize = freeSize;
    sizeInfo.totalSize = totalSize;
    sizeInfo.freeInode = freeInode;
    sizeInfo.totalInode = totalInode;
    if (freeSize < lowThreshold || freeInode < lowInodeThreshold) {
        HandleEventAndClean(CLEAN_LEVEL_LOW, sizeInfo);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_LOW, storageUsage);
        LOGI("Device running is lower than low threshold");
        return;
    }
    if (freeSize < thresholds["clean_m"] || freeInode < inodeThresholds_["clean_m"]) {
        HandleEventAndClean(CLEAN_LEVEL_MEDIUM, sizeInfo);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_MEDIUM, storageUsage);
        LOGI("Device running is lower than medium threshold");
    } else {
        HandleEventAndClean(CLEAN_LEVEL_HIGH, sizeInfo);
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_CLEAN_HIGH, storageUsage);
        LOGI("Device running is lower than high threshold");
    }
    cleanLowLevelTime_.store(CLEAN_LOW_LEVEL_TIME);
}

void StorageMonitorService::ReportRadarStorageUsage(enum StorageService::BizStage stage, const std::string &extraData)
{
    LOGI("storage monitor service report radar storage usage start");
    StorageService::StorageRadar::ReportStorageUsage(stage, extraData);
}

void StorageMonitorService::CleanBundleCache(int64_t lowThreshold, int64_t lowInodeThreshold, bool isCleanSpace,
                                             const std::string &cleanLevel)
{
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return;
    }
    LOGI("Device storage free size not enough, start clean bundle cache files automatic.");
    int32_t ret = ERR_OK;
    std::optional<uint64_t> cleanedSize = 0;
    if (isCleanSpace) {
        ret = bundleMgr->CleanBundleCacheFilesAutomatic(lowThreshold * CONST_NUM_TWO,
            static_cast<OHOS::AppExecFwk::CleanType>(CleanType::CACHE_SPACE), cleanedSize);
    } else {
        ret = bundleMgr->CleanBundleCacheFilesAutomatic(lowInodeThreshold * CONST_NUM_TWO,
            static_cast<OHOS::AppExecFwk::CleanType>(CleanType::CACHE_INODE), cleanedSize);
    }
    if (ret != ERR_OK) {
        LOGE("Invoke bundleMgr interface to clean bundle cache automatic fail, ret=%{public}d", ret);
        StorageRadar::ReportBundleMgrResult("CleanBundleCacheFilesAutomatic", ret, DEFAULT_USERID, "");
        return;
    }
    if (!cleanedSize.has_value()) {
        LOGW("bundleMgr cleanedSize is empty");
    }
    if (cleanLevel == CLEAN_LEVEL_LOW) {
        if (isCleanSpace && cleanedSize < static_cast<uint64_t>(lowThreshold * CONST_NUM_TWO)) {
            LOGW("space cleanedSize did not meet expectations, cleanedSize=%{public}" PRIu64, cleanedSize.value_or(0));
            cleanLowLevelTime_.store(CLEAN_LOW_TIME_ONE_HOUR);
        } else if (!isCleanSpace && cleanedSize < static_cast<uint64_t>(lowInodeThreshold * CONST_NUM_TWO)) {
            LOGW("inode cleanedSize did not meet expectations, cleanedSize=%{public}" PRIu64, cleanedSize.value_or(0));
            cleanLowLevelTime_.store(CLEAN_LOW_TIME_ONE_HOUR);
        }
    }
    LOGI("Invoke bundleMgr interface to clean bundle cache files automatic success. cleanedSize=%{public}" PRIu64,
         cleanedSize.value_or(0));
}

void StorageMonitorService::CheckAndEventNotify(int64_t freeSize, int64_t totalSize, int64_t freeInode,
                                                int64_t totalInode)
{
    LOGI("Device storage or inode not enough, start to check and notify, freeSize=%{public}" PRId64
         ", freeInode=%{public}" PRId64, freeSize, freeInode);
    std::string freeSizeStr = std::to_string(freeSize);
    std::string totalSizeStr = std::to_string(totalSize);
    std::string freeInodeStr = std::to_string(freeInode);
    std::string totalInodeStr = std::to_string(totalInode);
    std::string storageUsage = "storage usage not enough event notify freeSize = " + freeSizeStr + ", totalSize = " +
                               totalSizeStr + ", freeInode = " + freeInodeStr + ", totalInode = " + totalInodeStr;
    bool isCleanSpace = true;
    if (freeSize < thresholds["notify_l"] || freeInode < inodeThresholds_["notify_l"]) {
        isCleanSpace = freeSize < thresholds["notify_l"] ? true : false;
        EventNotifyFreqHandlerForLow(isCleanSpace, freeInode, totalInode);
        storageUsage += ", freeSize < " + std::to_string(thresholds["notify_l"]) + ", freeInode < " +
                        std::to_string(inodeThresholds_["notify_l"]) + ", success notify event";
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_LOW, storageUsage);
        return;
    }
    if (freeSize < thresholds["notify_m"] || freeInode < inodeThresholds_["notify_m"]) {
        isCleanSpace = freeSize < thresholds["notify_m"] ? true : false;
        EventNotifyFreqHandlerForMedium(isCleanSpace, freeInode, totalInode);
        storageUsage += ", freeSize < " + std::to_string(thresholds["notify_m"]) + ", freeInode < " +
                        std::to_string(inodeThresholds_["notify_m"]) + ", success notify event";
        ReportRadarStorageUsage(StorageService::BizStage::BIZ_STAGE_THRESHOLD_NOTIFY_MEDIUM, storageUsage);
        return;
    }
    isCleanSpace = freeSize < thresholds["notify_h"] ? true : false;
    EventNotifyFreqHandlerForHigh(isCleanSpace, freeInode, totalInode);
}

std::string StorageMonitorService::GetJsonStringForInode(const std::string &faultDesc,
                                                         const std::string &faultSuggest,
                                                         int64_t freeInode,
                                                         int64_t totalInode)
{
    std::string eventDataStr = "{}";
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create json object failed.");
        return eventDataStr;
    }
    cJSON_AddStringToObject(root, "faultDescription", faultDesc.c_str());
    cJSON_AddStringToObject(root, "faultSuggestion", faultSuggest.c_str());

    cJSON *params = cJSON_CreateObject();
    if (params == nullptr) {
        LOGE("Create params json object failed.");
        cJSON_Delete(root);
        return eventDataStr;
    }
    std::string freeInodeStr = std::to_string(freeInode);
    std::string totalInodeStr = std::to_string(totalInode);
    cJSON_AddStringToObject(params, "free", freeInodeStr.c_str());
    cJSON_AddStringToObject(params, "total", totalInodeStr.c_str());

    if (!cJSON_AddItemToObject(root, "extraData", params)) {
        LOGE("Add params to root failed.");
        cJSON_Delete(params);
        cJSON_Delete(root);
        return eventDataStr;
    }

    char *json_str = cJSON_Print(root);
    if (json_str == nullptr) {
        LOGE("Print json string failed.");
        cJSON_Delete(root);
        return eventDataStr;
    }
    eventDataStr = json_str;
    cJSON_free(json_str);
    cJSON_Delete(root);
    return eventDataStr;
}

std::string StorageMonitorService::GetJsonString(const std::string &faultDesc,
    const std::string &faultSuggest, bool isHighFreq)
{
    std::string eventDataStr = "{}";
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create json object failed.");
        return eventDataStr;
    }
    cJSON_AddStringToObject(root, "faultDescription", faultDesc.c_str());
    cJSON_AddStringToObject(root, "faultSuggestion", faultSuggest.c_str());
    if (isHighFreq) {
        cJSON *faultSuggestionParam = cJSON_CreateString("500M");
        if (faultSuggestionParam == nullptr) {
            LOGE("Create json string failed.");
            cJSON_Delete(root);
            return eventDataStr;
        }
        cJSON *faultSuggestionArray = cJSON_CreateArray();
        if (faultSuggestionArray == nullptr) {
            LOGE("Create json array failed.");
            cJSON_Delete(faultSuggestionParam);
            cJSON_Delete(root);
            return eventDataStr;
        }
        cJSON_AddItemToArray(faultSuggestionArray, faultSuggestionParam);
        cJSON_AddItemToObject(root, "faultSuggestionParams", faultSuggestionArray);
    }
    char *json_string = cJSON_Print(root);
    if (json_string == nullptr) {
        LOGE("Print json string failed.");
        cJSON_Delete(root);
        return eventDataStr;
    }
    eventDataStr = json_string;
    cJSON_free(json_string);
    cJSON_Delete(root);
    return eventDataStr;
}

void StorageMonitorService::SendSmartNotificationInodeEvent(const std::string &faultDesc,
                                                            const std::string &faultSuggest,
                                                            int64_t freeInode,
                                                            int64_t totalInode)
{
    LOGI("StorageMonitorService, start SendSmartNotificationInodeEvent.");
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

    std::string eventDataStr = GetJsonStringForInode(faultDesc, faultSuggest, freeInode, totalInode);
    eventDataStr.erase(remove(eventDataStr.begin(), eventDataStr.end(), '\n'), eventDataStr.end());
    eventDataStr.erase(remove(eventDataStr.begin(), eventDataStr.end(), '\t'), eventDataStr.end());

    LOGI("send message is %{public}s", eventDataStr.c_str());
    eventData.SetData(eventDataStr);
    EventFwk::CommonEventManager::PublishCommonEvent(eventData, publishInfo, nullptr);
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

    std::string eventDataStr = GetJsonString(faultDesc, faultSuggest, isHighFreq);
    eventDataStr.erase(remove(eventDataStr.begin(), eventDataStr.end(), '\n'), eventDataStr.end());
    eventDataStr.erase(remove(eventDataStr.begin(), eventDataStr.end(), '\t'), eventDataStr.end());

    LOGI("send message is %{public}s", eventDataStr.c_str());
    eventData.SetData(eventDataStr);
    EventFwk::CommonEventManager::PublishCommonEvent(eventData, publishInfo, nullptr);
}

void StorageMonitorService::EventNotifyFreqHandlerForLow(bool isCleanSpace, int64_t freeInode, int64_t totalInode)
{
    auto currentTime = std::chrono::system_clock::now();
    int32_t duration = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::minutes>
            (currentTime - lastNotificationTimeHighFreq_).count());
    LOGW("StorageMonitorService notify, left Storage Size < Low, duration is %{public}d", duration);
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

void StorageMonitorService::EventNotifyFreqHandlerForMedium(bool isCleanSpace, int64_t freeInode, int64_t totalInode)
{
    auto currentTime = std::chrono::system_clock::now();
    int32_t duration = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::hours>
            (currentTime - lastNotificationTimeMedium_).count());
    LOGW("StorageMonitorService notify, left Storage Size < Medium, duration is %{public}d", duration);
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

void StorageMonitorService::EventNotifyFreqHandlerForHigh(bool isCleanSpace, int64_t freeInode, int64_t totalInode)
{
    auto currentTime = std::chrono::system_clock::now();
    int32_t duration = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::hours>
            (currentTime - lastNotificationTime_).count());
    LOGW("StorageMonitorService notify, left Storage Size < High, duration is %{public}d", duration);
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

void StorageMonitorService::HapAndSaStatisticsThd()
{
    // 当前时间是0:00 || 8:00: || 16:00 打印
    std::time_t now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);
    if (localTime == nullptr) {
        LOGE("cur time parse failed, errno is %{public}d.", errno);
        return;
    }
    if ((localTime->tm_min != STORAGE_FIRST_STATIC_MINUTE &&
        localTime->tm_min != STORAGE_SECOND_STATIC_MINUTE) ||
        (localTime->tm_hour != STORAGE_FIRST_STATIC_HOUR &&
         localTime->tm_hour != STORAGE_SECOND_STATIC_HOUR &&
         localTime->tm_hour != STORAGE_THIRD_STATIC_HOUR)) {
        return;
    }
    StorageDfxReporter::GetInstance().CheckAndTriggerHapAndSaStatistics();
}

int32_t StorageMonitorService::SendCommonEventToCleanCache(const std::string &cleanLevel, struct SizeInfo &sizeInfo,
                                                           bool isCleanSpace)
{
    if (cleanLevel.empty()) {
        return E_PARAMS_INVALID;
    }
    std::lock_guard<std::mutex> lock(notifyCleanMtx_);
    time_t now = std::time(nullptr);
    if (now == static_cast<time_t>(E_ERR)) {
        LOGE("get sys time failed, errno is %{public}d", errno);
        return E_PARAMS_INVALID;
    }
    int64_t curTime = static_cast<int64_t>(now);
    int64_t lastNotifyTime = 0;
    int32_t ret = GetLastNotifyTime(cleanLevel, lastNotifyTime);
    if (ret != E_OK) {
        LOGW("GetLastNotifyTime failed, insert a new value.");
        SetLastNotifyTime(cleanLevel, curTime);
        if (lastNotifyTime == 0) {
            PublishCleanCacheEvent(cleanLevel, isCleanSpace, sizeInfo);
        }
        return ret;
    }
    int64_t interval = curTime - lastNotifyTime;
    if ((cleanLevel == CLEAN_LEVEL_LOW && interval < cleanLowLevelTime_.load()) ||
        (cleanLevel == CLEAN_LEVEL_MEDIUM && interval < CLEAN_MEDIUM_TIME) ||
        (cleanLevel == CLEAN_LEVEL_HIGH && interval < CLEAN_HIGH_TIME) ||
        (cleanLevel == CLEAN_LEVEL_RICH && interval < CLEAN_RICH_TIME)) {
        return E_CLEAN_TIME_INTERVAL_NOT_ENOUGH;
    }
    PublishCleanCacheEvent(cleanLevel, isCleanSpace, sizeInfo);
    SetLastNotifyTime(cleanLevel, curTime);
    return E_OK;
}

void StorageMonitorService::PublishCleanCacheEvent(const std::string &cleanLevel, bool isCleanSpace,
                                                   struct SizeInfo &sizeInfo)
{
    AAFwk::Want want;
    want.SetAction("usual.event.DEVICE_STORAGE_LOW");
    std::string cleanType = "";
    int64_t freeSize = 0;
    int64_t totalSize = 0;
    if (isCleanSpace) {
        cleanType = "space";
        freeSize = sizeInfo.freeSize;
        totalSize = sizeInfo.totalSize;
    } else {
        cleanType = "inode";
        freeSize = sizeInfo.freeInode;
        totalSize = sizeInfo.totalInode;
    }
    want.SetParam(CLEAN_LEVEL, static_cast<std::string>(cleanLevel));
    want.SetParam(CLEAN_TYPE, static_cast<std::string>(cleanType));
    want.SetParam(CLEAN_FREE, static_cast<int64_t>(freeSize));
    want.SetParam(CLEAN_TOTAL, static_cast<int64_t>(totalSize));
    EventFwk::CommonEventData commonData{want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
    LOGI("Send usual.event.DEVICE_STORAGE_LOW event success, cleanLevel=%{public}s, cleantype=%{public}s, "
         "freeSize=%{public}" PRId64 ", totalSize=%{public}" PRId64, cleanLevel.c_str(), cleanType.c_str(), freeSize,
         totalSize);
}

int32_t StorageMonitorService::GetLastNotifyTime(const std::string &cleanLevel, int64_t &lastNotifyTime)
{
    auto notifyPtr = FileCacheAdapter::GetInstance().GetCleanNotify(cleanLevel);
    if (notifyPtr == nullptr) {
        lastNotifyTime = 0;
        return E_NON_EXIST;
    }

    lastNotifyTime = notifyPtr->lastCleanNotifyTime;
    return E_OK;
}

int32_t StorageMonitorService::SetLastNotifyTime(const std::string &cleanLevel, int64_t curTime)
{
    CleanNotify notify;
    notify.cleanLevelName = cleanLevel;
    notify.lastCleanNotifyTime = curTime;

    return FileCacheAdapter::GetInstance().InsertOrUpdateCleanNotify(notify);
}

} // StorageManager
} // OHOS