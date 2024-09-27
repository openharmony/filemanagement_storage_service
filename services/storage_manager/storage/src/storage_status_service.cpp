/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "storage/storage_status_service.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "hap_token_info.h"
#include "hitrace_meter.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_total_status_service.h"
#include "application_info.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils/storage_radar.h"
#include "utils/storage_utils.h"
#ifdef STORAGE_SERVICE_GRAPHIC
#include "datashare_abs_result_set.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#endif

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
using namespace OHOS::StorageService;
static constexpr int32_t GET_DATA_SHARE_HELPER_TIMES = 5;
namespace {
const string MEDIA_TYPE = "media";
const string FILE_TYPE = "file";
const string MEDIALIBRARY_DATA_URI = "datashare:///media";
const string MEDIA_QUERYOPRN_QUERYVOLUME = "query_media_volume";
const int MEDIA_TYPE_IMAGE = 1;
const int MEDIA_TYPE_AUDIO = 3;
const int MEDIA_TYPE_VIDEO = 2;
const int32_t GET_DATA_SHARE_HELPER_TIMES = 5;
} // namespace

StorageStatusService::StorageStatusService() {}
StorageStatusService::~StorageStatusService() {}

#ifdef STORAGE_SERVICE_GRAPHIC
void GetMediaTypeAndSize(const std::shared_ptr<DataShare::DataShareResultSet> &resultSet, StorageStats &storageStats)
{
    if (resultSet == nullptr) {
        LOGE("StorageStatusService::GetMediaTypeAndSize, input resultSet is nullptr.");
        return;
    }
    int thumbnailType = -1;
    while (resultSet->GoToNextRow() == E_OK) {
        int32_t index = 0;
        int mediatype = 0;
        int64_t size = 0;
        if (resultSet->GetColumnIndex("media_type", index) || resultSet->GetInt(index, mediatype)) {
            LOGE("get media_type column index or int value err.");
            continue;
        }
        if (resultSet->GetColumnIndex("size", index) || resultSet->GetLong(index, size)) {
            LOGE("get size column index or long value err.");
            continue;
        }
        LOGI("media_type: %{public}d, size: %{public}lld", mediatype, static_cast<long long>(size));
        if (mediatype == MEDIA_TYPE_IMAGE || mediatype == thumbnailType) {
            storageStats.image_ += size;
        } else if (mediatype == MEDIA_TYPE_AUDIO) {
            storageStats.audio_ = size;
        } else if (mediatype == MEDIA_TYPE_VIDEO) {
            storageStats.video_ = size;
        } else {
            LOGD("unsupprted media_type: %{public}d", mediatype);
        }
    }
}
#endif

int32_t GetMediaStorageStats(StorageStats &storageStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGI("GetMediaStorageStats start");
#ifdef STORAGE_SERVICE_GRAPHIC
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("StorageStatusService::GetMediaStorageStats samgr == nullptr");
        return E_SA_IS_NULLPTR;
    }
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOGE("StorageStatusService::GetMediaStorageStats remoteObj == nullptr");
        return E_REMOTE_IS_NULLPTR;
    }
    int32_t tryCount = 0;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
    while (dataShareHelper == nullptr && tryCount++ < GET_DATA_SHARE_HELPER_TIMES) {
        LOGW("dataShareHelper is retrying");
        auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
    }
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null!");
        return E_MEDIALIBRARY_ERROR;
    }
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI + "/" + MEDIA_QUERYOPRN_QUERYVOLUME + "/" + MEDIA_QUERYOPRN_QUERYVOLUME);
    DataShare::DataSharePredicates predicates;
    auto queryResultSet = dataShareHelper->Query(uri, predicates, columns);
    if (queryResultSet == nullptr) {
        LOGE("queryResultSet is null!");
        return E_MEDIALIBRARY_ERROR;
    }
    auto count = 0;
    auto ret = queryResultSet->GetRowCount(count);
    if ((ret != E_OK) || (count < 0)) {
        LOGE("get row count from rdb failed");
        return E_MEDIALIBRARY_ERROR;
    }
    GetMediaTypeAndSize(queryResultSet, storageStats);
    dataShareHelper->Release();
#endif
    LOGI("GetMediaStorageStats end");
    return E_OK;
}

int32_t GetFileStorageStats(int32_t userId, StorageStats &storageStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGI("GetFileStorageStats start");
    int32_t err = E_OK;
    int32_t prjId = userId * USER_ID_BASE + UID_FILE_MANAGER;
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GetOccupiedSpace(StorageDaemon::USRID, prjId, storageStats.file_);
    LOGI("GetFileStorageStats end");
    return err;
}

int StorageStatusService::GetCurrentUserId()
{
    int uid = -1;
    uid = IPCSkeleton::GetCallingUid();
    int userId = uid / 200000;
    return userId;
}

std::string StorageStatusService::GetCallingPkgName()
{
    uint32_t pid = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::HapTokenInfo tokenInfo = Security::AccessToken::HapTokenInfo();
    Security::AccessToken::AccessTokenKit::GetHapTokenInfo(pid, tokenInfo);
    return tokenInfo.bundleName;
}

int32_t StorageStatusService::GetBundleStats(const std::string &pkgName,
    BundleStats &bundleStats, int32_t appIndex)
{
    int userId = GetCurrentUserId();
    LOGD("StorageStatusService::userId is:%d, appIndex is: %d", userId, appIndex);
    return GetBundleStats(pkgName, userId, bundleStats, appIndex);
}

