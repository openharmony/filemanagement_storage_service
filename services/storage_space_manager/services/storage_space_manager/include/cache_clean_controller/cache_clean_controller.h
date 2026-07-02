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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CACHE_CLEAN_CONTROLLER_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CACHE_CLEAN_CONTROLLER_H

#include <mutex>
#include <string>
#include <vector>
#include <sstream>

#include <nocopyable.h>
#include <singleton.h>
#include "application_info.h"
#include "bundle_mgr_interface.h"
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
using OHOS::DeviceUsageStats::BundleActivePackageStats;
#endif
#include "i_quota_calculator.h"
#include <dlfcn.h>

using OHOS::AppExecFwk::ApplicationInfo;
using OHOS::AppExecFwk::CleanCacheInfo;

namespace OHOS {
namespace StorageSpaceManager {

/**
 * @brief Parameters for querying bundle active stats.
 */
struct BundleStatsQueryParams {
    int32_t userId = 0;
    int32_t topRankingHoursSpan = 0;
    int32_t noUseHoursForCleanAll = 0;
    int64_t currentTime = 0;
};

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
/**
 * @brief Parameters for building clean cache infos.
 */
struct CleanCacheBuildParams {
    std::vector<BundleActivePackageStats> bundleActiveStatsTopRanking;
    std::vector<BundleActivePackageStats> bundleActiveStatsExceptNoUse;
    const std::vector<ApplicationInfo> *appInfos = nullptr;
    int32_t userId = 0;
};
#endif

/**
 * @brief Resources for cache cleaning operations (input).
 */
struct CleanResources {
    sptr<AppExecFwk::IBundleMgr> bundleMgr;
    int64_t totalStorage = 0;
    int32_t userId = -1;
};

/**
 * @brief Statistics for cache cleaning operations (output).
 */
struct CleanStats {
    int32_t totalCleanedCount = 0;
    int32_t failedCount = 0;
    int64_t cleanBefore = 0;
    int64_t cleanAfter = 0;
};

/**
 * @brief Controller for managing bundle cache cleaning operations.
 *        This class implements the singleton pattern using DELAYED_SINGLETON.
 */
class CacheCleanController : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(CacheCleanController);
    friend class CacheCleanControllerTest;

public:
    /**
     * @brief Clean cache files for all bundles.
     * @param userId Indicates the user ID.
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t CleanBundleCache(int32_t userId);

    /**
     * @brief Get all application infos.
     * @param userId Indicates the user ID.
     * @param appInfos Output all application infos.
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t GetAllAppInfos(int32_t userId, std::vector<ApplicationInfo> &appInfos);

    /**
     * @brief Rank applications by activity level.
     * @param userId Indicates the user ID.
     * @param appInfos Input application infos to be ranked.
     * @param rankedCleanInfos Output ranked clean cache info.
     * @param cleanAllCacheInfos Output all application infos as clean cache info.
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t BundleActiveRank(int32_t userId, const std::vector<ApplicationInfo> &appInfos,
                             std::vector<CleanCacheInfo> &rankedCleanInfos,
                             std::vector<CleanCacheInfo> &cleanAllCacheInfos);

    /**
     * @brief Filter application infos by bundle type.
     * @param appInfos Input application infos to be filtered.
     * @param filteredAppInfos Output application infos with bundleType equal to BundleType::APP.
     * @return Returns E_OK on success, error code otherwise.
     * @note This method removes applications whose bundleType is not equal to BundleType::APP.
     */
    int32_t FilterAppInfosByBundleType(const std::vector<ApplicationInfo> &appInfos,
                                       std::vector<ApplicationInfo> &filteredAppInfos);

