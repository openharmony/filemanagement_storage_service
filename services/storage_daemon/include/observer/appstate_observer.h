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

#ifndef APPSTATE_OBSERVER_H
#define APPSTATE_OBSERVER_H
#ifdef STORAGE_SERVICE_MEDIA_FUSE
#include "app_mgr_client.h"
#include "application_state_observer_stub.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace OHOS::AppExecFwk;
class AppStateObserverManager {
    public:
        AppStateObserverManager() = default;
        ~AppStateObserverManager() = default;
        static AppStateObserverManager &GetInstance();
        void SubscribeAppState(const std::vector<std::string> &bundleNameList);
        void UnSubscribeAppState();

    protected:
        sptr<AppExecFwk::ApplicationStateObserverStub> appStateObserver_ = nullptr;
};

class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        AppStateObserver() {};
        ~AppStateObserver() override = default;

        void OnAppStopped(const AppExecFwk::AppStateData &appStateData) override;
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_SERVICE_MEDIA_FUSE
#endif // AppStateObserverManager