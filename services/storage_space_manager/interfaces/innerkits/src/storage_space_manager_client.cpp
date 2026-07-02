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

#include "storage_space_manager_client.h"

#include <chrono>

#include <errors.h>
#include <iremote_broker.h>
#include <iservice_registry.h>

#include "callback/storage_space_manager_load_callback.h"

#define LOG_DOMAIN 0xD004302

#include "istorage_space_manager.h"
#include "storage_space_manager_hilog.h"
#include "storage_space_manager_errno.h"

namespace OHOS {
namespace StorageSpaceManager {

constexpr int32_t STORAGE_SPACE_MANAGER_SA_ID = 8650;
constexpr int32_t LOAD_SA_TIMEOUT_MS = 10000;

StorageSpaceManagerClient::StorageSpaceManagerClient() = default;

StorageSpaceManagerClient::~StorageSpaceManagerClient()
{
    ResetProxy();
}

sptr<IStorageSpaceManager> StorageSpaceManagerClient::GetProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageSpaceManager_ != nullptr) {
        return storageSpaceManager_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("GetProxy samgr is nullptr");
        return nullptr;
    }
    auto object = samgr->CheckSystemAbility(STORAGE_SPACE_MANAGER_SA_ID);
    if (object == nullptr) {
        return nullptr;
    }
    storageSpaceManager_ = iface_cast<IStorageSpaceManager>(object);
    if (storageSpaceManager_ == nullptr) {
        LOGE("GetProxy iface_cast failed");
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) SsmDeathRecipient();
    }
    sptr<IRemoteObject> remote = storageSpaceManager_->AsObject();
    if (remote != nullptr && deathRecipient_ != nullptr) {
        remote->AddDeathRecipient(deathRecipient_);
    }
    LOGI("GetProxy reuse running SA");
    return storageSpaceManager_;
}

int32_t StorageSpaceManagerClient::LoadStorageSpaceManagerService()
{
    sptr<StorageSpaceManagerLoadCallback> loadCallback =
        new (std::nothrow) StorageSpaceManagerLoadCallback();
    if (loadCallback == nullptr) {
        LOGE("LoadService new loadCallback failed");
        return E_SERVICE_IS_NULLPTR;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("LoadService samgr is nullptr");
        return E_SA_IS_NULLPTR;
    }
    int32_t ret = samgr->LoadSystemAbility(STORAGE_SPACE_MANAGER_SA_ID, loadCallback);
    if (ret != ERR_OK) {
        LOGE("LoadSystemAbility failed, ret=%{public}d", ret);
        return E_REMOTE_IS_NULLPTR;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    if (storageSpaceManager_ != nullptr) {
        LOGI("LoadService success");
        return E_OK;
    }
    bool ready = proxyConVar_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [this]() { return loadFinished_; });
    loadFinished_ = false;
    if (!ready || storageSpaceManager_ == nullptr) {
        LOGE("LoadService wait proxy timeout or null");
        return E_SERVICE_IS_NULLPTR;
    }
    LOGI("LoadService success");
    return E_OK;
}

void StorageSpaceManagerClient::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    LOGI("LoadSystemAbilitySuccess");
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObject != nullptr) {
        if (deathRecipient_ == nullptr) {
            deathRecipient_ = new (std::nothrow) SsmDeathRecipient();
        }
        remoteObject->AddDeathRecipient(deathRecipient_);
        storageSpaceManager_ = iface_cast<IStorageSpaceManager>(remoteObject);
    }
    loadFinished_ = true;
    proxyConVar_.notify_one();
}

void StorageSpaceManagerClient::LoadSystemAbilityFail()
{
    LOGE("LoadSystemAbilityFail");
    std::lock_guard<std::mutex> lock(mutex_);
    storageSpaceManager_ = nullptr;
    loadFinished_ = true;
    proxyConVar_.notify_one();
}

void StorageSpaceManagerClient::SsmDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    (void)remote;
    LOGW("StorageSpaceManager SA died, resetting proxy");
    DelayedSingleton<StorageSpaceManagerClient>::GetInstance()->ResetProxy();
}

int32_t StorageSpaceManagerClient::ResetProxy()
{
    LOGI("ResetProxy");
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageSpaceManager_ != nullptr && deathRecipient_ != nullptr) {
        sptr<IRemoteObject> remote = storageSpaceManager_->AsObject();
        if (remote != nullptr) {
            remote->RemoveDeathRecipient(deathRecipient_);
        }
    }
    storageSpaceManager_ = nullptr;
    deathRecipient_ = nullptr;
    return E_OK;
}

