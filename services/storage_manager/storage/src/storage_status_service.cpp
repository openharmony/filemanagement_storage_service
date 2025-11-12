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
#include "os_account_manager.h"
#include "ipc_skeleton.h"
#include "hitrace_meter.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_total_status_service.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
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

namespace {
const string MEDIA_TYPE = "media";
const string FILE_TYPE = "file";
const string MEDIALIBRARY_DATA_URI = "datashare:///media";
const string MEDIA_QUERYOPRN_QUERYVOLUME = "query_media_volume";
constexpr const char *SETTING_BUNDLE_NAME = "settings";
static std::atomic<bool> checkDirSizeFlag = false;
constexpr int DECIMAL_PLACE = 2;
constexpr double TO_MB = 1000.0 * 1000.0;
const int64_t MAX_INT64 = std::numeric_limits<int64_t>::max();
constexpr int64_t CHANGE_RANGE = 1024LL * 1024 * 1024 * 2;
#ifdef STORAGE_SERVICE_GRAPHIC
const int MEDIA_TYPE_IMAGE = 1;
const int MEDIA_TYPE_AUDIO = 3;
const int MEDIA_TYPE_VIDEO = 2;
const int32_t GET_DATA_SHARE_HELPER_TIMES = 5;
#endif
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
        LOGI("media type: %{public}d, size: %{public}lld", mediatype, static_cast<long long>(size));
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
    LOGE("GetMediaStorageStats start");
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
    int32_t tryCount = 1;
    LOGE("GetMediaStorageStats start Creator");
    auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
    while (dataShareHelper == nullptr && tryCount < GET_DATA_SHARE_HELPER_TIMES) {
        LOGW("dataShareHelper is retrying, attempt %{public}d", tryCount);
        dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
        tryCount++;
    }
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null!");
        return E_MEDIALIBRARY_ERROR;
    }
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI + "/" + MEDIA_QUERYOPRN_QUERYVOLUME + "/" + MEDIA_QUERYOPRN_QUERYVOLUME);
    DataShare::DataSharePredicates predicates;
    LOGE("GetMediaStorageStats start Query");
    auto queryResultSet = dataShareHelper->Query(uri, predicates, columns);
    if (queryResultSet == nullptr) {
        LOGE("queryResultSet is null!");
        return E_QUERY;
    }
    auto count = 0;
    auto ret = queryResultSet->GetRowCount(count);
    if ((ret != E_OK) || (count < 0)) {
        LOGE("get row count from rdb failed");
        return E_GETROWCOUNT;
    }
    GetMediaTypeAndSize(queryResultSet, storageStats);
    dataShareHelper->Release();
#endif
    LOGE("GetMediaStorageStats end");
    return E_OK;
}

int32_t GetFileStorageStats(int32_t userId, StorageStats &storageStats)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGE("GetFileStorageStats start");
    int32_t err = E_OK;
    int32_t prjId = userId * USER_ID_BASE + UID_FILE_MANAGER;
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    err = sdCommunication->GetOccupiedSpace(StorageDaemon::USRID, prjId, storageStats.file_);
    LOGE("GetFileStorageStats end");
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
    BundleStats &bundleStats, int32_t appIndex, uint32_t statFlag)
{
    int userId = GetCurrentUserId();
    return GetBundleStats(pkgName, userId, bundleStats, appIndex, statFlag);
}

int32_t StorageStatusService::GetUserStorageStats(StorageStats &storageStats)
{
    int userId = GetCurrentUserId();
    return GetUserStorageStats(userId, storageStats);
}

int32_t StorageStatusService::GetUserStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule)
{
    bool isCeEncrypt = false;
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    int ret = sdCommunication->GetFileEncryptStatus(userId, isCeEncrypt, true);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("User %{public}d de has not decrypt.", userId);
        return ret;
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    // totalSize
    int64_t totalSize = 0;
    int32_t err = StorageTotalStatusService::GetInstance().GetTotalSize(totalSize);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getTotalSize failed");
        StorageRadar::ReportGetStorageStatus("GetUserStorageStats::GetTotalSize", userId, err, GetCallingPkgName());
        return err;
    }
    // appSize
    LOGD("StorageStatusService::GetUserStorageStats userId is %{public}d", userId);
    int64_t appSize = 0;
    err = GetAppSize(userId, appSize);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getAppSize failed");
        StorageRadar::ReportGetStorageStatus("GetUserStorageStats::GetAppSize", userId, err, GetCallingPkgName());
        return err;
    }

    storageStats.total_ = totalSize;
    storageStats.app_ = appSize;

    err = GetMediaAndFileStorageStats(userId, storageStats, isSchedule);

    LOGE("StorageStatusService::GetUserStorageStats success for userId=%{public}d, "
        "totalSize=%{public}lld, appSize=%{public}lld, videoSize=%{public}lld, audioSize=%{public}lld, "
        "imageSize=%{public}lld, fileSize=%{public}lld",
        userId, static_cast<long long>(storageStats.total_), static_cast<long long>(storageStats.app_),
        static_cast<long long>(storageStats.video_), static_cast<long long>(storageStats.audio_),
        static_cast<long long>(storageStats.image_), static_cast<long long>(storageStats.file_));
    return err;
}

