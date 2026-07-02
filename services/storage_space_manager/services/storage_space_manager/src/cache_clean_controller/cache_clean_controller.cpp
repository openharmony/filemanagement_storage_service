/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cache_clean_controller/cache_clean_controller.h"
#include "adapter/bundle_manager_connector.h"
#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"
#include "storage_space_manager_client.h"
#include "storage/storage_total_status_service.h"
#include "utils/storage_radar.h"

#include <algorithm>
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>

namespace {
OHOS::AppExecFwk::CleanCacheInfo ToAppExecFwkCleanCacheInfo(const CleanCacheInfo &info)
{
    OHOS::AppExecFwk::CleanCacheInfo result;
    result.bundleName = info.bundleName;
    result.userId = info.userId;
    result.appIndex = info.appIndex;
    result.cacheThreshold = info.cacheThreshold;
    return result;
}
}
#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace StorageSpaceManager {

namespace {
constexpr uint64_t MB_TO_BYTES = 1024ULL * 1024;
constexpr uint64_t GB_TO_BYTES = 1024ULL * 1024 * 1024;
constexpr size_t BOUNDARY = 3000;
constexpr const char* CONFIG_DIR = "/data/service/el1/public/storage_space_manager";
constexpr const char* CONFIG_FILE_NAME = "cache_clean_config.json";
constexpr const char* LAST_CLEAN_CACHE_TIMESTAMP = "last_cache_clean_timestamp";
constexpr const char* LIB_QUOTA_CALCULATOR_NAME = "libstorage_service_ext_adapter.z.so";
constexpr int32_t MIN_RANK = 1;
constexpr const char* STORAGE_RANGE_PREFIX = "size_lowerlimit_";
constexpr const char* STORAGE_RANGE_SUFFIX = "_upperlimit_";
constexpr const char* MAX_LABEL = "max";
constexpr const char* DEFAULT_TIER_NAME = "default";
constexpr const char* TOP_TIER_PREFIX = "top";
constexpr const char* TOP_APP_CACHE_CONFIG = "top_app_cache_config";
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
constexpr int32_t DEFAULT_TOP_RANKING_HOURS = 336;
constexpr int32_t DEFAULT_NO_USE_HOURS = 2160;
constexpr int32_t DEFAULT_TOP_COUNT = 20;
#endif
constexpr uint64_t DISPLAY_MB_DIVISOR = 1000ULL * 1000;
constexpr mode_t CONFIG_DIR_MODE = 0755;
constexpr int32_t JSON_INDENT = 4;
// Default quota config, used as fallback when GetQuotaByRank fails
// e.g., the number in size_lowerlimit_0_upperlimit_256 is in GB
//       tier units are MB, e.g. "top1-3": 750 means 750MB
// Format matches top_app_cache_config in the cache cleanup config file
constexpr const char* DEFAULT_QUOTA_CONFIG = R"({
    "top_app_cache_config": {
        "size_lowerlimit_0_upperlimit_256": {
            "top1-3": 750,
            "top4-10": 300,
            "top11-20": 150,
            "default": 50
        },
        "size_lowerlimit_257_upperlimit_max": {
            "top1-3": 1500,
            "top4-10": 500,
            "top11-20": 250,
            "default": 50
        }
    }
})";
struct StorageRangeInfo {
    int64_t lowerLimit;
    int64_t upperLimit;
};

bool ParseStorageRangeKey(const std::string &key, StorageRangeInfo &info)
{
    if (key.find(STORAGE_RANGE_PREFIX) != 0) {
        return false;
    }
    std::string remaining = key.substr(strlen(STORAGE_RANGE_PREFIX));
    size_t upperPos = remaining.find(STORAGE_RANGE_SUFFIX);
    if (upperPos == std::string::npos) {
        return false;
    }
    info.lowerLimit = std::atoll(remaining.substr(0, upperPos).c_str());
    std::string upperStr = remaining.substr(upperPos + strlen(STORAGE_RANGE_SUFFIX));
    if (upperStr == MAX_LABEL) {
        info.upperLimit = -1;
    } else {
        info.upperLimit = std::atoll(upperStr.c_str());
    }
    return true;
}

bool IsInStorageRange(int64_t totalGB, const StorageRangeInfo &info)
{
    if (totalGB < info.lowerLimit) {
        return false;
    }
    if (info.upperLimit >= 0 && totalGB > info.upperLimit) {
        return false;
    }
    return true;
}

