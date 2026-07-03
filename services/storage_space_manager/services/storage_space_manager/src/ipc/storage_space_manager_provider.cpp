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

#include "storage_space_manager_provider.h"

#include <cinttypes>
#include <system_ability_definition.h>

#include "cache_clean_controller.h"
#include "common_event/storage_common_event_subscriber.h"
#include "ipc_caller_auth.h"
#include "storage/storage_total_status_service.h"
#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"

#include "errors.h"
#include "event_handler.h"
#include "event_runner.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "mem_mgr_client.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace OHOS::StorageSpaceManager;

namespace {
const std::string UNLOAD_TASK_ID = "unload_ssm_svr";
constexpr int32_t DELAY_UNLOAD_TIME = 180000; // 180s
constexpr int32_t SA_READY_INTO_IDLE = 0;
constexpr int32_t SA_REFUSE_INTO_IDLE = -1;
}

REGISTER_SYSTEM_ABILITY_BY_ID(StorageSpaceManagerProvider, STORAGE_SPACE_MANAGER_SA_ID, false);

StorageSpaceManagerProvider::StorageSpaceManagerProvider(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
    LOGI("StorageSpaceManagerProvider instance created");
}

void StorageSpaceManagerProvider::OnStart(const SystemAbilityOnDemandReason& startReason)
{
    LOGI("OnStart reason=%{public}s", startReason.GetName().c_str());

    std::unordered_set<int32_t> deps = {
        BUNDLE_MGR_SERVICE_SYS_ABILITY_ID,
    };
    {
        std::lock_guard<std::mutex> lock(depSaIdsMtx_);
        depSaIds_ = deps;
    }
    for (int32_t saId : deps) {
        AddSystemAbilityListener(saId);
    }
}

void StorageSpaceManagerProvider::OnStop()
{
    LOGI("OnStop called");
    isStopped_ = true;
    serviceReady_.store(false, std::memory_order_release);
    
    DestroyUnloadHandler();

    Memory::MemMgrClient::GetInstance().SetCritical(getpid(), false, STORAGE_SPACE_MANAGER_SA_ID);
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 0, STORAGE_SPACE_MANAGER_SA_ID);
    LOGI("OnStop done");
}

void StorageSpaceManagerProvider::OnActive(const SystemAbilityOnDemandReason& activeReason)
{
    LOGI("OnActive reasonId=%{public}d, name=%{public}s",
        activeReason.GetId(), activeReason.GetName().c_str());
    Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, STORAGE_SPACE_MANAGER_SA_ID);
}

int32_t StorageSpaceManagerProvider::OnIdle(const SystemAbilityOnDemandReason& idleReason)
{
    LOGI("idleReason name=%{public}s, id=%{public}d, value=%{public}s", idleReason.GetName().c_str(),
        idleReason.GetId(), idleReason.GetValue().c_str());
    if (IsReadyIntoIdle()) {
        Memory::MemMgrClient::GetInstance().SetCritical(getpid(), false, STORAGE_SPACE_MANAGER_SA_ID);
        return SA_READY_INTO_IDLE;
    }
    return SA_REFUSE_INTO_IDLE;
}

void StorageSpaceManagerProvider::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility saId=%{public}d", systemAbilityId);
    if (serviceReady_.load(std::memory_order_acquire)) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(depSaIdsMtx_);
        if (depSaIds_.empty()) {
            return;
        }
        depSaIds_.erase(systemAbilityId);
        if (!depSaIds_.empty()) {
            LOGI("Waiting for dependent SAs, remaining=%{public}zu", depSaIds_.size());
            return;
        }
    }
    if (!Init()) {
        LOGE("Init failed");
        return;
    }
    if (!Publish(this)) {
        LOGE("Publish SA failed");
        return;
    }
    if (CreateUnloadHandler() == E_OK) {
        DelayUnloadTask();
    }
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 1, STORAGE_SPACE_MANAGER_SA_ID);
    Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, STORAGE_SPACE_MANAGER_SA_ID);
    LOGI("Service started successfully");
}

