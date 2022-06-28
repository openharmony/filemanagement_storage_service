/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage/storage_total_status_service.h"
#include "installd_client.h"
#include "bundle_mgr_interface.h"
#include "application_info.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#ifdef STORAGE_SERVICE_GRAPHIC
#include "media_library_manager.h"
#include "media_volume.h"
#endif

using namespace std;

namespace OHOS {
namespace StorageManager {
StorageStatusService::StorageStatusService() {}
StorageStatusService::~StorageStatusService() {}

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

BundleStats StorageStatusService::GetBundleStats(std::string pkgName)
{
    BundleStats result;
    int userId = GetCurrentUserId();
    LOGD("StorageStatusService::userId is:%d", userId);
    if (userId < 0 || userId > StorageService::MAX_USER_ID) {
        LOGE("StorageStatusService::Invaild userId.");
        return result;
    }
    return GetBundleStats(pkgName, userId);
}

StorageStats StorageStatusService::GetUserStorageStats()
{
    int userId = GetCurrentUserId();
    return GetUserStorageStats(userId);
}

StorageStats StorageStatusService::GetUserStorageStats(int32_t userId)
{
    StorageStats result;
    // totalSize
    int64_t totalSize = 0;
    totalSize = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize();
    // appSize
    LOGI("StorageStatusService::GetUserStorageStats userId is %{public}d", userId);
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats samgr == nullptr");
        return result;
    }

    sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        LOGE("StorageStatusService::GetUserStorageStats remoteObj == nullptr");
        return result;
    }

    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats bundleMgr == nullptr");
        return result;
    }
    vector<AppExecFwk::ApplicationInfo> appInfos;
    bool res = bundleMgr->GetApplicationInfos(
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfos);
    if (!res) {
        LOGE("StorageStatusService::GetUserStorageStats an error occured in querying appInfos");
        return result;
    }
    int64_t appSize = 0;
    for (auto appInfo : appInfos) {
        int64_t bundleSize = 0;
        LOGD("StorageStatusService::GetCurUserStorageStats pkgname is %{public}s", appInfo.name.c_str());
        vector<int64_t> bundleStats;
        res = bundleMgr->GetBundleStats(appInfo.name, userId, bundleStats);
        if (!res || bundleStats.size() != dataDir.size()) {
            LOGE("StorageStatusService::An error occurred in querying bundle stats.");
            return result;
        }
        for (uint i = 0; i < bundleStats.size(); i++) {
            bundleSize += bundleStats[i];
        }
        appSize += bundleSize;
    }
    // mediaSize
#ifdef STORAGE_SERVICE_GRAPHIC
    Media::MediaLibraryManager mgr;
    Media::MediaVolume mediaVol;
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats remoteObj == nullptr");
        return result;
    }
    mgr.InitMediaLibraryManager(remoteObj);
    if (mgr.QueryTotalSize(mediaVol)) {
        LOGE("StorageStatusService::GetUserStorageStats an error occured in querying mediaSize");
        return result;
    }
#endif
    result.total_ = totalSize;
    result.app_ = appSize;
#ifdef STORAGE_SERVICE_GRAPHIC
    result.audio_ = mediaVol.GetAudiosSize();
    result.video_ = mediaVol.GetVideosSize();
    result.image_ = mediaVol.GetImagesSize();
    result.file_ = mediaVol.GetFilesSize();
#endif
    return result;
}

BundleStats StorageStatusService::GetCurrentBundleStats()
{
    BundleStats result;
    int userId = GetCurrentUserId();
    LOGD("StorageStatusService::userId is:%d", userId);
    if (userId < 0 || userId > StorageService::MAX_USER_ID) {
        LOGE("StorageStatusService::Invaild userId.");
        return result;
    }
    std::string pkgName = GetCallingPkgName();
    LOGD("StorageStatusService::pkgName is %{public}s", pkgName.c_str());
    return GetBundleStats(pkgName, userId);
}

BundleStats StorageStatusService::GetBundleStats(const std::string &pkgName, int32_t userId)
{
    BundleStats result;
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("StorageStatusService::GetBundleStats samgr == nullptr");
        return result;
    }

    sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        LOGE("StorageStatusService::GetBundleStats remoteObj == nullptr");
        return result;
    }

    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats bundleMgr == nullptr");
        return result;
    }
    vector<int64_t> bundleStats;
    bool res = bundleMgr->GetBundleStats(pkgName, userId, bundleStats);
    if (!res || bundleStats.size() != dataDir.size()) {
        LOGE("StorageStatusService::An error occurred in querying bundle stats.");
        return result;
    }
    for (uint i = 0; i < bundleStats.size(); i++) {
        if (bundleStats[i] == E_ERR) {
            LOGE("StorageStatusService::Failed to query %s data.", dataDir[i].c_str());
            bundleStats[i] = 0;
        }
    }
    result.appSize_ = bundleStats[APP];
    result.cacheSize_ = bundleStats[CACHE];
    result.dataSize_ = bundleStats[LOCAL] + bundleStats[DISTRIBUTED] + bundleStats[DATABASE];
    return result;
}
} // StorageManager
} // OHOS