bool MatchTierByRank(const std::string &tierName, int32_t tierQuota, int32_t rank, int32_t &quota)
{
    if (tierName.find(TOP_TIER_PREFIX) != 0 || tierName.size() <= strlen(TOP_TIER_PREFIX)) {
        return false;
    }
    std::string rankRange = tierName.substr(strlen(TOP_TIER_PREFIX));
    size_t dashPos = rankRange.find("-");
    if (dashPos == std::string::npos) {
        return false;
    }
    int32_t lowerRank = std::atoi(rankRange.substr(0, dashPos).c_str());
    int32_t upperRank = std::atoi(rankRange.substr(dashPos + 1).c_str());
    if (rank < lowerRank || rank > upperRank) {
        return false;
    }
    quota = tierQuota;
    return true;
}

int32_t GetQuotaFromRangeConfig(const nlohmann::json &rangeConfig, int32_t rank, int32_t &quota)
{
    int32_t defaultQuota = 0;
    bool foundDefault = false;
    for (auto it = rangeConfig.begin(); it != rangeConfig.end(); ++it) {
        std::string tierName = it.key();
        int32_t tierQuota = it.value().get<int32_t>();
        if (tierName == DEFAULT_TIER_NAME) {
            defaultQuota = tierQuota;
            foundDefault = true;
            continue;
        }
        if (MatchTierByRank(tierName, tierQuota, rank, quota)) {
            return E_OK;
        }
    }
    if (foundDefault) {
        quota = defaultQuota;
        return E_OK;
    }
    return E_FAIL;
}
} // namespace

CacheCleanController::CacheCleanController()
{
}

CacheCleanController::~CacheCleanController()
{
    if (quotaCalculatorSoHandle_ != nullptr) {
        dlclose(quotaCalculatorSoHandle_);
        quotaCalculatorSoHandle_ = nullptr;
    }
}

