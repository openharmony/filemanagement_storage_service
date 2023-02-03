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
#include "bundle_mgr_proxy.h"
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

int32_t StorageStatusService::GetBundleStats(std::string pkgName, BundleStats &bundleStats)
{
    int userId = GetCurrentUserId();
    LOGD("StorageStatusService::userId is:%d", userId);
    return GetBundleStats(pkgName, userId, bundleStats);
}

int32_t StorageStatusService::GetUserStorageStats(StorageStats &storageStats)
{
    int userId = GetCurrentUserId();
    return GetUserStorageStats(userId, storageStats);
}

int32_t StorageStatusService::GetUserStorageStats(int32_t userId, StorageStats &storageStats)
{
    // totalSize
    int64_t totalSize;
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize(totalSize);
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats getTotalSize failed");
        return err;
    }
    // appSize
    LOGI("StorageStatusService::GetUserStorageStats userId is %{public}d", userId);
    err = ConnectBundleMgr();
    if (err != E_OK) {
        LOGE("StorageStatusService::GetUserStorageStats connect bundlemgr failed");
        return err;
    }
    vector<AppExecFwk::ApplicationInfo> appInfos;
    bool res = bundleMgr_->GetApplicationInfos(
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfos);
    if (!res) {
        LOGE("StorageStatusService::GetUserStorageStats an error occured in querying appInfos");
        return E_BUNDLEMGR_ERROR;
    }
    int64_t appSize = 0;
    for (auto appInfo : appInfos) {
        int64_t bundleSize = 0;
        LOGD("StorageStatusService::GetCurUserStorageStats pkgname is %{public}s", appInfo.name.c_str());
        vector<int64_t> bundleStats;
        res = bundleMgr_->GetBundleStats(appInfo.name, userId, bundleStats);
        if (!res || bundleStats.size() != dataDir.size()) {
            LOGE("StorageStatusService::An error occurred in querying bundle stats.");
            return E_BUNDLEMGR_ERROR;
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
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats samgr == nullptr");
        return E_SA_IS_NULLPTR;
    }
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOGE("StorageStatusService::GetUserStorageStats remoteObj == nullptr");
        return E_REMOTE_IS_NULLPTR;
    }
    mgr.InitMediaLibraryManager(remoteObj);
    if (mgr.QueryTotalSize(mediaVol)) {
        LOGE("StorageStatusService::GetUserStorageStats an error occured in querying mediaSize");
        return E_MEDIALIBRARY_ERROR;
    }
#endif
    storageStats.total_ = totalSize;
    storageStats.app_ = appSize;
#ifdef STORAGE_SERVICE_GRAPHIC
    storageStats.audio_ = mediaVol.GetAudiosSize();
    storageStats.video_ = mediaVol.GetVideosSize();
    storageStats.image_ = mediaVol.GetImagesSize();
    storageStats.file_ = mediaVol.GetFilesSize();
#endif
    return E_OK;
}

int32_t StorageStatusService::GetCurrentBundleStats(BundleStats &bundleStats)
{
    int userId = GetCurrentUserId();
    LOGD("StorageStatusService::userId is:%d", userId);
    std::string pkgName = GetCallingPkgName();
    return GetBundleStats(pkgName, userId, bundleStats);
}

int32_t StorageStatusService::GetBundleStats(const std::string &pkgName, int32_t userId, BundleStats &pkgStats)
{
    int32_t err = ConnectBundleMgr();
    if (err != E_OK) {
        LOGE("StorageStatusService::GetBundleStats connect bundlemgr failed");
        return err;
    }

    if (userId < 0 || userId > StorageService::MAX_USER_ID) {
        LOGE("StorageStatusService::Invaild userId.");
        return E_USERID_RANGE;
    }

    vector<int64_t> bundleStats;
    bool res = bundleMgr_->GetBundleStats(pkgName, userId, bundleStats);
    if (!res || bundleStats.size() != dataDir.size()) {
        LOGE("StorageStatusService::An error occurred in querying bundle stats.");
        return E_BUNDLEMGR_ERROR;
    }
    for (uint i = 0; i < bundleStats.size(); i++) {
        if (bundleStats[i] == E_ERR) {
            LOGE("StorageStatusService::Failed to query %s data.", dataDir[i].c_str());
            bundleStats[i] = 0;
        }
    }
    pkgStats.appSize_ = bundleStats[APP];
    pkgStats.cacheSize_ = bundleStats[CACHE];
    pkgStats.dataSize_ = bundleStats[LOCAL] + bundleStats[DISTRIBUTED] + bundleStats[DATABASE];
    return E_OK;
}

int32_t StorageStatusService::ConnectBundleMgr()
{
    LOGI("connect begin");
    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("StorageStatusService::ConnectBundleMgr samgr == nullptr");
            return E_SA_IS_NULLPTR;
        }

        sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (!remoteObject) {
            LOGE("StorageStatusService::ConnectBundleMgr remoteObj == nullptr");
            return E_REMOTE_IS_NULLPTR;
        }

        bundleMgr_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
        if (bundleMgr_ == nullptr) {
            LOGE("StorageStatusService::ConnectBundleMgr bundleMgr == nullptr");
            return E_SERVICE_IS_NULLPTR;
        }

        deathRecipient_ = new (std::nothrow) BundleMgrDeathRecipient();
        if (!deathRecipient_) {
            LOGE("StorageStatusService::ConnectBundleMgr failed to create death recipient");
            return E_DEATH_RECIPIENT_IS_NULLPTR;
        }

        bundleMgr_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    LOGI("connect end");
    return E_OK;
}

int32_t StorageStatusService::ResetBundleMgrProxy()
{
    LOGD("enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((bundleMgr_ != nullptr) && (bundleMgr_->AsObject() != nullptr)) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    bundleMgr_ = nullptr;

    return E_OK;
}

void BundleMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DelayedSingleton<StorageStatusService>::GetInstance()->ResetBundleMgrProxy();
}
} // StorageManager
} // OHOS