    /**
     * @brief Sort bundle active package stats by totalInFrontTime_ in descending order.
     * @param bundleStats Input bundle active package stats to be sorted.
     * @note This method sorts the input array in-place, with the highest totalInFrontTime_ first.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    void SortBundleStatsByInFrontTime(std::vector<BundleActivePackageStats> &bundleStats);
#endif

    /**
     * @brief Filter bundle active package stats to keep only APP type applications.
     * @param bundleStats Input bundle active package stats to be filtered.
     * @param appInfos Application infos used for filtering.
     * @note This method removes BundleActivePackageStats entries whose bundleName_ and appIndex_
     *       do not match any ApplicationInfo in appInfos.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    void FilterBundleStatsByAppType(std::vector<BundleActivePackageStats> &bundleStats,
                                    const std::vector<ApplicationInfo> &appInfos);
#endif

    /**
     * @brief Remove applications from appInfos based on bundle active stats.
     * @param appInfos Input application infos to be filtered.
     * @param bundleStatsToRemove Bundle active stats whose applications should be removed.
     * @return Filtered application infos.
     * @note This method removes applications from appInfos whose bundleName and appIndex
     *       match any entry in bundleStatsToRemove.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    std::vector<ApplicationInfo> RemoveAppsByStats(const std::vector<ApplicationInfo> &appInfos,
                                                  const std::vector<BundleActivePackageStats> &bundleStatsToRemove);

    /**
     * @brief Remove bundle active stats entries based on another bundle active stats collection.
     * @param bundleStats Input bundle active stats to be filtered.
     * @param bundleStatsToRemove Bundle active stats whose entries should be removed.
     * @return Filtered bundle active stats.
     * @note This method removes entries from bundleStats whose bundleName_ and appIndex_
     *       match any entry in bundleStatsToRemove.
     */
    std::vector<BundleActivePackageStats> RemoveBundleStatsByBundleStats(
        const std::vector<BundleActivePackageStats> &bundleStats,
        const std::vector<BundleActivePackageStats> &bundleStatsToRemove);
#endif

    /**
     * @brief Get configuration and validate hours span.
     * @param topRankingHoursSpan Output top ranking hours span.
     * @param noUseHoursForCleanAll Output no use hours for clean all.
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t GetConfigAndValidate(int32_t &topRankingHoursSpan, int32_t &noUseHoursForCleanAll);

    /**
     * @brief Get current timestamp in milliseconds.
     * @return Current timestamp in milliseconds.
     */
    int64_t GetCurrentTime();

    /**
     * @brief Calculate time range based on current time and hours span.
     * @param currentTime Current timestamp in milliseconds.
     * @param hoursSpan Hours span for calculation.
     * @param startTime Output start time in milliseconds.
     * @param endTime Output end time in milliseconds.
     */
    void CalculateTimeRange(int64_t currentTime, int32_t hoursSpan, int64_t &startTime, int64_t &endTime);

    /**
     * @brief Query bundle active stats for a given time range.
     * @param userId User ID.
     * @param startTime Start time in milliseconds.
     * @param endTime End time in milliseconds.
     * @param bundleStats Output bundle active stats.
     * @return Returns E_OK on success, error code otherwise.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    int32_t QueryBundleActiveStats(int32_t userId, int64_t startTime, int64_t endTime,
                                   std::vector<BundleActivePackageStats> &bundleStats);

    /**
     * @brief Query all bundle active stats for both top ranking and active intervals.
     * @param params Query parameters including userId, hours spans, and current time.
     * @param topRankingStats Output top ranking bundle stats.
     * @param activeStats Output active bundle stats.
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t QueryAllBundleStats(const BundleStatsQueryParams &params,
                                 std::vector<BundleActivePackageStats> &topRankingStats,
                                 std::vector<BundleActivePackageStats> &activeStats);
#endif

    /**
     * @brief Get top app count based on storage space.
     * @param topCount Output top app count.
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t GetTopAppCountFromStorage(int32_t &topCount);

    /**
     * @brief Process bundle active stats: filter, sort, and trim top ranking.
     * @param topRankingStats Top ranking bundle stats to process.
     * @param activeStats Active bundle stats to process.
     * @param appInfos Application infos for filtering.
     * @param userId User ID.
     * @param topCount Top app count to keep.
     * @return Returns E_OK on success, error code otherwise.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    int32_t ProcessBundleActiveStats(std::vector<BundleActivePackageStats> &topRankingStats,
                                     std::vector<BundleActivePackageStats> &activeStats,
                                     const std::vector<ApplicationInfo> &appInfos,
                                     int32_t userId, int32_t topCount);

    /**
     * @brief Convert bundle active stats to clean cache info.
     * @param bundleStats Input bundle active stats.
     * @param userId User ID.
     * @param cleanCacheInfos Output clean cache info.
     */
    void ConvertBundleStatsToCleanCacheInfo(const std::vector<BundleActivePackageStats> &bundleStats,
                                             int32_t userId,
                                             std::vector<CleanCacheInfo> &cleanCacheInfos);
#endif

    /**
     * @brief Convert application infos to clean cache info.
     * @param appInfos Input application infos.
     * @param userId User ID.
     * @param cleanCacheInfos Output clean cache info.
     */
    void ConvertAppInfosToCleanCacheInfo(const std::vector<ApplicationInfo> &appInfos,
                                         int32_t userId,
                                         std::vector<CleanCacheInfo> &cleanCacheInfos);