bool CacheCleanController::LoadQuotaCalculator()
{
    {
        std::lock_guard<std::mutex> lock(loadQuotaMutex_);
        if (quotaCalculatorSoLoaded_ && quotaCalculator_ != nullptr) {
            quotaCalculator_->Init();
            return true;
        }
        if (quotaCalculatorSoLoaded_) {
            LOGE("quotaCalculatorSoLoaded_ is true but quotaCalculator_ is null, reloading");
        }
        quotaCalculatorSoHandle_ = dlopen(LIB_QUOTA_CALCULATOR_NAME, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
        if (quotaCalculatorSoHandle_ == nullptr) {
            quotaCalculatorSoHandle_ = dlopen(LIB_QUOTA_CALCULATOR_NAME, RTLD_NOW | RTLD_NODELETE);
        }
        if (quotaCalculatorSoHandle_ == nullptr) {
            LOGE("load quota calculator so failed.");
            return false;
        }
        dlerror();
        auto func = (CreateQuotaCalculatorFuncPtr)dlsym(quotaCalculatorSoHandle_, "CreateQuotaCalculatorObject");
        if (dlerror() != nullptr || func == nullptr) {
            dlclose(quotaCalculatorSoHandle_);
            quotaCalculatorSoHandle_ = nullptr;
            LOGE("Create object function is not exist.");
            return false;
        }
        quotaCalculator_ = std::shared_ptr<IQuotaCalculator>(func());
        if (quotaCalculator_ == nullptr) {
            dlclose(quotaCalculatorSoHandle_);
            quotaCalculatorSoHandle_ = nullptr;
            LOGE("quota calculator instance is null.");
            return false;
        }
        quotaCalculator_->Init();
        quotaCalculatorSoLoaded_ = true;
    }
    LOGI("Success.");
    return true;
}

int32_t CacheCleanController::CleanBundleCache(int32_t userId)
{
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    LOGI("Starting bundle cache clean for user %{public}d", userId);
    if (userId < 0) {
        LOGE("Invalid userId: %{public}d", userId);
        return E_INVALID_ARGUMENT;
    }
    if (!LoadQuotaCalculator()) {
        LOGE("Failed to load quota calculator");
        return E_FAIL;
    }
    if (isCleanRunning_.load()) {
        LOGE("Clean cache is running");
        return E_OK;
    }
    if (quotaCalculator_->GetCacheAutoCleanSwitch() == CacheAutoCleanSwitch::CLOSE) {
        LOGE("CacheAutoCleanSwitch is CLOSE");
        return E_OK;
    }
    return ExecuteCleanBundleCache(userId);
#endif
    return E_OK;
}

int32_t CacheCleanController::ExecuteCleanBundleCache(int32_t userId)
{
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    isCleanRunning_.store(true);
    stopCleanCacheFlag_.store(false);
    std::vector<ApplicationInfo> appInfos;
    int32_t ret = GetAllAppInfos(userId, appInfos);
    if (ret != E_OK) {
        LOGE("Failed to get apps for user %{public}d", userId);
        isCleanRunning_.store(false);
        return ret;
    }
    std::vector<CleanCacheInfo> rankedCleanInfos;
    std::vector<CleanCacheInfo> cleanAllCacheInfos;
    ret = BundleActiveRank(userId, appInfos, rankedCleanInfos, cleanAllCacheInfos);
    if (ret != E_OK) {
        LOGE("Failed to rank bundles, ret=%{public}d", ret);
        isCleanRunning_.store(false);
        return ret;
    }
    CleanResources resources;
    resources.userId = userId;
    ret = PrepareCleanResources(resources);
    if (ret != E_OK) {
        isCleanRunning_.store(false);
        return ret;
    }
    CleanStats stats;
    ret = ExecuteCacheCleaning(rankedCleanInfos, cleanAllCacheInfos, resources, stats);
    if (ret != E_OK) {
        isCleanRunning_.store(false);
        return ret;
    }
    int64_t currentTime = GetCurrentTime();
    ret = SaveCacheCleaningTimestamp(currentTime);
    if (ret != E_OK) {
        LOGE("Failed to save cache cleaning timestamp");
    }
    LOGI("Cache clean completed: success=%{public}d, failed=%{public}d",
         stats.totalCleanedCount, stats.failedCount);
    isCleanRunning_.store(false);
    stopCleanCacheFlag_.store(false);
    return (stats.failedCount == 0) ? E_OK : E_FAIL;
#else
    return E_OK;
#endif
}

int32_t CacheCleanController::PrepareCleanResources(CleanResources &resources)
{
    resources.bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (resources.bundleMgr == nullptr) {
        LOGE("Failed to get bundle manager service");
        return E_SERVICE_IS_NULLPTR;
    }

    int32_t storageRet = StorageTotalStatusService::GetInstance().GetTotalSize(resources.totalStorage);
    if (storageRet != E_OK) {
        LOGE("Failed to get storage size, ret=%{public}d", storageRet);
        return storageRet;
    }

    return E_OK;
}

int32_t CacheCleanController::ExecuteCacheCleaning(const std::vector<CleanCacheInfo> &rankedCleanInfos,
    const std::vector<CleanCacheInfo> &cleanAllCacheInfos,
    const CleanResources &resources, CleanStats &stats)
{
    extraData_.str("");
    auto timeStamp = GetCurrentTime();
    extraData_ << "{startTimeStamp:" << timeStamp << "}" << std::endl;
    extraData_ << "{userId:" << resources.userId << "}" << std::endl;
    // Clean all cache for remaining apps
    int32_t ret = CleanAllCacheForApps(cleanAllCacheInfos, resources, stats);
    if (ret != E_OK) {
        LOGE("Full clean failed, ret=%{public}d", ret);
        return ret;
    }

    // Clean rank-based cache
    ret = CleanRankBasedCache(rankedCleanInfos, resources, stats);
    if (ret != E_OK) {
        LOGE("Rank-based clean failed, ret=%{public}d", ret);
        return ret;
    }
    extraData_ << "{totalCacheBeforeClean:" << stats.cleanBefore << "}" << std::endl;
    extraData_ << "{totalCacheAfterClean:" << stats.cleanAfter << "}" << std::endl;
    extraData_ << "{appSuccCount:" << stats.totalCleanedCount << "}" << std::endl;
    extraData_ << "{appFailCount:" << stats.failedCount << "}" << std::endl;
    timeStamp = GetCurrentTime();
    extraData_ << "{endTimeStamp:" << timeStamp << "}" << std::endl;
    PrintOverLongLog(extraData_.str());
    StorageService::StorageRadar::ReportStorageStatusRadar("cacheCleanResult", extraData_.str());

    return E_OK;
}

void CacheCleanController::PrintOverLongLog(std::string str)
{
    size_t len = str.size();
    while (len > 0) {
        std::string sub = str.substr(0, std::min(len, BOUNDARY));
        LOGI("%{public}s", sub.c_str());
        if (len <= BOUNDARY) {
            return;
        }
        str = str.substr(BOUNDARY);
        len -= BOUNDARY;
    }
}

int32_t CacheCleanController::CleanRankBasedCache(const std::vector<CleanCacheInfo> &rankedCleanInfos,
    const CleanResources &resources, CleanStats &stats)
{
    LOGI("Starting rank-based cleaning for %{public}zu apps", rankedCleanInfos.size());

    std::unordered_map<std::string, int32_t> systemAppCacheQuata = quotaCalculator_->GetSystemAppCacheSize();

    for (size_t i = 0; i < rankedCleanInfos.size(); ++i) {
        if (stopCleanCacheFlag_.load()) {
            LOGE("Cleanup conditions not met, stopCleanCacheFlag is true");
            return E_OK;
        }

        // Check for index overflow
        if (i + 1 > static_cast<size_t>(INT32_MAX)) {
            LOGE("Index overflow when converting to rank");
            stats.failedCount++;
            continue;
        }

        int32_t rank = static_cast<int32_t>(i + 1);
        int32_t ret = CleanSingleAppWithQuota(rankedCleanInfos[i], rank, resources, stats,
            systemAppCacheQuata);
        if (ret != E_OK) {
            continue;
        }
    }

    return E_OK;
}

int32_t CacheCleanController::CleanSingleAppWithQuota(const CleanCacheInfo &cleanInfo, int32_t rank,
    const CleanResources &resources, CleanStats &stats,
    const std::unordered_map<std::string, int32_t> &systemAppCacheQuata)
{
    int32_t quotaMB = 0;
    int32_t ret = quotaCalculator_->GetQuotaByRank(rank, resources.totalStorage, quotaMB);
    if (ret != E_OK) {
        LOGE("Failed to get quota for rank %{public}d, trying default quota", rank);
        ret = GetDefaultQuotaByRank(rank, resources.totalStorage, quotaMB);
        if (ret != E_OK) {
            LOGE("Failed to get default quota for rank %{public}d", rank);
            stats.failedCount++;
            return ret;
        }
    }

    if (quotaMB < 0) {
        LOGE("Invalid quota: %{public}d MB", quotaMB);
        stats.failedCount++;
        return E_INVALID_ARGUMENT;
    }
    if (systemAppCacheQuata.find(cleanInfo.bundleName) != systemAppCacheQuata.end()) {
        quotaMB = systemAppCacheQuata.at(cleanInfo.bundleName);
    }
    uint64_t quotaBytes = static_cast<uint64_t>(quotaMB) * MB_TO_BYTES;
    CleanCacheInfo cacheInfo = cleanInfo;
    cacheInfo.cacheThreshold = quotaBytes;

    LOGI("Cleaning %{public}s: rank=%{public}d, quota=%{public}d MB",
         cleanInfo.bundleName.c_str(), rank, quotaMB);

    return PerformCacheCleaning(cacheInfo, resources, stats);
}

void CacheCleanController::WriteCleanInfoToExtra(const CleanCacheInfo &cleanInfo, uint64_t before,
    uint64_t after, bool isSucc)
{
    std::string index = "";
    if (cleanInfo.appIndex != 0) {
        index = std::string(",IDX:") + std::to_string(cleanInfo.appIndex);
    }
    if (isSucc) {
        extraData_ << "{BN:" << cleanInfo.bundleName << index <<
            ",CT:" << cleanInfo.cacheThreshold << ",BS:" << before <<
            ",AS:" << after << "}" << std::endl;
            return;
    }
    extraData_ << "{BN:" << cleanInfo.bundleName << index <<
            ",CT:" << cleanInfo.cacheThreshold << ",FAIL}" << std::endl;
}

int32_t CacheCleanController::PerformCacheCleaning(const CleanCacheInfo &cleanInfo,
    const CleanResources &resources, CleanStats &stats)
{
    uint64_t beforeCleanedSize = 0;
    uint64_t afterCleanedSize = 0;

    ErrCode cleanRet = resources.bundleMgr->CleanBundlePartialCacheAutomatic(
        ToAppExecFwkCleanCacheInfo(cleanInfo), beforeCleanedSize, afterCleanedSize);
    if (cleanRet != ERR_OK) {
        LOGE("Failed to clean %{public}s, ret=%{public}d",
             cleanInfo.bundleName.c_str(), cleanRet);
        stats.failedCount++;
        WriteCleanInfoToExtra(cleanInfo, beforeCleanedSize, afterCleanedSize, false);
        return E_FAIL;
    }

    LOGI("Cleaned %{public}s: +%{public}llu MB",
         cleanInfo.bundleName.c_str(),
         static_cast<unsigned long long>((beforeCleanedSize - afterCleanedSize) / DISPLAY_MB_DIVISOR));
    stats.totalCleanedCount++;
    stats.cleanBefore += beforeCleanedSize;
    stats.cleanAfter += afterCleanedSize;
    WriteCleanInfoToExtra(cleanInfo, beforeCleanedSize, afterCleanedSize, true);

    return E_OK;
}

int32_t CacheCleanController::CleanAllCacheForApps(const std::vector<CleanCacheInfo> &cleanAllCacheInfos,
    const CleanResources &resources, CleanStats &stats)
{
    LOGI("Starting full cache clean for %{public}zu apps", cleanAllCacheInfos.size());
    std::unordered_map<std::string, int32_t> systemAppCacheQuata = quotaCalculator_->GetSystemAppCacheSize();

    for (size_t i = 0; i < cleanAllCacheInfos.size(); ++i) {
        if (stopCleanCacheFlag_.load()) {
            LOGE("Cleanup conditions not met, stopCleanCacheFlag is true");
            return E_OK;
        }
        uint64_t beforeCleanedSize = 0;
        uint64_t afterCleanedSize = 0;

        // cacheThreshold=0 means clean all cache
        CleanCacheInfo cacheInfo = cleanAllCacheInfos[i];
        cacheInfo.cacheThreshold = 0;
        if (systemAppCacheQuata.find(cacheInfo.bundleName) != systemAppCacheQuata.end()) {
            cacheInfo.cacheThreshold = systemAppCacheQuata.at(cacheInfo.bundleName);
        }
        ErrCode cleanRet = resources.bundleMgr->CleanBundlePartialCacheAutomatic(
            ToAppExecFwkCleanCacheInfo(cacheInfo), beforeCleanedSize, afterCleanedSize);
        if (cleanRet != ERR_OK) {
            LOGE("Failed to clean %{public}s, ret=%{public}d",
                 cleanAllCacheInfos[i].bundleName.c_str(), cleanRet);
            stats.failedCount++;
            WriteCleanInfoToExtra(cacheInfo, beforeCleanedSize, afterCleanedSize, false);
            continue;
        }

        LOGI("Cleaned %{public}s: +%{public}llu MB",
             cleanAllCacheInfos[i].bundleName.c_str(),
             static_cast<unsigned long long>((beforeCleanedSize - afterCleanedSize) / DISPLAY_MB_DIVISOR));
        stats.totalCleanedCount++;
        stats.cleanBefore += beforeCleanedSize;
        stats.cleanAfter += afterCleanedSize;
        WriteCleanInfoToExtra(cacheInfo, beforeCleanedSize, afterCleanedSize, true);
    }

    return E_OK;
}

int32_t CacheCleanController::GetAllAppInfos(int32_t userId, std::vector<ApplicationInfo> &appInfos)
{
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Failed to get bundle manager service");
        return E_SERVICE_IS_NULLPTR;
    }
    ErrCode ret = bundleMgr->GetApplicationInfosV9(static_cast<int32_t>(
        AppExecFwk::GetApplicationFlag::GET_APPLICATION_INFO_WITH_DISABLE), userId, appInfos);
    if (ret != ERR_OK) {
        LOGE("Failed to get apps, ret=%{public}d", ret);
        return ret;
    }
    std::vector<ApplicationInfo> filteredAppInfos;
    FilterAppInfosByBundleType(appInfos, filteredAppInfos);
    appInfos = filteredAppInfos;
    LOGI("Got %{public}zu apps for user %{public}d", appInfos.size(), userId);
    return E_OK;
}

