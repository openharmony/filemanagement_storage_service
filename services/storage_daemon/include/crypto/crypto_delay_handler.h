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

 #ifndef STORAGE_DAEMON_CRYPTO_DELAY_HANDLER_H
 #define STORAGE_DAEMON_CRYPTO_DELAY_HANDLER_H
 
 #include <atomic>
 #include <mutex>
 #include <vector>
 
 #include "timer.h"
 
 #include "base_key.h"
 #include "libfscrypt/key_control.h"
 
 namespace OHOS {
 namespace StorageDaemon {
 class DelayHandler {
 public:
     DelayHandler(uint32_t userId);
     ~DelayHandler();
     void StartDelayTask(const std::shared_ptr<BaseKey> &el4Key);
     void CancelDelayTask();
 
 private:
     uint32_t timerId_;
     Utils::Timer timer_ {"DeativationEl3El4El5_Task_Timer"};
 
     std::atomic<bool> cancelled_;
     std::shared_ptr<BaseKey> el4Key_;
     uint32_t userId_;
 
     std::mutex handlerMutex_;
 
     void DeactiveEl3El4El5();
     int64_t GetTickCount();
 };
 } // StorageDaemon
 } // OHOS
 
 #endif // STORAGE_DAEMON_CRYPTO_DELAY_HANDLER_H
