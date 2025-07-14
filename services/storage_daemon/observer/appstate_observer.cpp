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
#ifdef STORAGE_SERVICE_MEDIA_FUSE
#include "observer/appstate_observer.h"

#include "singleton.h"
#include "storage_service_constant.h"
#include "user/mount_manager.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;

constexpr int32_t BASE_USER_RANGE = 200000;

void AppStateObserverManager::SubscribeAppState(const std::vector<std::string> &bundleNameList)
{
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        LOGE("appMgrClient_ is nullptr");
        return;
    }
    if (appStateObserver_ != nullptr) {
        LOGI("appStateObserver has been registed");
        return;
    }
    appStateObserver_ = sptr<AppStateObserver>(new (std::nothrow) AppStateObserver());
    if (appStateObserver_ == nullptr) {
        LOGI("get appStateObserver failed");
        return;
    }

    int32_t result = appMgrClient->RegisterApplicationStateObserver(appStateObserver_, bundleNameList);
    if (result != E_OK) {
        LOGE("RegistApplicationStateObserver failed result = %{public}d", result);
        appStateObserver_ = nullptr;
    }
}

void AppStateObserverManager::UnSubscribeAppState()
{
    if (appStateObserver_ == nullptr) {
        LOGI("appStateObserver is nullptr");
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        LOGE("appMgrClient_ is nullptr");
        return;
    }
    int32_t result = appMgrClient->UnregisterApplicationStateObserver(appStateObserver_);
    if (result != E_OK) {
        LOGE("UnregisterApplicationStateObserver failed result = %{public}d", result);
    }
    appStateObserver_ = nullptr;
}

AppStateObserverManager &AppStateObserverManager::GetInstance()
{
    static AppStateObserverManager instance;
    return instance;
}

void AppStateObserver::OnAppStopped(const AppStateData &appStateData)
{
    if (appStateData.bundleName == MEDIALIBRARY_NAME) {
        LOGI("StorageDaemon OnAppStopped start %{public}d", appStateData.uid);
        int32_t userId = appStateData.uid / BASE_USER_RANGE;
        list<string> killList = { MEDIA_FUSE_EL2 };
        MountManager::GetInstance().FindAndKillProcessWithoutRadar(userId, killList);
    }
}
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_SERVICE_MEDIA_FUSE