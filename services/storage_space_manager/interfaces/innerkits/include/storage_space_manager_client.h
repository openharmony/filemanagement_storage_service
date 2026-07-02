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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CLIENT_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CLIENT_H

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include <iremote_object.h>
#include <nocopyable.h>
#include <refbase.h>
#include <singleton.h>
#include <system_ability_status_change_stub.h>

#include "storage_space_manager_errno.h"

namespace OHOS {
namespace StorageSpaceManager {

class IStorageSpaceManager;

class StorageSpaceManagerClient : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageSpaceManagerClient);

public:
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetSystemSize(int64_t &systemSize);
    int32_t GetFreeSize(int64_t &freeSize);
    int32_t GetTotalInodes(int64_t &totalInodes);
    int32_t GetFreeInodes(int64_t &freeInodes);

    int32_t CleanBundleCache(int32_t userId);

    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();

    int32_t ResetProxy();

private:
    sptr<IStorageSpaceManager> GetProxy();
    int32_t LoadStorageSpaceManagerService();
    int32_t Connect(sptr<IStorageSpaceManager> &proxy);
    void SubscribeSsmSA();
    void OnAddSystemAbility();
    
    class SsmDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        SsmDeathRecipient() = default;
        ~SsmDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

    class SystemAbilityStatusListener : public SystemAbilityStatusChangeStub {
    public:
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    };

private:
    sptr<IStorageSpaceManager> storageSpaceManager_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    sptr<SystemAbilityStatusListener> statusListener_;
    std::mutex mutex_;
    std::condition_variable proxyConVar_;
    bool loadFinished_ = false;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CLIENT_H