bool StorageSpaceManagerProvider::Init()
{
    LOGI("Init begin");
    serviceReady_.store(true, std::memory_order_release);
    LOGI("Init success");
    return true;
}

int32_t StorageSpaceManagerProvider::CreateUnloadHandler()
{
    std::lock_guard<std::mutex> lock(unloadMutex_);
    if (unloadHandler_ != nullptr) {
        return E_OK;
    }
    unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(
        AppExecFwk::EventRunner::Create(false));
    if (unloadHandler_ == nullptr) {
        LOGE("Create unload handler failed");
        return E_FAIL;
    }
    return E_OK;
}

int32_t StorageSpaceManagerProvider::DestroyUnloadHandler()
{
    std::lock_guard<std::mutex> lock(unloadMutex_);
    if (unloadHandler_ == nullptr) {
        return E_OK;
    }
    unloadHandler_->RemoveTask(UNLOAD_TASK_ID);
    unloadHandler_ = nullptr;
    return E_OK;
}

void StorageSpaceManagerProvider::DelayUnloadTask()
{
    std::lock_guard<std::mutex> lock(unloadMutex_);
    if (unloadHandler_ == nullptr) {
        LOGE("UnloadHandler is nullptr");
        return;
    }
    unloadHandler_->RemoveTask(UNLOAD_TASK_ID);
    auto task = []() {
        LOGI("DelayUnloadTask: unloading SA");
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            LOGE("Get samgr failed");
            return;
        }
        int32_t ret = samgr->UnloadSystemAbility(STORAGE_SPACE_MANAGER_SA_ID);
        if (ret != ERR_OK) {
            LOGE("UnloadSystemAbility failed, ret=%{public}d", ret);
            return;
        }
        LOGI("UnloadSystemAbility success");
    };
    unloadHandler_->PostTask(task, UNLOAD_TASK_ID, DELAY_UNLOAD_TIME);
    LOGI("DelayUnloadTask posted, delay=%{public}dms", DELAY_UNLOAD_TIME);
}

bool StorageSpaceManagerProvider::IsReadyIntoIdle()
{
    int32_t count = runningIpcCount_.load();
    if (count > 0) {
        LOGI("IPC running, count=%{public}d, refuse idle", count);
        return false;
    }
    LOGI("ready into idle!");
    return true;
}

bool StorageSpaceManagerProvider::ExitIdleState()
{
    if (!CancelIdle()) {
        LOGI("Cancel idle failed!");
        return false;
    }
    return true;
}

void StorageSpaceManagerProvider::AddRunningIpcCount()
{
    runningIpcCount_.fetch_add(1);
}

void StorageSpaceManagerProvider::SubtractRunningIpcCount()
{
    runningIpcCount_.fetch_sub(1);
}