int32_t StorageStatusService::GetUserStorageStats(StorageStats &storageStats)
{
    int userId = GetCurrentUserId();
    return GetUserStorageStats(userId, storageStats);
}

int32_t StorageStatusService::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    bool isCeEncrypt = false;
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int ret = sdCommunication->GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("User %{public}d de has not decrypt.", userId);
        return ret;
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    // totalSize
    int64_t totalSize = 0;
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize(totalSize);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getTotalSize failed");
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(
            "GetTotalSize", BizScene::SPACE_STATISTICS, BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS, "EL1", err);
        return err;
    }
    // appSize
    LOGI("StorageStatusService::GetUserStorageStats userId is %{public}d", userId);
    int64_t appSize = 0;
    err = GetAppSize(userId, appSize);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getAppSize failed");
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(
            "GetAppSize", BizScene::SPACE_STATISTICS, BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS, "EL1", err);
        return err;
    }

    storageStats.total_ = totalSize;
    storageStats.app_ = appSize;

    // mediaSize
    err = GetMediaStorageStats(storageStats);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getMedia failed");
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(
            "GetMediaStorageStats", BizScene::SPACE_STATISTICS, BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS, "EL1", err);
        return err;
    }
    // fileSize
    err = GetFileStorageStats(userId, storageStats);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats GetFileStorageStats failed");
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(
            "GetFileStorageStats", BizScene::SPACE_STATISTICS, BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS, "EL1", err);
    }
    return err;
}

int32_t StorageStatusService::GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
    const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes,
    std::vector<int64_t> &incPkgFileSizes)
{
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int32_t err = sdCommunication->GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes,
        pkgFileSizes, incPkgFileSizes);
    LOGI("StorageStatusService::GetBundleStatsForIncrease err is %{public}d", err);
    return err;
}

int32_t StorageStatusService::GetCurrentBundleStats(BundleStats &bundleStats)
{
    int userId = GetCurrentUserId();
    LOGI("StorageStatusService::userId is: %{public}d", userId);
    std::string pkgName = GetCallingPkgName();
    int32_t ret = GetBundleStats(pkgName, userId, bundleStats, DEFAULT_APP_INDEX);
    if (ret != E_OK) {
        LOGE("storage status service GetBundleStats failed, please check");
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(
            "GetBundleStats", BizScene::SPACE_STATISTICS, BizStage::BIZ_STAGE_GET_BUNDLE_STATS, "EL1", ret);
    }
    return ret;
}

int32_t StorageStatusService::GetBundleStats(const std::string &pkgName, int32_t userId,
    BundleStats &pkgStats, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto bundleMgr = DelayedSingleton<BundleMgrConnector>::GetInstance()->GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("StorageStatusService::GetBundleStats connect bundlemgr failed");
        return E_SERVICE_IS_NULLPTR;
    }

    if (userId < 0 || userId > StorageService::MAX_USER_ID) {
        LOGE("StorageStatusService::Invaild userId.");
        return E_USERID_RANGE;
    }

    if (appIndex < 0 || appIndex > StorageService::MAX_APP_INDEX) {
        LOGE("StorageStatusService::Invalid appIndex: %{public}d", appIndex);
        return E_USERID_RANGE;
    }
    vector<int64_t> bundleStats;
    bool res = bundleMgr->GetBundleStats(pkgName, userId, bundleStats, appIndex);
    if (!res || bundleStats.size() != dataDir.size()) {
        LOGE("StorageStatusService::An error occurred in querying bundle stats.");
        return E_BUNDLEMGR_ERROR;
    }
    for (uint i = 0; i < bundleStats.size(); i++) {
        if (bundleStats[i] == E_ERR) {
            LOGE("StorageStatusService::Failed to query %{public}s data.", dataDir[i].c_str());
            bundleStats[i] = 0;
        }
    }
    pkgStats.appSize_ = bundleStats[APP];
    pkgStats.cacheSize_ = bundleStats[CACHE];
    pkgStats.dataSize_ = bundleStats[LOCAL] + bundleStats[DISTRIBUTED] + bundleStats[DATABASE];
    return E_OK;
}

int32_t StorageStatusService::GetAppSize(int32_t userId, int64_t &appSize)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGI("StorageStatusService::GetAppSize start");
    auto bundleMgr = DelayedSingleton<BundleMgrConnector>::GetInstance()->GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats connect bundlemgr failed");
        return E_SERVICE_IS_NULLPTR;
    }

    vector<int64_t> bundleStats;
    bool res = bundleMgr->GetAllBundleStats(userId, bundleStats);
    if (!res || bundleStats.size() != dataDir.size()) {
        LOGE("StorageStatusService::GetAllBundleStats fail. res %{public}d, bundleStats.size %{public}zu",
             res, bundleStats.size());
        return E_BUNDLEMGR_ERROR;
    }

    for (uint i = 0; i < bundleStats.size(); i++) {
        appSize += bundleStats[i];
    }
    LOGI("StorageStatusService::GetAppSize end");
    return E_OK;
}

int32_t StorageStatusService::GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, std::string type)
{
    storageStats.video_ = 0;
    storageStats.image_ = 0;
    storageStats.file_ = 0;
    int32_t err = E_OK;
    if (type == MEDIA_TYPE) {
        LOGI("GetUserStorageStatsByType media");
        err = GetMediaStorageStats(storageStats);
    } else if (type == FILE_TYPE) {
        LOGI("GetUserStorageStatsByType file");
        err = GetFileStorageStats(userId, storageStats);
    } else {
        LOGI("GetUserStorageStatsByType type: %{public}s", type.c_str());
    }

    return err;
}
} // StorageManager
} // OHOS
