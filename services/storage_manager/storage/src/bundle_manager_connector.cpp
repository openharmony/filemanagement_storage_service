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

#include "storage/bundle_manager_connector.h"

#include <cstdlib>
#include <cstring>
#include <mntent.h>
#include <singleton.h>
#include <sys/statvfs.h>
#include <unordered_set>

#include "iservice_registry.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace StorageManager {
BundleMgrConnector::BundleMgrConnector()
{
    LOGI("BundleMgrConnector Constructor.");
}

BundleMgrConnector::~BundleMgrConnector()
{
    LOGI("BundleMgrConnector Destructor.");
    bundleMgr_ = nullptr;
    deathRecipient_ = nullptr;
}

sptr<AppExecFwk::IBundleMgr> BundleMgrConnector::GetBundleMgrProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("BundleMgrConnector::GetBundleMgrProxy samgr == nullptr");
            return nullptr;
        }
        sptr<IRemoteObject> remoteObject = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (!remoteObject) {
            LOGE("BundleMgrConnector::GetBundleMgrProxy remoteObj == nullptr");
            return nullptr;
        }
        bundleMgr_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
        if (bundleMgr_ == nullptr) {
            LOGE("BundleMgrConnector::GetBundleMgrProxy bundleMgr == nullptr");
            return nullptr;
        }
        deathRecipient_ = new (std::nothrow) BundleMgrDeathRecipient();
        if (!deathRecipient_) {
            LOGE("BundleMgrConnector::GetBundleMgrProxy failed to create death recipient");
            return nullptr;
        }
        bundleMgr_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    return bundleMgr_;
}

int32_t BundleMgrConnector::ResetBundleMgrProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((bundleMgr_ != nullptr) && (bundleMgr_->AsObject() != nullptr)) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    bundleMgr_ = nullptr;
    return E_OK;
}

void BundleMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DelayedSingleton<BundleMgrConnector>::GetInstance()->ResetBundleMgrProxy();
}
} // StorageManager
} // OHOS
