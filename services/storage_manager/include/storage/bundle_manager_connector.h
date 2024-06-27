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

#ifndef OHOS_STORAGE_MANAGER_BUNDLE_MANAGER_CONNECTOR_H
#define OHOS_STORAGE_MANAGER_BUNDLE_MANAGER_CONNECTOR_H

#include <nocopyable.h>
#include <singleton.h>
#include <iostream>
#include "bundle_stats.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iremote_object.h"

namespace OHOS {
namespace StorageManager {
class BundleMgrConnector : public NoCopyable  {
    DECLARE_DELAYED_SINGLETON(BundleMgrConnector);

public:
    sptr<AppExecFwk::IBundleMgr> GetBundleMgrProxy();
    int32_t ResetBundleMgrProxy();

private:
    std::mutex mutex_;
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
};

class BundleMgrDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    BundleMgrDeathRecipient() = default;
    virtual ~BundleMgrDeathRecipient() = default;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_BUNDLE_MANAGER_CONNECTOR_H