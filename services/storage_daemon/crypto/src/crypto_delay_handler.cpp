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

#include "crypto_delay_handler.h"

#include <pthread.h>

#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/storage_radar.h"
 
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t WAIT_THREAD_TIMEOUT_MS = 5;
constexpr int32_t DEFAULT_CHECK_INTERVAL = 10 * 1000; // 10s
const std::string CLEAR_TASK_NAME = "clear_ece_sece_key";

DelayHandler::DelayHandler(uint32_t userId) : userId_(userId) {}
DelayHandler::~DelayHandler()
{
    LOGI("DelayHandler Destructor.");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if ((eventHandler_ != nullptr) && (eventHandler_->GetEventRunner() != nullptr)) {
        eventHandler_->RemoveAllEvents();
        eventHandler_->GetEventRunner()->Stop();
    }
    if (eventThread_.joinable()) {
        eventThread_.join();
    }
    eventHandler_ = nullptr;
    LOGI("success");
}

void DelayHandler::StartDelayTask(std::shared_ptr<BaseKey> &el4Key)
{
    CancelDelayTask();
    if (el4Key == nullptr) {
        LOGI("elKey is nullptr do not clean.");
        return;
    }
    el4Key_ = el4Key;

    LOGI("StartDelayTask, start delay clear key task.");
    std::unique_lock<std::mutex> lock(eventMutex_);
    if (eventHandler_ == nullptr) {
        eventThread_ = std::thread(&DelayHandler::StartDelayHandler, this);
        eventCon_.wait_for(lock, std::chrono::seconds(WAIT_THREAD_TIMEOUT_MS), [this] {
            return eventHandler_ != nullptr;
        });
    }

    auto executeFunc = [this] { DeactiveEl3El4El5(); };
    eventHandler_->PostTask(executeFunc, CLEAR_TASK_NAME, DEFAULT_CHECK_INTERVAL,
                            AppExecFwk::EventHandler::Priority::IMMEDIATE);
    LOGI("success");
}

void DelayHandler::StartDelayHandler()
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
    LOGI("success");
}

void DelayHandler::CancelDelayTask()
{
    LOGI("enter");
    if (eventHandler_ == nullptr) {
        LOGE("eventHandler_ is nullptr !");
        return;
    }
    eventHandler_->RemoveTask(CLEAR_TASK_NAME);
    LOGI("success");
}

void DelayHandler::DeactiveEl3El4El5()
{
    LOGI("enter");
    if (el4Key_ == nullptr) {
        LOGI("elKey is nullptr do not clean.");
        StorageRadar::ReportUpdateUserAuth("DeactiveEl3El4El5", userId_, E_PARAMS_INVAL, "EL4", "");
        return;
    }
    if (!el4Key_->LockUserScreen(userId_, FSCRYPT_SDP_ECE_CLASS)) {
        LOGE("Clear user %{public}u key failed", userId_);
        StorageRadar::ReportUpdateUserAuth("DeactiveEl3El4El5::LockUserScreen", userId_, E_SYS_ERR, "EL4", "");
        return;
    }
    LOGW("success");
}
} // StorageDaemon
} // OHOS