int32_t CacheCleanController::FilterAppInfosByBundleType(const std::vector<ApplicationInfo> &appInfos,
    std::vector<ApplicationInfo> &filteredAppInfos)
{
    if (appInfos.empty()) {
        return E_OK;
    }

    for (const auto &appInfo : appInfos) {
        if (appInfo.bundleType == AppExecFwk::BundleType::APP) {
            filteredAppInfos.push_back(appInfo);
        }
    }
    return E_OK;
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
int32_t CacheCleanController::BundleActiveRank(int32_t userId, const std::vector<ApplicationInfo> &appInfos,
    std::vector<CleanCacheInfo> &rankedCleanInfos, std::vector<CleanCacheInfo> &cleanAllCacheInfos)
{
    if (userId < 0) {
        LOGE("Invalid userId: %{public}d", userId);
        return E_INVALID_ARGUMENT;
    }

    // Get configuration and validate
    int32_t topRankingHoursSpan = 0;
    int32_t noUseHoursForCleanAll = 0;
    int32_t ret = GetConfigAndValidate(topRankingHoursSpan, noUseHoursForCleanAll);
    if (ret != E_OK) {
        topRankingHoursSpan = DEFAULT_TOP_RANKING_HOURS;
        noUseHoursForCleanAll = DEFAULT_NO_USE_HOURS;
        LOGE("GetConfigAndValidate failed, use default hours, topRankingHoursSpan : %{public}d,\
            noUseHoursForCleanAll : %{public}d", topRankingHoursSpan, noUseHoursForCleanAll);
    }

    // Query bundle stats for different time ranges
    std::vector<BundleActivePackageStats> topRankingStats;
    std::vector<BundleActivePackageStats> activeStats;
    ret = QueryBundleStatsForRanking(userId, topRankingHoursSpan, noUseHoursForCleanAll,
        topRankingStats, activeStats);
    if (ret != E_OK) {
        return ret;
    }

    // Get top app count and process stats
    int32_t topCount = 0;
    ret = GetTopAppCountFromStorage(topCount);
    if (ret != E_OK) {
        topCount = DEFAULT_TOP_COUNT;
        LOGE("GetTopAppCountFromStorage failed, use default topCount : %{public}d", topCount);
    }

    ret = ProcessBundleActiveStats(topRankingStats, activeStats,
                                   appInfos, userId, topCount);
    if (ret != E_OK) {
        return ret;
    }

    // Build clean cache lists for ranking and full cleaning
    CleanCacheBuildParams buildParams;
    buildParams.bundleActiveStatsTopRanking = topRankingStats;
    buildParams.bundleActiveStatsExceptNoUse = activeStats;
    buildParams.appInfos = &appInfos;
    buildParams.userId = userId;
    BuildCleanCacheInfos(buildParams, rankedCleanInfos, cleanAllCacheInfos);

    return E_OK;
}
#endif

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
int32_t CacheCleanController::QueryBundleStatsForRanking(
    int32_t userId, int32_t topRankingHoursSpan, int32_t noUseHoursForCleanAll,
    std::vector<BundleActivePackageStats> &topRankingStats,
    std::vector<BundleActivePackageStats> &activeStats)
{
    int64_t currentTime = GetCurrentTime();

    BundleStatsQueryParams queryParams;
    queryParams.userId = userId;
    queryParams.topRankingHoursSpan = topRankingHoursSpan;
    queryParams.noUseHoursForCleanAll = noUseHoursForCleanAll;
    queryParams.currentTime = currentTime;
    return QueryAllBundleStats(queryParams, topRankingStats, activeStats);
}

void CacheCleanController::SortBundleStatsByInFrontTime(std::vector<BundleActivePackageStats> &bundleStats)
{
    if (bundleStats.empty()) {
        return;
    }
    // Sort by totalInFrontTime_ in descending order
    std::sort(bundleStats.begin(), bundleStats.end(),
        [](const BundleActivePackageStats &a, const BundleActivePackageStats &b) {
            return a.totalInFrontTime_ > b.totalInFrontTime_;
    });
}

void CacheCleanController::FilterBundleStatsByAppType(std::vector<BundleActivePackageStats> &bundleStats,
    const std::vector<ApplicationInfo> &appInfos)
{
    if (bundleStats.empty() || appInfos.empty()) {
        bundleStats.clear();
        return;
    }

    // Create set for O(1) lookup performance
    std::set<std::pair<std::string, int32_t>> appInfoSet;
    for (const auto &appInfo : appInfos) {
        appInfoSet.insert({appInfo.bundleName, appInfo.appIndex});
    }

    auto it = bundleStats.begin();
    while (it != bundleStats.end()) {
        const auto &key = std::make_pair(it->bundleName_, it->appIndex_);
        if (appInfoSet.find(key) == appInfoSet.end()) {
            it = bundleStats.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<ApplicationInfo> CacheCleanController::RemoveAppsByStats(
    const std::vector<ApplicationInfo> &appInfos, const std::vector<BundleActivePackageStats> &bundleStatsToRemove)
{
    if (appInfos.empty() || bundleStatsToRemove.empty()) {
        return appInfos;
    }

    std::set<std::pair<std::string, int32_t>> removeSet;
    for (const auto &stats : bundleStatsToRemove) {
        removeSet.insert({stats.bundleName_, stats.appIndex_});
    }

    std::vector<ApplicationInfo> result;
    for (const auto &appInfo : appInfos) {
        const auto &key = std::make_pair(appInfo.bundleName, appInfo.appIndex);
        if (removeSet.find(key) == removeSet.end()) {
            result.push_back(appInfo);
        }
    }

    return result;
}

std::vector<BundleActivePackageStats> CacheCleanController::RemoveBundleStatsByBundleStats(
    const std::vector<BundleActivePackageStats> &bundleStats,
    const std::vector<BundleActivePackageStats> &bundleStatsToRemove)
{
    if (bundleStats.empty() || bundleStatsToRemove.empty()) {
        return bundleStats;
    }

    std::set<std::pair<std::string, int32_t>> removeSet;
    for (const auto &stats : bundleStatsToRemove) {
        removeSet.insert({stats.bundleName_, stats.appIndex_});
    }

    std::vector<BundleActivePackageStats> result;
    for (const auto &stats : bundleStats) {
        const auto &key = std::make_pair(stats.bundleName_, stats.appIndex_);
        if (removeSet.find(key) == removeSet.end()) {
            result.push_back(stats);
        }
    }

    return result;
}
#endif

void CacheCleanController::CalculateTimeRange(int64_t currentTime, int32_t hoursSpan,
    int64_t &startTime, int64_t &endTime)
{
    constexpr int64_t MILLIS_PER_HOUR = 60 * 60 * 1000LL;
    startTime = currentTime - (hoursSpan * MILLIS_PER_HOUR);
    endTime = currentTime;
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
int32_t CacheCleanController::QueryBundleActiveStats(int32_t userId, int64_t startTime, int64_t endTime,
    std::vector<BundleActivePackageStats> &bundleStats)
{
    ErrCode ret = DeviceUsageStats::BundleActiveClient::GetInstance().
        QueryBundleStatsInfoByInterval(bundleStats, 0, startTime, endTime, userId);
    if (ret != ERR_OK) {
        LOGE("Query bundle stats failed, range=[%{public}lld, %{public}lld]",
             static_cast<long long>(startTime), static_cast<long long>(endTime));
        return E_FAIL;
    }
    return E_OK;
}

int32_t CacheCleanController::ProcessBundleActiveStats(std::vector<BundleActivePackageStats> &topRankingStats,
    std::vector<BundleActivePackageStats> &activeStats, const std::vector<ApplicationInfo> &appInfos,
    int32_t userId, int32_t topCount)
{
    // Filter to keep only APP type applications
    FilterBundleStatsByAppType(topRankingStats, appInfos);
    FilterBundleStatsByAppType(activeStats, appInfos);

    // Sort by usage level (most active first)
    SortBundleStatsByInFrontTime(topRankingStats);
    SortBundleStatsByInFrontTime(activeStats);

    // Keep only top N most active apps
    if (topCount > 0 && static_cast<size_t>(topCount) < topRankingStats.size()) {
        topRankingStats.resize(topCount);
    }

    LOGI("Processed stats: topRanking=%{public}zu, active=%{public}zu, topCount=%{public}d",
         topRankingStats.size(), activeStats.size(), topCount);

    return E_OK;
}

void CacheCleanController::ConvertBundleStatsToCleanCacheInfo(const std::vector<BundleActivePackageStats> &bundleStats,
    int32_t userId, std::vector<CleanCacheInfo> &cleanCacheInfos)
{
    cleanCacheInfos.reserve(cleanCacheInfos.size() + bundleStats.size());
    for (const auto &stats : bundleStats) {
        CleanCacheInfo cleanInfo;
        cleanInfo.bundleName = stats.bundleName_;
        cleanInfo.userId = userId;
        cleanInfo.appIndex = stats.appIndex_;
        cleanInfo.cacheThreshold = 0;
        cleanCacheInfos.push_back(cleanInfo);
    }
}
#endif

void CacheCleanController::ConvertAppInfosToCleanCacheInfo(const std::vector<ApplicationInfo> &appInfos,
    int32_t userId, std::vector<CleanCacheInfo> &cleanCacheInfos)
{
    cleanCacheInfos.reserve(cleanCacheInfos.size() + appInfos.size());
    for (const auto &appInfo : appInfos) {
        CleanCacheInfo cleanInfo;
        cleanInfo.bundleName = appInfo.bundleName;
        cleanInfo.userId = userId;
        cleanInfo.appIndex = appInfo.appIndex;
        cleanInfo.cacheThreshold = 0;
        cleanCacheInfos.push_back(cleanInfo);
    }
}

int32_t CacheCleanController::GetConfigAndValidate(int32_t &topRankingHoursSpan, int32_t &noUseHoursForCleanAll)
{
    topRankingHoursSpan = quotaCalculator_->GetTopRankingHoursSpan();
    noUseHoursForCleanAll = quotaCalculator_->GetNoUseHoursForCleanAll();
    if (topRankingHoursSpan <= 0 || noUseHoursForCleanAll <= 0) {
        LOGE("Invalid hours span from config: topRankingHoursSpan=%{public}d, noUseHoursForCleanAll=%{public}d",
            topRankingHoursSpan, noUseHoursForCleanAll);
        return E_FAIL;
    }
    return E_OK;
}

int64_t CacheCleanController::GetCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
int32_t CacheCleanController::QueryAllBundleStats(const BundleStatsQueryParams &params,
    std::vector<BundleActivePackageStats> &topRankingStats, std::vector<BundleActivePackageStats> &activeStats)
{
    int64_t startTimeTopRanking = 0;
    int64_t endTimeTopRanking = 0;
    CalculateTimeRange(params.currentTime, params.topRankingHoursSpan, startTimeTopRanking, endTimeTopRanking);

    int64_t startTimeActive = 0;
    int64_t endTimeActive = 0;
    CalculateTimeRange(params.currentTime, params.noUseHoursForCleanAll, startTimeActive, endTimeActive);

    int32_t ret = QueryBundleActiveStats(params.userId, startTimeTopRanking, endTimeTopRanking, topRankingStats);
    if (ret != E_OK) {
        return ret;
    }

    ret = QueryBundleActiveStats(params.userId, startTimeActive, endTimeActive, activeStats);
    if (ret != E_OK) {
        return ret;
    }

    return E_OK;
}
#endif

int32_t CacheCleanController::GetTopAppCountFromStorage(int32_t &topCount)
{
    int64_t totalStorage = 0;
    int32_t storageRet = DelayedSingleton<StorageSpaceManagerClient>::GetInstance()->GetTotalSize(totalStorage);
    if (storageRet != E_OK) {
        LOGE("Failed to get storage size, ret=%{public}d", storageRet);
        return E_FAIL;
    }

    int32_t ret = quotaCalculator_->GetTopAppCount(totalStorage, topCount);
    if (ret != E_OK) {
        LOGE("Failed to get top app count, storage=%{public}lld GB",
             static_cast<long long>(totalStorage / GB_TO_BYTES));
        return ret;
    }

    return E_OK;
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
void CacheCleanController::BuildCleanCacheInfos(const CleanCacheBuildParams &params,
    std::vector<CleanCacheInfo> &rankedCleanInfos, std::vector<CleanCacheInfo> &cleanAllCacheInfos)
{
    // Remove active apps from appInfos to get unused apps
    std::vector<ApplicationInfo> noUseAppInfos = RemoveAppsByStats(*params.appInfos,
        params.bundleActiveStatsExceptNoUse);
    LOGI("Removed active apps, remaining: %{public}zu", noUseAppInfos.size());

    // Remove top-ranking from active stats
    std::vector<BundleActivePackageStats> filteredActiveStats =
        RemoveBundleStatsByBundleStats(params.bundleActiveStatsExceptNoUse, params.bundleActiveStatsTopRanking);
    LOGI("Active apps excluding top-ranked: %{public}zu", filteredActiveStats.size());

    // Build ranked clean list: top-ranking + active (excluding top)
    ConvertBundleStatsToCleanCacheInfo(params.bundleActiveStatsTopRanking, params.userId, rankedCleanInfos);
    ConvertBundleStatsToCleanCacheInfo(filteredActiveStats, params.userId, rankedCleanInfos);
    LOGI("Built ranked clean list: top=%{public}zu, active=%{public}zu, total=%{public}zu",
         params.bundleActiveStatsTopRanking.size(), filteredActiveStats.size(), rankedCleanInfos.size());

    // Build full clean list: unused apps (cacheThreshold=0)
    ConvertAppInfosToCleanCacheInfo(noUseAppInfos, params.userId, cleanAllCacheInfos);
    LOGI("Built full clean list: %{public}zu apps", noUseAppInfos.size());
}
#endif

int32_t CacheCleanController::SaveCacheCleaningTimestamp(int64_t timestamp)
{
    LOGI("SaveCacheCleaningTimestamp start, timestamp=%{public}lld", static_cast<long long>(timestamp));

    // Build config file path
    std::string configFilePath = std::string(CONFIG_DIR) + "/" + CONFIG_FILE_NAME;

    // Create directory if not exists
    mode_t mode = CONFIG_DIR_MODE;
    int mkdirRet = mkdir(CONFIG_DIR, mode);
    if (mkdirRet != 0 && errno != EEXIST) {
        LOGE("Failed to create directory: %{public}s, errno=%{public}d", CONFIG_DIR, errno);
        return E_IO_ERROR;
    }

    // Create JSON object with timestamp
    nlohmann::json configJson;
    configJson[LAST_CLEAN_CACHE_TIMESTAMP] = timestamp;

    // Write JSON to file
    std::ofstream configFile(configFilePath);
    if (!configFile.is_open()) {
        LOGE("Failed to open config file for writing: %{public}s", configFilePath.c_str());
        return E_IO_ERROR;
    }

    configFile << configJson.dump(JSON_INDENT);  // 4-space indentation for pretty printing
    configFile.close();

    if (configFile.fail()) {
        LOGE("Failed to write config file: %{public}s", configFilePath.c_str());
        return E_IO_ERROR;
    }

    LOGI("Successfully saved cache cleaning timestamp to: %{public}s", configFilePath.c_str());
    return E_OK;
}

int32_t CacheCleanController::GetDefaultQuotaByRank(int32_t appRank, int64_t totalStorage, int32_t &quota)
{
    if (appRank < MIN_RANK) {
        LOGE("Invalid app rank: %{public}d", appRank);
        return E_INVALID_ARGUMENT;
    }
    static nlohmann::json config = nlohmann::json::parse(DEFAULT_QUOTA_CONFIG);
    nlohmann::json topAppConfig = config[TOP_APP_CACHE_CONFIG];
    int64_t totalGB = totalStorage / static_cast<int64_t>(GB_TO_BYTES);
    for (auto it = topAppConfig.begin(); it != topAppConfig.end(); ++it) {
        StorageRangeInfo rangeInfo;
        if (!ParseStorageRangeKey(it.key(), rangeInfo)) {
            continue;
        }
        if (!IsInStorageRange(totalGB, rangeInfo)) {
            continue;
        }
        return GetQuotaFromRangeConfig(it.value(), appRank, quota);
    }
    LOGE("No matching default quota for rank %{public}d", appRank);
    return E_FAIL;
}

void CacheCleanController::SetStopCleanCacheFlag(bool stopCleanCacheFlag)
{
    stopCleanCacheFlag_ = stopCleanCacheFlag;
}
} // namespace StorageSpaceManager
} // namespace OHOS