int32_t StorageStatusService::GetMediaAndFileStorageStats(int32_t userId, StorageStats &storageStats, bool isSchedule)
{
    // mediaSize
    auto err = GetMediaStorageStats(storageStats);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats GetMediaStorageStats failed");
        StorageRadar::ReportGetStorageStatus("GetUserStorageStats::GetMediaStorageStats", userId, err,
            GetCallingPkgName());
        return err;
    }
    // fileSize
    err = GetFileStorageStats(userId, storageStats);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats GetFileStorageStats failed");
        StorageRadar::ReportGetStorageStatus("GetUserStorageStats::GetFileStorageStats", userId, err,
            GetCallingPkgName());
        return err;
    }
    auto ret = QueryOccupiedSpaceForSa(storageStats, userId, isSchedule);
    if (ret != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats QueryOccupiedSpaceForSa failed, err: %{public}d", ret);
        StorageRadar::ReportGetStorageStatus("GetUserStorageStats::QueryOccupiedSpaceForSa", userId, ret,
            GetCallingPkgName());
    }
    return err;
}

std::string StorageStatusService::ConvertBytesToMB(int64_t bytes)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(DECIMAL_PLACE) << static_cast<double>(bytes) / TO_MB << "MB";
    return oss.str();
}

int32_t StorageStatusService::QueryOccupiedSpaceForSa(StorageStats &storageStats, int32_t userId, bool isSchedule)
{
    std::string bundleName;
    int32_t uid = IPCSkeleton::GetCallingUid();
    // 检查权限
    int32_t errNo = CheckBundlePermissions(uid, bundleName, isSchedule);
    if (errNo != E_OK) {
        return errNo;
    }
    // 检查是否正在运行任务
    if (checkDirSizeFlag.load()) {
        LOGI("The task to query SA space usage is running, ignore");
        return E_OK;
    }
    // 启动线程处理
    std::thread([this, storageStats, userId, isSchedule]() {
        ProcessStorageStatus(storageStats, userId, isSchedule);
    }).detach();
    return E_OK;
}

void StorageStatusService::ProcessStorageStatus(const StorageStats &storageStats, int32_t userId, bool isSchedule)
{
    checkDirSizeFlag.store(true);
    int64_t freeSize = 0;
    int64_t systemSize = 0;
    int32_t errNo = StorageTotalStatusService::GetInstance().GetFreeSize(freeSize);
    if (errNo != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getFreeSize failed, errNo:%{public}d.", errNo);
    }
    errNo = StorageTotalStatusService::GetInstance().GetSystemSize(systemSize);
    if (errNo != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getSysSize failed, errNo:%{public}d.", errNo);
    }
    int64_t changeSize = freeSize - oldFreeSizeCache;
    if (abs(changeSize) < CHANGE_RANGE && isSchedule) {
        LOGI("free size changed less than 2G, do not report storage");
        checkDirSizeFlag.store(false);
        return;
    }
    oldFreeSizeCache = freeSize;
    std::map<int32_t, std::string> bundleNameAndUid;
    errNo = GetBundleNameAndUid(userId, bundleNameAndUid);
    if (errNo != E_OK) {
        LOGE("GetAllBundleNames failed, errNo:%{public}d.", errNo);
        std::string extraData = "errNo is :" + std::to_string(errNo);
        StorageRadar::ReportCommonResult("QueryOccupiedSpaceForSa::GetBundleNameAndUid",
            E_GET_BUNDLE_NAME_FAILED, userId, extraData);
    }
    std::string storageStatusResult = "{userId is:" + std::to_string(userId) + "}\n" +
                                     "{app size is:" + ConvertBytesToMB(storageStats.app_) +
                                     ",audio size is:" + ConvertBytesToMB(storageStats.audio_) +
                                     ",image size is:" + ConvertBytesToMB(storageStats.image_) +
                                     ",video size is:" + ConvertBytesToMB(storageStats.video_) +
                                     ",file size is:" + ConvertBytesToMB(storageStats.file_) +
                                     ",total size is:" + std::to_string(storageStats.total_) + "B" +
                                     ",sys size is:" + ConvertBytesToMB(systemSize) +
                                     ",free size is:" + ConvertBytesToMB(freeSize) + "}";
    auto sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("get sdCommunication error");
        checkDirSizeFlag.store(false);
        return;
    }
    sdCommunication->QueryOccupiedSpaceForSa(storageStatusResult, bundleNameAndUid);
    checkDirSizeFlag.store(false);
}

int32_t StorageStatusService::CheckBundlePermissions(int32_t uid, std::string &bundleName, bool isSchedule)
{
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return E_SERVICE_IS_NULLPTR;
    }
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName) && !isSchedule) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return E_BUNDLEMGR_ERROR;
    }
    if (bundleName != SETTING_BUNDLE_NAME && !isSchedule) {
        LOGE("permissionCheck error");
        return E_PERMISSION_DENIED;
    }
    return E_OK;
}

