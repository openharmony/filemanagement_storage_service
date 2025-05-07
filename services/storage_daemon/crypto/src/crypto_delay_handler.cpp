/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t DEFAULT_CHECK_INTERVAL = 10 * 1000; // 10s

DelayHandler::DelayHandler(uint32_t userId): timerId_(0), cancelled_(false), userId_(userId)
{
    timer_.Setup();
}

DelayHandler::~DelayHandler()
{
    LOGI("DelayHandler Destructor.");
    timer_.Shutdown();
    LOGI("DelayHandler::Destruct success.");
}

void DelayHandler::StartDelayTask(const std::shared_ptr<BaseKey>& el4Key)
{
    LOGI("DelayHandler::StartDelayTask: enter.");
    CancelDelayTask();
    std::lock_guard<std::mutex> lock(handlerMutex_);
    if (el4Key == nullptr) {
        LOGI("elKey is nullptr do not clean.");
        return;
    }
    el4Key_ = el4Key;
    LOGI("DelayHandler::StartDelayTask, start delay clear key task.");
    cancelled_ = false;
    std::string curTime = std::to_string(GetTickCount());
    std::string expExeTime = std::to_string(GetTickCount() + DEFAULT_CHECK_INTERVAL);
    LOGI("DelayHandler: start timer for user=%{public}d, curTime=%{public}s ms, exeTime=%{public}s ms.",
        userId_, curTime.c_str(), expExeTime.c_str());
    timerId_ = timer_.Register([this, expExeTime]() {
            std::string realExeTime = std::to_string(GetTickCount());
            LOGI("DelayHandler: EXECUTE for user=%{public}d, curTime=%{public}s ms, expExeTime=%{public}s ms.",
                userId_, realExeTime.c_str(), expExeTime.c_str());
            DeactiveEl3El4El5();
        }, DEFAULT_CHECK_INTERVAL, true);

    LOGI("DelayHandler::success.");
}

void DelayHandler::CancelDelayTask()
{
    LOGI("DelayHandler::CancelDelayTask:: enter.");
    std::lock_guard<std::mutex> lock(handlerMutex_);
    timer_.Unregister(timerId_);
    cancelled_ = true;
    
    LOGI("DelayHandler::CancelDelayTask:: success.");
}

void DelayHandler::DeactiveEl3El4El5()
{
    LOGI("DelayHandler::DeactiveEl3El4El5:: enter.");
    std::lock_guard<std::mutex> lock(handlerMutex_);
    if (el4Key_ == nullptr) {
        LOGI("DelayHandler::DeactiveEl3El4El5:: elKey is nullptr do not clean.");
        StorageRadar::ReportUpdateUserAuth("DeactiveEl3El4El5", userId_, E_PARAMS_INVALID, "EL4", "");
        return;
    }
    if (cancelled_) {
        LOGI(" DelayHandler: task is cancelled.");
        return;
    }
    int32_t ret = el4Key_->LockUserScreen(userId_, FSCRYPT_SDP_ECE_CLASS);
    if (ret != E_OK) {
        LOGE("DelayHandler::DeactiveEl3El4El5:: Clear user %{public}u key failed.", userId_);
        StorageRadar::ReportUpdateUserAuth("DeactiveEl3El4El5::LockUserScreen", userId_, E_SYS_KERNEL_ERR, "EL4", "");
        return;
    }
    cancelled_ = false;
    LOGW("success");
}

int64_t DelayHandler::GetTickCount()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
} // StorageDaemon
} // OHOS