    /**
     * @brief Build final clean cache infos for ranking and cleaning all.
     * @param params Build parameters including bundle stats and app infos.
     * @param rankedCleanInfos Output ranked clean cache info.
     * @param cleanAllCacheInfos Output all application infos as clean cache info.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    void BuildCleanCacheInfos(const CleanCacheBuildParams &params,
                               std::vector<CleanCacheInfo> &rankedCleanInfos,
                               std::vector<CleanCacheInfo> &cleanAllCacheInfos);
#endif

    /**
     * @brief Clean cache for apps based on rank quota.
     * @param rankedCleanInfos Ranked clean cache info list.
     * @param resources Clean resources (input).
     * @param stats Clean statistics (output).
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t CleanRankBasedCache(const std::vector<CleanCacheInfo> &rankedCleanInfos,
                                 const CleanResources &resources,
                                 CleanStats &stats);

    /**
     * @brief Clean all cache for apps.
     * @param cleanAllCacheInfos Clean cache info list for full cleaning.
     * @param resources Clean resources (input).
     * @param stats Clean statistics (output).
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t CleanAllCacheForApps(const std::vector<CleanCacheInfo> &cleanAllCacheInfos,
                                  const CleanResources &resources,
                                  CleanStats &stats);

    /**
     * @brief Prepare resources for cache cleaning.
     * @param resources Clean resources to initialize (output).
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t PrepareCleanResources(CleanResources &resources);

    /**
     * @brief Clean cache for a single app with rank quota.
     * @param cleanInfo Clean cache info.
     * @param rank Application rank.
     * @param resources Clean resources (input).
     * @param stats Clean statistics (output).
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t CleanSingleAppWithQuota(const CleanCacheInfo &cleanInfo,
                                      int32_t rank,
                                      const CleanResources &resources,
                                      CleanStats &stats);

    /**
     * @brief Perform cache cleaning for a single app.
     * @param cleanInfo Clean cache info with quota threshold.
     * @param resources Clean resources (input).
     * @param stats Clean statistics (output).
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t PerformCacheCleaning(const CleanCacheInfo &cleanInfo,
                                  const CleanResources &resources,
                                  CleanStats &stats);

    /**
     * @brief Query bundle stats for ranking.
     * @param userId User ID.
     * @param topRankingHoursSpan Top ranking hours span.
     * @param noUseHoursForCleanAll No use hours for clean all.
     * @param topRankingStats Output top ranking stats.
     * @param activeStats Output active stats.
     * @return Returns E_OK on success, error code otherwise.
     */
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    int32_t QueryBundleStatsForRanking(int32_t userId,
                                        int32_t topRankingHoursSpan,
                                        int32_t noUseHoursForCleanAll,
                                        std::vector<BundleActivePackageStats> &topRankingStats,
                                        std::vector<BundleActivePackageStats> &activeStats);
#endif

    /**
     * @brief Execute cache cleaning operations.
     * @param rankedCleanInfos Ranked clean cache info list.
     * @param cleanAllCacheInfos Clean cache info list for full cleaning.
     * @param resources Clean resources (input).
     * @param stats Clean statistics (output).
     * @return Returns E_OK on success, error code otherwise.
     */
    int32_t ExecuteCacheCleaning(const std::vector<CleanCacheInfo> &rankedCleanInfos,
                                  const std::vector<CleanCacheInfo> &cleanAllCacheInfos,
                                  const CleanResources &resources,
                                  CleanStats &stats);

    /**
     * @brief Generate JSON config file to save cache cleaning timestamp.
     * @param timestamp Cache cleaning execution timestamp in milliseconds.
     * @return Returns E_OK on success, error code otherwise.
     * @note The config file will be saved to /data/service/el1/public/storage_space_manager
     */
    int32_t SaveCacheCleaningTimestamp(int64_t timestamp);

    void SetStopCleanCacheFlag(bool stopCleanCacheFlag);

private:
    void PrintOverLongLog(std::string str);
    void WriteCleanInfoToExtra(const CleanCacheInfo &cleanInfo, uint64_t before, uint64_t after, bool isSucc);
    int32_t ExecuteCleanBundleCache(int32_t userId);
    int32_t GetDefaultQuotaByRank(int32_t appRank, int64_t totalStorage, int32_t &quota);

    bool LoadQuotaCalculator();
    bool quotaCalculatorSoLoaded_ = false;
    void *quotaCalculatorSoHandle_ = nullptr;
    std::shared_ptr<IQuotaCalculator> quotaCalculator_;
    std::atomic<bool> stopCleanCacheFlag_{false};  // Stop clean cache flag
    std::atomic<bool> isCleanRunning_{false};       // Scan running flag
    std::ostringstream extraData_;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CACHE_CLEAN_CONTROLLER_H