int32_t StorageStatusService::GetBundleNameAndUid(int32_t userId, std::map<int32_t, std::string> &bundleNameAndUid)
{
    LOGI("begin getBundleNameAndUid.");
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed, userId:%{public}d.", userId);
        return E_SERVICE_IS_NULLPTR;
    }
    std::vector<std::string> bundleNames;
    std::vector<AccountSA::OsAccountInfo> OsAccountInfo;
    int32_t errNo = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(OsAccountInfo);
    if (errNo != ERR_OK || OsAccountInfo.empty()) {
        LOGE("Query active userid failed, ret = %{public}u", errNo);
        StorageRadar::ReportOsAccountResult("GetBundleNameAndUid::QueryActiveOsAccountIds", errNo, DEFAULT_USERID);
    }
    for (const auto &info : OsAccountInfo) {
        std::vector<AppExecFwk::BundleInfo> bundleInfos;
        auto ret = bundleMgr->GetBundleInfosV9(0, bundleInfos, info.GetLocalId());
        if (ret != E_OK) {
            LOGE("GetBundleInfosV9 failed, ret is %{public}d", ret);
            return ret;
        }
        for (const auto &bundInfo : bundleInfos) {
            bundleNameAndUid[bundInfo.uid] = bundInfo.name;
        }
    }
    LOGI("end getBundleNameAndUid.");
    return E_OK;
}

int32_t StorageStatusService::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
    int userId = GetCurrentUserId();
    std::string pkgName = GetCallingPkgName();
    int32_t ret = GetBundleStats(pkgName, userId, bundleStats, DEFAULT_APP_INDEX, statFlag);
    if (ret != E_OK) {
        LOGE("storage status service GetBundleStats failed, please check");
        std::string extraData = "pkgName=" + pkgName + ",statFlag=" + std::to_string(statFlag);
        StorageRadar::ReportBundleMgrResult("GetCurrentBundleStats::GetBundleStats", ret, userId, extraData);
    }
    return ret;
}

int32_t StorageStatusService::GetBundleStats(const std::string &pkgName, int32_t userId,
    BundleStats &pkgStats, int32_t appIndex, uint32_t statFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
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
        return E_APPINDEX_RANGE;
    }
    vector<int64_t> bundleStats;
    bool res = bundleMgr->GetBundleStats(pkgName, userId, bundleStats, appIndex, statFlag);
    if (!res || bundleStats.size() != dataDir.size()) {
        LOGE("StorageStatusService::An error occurred in querying bundle stats.");
        std::string extraData = "bundleStats size=" + std::to_string(bundleStats.size())
            + ", dataDir size=" + std::to_string(dataDir.size());
        StorageRadar::ReportBundleMgrResult("GetBundleStats", res, userId, extraData);
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
    LOGE("StorageStatusService::GetBundleStats success for pkgName=%{public}s, userId=%{public}d, appIndex=%{public}d"
        ", appSize=%{public}lld, cacheSize=%{public}lld, dataSize=%{public}lld",
        pkgName.c_str(), userId, appIndex, static_cast<long long>(pkgStats.appSize_),
        static_cast<long long>(pkgStats.cacheSize_), static_cast<long long>(pkgStats.dataSize_));
    return E_OK;
}

int32_t StorageStatusService::GetAppSize(int32_t userId, int64_t &appSize)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGD("StorageStatusService::GetAppSize start");
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats connect bundlemgr failed");
        return E_SERVICE_IS_NULLPTR;
    }

    vector<int64_t> bundleStats;
    bool res = bundleMgr->GetAllBundleStats(userId, bundleStats);
    if (!res || bundleStats.size() != dataDir.size()) {
        LOGE("StorageStatusService::GetAllBundleStats fail. res %{public}d, bundleStats.size %{public}zu",
             res, bundleStats.size());
        std::string extraData = "bundleStats size=" + std::to_string(bundleStats.size())
            + ", dataDir size=" + std::to_string(dataDir.size());
        StorageRadar::ReportBundleMgrResult("GetAppSize::GetAllBundleStats", res, userId, extraData);
        return E_BUNDLEMGR_ERROR;
    }

    for (uint i = 0; i < bundleStats.size(); i++) {
        if (bundleStats[i] > 0 && appSize > MAX_INT64 - bundleStats[i]) {
            return E_CALCULATE_OVERFLOW_UP;
        }
        appSize += bundleStats[i];
    }
    LOGD("StorageStatusService::GetAppSize end");
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
        if (err != E_OK) {
            StorageRadar::ReportGetStorageStatus("GetMediaStorageStats", userId, err, "setting");
        }
    } else if (type == FILE_TYPE) {
        LOGI("GetUserStorageStatsByType file");
        err = GetFileStorageStats(userId, storageStats);
        if (err != E_OK) {
            StorageRadar::ReportGetStorageStatus("GetFileStorageStats", userId, err, "setting");
        }
    } else {
        LOGI("GetUserStorageStatsByType type: %{public}s", type.c_str());
    }

    return err;
}
} // StorageManager
} // OHOS