int32_t StorageSpaceManagerProvider::GetTotalSize(int64_t &totalSize)
{
    if (!serviceReady_.load(std::memory_order_acquire)) {
        LOGE("Service is not ready");
        return E_SERVICE_NOT_READY;
    }
    LOGI("GetTotalSize called");
    if (!IpcCallerAuth::VerifyCallerPermission(PERMISSION_STORAGE_MANAGER)) {
        LOGE("Permission denied, need %{public}s", PERMISSION_STORAGE_MANAGER.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!ExitIdleState()) {
        return E_SERVICE_ON_IDLE;
    }
    AddRunningIpcCount();
    int32_t ret = StorageTotalStatusService::GetInstance().GetTotalSize(totalSize);
    SubtractRunningIpcCount();
    return ret;
}

int32_t StorageSpaceManagerProvider::GetSystemSize(int64_t &systemSize)
{
    if (!serviceReady_.load(std::memory_order_acquire)) {
        LOGE("Service is not ready");
        return E_SERVICE_NOT_READY;
    }
    LOGI("GetSystemSize called");
    if (!IpcCallerAuth::VerifyCallerPermission(PERMISSION_STORAGE_MANAGER)) {
        LOGE("Permission denied, need %{public}s", PERMISSION_STORAGE_MANAGER.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!ExitIdleState()) {
        return E_SERVICE_ON_IDLE;
    }
    AddRunningIpcCount();
    int32_t ret = StorageTotalStatusService::GetInstance().GetSystemSize(systemSize);
    SubtractRunningIpcCount();
    return ret;
}

int32_t StorageSpaceManagerProvider::GetFreeSize(int64_t &freeSize)
{
    if (!serviceReady_.load(std::memory_order_acquire)) {
        LOGE("Service is not ready");
        return E_SERVICE_NOT_READY;
    }
    LOGI("GetFreeSize called");
    if (!IpcCallerAuth::VerifyCallerPermission(PERMISSION_STORAGE_MANAGER)) {
        LOGE("Permission denied, need %{public}s", PERMISSION_STORAGE_MANAGER.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!ExitIdleState()) {
        return E_SERVICE_ON_IDLE;
    }
    AddRunningIpcCount();
    int32_t ret = StorageTotalStatusService::GetInstance().GetFreeSize(freeSize);
    SubtractRunningIpcCount();
    return ret;
}

int32_t StorageSpaceManagerProvider::GetTotalInodes(int64_t &totalInodes)
{
    if (!serviceReady_.load(std::memory_order_acquire)) {
        LOGE("Service is not ready");
        return E_SERVICE_NOT_READY;
    }
    LOGI("GetTotalInodes called");
    if (!IpcCallerAuth::VerifyCallerPermission(PERMISSION_STORAGE_MANAGER)) {
        LOGE("Permission denied, need %{public}s", PERMISSION_STORAGE_MANAGER.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!ExitIdleState()) {
        return E_SERVICE_ON_IDLE;
    }
    AddRunningIpcCount();
    int32_t ret = StorageTotalStatusService::GetInstance().GetTotalInodes(totalInodes);
    SubtractRunningIpcCount();
    return ret;
}

int32_t StorageSpaceManagerProvider::GetFreeInodes(int64_t &freeInodes)
{
    if (!serviceReady_.load(std::memory_order_acquire)) {
        LOGE("Service is not ready");
        return E_SERVICE_NOT_READY;
    }
    LOGI("GetFreeInodes called");
    if (!IpcCallerAuth::VerifyCallerPermission(PERMISSION_STORAGE_MANAGER)) {
        LOGE("Permission denied, need %{public}s", PERMISSION_STORAGE_MANAGER.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!ExitIdleState()) {
        return E_SERVICE_ON_IDLE;
    }
    AddRunningIpcCount();
    int32_t ret = StorageTotalStatusService::GetInstance().GetFreeInodes(freeInodes);
    SubtractRunningIpcCount();
    return ret;
}

int32_t StorageSpaceManagerProvider::CleanBundleCache(int32_t userId)
{
    if (!serviceReady_.load(std::memory_order_acquire)) {
        LOGE("Service is not ready");
        return E_SERVICE_NOT_READY;
    }
    if (!IpcCallerAuth::VerifyCallerPermission(PERMISSION_STORAGE_MANAGER)) {
        LOGE("Permission denied, need %{public}s", PERMISSION_STORAGE_MANAGER.c_str());
        return E_PERMISSION_DENIED;
    }
    if (!ExitIdleState()) {
        return E_SERVICE_ON_IDLE;
    }
    AddRunningIpcCount();
    int32_t ret = DelayedSingleton<CacheCleanController>::GetInstance()->CleanBundleCache(userId);
    SubtractRunningIpcCount();
    return ret;
}

} // namespace StorageSpaceManager
} // namespace OHOS
