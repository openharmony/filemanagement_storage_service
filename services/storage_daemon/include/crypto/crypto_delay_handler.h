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

#ifndef STORAGE_DAEMON_CRYPTO_DELAY_HANDLER_H
#define STORAGE_DAEMON_CRYPTO_DELAY_HANDLER_H

#include <atomic>
#include <thread>
#include <vector>

#include "base_key.h"
#include "event_handler.h"
#include "libfscrypt/key_control.h"

namespace OHOS {
namespace StorageDaemon {
class DelayHandler {
static const uint8_t CANCEL_CLEAR_ECE_SECE_TASK = 0;
static const uint8_t START_CLEAR_ECE_SECE_TASK = 1;
public:
    DelayHandler(uint32_t userId);
    ~DelayHandler();
    void StartDelayTask(std::shared_ptr<BaseKey> &elKey);
    void CancelDelayTask();

private:
    void StartDelayHandler();
    void ClearEceSeceKey();

    std::mutex eventMutex_;
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    std::shared_ptr<BaseKey> el4Key_;
    uint32_t userId_;
};
} // StorageDaemon
} // OHOS

#endif // STORAGE_DAEMON_CRYPTO_DELAY_HANDLER_H
