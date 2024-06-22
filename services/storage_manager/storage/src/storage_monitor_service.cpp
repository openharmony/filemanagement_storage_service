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

#include "storage/storage_monitor_service.h"

#include <cstdlib>
#include <cstring>
#include <mntent.h>
#include <pthread.h>
#include <singleton.h>
#include <sys/statvfs.h>
#include <unordered_set>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage/bundle_manager_connector.h"
#include "storage/storage_total_status_service.h"

namespace OHOS {
namespace StorageManager {
constexpr int32_t CONST_NUM_TWO = 2;
constexpr int32_t CONST_NUM_THREE = 3;
constexpr int32_t DEFAULT_CHECK_INTERVAL = 60 * 1000; // 60s
constexpr int32_t STORAGE_THRESHOLD_PERCENTAGE = 5; // 5%
constexpr int64_t STORAGE_THRESHOLD_MAX_BYTES = 500 * 1024 * 1024; // 500M

StorageMonitorService::StorageMonitorService()
{
    LOGI("StorageMonitorService Constructor.");
}

StorageMonitorService::~StorageMonitorService()
{
    LOGI("StorageMonitorService Destructor.");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if ((eventHandler_ != nullptr) && (eventHandler_->GetEventRunner() != nullptr)) {
        eventHandler_->RemoveAllEvents();
        eventHandler_->GetEventRunner()->Stop();
    }
    if (eventThread_.joinable()) {
        eventThread_.join();
    }
    eventHandler_ = nullptr;
}

int32_t StorageMonitorService::StartStorageMonitorTask()
{
    LOGI("StorageMonitorService, start deicve storage monitor task.");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if (eventHandler_ == nullptr) {
        eventThread_ = std::thread(&StorageMonitorService::StartEventHandler, this);
        eventCon_.wait(lock, [this] {
            return eventHandler_ != nullptr;
        });
    }

    auto executeFunc = [this] { Execute(); };
    eventHandler_->PostTask(executeFunc, DEFAULT_CHECK_INTERVAL);
}

void StorageMonitorService::StartEventHandler()
{
    pthread_setname_np(pthread_self(), "storage_monitor_task_event");
    auto runner = AppExecFwk::EventRunner::Create(false);
    if (runner == nullptr) {
        LOGE("event runner is nullptr.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
}

void StorageMonitorService::Execute()
{
    if (eventHandler_ == nullptr) {
        LOGE("event handler is nullptr.");
        return;
    }
    CheckAndCleanBundleCache();
    auto executeFunc = [this] { Execute(); };
    eventHandler_->PostTask(executeFunc, DEFAULT_CHECK_INTERVAL);
}

void StorageMonitorService::CheckAndCleanBundleCache()
{
    int64_t totalSize;
    int32_t err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetTotalSize(totalSize);
    if ((err != E_OK) || (totalSize <= 0)) {
        LOGE("Get device total size failed.");
        return;
    }

    int64_t freeSize;
    err = DelayedSingleton<StorageTotalStatusService>::GetInstance()->GetFreeSize(freeSize);
    if ((err != E_OK) || (freeSize <= 0)) {
        LOGE("Get device free size failed.");
        return;
    }

    int64_t lowThreshold = GetLowerThreshold(totalSize);
    if (lowThreshold <= 0) {
        LOGE("Lower threshold value is invalid.");
        return;
    }

    if (freeSize >= (lowThreshold * CONST_NUM_THREE) / CONST_NUM_TWO) {
        LOGI("The clean cache threshold had not been reached, skip this event.");
        return;
    }

    auto bundleMgr = DelayedSingleton<BundleMgrConnector>::GetInstance()->GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("StorageMonitorService::CleanBundleCacheFiles connect bundlemgr failed");
        return;
    }
    ErrCode ret = bundleMgr->CleanBundleCacheFilesAutomatic(lowThreshold * 2);
}

int64_t StorageMonitorService::GetLowerThreshold(int64_t totalSize)
{
    int64_t lowBytes = (totalSize * STORAGE_THRESHOLD_PERCENTAGE) / 100;
    return (lowBytes < STORAGE_THRESHOLD_MAX_BYTES) ? lowBytes : STORAGE_THRESHOLD_MAX_BYTES;
}
} // StorageManager
} // OHOS