void StorageSpaceManagerClient::SubscribeSsmSA()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (statusListener_ != nullptr) {
        return;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("SubscribeSsmSA samgr is nullptr");
        return;
    }
    statusListener_ = new (std::nothrow) SystemAbilityStatusListener();
    if (statusListener_ == nullptr) {
        LOGE("SubscribeSsmSA new listener failed");
        return;
    }
    int32_t ret = samgr->SubscribeSystemAbility(STORAGE_SPACE_MANAGER_SA_ID, statusListener_);
    if (ret != ERR_OK) {
        LOGE("SubscribeSystemAbility failed, ret=%{public}d", ret);
        statusListener_ = nullptr;
        return;
    }
    LOGI("SubscribeSsmSA success");
}

void StorageSpaceManagerClient::OnAddSystemAbility()
{
    LOGI("SA restarted, clear proxy cache for reconnect");
    std::lock_guard<std::mutex> lock(mutex_);
    storageSpaceManager_ = nullptr;
    deathRecipient_ = nullptr;
    loadFinished_ = false;
}

void StorageSpaceManagerClient::SystemAbilityStatusListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility, systemAbilityId=%{public}d", systemAbilityId);
    DelayedSingleton<StorageSpaceManagerClient>::GetInstance()->OnAddSystemAbility();
}

void StorageSpaceManagerClient::SystemAbilityStatusListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    LOGD("OnRemoveSystemAbility, systemAbilityId=%{public}d", systemAbilityId);
}

int32_t StorageSpaceManagerClient::Connect(sptr<IStorageSpaceManager> &proxy)
{
    SubscribeSsmSA();
    sptr<IStorageSpaceManager> cached = GetProxy();
    if (cached != nullptr) {
        proxy = cached;
        return E_OK;
    }
    int32_t err = LoadStorageSpaceManagerService();
    if (err != E_OK) {
        return err;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    proxy = storageSpaceManager_;
    if (proxy == nullptr) {
        return E_SERVICE_IS_NULLPTR;
    }
    return E_OK;
}

int32_t StorageSpaceManagerClient::GetTotalSize(int64_t &totalSize)
{
    LOGI("GetTotalSize called");
    sptr<IStorageSpaceManager> proxy;
    int32_t err = Connect(proxy);
    if (err != E_OK) {
        return err;
    }
    return proxy->GetTotalSize(totalSize);
}

int32_t StorageSpaceManagerClient::GetSystemSize(int64_t &systemSize)
{
    LOGI("GetSystemSize called");
    sptr<IStorageSpaceManager> proxy;
    int32_t err = Connect(proxy);
    if (err != E_OK) {
        return err;
    }
    return proxy->GetSystemSize(systemSize);
}

int32_t StorageSpaceManagerClient::GetFreeSize(int64_t &freeSize)
{
    LOGI("GetFreeSize called");
    sptr<IStorageSpaceManager> proxy;
    int32_t err = Connect(proxy);
    if (err != E_OK) {
        return err;
    }
    return proxy->GetFreeSize(freeSize);
}

int32_t StorageSpaceManagerClient::GetTotalInodes(int64_t &totalInodes)
{
    LOGI("GetTotalInodes called");
    sptr<IStorageSpaceManager> proxy;
    int32_t err = Connect(proxy);
    if (err != E_OK) {
        return err;
    }
    return proxy->GetTotalInodes(totalInodes);
}

int32_t StorageSpaceManagerClient::GetFreeInodes(int64_t &freeInodes)
{
    LOGI("GetFreeInodes called");
    sptr<IStorageSpaceManager> proxy;
    int32_t err = Connect(proxy);
    if (err != E_OK) {
        return err;
    }
    return proxy->GetFreeInodes(freeInodes);
}

int32_t StorageSpaceManagerClient::CleanBundleCache(int32_t userId)
{
    LOGI("CleanBundleCache userId=%{public}d", userId);
    sptr<IStorageSpaceManager> proxy;
    int32_t err = Connect(proxy);
    if (err != E_OK) {
        return err;
    }
    return proxy->CleanBundleCache(userId);
}

} // namespace StorageSpaceManager
} // namespace OHOS
